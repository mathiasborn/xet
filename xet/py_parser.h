#pragma once
#include <string>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>

#include "py_literal_parser.h"

template <typename Iterator, typename Skipper>
struct PyCommonParser
{
	PyCommonParser();
	
	typedef qi::rule<Iterator, void(), Skipper> Rule;

	PyStrParser<Iterator> STRING;
	PyNumberParser<Iterator> NUMBER;
	Rule NAME, NEWLINE, ASYNC, INDENT, DEDENT, AWAIT;
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


template <typename Iterator, typename Skipper>
struct PyExprParser: qi::grammar<Iterator, void(), Skipper>, public PyCommonParser<Iterator, Skipper>
{
	PyExprParser();
};


bool parsePythonExpression(std::u32string::const_iterator& first, std::u32string::const_iterator last);

