#include "Terminal.h"

// TODO: Refactor and improve key handling...

#define LLOG(x)	// RLOG("Terminal: " << x)

namespace Upp {

bool Terminal::VTKey(dword key, int count)
{
    enum {
        VTKEY_CONTROL,
        VTKEY_CURSOR,
        VTKEY_FUNCTION,
        VTKEY_NUMPAD,
        VTKEY_EDITPAD
    };

    static Tuple<dword, int, int, const char*> vtkeys[] = {
        {   K_UP,       VTKEY_CURSOR,   LEVEL_0,    "A"     },
        {   K_DOWN,     VTKEY_CURSOR,   LEVEL_0,    "B"     },
        {   K_RIGHT,    VTKEY_CURSOR,   LEVEL_0,    "C"     },
        {   K_LEFT,     VTKEY_CURSOR,   LEVEL_0,    "D"     },
        {   K_INSERT,   VTKEY_EDITPAD,  LEVEL_2,    "2~"    },
        {   K_DELETE,   VTKEY_EDITPAD,  LEVEL_2,    "3~"    },
        {   K_HOME,     VTKEY_EDITPAD,  LEVEL_2,    "1~"    },
        {   K_END,      VTKEY_EDITPAD,  LEVEL_2,    "4~"    },
        {   K_PAGEUP,   VTKEY_EDITPAD,  LEVEL_2,    "5~"    },
        {   K_PAGEDOWN, VTKEY_EDITPAD,  LEVEL_2,    "6~"    },
        {   K_MULTIPLY, VTKEY_NUMPAD,   LEVEL_0,    "j"     },
        {   K_ADD,      VTKEY_NUMPAD,   LEVEL_0,    "k"     },
        {   K_SEPARATOR,VTKEY_NUMPAD,   LEVEL_0,    "l"     },
        {   K_SUBTRACT, VTKEY_NUMPAD,   LEVEL_0,    "m"     },
        {   K_DECIMAL,  VTKEY_NUMPAD,   LEVEL_0,    "n"     },
        {   K_DIVIDE,   VTKEY_NUMPAD,   LEVEL_0,    "o"     },
        {   K_NUMPAD0,  VTKEY_NUMPAD,   LEVEL_0,    "p"     },
        {   K_NUMPAD1,  VTKEY_NUMPAD,   LEVEL_0,    "q"     },
        {   K_NUMPAD2,  VTKEY_NUMPAD,   LEVEL_0,    "r"     },
        {   K_NUMPAD3,  VTKEY_NUMPAD,   LEVEL_0,    "s"     },
        {   K_NUMPAD4,  VTKEY_NUMPAD,   LEVEL_0,    "t"     },
        {   K_NUMPAD5,  VTKEY_NUMPAD,   LEVEL_0,    "u"     },
        {   K_NUMPAD6,  VTKEY_NUMPAD,   LEVEL_0,    "v"     },
        {   K_NUMPAD7,  VTKEY_NUMPAD,   LEVEL_0,    "w"     },
        {   K_NUMPAD8,  VTKEY_NUMPAD,   LEVEL_0,    "x"     },
        {   K_NUMPAD9,  VTKEY_NUMPAD,   LEVEL_0,    "y"     },
        {   K_F1,       VTKEY_FUNCTION, LEVEL_0,    "P"     },  // PF1
        {   K_F2,       VTKEY_FUNCTION, LEVEL_0,    "Q"     },  // PF2
        {   K_F3,       VTKEY_FUNCTION, LEVEL_0,    "R"     },  // PF3
        {   K_F4,       VTKEY_FUNCTION, LEVEL_0,    "S"     },  // PF4
        {   K_F5,       VTKEY_FUNCTION, LEVEL_2,    "15~"   },
        {   K_F6,       VTKEY_FUNCTION, LEVEL_2,    "17~"   },
        {   K_F7,       VTKEY_FUNCTION, LEVEL_2,    "18~"   },
        {   K_F8,       VTKEY_FUNCTION, LEVEL_2,    "19~"   },
        {   K_F9,       VTKEY_FUNCTION, LEVEL_2,    "20~"   },
        {   K_F10,      VTKEY_FUNCTION, LEVEL_2,    "21~"   },
        {   K_F11,      VTKEY_FUNCTION, LEVEL_2,    "23~"   },
        {   K_F12,      VTKEY_FUNCTION, LEVEL_2,    "24~"   },
    };

	auto *k = FindTuple(vtkeys, __countof(vtkeys), key);
	if(k) {
			if(IsLevel0()) {	// VT52
				switch(k->b) {
				case VTKEY_CURSOR:
					PutESC(k->d, count);
					break;
				case VTKEY_NUMPAD:
					if(!modes[DECKPAM])
						return false;
					PutESC(Format("?%s", k->d), count);
					break;
				case VTKEY_FUNCTION:
					if(k->c == LEVEL_0) // VT52 has only PF1 to PF4.
						PutESC(k->d, count);
					break;
				default:
					break;
				}
			}
			else
			if(IsLevel1()) {	// VT100+
				switch(k->b) {
				case VTKEY_CURSOR:
					if(modes[DECCKM])
						PutSS3(k->d, count);
					else
						PutCSI(k->d, count);
					break;
				case VTKEY_NUMPAD:
					PutSS3(k->d, count);
					break;
				case VTKEY_EDITPAD:
					PutCSI(k->d, count);
					break;
				case VTKEY_FUNCTION:
					if(k->c < LEVEL_2)
						PutSS3(k->d, count);
					else
						PutCSI(k->d, count);
					break;
				default:
					break;
				}
			}
	}
	return k;
}

bool Terminal::UDKey(dword key, int count)
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
	bool b = GetUDKString(userkey, s);
	if(b && !IsNull(s))
		Put(s, count);
	return b;
}

