#ifndef _AnsiParserGUI_AnsiParserGUI_h
#define _AnsiParserGUI_AnsiParserGUI_h

#include <CtrlLib/CtrlLib.h>
#include <AnsiParser/AnsiParser.h>

using namespace Upp;

class AnsiParserGUI : public TopWindow {
    struct Cell : Moveable<Cell> {
        wchar   chr;
        Font    font;
        Color   ink;
        Color   highlight;
        void    Clear()                     { chr = 0; font = Monospace(16); ink = White(); highlight = Black(); }
        Cell()                              { Clear(); }
    };
    
    Cell active_cell;
    Cell default_cell;
    AnsiParser parser;
    BiVector<BiVector<Cell>> lines;
    
    int     GetCellWidth() const            { return active_cell.font.Info().GetAveWidth(); }
    int     GetCellHeight() const           { return active_cell.font.Info().GetHeight();   }
    
    void    SetCellAttrs();
    void    SetWindowTitle();
    
public:
    void    Paint(Draw& w) override;
    void    Parse(Stream& s);
    
    typedef AnsiParserGUI CLASSNAME;
    AnsiParserGUI();
};

#endif
