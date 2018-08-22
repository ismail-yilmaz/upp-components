#include "AnsiParserGUI.h"

String ansistr = "Hello world!\n"
                "\x1b\[7mHello world! (Inverted)\n\x1b\[0m"
                "\x1b\[31mRed \x1b\[32mGreen \x1b\[33mYellow\n\x1b\[0m"
                "\x1b\[1;31mBold \x1b\[0m\x1b\[3;32mItalic \x1b\[0m\x1b\[4;33mUnderlined\x1b\[0m "
                "\x1b\[33;105mHighlighted\x1b\[0m \x1b\[1;3;4;33;105mCombined\n\x1b\[0m"
                "\x1b\]0;This is a window title\x07";

void AnsiParserGUI::Paint(Draw& w)
{
	Size sz  = GetSize();
	auto ccx = GetCellWidth();
	auto ccy = GetCellHeight();

	w.DrawRect(sz, Black());

	for(int i = 0; i < lines.GetCount(); i++) {
		const auto& line = lines[i];
		for(int j = 0; j < line.GetCount(); j++) {
			const auto cell = line[j];
			auto x = j * ccx + 4;
			auto y = i * ccy;
			if(!IsNull(cell.highlight))
				w.DrawRect(x, y, ccx, ccy, cell.highlight);
			w.DrawText(x, y, WString(cell.chr, 1), cell.font, cell.ink);
		}
	}
}

void AnsiParserGUI::Parse(Stream& s)
{
	try {
		parser.Parse(s, [=](int c){
			if(c == '\r')
				return;
			if(lines.IsEmpty() || c == '\n') {
				lines.AddTail();
				if(c == '\n')
					return;
			}
			auto& ln = lines.Tail();
			Cell cc;
			cc.chr  = c;
			cc.font = active_cell.font;
			cc.ink  = active_cell.ink;
			cc.highlight = active_cell.highlight;
			ln.AddTail(cc);
			Refresh();
		});
	}
	catch(const AnsiParser::Error& e) {
		LOG("Parser error: " << e);
	}
}

void AnsiParserGUI::SetCellAttrs()
{
		auto fin = parser.GetTerminator();
		if(fin == 'm') {
			auto cell_attr = parser.GetParameters();
			for(const auto& attr : cell_attr) {
				switch(StrInt(attr)) {
					case 0:   active_cell = clone(default_cell);				break;
					case 1:   active_cell.font.Bold(true);						break;
					case 3:   active_cell.font.Italic(true);					break;
					case 4:   active_cell.font.Underline(true);					break;
					case 7:   Swap(active_cell.ink, active_cell.highlight);		break;
					case 21:  active_cell.font.Bold(false);						break;
					case 23:  active_cell.font.Italic(false);					break;
					case 24:  active_cell.font.Underline(false);				break;
					case 27:  Swap(active_cell.ink, active_cell.highlight);		break;
					case 30:  active_cell.ink = Black();						break;
					case 31:  active_cell.ink = Red();							break;
					case 32:  active_cell.ink = Green();						break;
					case 33:  active_cell.ink = Yellow();						break;
					case 34:  active_cell.ink = Blue();							break;
					case 35:  active_cell.ink = Magenta();						break;
					case 36:  active_cell.ink = Cyan();							break;
					case 37:  active_cell.ink = White();						break;
					case 39:  active_cell.ink = default_cell.ink;				break;
					case 90:  active_cell.ink = Black();						break;
					case 91:  active_cell.ink = LtRed();						break;
					case 92:  active_cell.ink = LtGreen();						break;
					case 93:  active_cell.ink = LtYellow();						break;
					case 94:  active_cell.ink = LtBlue();						break;
					case 95:  active_cell.ink = LtMagenta();					break;
					case 96:  active_cell.ink = LtCyan();						break;
					case 97:  active_cell.ink = LtGray();						break;
					case 40:  active_cell.highlight = Black();					break;
					case 41:  active_cell.highlight = Red();					break;
					case 42:  active_cell.highlight = Green();					break;
					case 43:  active_cell.highlight = Yellow();					break;
					case 44:  active_cell.highlight = Blue();					break;
					case 45:  active_cell.highlight = Magenta();				break;
					case 46:  active_cell.highlight = Cyan();					break;
					case 47:  active_cell.highlight = White();					break;
					case 49:  active_cell.highlight = default_cell.highlight;	break;
					case 100: active_cell.highlight = Black();					break;
					case 101: active_cell.highlight = LtRed();					break;
					case 102: active_cell.highlight = LtGreen();				break;
					case 103: active_cell.highlight = LtYellow();				break;
					case 104: active_cell.highlight = LtBlue();					break;
					case 105: active_cell.highlight = LtMagenta();				break;
					case 106: active_cell.highlight = LtCyan();					break;
					case 107: active_cell.highlight = LtGray();					break;
					default:
						if(cell_attr.IsEmpty())
							active_cell = clone(default_cell);
						LOG("Unhandled display attribute: " << StrInt(attr));
						break;
				}
			}
		}
}

void AnsiParserGUI::SetWindowTitle()
{
	Title(parser.GetParameters()[1]);
}

AnsiParserGUI::AnsiParserGUI()
{
	SetRect(0, 0, 640, 480);
	CenterScreen();
	parser.WhenCsi = THISFN(SetCellAttrs);
	parser.WhenOsc = THISFN(SetWindowTitle);
	StringStream ss(ansistr);
	Parse(ss);
	Refresh();
}

GUI_APP_MAIN
{
	AnsiParserGUI().Run();
}
