#include "stdafx.h"
#include "py_cs_parser.h"

namespace parser {

using namespace boost::spirit;

PyCSParser::PyCSParser(): PyCSParser::base_type(start, "start")
{
	start = skip(unicode::blank)[
		lexeme[omit['\\'] > raw[NS_NAME]] >> -raw['(' >> -arglist >> ')']
	];
}

} // namespace parser
