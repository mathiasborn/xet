#pragma once

#include <string>
#include <unordered_map>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace xet {

class Document
{
	struct ControlSequence
	{
		py::object callable;
	};
	typedef std::unordered_map<std::u32string, ControlSequence> ControlSequences;

	Document();
	ControlSequences& controlSequences() { return m_controlSequences; }
	py::dict& environment() { return m_environment; }
private:
	ControlSequences m_controlSequences;
	py::dict m_environment;
};

}	// namespace xet
