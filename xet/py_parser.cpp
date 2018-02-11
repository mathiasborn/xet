#include "stdafx.h"
#include <string>
#include <fstream>
#include <sstream>

#include <boost/spirit/include/support.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include "py_parser.h"
#include "py_literal_parser.h"
#include "xet_line_pos_iterator.h"

namespace qi = boost::spirit::qi;
using namespace boost::spirit;

template <typename Iterator, typename Skipper> PyCommonParser<Iterator, Skipper>::PyCommonParser()
{
	using qi::unicode::char_;
	using qi::unicode::blank;
	using qi::int_;
	using qi::no_case;

	NAME = lexeme[char_(L"A-Za-z_") >> *char_(L"0-9A-Za-z_")];
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

	vfpdef = NAME.alias();

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
	yield_stmt = yield_expr.alias();
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
		NAME | lexeme[NUMBER] | +lexeme[STRING] | "..." | "None" | "True" | "False";
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

	argument = test >> '=' >> test | test >> -comp_for | "**" >> test | '*' >> test;

	comp_iter = comp_for | comp_if;
	comp_for = -ASYNC >> "for" >> exprlist >> "in" >> or_test >> -comp_iter;
	comp_if = "if" >> test_nocond >> -comp_iter;

	yield_expr = "yield" >> -yield_arg;
	yield_arg = "from" >> test | testlist;

	//BOOST_SPIRIT_DEBUG_NODE(STRING);
	BOOST_SPIRIT_DEBUG_NODE(NAME);
	BOOST_SPIRIT_DEBUG_NODE(NEWLINE);
	BOOST_SPIRIT_DEBUG_NODE(ASYNC);
	BOOST_SPIRIT_DEBUG_NODE(INDENT);
	BOOST_SPIRIT_DEBUG_NODE(DEDENT);
	BOOST_SPIRIT_DEBUG_NODE(AWAIT);
	BOOST_SPIRIT_DEBUG_NODE(single_input);
	BOOST_SPIRIT_DEBUG_NODE(file_input);
	BOOST_SPIRIT_DEBUG_NODE(eval_input);
	BOOST_SPIRIT_DEBUG_NODE(decorator);
	BOOST_SPIRIT_DEBUG_NODE(decorators);
	BOOST_SPIRIT_DEBUG_NODE(decorated);
	BOOST_SPIRIT_DEBUG_NODE(async_funcdef);
	BOOST_SPIRIT_DEBUG_NODE(funcdef);
	BOOST_SPIRIT_DEBUG_NODE(parameters);
	BOOST_SPIRIT_DEBUG_NODE(typedargslist);
	BOOST_SPIRIT_DEBUG_NODE(tfpdef);
	BOOST_SPIRIT_DEBUG_NODE(varargslist);
	BOOST_SPIRIT_DEBUG_NODE(vfpdef);
	BOOST_SPIRIT_DEBUG_NODE(stmt);
	BOOST_SPIRIT_DEBUG_NODE(simple_stmt);
	BOOST_SPIRIT_DEBUG_NODE(small_stmt);
	BOOST_SPIRIT_DEBUG_NODE(expr_stmt);
	BOOST_SPIRIT_DEBUG_NODE(annassign);
	BOOST_SPIRIT_DEBUG_NODE(testlist_star_expr);
	BOOST_SPIRIT_DEBUG_NODE(augassign);
	BOOST_SPIRIT_DEBUG_NODE(del_stmt);
	BOOST_SPIRIT_DEBUG_NODE(pass_stmt);
	BOOST_SPIRIT_DEBUG_NODE(flow_stmt);
	BOOST_SPIRIT_DEBUG_NODE(break_stmt);
	BOOST_SPIRIT_DEBUG_NODE(continue_stmt);
	BOOST_SPIRIT_DEBUG_NODE(return_stmt);
	BOOST_SPIRIT_DEBUG_NODE(yield_stmt);
	BOOST_SPIRIT_DEBUG_NODE(raise_stmt);
	BOOST_SPIRIT_DEBUG_NODE(import_stmt);
	BOOST_SPIRIT_DEBUG_NODE(import_name);
	BOOST_SPIRIT_DEBUG_NODE(import_from);
	BOOST_SPIRIT_DEBUG_NODE(import_as_name);
	BOOST_SPIRIT_DEBUG_NODE(dotted_as_name);
	BOOST_SPIRIT_DEBUG_NODE(import_as_names);
	BOOST_SPIRIT_DEBUG_NODE(dotted_as_names);
	BOOST_SPIRIT_DEBUG_NODE(dotted_name);
	BOOST_SPIRIT_DEBUG_NODE(global_stmt);
	BOOST_SPIRIT_DEBUG_NODE(nonlocal_stmt);
	BOOST_SPIRIT_DEBUG_NODE(assert_stmt);
	BOOST_SPIRIT_DEBUG_NODE(compound_stmt);
	BOOST_SPIRIT_DEBUG_NODE(async_stmt);
	BOOST_SPIRIT_DEBUG_NODE(if_stmt);
	BOOST_SPIRIT_DEBUG_NODE(while_stmt);
	BOOST_SPIRIT_DEBUG_NODE(for_stmt);
	BOOST_SPIRIT_DEBUG_NODE(try_stmt);
	BOOST_SPIRIT_DEBUG_NODE(with_stmt);
	BOOST_SPIRIT_DEBUG_NODE(with_item);
	BOOST_SPIRIT_DEBUG_NODE(except_clause);
	BOOST_SPIRIT_DEBUG_NODE(suite);
	BOOST_SPIRIT_DEBUG_NODE(test);
	BOOST_SPIRIT_DEBUG_NODE(test_nocond);
	BOOST_SPIRIT_DEBUG_NODE(lambdef);
	BOOST_SPIRIT_DEBUG_NODE(lambdef_nocond);
	BOOST_SPIRIT_DEBUG_NODE(or_test);
	BOOST_SPIRIT_DEBUG_NODE(and_test);
	BOOST_SPIRIT_DEBUG_NODE(not_test);
	BOOST_SPIRIT_DEBUG_NODE(comparison);
	BOOST_SPIRIT_DEBUG_NODE(comp_op);
	BOOST_SPIRIT_DEBUG_NODE(star_expr);
	BOOST_SPIRIT_DEBUG_NODE(expr);
	BOOST_SPIRIT_DEBUG_NODE(xor_expr);
	BOOST_SPIRIT_DEBUG_NODE(and_expr);
	BOOST_SPIRIT_DEBUG_NODE(shift_expr);
	BOOST_SPIRIT_DEBUG_NODE(arith_expr);
	BOOST_SPIRIT_DEBUG_NODE(term);
	BOOST_SPIRIT_DEBUG_NODE(factor);
	BOOST_SPIRIT_DEBUG_NODE(power);
	BOOST_SPIRIT_DEBUG_NODE(atom_expr);
	BOOST_SPIRIT_DEBUG_NODE(atom);
	BOOST_SPIRIT_DEBUG_NODE(testlist_comp);
	BOOST_SPIRIT_DEBUG_NODE(trailer);
	BOOST_SPIRIT_DEBUG_NODE(subscriptlist);
	BOOST_SPIRIT_DEBUG_NODE(subscript);
	BOOST_SPIRIT_DEBUG_NODE(sliceop);
	BOOST_SPIRIT_DEBUG_NODE(exprlist);
	BOOST_SPIRIT_DEBUG_NODE(testlist);
	BOOST_SPIRIT_DEBUG_NODE(dictorsetmaker);
	BOOST_SPIRIT_DEBUG_NODE(classdef);
	BOOST_SPIRIT_DEBUG_NODE(arglist);
	BOOST_SPIRIT_DEBUG_NODE(argument);
	BOOST_SPIRIT_DEBUG_NODE(comp_iter);
	BOOST_SPIRIT_DEBUG_NODE(comp_for);
	BOOST_SPIRIT_DEBUG_NODE(comp_if);
	BOOST_SPIRIT_DEBUG_NODE(yield_expr);
	BOOST_SPIRIT_DEBUG_NODE(yield_arg);

}


