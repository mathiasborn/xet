// xet.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "py_xet.h"
#include "gtl_tests.h"


int main()
{
	initializePythonInterpreter();
	gtl_test1();
	math_test1();
	//pdf_test1();
	polygon_test1();
	return 0;
}

