#include "stdafx.h"
#include "Input\CoreInput.h"

#define WIN32_LEAN_AND_MEAN //Gets rid of bloat on Windows.h
#include <Windows.h> 
#include <windowsx.h>

#include "Engine\Log.h"
#include <iostream>

int32_t CoreInput::mouseX = 0;
int32_t CoreInput::mouseY = 0;
uint8_t CoreInput::keyStates[256] {};


void CoreInput::InitializeInput(HWND hwnd)
{
	RAWINPUTDEVICE Rid[2];

	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x02;
	Rid[0].dwFlags = 0;   // adds HID mouse and also ignores legacy mouse messages
	Rid[0].hwndTarget = 0;

	Rid[1].usUsagePage = 0x01;
	Rid[1].usUsage = 0x06;
	Rid[1].dwFlags = 0;   // adds HID keyboard and also ignores legacy keyboard messages
	Rid[1].hwndTarget = 0;

	if(RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE)
	{
		DWORD error = GetLastError();
		DebugBreak();
		exit(EXIT_FAILURE);
	}

	ZeroMemory(keyStates, sizeof(uint8_t) * 256);
}

void CoreInput::ResetAxes()
{
	mouseX = 0;
	mouseY = 0;
}

void CoreInput::GatherInput(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT dwSize = sizeof(RAWINPUT);
	RAWINPUT raw {};

	GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
		&raw, &dwSize, sizeof(RAWINPUTHEADER));


	if(raw.header.dwType == RIM_TYPEMOUSE)
	{
		 CoreInput::mouseX += raw.data.mouse.lLastX;
		 CoreInput::mouseY += raw.data.mouse.lLastY;
	}
	if(raw.header.dwType == RIM_TYPEKEYBOARD)
	{
		keyStates[raw.data.keyboard.VKey] &= 0xfe;
		keyStates[raw.data.keyboard.VKey] |= 1 - (RI_KEY_BREAK & raw.data.keyboard.Flags);
	}
}

KeyState CoreInput::GetKeyState(KeyCode target)
{
	return static_cast<KeyState>(0x3 & keyStates[static_cast<int>(target)]);
}
