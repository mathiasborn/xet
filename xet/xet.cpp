// xet.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <pybind11/embed.h>

#include "py_xet.h"
#include "gtl_tests.h"
#include "py_parser.h"
#include "xet_input.h"

namespace py = pybind11;

int main()
{
	using namespace std::string_literals;
/*	
	auto s = U"frame(center='hallo', width=5.4e9j*cm) { hallo }"s;
	auto first = s.cbegin();
	auto last = s.cend();
	auto r = parsePythonExpression(first, last);
	std::cout << r << std::endl;
*/
	initializePythonPath();
	py::scoped_interpreter guard{};
	setupPythonInterpreter();
	gtl_test1();
	math_test1();
	//pdf_test1();
	polygon_test1();
	
	test_xet_input("sample3.xet");


	return 0;
}

