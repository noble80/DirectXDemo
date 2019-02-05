#include "stdafx.h"

#include "Renderer/ShaderUtilities.h"

#include <fstream>

using namespace std;

bool ShaderUtilities::LoadShaderFromFile(std::string path, vector<byte>& byteCode)
{
	basic_ifstream<byte> stream(path, std::ios::binary);

	if(stream.is_open())
	{
		byteCode = vector<byte>((std::istreambuf_iterator<byte>(stream)),
			std::istreambuf_iterator<byte>());

		stream.close();
		return true;
	}

	return false;
}
