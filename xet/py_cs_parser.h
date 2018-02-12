#pragma once

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include "py_parser.h"
#include "xet_line_pos_iterator.h"

namespace parser {

struct PyControlSequence
{
	LinePosTextRange name;
	boost::optional<LinePosTextRange> args;
};

struct PyCSParser: qi::grammar<LinePosIterator, PyControlSequence()>, public PyCommonParser<LinePosIterator, qi::unicode::blank_type>
{
	PyCSParser();
	qi::rule<LinePosIterator, PyControlSequence()> start;
};

} // namespace parser

BOOST_FUSION_ADAPT_STRUCT(
	parser::PyControlSequence,
	(LinePosTextRange, name)
	(boost::optional<LinePosTextRange>, args)
)
