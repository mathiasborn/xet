#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/regex/pending/unicode_iterator.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <pybind11/pybind11.h>

#include "unicode_string_support.h"
#include "xet_input.h"
#include "xet_parser.h"
#include "xet_input_error.h"
#include "xet_document.h"


namespace py = pybind11;
using namespace std::string_literals;

namespace input {

std::u32string loadUTF8TextFile(fs::path const& path)
{
	using namespace std::string_literals;

	fs::ifstream instream(path, std::ios::in);
	if (!instream) throw std::runtime_error("file error"); //error::FileError(path, "Unable to open file.");
	instream.unsetf(std::ios::skipws);      // No white space skipping!
	auto sbuf = std::string{std::istreambuf_iterator<char>(instream.rdbuf()), std::istreambuf_iterator<char>()};
	boost::replace_all(sbuf, "\r\n", "\n");
	boost::replace_all(sbuf, "\r", "\n");

	typedef boost::u8_to_u32_iterator<std::string::const_iterator> Conv;
	
	//r.assign(Conv(u8buffer.cbegin()), Conv(u8buffer.cend()));
	return std::u32string{Conv(sbuf.cbegin()), Conv(sbuf.cend())};
}

/*
void test_xet_input(fs::path const& path)
{
	auto text = loadUTF8TextFile(path);

	auto tokens = parser::parse(text.cbegin(), text.cend());
	std::cout << tokens.size() << std::endl;
}
*/

ActiveToken::operator std::u32string() const
{
	return U"<xet.ActiveToken at "s + uts::toUtf32(this) + U'>';
}

Glue::operator std::u32string() const
{
	return U"xet.Glue("s + uts::toUtf32(m_width) + U", " + uts::toUtf32(m_stretchability) + U", " + uts::toUtf32(m_shrinkability) + U')';
}

Penalty::operator std::u32string() const
{
	return U"xet.Penalty("s + uts::toUtf32(m_value) + U", " + uts::toUtf32(m_width) + U')';
}

ParagraphSeperator::operator std::u32string() const
{
	return U"xet.ParagraphSeperator"s;
}

Push::operator std::u32string() const
{
	return U"xet.Push"s;
}

Pop::operator std::u32string() const
{
	return U"xet.Pop"s;
}

Stream::operator std::u32string() const
{
	return U"xet.Stream("s + uts::toUtf32(m_n) + U')';
}


class TokenVisitor : public boost::static_visitor<>
{
	Tokens& m_tokens;
	fs::path const& m_path;
	std::u32string m_fileName;
	xet::Document& m_doc;
	py::object m_compile, m_exec, m_eval, m_signature;
	py::dict m_argsConverter;
public:
	TokenVisitor(Tokens& tokens, fs::path const& fileName, xet::Document& doc): m_tokens(tokens), m_path(fileName), m_fileName{uts::toUtf32(fileName.wstring())}, m_doc{doc}
	{
		auto builtins = py::module::import("builtins");
		m_compile = builtins.attr("compile");
		m_exec = builtins.attr("exec");
		m_eval = builtins.attr("eval");
		m_argsConverter["___f___"] = m_eval("lambda **kwds: kwds");
		auto inspect = py::module::import("inspect");
		m_signature = inspect.attr("signature");
	};

	void operator()(parser::PyExpr const& a)
	{
		auto name = std::u32string{ a.cs.name.begin(), a.cs.name.end() };
		//auto src = name + a.cs.args ? std::u32string{ a.cs.args.begin(), a.cs.args.end() } : U"()"s;
		auto csi = m_doc.controlSequences().find(name);
		if (csi == m_doc.controlSequences().end())
			throw Error(m_fileName, a.cs.name.begin(), U"Unknown control sequence '"s + name + U"'.");
		py::object r;
		py::dict args_dict;
		if (a.cs.args)
			args_dict = m_eval(U"___f___"s + std::u32string{ a.cs.args->begin(), a.cs.args->end() }, m_doc.environment(), m_argsConverter);
		Groups groups;
		groups.reserve(a.groups.size());
		for(auto const& group: a.groups) groups.push_back(convert(group, m_path, m_doc));
		if (csi->second.groups == 1)
		{
			if (a.groups.size() != 1)
				throw Error(m_fileName, a.cs.name.begin(), U"Control sequence '"s + name + U"' requires one group, but " + uts::toUtf32(a.groups.size()) + U" is/are given.");
			args_dict["group"] = groups[0];
		}
		else if (csi->second.groups > 1)
		{
			if (a.groups.size() != csi->second.groups)
				throw Error(m_fileName, a.cs.name.begin(), U"Control sequence '"s + name + U"' requires " + uts::toUtf32(csi->second.groups) + U" groups, but " + uts::toUtf32(a.groups.size()) + U" is/are given.");
			args_dict["groups"] = groups;
		}
		
		r = csi->second.callable(**args_dict);
		
		if (py::isinstance<py::str>(r))
		{
			m_tokens.emplace_back(input::Text(py::cast<std::u32string>(r)));
		}
		else if (py::isinstance<input::Text>(r) || py::isinstance<input::Glue>(r) || py::isinstance<input::Penalty>(r) || py::isinstance<input::ParagraphSeperator>(r) || py::isinstance<input::Push>(r) || py::isinstance<input::Pop>(r))
		{
			auto t = py::cast<input::Token>(r);
			m_tokens.push_back(t);
		}
		else if (py::isinstance<input::Tokens>(r))
		{
			auto t = py::cast<input::PTokens>(r);
			m_tokens.insert(m_tokens.end(), t->begin(), t->end());
		}
		else if (py::isinstance<input::Actor>(r))
		{
			auto t = py::cast<input::Actor*>(r);
			m_tokens.emplace_back(input::ActiveToken(t));
		}
		else
			throw Error(m_fileName, a.cs.name.begin(), U"Control sequence '"s + name + U"': Invalid return type.");
	}

	void operator()(parser::PyCode const& a)
	{
		auto src = std::u32string(static_cast<std::u32string::size_type>(a.text.begin().line()-1), U'\n') + std::u32string{a.text.begin(), a.text.end()};
		auto code = m_compile(src, m_fileName, "exec");
		m_exec(code, m_doc.environment());
	}

	void operator()(parser::NewParagraph const&)
	{
		m_tokens.emplace_back(
			Token(std::in_place_type_t<ParagraphSeperator>())
		);
	}

	void operator()(parser::Text const& a)
	{
		m_tokens.emplace_back(
			Token(std::in_place_type_t<Text>(), std::u32string{a.text.begin(), a.text.end()})
		);
	}
};

PTokens convert(parser::Tokens const& in, fs::path const& fileName, xet::Document& doc)
{
	auto r = std::make_shared<Tokens>();
	auto visitor = TokenVisitor{*r, fileName, doc};
	for (auto& token: in)
		boost::apply_visitor(visitor, token);
	return r;
}


} // namespace input
