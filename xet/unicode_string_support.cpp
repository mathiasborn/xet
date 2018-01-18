/***************************************************************************
	xet
	Copyright (C) 2018 Mathias Born

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

#include "stdafx.h"
#include "unicode_string_support.h"
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
#include <boost/regex/pending/unicode_iterator.hpp>

using namespace std;
namespace fs = boost::filesystem;

namespace uts {

std::string toUtf8(std::u32string const& s)
{
	auto r = std::string{};
	typedef boost::u32_to_u8_iterator<std::u32string::const_iterator> Conv;
	r.assign(Conv(s.begin()), Conv(s.end()));
	return r;
}

std::u32string toUtf32(std::string const& s)
{
	auto r = std::u32string{};
	typedef boost::u8_to_u32_iterator<std::string::const_iterator> Conv;
	r.assign(Conv(s.begin()), Conv(s.end()));
	return r;
}

std::u32string toUtf32(std::wstring const& s)
{
	auto r = std::u32string{};
	typedef boost::u16_to_u32_iterator<std::wstring::const_iterator> Conv;
	r.assign(Conv(s.begin()), Conv(s.end()));
	return r;
}

std::u32string toUtf32(char const* s, size_t size)
{
	auto r = std::u32string{};
	typedef boost::u8_to_u32_iterator<char const*> Conv;
	r.assign(Conv(s), Conv(s + size));
	return r;
}

};	// namespace uts
