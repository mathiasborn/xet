#include "stdafx.h"
#include "platform.h"
#include <Windows.h>

#ifdef BOOST_OS_WINDOWS

namespace platform {

fs::path const& appFileName()
{
	static fs::path s_appFileName;

	if (s_appFileName.empty())
	{
		// This is taken from the Qt source code and adapted.
		/*
		GetModuleFileName() returns the length of the module name, when it has
		space to store it and 0-terminate; this return is necessarily smaller than
		the buffer size, as it doesn't include the terminator. When it lacks
		space, the function returns the full size of the buffer and fills the
		buffer, truncating the full path to make it fit. We have reports that
		GetModuleFileName sometimes doesn't set the error number to
		ERROR_INSUFFICIENT_BUFFER, as claimed by the MSDN documentation; so we
		only trust the answer when the return is actually less than the buffer
		size we pass in. (When truncating, except on XP, it does so by enough to
		still have space to 0-terminate; in either case, it fills the claimed
		space and returns the size of the space. While XP might thus give us the
		full name, without a 0 terminator, and return its actual length, we can
		never be sure that's what's happened until a later call with bigger buffer
		confirms it by returning less than its buffer size.)
		*/
		// Full path may be longer than MAX_PATH - expand until we have enough space:
		auto space = std::wstring{MAX_PATH + 1, L' '};
		DWORD v;
		size_t size = 1;
		do {
			size += MAX_PATH;
			space.resize(size);
			v = GetModuleFileName(NULL, &space[0], DWORD(space.size()));
		} while (v >= size);
		s_appFileName.assign(space.cbegin(), space.cbegin() + v);
	}
	return s_appFileName;
}

}; // namespace Platform

#endif // BOOST_OS_WINDOWS
