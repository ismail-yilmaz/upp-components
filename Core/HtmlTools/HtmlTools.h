#ifndef _HtmlTools_HtmlTools_h_
#define _HtmlTools_HtmlTools_h_

#include <Core/Core.h>

#include "libtidy/tidy.h"
#include "libtidy/tidybuffio.h"

namespace Upp {

class HtmlNode : Moveable< HtmlNode, DeepCopyOption<HtmlNode> > {
public:
    enum class Type {
        Root,
        DocType,
        Tag,
        Text,
        Decl,
        PI,
        Comment,
        CData,
        Asp,
        Php,
        Jste,
        XmlSect,
        XmlDecl
    };
    
    static const HtmlNode& Void();
    bool           IsVoid() const                              { return this == &Void(); }

    Type            GetType() const                            { return type; }
    String          GetText() const                            { return text; }
    String          GetTag() const                             { return text; }
    bool            IsTag() const                              { return type == Type::Tag; }
    bool            IsTag(const char *tag) const               { return IsTag() && text == tag; }
    bool            IsText() const                             { return type == Type::Text; }

    void            Clear()                                    { text.Clear(); attr.Clear(); node.Clear(); type = Type::Root; }
    void            CreateTag(const char *tag)                 { type = Type::Tag; text = tag; }
    void            CreateText(const String& txt)              { type = Type::Text; text = txt; }
    void            CreatePI(const String& pi)                 { type = Type::PI; text = pi; }
    void            CreateDecl(const String& decl)             { type = Type::Decl; text = decl; }
    void            CreateComment(const String& comment)       { type = Type::Comment; text = comment; }
    void            CreateCDATA(const String& cdata)           { type = Type::CData; text = cdata; }
    void            CreateASP(const String& asp)               { type = Type::Asp; text = asp; }
    void            CreateJSTE(const String& jste)             { type = Type::Jste; text = jste; }
    void            CreatePHP(const String& php)               { type = Type::Php; text = php; }
    void            CreateXMLDecl(const String& xmldecl)       { type = Type::XmlDecl; text = xmldecl; }
    void            CreateXMLSect(const String& xmlsect)       { type = Type::XmlSect; text = xmlsect; }
    void            CreateDocType(const String& doctype)       { type = Type::DocType; text = doctype; }
    void            CreateRoot()                               { Clear(); type = Type::Root; }
    bool            IsEmpty() const                            { return type == Type::Root && node.GetCount() == 0; }
    operator bool() const                                      { return !IsEmpty(); }

    int             GetCount() const                           { return node.GetCount(); }
    HtmlNode&       At(int i)                                  { return node.At(i); }
    const HtmlNode& Node(int i) const                          { return node[i]; }
    const HtmlNode& operator[](int i) const                    { return i >= 0 && i < node.GetCount() ? node[i] : Void(); }
    const HtmlNode& operator[](const char *tag) const;
    HtmlNode&       Add()                                      { return node.Add(); }
    void            Remove(int i);
    void            AddText(const String& txt)                 { Add().CreateText(txt); }
    int             FindTag(const char *tag) const;
    HtmlNode&       Add(const char *tag);
    HtmlNode&       GetAdd(const char *tag);
    HtmlNode&       operator()(const char *tag)                { return GetAdd(tag); }
    void            Remove(const char *tag);

    String          GatherText() const;
    String          operator~() const                          { return GatherText(); }
    bool            HasTags() const;

    int             GetAttrCount() const                       { return attr ? attr->GetCount() : 0; }
    String          AttrId(int i) const                        { return attr->GetKey(i); }
    String          Attr(int i) const                          { return (*attr)[i]; }
    String          Attr(const char *id) const                 { return attr ? attr->Get(id, Null) : String(); }
    HtmlNode&       SetAttr(const char *id, const String& val);
    int             AttrInt(const char *id, int def = Null) const;
    HtmlNode&       SetAttr(const char *id, int val);
    bool            HasAttrs() const                           { return (bool) GetAttrCount(); }

    void            SetAttrs(VectorMap<String, String>&& a);
    
    void            Shrink();
    
    rval_default(HtmlNode);

    HtmlNode(const HtmlNode& n, int);
    
    HtmlNode()                                                 { type = Type::Root; }

