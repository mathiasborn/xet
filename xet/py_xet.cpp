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

#include "stdafx.h"
#include <boost/filesystem.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <fcntl.h>
#include "py_xet.h"
#include "unicode_string_support.h"
#include "platform.h"
#include "xet_document.h"

namespace py = pybind11;


PyException::PyException(py::error_already_set& e)
{
	try
	{
		e.restore();
		PyErr_Fetch(&m_exc.ptr(), &m_val.ptr(), &m_tb.ptr());
		PyErr_NormalizeException(&m_exc.ptr(), &m_val.ptr(), &m_tb.ptr());
		if (!m_val.ptr()) m_val = py::none();
		if (!m_tb.ptr()) m_tb = py::none();
		{
			py::object traceback = py::module::import("traceback");
			py::object print_exception = traceback.attr("print_exception");
			py::object io = py::module::import("io");
			py::object buf = io.attr("StringIO")();
			print_exception(m_exc, m_val, m_tb, py::none(), buf, true);
			py::object text = buf.attr("getvalue")();
			m_text = text.cast<std::string>();
		}
	}
	catch(py::error_already_set&)
	{
		//PyErr_Clear();
		m_text = "Exception during exception handling. Giving up.";
	}
}

void PyException::restore()
{
	PyErr_Restore(m_exc.release().ptr(), m_val.release().ptr(), m_tb.release().ptr());
}


std::string pythonExceptionToText(py::error_already_set& e)
{
	try
	{
		e.restore();
		py::object m_exc, m_val, m_tb;
		PyErr_Fetch(&m_exc.ptr(), &m_val.ptr(), &m_tb.ptr());
		PyErr_NormalizeException(&m_exc.ptr(), &m_val.ptr(), &m_tb.ptr());
		if (!m_val.ptr()) m_val = py::none();
		if (!m_tb.ptr()) m_tb = py::none();
		{
			py::object traceback = py::module::import("traceback");
			py::object print_exception = traceback.attr("print_exception");
			py::object io = py::module::import("io");
			py::object buf = io.attr("StringIO")();
			print_exception(m_exc, m_val, m_tb, py::none(), buf, true);
			py::object text = buf.attr("getvalue")();
			return text.cast<std::string>();
		}
	}
	catch(py::error_already_set&)
	{
		//PyErr_Clear();
		return "Exception during exception handling. Giving up.";
	}
}


class TestClass
{
public:
	TestClass(int z, int64_t width, py::tuple adjustment)
	{
		std::cout << "z=" << z << " width=" << width << " len(adjustment)=" << py::len(adjustment) << std::endl;
	};
};

class PyActor: public input::Actor
{
public:
	/* Inherit the constructors */
	using Actor::Actor;
/*
	void addedToPage(xet::PPage& page) override {
		PYBIND11_OVERLOAD(
			void,			// Return type
			input::Actor,	// Parent class
			addedToPage,	// Name of function in C++ (must match Python name)
			page			// Argument(s)
		);
	}
*/
	void addedToPage() override {
		PYBIND11_OVERLOAD(
			void,			// Return type
			input::Actor,	// Parent class
			addedToPage		// Name of function in C++ (must match Python name)
		);
	}
};

PYBIND11_MAKE_OPAQUE(std::vector<input::Token>);
//PYBIND11_MAKE_OPAQUE(input::Tokens);
PYBIND11_MAKE_OPAQUE(std::vector<input::Tokens>);

PYBIND11_MODULE(xet, m)
{
	using namespace py::literals;
		
	m.attr("cm") = 10'000'000;
	m.attr("mm") = 1'000'000;
	m.attr("um") = 1'000;
	m.attr("nm") = 1;

	py::class_<TestClass> testClass(m, "TestClass");
	testClass
		.def(py::init<int, int64_t, py::tuple>(), "z"_a = 10, "width"_a = 100, "adjustment"_a = py::tuple{});

	//py::bind_vector<input::Tokens>(m, "Tokens");
	py::bind_vector<std::vector<input::Token>>(m, "Tokens");
	py::bind_vector<std::vector<input::Tokens>>(m, "Groups");

	py::class_<xet::CSDecoratorFromArgs>(m, "CSDecoratorFromArgs")
		.def("__call__", &xet::CSDecoratorFromArgs::operator());
	py::class_<xet::CSDecorator>(m, "CSDecorator")
		.def("__call__", py::overload_cast<std::u32string const&>(&xet::CSDecorator::operator()))
		.def("__call__", py::overload_cast<std::u32string const&, unsigned int>(&xet::CSDecorator::operator()))
		.def("__call__", py::overload_cast<unsigned int>(&xet::CSDecorator::operator()))
		.def("__call__", py::overload_cast<py::object>(&xet::CSDecorator::operator()));

	py::class_<xet::Document>(m, "Document")
		.def(py::init<>())
		.def("addInput", &xet::Document::addInput);

	py::class_<input::Actor, PyActor>(m, "Actor")
		.def(py::init<>())
		.def("addedToPage", &input::Actor::addedToPage);

}


void initializePythonPath()
{
	using namespace std;
	fs::path appDir = platform::appFileName().parent_path();

	fs::ifstream pypath(appDir / "pypath", ios::in);
	if (pypath)
	{
		pypath.unsetf(ios::skipws);      // No white space skipping!
		wstring sbuf = wstring(istreambuf_iterator<char>(pypath.rdbuf()), istreambuf_iterator<char>());
		Py_SetPath(sbuf.c_str());
	}
	else
	{
	}
}

void setupPythonInterpreter()
{
	guarded_python([&]()
	{
		py::object sys_module = py::module::import("sys");
		sys_module.attr("dont_write_bytecode") = true;
		py::object main_module = py::module::import("__main__");
		py::object global = main_module.attr("__dict__");
		//fs::path f = appDir / ".." / "python" / "autostart.py";
		//exec_file(py::str(f.wstring()), global, global);
		py::object autostart_module = py::module::import("xet_test");
		autostart_module.attr("run")();
	});
}

