#include "Terminal.h"

#define LLOG(x)	// RLOG("TerminalCtrl: " << x)

namespace Upp {

bool TerminalCtrl::ProcessKey(dword key, bool ctrlkey, bool altkey, int count)
{
	if((key = EncodeCodepoint(key, gsets.Get(key, IsLevel2()))) == DEFAULTCHAR)
		return false;

	if(ctrlkey)
		key = ToAscii(key) & 0x1F;

	if(key < 128 && altkey && metakeyflags != MKEY_NONE) {
		if(metakeyflags & MKEY_SHIFT)
			key |= 0x80;
		if((metakeyflags & MKEY_ESCAPE) || modes[XTALTESCM])
			PutESC(key, count);
		else
			Put(key, count);
	}
	else
		Put(key, count);

	return true;
}

bool TerminalCtrl::ProcessVTStyleFunctionKey(const FunctionKey& k, dword modkeys, int count)
{
	if(k.type == FunctionKey::Cursor) {
		modes[DECCKM] ? PutSS3(k.code, count) : PutCSI(k.code, count);
		return true;
	}
	else
	if(k.type == FunctionKey::EditPad) {
		PutCSI(String(k.code) << "~", count);
		return true;
	}
	else
	if(k.type == FunctionKey::NumPad && modes[DECKPAM]) {
		PutSS3(k.code, count);
		return true;
	}
	else
	if(k.type == FunctionKey::Programmable) {
		PutSS3(k.code, count);
		return true;
	}
	else
	if(k.type == FunctionKey::Function) {
		PutCSI(String(k.code) << "~", count);
		return true;
	}

	return false;
}

bool TerminalCtrl::ProcessPCStyleFunctionKey(const FunctionKey& k, dword modkeys, int count)
{
	int modifiers = 0;

	switch(modkeys) {
	case K_SHIFT:
		modifiers = 2;
		break;
	case K_ALT:
		modifiers = 3;
		break;
	case K_ALT|K_SHIFT:
		modifiers = 4;
		break;
	case K_CTRL:
		modifiers = 5;
		break;
	case K_CTRL|K_SHIFT:
		modifiers = 6;
		break;
	case K_CTRL|K_ALT:
		modifiers = 7;
		break;
	case K_SHIFT|K_ALT|K_CTRL:
		modifiers = 8;
		break;
	default:
		break;
	}

	if(modifiers) {
		if(k.type == FunctionKey::Cursor || (k.type == FunctionKey::NumPad && modes[DECKPAM])) {
			PutCSI(~Format("1;%d`%s", modifiers, k.code));
			return true;
		}
		else
		if(k.type == FunctionKey::Programmable) {
			PutCSI(Format("1;%d`%s", modifiers, k.code));
			return true;
		}
		else
		if(k.type == FunctionKey::EditPad && k.altcode) {
			PutCSI(Format("1;%d`%s", modifiers, k.altcode));
			return true;
		}
		else
		if(k.type == FunctionKey::EditPad || k.type == FunctionKey::Function) {
			PutCSI(Format("%s;%d~", k.code, modifiers));
			return true;
		}
	}
	else
	if(k.type == FunctionKey::EditPad && k.altcode) {
		PutCSI(k.altcode, count); // CSI H and CSI F
		return true;
	}

	// Basically, all other f-keys are same as in VT-style f-keys.
	return ProcessVTStyleFunctionKey(k, modkeys, count);
}

bool TerminalCtrl::VTKey(dword key, int count)
{
	const static VectorMap<dword, FunctionKey> sFunctionKeyMap = {
        { { K_UP,       }, { FunctionKey::Cursor,       LEVEL_0, "A"  } },
        { { K_DOWN,     }, { FunctionKey::Cursor,       LEVEL_0, "B"  } },
        { { K_RIGHT,    }, { FunctionKey::Cursor,       LEVEL_0, "C"  } },
        { { K_LEFT,     }, { FunctionKey::Cursor,       LEVEL_0, "D"  } },
        { { K_INSERT,   }, { FunctionKey::EditPad,      LEVEL_2, "2"  } },
        { { K_DELETE,   }, { FunctionKey::EditPad,      LEVEL_2, "3"  } },
        { { K_HOME,     }, { FunctionKey::EditPad,      LEVEL_2, "1", "H"  } },
        { { K_END,      }, { FunctionKey::EditPad,      LEVEL_2, "4", "F"  } },
        { { K_PAGEUP,   }, { FunctionKey::EditPad,      LEVEL_2, "5"  } },
        { { K_PAGEDOWN, }, { FunctionKey::EditPad,      LEVEL_2, "6"  } },
        { { K_MULTIPLY, }, { FunctionKey::NumPad,       LEVEL_0, "j"  } },
        { { K_ADD,      }, { FunctionKey::NumPad,       LEVEL_0, "k"  } },
        { { K_SEPARATOR,}, { FunctionKey::NumPad,       LEVEL_0, "l"  } },
        { { K_SUBTRACT, }, { FunctionKey::NumPad,       LEVEL_0, "m"  } },
        { { K_DECIMAL,  }, { FunctionKey::NumPad,       LEVEL_0, "n"  } },
        { { K_DIVIDE,   }, { FunctionKey::NumPad,       LEVEL_0, "o"  } },
        { { K_NUMPAD0,  }, { FunctionKey::NumPad,       LEVEL_0, "p"  } },
        { { K_NUMPAD1,  }, { FunctionKey::NumPad,       LEVEL_0, "q"  } },
        { { K_NUMPAD2,  }, { FunctionKey::NumPad,       LEVEL_0, "r"  } },
        { { K_NUMPAD3,  }, { FunctionKey::NumPad,       LEVEL_0, "s"  } },
        { { K_NUMPAD4,  }, { FunctionKey::NumPad,       LEVEL_0, "t"  } },
        { { K_NUMPAD5,  }, { FunctionKey::NumPad,       LEVEL_0, "u"  } },
        { { K_NUMPAD6,  }, { FunctionKey::NumPad,       LEVEL_0, "v"  } },
        { { K_NUMPAD7,  }, { FunctionKey::NumPad,       LEVEL_0, "w"  } },
        { { K_NUMPAD8,  }, { FunctionKey::NumPad,       LEVEL_0, "x"  } },
        { { K_NUMPAD9,  }, { FunctionKey::NumPad,       LEVEL_0, "y"  } },
        { { K_F1,       }, { FunctionKey::Programmable, LEVEL_0, "P"  } },  // PF1
        { { K_F2,       }, { FunctionKey::Programmable, LEVEL_0, "Q"  } },  // PF2
        { { K_F3,       }, { FunctionKey::Programmable, LEVEL_0, "R"  } },  // PF3
        { { K_F4,       }, { FunctionKey::Programmable, LEVEL_0, "S"  } },  // PF4
        { { K_F5,       }, { FunctionKey::Function,     LEVEL_2, "15" } },
        { { K_F6,       }, { FunctionKey::Function,     LEVEL_2, "17" } },
        { { K_F7,       }, { FunctionKey::Function,     LEVEL_2, "18" } },
        { { K_F8,       }, { FunctionKey::Function,     LEVEL_2, "19" } },
        { { K_F9,       }, { FunctionKey::Function,     LEVEL_2, "20" } },
        { { K_F10,      }, { FunctionKey::Function,     LEVEL_2, "21" } },
        { { K_F11,      }, { FunctionKey::Function,     LEVEL_2, "23" } },
        { { K_F12,      }, { FunctionKey::Function,     LEVEL_2, "24" } },
        { { K_CTRL_F1,  }, { FunctionKey::Function,     LEVEL_2, "25" } },  // In VT-key mode: F13
        { { K_CTRL_F2,  }, { FunctionKey::Function,     LEVEL_2, "26" } },  // In VT-key mode: F14
        { { K_CTRL_F3,  }, { FunctionKey::Function,     LEVEL_2, "28" } },  // In VT-key mode: F15
        { { K_CTRL_F4,  }, { FunctionKey::Function,     LEVEL_2, "29" } },  // In VT-key mode: F16
        { { K_CTRL_F5,  }, { FunctionKey::Function,     LEVEL_2, "31" } },  // In VT-key mode: F17
        { { K_CTRL_F6,  }, { FunctionKey::Function,     LEVEL_2, "32" } },  // In VT-key mode: F18
        { { K_CTRL_F7,  }, { FunctionKey::Function,     LEVEL_2, "33" } },  // In VT-key mode: F19
        { { K_CTRL_F8,  }, { FunctionKey::Function,     LEVEL_2, "34" } },  // In VT-key mode: F20
	};

	dword keymask = K_SHIFT|K_ALT|(pcstylefunctionkeys * K_CTRL);

	int i = sFunctionKeyMap.Find(key & ~keymask);
	if(i < 0)
		return false;
	
	const FunctionKey& k = sFunctionKeyMap[i];
	if(k.level > clevel)
		return false;
	
	if(IsLevel0()) { // VT52
			if(k.type == FunctionKey::Cursor || k.type == FunctionKey::Programmable) {
				PutESC(k.code, count);
				return true;
			}
			else
			if(k.type == FunctionKey::NumPad && modes[DECKPAM]) {
				PutESC(String("?") << k.code, count);
				return true;
			}
	}
	else
	if(IsLevel1()) { // ANSI/PC
		if(pcstylefunctionkeys)
			return ProcessPCStyleFunctionKey(k, key & keymask, count);
		else
			return ProcessVTStyleFunctionKey(k, key & keymask, count);
	}

	return false;
}

bool TerminalCtrl::UDKey(dword key, int count)
{
	if(!HasUDK())
		return false;

	byte userkey  = 0;

	// DEC user-defined keys (DECUDK) support
	
	enum UserDefinedKeys : byte
	{
		UDK_F1      = 11,
		UDK_F2      = 12,
		UDK_F3      = 13,
		UDK_F4      = 14,
		UDK_F5      = 15,
		UDK_F6      = 17,
		UDK_F7      = 18,
		UDK_F8      = 19,
		UDK_F9      = 20,
		UDK_F10     = 21,
		UDK_F11     = 23,
		UDK_F12     = 24,
		UDK_ALT     = 64,
		UDK_SHIFT   = 128
	};
		
//	if(key & K_ALT)
//		userkey |= UDK_ALT;
//	if(key & K_SHIFT)
//		userkey |= UDK_SHIFT;

	switch(key) {
	case K_SHIFT_F1:
		userkey |= UDK_F1;
		break;
	case K_SHIFT_F2:
		userkey |= UDK_F2;
		break;
	case K_SHIFT_F3:
		userkey |= UDK_F3;
		break;
	case K_SHIFT_F4:
		userkey |= UDK_F4;
		break;
	case K_SHIFT_F5:
		userkey |= UDK_F5;
		break;
	case K_SHIFT_F6:
		userkey |= UDK_F6;
		break;
	case K_SHIFT_F7:
		userkey |= UDK_F7;
		break;
	case K_SHIFT_F8:
		userkey |= UDK_F8;
		break;
	case K_SHIFT_F9:
		userkey |= UDK_F9;
		break;
	case K_SHIFT_F10:
		userkey |= UDK_F10;
		break;
	case K_SHIFT_F11:
		userkey |= UDK_F11;
		break;
	case K_SHIFT_F12:
		userkey |= UDK_F12;
		break;
	default:
		return false;
	}
	
	String s;
	if(GetUDKString(userkey, s)) {
		Put(s.ToWString(), count);
		return true;
	}

	return false;
}

bool TerminalCtrl::NavKey(dword key, int count)
{
	if(!keynavigation)
		return false;
	
	switch(key) {
	case K_SHIFT_CTRL_UP:
		sb.PrevLine();
		break;
	case K_SHIFT_CTRL_DOWN:
		sb.NextLine();
		break;;
	case K_SHIFT_CTRL_PAGEUP:
		sb.PrevPage();
		break;
	case K_SHIFT_CTRL_PAGEDOWN:
		sb.NextPage();
		break;
	case K_SHIFT_CTRL_HOME:
		sb.ScrollInto(0);
		break;
	case K_SHIFT_CTRL_END:
		sb.End();
		break;
	default:
		return false;
	}
	return true;
}

bool TerminalCtrl::Key(dword key, int count)
{
	if(IsReadOnly()	|| (!modes[DECARM] && count > 1))
		return MenuBar::Scan(WhenBar, key);

	bool ctrlkey  = key & K_CTRL;
	bool altkey   = key & K_ALT;
	bool shiftkey = key & K_SHIFT;
	
	if(UDKey(key, count))
		goto KeyAccepted;
	else
	if(NavKey(key, count))
		goto End;
	else
	if(MenuBar::Scan(WhenBar, key))
		return true;

	if(key & K_KEYUP)	// We don't really need to handle key-ups...
		return true;

#ifdef PLATFORM_COCOA
	if(findarg(key & ~(K_CTRL|K_ALT|K_SHIFT|K_OPTION), K_CTRL_KEY, K_ALT_KEY, K_SHIFT_KEY, K_OPTION_KEY) >= 0)
		return true;
	key &= ~K_OPTION;
#else
	if(findarg(key & ~(K_CTRL|K_ALT|K_SHIFT), K_CTRL_KEY, K_ALT_KEY, K_SHIFT_KEY) >= 0)
		return true;
#endif

	if(key == K_RETURN) {
		PutEol();
	}
	else {
		// Handle character.
		if(!shiftkey && key >= ' ' && key < 65536) {
			if(!ProcessKey(key, ctrlkey, altkey, count))
				return false;
		}
		else {
			// Handle control key (including information separators).
			switch(key & ~(K_ALT|K_SHIFT)) {
			case K_BREAK:
				key = 0x03;
				break;
			case K_BACKSPACE:
				key = modes[DECBKM] ? 0x08 : 0x7F;
				break;
			case K_ESCAPE:
				key = 0x1B;
				break;
			case K_TAB:
				key = 0x09;
				break;
//			case K_DELETE:
//				key = 0x7F;
//				break;
			case K_CTRL_LBRACKET:
				key = '[';
				break;
			case K_CTRL_RBRACKET:
				key = ']';
				break;
			case K_CTRL_MINUS:
				key = '-';
				break;
			case K_CTRL_GRAVE:
				key = '`';
				break;
			case K_CTRL_SLASH:
				key = '/';
				break;
			case K_CTRL_BACKSLASH:
				key = '\\';
				break;
			case K_CTRL_COMMA:
				key = ',';
				break;
			case K_CTRL_PERIOD:
				key = '.';
				break;
			#ifndef PLATFORM_WIN32 // U++ ctrl + period and ctrl + semicolon enumeratos have the same value on Windows (a bug?)
			case K_CTRL_SEMICOLON:
				key = ';';
				break;
			#endif
			case K_CTRL_EQUAL:
				key = '=';
				break;
			case K_CTRL_APOSTROPHE:
				key = '\'';
				break;
			default:
				if(VTKey(key, count))
					goto KeyAccepted;
				if(ctrlkey || altkey) {
					key &= ~(K_CTRL|K_ALT|K_SHIFT);
					if(key >= K_A && key <= K_Z) {
						key = 'a' + (key - K_A);
					}
					else
					if(key == K_2) {
						key = '@';
					}
					else
					if(key >= K_3 && key <= K_8) {
						key = '[' + (key - K_3);
					}
					else
					if(key < K_DELTA + 65535) {
						key &= ~K_DELTA;
					}
				}
			}
			if(key > K_DELTA || !ProcessKey(key, ctrlkey, altkey, count))
				return false;
		}
	}

KeyAccepted:
	PlaceCaret(true);

End:
	if(hidemousecursor)
		mousehidden = true;
	
	return true;
}
}
