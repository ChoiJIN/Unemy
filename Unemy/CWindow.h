#pragma once
#include <windows.h>
#include <windowsx.h>

#include <mmsystem.h>

#include <ddraw.h>
#include <dsound.h>

#include "dsutil.h"
#include "ddutil.h"

class CWindow
{
public:

	CWindow();
	~CWindow();

	void Init(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow);
	void MainLoop();
	void SetGameProc(void(*ptr)(HWND, LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE, int, int, bool));
	void SetWndProc(LRESULT(*ptr)(HWND, LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE, int, int, UINT, WPARAM, LPARAM));

	// DirectDraw Object (for drawing)
	LPDIRECTDRAW DirectOBJ;

	void ReleaseAll();
	BOOL Fail(HWND hwnd, char *Output);

	void(*pGameProc)(HWND hWnd, LPDIRECTDRAWSURFACE backScreen, LPDIRECTDRAWSURFACE realScreen, int windth, int height, bool fullScreen);
	LRESULT(*pWndProc)(HWND hWnd, LPDIRECTDRAWSURFACE backScreen, LPDIRECTDRAWSURFACE realScreen, int windth, int height, UINT, WPARAM, LPARAM);

	// Window size
	static const int WIN_WIDTH = 800;
	static const int WIN_HEIGHT = 600;
	static const int REDNER_TIME = 20;	// milliseconds

private:
	HWND hWnd;
	MSG msg;

	// Screen
	LPDIRECTDRAWSURFACE  RealScreen;
	LPDIRECTDRAWSURFACE  BackScreen;
	LPDIRECTDRAWCLIPPER	 ClipScreen;

	BOOL GameMode(HINSTANCE hInstance, int nCmdShow, DWORD  x, DWORD  y, DWORD  bpp, int fullScreen);

	static LRESULT	CALLBACK StaticWndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc
	LRESULT WndProc(UINT, WPARAM, LPARAM);
};

