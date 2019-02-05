#pragma once
#include "stdafx.h"

namespace StringUtility
{
	std::string utf8_encode(const std::wstring &wstr);
	// Convert an UTF8 string to a wide Unicode String
	std::wstring utf8_decode(const std::string &str);
}