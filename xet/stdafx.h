// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#if defined _MSC_VER
#pragma warning(disable:4180)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <string>

#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
//#define _HAS_AUTO_PTR_ETC 1
#define BOOST_LIB_DIAGNOSTIC

// TODO: reference additional headers your program requires here
/*
#ifdef _DEBUG
#	define BOOST_DEBUG_PYTHON
#	define BOOST_LINKING_PYTHON
#else
//#	define Py_LIMITED_API
#endif
*/
#define BOOST_SPIRIT_UNICODE

#define BOOST_SPIRIT_DEBUG
#define BOOST_SPIRIT_DEBUG_OUT std::cout


#include <boost/predef.h>
#include <boost/filesystem.hpp>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>


namespace fs = boost::filesystem;
namespace qi = boost::spirit::qi;

// http://stackoverflow.com/questions/10474571/how-to-match-unicode-characters-with-boostspirit
// http://boost-spirit.com/home/wp-content/uploads/2010/05/A_Framework_for_RAD_Spirit.pdf
// http://www.boost.org/doc/libs/1_62_0/libs/regex/doc/html/boost_regex/ref/internal_details/uni_iter.html
// http://www.nubaria.com/en/blog/?p=289
// http://www.cprogramming.com/tutorial/unicode.html

// This is needed to make qi::phrase_parse work with ustring::const_iterator.
namespace boost {
template<> struct is_scalar<std::u32string::const_iterator> : public true_type {};
}

constexpr double toPDF(int64_t u)
{
	return static_cast<double>(u)*(1.0e-9 / (2.54 / 100.0 / 72.0));	// nm -> 1/72in
}
