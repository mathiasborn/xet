#pragma once

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>

namespace parser {

template <typename Iterator>
struct XetSkipper : qi::grammar<Iterator>
{
	XetSkipper();
	qi::rule<Iterator> skip;
};

};	// namespace parser
