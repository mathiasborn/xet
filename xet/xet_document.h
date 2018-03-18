#pragma once

#include <string>
#include <unordered_map>
#include <forward_list>
#include <tuple>
#include <memory>
#include <pybind11/pybind11.h>
#include "xet_input.h"
#include "xet_font.h"

namespace py = pybind11;

namespace xet {

class Document
{
public:
	struct ControlSequence
	{
		py::object callable;
		unsigned int groups;
	};
	typedef std::unordered_map<std::u32string, ControlSequence> ControlSequences;

	Document();

	void addInput(fs::path const& fileName);

	ControlSequences& controlSequences() { return m_controlSequences; }
	py::dict& environment() { return m_environment; }
	input::PTokens tokens() const { return m_tokens; }

	PFont font(fs::path const& path, int size) { return m_fontRegistry.font(path, size); }

	GlyphInfos shape(Font& font, std::u32string const& text);

private:
	ControlSequences m_controlSequences;
	py::dict m_environment;
	std::forward_list<std::tuple<fs::path, std::u32string>> m_inputs;
	input::PTokens m_tokens = std::make_shared<input::Tokens>();
	FontRegistry m_fontRegistry;
};

class CSDecoratorFromArgs
{
public:
	CSDecoratorFromArgs(Document::ControlSequences& seqs, std::u32string const& name, unsigned int groups = 0):
		m_controlSequences(seqs), m_name(name), m_groups(groups) {};
	py::object operator()(py::object);
private:
	Document::ControlSequences& m_controlSequences;
	std::u32string m_name;
	unsigned int m_groups;
};

class CSDecorator
{
public:
	CSDecorator(Document::ControlSequences& seqs): m_controlSequences(seqs) {};
	py::object operator()(py::object);
	CSDecoratorFromArgs operator()(std::u32string const&);
	CSDecoratorFromArgs operator()(std::u32string const&, unsigned int groups);
	CSDecoratorFromArgs operator()(unsigned int groups);
private:
	Document::ControlSequences& m_controlSequences;
};

}	// namespace xet
