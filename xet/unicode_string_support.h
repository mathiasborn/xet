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
#ifndef _UNICODE_STRING_SUPPORTH_
#define _UNICODE_STRING_SUPPORTH_

#ifndef BOOST_FILESYSTEM_VERSION
#	define BOOST_FILESYSTEM_VERSION 3
#endif

#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <unordered_map>
#include <boost/optional.hpp>
#include <boost/regex/pending/unicode_iterator.hpp>
#include <boost/filesystem.hpp>


namespace uts {

std::string toUtf8(std::u32string const& s);

inline std::string toUtf8(std::string const& s)
{
	return s;
}

std::u32string toUtf32(std::string const& s);
std::u32string toUtf32(std::wstring const& s);
std::u32string toUtf32(char const* s, size_t size);

inline std::u32string toUtf32(std::u32string const& s)
{
	return s;
}

// traits declarations to deduce string type from iterator type
template<typename T>
struct StringTypeOf;

template<> struct StringTypeOf<char*>
{
	typedef std::string Type;
};

template<> struct StringTypeOf<char const*>
{
	typedef std::string Type;
};

template<> struct StringTypeOf<std::string::iterator>
{
	typedef std::string Type;
};

template<> struct StringTypeOf<std::string::const_iterator>
{
	typedef std::string Type;
};

template<> struct StringTypeOf<std::u32string::iterator>
{
	typedef std::u32string Type;
};

template<> struct StringTypeOf<std::u32string::const_iterator>
{
	typedef std::u32string Type;
};

}; // namespace uts

#endif	// _UNICODE_STRING_SUPPORTH_
