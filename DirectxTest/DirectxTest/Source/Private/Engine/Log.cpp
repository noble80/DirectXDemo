#include "stdafx.h"
#include "Engine\Log.h"

#include <windows.h>
#include <iostream>

using namespace std;

std::stringstream Log::DebugConsole::DeferredMessage;

void Log::DebugConsole::Initialize()
{
	// set up output console
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
}

void Log::DebugConsole::PrintDeferred()
{
	cout << DeferredMessage.str();
	DeferredMessage.str(std::string());
}

void Log::DebugConsole::PrintImmediate(std::string log)
{
	cout << log << endl;
}

