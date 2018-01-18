#include "stdafx.h"
#include <string>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
using namespace boost::spirit;

// http://stackoverflow.com/questions/10474571/how-to-match-unicode-characters-with-boostspirit
// http://boost-spirit.com/home/wp-content/uploads/2010/05/A_Framework_for_RAD_Spirit.pdf
// http://www.boost.org/doc/libs/1_62_0/libs/regex/doc/html/boost_regex/ref/internal_details/uni_iter.html
// http://www.nubaria.com/en/blog/?p=289
// http://www.cprogramming.com/tutorial/unicode.html

// This is needed to make qi::phrase_parse work with ustring::const_iterator.
namespace boost {
template<> struct is_scalar<std::u32string::const_iterator> : public true_type {};
}

template <typename Iterator>
struct PyCommonParser
{
public:
	PyCommonParser()
	{
		using qi::unicode::char_;
		using qi::unicode::blank;
		using qi::int_;
		using qi::no_case;
/*
		NAME = lexeme[+char_(L"0-9A-Za-z_")];
		NEWLINE = eol;
		ASYNC = "async";
		AWAIT = "await";
		INDENT = eps;
		DEDENT = eps;

		single_input = NEWLINE | simple_stmt | compound_stmt >> NEWLINE;
		file_input = *(NEWLINE | stmt) >> eoi;
		eval_input = testlist >> *NEWLINE >> eoi;

		decorator = '@' >> dotted_name >> -('(' >> -arglist >> ')') >> NEWLINE;
		decorators = +decorator;
		decorated = decorators >> (classdef | funcdef | async_funcdef);

		async_funcdef = ASYNC >> funcdef;
		funcdef = "def" >> NAME >> parameters >> -("->" >> test) >> ':' >> suite;

		parameters = '(' >> -(typedargslist) >> ')';
		typedargslist = tfpdef >> -('=' >> test) >> *(',' >> tfpdef >> -('=' >> test)) >>
			-(',' >> -('*' >> -tfpdef >> *(',' >> tfpdef >> -('=' >> test)) >> -(',' >> -("**" >> tfpdef >> -lit(','))) | "**" >> tfpdef >> -lit(','))) |
			'*' >> -tfpdef >> *(',' >> tfpdef >> -('=' >> test)) >> -(',' >> -("**" >> tfpdef >> -lit(','))) |
			"**" >> tfpdef >> -lit(',');
		tfpdef = NAME >> -(':' >> test);

		varargslist = vfpdef >> -('=' >> test) >> *(',' >> vfpdef >> -('=' >> test)) >>
			-(',' >> -('*' >> -vfpdef >> *(',' >> vfpdef >> -('=' >> test)) >> -(',' >> -("**" >> vfpdef >> -lit(','))) | "**" >> vfpdef >> -lit(','))) |
		'*' >> -vfpdef >> *(',' >> vfpdef >> -('=' >> test)) >> -(',' >> -("**" >> vfpdef >> -lit(','))) |
		"**" >> vfpdef >> -lit(',');

		vfpdef = NAME;

		stmt = simple_stmt | compound_stmt;
		simple_stmt = small_stmt >> *(';' >> small_stmt) >> -lit(';') >> NEWLINE;
		small_stmt = expr_stmt | del_stmt | pass_stmt | flow_stmt | import_stmt | global_stmt | nonlocal_stmt | assert_stmt;
		expr_stmt = testlist_star_expr >> (annassign | augassign >> (yield_expr | testlist) | *('=' >> (yield_expr | testlist_star_expr)));
		annassign = ':' >> test >> -('=' >> test);
		
		testlist_star_expr = (test | star_expr) >> *(',' >> (test | star_expr)) >> -lit(',');
		augassign = lit("+=") | "-=" | "*=" | "@=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>=" | "**=" | "//=";
		del_stmt = "del" >> exprlist;
		pass_stmt = "pass";
		flow_stmt = break_stmt | continue_stmt | return_stmt | raise_stmt | yield_stmt;
		break_stmt = "break";
		continue_stmt = "continue";
		return_stmt = "return" >> -testlist;
		yield_stmt = yield_expr;
		raise_stmt = "raise" >> -(test >> -("from" >> test));
		import_stmt = import_name | import_from;
		import_name = "import" >> dotted_as_names;
		import_from = "from" >> (*(lit('.') | "...") >> dotted_name | +(lit('.') | "...")) >> "import" >> ('*' | '(' >> import_as_names >> ')' | import_as_names);
		import_as_name = NAME >> -("as" >> NAME);
		dotted_as_name = dotted_name >> -("as" >> NAME);
		import_as_names = import_as_name >> *(',' >> import_as_name) >> -lit(',');
		dotted_as_names = dotted_as_name >> *(',' >> dotted_as_name);
		dotted_name = NAME >> *('.' >> NAME);
		global_stmt = "global" >> NAME >> *(',' >> NAME);
		nonlocal_stmt = "nonlocal" >> NAME >> *(',' >> NAME);
		assert_stmt = "assert" >> test >> -(',' >> test);

		compound_stmt = if_stmt | while_stmt | for_stmt | try_stmt | with_stmt | funcdef | classdef | decorated | async_stmt;
		async_stmt = ASYNC >> (funcdef | with_stmt | for_stmt);
		if_stmt = "if" >> test >> ':' >> suite >> *("elif" >> test >> ':' >> suite) >> -(lit("else") >> ':' >> suite);
		while_stmt = "while" >> test >> ':' >> suite >> -(lit("else") >> ':' >> suite);
		for_stmt = "for" >> exprlist >> "in" >> testlist >> ':' >> suite >> -(lit("else") >> ':' >> suite);
		try_stmt = lit("try") >> ':' >> suite >>
			(+(except_clause >> ':' >> suite) >>
			-(lit("else") >> ':' >> suite) >>
			-(lit("finally") >> ':' >> suite) |
			lit("finally") >> ':' >> suite);
		with_stmt = "with" >> with_item >> *(',' >> with_item) >> ':' >> suite;
		with_item = test >> -("as" >> expr);
		except_clause = "except" >> -(test >> -("as" >> NAME));
		suite = simple_stmt | NEWLINE >> INDENT >> +stmt >> DEDENT;

		test = or_test >> -("if" >> or_test >> "else" >> test) | lambdef;
		test_nocond = or_test | lambdef_nocond;
		lambdef = "lambda" >> -varargslist >> ':' >> test;
		lambdef_nocond = "lambda" >> -varargslist >> ':' >> test_nocond;
		or_test = and_test >> *("or" >> and_test);
		and_test = not_test >> *("and" >> not_test);
		not_test = "not" >> not_test | comparison;
		comparison = expr >> *(comp_op >> expr);
		// <> isn't actually a valid comparison operator in Python. It's here for the
		// sake of a __future__ import described in PEP 401 (which really works : -)
		comp_op = lit('<') | '>' | "==" | ">=" | "<=" | "<>" | "!=" | "in" | lit("not") >> "in" | "is" | lit("is") >> "not";
		star_expr = '*' >> expr;
		expr = xor_expr >> *('|' >> xor_expr);
		xor_expr = and_expr >> *('^' >> and_expr);
		and_expr = shift_expr >> *('&' >> shift_expr);
		shift_expr = arith_expr >> *((lit("<<") | ">>") >> arith_expr);
		arith_expr = term >> *((lit('+') | '-') >> term);
		term = factor >> *((lit('*') | '@' | '/' | '%' | "//") >> factor);
		factor = (lit('+') | '-' | '~') >> factor | power;
		power = atom_expr >> -("**" >> factor);
		atom_expr = -AWAIT >> atom >> *trailer;
		atom = '(' >> -(yield_expr | testlist_comp) >> ')' |
			'[' >> -testlist_comp >> ']' |
			'{' >> -dictorsetmaker >> '}' |
			NAME | NUMBER | +STRING | "..." | "None" | "True" | "False";
		testlist_comp = (test | star_expr) >> (comp_for | *(',' >> (test | star_expr)) >> -lit(','));
		trailer = '(' >> -arglist >> ')' | '[' >> subscriptlist >> ']' | '.' >> NAME;
		subscriptlist = subscript >> *(',' >> subscript) >> -lit(',');
		subscript = test | -test >> ':' >> -test >> -sliceop;
		sliceop = ':' >> -test;
		exprlist = (expr | star_expr) >> *(',' >> (expr | star_expr)) >> -lit(',');
		testlist = test >> *(char_(',') >> test) >> -char_(',');

		dictorsetmaker = ((test >> ':' >> test | "**" >> expr) >> (comp_for | *(',' >> (test >> ':' >> test | "**" >> expr)) >> -lit(','))) |
			((test | star_expr) >> (comp_for | *(',' >> (test | star_expr)) >> -lit(',')));

		classdef = "class" >> NAME >> -('(' >> -arglist >> ')') >> ':' >> suite;

		arglist = argument >> *(',' >> argument) >> -lit(',');

		argument = test >> -comp_for | test >> '=' >> test | "**" >> test | '*' >> test;

		comp_iter = comp_for | comp_if;
		comp_for = -ASYNC >> "for" >> exprlist >> "in" >> or_test >> -comp_iter;
		comp_if = "if" >> test_nocond >> -comp_iter;

		yield_expr = "yield" >> -yield_arg;
		yield_arg = "from" >> test | testlist;
*/

	};
	typedef qi::unicode::blank_type Skipper;
	typedef qi::rule<Iterator, void(), Skipper> Rule;

