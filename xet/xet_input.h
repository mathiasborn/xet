#pragma once

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <boost/filesystem.hpp>
#include "interfaces.h"
#include "xet_parser.h"

namespace input {

// input stream

class Token
{
public:
	virtual ~Token() {};
};

typedef std::shared_ptr<Token> PToken;
typedef std::vector<PToken> Tokens;

class Marker : public Token {};

class Text : public Token
{
public:
	Text() {};
	Text(std::u32string const& text) : m_text(text) {};

	virtual std::u32string const& text() const { return {}; }
private:
	std::u32string m_text;
};

class Callable : public Token
{
public:
	Callable() {};
	Callable(Tokens&& block) : m_block{ std::move(block) } {};

	virtual Tokens result() = 0;
protected:
	Tokens m_block;
private:
	Tokens m_result;
};

class Actor : public Token
{
public:
	virtual void addedToPage(xet::PPage&) {};
	virtual void addedToTypeSetter() {};
};

class Glue : public Token
{

private:
	xet::Size m_width;
	xet::Size m_stretchability;
	xet::Size m_shrinkability;
};

class Penalty : public Token
{
public:
	static constexpr int pinf = +1000;
	static constexpr int ninf = -1000;
private:
	int m_value;
	xet::Size m_width;
};


/*
class Text: public Token
{
std::u32string m_text;
bool m_hyphenation = false;
std::vector<std::u32string> m_parts;
};
*/

Tokens convert(parser::Tokens const&);

} // namespace input



void test_xet_input(fs::path const& path);

