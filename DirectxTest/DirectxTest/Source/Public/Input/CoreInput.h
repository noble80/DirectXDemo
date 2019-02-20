#pragma once
#include "stdafx.h"

#ifndef _WINDEF_
struct HINSTANCE__; // Forward or never
typedef HINSTANCE__* HINSTANCE;

typedef unsigned int        UINT;

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

enum class KeyCode
{
	Space = 0x20, Control = 0x11,
	A = 0x41, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	Zero = 0x30, One, Two, Three, Four, Five, Six, Seven, Eight, Nine,
	Num0 = 0x60, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num10,
	Plus = 0x6B, Minus = 0x6D,
	Left = 0x25, Up, Right, Down
};

enum KeyState
{
	DownFirst = 1, Release, Down
};

class CoreInput
{
public:
	static void InitializeInput(HWND hwnd);
	static void ResetAxes();
	static void GatherInput(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static void UpdateInput();
	inline static int32_t GetMouseX() { return mouseX; };
	inline static int32_t GetMouseY() { return mouseY; };

	static KeyState GetKeyState(KeyCode target);
private:
	static int32_t mouseX;
	static int32_t mouseY;
	static uint8_t keyStates[256];
};