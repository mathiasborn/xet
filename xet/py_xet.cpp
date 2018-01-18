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
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/filesystem.hpp>
#include <fcntl.h>

using namespace boost::python;

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
/*
	using namespace spice;
	{
		scope inDDevice = class_<DDevice>("DDevice")
			.add_property("name", make_getter(&DDevice::name, return_value_policy<return_by_value>()), make_setter(&DDevice::name, return_value_policy<return_by_value>()))
			.def_readwrite("nodes", &DDevice::nodes)
			.add_property("model", make_getter(&DDevice::model, return_value_policy<return_by_value>()), make_setter(&DDevice::model, return_value_policy<return_by_value>()))
			.def("prefix", &DDevice::prefix)
			.def(self == self);
		enum_<DDevice::Source>("Source")
			.value("Voltage", DDevice::Voltage)
			.value("Current", DDevice::Current);
		enum_<DDevice::Type>("Type")
			.value("Behavioral", DDevice::Behavioral)
			.value("Gain", DDevice::Gain)
			.value("Table", DDevice::Table)
			.value("Laplace", DDevice::Laplace);
	}
	class_<DBase>("DBase");
	class_<DIgnore, bases<DDevice>>("DIgnore")
		.add_property("text", make_getter(&DIgnore::text, return_value_policy<return_by_value>()), make_setter(&DIgnore::text, return_value_policy<return_by_value>()))
		.def(self == self);
	class_<DInclude, bases<DBase>>("DInclude")
		.add_property("file_name", make_getter(&DInclude::file_name, return_value_policy<return_by_value>()), make_setter(&DInclude::file_name, return_value_policy<return_by_value>()))
		.add_property("lib_name", make_getter(&DInclude::lib_name, return_value_policy<return_by_value>()), make_setter(&DInclude::lib_name, return_value_policy<return_by_value>()))
		.def(self == self);
	class_<DSubCkt, bases<DBase>>("DSubCkt")
		.add_property("name", make_getter(&DSubCkt::name, return_value_policy<return_by_value>()), make_setter(&DSubCkt::name, return_value_policy<return_by_value>()))
		.def_readwrite("nodes", &DSubCkt::nodes);
	class_<NetList>("NetList")
		.add_property("text", make_getter(&NetList::text, return_value_policy<return_by_value>()), make_setter(&NetList::text, return_value_policy<return_by_value>()));
	def("findSubCkt", findSubCkt);
*/
	class_<TestClass>("TestClass",
		init<int, int64_t, tuple>((arg("z")=10, arg("width")=100, arg("adjustment")=tuple())))
	;
	scope().attr("cm") = 10'000'000;
	scope().attr("mm") =  1'000'000;
	scope().attr("um") =      1'000;
	scope().attr("nm") =          1;

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
	PyImport_AppendInittab("xet", PyInit_xet);

/*
    _setmode(fileno(stdin), O_BINARY);
    _setmode(fileno(stdout), O_BINARY);
    _setmode(fileno(stderr), O_BINARY);
    setvbuf(stdin,  (char *)NULL, _IONBF, BUFSIZ);
    setvbuf(stdout, (char *)NULL, _IONBF, BUFSIZ);
    setvbuf(stderr, (char *)NULL, _IONBF, BUFSIZ);
*/

	Py_Initialize();
	InitializeConverters();

	// NoProxy=true is default for std::string elements
	class_<std::vector<std::string>>("string_vector")
		.def(vector_indexing_suite<std::vector<std::string>>());

	// This only works with NoProxy=true?
	class_<std::vector<std::u32string>>("u32string_vector")
		.def(vector_indexing_suite<std::vector<std::u32string>, true>());

	guarded_python([&]()
	{
		object sys_module = import("sys");
		sys_module.attr("dont_write_bytecode") = true;
		object main_module = import("__main__");
		object global = main_module.attr("__dict__");
		//fs::path f = appDir / ".." / "python" / "autostart.py";
		//exec_file(py::str(f.wstring()), global, global);
		object autostart_module = import("xet_test");
		autostart_module.attr("run")();

		auto node = PyParser_SimpleParseString(
			//"frame(pos=center, width=5)", 0);
			"frame(n=10) {hallo}", Py_single_input);
		std::cerr << pythonExceptionToText() << std::endl;
		//auto mod = PyParser_ASTFromString("2+3", "", 0, 0, 0);

	});
}

