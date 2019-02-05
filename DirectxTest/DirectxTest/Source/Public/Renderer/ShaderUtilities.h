#pragma once
#include "stdafx.h"

namespace ShaderUtilities
{
	bool LoadShaderFromFile(std::string File, std::vector<std::byte>& byteCode);
}
