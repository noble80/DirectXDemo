#pragma once
#include "stdafx.h"

constexpr auto WINDOW_FLAGS_FULLSCREEN = 0x1;
#define WINDOW_FLAGS_DEFAULT (WINDOW_FLAGS_FULLSCREEN)

#ifndef _WINDEF_
struct HINSTANCE__; // Forward or never
typedef HINSTANCE__* HINSTANCE;

#if defined(_WIN64)
typedef __int64 INT_PTR, *PINT_PTR;
typedef unsigned __int64 UINT_PTR, *PUINT_PTR;
typedef __int64 LONG_PTR, *PLONG_PTR;
typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;
#else
typedef __w64 int INT_PTR, *PINT_PTR;
typedef __w64 unsigned int UINT_PTR, *PUINT_PTR;
typedef __w64 long LONG_PTR, *PLONG_PTR;
typedef __w64 unsigned long ULONG_PTR, *PULONG_PTR;
#endif

typedef UINT_PTR            WPARAM;
typedef LONG_PTR            LPARAM;
typedef LONG_PTR            LRESULT;
struct HWND__;
typedef HWND__* HWND;

#endif

class Renderer;
class Window
{
public:
	Window();
	~Window();

	friend class Renderer;

	bool Initialize(Vector2 dimensions, uint32_t flags, std::wstring appName);
	bool Update();
	bool Shutdown();

	inline bool IsFullscreen() { return (m_WindowFlags & WINDOW_FLAGS_FULLSCREEN) != 0; }

	inline HWND GetHandle() { return m_WindowHandle; };
	Vector2 GetDimensions() { return m_Dimensions; }

	void HandleResize();
	void HandleFullscreenChange(LPARAM lparam, WPARAM wparam);

	bool Paused = false;

	inline Renderer* GetRenderer() { return m_Renderer; };
private:


	Vector2 m_Dimensions;
	uint32_t m_WindowFlags;
	HWND m_WindowHandle;
	HINSTANCE m_HInstance;
	std::wstring m_AppName;
	Renderer* m_Renderer;
};

