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
#include <pybind11/stl.h>
#include <fcntl.h>
#include "py_xet.h"
#include "unicode_string_support.h"
#include "platform.h"
#include "xet_document.h"
#include "xet_input.h"
#include "xet_geometry.h"

namespace py = pybind11;
using namespace std::string_literals;

PYBIND11_MAKE_OPAQUE(input::Tokens);
PYBIND11_MAKE_OPAQUE(input::Groups);
PYBIND11_DECLARE_HOLDER_TYPE(T, boost::intrusive_ptr<T>, true);

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
	TestClass(TestClass const&)
	{
		std::cout << "TestClass(TestClass const&)" << std::endl;
	}
	TestClass(int z, int64_t width, py::tuple adjustment)
	{
		std::cout << "z=" << z << " width=" << width << " len(adjustment)=" << py::len(adjustment) << std::endl;
	};
};
typedef std::shared_ptr<TestClass> PTestClass;

/*
class PyTestClass: public TestClass
{
public:
	using TestClass::TestClass;
*/

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
			addedToPage,	// Name of function in C++ (must match Python name)
		);
	}
};

class PyTypeSetter: public xet::TypeSetter
{
public:
	using TypeSetter::TypeSetter;	// Inherit the constructors

	xet::CPolygonSets geometry(double a) override
	{
		PYBIND11_OVERLOAD_PURE(
			xet::CPolygonSets,	// Return type
			xet::TypeSetter,	// Parent class
			geometry,			// Name of function in C++ (must match Python name)
			a					// Argument(s)
		);
	}
};

class PyPage: public xet::Page
{
public:
	using Page::Page;			// Inherit the constructors

	xet::PPage nextPage() override
	{
		PYBIND11_OVERLOAD_PURE(
			xet::PPage,			// Return type
			xet::Page,			// Parent class
			nextPage,			// Name of function in C++ (must match Python name)
		);
	}
};

PYBIND11_MODULE(xet, m)
{
	using namespace py::literals;
		
	m.attr("cm") = 10'000'000;
	m.attr("mm") = 1'000'000;
	m.attr("um") = 1'000;
	m.attr("nm") = 1;

	py::class_<TestClass, PTestClass> testClass(m, "TestClass");
	testClass
		.def(py::init<TestClass const&>(), "other"_a)
		.def(py::init<int, int64_t, py::tuple>(), "z"_a = 10, "width"_a = 100, "adjustment"_a = py::tuple{});

	py::bind_vector<input::Tokens, input::PTokens>(m, "Tokens");
	py::bind_vector<input::Groups>(m, "Groups");

	py::class_<xet::CSDecoratorFromArgs>(m, "CSDecoratorFromArgs")
		.def("__call__", &xet::CSDecoratorFromArgs::operator());
	py::class_<xet::CSDecorator>(m, "CSDecorator")
		.def("__call__", py::overload_cast<std::u32string const&>(&xet::CSDecorator::operator()))
		.def("__call__", py::overload_cast<std::u32string const&, unsigned int>(&xet::CSDecorator::operator()))
		.def("__call__", py::overload_cast<unsigned int>(&xet::CSDecorator::operator()))
		.def("__call__", py::overload_cast<py::object>(&xet::CSDecorator::operator()));

	py::class_<xet::Document, xet::PDocument>(m, "Document")
		.def(py::init<>())
		.def("addInput", &xet::Document::addInput)
		.def_property_readonly("tokens", &xet::Document::tokens);

	py::class_<input::Actor, PyActor, input::PActor>(m, "Actor")
		.def(py::init<>())
		.def("addedToPage", &input::Actor::addedToPage);

	py::class_<input::Text>(m, "Text")
		.def(py::init<std::u32string const&>(), "text"_a)
		.def("__repr__", [](input::Text const& a){ return U"xet.Text('"s + a.text() + U"')"; })
		.def_property_readonly("text", &input::Text::text);

	py::class_<input::Glue>(m, "Glue")
		.def(py::init<>())
		.def(py::init<xet::Size, xet::Size, xet::Size>(), "width"_a, "stretchability"_a, "shrinkability"_a)
		.def("__repr__", [](input::Glue const& a){ return static_cast<std::u32string>(a); });

	py::class_<input::Penalty>(m, "Penalty")
		.def(py::init<int, xet::Size>(), "value"_a, "width"_a)
		.def_property_readonly_static("pinf", [](py::object){ return input::Penalty::pinf; })
		.def_property_readonly_static("ninf", [](py::object){ return input::Penalty::ninf; })
		.def("__repr__", [](input::Penalty const& a){ return static_cast<std::u32string>(a); });

	py::class_<input::ParagraphSeperator>(m, "ParagraphSeperator")
		.def(py::init<>())
		.def("__repr__", [](input::ParagraphSeperator const& a){ return static_cast<std::u32string>(a); });

	py::class_<input::Push>(m, "Push")
		.def(py::init<>())
		.def("__repr__", [](input::Push const& a){ return static_cast<std::u32string>(a); });

	py::class_<input::Pop>(m, "Pop")
		.def(py::init<>())
		.def("__repr__", [](input::Pop const& a){ return static_cast<std::u32string>(a); });

	py::class_<input::Stream>(m, "Stream")
		.def(py::init<int32_t>(), "n"_a)
		.def("__repr__", [](input::Stream const& a){ return static_cast<std::u32string>(a); });

	py::class_<input::ActiveToken>(m, "ActiveToken")
		.def("__repr__", [](input::ActiveToken const& a){ return static_cast<std::u32string>(a); });

	py::class_<xet::TypeSetter, PyTypeSetter, xet::PTypeSetter>(m, "TypeSetter")
		.def(py::init<int32_t, int32_t, std::u32string, bool>(), "layer"_a, "cutOrder"_a, "name"_a, "simple"_a)
		.def("geometry", &xet::TypeSetter::geometry);

	py::class_<xet::Page, PyPage, xet::PPage>(m, "Page")
		.def(py::init<xet::Size, xet::Size>(), "width"_a, "height"_a)
		.def("nextPage", &xet::Page::nextPage);

	xet::pyInitGeometry(m);
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

