#pragma once
#include "stdafx.h"

constexpr auto WINDOW_FLAGS_FULLSCREEN = 0x1;
#define WINDOW_FLAGS_DEFAULT (WINDOW_FLAGS_FULLSCREEN)

#ifndef _WINDEF_
struct HINSTANCE__; // Forward or never
typedef HINSTANCE__* HINSTANCE;

struct HWND__;
typedef HWND__* HWND;
#endif

class Renderer;
class Window
{
public:
	Window();
	~Window();

	bool Initialize(Vector2 dimensions, uint32_t flags, std::wstring appName);
	bool Update();
	bool Shutdown();

	inline bool IsFullscreen() { return (m_WindowFlags & WINDOW_FLAGS_FULLSCREEN) != 0; }

	inline HWND GetHandle() { return m_WindowHandle; };
	Vector2 GetDimensions() { return m_Dimensions; }
private:
	Vector2 m_Dimensions;
	uint32_t m_WindowFlags;
	HWND m_WindowHandle;
	HINSTANCE m_HInstance;
	std::wstring m_AppName;
};

