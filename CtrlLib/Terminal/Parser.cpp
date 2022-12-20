#include "Parser.h"

// VTInStream: A "lexical" parser for DEC and ANSI escape sequences in general.
// This parser is based on the UML state diagram provided by Paul-Flo Williams
// See: https://vt100.net/emu/dec_ansi_parser

// Deviations from the DEC STD-070:
// 1) ISO 8613-6: 0x3a ("colon") is considered as a legitimate delimiter.
// 2) The OSC sequences allow UTF-8 payload if the UTF-8 mode is enabled.

#define LLOG(x)	   // RLOG("VTInStream: " << x);
#define LTIMING(x) // RTIMING(x)

namespace Upp {

#define VT_BEGIN_STATE_MAP(sname)                   \
    const static Vector<VTInStream::State> sname =  {

#define VT_END_STATE_MAP                            \
    }

#define VT_STATE(begin, end, action, next)          \
    {                                               \
        begin,                                      \
        end,                                        \
        VTInStream::State::Action::action,          \
        VTInStream::State::Id::next                 \
    }

VT_BEGIN_STATE_MAP(EscEntry)
    VT_STATE(0x00, 0x17, Control,       Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Control,       Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, Ignore,        Repeat),
    VT_STATE(0x1c, 0x1f, Control,       Repeat),
    VT_STATE(0x20, 0x2f, Collect,       EscIntermediate),
    VT_STATE(0x30, 0x4f, DispatchEsc,   Ground),
    VT_STATE(0x50, 0x50, Ignore,        DcsEntry),
    VT_STATE(0x51, 0x57, DispatchEsc,   Ground),
    VT_STATE(0x58, 0x58, Ignore,        Ignore),
    VT_STATE(0x59, 0x59, DispatchEsc,   Ground),
    VT_STATE(0x5a, 0x5a, DispatchEsc,   Ground),
    VT_STATE(0x5b, 0x5b, Ignore,        CsiEntry),
    VT_STATE(0x5c, 0x5c, DispatchEsc,   Ground),
    VT_STATE(0x5d, 0x5d, Ignore,        OscString),
    VT_STATE(0x5e, 0x5e, Ignore,        Ignore),
    VT_STATE(0x5f, 0x5f, Ignore,        ApcString),
    VT_STATE(0x60, 0x7e, DispatchEsc,   Ground),
    VT_STATE(0x7f, 0x7f, Control,       Repeat),
    VT_STATE(0x80, 0x8f, Control,       Ground),
    VT_STATE(0x90, 0x90, Ignore,        DcsEntry),
    VT_STATE(0x91, 0x97, Control,       Ground),
    VT_STATE(0x98, 0x98, Ignore,        Ignore),
    VT_STATE(0x99, 0x9a, Control,       Ground),
    VT_STATE(0x9b, 0x9b, Ignore,        CsiEntry),
    VT_STATE(0x9c, 0x9c, Ignore,        Repeat),
    VT_STATE(0x9d, 0x9d, Ignore,        OscString),
    VT_STATE(0x9e, 0x9e, Ignore,        Ignore),
    VT_STATE(0x9f, 0x9f, Ignore,        ApcString)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(EscIntermediate)
    VT_STATE(0x00, 0x17, Control,       Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Control,       Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, Ignore,        EscEntry),
    VT_STATE(0x1c, 0x1f, Control,       Repeat),
    VT_STATE(0x20, 0x2f, Collect,       Repeat),
    VT_STATE(0x30, 0x7e, DispatchEsc,   Ground),
    VT_STATE(0x7f, 0x7f, Control,       Repeat),
    VT_STATE(0x80, 0x8f, Control,       Ground),
    VT_STATE(0x90, 0x90, Ignore,        DcsEntry),
    VT_STATE(0x91, 0x97, Control,       Ground),
    VT_STATE(0x98, 0x98, Ignore,        Ignore),
    VT_STATE(0x99, 0x9a, Control,       Ground),
    VT_STATE(0x9b, 0x9b, Ignore,        CsiEntry),
    VT_STATE(0x9c, 0x9c, Ignore,        Repeat),
    VT_STATE(0x9d, 0x9d, Ignore,        OscString),
    VT_STATE(0x9e, 0x9e, Ignore,        Ignore),
    VT_STATE(0x9f, 0x9f, Ignore,        ApcString)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(CsiEntry)
    VT_STATE(0x00, 0x17, Control,       Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Control,       Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, Ignore,        EscEntry),
    VT_STATE(0x1c, 0x1f, Control,       Repeat),
    VT_STATE(0x20, 0x2f, Collect,       CsiIntermediate),
    VT_STATE(0x30, 0x3b, Parameter,     CsiParameter),
    VT_STATE(0x3c, 0x3f, Mode,          CsiParameter),
    VT_STATE(0x40, 0x7e, DispatchCsi,   Ground),
    VT_STATE(0x7f, 0x7f, Control,       Repeat),
    VT_STATE(0x80, 0x8f, Control,       Ground),
    VT_STATE(0x90, 0x90, Ignore,        DcsEntry),
    VT_STATE(0x91, 0x97, Control,       Ground),
    VT_STATE(0x98, 0x98, Ignore,        Ignore),
    VT_STATE(0x99, 0x9a, Control,       Ground),
    VT_STATE(0x9b, 0x9b, Ignore,        Repeat),
    VT_STATE(0x9c, 0x9c, Ignore,        Repeat),
    VT_STATE(0x9d, 0x9d, Ignore,        OscString),
    VT_STATE(0x9e, 0x9e, Ignore,        Ignore),
    VT_STATE(0x9f, 0x9f, Ignore,        ApcString)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(CsiParameter)
    VT_STATE(0x00, 0x17, Control,       Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Control,       Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, Ignore,        EscEntry),
    VT_STATE(0x1c, 0x1f, Control,       Repeat),
    VT_STATE(0x20, 0x2f, Collect,       CsiIntermediate),
    VT_STATE(0x30, 0x3b, Parameter,     Repeat),
    VT_STATE(0x3c, 0x3f, Ignore,        CsiIgnore),
    VT_STATE(0x40, 0x7e, DispatchCsi,   Ground),
    VT_STATE(0x7f, 0x7f, Control,       Repeat),
    VT_STATE(0x80, 0x8f, Control,       Ground),
    VT_STATE(0x90, 0x90, Ignore,        DcsEntry),
    VT_STATE(0x91, 0x97, Control,       Ground),
    VT_STATE(0x98, 0x98, Ignore,        Ignore),
    VT_STATE(0x99, 0x9a, Control,       Ground),
    VT_STATE(0x9b, 0x9b, Ignore,        CsiEntry),
    VT_STATE(0x9c, 0x9c, Ignore,        Repeat),
    VT_STATE(0x9d, 0x9d, Ignore,        OscString),
    VT_STATE(0x9e, 0x9e, Ignore,        Ignore),
    VT_STATE(0x9f, 0x9f, Ignore,        ApcString)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(CsiIntermediate)
    VT_STATE(0x00, 0x17, Control,       Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Control,       Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, Ignore,        EscEntry),
    VT_STATE(0x1c, 0x1f, Control,       Repeat),
    VT_STATE(0x20, 0x2f, Collect,       Repeat),
    VT_STATE(0x30, 0x3f, Ignore,        CsiIgnore),
    VT_STATE(0x40, 0x7e, DispatchCsi,   Ground),
    VT_STATE(0x7f, 0x7f, Control,       Repeat),
    VT_STATE(0x80, 0x8f, Control,       Ground),
    VT_STATE(0x90, 0x90, Ignore,        DcsEntry),
    VT_STATE(0x91, 0x97, Control,       Ground),
    VT_STATE(0x98, 0x98, Ignore,        Ignore),
    VT_STATE(0x99, 0x9a, Control,       Ground),
    VT_STATE(0x9b, 0x9b, Ignore,        CsiEntry),
    VT_STATE(0x9c, 0x9c, Ignore,        Repeat),
    VT_STATE(0x9d, 0x9d, Ignore,        OscString),
    VT_STATE(0x9e, 0x9e, Ignore,        Ignore),
    VT_STATE(0x9f, 0x9f, Ignore,        ApcString)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(CsiIgnore)
    VT_STATE(0x00, 0x17, Control,       Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Control,       Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, Ignore,        EscEntry),
    VT_STATE(0x1c, 0x1f, Control,       Repeat),
    VT_STATE(0x20, 0x3f, Ignore,        Repeat),
    VT_STATE(0x40, 0x7e, Ignore,        Ground),
    VT_STATE(0x7f, 0x7f, Control,       Repeat),
    VT_STATE(0x80, 0x8f, Control,       Ground),
    VT_STATE(0x90, 0x90, Ignore,        DcsEntry),
    VT_STATE(0x91, 0x97, Control,       Ground),
    VT_STATE(0x98, 0x98, Ignore,        Ignore),
    VT_STATE(0x99, 0x9a, Control,       Ground),
    VT_STATE(0x9b, 0x9b, Ignore,        CsiEntry),
    VT_STATE(0x9c, 0x9c, Ignore,        Repeat),
    VT_STATE(0x9d, 0x9d, Ignore,        OscString),
    VT_STATE(0x9e, 0x9e, Ignore,        Ignore),
    VT_STATE(0x9f, 0x9f, Ignore,        ApcString)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(DcsEntry)
    VT_STATE(0x00, 0x17, Ignore,        Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Ignore,        Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, Ignore,        EscEntry),
    VT_STATE(0x1c, 0x1f, Ignore,        Repeat),
    VT_STATE(0x20, 0x2f, Collect,       DcsIntermediate),
    VT_STATE(0x30, 0x3b, Parameter,     DcsParameter),
    VT_STATE(0x3c, 0x3f, Mode,          DcsParameter),
    VT_STATE(0x40, 0x7e, Final,         DcsPassthrough),
    VT_STATE(0x7f, 0x7f, Control,       Repeat),
    VT_STATE(0x80, 0x8f, Control,       Ground),
    VT_STATE(0x90, 0x90, Ignore,        Repeat),
    VT_STATE(0x91, 0x97, Control,       Ground),
    VT_STATE(0x98, 0x98, Ignore,        Ignore),
    VT_STATE(0x99, 0x9a, Control,       Ground),
    VT_STATE(0x9b, 0x9b, Ignore,        CsiEntry),
    VT_STATE(0x9c, 0x9c, Ignore,        Repeat),
    VT_STATE(0x9d, 0x9d, Ignore,        OscString),
    VT_STATE(0x9e, 0x9e, Ignore,        Ignore),
    VT_STATE(0x9f, 0x9f, Ignore,        ApcString)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(DcsParameter)
    VT_STATE(0x00, 0x17, Ignore,        Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Ignore,        Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, Ignore,        EscEntry),
    VT_STATE(0x1c, 0x1f, Ignore,        Repeat),
    VT_STATE(0x20, 0x2f, Collect,       DcsIntermediate),
    VT_STATE(0x30, 0x3b, Parameter,     Repeat),
    VT_STATE(0x3c, 0x3f, Ignore,        DcsIgnore),
    VT_STATE(0x40, 0x7e, Final,         DcsPassthrough),
    VT_STATE(0x7f, 0x7f, Control,       Repeat),
    VT_STATE(0x80, 0x8f, Control,       Ground),
    VT_STATE(0x90, 0x90, Ignore,        DcsEntry),
    VT_STATE(0x91, 0x97, Control,       Ground),
    VT_STATE(0x98, 0x98, Ignore,        Ignore),
    VT_STATE(0x99, 0x9a, Control,       Ground),
    VT_STATE(0x9b, 0x9b, Ignore,        CsiEntry),
    VT_STATE(0x9c, 0x9c, Ignore,        Repeat),
    VT_STATE(0x9d, 0x9d, Ignore,        OscString),
    VT_STATE(0x9e, 0x9e, Ignore,        Ignore),
    VT_STATE(0x9f, 0x9f, Ignore,        ApcString)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(DcsIntermediate)
    VT_STATE(0x00, 0x17, Ignore,        Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Ignore,        Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, Ignore,        EscEntry),
    VT_STATE(0x1c, 0x1f, Ignore,        Repeat),
    VT_STATE(0x20, 0x2f, Collect,       Repeat),
    VT_STATE(0x30, 0x3f, Ignore,        DcsIgnore),
    VT_STATE(0x40, 0x7e, Final,         DcsPassthrough),
    VT_STATE(0x7f, 0x7f, Control,       Repeat),
    VT_STATE(0x80, 0x8f, Control,       Ground),
    VT_STATE(0x90, 0x90, Ignore,        DcsEntry),
    VT_STATE(0x91, 0x97, Control,       Ground),
    VT_STATE(0x98, 0x98, Ignore,        Ignore),
    VT_STATE(0x99, 0x9a, Control,       Ground),
    VT_STATE(0x9b, 0x9b, Ignore,        CsiEntry),
    VT_STATE(0x9c, 0x9c, Ignore,        Repeat),
    VT_STATE(0x9d, 0x9d, Ignore,        OscString),
    VT_STATE(0x9e, 0x9e, Ignore,        Ignore),
    VT_STATE(0x9f, 0x9f, Ignore,        ApcString)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(DcsPassthrough)
    VT_STATE(0x00, 0x17, Passthrough,   Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Passthrough,   Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, DispatchDcs,   EscEntry),
    VT_STATE(0x1c, 0x1f, Passthrough,   Repeat),
    VT_STATE(0x20, 0x7e, Passthrough,   Repeat),
    VT_STATE(0x7f, 0x7f, Control,       Repeat),
    VT_STATE(0x80, 0x8f, Control,       Ground),
    VT_STATE(0x90, 0x90, Ignore,        DcsEntry),
    VT_STATE(0x91, 0x97, Control,       Ground),
    VT_STATE(0x98, 0x98, Ignore,        Ignore),
    VT_STATE(0x99, 0x9a, Control,       Ground),
    VT_STATE(0x9b, 0x9b, Ignore,        CsiEntry),
    VT_STATE(0x9c, 0x9c, DispatchDcs,   Ground),
    VT_STATE(0x9d, 0x9d, Ignore,        OscString),
    VT_STATE(0x9e, 0x9e, Ignore,        Ignore),
    VT_STATE(0x9f, 0x9f, Ignore,        ApcString)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(DcsIgnore)
    VT_STATE(0x00, 0x17, Ignore,        Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Ignore,        Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, Ignore,        EscEntry),
    VT_STATE(0x1c, 0x1f, Ignore,        Repeat),
    VT_STATE(0x20, 0x7f, Ignore,        Repeat),
    VT_STATE(0x80, 0x8f, Control,       Ground),
    VT_STATE(0x90, 0x90, Ignore,        DcsEntry),
    VT_STATE(0x91, 0x97, Control,       Ground),
    VT_STATE(0x98, 0x98, Ignore,        Ignore),
    VT_STATE(0x99, 0x9a, Control,       Ground),
    VT_STATE(0x9b, 0x9b, Ignore,        CsiEntry),
    VT_STATE(0x9c, 0x9c, Ignore,        Ground),
    VT_STATE(0x9d, 0x9d, Ignore,        OscString),
    VT_STATE(0x9e, 0x9e, Ignore,        Ignore),
    VT_STATE(0x9f, 0x9f, Ignore,        ApcString)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(OscString)
    VT_STATE(0x00, 0x06, Ignore,        Repeat),
    VT_STATE(0x07, 0x07, DispatchOsc,   Ground),
    VT_STATE(0x08, 0x17, Ignore,        Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Ignore,        Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, DispatchOsc,   EscEntry),
    VT_STATE(0x1c, 0x1f, Ignore,        Repeat),
    VT_STATE(0x20, 0x7f, String,        Repeat),
    VT_STATE(0x80, 0x8f, Control,       Ground),
    VT_STATE(0x90, 0x90, Ignore,        DcsEntry),
    VT_STATE(0x91, 0x97, Control,       Ground),
    VT_STATE(0x98, 0x98, Ignore,        Ignore),
    VT_STATE(0x99, 0x9a, Control,       Ground),
    VT_STATE(0x9b, 0x9b, Ignore,        CsiEntry),
    VT_STATE(0x9c, 0x9c, DispatchOsc,   Ground),
    VT_STATE(0x9d, 0x9d, Ignore,        Repeat),
    VT_STATE(0x9e, 0x9e, Ignore,        Ignore),
    VT_STATE(0x9f, 0x9f, Ignore,        ApcString),
    VT_STATE(0xa0, 0xff, String,        Repeat)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(ApcString)
    VT_STATE(0x00, 0x06, Ignore,        Repeat),
    VT_STATE(0x07, 0x07, DispatchApc,   Ground),
    VT_STATE(0x08, 0x17, Ignore,        Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Ignore,        Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, DispatchApc,   EscEntry),
    VT_STATE(0x1c, 0x1f, Ignore,        Repeat),
    VT_STATE(0x20, 0x7f, String,        Repeat),
    VT_STATE(0x80, 0x8f, Control,       Ground),
    VT_STATE(0x90, 0x90, Ignore,        DcsEntry),
    VT_STATE(0x91, 0x97, Control,       Ground),
    VT_STATE(0x98, 0x98, Ignore,        Ignore),
    VT_STATE(0x99, 0x9a, Control,       Ground),
    VT_STATE(0x9b, 0x9b, Ignore,        CsiEntry),
    VT_STATE(0x9c, 0x9c, DispatchApc,   Ground),
    VT_STATE(0x9d, 0x9d, Ignore,        Repeat),
    VT_STATE(0x9e, 0x9e, Ignore,        Ignore),
    VT_STATE(0x9f, 0x9f, Ignore,        ApcString)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(Ground)
    VT_STATE(0x00, 0x17, Control,       Repeat),
    VT_STATE(0x18, 0x18, Control,       Repeat),
    VT_STATE(0x19, 0x19, Control,       Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Repeat),
    VT_STATE(0x1b, 0x1b, Ignore,        EscEntry),
    VT_STATE(0x1c, 0x1f, Control,       Repeat),
    VT_STATE(0x20, 0x7e, Ground,        Repeat),
    VT_STATE(0x7f, 0x7f, Control,       Repeat),
    VT_STATE(0x80, 0x8f, Control,       Repeat),
    VT_STATE(0x90, 0x90, Ignore,        DcsEntry),
    VT_STATE(0x91, 0x97, Control,       Repeat),
    VT_STATE(0x98, 0x98, Ignore,        Ignore),
    VT_STATE(0x99, 0x9a, Control,       Repeat),
    VT_STATE(0x9b, 0x9b, Ignore,        CsiEntry),
    VT_STATE(0x9c, 0x9c, Ignore,        Repeat),
    VT_STATE(0x9d, 0x9d, Ignore,        OscString),
    VT_STATE(0x9e, 0x9e, Ignore,        Ignore),
    VT_STATE(0x9f, 0x9f, Ignore,        ApcString),
    VT_STATE(0xa0, 0xff, Ground,        Repeat)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(Ignore)  // SOS/PM's are currently ignored.
    VT_STATE(0x00, 0x17, Ignore,        Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Ignore,        Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, Ignore,        EscEntry),
    VT_STATE(0x1c, 0x1f, Ignore,        Repeat),
    VT_STATE(0x20, 0x7f, Ignore,        Repeat),
    VT_STATE(0x80, 0x8f, Control,       Ground),
    VT_STATE(0x90, 0x90, Ignore,        DcsEntry),
    VT_STATE(0x91, 0x97, Control,       Ground),
    VT_STATE(0x98, 0x98, Ignore,        Repeat),
    VT_STATE(0x99, 0x9a, Control,       Ground),
    VT_STATE(0x9b, 0x9b, Ignore,        CsiEntry),
    VT_STATE(0x9c, 0x9c, Ignore,        Ground),
    VT_STATE(0x9d, 0x9d, Ignore,        OscString),
    VT_STATE(0x9e, 0x9e, Ignore,        Repeat),
    VT_STATE(0x9f, 0x9f, Ignore,        ApcString)
VT_END_STATE_MAP;

#undef VT_STATE
#undef VT_BEGIN_STATE_MAP
#undef VT_END_STATE_MAP

force_inline
bool sCheckRange(int c, int lo, int hi)
{
	return dword(c - lo) < (hi - lo + 1);
}

force_inline
int sCheckSplit(const char *s, int len)
{
	for(int i = len - 1, n = 1; i >= 0; --i, ++n) {
		if(n >= 1 && ((s[i] & 0x80) == 0x00)) return len - (i + 1);
		if(n >= 2 && ((s[i] & 0xE0) == 0xC0)) return len - (i + 2);
		if(n >= 3 && ((s[i] & 0xF0) == 0xE0)) return len - (i + 3);
		if(n >= 4 && ((s[i] & 0xF8) == 0xF0)) return len - (i + 4);
	}
	return 0;
}

void VTInStream::Parse(const void *data, int size, bool utf8)
{
	String iutf8;
	utf8mode = utf8;
	
	LTIMING("VTInStream::Parse");
	
	CheckLoadData((const char*) data, size, iutf8);
		
	while(!IsEof()) {
		int c = GetChr();
		const State* st = GetState(c);
		switch(st->action) {
		case State::Action::Mode:
			sequence.mode = byte(c);
			break;
		case State::Action::Parameter:
			CollectParameter(c);
			break;
		case State::Action::Collect:
			CollectIntermediate(c);
			break;
		case State::Action::Final:
			sequence.opcode = byte(c);
			break;
		case State::Action::Control:
			WhenCtl(byte(c));
			break;
		case State::Action::Ground:
			CollectChr(c);
			break;
		case State::Action::Passthrough:
			CollectPayload(c);
			break;
		case State::Action::String:
			CollectString(c);
			break;
		case State::Action::DispatchEsc:
			sequence.opcode = byte(c);
			Dispatch(Sequence::ESC, WhenEsc);
			break;
		case State::Action::DispatchCsi:
			sequence.opcode = byte(c);
			Dispatch(Sequence::CSI, WhenCsi);
			break;
		case State::Action::DispatchDcs:
			Dispatch(Sequence::DCS, WhenDcs);
			break;
		case State::Action::DispatchOsc:
			Dispatch(Sequence::OSC, WhenOsc);
			break;
		case State::Action::DispatchApc:
			Dispatch(Sequence::APC, WhenApc);
			break;
		case State::Action::Ignore:
			break;
		default:
			NEVER();
		}
		NextState(st->next);
	}

	buffer.Clear();
	if(iutf8.GetCount())
		buffer = iutf8;
}


void VTInStream::NextState(State::Id  sid)
{
	LTIMING("VTInStream::NextState");
	
	switch(sid) {
	case State::Id::EscEntry:
		Reset0(&EscEntry);
		break;
	case State::Id::EscIntermediate:
		state = &EscIntermediate;
		break;
	case State::Id::CsiEntry:
		Reset0(&CsiEntry);
		break;
	case State::Id::CsiParameter:
		state = &CsiParameter;
		break;
	case State::Id::CsiIntermediate:
		state = &CsiIntermediate;
		break;
	case State::Id::CsiIgnore:
		state = &CsiIgnore;
		break;
	case State::Id::DcsEntry:
		Reset0(&DcsEntry);
		break;
	case State::Id::DcsParameter:
		state = &DcsParameter;
		break;
	case State::Id::DcsIntermediate:
		state = &DcsIntermediate;
		break;
	case State::Id::DcsPassthrough:
		state = &DcsPassthrough;
		break;
	case State::Id::DcsIgnore:
		state = &DcsIgnore;
		break;
	case State::Id::OscString:
		Reset0(&OscString);
		break;
	case State::Id::ApcString:
		Reset0(&ApcString);
		break;
	case State::Id::Ignore:
		Reset0(&Ignore);
		break;
	case State::Id::Repeat:
		break;
	default:
		state = &Ground;
		break;
	}
}

force_inline
const VTInStream::State* VTInStream::GetState(const int& c) const
{
	LTIMING("VTInStream::GetState");

	if(c >= 0) {
		int l = 0, r = state->GetCount() - 1;
		while(l <= r) {
			int mid = (l + r) >> 1;
			const State& st = (*state)[mid];
			if(c < st.begin)
				r = mid - 1;
			else
			if(c > st.end && st.end != 0xff)	// Allow unicode code points in ground state...
				l = mid + 1;
			else
				return &st;
		}
	}

	return &State::GetVoid();
}

force_inline
int VTInStream::GetChr()
{
	LTIMING("VtInStream::GetChr()");
	
	if(Term() < 0x80 || !utf8mode)
		return Get();
	int p = GetPos() + 1;
	int c = GetUtf8();
	if(c == -1 && !IsEof()) {
		Seek(p);
		return 0xFFFD;
	}
	return c;
}

force_inline
void VTInStream::CollectChr(int c)
{
	LTIMING("VtInStream::CollectChr()");

	int p = -1;
	while(sCheckRange(c, 0x20, 0x7E) || c > 0x9F) {
		WhenChr(c);
		p = GetPos();
		c = GetChr();
	}
	if(c != -1)
		Seek(p);
	waschr = true;
}

force_inline
void VTInStream::CollectIntermediate(int c)
{
	LTIMING("VtInStream::CollectParameter()");
	
	const byte *b = ptr;
	int   n = 0;

	sequence.intermediate[0] = c;
	while((b < rdlim) && sCheckRange(*b++, 0x20, 0x2F) && ++n < 4) {
		sequence.intermediate[n] = *b;
	}
	
	ptr += n;
}

force_inline
void VTInStream::CollectParameter(int c)
{
	LTIMING("VtInStream::CollectParameter()");
	
	const byte  *b = ptr;
	int  n = 0;
	
	while((b < rdlim) && sCheckRange(*b++, 0x30, 0x3B)) {
		++n;
	}

	collected.Cat(ptr - 1, n + 1);
	ptr += n;
}

force_inline
void VTInStream::CollectPayload(int c)
{
	LTIMING("VtInStream::CollectPayload()");

	const byte  *b = ptr;
	int  n = 0;
	
	while((b < rdlim) && (sCheckRange(c = *b++, 0x20, 0x7E) || sCheckRange(c, 0x00, 0x17) || c == 0x19)) {
		++n;
	}
	sequence.payload.Cat(ptr - 1, n + 1);
	ptr += n;
}

force_inline
void VTInStream::CollectString(int c)
{
	LTIMING("VtInStream::CollectString()");
	
	const byte  *b = ptr;
	int  n = 0;

	while((b < rdlim) && (sCheckRange(c = *b++, 0x20, 0x7F) || (utf8mode && c >= 0xA0))) {
			++n;
	}
	sequence.payload.Cat(ptr - 1, n + 1);
	ptr += n;
}

force_inline
void VTInStream::Dispatch(byte type, const Event<const VTInStream::Sequence&>& fn)
{
	LTIMING("VtInStream::CollectString()");

	switch(type) {
	case Sequence::CSI:
	case Sequence::DCS:
		sequence.parameters = pick(Split(collected, ';', false));
		break;
	case Sequence::OSC:
	case Sequence::APC:
		sequence.parameters = pick(Split(sequence.payload, ';', false));
		break;
	}
	sequence.type = type;
	fn(sequence);
	waschr = false;
}

force_inline
void VTInStream::CheckLoadData(const char *data, int size, String& err)
{
	LTIMING("VtInStream::CheckLoadData()");
	
	if(!utf8mode) {
		Create(data, size);
		return;
	}
	// Check for a possibly split UTF-8 sequence at the end of the chunk.
	int n = sCheckSplit(data, size);
	if(n > 0) {
		size -= n;
		err.Cat(data + size, n);
	}
	if(!buffer.IsEmpty()) {
		buffer.Cat(data, size);
		Create(~buffer, buffer.GetLength());
	}
	else Create(data, size);
}

void VTInStream::Reset()
{
	Reset0(&Ground);
	waschr = false;
	utf8mode = false;
}

void VTInStream::Reset0(const Vector<VTInStream::State>* st)
{
	state = st;
	sequence.Clear();
	collected.Clear();
}

VTInStream::VTInStream()
{
	Reset();
}

int VTInStream::Sequence::GetInt(int n, int d) const
{
	LTIMING("VtInStream::Dequence::GetInt()");
	
	int c = 0, i = 0;
	const char *p = parameters.Get(n - 1, String::GetVoid());
	while(*p && dword((c = *p++) - '0') < 10)
		i = i * 10 + (c - '0');
	return !i ? d : i;
}

String VTInStream::Sequence::GetStr(int n) const
{
	LTIMING("VtInStream::Dequence::GetStr()");
	
	return parameters.Get(n - 1, String::GetVoid());
}

dword VTInStream::Sequence::GetHashValue() const
{
	return Hash32(type, opcode, mode, intermediate[0], intermediate[1]);
}

void VTInStream::Sequence::Clear()
{
	type = opcode = mode = NUL;
	Zero(intermediate);
	parameters.Clear();
	payload.Clear();
}

String VTInStream::Sequence::ToString() const
{
	// Diagnostics...
	String txt;
	txt << decode(type,
			PM,  "PM  ",
			SOS, "SOS ",
			APC, "APC ",
			OSC, "OSC ",
			DCS, "DCS ",
			CSI, "CSI ", "ESC ");
	if(intermediate[0] > 0)
		txt << intermediate[0] << " ";
	if(intermediate[1] > 0)
		txt << intermediate[1] << " ";
	if(findarg(type, CSI, DCS) >= 0)
		txt << parameters.ToString();
	if(findarg(type, ESC, CSI, DCS, APC) >= 0)
		txt << AsString(opcode)  << " ";
	if(mode)
		txt << "(private) ";
	if(!IsNull(payload))
		txt	<< "Payload: " << payload.ToString();
	return txt;
}

const VTInStream::State& VTInStream::State::GetVoid()
{
	static State st(0, 0, Action::Ignore, Id::Repeat);
	return st;
}

}
