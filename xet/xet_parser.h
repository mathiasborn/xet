#pragma once

#include <string>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>

template <typename Iterator, typename Skipper>
struct XetParser: qi::grammar<Iterator, void(), Skipper>
{
	XetParser();
};
