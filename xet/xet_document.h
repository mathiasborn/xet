#pragma once

#include <string>
#include <unordered_map>
#include <forward_list>
#include <tuple>
#include <pybind11/pybind11.h>
#include "xet_input.h"

namespace py = pybind11;

namespace xet {

class Document
{
public:
	struct ControlSequence
	{
		py::object callable;
	};
	typedef std::unordered_map<std::u32string, ControlSequence> ControlSequences;

	Document();

	void addInput(fs::path const& fileName);

	ControlSequences& controlSequences() { return m_controlSequences; }
	py::dict& environment() { return m_environment; }
private:
	ControlSequences m_controlSequences;
	py::dict m_environment;
	std::forward_list<std::tuple<fs::path, std::u32string>> m_inputs;
	input::Tokens m_tokens;
};

}	// namespace xet
