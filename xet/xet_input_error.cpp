#include "stdafx.h"
#include <string>
#include <boost/lexical_cast.hpp>
#include "xet_input_error.h"
#include "unicode_string_support.h"

using namespace std::string_literals;


namespace input {

static std::u32string errorDescription(std::u32string const& fileName, LinePosIterator const& start, std::u32string const& msg)
{
	auto r = U"File '"s + fileName + U"', line " + uts::toUtf32(boost::lexical_cast<std::string>(start.line())) + U": " + msg;
	return r;
}

Error::Error(std::u32string const& fileName, LinePosIterator const& start, std::u32string const& msg): Super(uts::toUtf8(errorDescription(fileName, start, msg)))
{}


}	// namespace input
