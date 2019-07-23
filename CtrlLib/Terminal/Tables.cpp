#include "Console.h"

#define LTIMING(x) // RTIMING(x)

namespace Upp {

#define DOREFRESH   true
#define NOREFRESH   false

#define BEGIN_VT_CTL(table)         \
    static VectorMap<               \
        byte,                       \
        Tuple<                      \
            byte,                   \
            ControlId,              \
            bool                    \
        >                           \
    > table = {
        
#define END_VT_CTL                  \
    }

#define VT_CTL(id, cbyte, flag, level)              \
    {                                               \
        { cbyte },                                  \
        { Console::level, ControlId::id, flag }     \
    }
    
Console::ControlId Console::FindControlId(byte ctl, byte level, bool& refresh)
{
    BEGIN_VT_CTL(vtcbytes)
        VT_CTL(NUL,         0x00,   NOREFRESH, LEVEL_0),
        VT_CTL(ENQ,         0x05,   NOREFRESH, LEVEL_0),
        VT_CTL(BEL,         0x07,   NOREFRESH, LEVEL_0),
        VT_CTL(BS,          0x08,   NOREFRESH, LEVEL_0),
        VT_CTL(HT,          0x09,   NOREFRESH, LEVEL_0),
        VT_CTL(LF,          0x0A,   NOREFRESH, LEVEL_0),
        VT_CTL(VT,          0x0B,   NOREFRESH, LEVEL_0),
        VT_CTL(FF,          0x0C,   NOREFRESH, LEVEL_0),
        VT_CTL(CR,          0x0D,   NOREFRESH, LEVEL_0),
        VT_CTL(LS1,         0x0E,   NOREFRESH, LEVEL_0),
        VT_CTL(LS0,         0x0F,   NOREFRESH, LEVEL_0),
        VT_CTL(XON,         0x11,   NOREFRESH, LEVEL_0),
        VT_CTL(XOFF,        0x13,   NOREFRESH, LEVEL_0),
        VT_CTL(DEL,         0x7F,   NOREFRESH, LEVEL_0),
        VT_CTL(IND,         0x84,   DOREFRESH, LEVEL_0),
        VT_CTL(NEL,         0x85,   DOREFRESH, LEVEL_0),
        VT_CTL(HTS,         0x88,   NOREFRESH, LEVEL_0),
        VT_CTL(RI,          0x8D,   DOREFRESH, LEVEL_0),
        VT_CTL(SS2,         0x8E,   NOREFRESH, LEVEL_2),
        VT_CTL(SS3,         0x8F,   NOREFRESH, LEVEL_2),
        VT_CTL(SPA,         0x96,   NOREFRESH, LEVEL_0),
        VT_CTL(EPA,         0x97,   NOREFRESH, LEVEL_0),
        VT_CTL(DECID,       0x9A,   NOREFRESH, LEVEL_0),
        VT_CTL(ST,          0x9C,   NOREFRESH, LEVEL_0)
    END_VT_CTL;

    const auto *p = vtcbytes.FindPtr(ctl);
    if(!p || p->a > level) {
        refresh = false;
        return ControlId::UNHANDLED;
    }
    refresh = p->c;
    return p->b;
}

#undef BEGIN_VT_CTL
#undef END_VT_CTL
#undef VT_CTL

#define BEGIN_VT_SEQUENCES(table)   \
    static VectorMap<               \
        Tuple<                      \
            byte,                   \
            byte,                   \
            byte,                   \
            byte                    \
        >,                          \
        Tuple<                      \
            byte,                   \
            SequenceId,             \
            bool                    \
        >                           \
    > table = {
        
#define END_VT_SEQUENCES            \
    }

#define VT_SEQUENCE(seq, id, opcode, mode, interm, flag, level) \
    {                                                           \
        { VTInStream::Sequence::seq, opcode, mode, interm},     \
        { Console::level, SequenceId::id, flag }                \
    }
    
#define VT_ESC(id, opcode, mode, interm, flag, level)           \
    VT_SEQUENCE(ESC, id, opcode, mode, interm, flag, level)

#define VT_CSI(id, opcode, mode, interm, flag, level)           \
    VT_SEQUENCE(CSI, id, opcode, mode, interm, flag, level)

#define VT_DCS(id, opcode, mode, interm, flag, level)           \
    VT_SEQUENCE(DCS, id, opcode, mode, interm, flag, level)

Console::SequenceId Console::FindSequenceId(byte type, byte level, const VTInStream::Sequence& seq, bool& refresh)
{
    BEGIN_VT_SEQUENCES(vtsequences)
        // Escape sequences
        VT_ESC(DECBI,           '6', 0x00, 0x00, DOREFRESH, LEVEL_4),   // Back index
        VT_ESC(DECSC,           '7', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Save cursor
        VT_ESC(DECRC,           '8', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Restore cursor
        VT_ESC(DECALN,          '8', 0x00, '#',  DOREFRESH, LEVEL_1),   // Display alignment test
        VT_ESC(DECFI,           '9', 0x00, 0x00, DOREFRESH, LEVEL_4),   // Forward index
        VT_ESC(S7C1T,           'F', 0x00, ' ',  NOREFRESH, LEVEL_2),   // 7-bits mode
        VT_ESC(S8C1T,           'G', 0x00, ' ',  NOREFRESH, LEVEL_2),   // 8-bits mode.
        VT_ESC(ANSICL1,         'L', 0x00, ' ',  NOREFRESH, LEVEL_2),   // Select ANSI conformance level 1
        VT_ESC(ANSICL2,         'M', 0x00, ' ',  NOREFRESH, LEVEL_2),   // Select ANSI conformance level 2
        VT_ESC(ANSICL3,         'N', 0x00, ' ',  NOREFRESH, LEVEL_2),   // Select ANSI conformance level 3
        VT_ESC(DECKPAM,         '=', 0x00, 0x00, NOREFRESH, LEVEL_0),   // Keypad application mode
        VT_ESC(DECKPNM,         '>', 0x00, 0x00, NOREFRESH, LEVEL_0),   // Keypad mumeric mode
        VT_ESC(RIS,             'c', 0x00, 0x00, DOREFRESH, LEVEL_1),   // Hard reset
        VT_ESC(LS2,             'n', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Locking-shift 2, GL
        VT_ESC(LS3,             'o', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Locking-shift 3, GL
        VT_ESC(LS3R,            '|', 0x00, 0x00, NOREFRESH, LEVEL_2),   // Locking-shift 3, GR
        VT_ESC(LS2R,            '}', 0x00, 0x00, NOREFRESH, LEVEL_2),   // Locking-shift 2, GR
        VT_ESC(LS1R,            '~', 0x00, 0x00, NOREFRESH, LEVEL_2),   // Locking-shift 1, GR
        // SCS specific escape sequences
        VT_ESC(SCS_G0_DEC_DCS,  '0', 0x00, '(',  NOREFRESH, LEVEL_1),   // Invoke DEC Line-drawing charset into G0
        VT_ESC(SCS_G1_DEC_DCS,  '0', 0x00, ')',  NOREFRESH, LEVEL_1),   // Invoke DEC Line-drawing charset into G1
        VT_ESC(SCS_G2_DEC_DCS,  '0', 0x00, '*',  NOREFRESH, LEVEL_2),   // Invoke DEC Line-drawing charset into G2
        VT_ESC(SCS_G3_DEC_DCS,  '0', 0x00, '+',  NOREFRESH, LEVEL_2),   // Invoke DEC Line-drawing charset into G3
        VT_ESC(SCS_G0_DEC_ACS,  '1', 0x00, '(',  NOREFRESH, LEVEL_1),   // Invoke DEC standard ROM charset into G0 (stubbed)
        VT_ESC(SCS_G1_DEC_ACS,  '1', 0x00, ')',  NOREFRESH, LEVEL_1),   // Invoke DEC standard ROM charset into G1 (stubbed)
        VT_ESC(SCS_G0_DEC_ACS,  '2', 0x00, '(',  NOREFRESH, LEVEL_1),   // Invoke DEC alternate ROM charset into G0 (stubbed)
        VT_ESC(SCS_G1_DEC_ACS,  '2', 0x00, ')',  NOREFRESH, LEVEL_1),   // Invoke DEC alternate ROM charset into G1 (stubbed)
        VT_ESC(SCS_DEFAULT,     '@', 0x00, '%',  NOREFRESH, LEVEL_3),   // Select default charset
        VT_ESC(SCS_G0_ASCII,    'A', 0x00, '(',  NOREFRESH, LEVEL_1),   // Invoke EN-GB charset into G0 (stubbed)
        VT_ESC(SCS_G1_ASCII,    'A', 0x00, ')',  NOREFRESH, LEVEL_1),   // Invoke EN-GB charset into G1 (stubbed)
        VT_ESC(SCS_G2_ASCII,    'A', 0x00, '*',  NOREFRESH, LEVEL_2),   // Invoke EN-GB charset into G2 (stubbed)
        VT_ESC(SCS_G3_ASCII,    'A', 0x00, '+',  NOREFRESH, LEVEL_2),   // Invoke EN-GB charset into G3 (stubbed)
        VT_ESC(SCS_G1_LATIN1,   'A', 0x00, '-',  NOREFRESH, LEVEL_3),   // Invoke DEC/ISO Latin-1 charset into G1
        VT_ESC(SCS_G2_LATIN1,   'A', 0x00, '.',  NOREFRESH, LEVEL_3),   // Invoke DEC/ISO Latin-1 charset into G2
        VT_ESC(SCS_G3_LATIN1,   'A', 0x00, '/',  NOREFRESH, LEVEL_3),   // Invoke DEC/ISO Latin-1 charset into G3
        VT_ESC(SCS_G0_ASCII,    'B', 0x00, '(',  NOREFRESH, LEVEL_1),   // Invoke US-ASCII charset into G0
        VT_ESC(SCS_G1_ASCII,    'B', 0x00, ')',  NOREFRESH, LEVEL_1),   // Invoke US-ASCII charset into G1
        VT_ESC(SCS_G2_ASCII,    'B', 0x00, '*',  NOREFRESH, LEVEL_2),   // Invoke US-ASCII charset into G2
        VT_ESC(SCS_G3_ASCII,    'B', 0x00, '+',  NOREFRESH, LEVEL_2),   // Invoke US-ASCII charset into G3
        VT_ESC(SCS_UTF8,        'G', 0x00, '%',  NOREFRESH, LEVEL_3),   // Select UTF-8 charset
        VT_ESC(SCS_G0_DEC_MCS,  '<', 0x00, '(',  NOREFRESH, LEVEL_2),   // Invoke DEC supplemental charset into G0
        VT_ESC(SCS_G1_DEC_MCS,  '<', 0x00, ')',  NOREFRESH, LEVEL_2),   // Invoke DEC supplemental charset into G1
        VT_ESC(SCS_G2_DEC_MCS,  '<', 0x00, '*',  NOREFRESH, LEVEL_2),   // Invoke DEC supplemental charset into G2
        VT_ESC(SCS_G3_DEC_MCS,  '<', 0x00, '+',  NOREFRESH, LEVEL_2),   // Invoke DEC supplemental charset into G3
        VT_ESC(SCS_G0_DEC_TCS,  '>', 0x00, '(',  NOREFRESH, LEVEL_3),   // Invoke DEC technical charset into G0
        VT_ESC(SCS_G1_DEC_TCS,  '>', 0x00, ')',  NOREFRESH, LEVEL_3),   // Invoke DEC technical charset into G1
        VT_ESC(SCS_G2_DEC_TCS,  '>', 0x00, '*',  NOREFRESH, LEVEL_3),   // Invoke DEC technical charset into G2
        VT_ESC(SCS_G3_DEC_TCS,  '>', 0x00, '+',  NOREFRESH, LEVEL_3),   // Invoke DEC technical charset into G3
        // VT52 specific escape sequences
        VT_ESC(VT52_CUU,        'A', 0x00, 0x00, NOREFRESH, LEVEL_0),   // Move upward
        VT_ESC(VT52_CUD,        'B', 0x00, 0x00, NOREFRESH, LEVEL_0),   // Move downward
        VT_ESC(VT52_CUF,        'C', 0x00, 0x00, NOREFRESH, LEVEL_0),   // Move forward
        VT_ESC(VT52_CUB,        'D', 0x00, 0x00, NOREFRESH, LEVEL_0),   // Move backward
        VT_ESC(VT52_DCS_ON,     'F', 0x00, 0x00, NOREFRESH, LEVEL_0),   // Drawing characters: on
        VT_ESC(VT52_DCS_OFF,    'G', 0x00, 0x00, NOREFRESH, LEVEL_0),   // Drawing characters: off
        VT_ESC(VT52_HOME,       'H', 0x00, 0x00, NOREFRESH, LEVEL_0),   // Move home
        VT_ESC(VT52_RI,         'I', 0x00, 0x00, DOREFRESH, LEVEL_0),   // Reverse index
        VT_ESC(VT52_ED,         'J', 0x00, 0x00, DOREFRESH, LEVEL_0),   // Erase to end of screen
        VT_ESC(VT52_EL,         'K', 0x00, 0x00, DOREFRESH, LEVEL_0),   // Erase to end of line
        VT_ESC(VT52_CUP,        'Y', 0x00, 0x00, NOREFRESH, LEVEL_0),   // Direct cursor addressing
        VT_ESC(VT52_DA,         'Z', 0x00, 0x00, NOREFRESH, LEVEL_0),   // Device identify
        VT_ESC(VT52_DECANM,     '<', 0x00, 0x00, NOREFRESH, LEVEL_0),   // Enter ANSI mode (leave VT52 mode)
        // Command sequences
        VT_CSI(ICH,             '@', 0x00, 0x00, DOREFRESH, LEVEL_2),   // Insert character
        VT_CSI(SL,              '@', 0x00, ' ',  DOREFRESH, LEVEL_3),   // Scroll/shift left
        VT_CSI(CUU,             'A', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Move upward
        VT_CSI(SR,              'A', 0x00, ' ',  DOREFRESH, LEVEL_3),   // Scroll/shift right
        VT_CSI(CUD,             'B', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Move downward
        VT_CSI(CUF,             'C', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Move forward
        VT_CSI(CUB,             'D', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Move backward
        VT_CSI(CNL,             'E', 0x00, 0x00, NOREFRESH, LEVEL_3),   // Move to next line (no scrolling)
        VT_CSI(CPL,             'F', 0x00, 0x00, NOREFRESH, LEVEL_3),   // Move to prev line )no scrolling)
        VT_CSI(CHA,             'G', 0x00, 0x00, NOREFRESH, LEVEL_3),   // Cursor horizontal absolute
        VT_CSI(CUP,             'H', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Cursor position
        VT_CSI(CHT,             'I', 0x00, 0x00, NOREFRESH, LEVEL_3),   // Cursor horizontal tabulation
        VT_CSI(ED,              'J', 0x00, 0x00, DOREFRESH, LEVEL_1),   // Erase screen
        VT_CSI(DECSED,          'J', '?',  0x00, DOREFRESH, LEVEL_2),   // Selectively erase screen
        VT_CSI(EL,              'K', 0x00, 0x00, DOREFRESH, LEVEL_1),   // Erase line
        VT_CSI(DECSEL,          'K', '?',  0x00, DOREFRESH, LEVEL_2),   // Selectively erase line
        VT_CSI(IL,              'L', 0x00, 0x00, DOREFRESH, LEVEL_1),   // Insert line
        VT_CSI(DL,              'M', 0x00, 0x00, DOREFRESH, LEVEL_1),   // Remove line
        VT_CSI(DCH,             'P', 0x00, 0x00, DOREFRESH, LEVEL_1),   // Delete character
        VT_CSI(SU,              'S', 0x00, 0x00, DOREFRESH, LEVEL_4),   // Scroll up
        VT_CSI(SD,              'T', 0x00, 0x00, DOREFRESH, LEVEL_4),   // Scroll down
        VT_CSI(ECH,             'X', 0x00, 0x00, DOREFRESH, LEVEL_2),   // Erase character
        VT_CSI(CBT,             'Z', 0x00, 0x00, NOREFRESH, LEVEL_3),   // Cursor backward tabulation
        VT_CSI(ECH,             '^', 0x00, 0x00, NOREFRESH, LEVEL_3),   // FIXME
        VT_CSI(HPA,             '`', 0x00, 0x00, NOREFRESH, LEVEL_3),   // Horizontal position absolute
        VT_CSI(HPR,             'a', 0x00, 0x00, NOREFRESH, LEVEL_3),   // Horizontal position relative
        VT_CSI(REP,             'b', 0x00, 0x00, DOREFRESH, LEVEL_3),   // Repeat last character
        VT_CSI(DA1,             'c', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Send primary device attributes
        VT_CSI(DA2,             'c', '>',  0x00, NOREFRESH, LEVEL_2),   // Send secondary device attributes
        VT_CSI(DA3,             'c', '=',  0x00, NOREFRESH, LEVEL_4),   // Send tertiary device attributes
        VT_CSI(VPA,             'd', 0x00, 0x00, NOREFRESH, LEVEL_3),   // Vertical position absolute
        VT_CSI(VPR,             'e', 0x00, 0x00, NOREFRESH, LEVEL_3),   // Vertical position relative
        VT_CSI(HVP,             'f', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Cursor horz and vert position
        VT_CSI(TBC,             'g', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Clear tabs
        VT_CSI(SM,              'h', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Set ANSI modes
        VT_CSI(SM,              'h', '?',  0x00, NOREFRESH, LEVEL_1),   // Set private modes
        VT_CSI(RM,              'l', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Reset ANSI modes
        VT_CSI(RM,              'l', '?',  0x00, NOREFRESH, LEVEL_1),   // Reset private modes
        VT_CSI(SGR,             'm', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Select graphics rendition (ANSI)
        VT_CSI(SGR,             'm', '?',  0x00, NOREFRESH, LEVEL_1),   // Select graphics rendition (DEC)
        VT_CSI(DSR,             'n', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Send device status report (ANSI format)
        VT_CSI(DECXCPR,         'n', '?',  0x00, NOREFRESH, LEVEL_1),   // Send device status (cursor position) report (DEC format)
        VT_CSI(DECSCL,          'p', 0x00, '\"', NOREFRESH, LEVEL_1),   // Select device conformance level
        VT_CSI(DECSTR,          'p', 0x00, '!',  NOREFRESH, LEVEL_2),   // Soft reset
        VT_CSI(DECRQM,          'p', 0x00, '$',  NOREFRESH, LEVEL_2),   // Request ANSI mode
        VT_CSI(DECRQM,          'p', '?',  '$',  NOREFRESH, LEVEL_2),   // Request private mode
        VT_CSI(DECLL,           'q', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Set programmable LEDs
        VT_CSI(DECSCA,          'q', 0x00, '\"', NOREFRESH, LEVEL_2),   // Set character protection attribute
        VT_CSI(DECSCUSR,        'q', 0x00, ' ',  DOREFRESH, LEVEL_3),   // Set cursor style
        VT_CSI(DECSTBM,         'r', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Set vertical margins
        VT_CSI(DECCARA,         'r', 0x00, '$',  DOREFRESH, LEVEL_4),   // Change attributes in rectangular area
        VT_CSI(DECSLRM,         's', 0x00, 0x00, NOREFRESH, LEVEL_3),   // Set horizontal margins / SCO save cursor
        VT_CSI(DECRARA,         't', 0x00, '$',  DOREFRESH, LEVEL_4),   // Reverse attributes in rectangular area
        VT_CSI(SCORC,           'u', 0x00, 0x00, NOREFRESH, LEVEL_3),   // SCO restore cursor
        VT_CSI(DECCRA,          'v', 0x00, '$',  DOREFRESH, LEVEL_4),   // Copy rectangular area.
        VT_CSI(DECREQTPARM,     'x', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Request terminal parameters
        VT_CSI(DECFRA,          'x', 0x00, '$',  DOREFRESH, LEVEL_4),   // Fill rectangular area
        VT_CSI(DECSACE,         'x', 0x00, '*',  NOREFRESH, LEVEL_4),   // Select rectangular area attribute change extent.
        VT_CSI(DECTST,          'y', 0x00, 0x00, NOREFRESH, LEVEL_1),   // Device confidence tests
        VT_CSI(DECRQCRA,        'y', 0x00, '*',  NOREFRESH, LEVEL_4),   // Request rectangular area checksum
        VT_CSI(DECERA,          'z', 0x00, '$',  DOREFRESH, LEVEL_4),   // Erase rectangular area
        VT_CSI(DECSERA,         '{', 0x00, '$',  DOREFRESH, LEVEL_4),   // Selectively erase rectangular area
        VT_CSI(DECIC,           '}', 0x00, '\'', DOREFRESH, LEVEL_4),   // Insert column
        VT_CSI(DECDC,           '~', 0x00, '\'', DOREFRESH, LEVEL_4),   // Delete column
        // Device control strings
        VT_DCS(DECRQSS,         'q', 0x00, '$',  NOREFRESH, LEVEL_4),   // Request control function strings
        VT_DCS(DECUDK,          '|', 0x00, 0x00, NOREFRESH, LEVEL_2)    // Set user-defined keys
    END_VT_SEQUENCES;
    
    LTIMING("Console::FındSequenceId");

    const auto *p = vtsequences.FindPtr(
            MakeTuple(type, seq.opcode, seq.mode, (byte) seq.intermediate[0])
        );
    if(!p || p->a > level) {
        refresh = false;
        return SequenceId::UNHANDLED;
    }
    refresh = p->c;
    return p->b;
}

#undef BEGIN_VT_SEQUENCES
#undef END_VT_SEQUENCES
#undef VT_SEQUENCE
#undef VT_ESC
#undef VT_CSI
#undef VT_DCS

#define BEGIN_VT_MODES(table)       \
    static VectorMap<               \
        Tuple<                      \
            word,                   \
            byte                    \
        >,                          \
        Tuple<                      \
            byte,                   \
            byte,                   \
            bool                    \
        >                           \
    > table = {
        
#define END_VT_MODES                }

#define VT_MODE(id, mode, type, flag, level)    \
    {                                           \
        { mode, type},                          \
        { Console::level, id, flag }            \
    }
    
int Console::FindModeId(word modenum, byte modetype, byte level, bool& refresh)
{
    BEGIN_VT_MODES(vtmodes)
        // ANSI modes
        VT_MODE(KAM,        2,      0x00,   NOREFRESH, LEVEL_1),    // Keyboard action mode
        VT_MODE(CRM,        3,      0x00,   NOREFRESH, LEVEL_1),    // Show/hide control characters
        VT_MODE(IRM,        4,      0x00,   NOREFRESH, LEVEL_1),    // Insert/replace characters
        VT_MODE(ERM,        6,      0x00,   NOREFRESH, LEVEL_1),    // Erasure mode
        VT_MODE(SRM,        12,     0x00,   NOREFRESH, LEVEL_1),    // Send/receive mode (local echo)
        VT_MODE(LNM,        20,     0x00,   NOREFRESH, LEVEL_1),    // Line feed/new line mode
        // Private modes
        VT_MODE(DECCKM,     1,      '?',    NOREFRESH, LEVEL_1),    // Cursor keys mode
        VT_MODE(DECANM,     2,      '?',    NOREFRESH, LEVEL_1),    // Leave ANSI mode (enter VT52 mode)
        VT_MODE(DECCOLM,    3,      '?',    DOREFRESH, LEVEL_1),    // 80/132 columns mode
        VT_MODE(DECSCLM,    4,      '?',    NOREFRESH, LEVEL_1),    // Scrolling mode
        VT_MODE(DECSCNM,    5,      '?',    NOREFRESH, LEVEL_1),    // Normal/inverse video mode
        VT_MODE(DECOM,      6,      '?',    DOREFRESH, LEVEL_1),    // Origin mode
        VT_MODE(DECAWM,     7,      '?',    NOREFRESH, LEVEL_1),    // Autowrap mode
        VT_MODE(DECARM,     8,      '?',    NOREFRESH, LEVEL_1),    // Autorepeat mode
        VT_MODE(DECTCEM,    25,     '?',    DOREFRESH, LEVEL_2),    // Show/hide caret
        VT_MODE(DECBKM,     67,     '?',    NOREFRESH, LEVEL_3),    // Send backspace when backarrow key is pressed.
        VT_MODE(DECLRMM,    69,     '?',    NOREFRESH, LEVEL_4),    // Enable/disable horizontal margins
        // Private mode extensions
        VT_MODE(XTX10MM,    9,      '?',    NOREFRESH, LEVEL_1),    // X10 mouse button tracking mode (compat.)
        VT_MODE(XTASBM,     47,     '?',    DOREFRESH, LEVEL_1),    // Alternate screen buffer mode (ver. 1)
        VT_MODE(XTX11MM,    1000,   '?',    NOREFRESH, LEVEL_1),    // X11 mouse button tracking mode (normal)
        VT_MODE(XTDRAGM,    1002,   '?',    NOREFRESH, LEVEL_1),    // X11 mouse motion tracking mode
        VT_MODE(XTANYMM,    1003,   '?',    NOREFRESH, LEVEL_1),    // X11 mouse motion tracking mode (any motion event)
        VT_MODE(XTFOCUSM,   1004,   '?',    NOREFRESH, LEVEL_1),    // Focus in/out mode
        VT_MODE(XTUTF8MM,   1005,   '?',    NOREFRESH, LEVEL_1),    // Enable/disable UTF8 mouse tracking coordinates
        VT_MODE(XTSGRMM,    1006,   '?',    NOREFRESH, LEVEL_1),    // Enable/disable SGR mouse tracking coordinates
        VT_MODE(XTASCM,     1007,   '?',    NOREFRESH, LEVEL_1),    // Alternate scroll mode
        VT_MODE(XTASBM,     1047,   '?',    DOREFRESH, LEVEL_1),    // Alternate screen buffer mode (ver. 2)
        VT_MODE(XTSRCM,     1048,   '?',    NOREFRESH, LEVEL_1),    // Save/restore cursor
        VT_MODE(XTASBM,     1049,   '?',    DOREFRESH, LEVEL_1),    // Alternate screen buffer mode (ver. 3)
        VT_MODE(XTBRPM,     2004,   '?',    NOREFRESH, LEVEL_1)     // Bracketed paste mode
    END_VT_MODES;
        
    const auto *p = vtmodes.FindPtr(MakeTuple(modenum, modetype));
    if(!p || p->a > level) {
        refresh = false;
        return -1;
    }
    refresh = p->c;
    return p->b;
}

#undef BEGIN_VT_MODES
#undef END_VT_MODES
#undef VT_MODE

#undef DOREFRESH
#undef NOREFRESH
}
