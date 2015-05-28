#include <windows.h>
#include <vector>
#include <string>

#include "CWindow.h"

#include "GameState.h"

using namespace std;

/************************************************************************/
/* Declaration                                                          */
/************************************************************************/
void loadImages();						// 모든 이미지를 로딩 함.
void changeScreen(Screen state);		// Screen 전환 함수
void changeBackground(Screen state);	// Background 전환 함수
char* backgroundFromEnum(Screen state);	// Enum으로부터 Background의 파일 경로를 받아옴.

/************************************************************************/
/* Global Variable                                                      */
/************************************************************************/
CWindow window;
LPDIRECTDRAWSURFACE BackImage;
struct Color {
	int r, g, b;
};
Color colorKey = { 0, 0, 0 };	// 현재 컬러키가 먹히지 않음. 무조건 검은색으로..

/************************************************************************/
/* Resources                                                            */
/************************************************************************/
vector<LPDIRECTDRAWSURFACE> backgroundImages;
vector<LPDIRECTDRAWSURFACE> unitImages;

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
		changeScreen(Screen::GAME);
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
			current.me.x -= 10;
			return 0;
		case VK_RIGHT:
			current.me.x += 10;
			return 0;
		case VK_UP:
			current.me.y -= 10;
			return 0;
		case VK_DOWN:
			current.me.y += 10;
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
void GameProc(HWND hWnd, LPDIRECTDRAWSURFACE BackScreen, LPDIRECTDRAWSURFACE RealScreen, int winWidth, int winHeight, 
	bool fullScreen)
{
	// Clear Back Ground
	RECT	BackRect = { 0, 0, winWidth, winHeight };
	BackScreen->BltFast(0, 0, BackImage, &BackRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);

	//////////////////////////////////////////////////////////////////////////

	// Draw me
	RECT meRect = { 0, 0, 16, 16 };
	BackScreen->BltFast(current.me.x, current.me.y, unitImages[3], &meRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);

	//////////////////////////////////////////////////////////////////////////

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

	loadImages();

	// [Game] ////////////////////////////////////////////////////////////////

	BackImage = backgroundImages[0];
	changeScreen(Screen::START);
	
	current.me.size = 1;

	//////////////////////////////////////////////////////////////////////////

	// Loop Start
	window.MainLoop();

	return TRUE;
}

/************************************************************************/
/* Implementation                                                       */
/************************************************************************/
void changeScreen(Screen state)
{
	if (current.screen == state) {
		// do nothing
	}
	else {
		current.screen = state;
		changeBackground(current.screen);
	}
}

void changeBackground(Screen state)
{
// 	BackImage = DDLoadBitmap(window.DirectOBJ, backgroundFromEnum(state), 0, 0);
// 	DDSetColorKey(BackImage, RGB(0, 0, 0));

	BackImage = backgroundImages[state];
}

char* backgroundFromEnum(Screen state)
{
	switch (state)
	{
	case MENU:
		return "menu.bmp";
		break;
	case GAME:
		return "game.bmp";
		break;
	default:
		return "back.bmp";
		break;
	}
}

void loadImages()
{
	LPDIRECTDRAWSURFACE temp;


	int background_number = 3;		// number of backgrounds
	char *background_files[] = {
		"images/background/back.bmp",
		"images/background/menu.bmp",
		"images/background/game.bmp"
	};
	for (int i = 0; i < background_number; i++)
	{
		temp = DDLoadBitmap(window.DirectOBJ, background_files[i], 0, 0);
		DDSetColorKey(temp, RGB(colorKey.r, colorKey.g, colorKey.b));
		backgroundImages.push_back(temp);
	}

	int unit_number = 1;
	int size_number = 4;
	char* unit_files[] = {
		"images/units/basic/"
	};
	char* size_files[] = {
		"s1.bmp",
		"s2.bmp",
		"s3.bmp",
		"s4.bmp"
	};
	for (int i = 0; i < unit_number; i++)
	{
		for (int j = 0; j < size_number; j++)
		{
			string str(unit_files[i]);
			str += size_files[j];

			temp = DDLoadBitmap(window.DirectOBJ, str.c_str(), 0, 0);
			DDSetColorKey(temp, RGB(colorKey.r, colorKey.g, colorKey.b));
			unitImages.push_back(temp);
		}
	}
}
