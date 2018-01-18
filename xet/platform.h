#pragma once

#ifndef _PLATFORM_WINDOWS_H_
#define _PLATFORM_WINDOWS_H_

#include "stdafx.h"
#include <boost/predef.h>
#include <boost/filesystem.hpp>

namespace platform
{
	fs::path const& appFileName();
}; // namespace Platform

#endif // _PLATFORM_WINDOWS_H_
