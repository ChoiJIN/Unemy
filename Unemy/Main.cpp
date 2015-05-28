#include <windows.h>

#include "CWindow.h"

#include "GameState.h"


/************************************************************************/
/* Declaration                                                          */
/************************************************************************/
void changeScreen(ScreenState state);
void changeBackground(ScreenState state);
char* backgroundFromEnum(ScreenState state);

/************************************************************************/
/* Global Variable                                                      */
/************************************************************************/
CWindow window;
LPDIRECTDRAWSURFACE BackImage;

/************************************************************************/
/* WndProc                                                              */
/************************************************************************/
LRESULT WndProc(HWND hWnd, LPDIRECTDRAWSURFACE BackScreen, LPDIRECTDRAWSURFACE RealScreen, int winWidth, int winHeight,
	UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_DESTROY:
		window.ReleaseAll();
		PostQuitMessage(0);
		break;

	case WM_TIMER:
		//_GameProc(false);
		window.pGameProc(hWnd, BackScreen, RealScreen, winWidth, winHeight, false);
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
/* GameProc                                                             */
/************************************************************************/
void GameProc(HWND hWnd, LPDIRECTDRAWSURFACE BackScreen, LPDIRECTDRAWSURFACE RealScreen, int winWidth, int winHeight, bool fullScreen)
{
	// Clear Back Ground
	RECT	BackRect = { 0, 0, winWidth, winHeight };
	BackScreen->BltFast(0, 0, BackImage, &BackRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);

	//////////////////////////////

	//////////////////////////////

	if (fullScreen)
		RealScreen->Flip(NULL, DDFLIP_WAIT);
	else{
		RECT WinRect;
		RECT Rect = { 0, 0, winWidth, winHeight };

		GetWindowRect(hWnd, &WinRect);
		RealScreen->Blt(&WinRect, BackScreen, &Rect, DDBLT_WAIT, NULL);
	}
}

/************************************************************************/
/* WinMain                                                              */
/************************************************************************/
int __stdcall WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	// Window Initialization
	window.SetGameProc(GameProc);
	window.SetWndProc(WndProc);
	window.Init(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

	// [Game] ////////////////////////////////////////////////////////////////

	current.screen = ScreenState::GAME;

	BackImage = DDLoadBitmap(window.DirectOBJ, backgroundFromEnum(ScreenState::START), 0, 0);
	DDSetColorKey(BackImage, RGB(0, 0, 0));

	//////////////////////////////////////////////////////////////////////////

	// Loop Start
	window.MainLoop();

	return TRUE;
}

/************************************************************************/
/* Implementation                                                       */
/************************************************************************/
void changeScreen(ScreenState state)
{
	if (current.screen == state) {
		// do nothing
	}
	else {
		current.screen = state;
		changeBackground(current.screen);
	}
}

void changeBackground(ScreenState state)
{
	BackImage = DDLoadBitmap(window.DirectOBJ, backgroundFromEnum(state), 0, 0);
	DDSetColorKey(BackImage, RGB(0, 0, 0));
}

char* backgroundFromEnum(ScreenState state)
{
	switch (state)
	{
	case MENU:
		return "menu.bmp";
		break;
	case NETWORK:
		return "network.bmp";
		break;
	case GAME:
		return "game.bmp";
		break;
	default:
		return "back.bmp";
		break;
	}
}