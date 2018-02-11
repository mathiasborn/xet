#include "stdafx.h"
#include "py_literal_parser.h"
#include "xet_line_pos_iterator.h"

using namespace boost::spirit;


template <typename Iterator>
PyStrParser<Iterator>::PyStrParser(): PyStrParser::base_type(stringliteral, "stringliteral")
{
	using qi::unicode::char_;

	stringliteral = -stringprefix >> (shortstring | longstring);
	stringprefix = char_("ruRU");
	shortstring = lit('\'') >> *shortstringitem1 >> '\'' | lit('"') >> *shortstringitem2 >> '"';
	longstring = lit("'''") >> *longstringitem >> "'''" | lit("\"\"\"") >> *longstringitem >> "\"\"\"";
	shortstringitem1 = shortstringchar1 | stringescapeseq;
	shortstringitem2 = shortstringchar2 | stringescapeseq;
	longstringitem = longstringchar | stringescapeseq;
	shortstringchar1 = char_ - char_("\\'") - eol;
	shortstringchar2 = char_ - char_("\\\"") - eol;
	longstringchar = char_ - char_('\\');
	stringescapeseq = lit('\\') >> char_;

	BOOST_SPIRIT_DEBUG_NODE(stringliteral);

}

template <typename Iterator>
PyNumberParser<Iterator>::PyNumberParser(): PyNumberParser::base_type(number, "number")
{
	using qi::unicode::char_;

	integer = decinteger | bininteger | octinteger | hexinteger;
	decinteger = nonzerodigit >> *(-lit('_') >> digit) | +lit('0') >> *(-lit('_') >> '0');
	bininteger = lit('0') >> (lit('b') | 'B') >> +(-lit('_') >> bindigit);
	octinteger = lit('0') >> (lit('o') | 'O') >> +(-lit('_') >> octdigit);
	hexinteger = lit('0') >> (lit('x') | 'X') >> +(-lit('_') >> hexdigit);
	nonzerodigit = char_("1-9");
	digit = char_("0-9");
	bindigit = char_("01");
	octdigit = char_("0-7");
	hexdigit = char_("0-9a-fA-F");

	floatnumber = exponentfloat | pointfloat;
	pointfloat = -digitpart >> fraction | digitpart >> '.';
	exponentfloat = (pointfloat | digitpart) >> exponent;
	digitpart = digit >> *(-lit('_') >> digit);
	fraction = '.' >> digitpart;
	exponent = char_("eE") >> -char_("+-") >> digitpart;

	imagnumber = (floatnumber | digitpart) >> char_("jJ");

	number = imagnumber | floatnumber | integer;

	BOOST_SPIRIT_DEBUG_NODE(number);

}

// This is not really called. Its only purpose is to
// instantiate the method of the grammar.
void instantiate_parser()
{
	typedef line_pos_iterator<std::u32string::const_iterator> iterator_type;
	//typedef std::u32string::const_iterator iterator_type;
	PyStrParser<iterator_type> g1;
	PyNumberParser<iterator_type> g2;
}
