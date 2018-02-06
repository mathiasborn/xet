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
#include "py_xet.h"
#include "unicode_string_support.h"
#include "platform.h"
#include <boost/filesystem.hpp>
#include <pybind11/embed.h>
#include <fcntl.h>

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

/*
struct u32string_to_python_str
{
	static PyObject* convert(std::u32string const& s)
	{
		auto u8 = uts::toUtf8(s);
		return PyUnicode_DecodeUTF8(u8.data(), u8.length(), nullptr);
	}
};

struct u32string_from_python_str
{
	u32string_from_python_str()
	{
		boost::python::converter::registry::push_back(
			&convertible,
			&construct,
			boost::python::type_id<std::u32string>()
		);
	}

	// Determine if obj_ptr can be converted in a std::u32string
	static void* convertible(PyObject* obj_ptr)
	{
		if (!PyUnicode_Check(obj_ptr)) return 0;
		return obj_ptr;
	}

	// Convert obj_ptr into a std::u32string
	static void construct(
		PyObject* obj_ptr,
		boost::python::converter::rvalue_from_python_stage1_data* data)
	{
		// Extract the character data from the python string
		Py_ssize_t size;
		char* s = PyUnicode_AsUTF8AndSize(obj_ptr, &size); // The caller is not responsible for deallocating the buffer s.

		// Verify that obj_ptr is a string (should be ensured by convertible())
		assert(s);

		auto value = uts::toUtf32(s, size);

		// Grab pointer to memory into which to construct the new QString
		void* storage = ( (boost::python::converter::rvalue_from_python_storage<std::u32string>*)data )->storage.bytes;

		// in-place construct the new std::u32string using the character data extraced from the python object
		new (storage) std::u32string{value};

		// Stash the memory chunk pointer for later use by boost.python
		data->convertible = storage;
	}
};

template<typename T>
struct optional_to_python
{
	static PyObject* convert(boost::optional<T> const& o)
	{
		return incref(o ? object(*o).ptr() : Py_None);
	}
};


void InitializeConverters()
{
	using namespace boost::python;

	to_python_converter<std::u32string, u32string_to_python_str>();
	u32string_from_python_str();
}

class TestClass
{
public:
	TestClass(int z, int64_t width, tuple adjustment)
	{
		std::cout << "z=" << z << " width=" << width << " len(adjustment)=" << len(adjustment) <<  std::endl;
	};
};

BOOST_PYTHON_MODULE(xet)
{
	class_<TestClass>("TestClass",
		init<int, int64_t, tuple>((arg("z")=10, arg("width")=100, arg("adjustment")=tuple())))
	;
	scope().attr("cm") = 10'000'000;
	scope().attr("mm") =  1'000'000;
	scope().attr("um") =      1'000;
	scope().attr("nm") =          1;

}
*/

class TestClass
{
public:
	TestClass(int z, int64_t width, py::tuple adjustment)
	{
		std::cout << "z=" << z << " width=" << width << " len(adjustment)=" << py::len(adjustment) << std::endl;
	};
};

PYBIND11_EMBEDDED_MODULE(xet, m)
{
	using namespace py::literals;
		
	m.attr("cm") = 10'000'000;
	m.attr("mm") = 1'000'000;
	m.attr("um") = 1'000;
	m.attr("nm") = 1;

	py::class_<TestClass> testClass(m, "TestClass");
	testClass
		.def(py::init<int, int64_t, py::tuple>(), "z"_a = 10, "width"_a = 100, "adjustment"_a = py::tuple{});

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

