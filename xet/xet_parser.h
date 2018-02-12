#pragma once

#include <string>

#include <boost/variant.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/vector.hpp>

#include "py_parser.h"
#include "py_cs_parser.h"
#include "xet_parser_skipper.h"
#include "xet_line_pos_iterator.h"

namespace parser {
typedef LinePosTextRange TextRange;
//typedef boost::iterator_range<std::u32string::const_iterator> TextRange;

struct PyCode
{
	TextRange text;
};

struct NewParagraph
{
};

struct Text
{
	TextRange text;
};

struct Block;
struct PyExpr;

typedef boost::variant<
	boost::recursive_wrapper<PyExpr>,
	PyCode,
	NewParagraph,
	Text
> Token;
typedef std::vector<Token> Tokens;

struct Block
{
	Tokens tokens;
};

struct PyExpr
{
	PyControlSequence cs;
	std::vector<Tokens> blocks;
};

std::ostream& operator<<(std::ostream& s, TextRange const& text);
std::ostream& operator<<(std::ostream& s, std::u32string const& text);
std::ostream& operator<<(std::ostream& s, PyCode const&);
std::ostream& operator<<(std::ostream& s, PyExpr const&);
std::ostream& operator<<(std::ostream& s, NewParagraph const&);
std::ostream& operator<<(std::ostream& s, Text const&);
std::ostream& operator<<(std::ostream& s, Token const&);
std::ostream& operator<<(std::ostream& s, Tokens const&);
std::ostream& operator<<(std::ostream& s, Block const&);

};	// namespace parser

namespace boost { namespace spirit { namespace traits
{
template<> struct is_container<parser::TextRange const> : mpl::false_ {};
template<> struct is_container<parser::PyCode const>: mpl::false_ {};
}}}

BOOST_FUSION_ADAPT_STRUCT(
	parser::PyExpr,
	(parser::PyControlSequence, cs)
	(std::vector<parser::Tokens>, blocks)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::PyCode,
	(parser::TextRange, text)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::Text,
	(parser::TextRange, text)
)

BOOST_FUSION_ADAPT_STRUCT(
	parser::Block,
	(parser::Tokens, tokens)
)

namespace parser {

template <typename Iterator, typename Skipper>
struct XetParser: qi::grammar<Iterator, Tokens(), Skipper>
{
	XetParser();

	qi::rule<Iterator, void(), Skipper> py_code_beg;
	qi::rule<Iterator, void()> py_code_end, py_identifier, new_paragraph_pattern;
	PyCSParser cs_parser;
	//PyExprParser<Iterator> _py_expr;
	qi::rule<Iterator, PyExpr(), Skipper> py_expr;
	qi::rule<Iterator, PyCode(), Skipper> py_code;
	qi::rule<Iterator, NewParagraph(), Skipper> new_paragraph;
	qi::rule<Iterator, Text(), Skipper> text;
	//qi::rule<Iterator, Token(), Skipper> token;
	qi::rule<Iterator, Tokens(), Skipper> tokens;
	qi::rule<Iterator, Tokens(), Skipper> block;
	qi::rule<Iterator, Tokens(), Skipper> start;
};

Tokens parse(std::u32string::const_iterator& first, std::u32string::const_iterator last);


};	// namespace parser
