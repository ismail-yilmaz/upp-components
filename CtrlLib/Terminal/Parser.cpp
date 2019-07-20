#include "Parser.h"

// VTInStream: A "lexical" parser for DEC and ANSI escape sequences in general.
// This parser is based on the UML state diagram provided by Paul-Flo Williams
// See: https://vt100.net/emu/dec_ansi_parser

#define LLOG(x)	   // RLOG("VTInStream: " << x);
#define LTIMING(x) // RTIMING(x)

namespace Upp {

#define VT_BEGIN_STATE_MAP(sname)                   \
    static Vector<VTInStream::State> sname =  {

#define VT_END_STATE_MAP                            \
    }

#define VT_STATE(begin, end, action, next)          \
    {                               \
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
    VT_STATE(0x5e, 0x5f, Ignore,        Ignore),
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
    VT_STATE(0x9e, 0x9f, Ignore,        Ignore)
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
    VT_STATE(0x9e, 0x9f, Ignore,        Ignore)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(CsiEntry)
    VT_STATE(0x00, 0x17, Control,       Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Control,       Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, Ignore,        EscEntry),
    VT_STATE(0x1c, 0x1f, Control,       Repeat),
    VT_STATE(0x20, 0x2f, Collect,       CsiIntermediate),
    VT_STATE(0x30, 0x39, Parameter,     CsiParameter),
    VT_STATE(0x3a, 0x3a, Ignore,        CsiIgnore),
    VT_STATE(0x3b, 0x3b, Parameter,     CsiParameter),
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
    VT_STATE(0x9e, 0x9f, Ignore,        Ignore)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(CsiParameter)
    VT_STATE(0x00, 0x17, Control,       Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Control,       Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, Ignore,        EscEntry),
    VT_STATE(0x1c, 0x1f, Control,       Repeat),
    VT_STATE(0x20, 0x2f, Collect,       CsiIntermediate),
    VT_STATE(0x30, 0x39, Parameter,     Repeat),
    VT_STATE(0x3a, 0x3a, Ignore,        CsiIgnore),
    VT_STATE(0x3b, 0x3b, Parameter,     Repeat),
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
    VT_STATE(0x9e, 0x9f, Ignore,        Ignore)
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
    VT_STATE(0x9e, 0x9f, Ignore,        Ignore)
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
    VT_STATE(0x9e, 0x9f, Ignore,        Ignore)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(DcsEntry)
    VT_STATE(0x00, 0x17, Ignore,        Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Ignore,        Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, Ignore,        EscEntry),
    VT_STATE(0x1c, 0x1f, Ignore,        Repeat),
    VT_STATE(0x20, 0x2f, Collect,       DcsIntermediate),
    VT_STATE(0x30, 0x39, Parameter,     DcsParameter),
    VT_STATE(0x3a, 0x3a, Ignore,        DcsIgnore),
    VT_STATE(0x3b, 0x3b, Parameter,     DcsParameter),
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
    VT_STATE(0x9e, 0x9f, Ignore,        Ignore)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(DcsParameter)
    VT_STATE(0x00, 0x17, Ignore,        Repeat),
    VT_STATE(0x18, 0x18, Control,       Ground),
    VT_STATE(0x19, 0x19, Ignore,        Repeat),
    VT_STATE(0x1a, 0x1a, Control,       Ground),
    VT_STATE(0x1b, 0x1b, Ignore,        EscEntry),
    VT_STATE(0x1c, 0x1f, Ignore,        Repeat),
    VT_STATE(0x20, 0x2f, Collect,       DcsIntermediate),
    VT_STATE(0x30, 0x39, Parameter,     Repeat),
    VT_STATE(0x3a, 0x3a, Ignore,        DcsIgnore),
    VT_STATE(0x3b, 0x3b, Parameter,     Repeat),
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
    VT_STATE(0x9e, 0x9f, Ignore,        Ignore)
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
    VT_STATE(0x9e, 0x9f, Ignore,        Ignore)
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
    VT_STATE(0x9e, 0x9f, Ignore,        Ignore)
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
    VT_STATE(0x9e, 0x9f, Ignore,        Ignore)
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
    VT_STATE(0x20, 0x7f, Parameter,     Repeat),
    VT_STATE(0x80, 0x8f, Control,       Ground),
    VT_STATE(0x90, 0x90, Ignore,        DcsEntry),
    VT_STATE(0x91, 0x97, Control,       Ground),
    VT_STATE(0x98, 0x98, Ignore,        Ignore),
    VT_STATE(0x99, 0x9a, Control,       Ground),
    VT_STATE(0x9b, 0x9b, Ignore,        CsiEntry),
    VT_STATE(0x9c, 0x9c, DispatchOsc,   Ground),
    VT_STATE(0x9d, 0x9d, Ignore,        Repeat),
    VT_STATE(0x9e, 0x9f, Ignore,        Ignore)
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
    VT_STATE(0x9e, 0x9f, Ignore,        Ignore),
    VT_STATE(0xa0, 0xff, Ground,        Repeat)
VT_END_STATE_MAP;

VT_BEGIN_STATE_MAP(Ignore)  // SOS/PM/APC's are currently ignored.
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
    VT_STATE(0x9e, 0x9f, Ignore,        Repeat)
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
	Create(data, size);

	while(!IsEof()) {
		int c = utf8 ? GetUtf8() : Get();
		const State* st;
		if(!(st = GetState(c)))
			continue;
		switch(st->action) {
		case State::Action::Mode:
			sequence.mode = (byte)c;
			break;
		case State::Action::Parameter:
			collected.Cat(c);
			break;
		case State::Action::Collect:
			sequence.intermediate.Cat(c);
			break;
		case State::Action::Final:
			sequence.opcode = (byte)c;
			break;
		case State::Action::Control:
			WhenCtl((byte)c);
			break;
		case State::Action::Ground:
			WhenChr(c);
			waschr = true;
			break;
		case State::Action::Passthrough:
			sequence.payload.Cat(c);
			break;
		case State::Action::DispatchEsc:
			sequence.opcode = (byte)c;
			Dispatch(WhenEsc);
			break;
		case State::Action::DispatchCsi:
			sequence.opcode = (byte)c;
			Dispatch(WhenCsi);
			break;
		case State::Action::DispatchDcs:
			Dispatch(WhenDcs);
			break;
		case State::Action::DispatchOsc:
			sequence.payload = collected;
			Dispatch(WhenOsc);
			break;
		case State::Action::Ignore:
			break;
		default:
			NEVER();
		}
		NextState(st->next);
	}
}

const VTInStream::State* VTInStream::GetState(const int& c) const
{
	// Using a simple binary search algorithm seems to yield higher performance on heavy loads.
	if(c >= 0) {
			
		LTIMING("VTInStream::Parse");
		int l = 0, r = state->GetCount() - 1;
		while(l <= r) {
			int mid = (l + r) >> 1;
			const State& st = (*state)[mid];
			if(c < st.begin)
				r = mid - 1;
			else
			if(c > st.end && st.end != 0xff)	// Allow bytes > 255 (unicode chars) in ground state...
				l = mid + 1;
			else
				return &st;
		}
	}
	return nullptr;
}

void VTInStream::Dispatch(const Event<const VTInStream::Sequence&>& fn)
{
	sequence.parameters = pick(Split(collected, ';', false));
	fn(sequence);
	waschr = false;
}

void VTInStream::Reset()
{
	Reset0(&Ground);
	waschr = false;
}

void VTInStream::Reset0(Vector<VTInStream::State>* st)
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
	bool b = parameters.GetCount() < max(1, n);
	return b ? d : max(Nvl(StrInt(parameters[--n]), d), d);
}

String VTInStream::Sequence::GetStr(int n) const
{
	bool b = parameters.GetCount() < max(1, n);
	return b ? String::GetVoid() : parameters[--n];
}

void VTInStream::Sequence::Clear()
{
	opcode = mode = 0;
	parameters.Clear();
	intermediate.Clear();
	payload.Clear();
}
}
