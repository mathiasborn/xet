#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/regex/pending/unicode_iterator.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "xet_input.h"
#include "xet_parser.h"

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

void test_xet_input(fs::path const& path)
{
	auto text = loadUTF8TextFile(path);

	auto tokens = parser::parse(text.cbegin(), text.cend());
	std::cout << tokens.size() << std::endl;
}

namespace input {

class TokenVisitor : public boost::static_visitor<>
{
	Tokens& m_tokens;
public:
	TokenVisitor(Tokens& tokens): m_tokens(tokens) {};

	void operator()(parser::PyExpr const& a)
	{}

	void operator()(parser::PyCode const& a)
	{}

	void operator()(parser::NewParagraph const& a)
	{}

	void operator()(parser::Text const& a)
	{
		m_tokens.push_back(std::make_shared<Text>(std::u32string{a.text.begin(), a.text.end()}));
	}

	void operator()(parser::Block const& a)
	{}
};

Tokens convert(parser::Tokens const& in)
{
	Tokens r;
	auto visitor = TokenVisitor{r};
	for (auto& token: in)
		boost::apply_visitor(visitor, token);
	return r;
}


} // namespace input
