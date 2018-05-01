#include "AnsiParser.h"

namespace Upp {
void AnsiParser::Reset()
{
	ctl   = -1;
	fin   = -1;
	mode  = Mode::PLAIN;
	type  = Type::PARAMETER;
	parameters   = Null;
	intermediate = Null;
}

auto AnsiParser::GetType(int c) -> Type
{
	return IsControl(c)
		? Type::CONTROL
		: IsParameter(c)
			? Type::PARAMETER
			: IsIntermediate(c)
				? Type::INTERMEDIATE
				: IsAlphabetic(c)
					? Type::ALPHABETIC
					: Type::INVALID;
}

void AnsiParser::Parse(Stream& in, Event<int> out)
{
	while(!in.IsEof()) {
		auto c = in.Get();
		if(c < 0)
			break;
		c = Parse(c);
		if(c != -1)
			out(c);
	}
	if(in.IsError())
		throw Error(in.GetErrorText());
}

int AnsiParser::Parse(int c)
{
	switch(mode) {
		case Mode::PLAIN:
			if(!IsEsc(c))
				return c;
			mode = Mode::ESCAPE;
			break;
		case Mode::ESCAPE:
			if(ctl < 0) {
				ctl = c;
				if(!IsCmd(c))
					type = GetType(c);
			}
			else
			if(IsCmd(ctl)) {
				IsCsi(ctl)
					? ParseCsi(c)
					: ParseOsc(c);
				break;
			}
			if(!IsCmd(ctl))
				ParseEsc(c);
			break;
		default:
			Reset();
			throw Error("Parser error.");
	}
	return -1;
}

void AnsiParser::ParseCsi(int c)
{
	switch(type) {
		case Type::PARAMETER:
			if(IsParameter(c)) {
				parameters.Cat(c);
				break;
			}
			type = Type::INTERMEDIATE;
		case Type::INTERMEDIATE:
			if(IsIntermediate(c)) {
				intermediate.Cat(c);
				break;
			}
			type = Type::ALPHABETIC;
		case Type::ALPHABETIC:
			if(IsAlphabetic(c)) {
				fin = c;
				WhenCsi();
				Reset();
				break;
			}
		default:
			Reset();
			throw Error("Malformed command sequence.");
	}
}

void AnsiParser::ParseOsc(int c)
{
	if(c == 0x07 || c == 0x9c) {
		WhenOsc();
		Reset();
	}
	else
		parameters.Cat(c);
}

void AnsiParser::ParseEsc(int c)
{
	bool err = false;

	switch(type) {
		case Type::INTERMEDIATE:
			if(IsIntermediate(c))
				intermediate.Cat(c);
			else
			if(IsParameter(c) || IsAlphabetic(c)) {
				fin = c;
				WhenEsc();
				Reset();
			}
			else
				err = true;
			break;
		case Type::CONTROL:
		case Type::ALPHABETIC:
		case Type::PARAMETER:
			WhenEsc();
			Reset();
			break;
		default:
			err = true;
			break;
	}
	if(err) {
		Reset();
		throw Error("Malformed escape sequence");
	}
}
}
