#pragma once

#include <string>

#include <boost/variant.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/vector.hpp>

#include "py_parser.h"

namespace parser {

typedef boost::iterator_range<std::u32string::const_iterator> TextRange;

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
	Text,
	boost::recursive_wrapper<Block>
> Token;
typedef std::vector<Token> Tokens;

struct Block
{
	Tokens tokens;
};

struct PyExpr
{
	TextRange text;
	Block block;
};

};	// namespace parser

namespace std {

std::ostream& operator<<(std::ostream& s, std::u32string const& text);
std::ostream& operator<<(std::ostream& s, PyCode const&);
std::ostream& operator<<(std::ostream& s, PyExpr const&);
std::ostream& operator<<(std::ostream& s, NewParagraph const&);
std::ostream& operator<<(std::ostream& s, Text const&);
std::ostream& operator<<(std::ostream& s, Tokens const&);
std::ostream& operator<<(std::ostream& s, Block const&);
std::ostream& operator<<(std::ostream& s, Tokens const&);

};




BOOST_FUSION_ADAPT_STRUCT(
	parser::PyExpr,
	(parser::TextRange, text)
	(parser::Block, block)
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
	qi::rule<Iterator, void()> py_code_end;
	PyExprParser<Iterator, Skipper> _py_expr;
	qi::rule<Iterator, PyExpr(), Skipper> py_expr;
	qi::rule<Iterator, PyCode(), Skipper> py_code;
	qi::rule<Iterator, NewParagraph(), Skipper> new_paragraph;
	qi::rule<Iterator, Text(), Skipper> text;
	//qi::rule<Iterator, Token(), Skipper> token;
	qi::rule<Iterator, Tokens(), Skipper> tokens;
	qi::rule<Iterator, Block(), Skipper> block;
	qi::rule<Iterator, Tokens(), Skipper> start;
};

Tokens parse(std::u32string::const_iterator& first, std::u32string::const_iterator last);


};	// namespace parser
