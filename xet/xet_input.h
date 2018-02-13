#pragma once

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <boost/filesystem.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <pybind11/pybind11.h>
#include "interfaces.h"
#include "xet_parser.h"
#include "xet_document.h"

namespace py = pybind11;

namespace input {

// input stream

/*
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
*/

class Actor;
typedef boost::intrusive_ptr<Actor> PActor;


class Text
{
public:
	Text() {};
	Text(std::u32string const& text) : m_text(text) {};

	virtual std::u32string const& text() const { return m_text; }
private:
	std::u32string m_text;
};

class ActiveToken
{
public:
	ActiveToken(PActor actor): m_actor(actor) {};
protected:
	PActor m_actor;
};

class Glue
{

private:
	xet::Size m_width;
	xet::Size m_stretchability;
	xet::Size m_shrinkability;
};

class Penalty
{
public:
	static constexpr int pinf = +1000;
	static constexpr int ninf = -1000;
private:
	int m_value;
	xet::Size m_width;
};

class ParagraphSeperator {};

typedef std::variant<ParagraphSeperator, Penalty, Glue, Text, ActiveToken> Token;
typedef std::vector<Token> Tokens;


class Actor : public boost::intrusive_ref_counter<Actor, boost::thread_unsafe_counter>
{
public:
	virtual ~Actor() {};
	virtual void addedToPage(xet::PPage&) {};
	virtual Tokens addedToTypeSetter() { return {}; }
};

/*
class Text: public Token
{
std::u32string m_text;
bool m_hyphenation = false;
std::vector<std::u32string> m_parts;
};
*/

Tokens convert(parser::Tokens const&, fs::path const& fileName, xet::Document& doc);

} // namespace input



void test_xet_input(fs::path const& path);

