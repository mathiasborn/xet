#pragma once

#include <string>
#include <unordered_map>
#include <forward_list>
#include <tuple>
#include <memory>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <pybind11/pybind11.h>
#include "xet_input.h"
#include "xet_font.h"
#include "interfaces.h"

namespace py = pybind11;

namespace xet {

class Document: public boost::intrusive_ref_counter<Document, boost::thread_unsafe_counter>
{
public:
	struct ControlSequence
	{
		ControlSequence(py::object& callable);
		ControlSequence(ControlSequence const&) = default;
		ControlSequence(ControlSequence&&) = default;
		ControlSequence& operator=(ControlSequence const&) = default;
		ControlSequence& operator=(ControlSequence&&) = default;
		py::object m_callable;
		bool m_callWithDocument;
		int m_minGroups, m_maxGroups;
		bool m_groupsRequested;
		bool callWithGroups() const { return m_groupsRequested || m_minGroups >= 0; }
	};
	typedef std::unordered_map<std::u32string, ControlSequence> ControlSequences;

	Document();

	void addInput(fs::path const& fileName);
	void toPDF(fs::path const& fileName);

	ControlSequences& controlSequences() { return m_controlSequences; }
	py::dict& environment() { return m_environment; }
	input::PTokens tokens() const { return m_tokens; }

	PFont font(fs::path const& path, int size) { return m_fontRegistry.font(path, size); }

private:
	ControlSequences m_controlSequences;
	py::dict m_environment;
	std::forward_list<std::tuple<fs::path, std::u32string>> m_inputs;
	input::PTokens m_tokens = std::make_shared<input::Tokens>();
	FontRegistry m_fontRegistry;
	input::InitialPage* m_initialPage = nullptr;
};

//typedef boost::intrusive_ptr<Document> PDocument;


class CSDecoratorFromArgs
{
public:
	CSDecoratorFromArgs(Document::ControlSequences& seqs, std::u32string const& name):
		m_controlSequences(seqs), m_name(name) {};
	py::object operator()(py::object);
private:
	Document::ControlSequences& m_controlSequences;
	std::u32string m_name;
	//unsigned int m_groups;
};

class CSDecorator
{
public:
	CSDecorator(Document::ControlSequences& seqs): m_controlSequences(seqs) {};
	py::object operator()(py::object);
	CSDecoratorFromArgs operator()(std::u32string const&);
	//CSDecoratorFromArgs operator()(std::u32string const&, unsigned int groups);
	//CSDecoratorFromArgs operator()(unsigned int groups);
private:
	Document::ControlSequences& m_controlSequences;
};

}	// namespace xet
