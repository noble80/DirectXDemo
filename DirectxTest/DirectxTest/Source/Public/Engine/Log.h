#pragma once
	

#include "stdafx.h"
#include <sstream>

namespace Log
{
	namespace DebugConsole
	{
		extern std::stringstream DeferredMessage;
		void Initialize();
		void PrintImmediate(std::string log);
		void PrintDeferred();
	}
}