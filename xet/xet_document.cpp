#include "stdafx.h"
#include "xet_document.h"
#include "xet_input.h"


namespace xet {

Document::Document()
{
	m_environment["cs"] = py::cpp_function(
		[this](py::object o)->py::object
		{
			if (py::isinstance<py::str>(o))
			{
				return py::cpp_function(
					[this, o](py::object arg)->py::object
					{
						auto name = py::cast<std::u32string>(o);
						this->m_controlSequences.insert_or_assign(name, ControlSequence{ arg });
						return arg;
					});
			}
			auto name = py::cast<std::u32string>(o.attr("__name__"));
			this->m_controlSequences.insert_or_assign(name, ControlSequence{o});
			return o;
		}, py::arg("o"));
}

void Document::addInput(fs::path const& fileName)
{
	LinePosTextRange name;

	auto t = m_inputs.emplace_front(fileName, input::loadUTF8TextFile(fileName));
	auto const& text = std::get<1>(t);
	auto parseTokens = parser::parse(text.begin(), text.end());
	auto tokens = input::convert(parseTokens, fileName, *this);
	m_tokens.insert(m_tokens.end(), tokens.begin(), tokens.end());
}

}	// namespace xet