bool Terminal::NavKey(dword key, int count)
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

bool Terminal::Key(dword key, int count)
{
	if(IsReadOnly() ||
		(!modes[DECARM] && count > 1))
			return MenuBar::Scan(WhenBar, key);

	dword keyflags = 0;
	
	if(UDKey(key, count))
		goto KeyAccepted;
	else
	if(NavKey(key, count))
		return true;
	else
	if(MenuBar::Scan(WhenBar, key))
		return true;
	else
	if(VTKey(key, count))
		goto KeyAccepted;

	if(key & K_KEYUP)	// We don't really need to handle key-ups...
		return true;
	
	if((key & K_ALT_KEY) == K_ALT_KEY   ||
	   (key & K_CTRL_KEY) == K_CTRL_KEY ||
	   (key & K_SHIFT_KEY) == K_SHIFT_KEY)
		return false;

	keyflags = key & (K_ALT|K_CTRL);

	switch(key &= ~(keyflags|K_SHIFT)) {
	case K_RETURN:
		PutEol();
		break;
	case K_BACKSPACE:
		key = modes[DECBKM] ? 0x08 : 0x7F;
	default:
		if(key > 65535 && !keyflags)
			return true;

		key &= ~K_DELTA;
		bool utf8 = GetCharset() == CHARSET_UTF8;
	
		key =  ConvertToCharset(key, GetLegacyCharsets().Get(key, IsLevel2()));
		if(key == DEFAULTCHAR)
			return true;
		
		if(keyflags & K_CTRL)
			key = ToAscii(key) & 0x1F;
		
		if(key < 0x80 && keyflags & K_ALT) {
			if(metakeyflags & MKEY_NONE)
				return false;
			if(metakeyflags & MKEY_SHIFT)
				key |= 0x80;
			if(metakeyflags & MKEY_ESCAPE || modes[XTALTESCM])
				utf8
					? PutESC(key, count)
					: Put(key, count);
		}
		else
			utf8
				? PutUtf8(key, count)
				: Put(key, count);
	}

KeyAccepted:
	PlaceCaret(true);
	return true;
}

Terminal& Terminal::MetaEscapesKeys(bool b)
{
	if(b)
		metakeyflags |=  MKEY_ESCAPE;
	else
		metakeyflags &= ~MKEY_ESCAPE;
	return *this;
}

Terminal& Terminal::MetaShiftsKeys(bool b)
{
	if(b)
		metakeyflags |=  MKEY_SHIFT;
	else
		metakeyflags &= ~MKEY_SHIFT;
	return *this;

}
}