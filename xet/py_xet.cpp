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

/*
PyException::PyException()
{
	try
	{
		PyObject *exc, *val, *tb;
		PyErr_Fetch(&exc, &val, &tb);
		PyErr_NormalizeException(&exc, &val, &tb);
		{
			m_exc = handle<>(exc);
			m_val = handle<>(allow_null(val));
			m_tb = handle<>(allow_null(tb));

			object traceback = import("traceback");
			object print_exception = traceback.attr("print_exception");
			object io = import("io");
			object buf = io.attr("StringIO")();
			print_exception(m_exc, m_val, m_tb, object(), buf, true);
			object text = buf.attr("getvalue")();
			m_text = extract<std::string>(text);
		}
	}
	catch(error_already_set const &)
	{
		PyErr_Clear();
		m_text = "Exception during exception handling. Giving up.";
	}
}

void PyException::PyErr_Restore()
{
	::PyErr_Restore(m_exc.release(), m_val.release(), m_tb.release());
}


std::string pythonExceptionToText()
{
	try
	{
		PyObject *exc, *val, *tb;
		PyErr_Fetch(&exc, &val, &tb);
		PyErr_NormalizeException(&exc, &val, &tb);
		//if (exc && val && tb)
		{
			handle<> hexc(exc), hval(allow_null(val)), htb(allow_null(tb));

			object traceback = import("traceback");
			object print_exception = traceback.attr("print_exception");
			object io = import("io");
			object buf = io.attr("StringIO")();
			print_exception(hexc, hval, htb, object(), buf, true);
			object text = buf.attr("getvalue")();
			return extract<std::string>(text);
		}
		return "Python Exception caught.";
	}
	catch(error_already_set const &)
	{
		PyErr_Clear();
		return "Exception during exception handling. Giving up.";
	}
}

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


void initializePythonInterpreter()
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
/*
	// NoProxy=true is default for std::string elements
	class_<std::vector<std::string>>("string_vector")
		.def(vector_indexing_suite<std::vector<std::string>>());

	// This only works with NoProxy=true?
	class_<std::vector<std::u32string>>("u32string_vector")
		.def(vector_indexing_suite<std::vector<std::u32string>, true>());
*/
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

