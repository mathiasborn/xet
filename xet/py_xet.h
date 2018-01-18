/***************************************************************************
	xet
	Copyright (C) 2018 Mathias Born

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/
#ifndef PY_XET_H
#define PY_XET_H

#include "stdafx.h"
#include <boost/function.hpp>
#include <boost/python.hpp>

namespace py = boost::python;


void initializePythonInterpreter();

class PyException: public std::exception
{
	std::string m_text;
	py::handle<> m_exc, m_val, m_tb;
public:
	PyException();
	virtual const char *what() const { return m_text.c_str(); }
	void PyErr_Restore();
};

std::string pythonExceptionToText();	// return UTF-8

template<typename Function>
auto guarded_python(Function const& func)->decltype(func())
{
	try
	{
		return func();
	}
	catch (py::error_already_set const &)
	{
		auto s = pythonExceptionToText();
		std::cerr << s << std::endl;
	}
	catch (std::exception const& e)
	{
		std::cerr << e.what() << std::endl;
	}
	typedef decltype(func()) T;
	return T();
}


// Convert py::error_already_set into std::exception
template<typename Function>
auto std_python(Function const& func)->decltype(func())
{
	try
	{
		return func();
	}
	catch(py::error_already_set const &)
	{
		throw PyException();
	}
	typedef decltype(func()) T;
	return T();
}


#endif // PY_XET_H
