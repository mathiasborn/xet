#include "stdafx.h"
#include "xet_parser_skipper.h"
#include "xet_line_pos_iterator.h"
#include <boost/spirit/include/support.hpp>

namespace qi = boost::spirit::qi;
using namespace boost::spirit;

namespace parser {

template <typename Iterator>
XetSkipper<Iterator>::XetSkipper() : XetSkipper::base_type(skip)
{
	skip = qi::unicode::blank
		| '#' >> *(unicode::char_ - eol)
		| eol;

	skip.name("skip");
	//BOOST_SPIRIT_DEBUG_NODE(skip);
}

// This is not really called. Its only purpose is to
// instantiate the method of the grammar.
void instantiate_xet_parser_skipper()
{
	//typedef std::u32string::const_iterator iterator_type;
	typedef line_pos_iterator<std::u32string::const_iterator> iterator_type;
	XetSkipper<iterator_type> g;
}

};	// namespace parser
