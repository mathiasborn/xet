#include "stdafx.h"
#include <string>
#include <fstream>
#include <sstream>

#include <boost/spirit/include/support.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include "xet_parser.h"


namespace qi = boost::spirit::qi;
using namespace boost::spirit;
namespace ph = boost::phoenix;

namespace parser {

template <typename Iterator, typename Skipper>
XetParser<Iterator, Skipper>::XetParser() : XetParser::base_type(start, "start")
{
	using qi::unicode::char_;
	using qi::unicode::graph;

	start = tokens.alias();
	py_expr = lexeme[omit['\\'] >> raw[+graph]] >> block | lexeme[omit['\\'] >> skip[_py_expr]] >> block;
	
	py_code_beg = lit("\\py") > '{' > eol;
	py_code_end = no_skip[eol >> '}'];
	py_code = lexeme[skip[py_code_beg] > raw[*(!py_code_end >> char_)] > py_code_end];

	new_paragraph = omit[lit(L'\u2029')] > attr(ph::construct<NewParagraph>());

	text = raw[+graph] >> eps;

	tokens = *(py_code | py_expr | new_paragraph | block | text);

	block = '{' >> *(py_expr | new_paragraph | block | text) >> '}';

	BOOST_SPIRIT_DEBUG_NODE(py_code_beg);
	BOOST_SPIRIT_DEBUG_NODE(py_code_end);
	BOOST_SPIRIT_DEBUG_NODE(py_expr);
	BOOST_SPIRIT_DEBUG_NODE(py_code);
	BOOST_SPIRIT_DEBUG_NODE(new_paragraph);
	BOOST_SPIRIT_DEBUG_NODE(text);
	BOOST_SPIRIT_DEBUG_NODE(tokens);
	BOOST_SPIRIT_DEBUG_NODE(block);
	BOOST_SPIRIT_DEBUG_NODE(start);

}

Tokens parse(std::u32string::const_iterator& first, std::u32string::const_iterator last)
{
#	ifdef BOOST_SPIRIT_DEBUG
	std::streambuf* strm_buffer = std::cout.rdbuf();
	std::ofstream file("parser.xml");
	std::cout.rdbuf(file.rdbuf());
	std::cerr.tie(&std::cout);
	std::cout << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" << std::endl;// << "<start>" << std::endl;
#	endif	

	XetParser<std::u32string::const_iterator, qi::unicode::blank_type> parser;

	Tokens result;
	auto r = qi::phrase_parse(first, last, parser, unicode::blank, qi::skip_flag::dont_postskip, result);

#	ifdef BOOST_SPIRIT_DEBUG
	//std::cout << "</start>" << std::endl;
	std::cout.rdbuf(strm_buffer);
#	endif

	return result;
}


// This is not really called. Its only purpose is to
// instantiate the method of the grammar.
void instantiate_parser()
{
	typedef std::u32string::const_iterator iterator_type;
	XetParser<iterator_type, qi::unicode::blank_type> g;
}


};	// namespace parser
