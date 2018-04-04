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
#pragma once

#include "stdafx.h"
#include "unicode_string_support.h"
#include <boost/function.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <pybind11/pybind11.h>

namespace py = pybind11;

void initializePythonPath();
void setupPythonInterpreter();


class PyException: public std::exception
{
	std::string m_text;
	py::object m_exc, m_val, m_tb;
public:
	PyException(py::error_already_set&);
	virtual const char *what() const { return m_text.c_str(); }
	void restore();
};


std::string pythonExceptionToText(py::error_already_set&);	// return UTF-8

template<typename Function>
auto guarded_python(Function const& func)->decltype(func())
{
	try
	{
		return func();
	}
	catch (py::error_already_set& e)
	{
		auto s = pythonExceptionToText(e);
		//auto s = e.what();
		std::cerr << s << std::endl;
	}
	catch (std::exception const& e)
	{
		std::cerr << e.what() << std::endl;
	}
	typedef decltype(func()) T;
	return T();
}

/*
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
*/

namespace pybind11::detail {
template<> struct type_caster<fs::path>
{
public:
	/**
	* This macro establishes the name 'fs::path' in
	* function signatures and declares a local variable
	* 'value' of type fs::path
	*/
	PYBIND11_TYPE_CASTER(fs::path, _("fs::path"));

	/**
	* Conversion part 1 (Python->C++): convert a PyObject into a fs::path
	* instance or return false upon failure. The second argument
	* indicates whether implicit conversions should be applied.
	*/
	bool load(handle src, bool)
	{
		auto o = py::reinterpret_borrow<py::object>(src);
		try
		{
			auto ps = o.cast<py::str>();
			auto s = ps.cast<std::u32string>();
			value = uts::toPath(s);
		}
		catch(py::cast_error const&)
		{
			return false;
		}
		return true;
	}

	/**
	* Conversion part 2 (C++ -> Python): convert an fs::path instance into
	* a Python object. The second and third arguments are used to
	* indicate the return value policy and parent object (for
	* ``return_value_policy::reference_internal``) and are generally
	* ignored by implicit casters.
	*/
	static handle cast(fs::path const& src, return_value_policy /* policy */, handle /* parent */)
	{
		auto r = py::cast(uts::toUtf32(src));
		return r.release();
	}
};
} // namespace pybind11::detail

class PyObjectHolderBase
{
protected:
	PyObject* m_pyObject = nullptr;
public:
	virtual ~PyObjectHolderBase() {};
};

// to be used together with boost::intrusive_ptr
template<typename T>
class PyObjectHolder: protected virtual PyObjectHolderBase
{
public:
	typedef boost::intrusive_ptr<T> Pointer;

	void _increaseRefCount()
	{
		if (m_pyObject)
			Py_INCREF(m_pyObject);
		else
		{
			auto _this = static_cast<T*>(this);
			try {
				auto o = py::cast(_this, py::return_value_policy::take_ownership);
				m_pyObject = o.release().ptr();
			}
			catch(...)
			{
				// The instance could not be wrapped. This should not happen and indicates a bug.
				// We know that the object has been created with "new", otherwise it couldn't be wrapped, thus we can safely "delete this" here.
				delete this;
				throw;
			}
		}
	}
	void _decreaseRefCount() { Py_DECREF(m_pyObject); }
	int refCount() const { return static_cast<int>(Py_REFCNT(m_pyObject)); }
	void _assignWrap(PyObject* o) noexcept { m_pyObject = o; }

	static Pointer cast(py::object& o)
	{
		auto p = py::cast<T*>(o);
		p->_assignWrap(o.release().ptr());
		return {p, false};
	}
};

template<typename T>
inline void intrusive_ptr_add_ref(PyObjectHolder<T>* p)
{
	p->_increaseRefCount();
}

template<typename T>
inline void intrusive_ptr_release(PyObjectHolder<T>* p)
{
	p->_decreaseRefCount();
}
