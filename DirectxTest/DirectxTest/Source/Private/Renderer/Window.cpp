#include "stdafx.h"
#include "Renderer/Window.h"
#include "Input\CoreInput.h"

#define WIN32_LEAN_AND_MEAN //Gets rid of bloat on Windows.h
#include <Windows.h> 
#include <windowsx.h>

// Main Windows procedure
LRESULT CALLBACK WindowProc(
	HWND hWnd, //windows handle
	UINT message, // message on what to do 
	WPARAM wParam, //first parameter
	LPARAM lParam); //second parameter

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// switch based on the input message
	switch(message)
	{
		// We are told to close the app
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_INPUT:
		{
			CoreInput::GatherInput(hWnd, message, wParam, lParam);
		}
		break;
	}

	// Any other messages, handle the default way
	return DefWindowProc(hWnd, message, wParam, lParam);
}

Window::Window()
{
	m_WindowHandle = nullptr;
}


Window::~Window()
{
}

bool Window::Initialize(Vector2 dimensions, uint32_t flags, std::wstring appName)
{
	m_AppName = appName;
	m_WindowFlags = flags;
	m_Dimensions = dimensions;
	m_HInstance = GetModuleHandleW(0);

	int posX, posY;

	// window info
	WNDCLASSEX winInfo;

	// zero out the win info struct
	ZeroMemory(&winInfo, sizeof(WNDCLASSEX));

	// Set win info struct data
	winInfo.cbSize = sizeof(WNDCLASSEX); //sets size of struct. Must because of backwards compatibility
	winInfo.style = CS_HREDRAW | CS_VREDRAW; //redraws whole window on horizontal or vertical modification
	winInfo.lpfnWndProc = WindowProc; //ptr to the window procedure
	winInfo.hInstance = m_HInstance; //ptr to app instance
	winInfo.hCursor = LoadCursor(nullptr, IDC_ARROW); //set cursor for this window
	//winInfo.hbrBackground = (HBRUSH)COLOR_WINDOW; //brush or color used for background
	winInfo.lpszClassName = m_AppName.c_str(); // Window class name. Local to current process

	 // register window class
	RegisterClassEx(&winInfo);

	// calculate the size of the client area

	/*if((m_WindowFlags & WINDOW_FLAGS_FULLSCREEN) != 0)
	{
		m_Dimensions.x = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
		m_Dimensions.y = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
		DEVMODEW screenMode;
		memset(&screenMode, 0, sizeof(screenMode));
		screenMode.dmSize = sizeof(screenMode);
		screenMode.dmPelsWidth = static_cast<unsigned long>(m_Dimensions.x); ;
		screenMode.dmPelsHeight = static_cast<unsigned long>(m_Dimensions.y); ;
		screenMode.dmBitsPerPel = 32;
		screenMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&screenMode, CDS_FULLSCREEN);

		posY = posX = 0;
	}*/

	RECT wr = {0, 0, static_cast<int>(m_Dimensions.x), static_cast<int>(m_Dimensions.y)};    // set the size
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);    // adjust the size

	posX = GetSystemMetrics(SM_CXSCREEN) / 2 - (wr.right - wr.left)/2;
	posY = GetSystemMetrics(SM_CYSCREEN) / 2 - (wr.bottom - wr.top)/2;

	// create the window and use the result as the handle
	m_WindowHandle = CreateWindowEx(WS_EX_APPWINDOW,
		m_AppName.c_str(),    // Window class name again
		m_AppName.c_str(),  // window title text
		//WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,    // style. Being later than extended style may be related to WINAPI
		WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_SIZEBOX,
		posX,    // x pos
		posY,    // y pos
		wr.right - wr.left,    // width of the window
		wr.bottom - wr.top,    // height of the window
		nullptr,    // parent handle
		nullptr,    // menu HMENU?
		GetModuleHandleW(0),    // app instance
		nullptr);    // parameters passed to new window (32 bit value)

	// show the window
	if(m_WindowHandle)
	{
		CoreInput::InitializeInput(m_WindowHandle);

		ShowWindow(
			m_WindowHandle,
			SW_SHOW
		);
		return true;
	}

	return false;
}

bool Window::Update()
{
	MSG msg{};

	// Check to see if any messages are waiting in the queue
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		// translate keystroke messages into the right format
		TranslateMessage(&msg);

		// send the message to the WindowProc function
		DispatchMessage(&msg);

		// check to see if it's time to quit
		if(msg.message == WM_QUIT)
			return false;
	}


	return true;
}

bool Window::Shutdown()
{

	DestroyWindow(m_WindowHandle);
	m_WindowHandle = nullptr;

	UnregisterClass(m_AppName.c_str(), m_HInstance);
	m_HInstance = nullptr;

	return true;
}
