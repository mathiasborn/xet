#include "stdafx.h"
#include <string>
#include <fstream>
#include <sstream>

#include <boost/spirit/include/support.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include "py_parser.h"
#include "py_literal_parser.h"

namespace qi = boost::spirit::qi;
using namespace boost::spirit;

template <typename Iterator, typename Skipper>
XetParser<Iterator, Skipper>::XetParser() : XetParser::base_type(number, "number")
{
	using qi::unicode::char_;
}
