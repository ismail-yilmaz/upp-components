#include "Terminal.h"

#define LLOG(x)     // RLOG("TerminalCtrl (#" << this << "]: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

void TerminalCtrl::DispatchCtl(byte ctl)
{
    #define VT_CTL(cbyte, minlevel, maxlevel, fn)                           \
    {                                                                       \
        { cbyte },                                                          \
        { TerminalCtrl::minlevel, TerminalCtrl::maxlevel, [](TerminalCtrl& t, byte c) fn } \
    }

    LLOG(Format("CTL 0x%02X (C%[1:0;1]s`)", ctl, ctl < 0x80));

    if(modes[CRM]) {
        LLOG("Printing the control character. (CRM enabled)");
        PutChar(ctl);
        return;
    }

    static VectorMap<dword, CbControl> vtcbytes;

    ONCELOCK {
    vtcbytes = {
        VT_CTL(0x00,   LEVEL_0, LEVEL_4, { /* NOP */                                              }),   // NUL:   Ignored
        VT_CTL(0x05,   LEVEL_0, LEVEL_4, { t.Put(t.answerback.ToWString());                       }),   // ENQ:   Terminal status request
        VT_CTL(0x07,   LEVEL_0, LEVEL_4, { t.WhenBell();                                          }),   // BEL:   Audio or visual bell
        VT_CTL(0x08,   LEVEL_0, LEVEL_4, { t.page->MoveLeft();                                    }),   // BS:    Backspace
        VT_CTL(0x09,   LEVEL_0, LEVEL_4, { t.page->NextTab();                                     }),   // HT:    Horizontal tab. Move the cursor to next tab stop
        VT_CTL(0x0A,   LEVEL_0, LEVEL_4, { t.modes[LNM] ? t.page->NewLine() : t.page->NextLine(); }),   // LF:    Line feed
        VT_CTL(0x0B,   LEVEL_0, LEVEL_4, { t.modes[LNM] ? t.page->NewLine() : t.page->NextLine(); }),   // VT:    Vertical tab (treated as LF)
        VT_CTL(0x0C,   LEVEL_0, LEVEL_4, { t.modes[LNM] ? t.page->NewLine() : t.page->NextLine(); }),   // FF:    Form feed (treated as LF)
        VT_CTL(0x0D,   LEVEL_0, LEVEL_4, { t.modes[LNM] ? t.page->NewLine() : t.page->MoveHome(); }),   // CR:    Carriage return
        VT_CTL(0x0E,   LEVEL_1, LEVEL_4, { t.gsets.G1toGL();                                      }),   // LS1:   Invoke and locks G1 into GL
        VT_CTL(0x0F,   LEVEL_1, LEVEL_4, { t.gsets.G0toGL();                                      }),   // LS0:   Invoke and locks G0 into GL
        VT_CTL(0x11,   LEVEL_0, LEVEL_4, { /* NOP */                                              }),   // XON:   Resume data transfer
        VT_CTL(0x13,   LEVEL_0, LEVEL_4, { /* NOP */                                              }),   // XOFF:  Pause data transfer
        VT_CTL(0x7F,   LEVEL_0, LEVEL_4, { /* NOP */                                              }),   // DEL:   Ignored
        VT_CTL(0x84,   LEVEL_1, LEVEL_4, { t.page->NextLine();                                    }),   // IND:   Index
        VT_CTL(0x85,   LEVEL_1, LEVEL_4, { t.page->NewLine();                                     }),   // NEL:   Move to next line
        VT_CTL(0x88,   LEVEL_1, LEVEL_4, { t.page->SetTab();                                      }),   // HTS:   Sets a tab at the active cursor position
        VT_CTL(0x8D,   LEVEL_1, LEVEL_4, { t.page->PrevLine();                                    }),   // RI:    Reverse index
        VT_CTL(0x8E,   LEVEL_1, LEVEL_4, { t.gsets.SS(c);                                         }),   // SS2:   Temporarily invoke G2 to GL
        VT_CTL(0x8F,   LEVEL_1, LEVEL_4, { t.gsets.SS(c);                                         }),   // SS3:   Temporarily invoke G3 to GL
        VT_CTL(0x96,   LEVEL_2, LEVEL_4, { t.SetISOStyleCellProtection(true);                     }),   // SPA:   Start of protected area
        VT_CTL(0x97,   LEVEL_2, LEVEL_4, { t.SetISOStyleCellProtection(false);                    }),   // EPA:   End of protected area
        VT_CTL(0x9A,   LEVEL_1, LEVEL_4, { t.ReportDeviceAttributes(VTInStream::Sequence());      }),   // DECID: Report terminal ID
        VT_CTL(0x9C,   LEVEL_1, LEVEL_4, { /* NOP */                                              })    // ST:    String terminator
    };
    }

    const CbControl *p = vtcbytes.FindPtr(ctl);
    if(p && clevel >= p->a && clevel <= p->b)
        p->c(*this, ctl);
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

const TerminalCtrl::CbFunction* TerminalCtrl::FindFunctionPtr(const VTInStream::Sequence& seq)
{
    #define VT_SEQUENCE(seq, opcode, mode, interm, minlevel, maxlevel, fn)       \
    {                                                                            \
        { MakeVTSequenceKey(VTInStream::Sequence::seq, opcode, mode, interm) },  \
        { TerminalCtrl::minlevel, TerminalCtrl::maxlevel, [](TerminalCtrl& t, const VTInStream::Sequence& q) fn }   \
    }
    
    #define VT_ESC(opcode, mode, interm, minlevel, maxlevel, fn)  VT_SEQUENCE(ESC, opcode, mode, interm, minlevel, maxlevel, fn)
    #define VT_CSI(opcode, mode, interm, minlevel, maxlevel, fn)  VT_SEQUENCE(CSI, opcode, mode, interm, minlevel, maxlevel, fn)
    #define VT_DCS(opcode, mode, interm, minlevel, maxlevel, fn)  VT_SEQUENCE(DCS, opcode, mode, interm, minlevel, maxlevel, fn)

    static VectorMap<dword, CbFunction> vtsequences;
   
    ONCELOCK {
    vtsequences = {
        // Escape sequences
        VT_ESC('6', 0x00, 0x00, LEVEL_4, LEVEL_4,  { t.page->PrevColumn();                                        }),   // DECBI:   Back index
        VT_ESC('7', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.Backup();                                                  }),   // DECSC:   Save cursor
        VT_ESC('8', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.Restore();                                                 }),   // DECRC:   Restore cursor
        VT_ESC('8', 0x00, '#',  LEVEL_1, LEVEL_4,  { t.DisplayAlignmentTest();                                    }),   // DECALN:  Display alignment test
        VT_ESC('9', 0x00, 0x00, LEVEL_4, LEVEL_4,  { t.page->NextColumn();                                        }),   // DECFI:   Forward index
        VT_ESC('F', 0x00, ' ',  LEVEL_2, LEVEL_4,  { t.Set8BitMode(false);                                        }),   // S7C1T:   7-bits mode
        VT_ESC('G', 0x00, ' ',  LEVEL_2, LEVEL_4,  { t.Set8BitMode(true);                                         }),   // S8C1T:   8-bits mode.
        VT_ESC('L', 0x00, ' ',  LEVEL_2, LEVEL_4,  { t.gsets.ConformtoANSILevel1();                               }),   // ANSICL1: Select ANSI conformance level 1
        VT_ESC('M', 0x00, ' ',  LEVEL_2, LEVEL_4,  { t.gsets.ConformtoANSILevel3();                               }),   // ANSICL2: Select ANSI conformance level 2
        VT_ESC('N', 0x00, ' ',  LEVEL_2, LEVEL_4,  { t.gsets.ConformtoANSILevel1();                               }),   // ANSICL3: Select ANSI conformance level 3
        VT_ESC('=', 0x00, 0x00, LEVEL_0, LEVEL_4,  { t.DECkpam(true);                                             }),   // DECKPAM: Keypad application mode
        VT_ESC('>', 0x00, 0x00, LEVEL_0, LEVEL_4,  { t.DECkpam(false);                                            }),   // DECKPNM: Keypad mumeric mode
        VT_ESC('c', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.HardReset();                                               }),   // RIS:     Hard reset
        VT_ESC('n', 0x00, 0x00, LEVEL_2, LEVEL_4,  { t.gsets.G2toGL();                                            }),   // LS2:     Invoke G2 into GL
        VT_ESC('o', 0x00, 0x00, LEVEL_2, LEVEL_4,  { t.gsets.G3toGL();                                            }),   // LS3:     Invoke G3 into GL
        VT_ESC('|', 0x00, 0x00, LEVEL_2, LEVEL_4,  { t.gsets.G3toGR();                                            }),   // LS3R:    Invoke G3 into GR
        VT_ESC('}', 0x00, 0x00, LEVEL_2, LEVEL_4,  { t.gsets.G2toGR();                                            }),   // LS2R:    Invoke G2 into GR
        VT_ESC('~', 0x00, 0x00, LEVEL_2, LEVEL_4,  { t.gsets.G1toGR();                                            }),   // LS1R:    Invoke G1 into GR
        // SCS specific escape sequences
        VT_ESC('0', 0x00, '(',  LEVEL_1, LEVEL_4,  { t.gsets.G0(CHARSET_DEC_DCS);                                 }),   // SCS_G0_DEC_DCS: Invoke DEC Line-drawing charset into G0
        VT_ESC('0', 0x00, ')',  LEVEL_1, LEVEL_4,  { t.gsets.G1(CHARSET_DEC_DCS);                                 }),   // SCS_G1_DEC_DCS: Invoke DEC Line-drawing charset into G1
        VT_ESC('0', 0x00, '*',  LEVEL_2, LEVEL_4,  { t.gsets.G2(CHARSET_DEC_DCS);                                 }),   // SCS_G2_DEC_DCS: Invoke DEC Line-drawing charset into G2
        VT_ESC('0', 0x00, '+',  LEVEL_2, LEVEL_4,  { t.gsets.G3(CHARSET_DEC_DCS);                                 }),   // SCS_G3_DEC_DCS: Invoke DEC Line-drawing charset into G3
        VT_ESC('1', 0x00, '(',  LEVEL_1, LEVEL_4,  { t.gsets.G0(CHARSET_TOASCII);                                 }),   // SCS_G0_DEC_ACS: Invoke DEC standard ROM charset into G0 (stubbed)
        VT_ESC('1', 0x00, ')',  LEVEL_1, LEVEL_4,  { t.gsets.G1(CHARSET_TOASCII);                                 }),   // SCS_G1_DEC_ACS: Invoke DEC standard ROM charset into G1 (stubbed)
        VT_ESC('2', 0x00, '(',  LEVEL_1, LEVEL_4,  { t.gsets.G0(CHARSET_TOASCII);                                 }),   // SCS_G0_DEC_ACS: Invoke DEC alternate ROM charset into G0 (stubbed)
        VT_ESC('2', 0x00, ')',  LEVEL_1, LEVEL_4,  { t.gsets.G1(CHARSET_TOASCII);                                 }),   // SCS_G1_DEC_ACS: Invoke DEC alternate ROM charset into G1 (stubbed)
        VT_ESC('@', 0x00, '%',  LEVEL_3, LEVEL_4,  { t.gsets.Reset();                                             }),   // SCS_DEFAULT:    Select default charset
        VT_ESC('A', 0x00, '(',  LEVEL_1, LEVEL_4,  { t.gsets.G0(CHARSET_TOASCII);                                 }),   // SCS_G0_ASCII:   Invoke EN-GB charset into G0 (stubbed)
        VT_ESC('A', 0x00, ')',  LEVEL_1, LEVEL_4,  { t.gsets.G1(CHARSET_TOASCII);                                 }),   // SCS_G1_ASCII:   Invoke EN-GB charset into G1 (stubbed)
        VT_ESC('A', 0x00, '*',  LEVEL_2, LEVEL_4,  { t.gsets.G2(CHARSET_TOASCII);                                 }),   // SCS_G2_ASCII:   Invoke EN-GB charset into G2 (stubbed)
        VT_ESC('A', 0x00, '+',  LEVEL_2, LEVEL_4,  { t.gsets.G3(CHARSET_TOASCII);                                 }),   // SCS_G3_ASCII:   Invoke EN-GB charset into G3 (stubbed)
        VT_ESC('A', 0x00, '-',  LEVEL_3, LEVEL_4,  { t.gsets.G1(CHARSET_ISO8859_1);                               }),   // SCS_G1_LATIN1:  Invoke DEC/ISO Latin-1 charset into G1
        VT_ESC('A', 0x00, '.',  LEVEL_3, LEVEL_4,  { t.gsets.G2(CHARSET_ISO8859_1);                               }),   // SCS_G2_LATIN1:  Invoke DEC/ISO Latin-1 charset into G2
        VT_ESC('A', 0x00, '/',  LEVEL_3, LEVEL_4,  { t.gsets.G3(CHARSET_ISO8859_1);                               }),   // SCS_G3_LATIN1:  Invoke DEC/ISO Latin-1 charset into G3
        VT_ESC('B', 0x00, '(',  LEVEL_1, LEVEL_4,  { t.gsets.G0(CHARSET_TOASCII);                                 }),   // SCS_G0_ASCII:   Invoke US-ASCII charset into G0
        VT_ESC('B', 0x00, ')',  LEVEL_1, LEVEL_4,  { t.gsets.G1(CHARSET_TOASCII);                                 }),   // SCS_G1_ASCII:   Invoke US-ASCII charset into G1
        VT_ESC('B', 0x00, '*',  LEVEL_2, LEVEL_4,  { t.gsets.G2(CHARSET_TOASCII);                                 }),   // SCS_G2_ASCII:   Invoke US-ASCII charset into G2
        VT_ESC('B', 0x00, '+',  LEVEL_2, LEVEL_4,  { t.gsets.G3(CHARSET_TOASCII);                                 }),   // SCS_G3_ASCII:   Invoke US-ASCII charset into G3
        VT_ESC('G', 0x00, '%',  LEVEL_3, LEVEL_4,  { t.gsets.Broadcast(CHARSET_UTF8);                             }),   // SCS_UTF8:       Select UTF-8 charset
        VT_ESC('<', 0x00, '(',  LEVEL_2, LEVEL_4,  { t.gsets.G0(CHARSET_DEC_MCS);                                 }),   // SCS_G0_DEC_MCS: Invoke DEC supplemental charset into G0
        VT_ESC('<', 0x00, ')',  LEVEL_2, LEVEL_4,  { t.gsets.G1(CHARSET_DEC_MCS);                                 }),   // SCS_G1_DEC_MCS: Invoke DEC supplemental charset into G1
        VT_ESC('<', 0x00, '*',  LEVEL_2, LEVEL_4,  { t.gsets.G2(CHARSET_DEC_MCS);                                 }),   // SCS_G2_DEC_MCS: Invoke DEC supplemental charset into G2
        VT_ESC('<', 0x00, '+',  LEVEL_2, LEVEL_4,  { t.gsets.G3(CHARSET_DEC_MCS);                                 }),   // SCS_G3_DEC_MCS: Invoke DEC supplemental charset into G3
        VT_ESC('>', 0x00, '(',  LEVEL_3, LEVEL_4,  { t.gsets.G0(CHARSET_DEC_TCS);                                 }),   // SCS_G0_DEC_TCS: Invoke DEC technical charset into G0
        VT_ESC('>', 0x00, ')',  LEVEL_3, LEVEL_4,  { t.gsets.G1(CHARSET_DEC_TCS);                                 }),   // SCS_G1_DEC_TCS: Invoke DEC technical charset into G1
        VT_ESC('>', 0x00, '*',  LEVEL_3, LEVEL_4,  { t.gsets.G2(CHARSET_DEC_TCS);                                 }),   // SCS_G2_DEC_TCS: Invoke DEC technical charset into G2
        VT_ESC('>', 0x00, '+',  LEVEL_3, LEVEL_4,  { t.gsets.G3(CHARSET_DEC_TCS);                                 }),   // SCS_G3_DEC_TCS: Invoke DEC technical charset into G3
        // VT52 specific escape sequences
        VT_ESC('A', 0x00, 0x00, LEVEL_0, LEVEL_0,  { t.page->MoveUp();                                            }),   // VT52_CUU:     Move upward
        VT_ESC('B', 0x00, 0x00, LEVEL_0, LEVEL_0,  { t.page->MoveDown();                                          }),   // VT52_CUD:     Move downward
        VT_ESC('C', 0x00, 0x00, LEVEL_0, LEVEL_0,  { t.page->MoveRight();                                         }),   // VT52_CUF:     Move forward
        VT_ESC('D', 0x00, 0x00, LEVEL_0, LEVEL_0,  { t.page->MoveLeft();                                          }),   // VT52_CUB:     Move backward
        VT_ESC('F', 0x00, 0x00, LEVEL_0, LEVEL_0,  { t.gsets.G0(CHARSET_DEC_VT52);                                }),   // VT52_DCS_ON:  Drawing characters: on
        VT_ESC('G', 0x00, 0x00, LEVEL_0, LEVEL_0,  { t.gsets.ResetG0();                                           }),   // VT52_DCS_OFF: Drawing characters: off
        VT_ESC('H', 0x00, 0x00, LEVEL_0, LEVEL_0,  { t.page->MoveTopLeft();                                       }),   // VT52_HOME:    Move home
        VT_ESC('I', 0x00, 0x00, LEVEL_0, LEVEL_0,  { t.page->PrevLine();                                          }),   // VT52_RI:      Reverse index
        VT_ESC('J', 0x00, 0x00, LEVEL_0, LEVEL_0,  { t.page->EraseAfter();                                        }),   // VT52_ED:      Erase to end of screen
        VT_ESC('K', 0x00, 0x00, LEVEL_0, LEVEL_0,  { t.page->EraseRight();                                        }),   // VT52_EL:      Erase to end of line
        VT_ESC('Y', 0x00, 0x00, LEVEL_0, LEVEL_0,  { t.VT52MoveCursor();                                          }),   // VT52_CUP:     Direct cursor addressing
        VT_ESC('Z', 0x00, 0x00, LEVEL_0, LEVEL_0,  { t.ReportDeviceAttributes(q);                                 }),   // VT52_DA:      Device identify
        VT_ESC('<', 0x00, 0x00, LEVEL_0, LEVEL_0,  { t.DECanm(true);                                              }),   // VT52_DECANM:  Enter ANSI mode (leave VT52 mode)
        // Command sequences
        VT_CSI('@', 0x00, 0x00, LEVEL_2, LEVEL_4,  { t.page->InsertCells(q.GetInt(1));                            }),   // ICH,         Insert character
        VT_CSI('@', 0x00, ' ',  LEVEL_3, LEVEL_4,  { t.page->ScrollRight(q.GetInt(1));                            }),   // SL,          Scroll/shift left
        VT_CSI('A', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.page->MoveUp(q.GetInt(1));                                 }),   // CUU,         Move upward
        VT_CSI('A', 0x00, ' ',  LEVEL_3, LEVEL_4,  { t.page->ScrollLeft(q.GetInt(1));                             }),   // SR,          Scroll/shift right
        VT_CSI('B', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.page->MoveDown(q.GetInt(1));                               }),   // CUD,         Move downward
        VT_CSI('C', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.page->MoveRight(q.GetInt(1));                              }),   // CUF,         Move forward
        VT_CSI('D', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.page->MoveLeft(q.GetInt(1));                               }),   // CUB,         Move backward
        VT_CSI('E', 0x00, 0x00, LEVEL_4, LEVEL_4,  { t.page->MoveHome().MoveDown(q.GetInt(1));                    }),   // CNL,         Move to next line (no scrolling)
        VT_CSI('F', 0x00, 0x00, LEVEL_4, LEVEL_4,  { t.page->MoveUp(q.GetInt(1)).MoveHome();                      }),   // CPL,         Move to prev line (no scrolling)
        VT_CSI('G', 0x00, 0x00, LEVEL_4, LEVEL_4,  { t.page->MoveToColumn(q.GetInt(1));                           }),   // CHA,         Cursor horizontal absolute
        VT_CSI('H', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.page->MoveTo(q.GetInt(2), q.GetInt(1));                    }),   // CUP,         Cursor position
        VT_CSI('I', 0x00, 0x00, LEVEL_4, LEVEL_4,  { t.page->NextTab(q.GetInt(1));                                }),   // CHT,         Cursor horizontal tabulation
        VT_CSI('J', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.ClearPage(q, t.GetISOStyleFillerFlags());                  }),   // ED,          Erase screen
        VT_CSI('J', '?',  0x00, LEVEL_2, LEVEL_4,  { t.ClearPage(q, t.GetDECStyleFillerFlags());                  }),   // DECSED,      Selectively erase screen
        VT_CSI('K', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.ClearLine(q, t.GetISOStyleFillerFlags());                  }),   // EL,          Erase line
        VT_CSI('K', '?',  0x00, LEVEL_2, LEVEL_4,  { t.ClearLine(q, t.GetDECStyleFillerFlags());                  }),   // DECSEL,      Selectively erase line
        VT_CSI('L', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.page->InsertLines(q.GetInt(1));                            }),   // IL,          Insert line
        VT_CSI('M', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.page->RemoveLines(q.GetInt(1));                            }),   // DL,          Remove line
        VT_CSI('P', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.page->RemoveCells(q.GetInt(1));                            }),   // DCH,         Delete character
        VT_CSI('S', 0x00, 0x00, LEVEL_3, LEVEL_4,  { t.page->ScrollDown(q.GetInt(1));                             }),   // SU,          Scroll up
        VT_CSI('T', 0x00, 0x00, LEVEL_3, LEVEL_4,  { t.page->ScrollUp(q.GetInt(1));                               }),   // SD,          Scroll down
        VT_CSI('W', '?',  0x00, LEVEL_4, LEVEL_4,  { if(q.GetInt(1) == 5) t.page->SetTabs(8);                     }),   // DECST8C,     Set a tab at every 8 columns
        VT_CSI('X', 0x00, 0x00, LEVEL_2, LEVEL_4,  { t.page->EraseCells(q.GetInt(1), t.GetISOStyleFillerFlags()); }),   // ECH,         Erase character
        VT_CSI('Z', 0x00, 0x00, LEVEL_4, LEVEL_4,  { t.page->PrevTab(q.GetInt(1));                                }),   // CBT,         Cursor backward tabulation
        VT_CSI('^', 0x00, 0x00, LEVEL_3, LEVEL_4,  { t.page->EraseCells(q.GetInt(1), t.GetISOStyleFillerFlags()); }),   // ECH,         FIXME
        VT_CSI('`', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.page->MoveToColumn(q.GetInt(1));                           }),   // HPA,         Horizontal position absolute
        VT_CSI('a', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.page->MoveToColumn(q.GetInt(1), true);                     }),   // HPR,         Horizontal position relative
        VT_CSI('b', 0x00, 0x00, LEVEL_3, LEVEL_4,  { if(t.parser.WasChr()) t.page->RepeatCell(q.GetInt(1));       }),   // REP,         Repeat last character
        VT_CSI('c', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.ReportDeviceAttributes(q);                                 }),   // DA1,         Send primary device attributes
        VT_CSI('c', '>',  0x00, LEVEL_1, LEVEL_4,  { t.ReportDeviceAttributes(q);                                 }),   // DA2,         Send secondary device attributes
        VT_CSI('c', '=',  0x00, LEVEL_4, LEVEL_4,  { t.ReportDeviceAttributes(q);                                 }),   // DA3,         Send tertiary device attributes
        VT_CSI('d', 0x00, 0x00, LEVEL_4, LEVEL_4,  { t.page->MoveToLine(q.GetInt(1));                             }),   // VPA,         Vertical position absolute
        VT_CSI('e', 0x00, 0x00, LEVEL_4, LEVEL_4,  { t.page->MoveToLine(q.GetInt(1), true);                       }),   // VPR,         Vertical position relative
        VT_CSI('f', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.page->MoveTo(q.GetInt(2), q.GetInt(1));                    }),   // HVP,         Cursor horz and vert position
        VT_CSI('g', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.ClearTabs(q);                                              }),   // TBC,         Clear tabs
        VT_CSI('h', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.SetMode(q, true);                                          }),   // SM,          Set ANSI modes
        VT_CSI('h', '?',  0x00, LEVEL_1, LEVEL_4,  { t.SetMode(q, true);                                          }),   // SM,          Set private modes
        VT_CSI('l', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.SetMode(q, false);                                         }),   // RM,          Reset ANSI modes
        VT_CSI('l', '?',  0x00, LEVEL_1, LEVEL_4,  { t.SetMode(q, false);                                         }),   // RM,          Reset private modes
        VT_CSI('m', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.SelectGraphicsRendition(q);                                }),   // SGR,         Select graphics rendition (ANSI)
        VT_CSI('m', '?',  0x00, LEVEL_1, LEVEL_4,  { t.SelectGraphicsRendition(q);                                }),   // SGR,         Select graphics rendition (DEC)
        VT_CSI('n', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.ReportDeviceStatus(q);                                     }),   // DSR,         Send device status report (ANSI format)
        VT_CSI('n', '?',  0x00, LEVEL_1, LEVEL_4,  { t.ReportDeviceStatus(q);                                     }),   // DECDSR,      Send device status (cursor position) report (DEC format)
        VT_CSI('p', 0x00, '\"', LEVEL_1, LEVEL_4,  { t.SetDeviceConformanceLevel(q);                              }),   // DECSCL,      Select device conformance level
        VT_CSI('p', 0x00, '!',  LEVEL_2, LEVEL_4,  { t.SoftReset();                                               }),   // DECSTR,      Soft reset
        VT_CSI('p', 0x00, '$',  LEVEL_2, LEVEL_4,  { t.ReportMode(q);                                             }),   // DECRQM,      Request ANSI mode
        VT_CSI('p', '?',  '$',  LEVEL_2, LEVEL_4,  { t.ReportMode(q);                                             }),   // DECRQM,      Request private mode
        VT_CSI('q', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.SetProgrammableLEDs(q);                                    }),   // DECLL,       Set programmable LEDs
        VT_CSI('q', 0x00, '\"', LEVEL_2, LEVEL_4,  { t.SetDECStyleCellProtection(q.GetInt(1, 0) == 1);            }),   // DECSCA,      Set character protection attribute
        VT_CSI('q', 0x00, ' ',  LEVEL_4, LEVEL_4,  { t.SetCaretStyle(q);                                          }),   // DECSCUSR,    Set cursor style
        VT_CSI('r', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.SetVerticalMargins(q);                                     }),   // DECSTBM,     Set vertical margins
        VT_CSI('r', 0x00, '$',  LEVEL_4, LEVEL_4,  { t.ChangeRectAreaAttrs(q, false);                             }),   // DECCARA,     Change attributes in rectangular area
        VT_CSI('s', 0x00, 0x00, LEVEL_3, LEVEL_4,  { t.SetHorizontalMargins(q);                                   }),   // DECSLRM,     Set horizontal margins / SCO save cursor
        VT_CSI('t', 0x00, 0x00, LEVEL_3, LEVEL_4,  { t.SetLinesPerPage(q);                                        }),   // DECSLPP,     Set lines per page
        VT_CSI('t', 0x00, '$',  LEVEL_4, LEVEL_4,  { t.ChangeRectAreaAttrs(q, true);                              }),   // DECRARA,     Reverse attributes in rectangular area
        VT_CSI('u', 0x00, 0x00, LEVEL_3, LEVEL_4,  { t.Restore();                                                 }),   // SCORC,       SCO restore cursor
        VT_CSI('v', 0x00, '$',  LEVEL_4, LEVEL_4,  { t.CopyRectArea(q);                                           }),   // DECCRA,      Copy rectangular area.
        VT_CSI('w', 0x00, '$',  LEVEL_3, LEVEL_4,  { t.ReportPresentationState(q);                                }),   // DECRQPSR,    Request presentation state report
        VT_CSI('x', 0x00, 0x00, LEVEL_1, LEVEL_4,  { t.ReportDeviceParameters(q);                                 }),   // DECREQTPARM, Request terminal parameters
        VT_CSI('x', 0x00, '$',  LEVEL_4, LEVEL_4,  { t.FillRectArea(q);                                           }),   // DECFRA,      Fill rectangular area
        VT_CSI('x', 0x00, '*',  LEVEL_4, LEVEL_4,  { t.SelectRectAreaAttrsChangeExtent(q);                        }),   // DECSACE,     Select rectangular area attribute change extent.
        VT_CSI('y', 0x00, 0x00, LEVEL_1, LEVEL_4,  { /* NOP */                                                    }),   // DECTST,      Device confidence tests
        VT_CSI('y', 0x00, '*',  LEVEL_4, LEVEL_4,  { t.ReportRectAreaChecksum(q);                                 }),   // DECRQCRA,    Request rectangular area checksum
        VT_CSI('z', 0x00, '$',  LEVEL_4, LEVEL_4,  { t.ClearRectArea(q);                                          }),   // DECERA,      Erase rectangular area
        VT_CSI('{', 0x00, '$',  LEVEL_4, LEVEL_4,  { t.ClearRectArea(q, true);                                    }),   // DECSERA,     Selectively erase rectangular area
        VT_CSI('|', 0x00, '$',  LEVEL_3, LEVEL_4,  { t.SetColumns(q.GetInt(1) != 132 ? 80 : 132);                 }),   // DECSCPP,     Set columns per page
        VT_CSI('|', 0x00, '*',  LEVEL_4, LEVEL_4,  { t.SetRows(max(q.GetInt(1), 1));                              }),   // DECSNLS,     Set number of lines per screen
        VT_CSI('}', 0x00, '\'', LEVEL_4, LEVEL_4,  { t.page->PanRight(q.GetInt(1));                               }),   // DECIC,       Insert column
        VT_CSI('~', 0x00, '\'', LEVEL_4, LEVEL_4,  { t.page->PanLeft(q.GetInt(1));                                }),   // DECDC,       Delete column
        // Device control strings
        VT_DCS('q', 0x00, 0x00, LEVEL_3, LEVEL_4,  { t.ParseSixelGraphics(q);                                     }),   // DECSIXEL: Parse sixel graphics
        VT_DCS('q', 0x00, '$',  LEVEL_2, LEVEL_4,  { t.ReportControlFunctionSettings(q);                          }),   // DECRQSS:  Request control function strings
        VT_DCS('t', 0x00, '$',  LEVEL_3, LEVEL_4,  { t.RestorePresentationState(q);                               }),   // DECRSPS:  Restore presentation state
        VT_DCS('|', 0x00, 0x00, LEVEL_2, LEVEL_4,  { t.SetUserDefinedKeys(q);                                     })    // DECUDK:   Set user-defined keys
    };
    }
    
    LTIMING("TerminalCtrl::FındFunctionPtr");

    const CbFunction *p = vtsequences.FindPtr(
        MakeVTSequenceKey(
            seq.type,
            seq.opcode,
            seq.mode,
            seq.intermediate[0]
        )
    );
    
    if(p && clevel >= p->a && clevel <= p->b)
        return p;
    
    LLOG(decode(seq.type,
        VTInStream::Sequence::ESC, "Unhandled ESC sequence",
        VTInStream::Sequence::CSI, "Unhandled CSI sequence",
        VTInStream::Sequence::DCS, "UnHandled DCS sequence", "Unknown sequence type"));

    return nullptr;
}