template <typename Iterator> PyExprParser<Iterator>::PyExprParser(): PyExprParser::base_type(start, "py_expr")
{
	start = raw[skip(unicode::blank)[testlist]];
}


/*
bool parsePythonExpression(std::u32string::const_iterator& first, std::u32string::const_iterator last, boost::iterator_range<std::u32string::const_iterator>& result)
{
#	ifdef BOOST_SPIRIT_DEBUG
	std::streambuf* strm_buffer = std::cout.rdbuf();
	std::ofstream file("cout.xml");
	std::cout.rdbuf(file.rdbuf());
	std::cerr.tie(&std::cout);
	std::cout << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" << std::endl;// << "<start>" << std::endl;
#	endif	

	PyExprParser<std::u32string::const_iterator> parser;
	auto r = qi::parse(first, last, parser, result);

#	ifdef BOOST_SPIRIT_DEBUG
	//std::cout << "</start>" << std::endl;
	std::cout.rdbuf(strm_buffer);
#	endif	

	return r;
}
*/


// This is not really called. Its only purpose is to
// instantiate the method of the grammar.
void instantiate_py_expr_parser()
{
	//typedef std::u32string::const_iterator Iterator;
	typedef line_pos_iterator<std::u32string::const_iterator> Iterator;
	PyExprParser<Iterator> g;
}

