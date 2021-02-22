#include "Terminal.h"

#define LTIMING(x) // RTIMING(x)

namespace Upp {

#define VT_CTL(id, cbyte, minlevel, maxlevel)                           \
{                                                                       \
    { cbyte },                                                          \
    { ControlId::id, MAKEWORD(TerminalCtrl::minlevel, TerminalCtrl::maxlevel) } \
}

TerminalCtrl::ControlId TerminalCtrl::FindControlId(byte ctl, byte level)
{
	const static VectorMap<dword, Tuple<ControlId, word> > vtcbytes = {
        VT_CTL(NUL,         0x00,   LEVEL_0, LEVEL_4),	// Ignored
        VT_CTL(ENQ,         0x05,   LEVEL_0, LEVEL_4),	// Terminal status request
        VT_CTL(BEL,         0x07,   LEVEL_0, LEVEL_4),	// Audio or visual bell
        VT_CTL(BS,          0x08,   LEVEL_0, LEVEL_4),	// Backspace
        VT_CTL(HT,          0x09,   LEVEL_0, LEVEL_4),	// Horizontal tab. Move the cursor to next tab stop
        VT_CTL(LF,          0x0A,   LEVEL_0, LEVEL_4),	// Line feed
        VT_CTL(VT,          0x0B,   LEVEL_0, LEVEL_4),	// Vertical tab (treated as LF)
        VT_CTL(FF,          0x0C,   LEVEL_0, LEVEL_4),	// Form feed (treated as LF)
        VT_CTL(CR,          0x0D,   LEVEL_0, LEVEL_4),	// Carriage return
        VT_CTL(LS1,         0x0E,   LEVEL_1, LEVEL_4),	// Invoke and locks G1 into GL
        VT_CTL(LS0,         0x0F,   LEVEL_1, LEVEL_4),	// Invoke and locks G0 into GL
        VT_CTL(XON,         0x11,   LEVEL_0, LEVEL_4),	// Resume data transfer
        VT_CTL(XOFF,        0x13,   LEVEL_0, LEVEL_4),	// Pause data transfer
        VT_CTL(DEL,         0x7F,   LEVEL_0, LEVEL_4),	// Ignored
        VT_CTL(IND,         0x84,   LEVEL_1, LEVEL_4),	// Index
        VT_CTL(NEL,         0x85,   LEVEL_1, LEVEL_4),	// Move to next line
        VT_CTL(HTS,         0x88,   LEVEL_1, LEVEL_4),	// Sets a tab at the active cursor position
        VT_CTL(RI,          0x8D,   LEVEL_1, LEVEL_4),	// Reverse index
        VT_CTL(SS2,         0x8E,   LEVEL_1, LEVEL_4),	// Temporarily invoke G2 to GL
        VT_CTL(SS3,         0x8F,   LEVEL_1, LEVEL_4),	// Temporarily invoke G3 to GL
        VT_CTL(SPA,         0x96,   LEVEL_2, LEVEL_4),	// Start of protected area
        VT_CTL(EPA,         0x97,   LEVEL_2, LEVEL_4),	// End of protected area
        VT_CTL(DECID,       0x9A,   LEVEL_1, LEVEL_4),	// Report terminal ID
        VT_CTL(ST,          0x9C,   LEVEL_1, LEVEL_4)	// String terminator
    };

    const auto *p = vtcbytes.FindPtr(ctl);
    return p && LOBYTE(p->b) <= level && level <= HIBYTE(p->b) ? p->a : ControlId::UNHANDLED;
}

force_inline
constexpr dword MakeVTSequenceKey(byte a, byte b, byte c, byte d) noexcept
{
	dword hash = 0;
	hash = (0xacf34ce7 * hash) ^ a;
	hash = (0xacf34ce7 * hash) ^ b;
	hash = (0xacf34ce7 * hash) ^ c;
	hash = (0xacf34ce7 * hash) ^ d;
	return hash;
}

#define VT_SEQUENCE(seq, id, opcode, mode, interm, minlevel, maxlevel)      \
{                                                                           \
    { MakeVTSequenceKey(VTInStream::Sequence::seq, opcode, mode, interm) }, \
    { SequenceId::id, MAKEWORD(TerminalCtrl::minlevel, TerminalCtrl::maxlevel)   }  \
}
    
#define VT_ESC(id, opcode, mode, interm, minlevel, maxlevel)                \
    VT_SEQUENCE(ESC, id, opcode, mode, interm, minlevel, maxlevel)

#define VT_CSI(id, opcode, mode, interm, minlevel, maxlevel)                \
    VT_SEQUENCE(CSI, id, opcode, mode, interm, minlevel, maxlevel)

#define VT_DCS(id, opcode, mode, interm, minlevel, maxlevel)                \
    VT_SEQUENCE(DCS, id, opcode, mode, interm, minlevel, maxlevel)

TerminalCtrl::SequenceId TerminalCtrl::FindSequenceId(const VTInStream::Sequence& seq, byte level)
{
    // TODO: Allow up to three intermediate bytes in the sequence lookup table.
    //       See: DEC STD-070, p. 3-40.
	
    const static VectorMap<dword, Tuple<SequenceId, word> > vtsequences = {
        // Escape sequences
        VT_ESC(DECBI,           '6', 0x00, 0x00, LEVEL_4, LEVEL_4),   // Back index
        VT_ESC(DECSC,           '7', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Save cursor
        VT_ESC(DECRC,           '8', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Restore cursor
        VT_ESC(DECALN,          '8', 0x00, '#',  LEVEL_1, LEVEL_4),   // Display alignment test
        VT_ESC(DECFI,           '9', 0x00, 0x00, LEVEL_4, LEVEL_4),   // Forward index
        VT_ESC(S7C1T,           'F', 0x00, ' ',  LEVEL_2, LEVEL_4),   // 7-bits mode
        VT_ESC(S8C1T,           'G', 0x00, ' ',  LEVEL_2, LEVEL_4),   // 8-bits mode.
        VT_ESC(ANSICL1,         'L', 0x00, ' ',  LEVEL_2, LEVEL_4),   // Select ANSI conformance level 1
        VT_ESC(ANSICL2,         'M', 0x00, ' ',  LEVEL_2, LEVEL_4),   // Select ANSI conformance level 2
        VT_ESC(ANSICL3,         'N', 0x00, ' ',  LEVEL_2, LEVEL_4),   // Select ANSI conformance level 3
        VT_ESC(DECKPAM,         '=', 0x00, 0x00, LEVEL_0, LEVEL_4),   // Keypad application mode
        VT_ESC(DECKPNM,         '>', 0x00, 0x00, LEVEL_0, LEVEL_4),   // Keypad mumeric mode
        VT_ESC(RIS,             'c', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Hard reset
        VT_ESC(LS2,             'n', 0x00, 0x00, LEVEL_2, LEVEL_4),   // Invoke G2 into GL
        VT_ESC(LS3,             'o', 0x00, 0x00, LEVEL_2, LEVEL_4),   // Invoke G3 into GL
        VT_ESC(LS3R,            '|', 0x00, 0x00, LEVEL_2, LEVEL_4),   // Invoke G3 into GR
        VT_ESC(LS2R,            '}', 0x00, 0x00, LEVEL_2, LEVEL_4),   // Invoke G2 into GR
        VT_ESC(LS1R,            '~', 0x00, 0x00, LEVEL_2, LEVEL_4),   // Invoke G1 into GR
        // SCS specific escape sequences
        VT_ESC(SCS_G0_DEC_DCS,  '0', 0x00, '(',  LEVEL_1, LEVEL_4),   // Invoke DEC Line-drawing charset into G0
        VT_ESC(SCS_G1_DEC_DCS,  '0', 0x00, ')',  LEVEL_1, LEVEL_4),   // Invoke DEC Line-drawing charset into G1
        VT_ESC(SCS_G2_DEC_DCS,  '0', 0x00, '*',  LEVEL_2, LEVEL_4),   // Invoke DEC Line-drawing charset into G2
        VT_ESC(SCS_G3_DEC_DCS,  '0', 0x00, '+',  LEVEL_2, LEVEL_4),   // Invoke DEC Line-drawing charset into G3
        VT_ESC(SCS_G0_DEC_ACS,  '1', 0x00, '(',  LEVEL_1, LEVEL_4),   // Invoke DEC standard ROM charset into G0 (stubbed)
        VT_ESC(SCS_G1_DEC_ACS,  '1', 0x00, ')',  LEVEL_1, LEVEL_4),   // Invoke DEC standard ROM charset into G1 (stubbed)
        VT_ESC(SCS_G0_DEC_ACS,  '2', 0x00, '(',  LEVEL_1, LEVEL_4),   // Invoke DEC alternate ROM charset into G0 (stubbed)
        VT_ESC(SCS_G1_DEC_ACS,  '2', 0x00, ')',  LEVEL_1, LEVEL_4),   // Invoke DEC alternate ROM charset into G1 (stubbed)
        VT_ESC(SCS_DEFAULT,     '@', 0x00, '%',  LEVEL_3, LEVEL_4),   // Select default charset
        VT_ESC(SCS_G0_ASCII,    'A', 0x00, '(',  LEVEL_1, LEVEL_4),   // Invoke EN-GB charset into G0 (stubbed)
        VT_ESC(SCS_G1_ASCII,    'A', 0x00, ')',  LEVEL_1, LEVEL_4),   // Invoke EN-GB charset into G1 (stubbed)
        VT_ESC(SCS_G2_ASCII,    'A', 0x00, '*',  LEVEL_2, LEVEL_4),   // Invoke EN-GB charset into G2 (stubbed)
        VT_ESC(SCS_G3_ASCII,    'A', 0x00, '+',  LEVEL_2, LEVEL_4),   // Invoke EN-GB charset into G3 (stubbed)
        VT_ESC(SCS_G1_LATIN1,   'A', 0x00, '-',  LEVEL_3, LEVEL_4),   // Invoke DEC/ISO Latin-1 charset into G1
        VT_ESC(SCS_G2_LATIN1,   'A', 0x00, '.',  LEVEL_3, LEVEL_4),   // Invoke DEC/ISO Latin-1 charset into G2
        VT_ESC(SCS_G3_LATIN1,   'A', 0x00, '/',  LEVEL_3, LEVEL_4),   // Invoke DEC/ISO Latin-1 charset into G3
        VT_ESC(SCS_G0_ASCII,    'B', 0x00, '(',  LEVEL_1, LEVEL_4),   // Invoke US-ASCII charset into G0
        VT_ESC(SCS_G1_ASCII,    'B', 0x00, ')',  LEVEL_1, LEVEL_4),   // Invoke US-ASCII charset into G1
        VT_ESC(SCS_G2_ASCII,    'B', 0x00, '*',  LEVEL_2, LEVEL_4),   // Invoke US-ASCII charset into G2
        VT_ESC(SCS_G3_ASCII,    'B', 0x00, '+',  LEVEL_2, LEVEL_4),   // Invoke US-ASCII charset into G3
        VT_ESC(SCS_UTF8,        'G', 0x00, '%',  LEVEL_3, LEVEL_4),   // Select UTF-8 charset
        VT_ESC(SCS_G0_DEC_MCS,  '<', 0x00, '(',  LEVEL_2, LEVEL_4),   // Invoke DEC supplemental charset into G0
        VT_ESC(SCS_G1_DEC_MCS,  '<', 0x00, ')',  LEVEL_2, LEVEL_4),   // Invoke DEC supplemental charset into G1
        VT_ESC(SCS_G2_DEC_MCS,  '<', 0x00, '*',  LEVEL_2, LEVEL_4),   // Invoke DEC supplemental charset into G2
        VT_ESC(SCS_G3_DEC_MCS,  '<', 0x00, '+',  LEVEL_2, LEVEL_4),   // Invoke DEC supplemental charset into G3
        VT_ESC(SCS_G0_DEC_TCS,  '>', 0x00, '(',  LEVEL_3, LEVEL_4),   // Invoke DEC technical charset into G0
        VT_ESC(SCS_G1_DEC_TCS,  '>', 0x00, ')',  LEVEL_3, LEVEL_4),   // Invoke DEC technical charset into G1
        VT_ESC(SCS_G2_DEC_TCS,  '>', 0x00, '*',  LEVEL_3, LEVEL_4),   // Invoke DEC technical charset into G2
        VT_ESC(SCS_G3_DEC_TCS,  '>', 0x00, '+',  LEVEL_3, LEVEL_4),   // Invoke DEC technical charset into G3
        // VT52 specific escape sequences
        VT_ESC(VT52_CUU,        'A', 0x00, 0x00, LEVEL_0, LEVEL_0),   // Move upward
        VT_ESC(VT52_CUD,        'B', 0x00, 0x00, LEVEL_0, LEVEL_0),   // Move downward
        VT_ESC(VT52_CUF,        'C', 0x00, 0x00, LEVEL_0, LEVEL_0),   // Move forward
        VT_ESC(VT52_CUB,        'D', 0x00, 0x00, LEVEL_0, LEVEL_0),   // Move backward
        VT_ESC(VT52_DCS_ON,     'F', 0x00, 0x00, LEVEL_0, LEVEL_0),   // Drawing characters: on
        VT_ESC(VT52_DCS_OFF,    'G', 0x00, 0x00, LEVEL_0, LEVEL_0),   // Drawing characters: off
        VT_ESC(VT52_HOME,       'H', 0x00, 0x00, LEVEL_0, LEVEL_0),   // Move home
        VT_ESC(VT52_RI,         'I', 0x00, 0x00, LEVEL_0, LEVEL_0),   // Reverse index
        VT_ESC(VT52_ED,         'J', 0x00, 0x00, LEVEL_0, LEVEL_0),   // Erase to end of screen
        VT_ESC(VT52_EL,         'K', 0x00, 0x00, LEVEL_0, LEVEL_0),   // Erase to end of line
        VT_ESC(VT52_CUP,        'Y', 0x00, 0x00, LEVEL_0, LEVEL_0),   // Direct cursor addressing
        VT_ESC(VT52_DA,         'Z', 0x00, 0x00, LEVEL_0, LEVEL_0),   // Device identify
        VT_ESC(VT52_DECANM,     '<', 0x00, 0x00, LEVEL_0, LEVEL_0),   // Enter ANSI mode (leave VT52 mode)
        // Command sequences
        VT_CSI(ICH,             '@', 0x00, 0x00, LEVEL_2, LEVEL_4),   // Insert character
        VT_CSI(SL,              '@', 0x00, ' ',  LEVEL_3, LEVEL_4),   // Scroll/shift left
        VT_CSI(CUU,             'A', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Move upward
        VT_CSI(SR,              'A', 0x00, ' ',  LEVEL_3, LEVEL_4),   // Scroll/shift right
        VT_CSI(CUD,             'B', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Move downward
        VT_CSI(CUF,             'C', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Move forward
        VT_CSI(CUB,             'D', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Move backward
        VT_CSI(CNL,             'E', 0x00, 0x00, LEVEL_4, LEVEL_4),   // Move to next line (no scrolling)
        VT_CSI(CPL,             'F', 0x00, 0x00, LEVEL_4, LEVEL_4),   // Move to prev line (no scrolling)
        VT_CSI(CHA,             'G', 0x00, 0x00, LEVEL_4, LEVEL_4),   // Cursor horizontal absolute
        VT_CSI(CUP,             'H', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Cursor position
        VT_CSI(CHT,             'I', 0x00, 0x00, LEVEL_4, LEVEL_4),   // Cursor horizontal tabulation
        VT_CSI(ED,              'J', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Erase screen
        VT_CSI(DECSED,          'J', '?',  0x00, LEVEL_2, LEVEL_4),   // Selectively erase screen
        VT_CSI(EL,              'K', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Erase line
        VT_CSI(DECSEL,          'K', '?',  0x00, LEVEL_2, LEVEL_4),   // Selectively erase line
        VT_CSI(IL,              'L', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Insert line
        VT_CSI(DL,              'M', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Remove line
        VT_CSI(DCH,             'P', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Delete character
        VT_CSI(SU,              'S', 0x00, 0x00, LEVEL_3, LEVEL_4),   // Scroll up
        VT_CSI(SD,              'T', 0x00, 0x00, LEVEL_3, LEVEL_4),   // Scroll down
        VT_CSI(DECST8C,         'W', '?',  0x00, LEVEL_4, LEVEL_4),   // Set a tab at every 8 columns
        VT_CSI(ECH,             'X', 0x00, 0x00, LEVEL_2, LEVEL_4),   // Erase character
        VT_CSI(CBT,             'Z', 0x00, 0x00, LEVEL_4, LEVEL_4),   // Cursor backward tabulation
        VT_CSI(ECH,             '^', 0x00, 0x00, LEVEL_3, LEVEL_4),   // FIXME
        VT_CSI(HPA,             '`', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Horizontal position absolute
        VT_CSI(HPR,             'a', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Horizontal position relative
        VT_CSI(REP,             'b', 0x00, 0x00, LEVEL_3, LEVEL_4),   // Repeat last character
        VT_CSI(DA1,             'c', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Send primary device attributes
        VT_CSI(DA2,             'c', '>',  0x00, LEVEL_1, LEVEL_4),   // Send secondary device attributes
        VT_CSI(DA3,             'c', '=',  0x00, LEVEL_4, LEVEL_4),   // Send tertiary device attributes
        VT_CSI(VPA,             'd', 0x00, 0x00, LEVEL_4, LEVEL_4),   // Vertical position absolute
        VT_CSI(VPR,             'e', 0x00, 0x00, LEVEL_4, LEVEL_4),   // Vertical position relative
        VT_CSI(HVP,             'f', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Cursor horz and vert position
        VT_CSI(TBC,             'g', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Clear tabs
        VT_CSI(SM,              'h', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Set ANSI modes
        VT_CSI(SM,              'h', '?',  0x00, LEVEL_1, LEVEL_4),   // Set private modes
        VT_CSI(RM,              'l', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Reset ANSI modes
        VT_CSI(RM,              'l', '?',  0x00, LEVEL_1, LEVEL_4),   // Reset private modes
        VT_CSI(SGR,             'm', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Select graphics rendition (ANSI)
        VT_CSI(SGR,             'm', '?',  0x00, LEVEL_1, LEVEL_4),   // Select graphics rendition (DEC)
        VT_CSI(DSR,             'n', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Send device status report (ANSI format)
        VT_CSI(DECDSR,          'n', '?',  0x00, LEVEL_1, LEVEL_4),   // Send device status (cursor position) report (DEC format)
        VT_CSI(DECSCL,          'p', 0x00, '\"', LEVEL_1, LEVEL_4),   // Select device conformance level
        VT_CSI(DECSTR,          'p', 0x00, '!',  LEVEL_2, LEVEL_4),   // Soft reset
        VT_CSI(DECRQM,          'p', 0x00, '$',  LEVEL_2, LEVEL_4),   // Request ANSI mode
        VT_CSI(DECRQM,          'p', '?',  '$',  LEVEL_2, LEVEL_4),   // Request private mode
        VT_CSI(DECLL,           'q', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Set programmable LEDs
        VT_CSI(DECSCA,          'q', 0x00, '\"', LEVEL_2, LEVEL_4),   // Set character protection attribute
        VT_CSI(DECSCUSR,        'q', 0x00, ' ',  LEVEL_4, LEVEL_4),   // Set cursor style
        VT_CSI(DECSTBM,         'r', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Set vertical margins
        VT_CSI(DECCARA,         'r', 0x00, '$',  LEVEL_4, LEVEL_4),   // Change attributes in rectangular area
        VT_CSI(DECSLRM,         's', 0x00, 0x00, LEVEL_3, LEVEL_4),   // Set horizontal margins / SCO save cursor
        VT_CSI(DECSLPP,         't', 0x00, 0x00, LEVEL_3, LEVEL_4),   // Set lines per page
        VT_CSI(DECRARA,         't', 0x00, '$',  LEVEL_4, LEVEL_4),   // Reverse attributes in rectangular area
        VT_CSI(SCORC,           'u', 0x00, 0x00, LEVEL_3, LEVEL_4),   // SCO restore cursor
        VT_CSI(DECCRA,          'v', 0x00, '$',  LEVEL_4, LEVEL_4),   // Copy rectangular area.
        VT_CSI(DECRQPSR,        'w', 0x00, '$',  LEVEL_3, LEVEL_4),   // Request presentation state report
        VT_CSI(DECREQTPARM,     'x', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Request terminal parameters
        VT_CSI(DECFRA,          'x', 0x00, '$',  LEVEL_4, LEVEL_4),   // Fill rectangular area
        VT_CSI(DECSACE,         'x', 0x00, '*',  LEVEL_4, LEVEL_4),   // Select rectangular area attribute change extent.
        VT_CSI(DECTST,          'y', 0x00, 0x00, LEVEL_1, LEVEL_4),   // Device confidence tests
        VT_CSI(DECRQCRA,        'y', 0x00, '*',  LEVEL_4, LEVEL_4),   // Request rectangular area checksum
        VT_CSI(DECERA,          'z', 0x00, '$',  LEVEL_4, LEVEL_4),   // Erase rectangular area
        VT_CSI(DECSERA,         '{', 0x00, '$',  LEVEL_4, LEVEL_4),   // Selectively erase rectangular area
        VT_CSI(DECSCPP,         '|', 0x00, '$',  LEVEL_3, LEVEL_4),   // Set columns per page
        VT_CSI(DECSNLS,         '|', 0x00, '*',  LEVEL_4, LEVEL_4),   // Set number of lines per screen
        VT_CSI(DECIC,           '}', 0x00, '\'', LEVEL_4, LEVEL_4),   // Insert column
        VT_CSI(DECDC,           '~', 0x00, '\'', LEVEL_4, LEVEL_4),   // Delete column
        // Device control strings
        VT_DCS(DECSIXEL,        'q', 0x00, 0x00, LEVEL_3, LEVEL_4),   // Parse sixel graphics
        VT_DCS(DECRQSS,         'q', 0x00, '$',  LEVEL_2, LEVEL_4),   // Request control function strings
        VT_DCS(DECRSPS,         't', 0x00, '$',  LEVEL_3, LEVEL_4),   // Restore presentation state
        VT_DCS(DECUDK,          '|', 0x00, 0x00, LEVEL_2, LEVEL_4)    // Set user-defined keys
    };
    
    LTIMING("TerminalCtrl::FındSequenceId");
	
    const auto *p = vtsequences.FindPtr(
	    MakeVTSequenceKey(
	        seq.type,
	        seq.opcode,
	        seq.mode,
	        seq.intermediate[0]
	    )
	);
	
	return p && LOBYTE(p->b) <= level && level <= HIBYTE(p->b) ? p->a : SequenceId::UNHANDLED;
}

#define VT_MODE(id, mode, type, minlevel, maxlevel)            \
{                                                              \
    { MAKELONG(mode, type) },                                  \
    { id, MAKEWORD(TerminalCtrl::minlevel, TerminalCtrl::maxlevel) }   \
}
    
int TerminalCtrl::FindModeId(word modenum, byte modetype, byte level)
{
    const static VectorMap<dword, Tuple<word, word> > vtmodes = {
        // ANSI modes
        VT_MODE(GATM,       1,      0x00,   LEVEL_1, LEVEL_4),    // Permanently reset
        VT_MODE(KAM,        2,      0x00,   LEVEL_1, LEVEL_4),    // Keyboard action mode
        VT_MODE(CRM,        3,      0x00,   LEVEL_1, LEVEL_4),    // Show/hide control characters
        VT_MODE(IRM,        4,      0x00,   LEVEL_1, LEVEL_4),    // Insert/replace characters
        VT_MODE(SRTM,       5,      0x00,   LEVEL_1, LEVEL_4),    // Permanently reset
        VT_MODE(ERM,        6,      0x00,   LEVEL_1, LEVEL_4),    // Erasure mode
        VT_MODE(VEM,        7,      0x00,   LEVEL_1, LEVEL_4),    // Permanently reset
        VT_MODE(HEM,        10,     0x00,   LEVEL_1, LEVEL_4),    // Permanently reset
        VT_MODE(PUM,        11,     0x00,   LEVEL_1, LEVEL_4),    // Permanently reset
        VT_MODE(SRM,        12,     0x00,   LEVEL_1, LEVEL_4),    // Send/receive mode (local echo)
        VT_MODE(FEAM,       13,     0x00,   LEVEL_1, LEVEL_4),    // Permanently reset
        VT_MODE(FETM,       14,     0x00,   LEVEL_1, LEVEL_4),    // Permanently reset
        VT_MODE(MATM,       15,     0x00,   LEVEL_1, LEVEL_4),    // Permanently reset
        VT_MODE(TTM,        16,     0x00,   LEVEL_1, LEVEL_4),    // Permanently reset
        VT_MODE(SATM,       17,     0x00,   LEVEL_1, LEVEL_4),    // Permanently reset
        VT_MODE(TSM,        18,     0x00,   LEVEL_1, LEVEL_4),    // Permanently reset
        VT_MODE(EBM,        19,     0x00,   LEVEL_1, LEVEL_4),    // Permanently reset
        VT_MODE(LNM,        20,     0x00,   LEVEL_1, LEVEL_4),    // Line feed/new line mode
        // Private modes
        VT_MODE(DECCKM,     1,      '?',    LEVEL_1, LEVEL_4),    // Cursor keys mode
        VT_MODE(DECANM,     2,      '?',    LEVEL_1, LEVEL_4),    // Leave ANSI mode (enter VT52 mode)
        VT_MODE(DECCOLM,    3,      '?',    LEVEL_1, LEVEL_4),    // 80/132 columns mode
        VT_MODE(DECSCLM,    4,      '?',    LEVEL_1, LEVEL_4),    // Scrolling mode
        VT_MODE(DECSCNM,    5,      '?',    LEVEL_1, LEVEL_4),    // Normal/inverse video mode
        VT_MODE(DECOM,      6,      '?',    LEVEL_1, LEVEL_4),    // Origin mode
        VT_MODE(DECAWM,     7,      '?',    LEVEL_1, LEVEL_4),    // Autowrap mode
        VT_MODE(DECARM,     8,      '?',    LEVEL_1, LEVEL_4),    // Autorepeat mode
        VT_MODE(DECTCEM,    25,     '?',    LEVEL_2, LEVEL_4),    // Show/hide caret
        VT_MODE(DECBKM,     67,     '?',    LEVEL_3, LEVEL_4),    // Send backspace when backarrow key is pressed
        VT_MODE(DECLRMM,    69,     '?',    LEVEL_4, LEVEL_4),    // Enable/disable horizontal margins
        VT_MODE(DECSDM,     80,     '?',    LEVEL_3, LEVEL_4),    // Enable/disable sixel scrolling
        // Private mode extensions
        VT_MODE(XTX10MM,    9,      '?',    LEVEL_1, LEVEL_4),    // X10 mouse button tracking mode (compat.)
        VT_MODE(XTREWRAPM,  45,     '?',    LEVEL_1, LEVEL_4),    // Reverse wrap mode
        VT_MODE(XTASBM,     47,     '?',    LEVEL_1, LEVEL_4),    // Alternate screen buffer mode (ver. 1)
        VT_MODE(XTX11MM,    1000,   '?',    LEVEL_1, LEVEL_4),    // X11 mouse button tracking mode (normal)
        VT_MODE(XTDRAGM,    1002,   '?',    LEVEL_1, LEVEL_4),    // X11 mouse motion tracking mode
        VT_MODE(XTANYMM,    1003,   '?',    LEVEL_1, LEVEL_4),    // X11 mouse motion tracking mode (any motion event)
        VT_MODE(XTFOCUSM,   1004,   '?',    LEVEL_1, LEVEL_4),    // Focus in/out mode
        VT_MODE(XTUTF8MM,   1005,   '?',    LEVEL_1, LEVEL_4),    // Enable/disable UTF8 mouse tracking coordinates
        VT_MODE(XTSGRMM,    1006,   '?',    LEVEL_1, LEVEL_4),    // Enable/disable SGR mouse tracking coordinates
        VT_MODE(XTASCM,     1007,   '?',    LEVEL_1, LEVEL_4),    // Alternate scroll mode
        VT_MODE(XTSGRPXMM,  1016,   '?',    LEVEL_1, LEVEL_4),    // Enable/disable SGR pixel-level mouse tracking coordinates
        VT_MODE(XTPCFKEYM,  1035,   '?',    LEVEL_1, LEVEL_4),    // Enable/disable PC-style function keys.
        VT_MODE(XTALTESCM,  1039,   '?',    LEVEL_1, LEVEL_4),    // Prefix the key with ESC when modified with Alt-key
        VT_MODE(XTASBM,     1047,   '?',    LEVEL_1, LEVEL_4),    // Alternate screen buffer mode (ver. 2)
        VT_MODE(XTSRCM,     1048,   '?',    LEVEL_1, LEVEL_4),    // Save/restore cursor
        VT_MODE(XTASBM,     1049,   '?',    LEVEL_1, LEVEL_4),    // Alternate screen buffer mode (ver. 3)
        VT_MODE(XTSPREG,    1070,   '?',    LEVEL_1, LEVEL_4),    // Use private color registers for each sixel (always set)
        VT_MODE(XTBRPM,     2004,   '?',    LEVEL_1, LEVEL_4)     // Bracketed paste mode
    };

    const auto *p = vtmodes.FindPtr(MAKELONG(modenum, modetype));
    return p && LOBYTE(p->b) <= level && level <= HIBYTE(p->b) ? int(p->a) : -1;
}

#undef VT_CTL
#undef VT_SEQUENCE
#undef VT_ESC
#undef VT_CSI
#undef VT_DCS
#undef VT_MODE
}
