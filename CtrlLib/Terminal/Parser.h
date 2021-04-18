#ifndef _VTInStream_h_
#define _VTInStream_h_

#include <Core/Core.h>

// VTInStream: A VT500 series "lexical" parser for DEC & ANSI escape sequences.
// This parser is based on the UML state diagram provided by Paul-Flo Williams.
// See: https://vt100.net/emu/dec_ansi_parser

// Deviations from the DEC STD-070:
// 1) ISO 8613-6: 0x3a ("colon") is considered as a legitimate delimiter.
// 2) The OSC sequences allow UTF-8 payload if the UTF-8 mode is enabled.

namespace Upp {

class VTInStream : public MemReadStream {
public:
    struct Sequence {
        enum Type : byte { NUL = 0, ESC, CSI, DCS, OSC, APC, PM, SOS };
        byte            type;
        byte            opcode;
        byte            mode;
        Vector<String>  parameters;
        String          intermediate;
        String          payload;
        int             GetInt(int n, int d = 1) const;
        String          GetStr(int n) const;
        String          ToString() const;
        void            Clear();
        Sequence()                                          { Clear(); }
    };
    
    struct State : Moveable<State> {
        enum  class Id : byte {
            Ground,
            EscEntry,
            EscIntermediate,
            CsiEntry,
            CsiIntermediate,
            CsiParameter,
            CsiIgnore,
            DcsEntry,
            DcsIntermediate,
            DcsParameter,
            DcsIgnore,
            DcsPassthrough,
            OscString,
            ApcString,
            Repeat,
            Ignore
        };

        enum class Action : byte {
            Mode,
            Collect,
            Parameter,
            Final,
            Control,
            Passthrough,
            String,
            Ignore,
            Ground,
            DispatchEsc,
            DispatchCsi,
            DispatchDcs,
            DispatchOsc,
            DispatchApc
        };
 
        byte    begin;
        byte    end;
        Action  action;
        Id      next;
        
        State(byte b, byte e, Action a, Id id)
        : begin(b)
        , end(e)
        , action(a)
        , next(id)
        {
        }
    };
    
public:
    void    Parse(const void *data, int size, bool utf8);
    void    Parse(const String& data, bool utf8)            { Parse(~data, data.GetLength(), utf8); }
    void    Reset();
    bool    WasChr() const                                  { return waschr; }
        
    Event<int>  WhenChr;
    Event<byte> WhenCtl;
    Event<const VTInStream::Sequence&>  WhenEsc;
    Event<const VTInStream::Sequence&>  WhenCsi;
    Event<const VTInStream::Sequence&>  WhenDcs;
    Event<const VTInStream::Sequence&>  WhenOsc;
    Event<const VTInStream::Sequence&>  WhenApc;
    
    VTInStream();
    virtual ~VTInStream() {}

private:
    int             GetUtf8(String& iutf8);
    void            Create(const void *data, int64 size);
    void            NextState(State::Id sid);
    const State*    GetState(const int& c) const;
    void            Dispatch(byte type, const Event<const VTInStream::Sequence&>& fn);
    void            Reset0(const Vector<VTInStream::State>* st);

    Function<int()> GetChr;
    
    // Collectors.
    void            CollectChr(int c);
    void            CollectIntermediate(int c);
    void            CollectParameter(int c);
    void            CollectPayload(int c);
    void            CollectString(int c, bool utf8);
    
private:
    Sequence    sequence;
    bool        waschr;
    String      collected;
    String      buffer;
    const Vector<VTInStream::State>*  state;
};
}
#endif