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

void VTInStream::NextState(State::Id  sid)
{
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

void VTInStream::Parse(const void *data, int size, bool utf8)
{
	// TODO: Add pseudo-rentrancy.
	
	String iutf8;
	Create(data, size);
	
	LTIMING("VTInStream::Parse");

	if(utf8) GetChr = [=, &iutf8] { return GetUtf8(iutf8); };
	else     GetChr = [=]         { return Get();          };
		
	while(!IsEof()) {
		int c = GetChr();
		const State* st = GetState(c);
		if(!st)	continue;
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
			CollectString(c, utf8);
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
			sequence.payload = collected;
			Dispatch(Sequence::OSC, WhenOsc);
			break;
		case State::Action::DispatchApc:
			sequence.payload = collected;
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

	if(!iutf8.IsEmpty())
		buffer = iutf8;
}

force_inline
void VTInStream::CollectChr(int c)
{
	int p = -1;
	while((c >= 0x20 && c <= 0x7E) || c >= 0xA0) {
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
	int p = -1;
	while(c >= 0x20 && c <= 0x2F) {
		sequence.intermediate.Cat(c);
		p = GetPos();
		c = GetChr();
	}
	if(c != -1)
		Seek(p);
}

force_inline
void VTInStream::CollectParameter(int c)
{
	int p = -1;
	while(c >= 0x30 && c <= 0x3B) {
		collected.Cat(c);
		p = GetPos();
		c = GetChr();
	}
	if(c != -1)
		Seek(p);
}

force_inline
void VTInStream::CollectPayload(int c)
{
	int p = -1;
	while((c >= 0x00 && c <= 0x17) || (c >= 0x1C && c <= 0x7E) || c == 0x19) {
		sequence.payload.Cat(c);
		p = GetPos();
		c = GetChr();
	}
	if(c != -1)
		Seek(p);
}

force_inline
void VTInStream::CollectString(int c, bool utf8)
{
	 // Let us allow utf-8 sequences in OSC strings.
	 // This is illegal, according to the DEC STD-070.
	 // However, even xterm allows this on utf-8 mode.

	int p = -1;
	while((c >= 0x20 && c <= 0x7F) || (utf8 && c >= 0xA0)) {
		if(c <= 0xFF) collected.Cat(c);
		else  collected.Cat(ToUtf8(c));
		p = GetPos();
		c = GetChr();
	}
	if(c != -1)
		Seek(p);
}

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
	return nullptr;
}

void VTInStream::Dispatch(byte type, const Event<const VTInStream::Sequence&>& fn)
{
	sequence.type = type;
	sequence.parameters = pick(Split(collected, ';', false));
	fn(sequence);
	waschr = false;
}

void VTInStream::Create(const void *data, int64 size)
{
	if(buffer.IsEmpty()) {
		MemReadStream::Create(data, size);
		return;
	}
	buffer.Cat((const char*) data, size);
	MemReadStream::Create(~buffer, buffer.GetLength());
}

int VTInStream::GetUtf8(String& iutf8)
{
	// This method is a slightly modified version of Stream::GetUtf8()

	int cx = Get();
	if(cx < 0) {
		LoadError();
		return -1;
	}
	else
	if(cx < 0x80)
		return cx;
	else
	if(cx < 0xC2)
		return -1;
	else
	if(cx < 0xE0) {
		if(IsEof()) {
			iutf8.Cat(cx);
			LoadError();
			return -1;
		}
		return ((cx - 0xC0) << 6) + Get() - 0x80;
	}
	else
	if(cx < 0xF0) {
		int c0 = Get();
		int c1 = Get();
		if(c1 >= 0) {
			return ((cx - 0xE0) << 12)
				 + ((c0 - 0x80) << 6)
				 + c1 - 0x80;
		}
		else
		if(IsEof()) {
			iutf8.Cat(cx);
			if(c0 >= 0) iutf8.Cat(c0);
			LoadError();
		}
		return -1;
	}
	else
	if(cx < 0xF8) {
		int c0 = Get();
		int c1 = Get();
		int c2 = Get();
		if(c2 >= 0) {
			return ((cx - 0xf0) << 18)
				 + ((c0 - 0x80) << 12)
				 + ((c1 - 0x80) << 6)
				 + c2 - 0x80;
		}
		else
		if(IsEof()) {
			iutf8.Cat(cx);
			if(c0 >= 0) iutf8.Cat(c0);
			if(c1 >= 0) iutf8.Cat(c1);
			LoadError();
		}
		return -1;
	}
	else
	if(cx < 0xFC) {
		int c0 = Get();
		int c1 = Get();
		int c2 = Get();
		int c3 = Get();
		if(c3 >= 0) {
			return ((cx - 0xF8) << 24)
				 + ((c0 - 0x80) << 18)
				 + ((c1 - 0x80) << 12)
				 + ((c2 - 0x80) << 6)
				 + c3 - 0x80;
		}
		else
		if(IsEof()) {
			iutf8.Cat(cx);
			if(c0 >= 0) iutf8.Cat(c0);
			if(c1 >= 0) iutf8.Cat(c1);
			if(c2 >= 0) iutf8.Cat(c2);
			LoadError();
		}
		return -1;
	}
	else
	if(cx < 0xFE) {
		int c0 = Get();
		int c1 = Get();
		int c2 = Get();
		int c3 = Get();
		int c4 = Get();
		if(c4 >= 0) {
			return ((cx - 0xFC) << 30)
				 + ((c0 - 0x80) << 24)
				 + ((c1 - 0x80) << 18)
				 + ((c2 - 0x80) << 12)
				 + ((c3 - 0x80) << 6)
				 + c4 - 0x80;
		}
		else
		if(IsEof()) {
			iutf8.Cat(cx);
			if(c0 >= 0) iutf8.Cat(c0);
			if(c1 >= 0) iutf8.Cat(c1);
			if(c2 >= 0) iutf8.Cat(c2);
			if(c3 >= 0) iutf8.Cat(c3);
			LoadError();
		}
		return -1;
	}
	else {
		if(IsEof())
			LoadError();
		return -1;
	}
}

void VTInStream::Reset()
{
	Reset0(&Ground);
	waschr = false;
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
	int rc = StrInt(parameters.Get(n - 1, Null));
	return min(IsNull(rc) || rc <= 0 ? d : rc, 65536);
}

String VTInStream::Sequence::GetStr(int n) const
{
	return parameters.Get(n - 1, String::GetVoid());
}

void VTInStream::Sequence::Clear()
{
	type = opcode = mode = NUL;
	parameters.Clear();
	intermediate.Clear();
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
	if(intermediate.GetLength())
		txt << intermediate << " ";
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

}
