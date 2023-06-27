#include "HtmlTools.h"

namespace Upp {
	
int HtmlNode::FindTag(const char *_tag) const
{
	String tag = _tag;
	for(int i = 0; i < node.GetCount(); i++)
		if(node[i].type == Type::Tag && node[i].text == tag)
			return i;
	return -1;
}

HtmlNode& HtmlNode::Add(const char *tag)
{
	HtmlNode& m = node.Add();
	m.CreateTag(tag);
	return m;
}

HtmlNode& HtmlNode::GetAdd(const char *tag)
{
	int q = FindTag(tag);
	return q >= 0 ? node[q] : Add(tag);
}

const HtmlNode& HtmlNode::Void()
{
	static HtmlNode h;
	return h;
}

const HtmlNode& HtmlNode::operator[](const char *tag) const
{
	int q = FindTag(tag);
	return q < 0 ? Void() : node[q];
}

void HtmlNode::Remove(int i)
{
	node.Remove(i);
}

void HtmlNode::Remove(const char *tag)
{
	int q = FindTag(tag);
	if(q >= 0)
		node.Remove(q);
}

String HtmlNode::GatherText() const
{
	String r;
	for(int i = 0; i < GetCount(); i++)
		if(node[i].IsText())
			r << node[i].GetText();
	return r;
}

bool HtmlNode::HasTags() const
{
	for(int i = 0; i < GetCount(); i++)
		if(node[i].IsTag())
			return true;
	return false;
}

int  HtmlNode::AttrInt(const char *id, int def) const
{
	String x = Attr(id);
	CParser p(x);
	return p.IsInt() ? p.ReadInt() : def;
}

HtmlNode& HtmlNode::SetAttr(const char *id, const String& text)
{
	if(!attr)
		attr.Create();
	attr->GetAdd(id) = text;
	return *this;
}

void HtmlNode::SetAttrs(VectorMap<String, String>&& a)
{
	if(a.GetCount() == 0)
		attr.Clear();
	else {
		if(!attr)
			attr.Create();
		*attr = pick(a);
	}
}

HtmlNode& HtmlNode::SetAttr(const char *id, int i)
{
	SetAttr(id, AsString(i));
	return *this;
}

void HtmlNode::Shrink()
{
	if(attr) {
		if(attr->GetCount() == 0)
			attr.Clear();
		else
			attr->Shrink();
	}
	node.Shrink();
}

HtmlNode::HtmlNode(const HtmlNode& n, int)
{
	type = n.type;
	text = n.text;
	node = clone(n.node);
	if(n.attr) {
		attr.Create();
		*attr = clone(*n.attr);
	}
}

}
