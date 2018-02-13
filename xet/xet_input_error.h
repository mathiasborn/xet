#pragma once

#include <stdexcept>
#include "xet_line_pos_iterator.h"

namespace input {

class Error: public std::runtime_error
{
	typedef std::runtime_error Super;
public:
	Error(std::u32string const& fileName, LinePosIterator const& start, std::u32string const& msg);
};

}	// namespace input
