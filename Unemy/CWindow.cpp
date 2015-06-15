#include "CWindow.h"

CWindow::CWindow()
{
}


CWindow::~CWindow()
{
}

void CWindow::SetGameProc(void(*ptr)(HWND, LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE, int, int, bool))
{
	this->pGameProc = ptr;
}

void CWindow::SetWndProc(LRESULT(*ptr)(HWND, LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE, int, int, UINT, WPARAM, LPARAM))
{
	this->pWndProc = ptr;
}

BOOL CWindow::GameMode(HINSTANCE hInstance, int nCmdShow, DWORD x, DWORD y, DWORD bpp, int FullScreen)
{
	WNDCLASS wc;
	DDSURFACEDESC ddsd;
	DDSCAPS ddscaps;
	LPDIRECTDRAW pdd;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = (WNDPROC)StaticWndProc;
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
		hWnd = CreateWindowEx(
			0, "GameProg", NULL, WS_POPUP, 0, 0,
			GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
			NULL, NULL, hInstance, NULL);
	}
	else{
		hWnd = CreateWindow("GameProg", "WindowMode",
			WS_OVERLAPPEDWINDOW, 0, 0, x, y, NULL, NULL, hInstance, NULL);
	}
	if (!hWnd) return FALSE;

	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);


	// 다이렉트 드로우(DD) 생성
	if (FAILED(DirectDrawCreate(NULL, &pdd, NULL)))
		return Fail(hWnd, "DirectDrawCreate");
	// DD에 연결
	if (FAILED(pdd->QueryInterface(IID_IDirectDraw, (LPVOID *)&DirectOBJ)))
		return Fail(hWnd, "QueryInterface");

	// 윈도우 핸들의 협력 단계를 설정한다.
	if (FullScreen){
		if (FAILED(DirectOBJ->SetCooperativeLevel(hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN)))
			return Fail(hWnd, "SetCooperativeLevel");
		// Set full screen display mode
		if (FAILED(DirectOBJ->SetDisplayMode(x, y, bpp)))
			return Fail(hWnd, "SetDisplayMode");

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;
		if (FAILED(DirectOBJ->CreateSurface(&ddsd, &RealScreen, NULL)))
			return Fail(hWnd, "CreateSurface");

		memset(&ddscaps, 0, sizeof(ddscaps));
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		if (FAILED(RealScreen->GetAttachedSurface(&ddscaps, &BackScreen)))
			return Fail(hWnd, "GetAttachedSurface");
	}
	else{
		if (FAILED(DirectOBJ->SetCooperativeLevel(hWnd, DDSCL_NORMAL)))
			return Fail(hWnd, "SetCooperativeLevel");

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		ddsd.dwBackBufferCount = 0;
		if (FAILED(DirectOBJ->CreateSurface(&ddsd, &RealScreen, NULL)))
			return Fail(hWnd, "CreateSurface");

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		ddsd.dwWidth = x;
		ddsd.dwHeight = y;
		if (FAILED(DirectOBJ->CreateSurface(&ddsd, &BackScreen, NULL)))
			return Fail(hWnd, "CreateSurface");

		if (FAILED(DirectOBJ->CreateClipper(0, &ClipScreen, NULL)))
			return Fail(hWnd, "CreateClipper");

		if (FAILED(ClipScreen->SetHWnd(0, hWnd)))
			return Fail(hWnd, "SetHWnd");

		if (FAILED(RealScreen->SetClipper(ClipScreen)))
			return Fail(hWnd, "SetClipper");

		SetWindowPos(hWnd, NULL, 0, 0, x, y, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	SetFocus(hWnd);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	ShowCursor(TRUE);

	return TRUE;
}

void CWindow::ReleaseAll()
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

BOOL CWindow::Fail(HWND hwnd, char *Output)
{
	ShowWindow(hwnd, SW_HIDE);
	MessageBox(hwnd, Output, "Game Programming", MB_OK);
	DestroyWindow(hwnd);
	return FALSE;
}

BOOL CWindow::_InitDirectSound(void)
{
	if (DirectSoundCreate(NULL, &SoundOBJ, NULL) == DS_OK)
	{
		if (SoundOBJ->SetCooperativeLevel(hWnd, DSSCL_PRIORITY) != DS_OK) return FALSE;

		memset(&DSB_desc, 0, sizeof(DSBUFFERDESC));
		DSB_desc.dwSize = sizeof(DSBUFFERDESC);
		DSB_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

		if (SoundOBJ->CreateSoundBuffer(&DSB_desc, &SoundDSB, NULL) != DS_OK) return FALSE;
		SoundDSB->SetVolume(0);
		SoundDSB->SetPan(0);
		return TRUE;
	}
	return FALSE;
}

void CWindow::Init(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	//if (!) result_mode = false;
	GameMode(hInstance, nCmdShow, width, height, 32, 0);

	// Set tick frequency (20ms per frame = 50 fps)
	SetTimer(hWnd, 1, 20, NULL);

	if (!_InitDirectSound())
	{
		// SOUND FAIL
		exit(1);
	}

	return;
}

void CWindow::MainLoop()
{
	while (GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DestroyWindow(hWnd);
}

LRESULT	CALLBACK CWindow::StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CWindow* pParent;

	// Get pointer to window
	if (message == WM_CREATE)
	{
		pParent = (CWindow*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)pParent);
	}
	else
	{
		pParent = (CWindow*)GetWindowLongPtr(hWnd, GWL_USERDATA);
		if (!pParent) return DefWindowProc(hWnd, message, wParam, lParam);
	}

	pParent->hWnd = hWnd;
	return pParent->WndProc(message, wParam, lParam);
}

LRESULT CWindow::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	return pWndProc(hWnd, BackScreen, RealScreen, width, height, message, wParam, lParam);
}
