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
		std::u32string m_name;

	};
	typedef std::unordered_map<std::u32string, ControlSequence> ControlSequences;
private:
	ControlSequences m_controlSequences;
};

}	// namespace xet