	Rule NAME, NEWLINE, ASYNC, INDENT, DEDENT, AWAIT, NUMBER, STRING;
	Rule single_input;
	Rule file_input;
	Rule eval_input;
	Rule decorator;
	Rule decorators;
	Rule decorated;
	Rule async_funcdef;
	Rule funcdef;
	Rule parameters;
	Rule typedargslist;
	Rule tfpdef;
	Rule varargslist;
	Rule vfpdef;
	Rule stmt;
	Rule simple_stmt;
	Rule small_stmt;
	Rule expr_stmt;
	Rule annassign;
	Rule testlist_star_expr;
	Rule augassign;
	Rule del_stmt;
	Rule pass_stmt;
	Rule flow_stmt;
	Rule break_stmt;
	Rule continue_stmt;
	Rule return_stmt;
	Rule yield_stmt;
	Rule raise_stmt;
	Rule import_stmt;
	Rule import_name;
	Rule import_from;
	Rule import_as_name;
	Rule dotted_as_name;
	Rule import_as_names;
	Rule dotted_as_names;
	Rule dotted_name;
	Rule global_stmt;
	Rule nonlocal_stmt;
	Rule assert_stmt;
	Rule compound_stmt;
	Rule async_stmt;
	Rule if_stmt;
	Rule while_stmt;
	Rule for_stmt;
	Rule try_stmt;
	Rule with_stmt;
	Rule with_item;
	Rule except_clause;
	Rule suite;
	Rule test;
	Rule test_nocond;
	Rule lambdef;
	Rule lambdef_nocond;
	Rule or_test;
	Rule and_test;
	Rule not_test;
	Rule comparison;
	Rule comp_op;
	Rule star_expr;
	Rule expr;
	Rule xor_expr;
	Rule and_expr;
	Rule shift_expr;
	Rule arith_expr;
	Rule term;
	Rule factor;
	Rule power;
	Rule atom_expr;
	Rule atom;
	Rule testlist_comp;
	Rule trailer;
	Rule subscriptlist;
	Rule subscript;
	Rule sliceop;
	Rule exprlist;
	Rule testlist;
	Rule dictorsetmaker;
	Rule classdef;
	Rule arglist;
	Rule argument;
	Rule comp_iter;
	Rule comp_for;
	Rule comp_if;
	Rule yield_expr;
	Rule yield_arg;
};

template <typename Iterator>
struct PyExprParser : qi::grammar<Iterator, void(), qi::unicode::blank_type>, public PyCommonParser<Iterator>
{
	typedef PyCommonParser<Iterator> Super;
	typedef typename PyCommonParser<Iterator>::Skipper Skipper;
	PyExprParser() : PyExprParser::base_type(testlist, "testlist")
	{
	}
};

bool parsePythonExpression(std::u32string::const_iterator& first, std::u32string::const_iterator last)
{
	PyExprParser<std::u32string::const_iterator> parser;
	return qi::phrase_parse(first, last, parser, unicode::blank, qi::skip_flag::dont_postskip);
}


// This is not really called. Its only purpose is to
// instantiate the method of the grammar.
static void instantiate_parser()
{
	typedef std::u32string::const_iterator iterator_type;
	//skipper<iterator_type> g1;
	PyExprParser<iterator_type> g2;
}
