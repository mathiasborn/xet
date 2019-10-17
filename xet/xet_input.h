#pragma once

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <functional>
#include <boost/filesystem.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <pybind11/pybind11.h>
#include "interfaces.h"
#include "xet_parser.h"

namespace py = pybind11;

namespace xet {
class Document;
typedef boost::intrusive_ptr<Document> PDocument;
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
	operator std::u32string() const;
protected:
	PActor m_actor;
};

class Glue
{
public:
	Glue() {};
	Glue(xet::Size width, xet::Size stretchability, xet::Size shrinkability):
		m_width(width), m_stretchability(stretchability), m_shrinkability(shrinkability) {};
	friend bool operator==(const Glue& lhs, const Glue& rhs)
	{
		return lhs.m_width == rhs.m_width && lhs.m_stretchability == rhs.m_stretchability && lhs.m_shrinkability == rhs.m_shrinkability;
	}
	operator std::u32string() const;

private:
	xet::Size m_width = 0;
	xet::Size m_stretchability = 0;
	xet::Size m_shrinkability = 0;
};

class Penalty
{
public:
	static constexpr int pinf = +1000;
	static constexpr int ninf = -1000;

	Penalty(int value, xet::Size width): m_value(value), m_width(width) {};

	friend bool operator==(const Penalty& lhs, const Penalty& rhs)
	{
		return lhs.m_value == rhs.m_value && lhs.m_width == rhs.m_width;
	}
	operator std::u32string() const;
private:
	int m_value;
	xet::Size m_width;
};

class ParagraphSeparator
{
public:
	friend bool operator==(const ParagraphSeparator&, const ParagraphSeparator&)
	{
		return true;
	}
	operator std::u32string() const;
};

class Push
{
public:
	friend bool operator==(const Push&, const Push&)
	{
		return true;
	}
	operator std::u32string() const;
};

class Pop
{
public:
	friend bool operator==(const Pop&, const Pop&)
	{
		return true;
	}
	operator std::u32string() const;
};

class Stream
{
public:
	Stream(int32_t n = 0): m_n(n) {};
	friend bool operator==(const Stream& lhs, const Stream& rhs)
	{
		return lhs.m_n == rhs.m_n;
	}
	operator std::u32string() const;
private:
	int32_t m_n;
};

class InitialPage
{
public:
	InitialPage(std::function<py::object(xet::PDocument)> const& factory): m_factory(factory) {};
	friend bool operator==(const InitialPage& lhs, const InitialPage& rhs)
	{
		return lhs.m_factory.target<xet::PPage(xet::PDocument)>() == rhs.m_factory.target<xet::PPage(xet::PDocument)>();
	}
	operator std::u32string() const;
	xet::PPage operator()(xet::PDocument doc);
private:
	std::function<py::object(xet::PDocument)> m_factory;
};




typedef std::variant<ParagraphSeparator, Penalty, Glue, Text, ActiveToken, Push, Pop, Stream, InitialPage> Token;
typedef std::vector<Token> Tokens;
typedef std::shared_ptr<Tokens> PTokens;
typedef std::vector<PTokens> Groups;
//typedef std::shared_ptr<Groups> PGroups;

class Actor: public PyObjectHolder<Actor>
{
public:
	virtual ~Actor() {};
	virtual void addedToPage(xet::PPage) {};
	virtual void addedToTypeSetter(xet::PTypeSetter) {};
};

/*
class Text: public Token
{
std::u32string m_text;
bool m_hyphenation = false;
std::vector<std::u32string> m_parts;
};
*/

PTokens convert(parser::Tokens const&, fs::path const& fileName, xet::Document& doc);

} // namespace input



//void test_xet_input(fs::path const& path);

