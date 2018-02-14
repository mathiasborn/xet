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

namespace py = pybind11;

namespace xet {
class Document;
}


namespace input {

std::u32string loadUTF8TextFile(fs::path const& path);

// input stream

/*
class Callable : public Token
{
public:
	Callable() {};
	Callable(Tokens&& group) : m_block{ std::move(group) } {};

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
	friend bool operator==(const Text& lhs, const Text& rhs)
	{
		return lhs.m_text == rhs.m_text;
	}

	virtual std::u32string const& text() const { return m_text; }
private:
	std::u32string m_text;
};

class ActiveToken
{
public:
	ActiveToken(PActor actor): m_actor(actor) {};
	friend bool operator==(const ActiveToken& lhs, const ActiveToken& rhs)
	{
		return lhs.m_actor == rhs.m_actor;
	}
protected:
	PActor m_actor;
};

class Glue
{
public:
	friend bool operator==(const Glue& lhs, const Glue& rhs)
	{
		return lhs.m_width == rhs.m_width && lhs.m_stretchability == rhs.m_stretchability && lhs.m_shrinkability == rhs.m_shrinkability;
	}

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

	friend bool operator==(const Penalty& lhs, const Penalty& rhs)
	{
		return lhs.m_value == rhs.m_value && lhs.m_width == rhs.m_width;
	}
private:
	int m_value;
	xet::Size m_width;
};

class ParagraphSeperator
{
public:
	friend bool operator==(const ParagraphSeperator&, const ParagraphSeperator&)
	{
		return true;
	}
};

typedef std::variant<ParagraphSeperator, Penalty, Glue, Text, ActiveToken> Token;
typedef std::vector<Token> Tokens;
typedef std::vector<Tokens> Groups;


class Actor : public boost::intrusive_ref_counter<Actor, boost::thread_unsafe_counter>
{
public:
	virtual ~Actor() {};
	virtual void addedToPage() {};
	//virtual void addedToPage(xet::PPage&) {};
	//virtual Tokens addedToTypeSetter() { return {}; }
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



//void test_xet_input(fs::path const& path);