const TerminalCtrl::CbMode* TerminalCtrl::FindModePtr(word modenum, byte modetype)
{
    #define VT_MODE(id, mode, type, minlevel, maxlevel, fn)        \
    {                                                              \
        { MAKELONG(mode, type) },                                  \
        { id, TerminalCtrl::minlevel, TerminalCtrl::maxlevel, [](TerminalCtrl& t, int n, bool b) fn  }   \
    }

    static VectorMap<dword, CbMode> vtmodes;
    
    ONCELOCK {
    vtmodes = {
        // ANSI modes
        VT_MODE(GATM,       1,      0x00,   LEVEL_1, LEVEL_4,  { /* NOP */       }),    // Permanently reset
        VT_MODE(KAM,        2,      0x00,   LEVEL_1, LEVEL_4,  { t.ANSIkam(b);   }),    // Keyboard action mode
        VT_MODE(CRM,        3,      0x00,   LEVEL_1, LEVEL_4,  { t.ANSIcrm(b);   }),    // Show/hide control characters
        VT_MODE(IRM,        4,      0x00,   LEVEL_1, LEVEL_4,  { t.ANSIirm(b);   }),    // Insert/replace characters
        VT_MODE(SRTM,       5,      0x00,   LEVEL_1, LEVEL_4,  { /* NOP */       }),    // Permanently reset
        VT_MODE(ERM,        6,      0x00,   LEVEL_1, LEVEL_4,  { t.ANSIerm(b);   }),    // Erasure mode
        VT_MODE(VEM,        7,      0x00,   LEVEL_1, LEVEL_4,  { /* NOP */       }),    // Permanently reset
        VT_MODE(HEM,        10,     0x00,   LEVEL_1, LEVEL_4,  { /* NOP */       }),    // Permanently reset
        VT_MODE(PUM,        11,     0x00,   LEVEL_1, LEVEL_4,  { /* NOP */       }),    // Permanently reset
        VT_MODE(SRM,        12,     0x00,   LEVEL_1, LEVEL_4,  { t.ANSIsrm(b);   }),    // Send/receive mode (local echo)
        VT_MODE(FEAM,       13,     0x00,   LEVEL_1, LEVEL_4,  { /* NOP */       }),    // Permanently reset
        VT_MODE(FETM,       14,     0x00,   LEVEL_1, LEVEL_4,  { /* NOP */       }),    // Permanently reset
        VT_MODE(MATM,       15,     0x00,   LEVEL_1, LEVEL_4,  { /* NOP */       }),    // Permanently reset
        VT_MODE(TTM,        16,     0x00,   LEVEL_1, LEVEL_4,  { /* NOP */       }),    // Permanently reset
        VT_MODE(SATM,       17,     0x00,   LEVEL_1, LEVEL_4,  { /* NOP */       }),    // Permanently reset
        VT_MODE(TSM,        18,     0x00,   LEVEL_1, LEVEL_4,  { /* NOP */       }),    // Permanently reset
        VT_MODE(EBM,        19,     0x00,   LEVEL_1, LEVEL_4,  { /* NOP */       }),    // Permanently reset
        VT_MODE(LNM,        20,     0x00,   LEVEL_1, LEVEL_4,  { t.ANSIlnm(b);   }),    // Line feed/new line mode
        // Private modes
        VT_MODE(DECCKM,     1,      '?',    LEVEL_1, LEVEL_4,  { t.DECckm(b);    }),    // Cursor keys mode
        VT_MODE(DECANM,     2,      '?',    LEVEL_1, LEVEL_4,  { t.DECanm(b);    }),    // Leave ANSI mode (enter VT52 mode)
        VT_MODE(DECCOLM,    3,      '?',    LEVEL_1, LEVEL_4,  { t.DECcolm(b);   }),    // 80/132 columns mode
        VT_MODE(DECSCLM,    4,      '?',    LEVEL_1, LEVEL_4,  { t.DECsclm(b);   }),    // Scrolling mode
        VT_MODE(DECSCNM,    5,      '?',    LEVEL_1, LEVEL_4,  { t.DECscnm(b);   }),    // Normal/inverse video mode
        VT_MODE(DECOM,      6,      '?',    LEVEL_1, LEVEL_4,  { t.DECom(b);     }),    // Origin mode
        VT_MODE(DECAWM,     7,      '?',    LEVEL_1, LEVEL_4,  { t.DECawm(b);    }),    // Autowrap mode
        VT_MODE(DECARM,     8,      '?',    LEVEL_1, LEVEL_4,  { t.DECarm(b);    }),    // Autorepeat mode
        VT_MODE(DECTCEM,    25,     '?',    LEVEL_2, LEVEL_4,  { t.DECtcem(b);   }),    // Show/hide caret
        VT_MODE(DECBKM,     67,     '?',    LEVEL_3, LEVEL_4,  { t.DECbkm(b);    }),    // Send backspace when backarrow key is pressed
        VT_MODE(DECLRMM,    69,     '?',    LEVEL_4, LEVEL_4,  { t.DEClrmm(b);   }),    // Enable/disable horizontal margins
        VT_MODE(DECSDM,     80,     '?',    LEVEL_3, LEVEL_4,  { t.DECsdm(b);    }),    // Enable/disable sixel scrolling
        // Private mode extensions
        VT_MODE(XTX10MM,    9,      '?',    LEVEL_1, LEVEL_4,  { t.XTx10mm(b);   }),    // X10 mouse button tracking mode (compat.)
        VT_MODE(XTREWRAPM,  45,     '?',    LEVEL_1, LEVEL_4,  { t.XTrewrapm(b); }),    // Reverse wrap mode
        VT_MODE(XTASBM,     47,     '?',    LEVEL_1, LEVEL_4,  { t.XTasbm(n, b); }),    // Alternate screen buffer mode (ver. 1)
        VT_MODE(XTX11MM,    1000,   '?',    LEVEL_1, LEVEL_4,  { t.XTx11mm(b);   }),    // X11 mouse button tracking mode (normal)
        VT_MODE(XTDRAGM,    1002,   '?',    LEVEL_1, LEVEL_4,  { t.XTdragm(b);   }),    // X11 mouse motion tracking mode
        VT_MODE(XTANYMM,    1003,   '?',    LEVEL_1, LEVEL_4,  { t.XTanymm(b);   }),    // X11 mouse motion tracking mode (any motion event)
        VT_MODE(XTFOCUSM,   1004,   '?',    LEVEL_1, LEVEL_4,  { t.XTfocusm(b);  }),    // Focus in/out mode
        VT_MODE(XTUTF8MM,   1005,   '?',    LEVEL_1, LEVEL_4,  { t.XTutf8mm(b);  }),    // Enable/disable UTF8 mouse tracking coordinates
        VT_MODE(XTSGRMM,    1006,   '?',    LEVEL_1, LEVEL_4,  { t.XTsgrmm(b);   }),    // Enable/disable SGR mouse tracking coordinates
        VT_MODE(XTASCM,     1007,   '?',    LEVEL_1, LEVEL_4,  { t.XTascm(b);    }),    // Alternate scroll mode
        VT_MODE(XTSGRPXMM,  1016,   '?',    LEVEL_1, LEVEL_4,  { t.XTsgrpxmm(b); }),    // Enable/disable SGR pixel-level mouse tracking coordinates
        VT_MODE(XTPCFKEYM,  1035,   '?',    LEVEL_1, LEVEL_4,  { t.XTpcfkeym(b); }),    // Enable/disable PC-style function keys.
        VT_MODE(XTALTESCM,  1039,   '?',    LEVEL_1, LEVEL_4,  { t.XTaltkeym(b); }),    // Prefix the key with ESC when modified with Alt-key
        VT_MODE(XTASBM,     1047,   '?',    LEVEL_1, LEVEL_4,  { t.XTasbm(n, b); }),    // Alternate screen buffer mode (ver. 2)
        VT_MODE(XTSRCM,     1048,   '?',    LEVEL_1, LEVEL_4,  { t.XTsrcm(b);    }),    // Save/restore cursor
        VT_MODE(XTASBM,     1049,   '?',    LEVEL_1, LEVEL_4,  { t.XTasbm(n, b); }),    // Alternate screen buffer mode (ver. 3)
        VT_MODE(XTSPREG,    1070,   '?',    LEVEL_1, LEVEL_4,  { /* NOP */       }),    // Use private color registers for each sixel (permanently set)
        VT_MODE(XTBRPM,     2004,   '?',    LEVEL_1, LEVEL_4,  { t.XTbrpm(b);    })     // Bracketed paste mode
    };
    }

     LTIMING("TerminalCtrl::FındModePtr");
     
    const CbMode *p = vtmodes.FindPtr(MAKELONG(modenum, modetype));
    return (p && clevel >= p->b && clevel <= p->c) ? p : nullptr;
}
}
