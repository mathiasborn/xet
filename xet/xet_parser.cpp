#include "stdafx.h"
#include <string>
#include <fstream>
#include <sstream>

#include <boost/spirit/include/support.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/optional/optional_io.hpp>

#include "unicode_string_support.h"
#include "xet_line_pos_iterator.h"
#include "xet_parser.h"
#include "xet_parser_skipper.h"


namespace qi = boost::spirit::qi;
using namespace boost::spirit;
namespace ph = boost::phoenix;

/*
namespace std {

std::ostream& operator<<(std::ostream& s, parser::TextRange const& text)
{
	s << uts::toUtf8(std::u32string{text.begin(), text.end()});
	return s;
}


std::ostream& operator<<(std::ostream& s, std::u32string const& text)
{
	s << uts::toUtf8(text);
	return s;
}

std::ostream& operator<<(std::ostream& s, parser::PyCode const& o)
{
	s << "PyCode: \"" << o.text << '"';
	return s;
}

std::ostream& operator<<(std::ostream& s, parser::PyExpr const& o)
{
	s << "PyExpr: \"" << o.text << '"';
	return s;
}

std::ostream& operator<<(std::ostream& s, parser::NewParagraph const&)
{
	s << "NewParagraph";
	return s;
}

std::ostream& operator<<(std::ostream& s, parser::Text const& o)
{
	s << o.text;
	return s;
}

std::ostream& operator<<(std::ostream& s, parser::Token const& o)
{
	s << "(Token)";
	return s;
}

std::ostream& operator<<(std::ostream& s, parser::Tokens const& o)
{
	s << "Tokens:" << o.size();
	return s;
}

std::ostream& operator<<(std::ostream& s, parser::Block const& o)
{
	s << "Block:" << o.tokens;
	return s;
}

};
*/

namespace parser {

std::ostream& operator<<(std::ostream& s, TextRange const& text)
{
	s << uts::toUtf8(std::u32string{ text.begin(), text.end() });
	return s;
}


std::ostream& operator<<(std::ostream& s, std::u32string const& text)
{
	s << uts::toUtf8(text);
	return s;
}

std::ostream& operator<<(std::ostream& s, PyCode const& o)
{
	s << "PyCode: \"" << o.text << '"';
	return s;
}

std::ostream& operator<<(std::ostream& s, PyExpr const& o)
{
	s << "PyExpr: \"" << o.cs.name << o.cs.args << '"';
	return s;
}

std::ostream& operator<<(std::ostream& s, NewParagraph const&)
{
	s << "NewParagraph";
	return s;
}

std::ostream& operator<<(std::ostream& s, Text const& o)
{
	s << o.text;
	return s;
}

std::ostream& operator<<(std::ostream& s, Token const& o)
{
	s << "(Token)";
	return s;
}

std::ostream& operator<<(std::ostream& s, Tokens const& o)
{
	s << "Tokens:" << o.size();
	return s;
}

std::ostream& operator<<(std::ostream& s, Block const& o)
{
	s << "Block:" << o.tokens;
	return s;
}

template <typename Iterator, typename Skipper>
XetParser<Iterator, Skipper>::XetParser() : XetParser::base_type(start, "start")
{
	using qi::unicode::char_;
	using qi::unicode::graph;
	using qi::unicode::blank;
	using boost::fusion::at_c;

	start = tokens.alias();
	py_identifier = char_(L"A-Za-z_") >> *char_(L"0-9A-Za-z_");

	//py_expr = lexeme[omit['\\'] >> raw[+py_identifier]] >> *block | lexeme[omit['\\'] >> no_skip[_py_expr]] >> *block;
	py_expr = cs_parser >> *block | cs_parser >> *block;
	
	py_code_beg = lit("\\py") > '{';
	//qi::on_error<qi::fail>(py_code_beg, std::cerr << ph::val("Expected '{' at offset ") << (qi::_3 - qi::_1) << " in \"" << std::string(qi::_1, qi::_2) << '"' << std::endl);
	qi::on_error<qi::fail>(py_code_beg, [](auto const& args, auto& context, auto& r){
		std::cerr << "Expected " << at_c<3>(args) << " at offset " << (at_c<2>(args) - at_c<0>(args)) << " in \"" << std::string(at_c<0>(args), at_c<1>(args)) << '"' << std::endl;
		r = qi::fail;
	});

	py_code_end = no_skip[eol >> '}'];
	py_code = lexeme[skip[py_code_beg] > raw[*(!py_code_end >> char_)] > py_code_end];

	new_paragraph_pattern = L'\u2029';
	new_paragraph = new_paragraph_pattern > attr(ph::construct<NewParagraph>());

	text = raw[lexeme[+(char_ - char_("{}\\#") - eol - blank - new_paragraph_pattern)]] >> eps;

	tokens = *(py_code | py_expr | new_paragraph | block | text);

	block = '{' >> *(py_expr | new_paragraph | text) >> '}';

	BOOST_SPIRIT_DEBUG_NODE(py_code_beg);
	//BOOST_SPIRIT_DEBUG_NODE(py_code_end);
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
	
	typedef line_pos_iterator<std::u32string::const_iterator> Iterator;
	XetParser<Iterator, XetSkipper<Iterator>> parser;

	Tokens result;
	auto lpFirst = Iterator{first, first};
	auto lpLast = Iterator{first, last};
	auto r = qi::phrase_parse(lpFirst, lpLast, parser, XetSkipper<Iterator>(), qi::skip_flag::dont_postskip, result);

#	ifdef BOOST_SPIRIT_DEBUG
	//std::cout << "</start>" << std::endl;
	std::cout.rdbuf(strm_buffer);
#	endif

	return result;
}

/*
// This is not really called. Its only purpose is to
// instantiate the method of the grammar.
void instantiate_parser()
{
	typedef std::u32string::const_iterator iterator_type;
	XetParser<iterator_type, qi::unicode::blank_type> g;
}
*/

};	// namespace parser