    typedef Array<HtmlNode>::ConstIterator ConstIterator;
    ConstIterator           Begin() const                      { return node.Begin(); }
    ConstIterator           End() const                        { return node.End(); }

    typedef HtmlNode        value_type;
    typedef ConstIterator   const_iterator;
    typedef const HtmlNode& const_reference;
    typedef int             size_type;
    typedef int             difference_type;
    const_iterator          begin() const                      { return Begin(); }
    const_iterator          end() const                        { return End(); }
    
private:
    Type                             type;
    String                           text;
    Array<HtmlNode>                  node;
    One< VectorMap<String, String> > attr;

};

// Upp-libtidy encapsulation

class TidyHtmlParser {
public:
    TidyHtmlParser(const String& html);
    virtual ~TidyHtmlParser();
    
    class Node {
        friend class TidyHtmlParser;
        
    public:
        TidyNodeType GetType() const        { return tidyNodeGetType(self); }
        TidyTagId    GetId() const          { return tidyNodeGetId(self);   }
        String       GetName() const        { return tidyNodeGetName(self); }
        Node         GetParent() const      { return { doc, tidyGetParent(self) }; }
        Node         GetChild() const       { return { doc, tidyGetChild(self) };  }
        Node         GetNext() const        { return { doc, tidyGetNext(self) };   }
        Node         GetPrev() const        { return { doc, tidyGetPrev(self) };   }
        bool         IsHeader() const       { return tidyNodeIsHeader(self); }
        bool         IsPropriety() const    { return tidyNodeIsProp(doc, self); }
        bool         IsText() const         { return tidyNodeIsText(self); }
        bool         HasText() const        { return tidyNodeHasText(doc, self); }
        String       GetText() const;
        String       GetValue() const;
        int          GetLine() const        { return tidyNodeLine(self);   }
        int          GetColumn() const      { return tidyNodeColumn(self); }
        Point        GetPos() const         { return Point(GetColumn(), GetLine()); }
        operator     bool() const           { return (bool) self; }
        
        class Attr {
            friend class Node;
            
        public:
            Attr     GetNext() const        { return { node, tidyAttrNext(self) }; }
            String   GetKey() const         { return tidyAttrName(self); }
            String   GetValue() const       { return tidyAttrValue(self); }
            bool     IsEvent() const        { return tidyAttrIsEvent(self); }
            operator bool() const           { return (bool) self; }

            Attr() = delete;
                
        private:
            Attr(TidyNode node_, TidyAttr attr_);
            TidyNode node;
            TidyAttr self;
        };
    
        Attr		GetFirstAttr() const    { return { self, tidyAttrFirst(self) }; }
        bool		HasAttrs() const        { return (bool) tidyAttrFirst(self); }
        
        HtmlNode	ToHtmlNode() const;
        operator    HtmlNode() const		{ return ToHtmlNode(); }
        
        Node() = delete;

    private:
        Node(TidyDoc doc_, TidyNode node_);
        TidyDoc  doc;
        TidyNode self;
    };

    TidyHtmlParser& SetOption(TidyOptionId id, const Value& value);
    TidyHtmlParser& SetOption(const String& id, const Value& value);

    Node        GetRoot() const                 { return { doc, tidyGetRoot(doc) }; }
    Node        GetHtml() const                 { return { doc, tidyGetHtml(doc) }; }
    Node        GetHead() const                 { return { doc, tidyGetHead(doc) }; }
    Node        GetBody() const                 { return { doc, tidyGetBody(doc) }; }

    int         Parse();

    TidyDoc     GetTidyDoc() const              { return doc; }

    int         GetWarningCount() const         { return tidyWarningCount(doc); }
    bool        HasWarnings() const             { return GetWarningCount() > 0; }
 
    int         GetErrorCount() const           { return tidyErrorCount(doc);   }
    bool        HasErrors() const               { return GetErrorCount() > 0;   }
    
private:
    
    TidyDoc       doc;
    TidyBuffer    errors;
    const String& htmlsource;
};

HtmlNode ParseHtml(const String& html, const VectorMap<String, Value>& options);
String   RepairHtml(const String& html, const VectorMap<String, Value>& options);
//String   ReportHtml(const String& html, const VectorMap<String, Value>& options);

}

#endif
