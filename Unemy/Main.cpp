#include <windows.h>
#include <windowsx.h>

#include <ddraw.h>
#include <dsound.h>

#include "dsutil.h"
#include "ddutil.h"

HWND MainHwnd;
LPDIRECTDRAW         DirectOBJ;

/************************************************************************/
/* Variables                                                            */
/************************************************************************/
// Window size
const int window_width = 800;
const int window_height = 600;

// Screen
LPDIRECTDRAWSURFACE  RealScreen;
LPDIRECTDRAWSURFACE  BackScreen;
LPDIRECTDRAWCLIPPER	 ClipScreen;

// Image (Global)
LPDIRECTDRAWSURFACE  BackImage;

// 

BOOL Fail(HWND hwnd, char *Output)
{
	ShowWindow(hwnd, SW_HIDE);
	MessageBox(hwnd, Output, "Game Programming", MB_OK);
	DestroyWindow(hwnd);
	return FALSE;
}

void _ReleaseAll(void)
{
	if (DirectOBJ != NULL)
	{

		if (BackScreen != NULL)
		{
			BackScreen->Release();
			BackScreen = NULL;
		}
		if (RealScreen != NULL)
		{
			RealScreen->Release();
			RealScreen = NULL;
		}

		DirectOBJ->Release();
		DirectOBJ = NULL;
	}
}

/************************************************************************/
/* Game Loop                                                            */
/************************************************************************/
void _GameProc(int FullScreen)
{
	// Clear Back Ground
	RECT	BackRect = { 0, 0, window_width, window_height };
	BackScreen->BltFast(0, 0, BackImage, &BackRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);

	//////////////////////////////

	//////////////////////////////

	if (FullScreen)
		RealScreen->Flip(NULL, DDFLIP_WAIT);
	else{
		RECT WinRect;
		RECT Rect = { 0, 0, window_width, window_height };

		GetWindowRect(MainHwnd, &WinRect);
		RealScreen->Blt(&WinRect, BackScreen, &Rect, DDBLT_WAIT, NULL);
	}
}

/************************************************************************/
/* Message Procedure                                                    */
/************************************************************************/
long FAR PASCAL WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_DESTROY:
		_ReleaseAll();
		PostQuitMessage(0);
		break;

	case WM_TIMER:
		_GameProc(false);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
		case VK_F12:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			return 0;
		case VK_LEFT:
			return 0;
		case VK_RIGHT:
			return 0;
		case VK_UP:
			return 0;
		case VK_DOWN:
			return 0;
		case VK_SPACE:
			break;
		case VK_CONTROL:
			break;
		}
		break;

	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

/************************************************************************/
/* Mode Setting                                                         */
/************************************************************************/
BOOL _GameMode(HINSTANCE hInstance, int nCmdShow, DWORD  x, DWORD  y, DWORD  bpp, int FullScreen)
{
	WNDCLASS wc;
	DDSURFACEDESC ddsd;
	DDSCAPS ddscaps;
	LPDIRECTDRAW pdd;

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetStockBrush(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "GameProg";
	RegisterClass(&wc);

	if (FullScreen){
		MainHwnd = CreateWindowEx(
			0, "GameProg", NULL, WS_POPUP, 0, 0,
			GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
			NULL, NULL, hInstance, NULL);
	}
	else{
		MainHwnd = CreateWindow("GameProg", "WindowMode",
			WS_OVERLAPPEDWINDOW, 0, 0, x, y, NULL, NULL, hInstance, NULL);
	}
	if (!MainHwnd) return FALSE;


	// 다이렉트 드로우(DD) 생성
	if (FAILED(DirectDrawCreate(NULL, &pdd, NULL)))
		return Fail(MainHwnd, "DirectDrawCreate");
	// DD에 연결
	if (FAILED(pdd->QueryInterface(IID_IDirectDraw, (LPVOID *)&DirectOBJ)))
		return Fail(MainHwnd, "QueryInterface");

	// 윈도우 핸들의 협력 단계를 설정한다.
	if (FullScreen){
		if (FAILED(DirectOBJ->SetCooperativeLevel(MainHwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN)))
			return Fail(MainHwnd, "SetCooperativeLevel");
		// Set full screen display mode
		if (FAILED(DirectOBJ->SetDisplayMode(x, y, bpp)))
			return Fail(MainHwnd, "SetDisplayMode");

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;
		if (FAILED(DirectOBJ->CreateSurface(&ddsd, &RealScreen, NULL)))
			return Fail(MainHwnd, "CreateSurface");

		memset(&ddscaps, 0, sizeof(ddscaps));
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		if (FAILED(RealScreen->GetAttachedSurface(&ddscaps, &BackScreen)))
			return Fail(MainHwnd, "GetAttachedSurface");
	}
	else{
		if (FAILED(DirectOBJ->SetCooperativeLevel(MainHwnd, DDSCL_NORMAL)))
			return Fail(MainHwnd, "SetCooperativeLevel");

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		ddsd.dwBackBufferCount = 0;
		if (FAILED(DirectOBJ->CreateSurface(&ddsd, &RealScreen, NULL)))
			return Fail(MainHwnd, "CreateSurface");

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		ddsd.dwWidth = x;
		ddsd.dwHeight = y;
		if (FAILED(DirectOBJ->CreateSurface(&ddsd, &BackScreen, NULL)))
			return Fail(MainHwnd, "CreateSurface");

		if (FAILED(DirectOBJ->CreateClipper(0, &ClipScreen, NULL)))
			return Fail(MainHwnd, "CreateClipper");

		if (FAILED(ClipScreen->SetHWnd(0, MainHwnd)))
			return Fail(MainHwnd, "SetHWnd");

		if (FAILED(RealScreen->SetClipper(ClipScreen)))
			return Fail(MainHwnd, "SetClipper");

		SetWindowPos(MainHwnd, NULL, 0, 0, x, y, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	SetFocus(MainHwnd);
	ShowWindow(MainHwnd, nCmdShow);
	UpdateWindow(MainHwnd);
	ShowCursor(TRUE);

	return TRUE;
}

/************************************************************************/
/* WinMain                                                              */
/************************************************************************/
int __stdcall WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	MSG msg;

	if (!_GameMode(hInstance, nCmdShow, window_width, window_height, 32, 0)) return FALSE;

	// Background Image
	BackImage = DDLoadBitmap(DirectOBJ, "back.BMP", 0, 0);
	DDSetColorKey(BackImage, RGB(0, 0, 0));

	// Set tick frequency (20ms per frame = 50 fps)
	SetTimer(MainHwnd, 1, 20, NULL);

	// Loop Message
	while (GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DestroyWindow(MainHwnd);

	return TRUE;
}