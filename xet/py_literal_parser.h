#pragma once

#include "stdafx.h"
#include <string>
#include <fstream>
#include <sstream>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>

template <typename Iterator>
struct PyStrParser: qi::grammar<Iterator, void()>
{
	PyStrParser();
	
	qi::rule<Iterator, void()>
		stringliteral,
		stringprefix,
		shortstring,
		longstring,
		shortstringitem1,
		shortstringitem2,
		longstringitem,
		shortstringchar1,
		shortstringchar2,
		longstringchar,
		stringescapeseq;
};

template <typename Iterator>
struct PyNumberParser : qi::grammar<Iterator, void()>
{
	PyNumberParser();

	qi::rule<Iterator, void()>
		number,
		integer,
		decinteger,
		bininteger,
		octinteger,
		hexinteger,
		nonzerodigit,
		digit,
		bindigit,
		octdigit,
		hexdigit,
		floatnumber,
		pointfloat,
		exponentfloat,
		digitpart,
		fraction,
		exponent,
		imagnumber;
};
