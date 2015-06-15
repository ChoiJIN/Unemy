//#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

#include "Game.h"
#include "CWindow.h"
#include "GameState.h"
#include "Controller.h"

using namespace std;

/************************************************************************/
/* Global Variable                                                      */
/************************************************************************/
CWindow window;

//Current current;
Controller controller;
Game game;

LPDIRECTDRAWSURFACE BackImage;
LPDIRECTDRAWSURFACE UnitImage;

/************************************************************************/
/* WndProc                                                              */
/************************************************************************/
// Toggle arrow key
bool tup, tdown, tleft, tright;
double lr_push, td_push;
double DELTA_PUSH = 0.02;

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
		game.elapse();

		if (tleft)
			controller.push(Controller::LEFT, lr_push += DELTA_PUSH);
		else if (tright)
			controller.push(Controller::RIGHT, lr_push += DELTA_PUSH);

		if (tup)
			controller.push(Controller::UP, td_push += DELTA_PUSH);
		else if (tdown)
			controller.push(Controller::DOWN, td_push += DELTA_PUSH);

		game.me_calculation();
		game.enemy_calculation();

		game.collision_detect();

		if (game.is_make_enemy_time())
		{
			game.make_enemy();
		}

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
			tleft = true;
			return 0;
		case VK_RIGHT:
			tright = true;
			return 0;
		case VK_UP:
			tup = true;
			return 0;
		case VK_DOWN:
			tdown = true;
			return 0;
		case VK_SPACE:
			break;
		case VK_CONTROL:
			break;
		}
		break;

	case WM_KEYUP:
		switch (wParam)
		{
		case VK_LEFT:
			tleft = false;
			lr_push = 0;
			return 0;
		case VK_RIGHT:
			tright = false;
			lr_push = 0;
			return 0;
		case VK_UP:
			tup = false;
			td_push = 0;
			return 0;
		case VK_DOWN:
			tdown = false;
			td_push = 0;
			return 0;
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
	BackImage = game.get_back_image();
	BackScreen->BltFast(0, 0, BackImage, &BackRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);

	//////////////////////////////////////////////////////////////////////////

	// Draw me
	RECT me_rect = game.get_me_rect();
	RECT im_rect = { 0, 0, 80, 80 };

	UnitImage = game.get_me_image();
	if (game.is_alive())
		BackScreen->Blt(&me_rect, UnitImage, &im_rect, DDBLT_WAIT | DDBLT_KEYSRC, NULL);

	// Draw Enemy
	if (game.is_enemy_exist())
	{
		int number = game.get_player_number();
		for (int i = 0; i < number; i++)
		{
			RECT enemy_rect = game.get_player_rect(i);
			BackScreen->Blt(&enemy_rect, UnitImage, &im_rect, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// TEXT
	HDC hdc;
	BackScreen->GetDC(&hdc);

	string str = game.get_me_info();
	TextOut(hdc, 50, 50, str.c_str(), str.size());
	
	str = game.get_game_info();
	TextOut(hdc, 50, 90, str.c_str(), str.size());

	BackScreen->ReleaseDC(hdc);
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

	game.game_init(&window);

	game.load_resources();

	game.play_bgm(0);

	controller.set_current(game.get_current());

	// [Game] ////////////////////////////////////////////////////////////////

	game.change_screen(Screen::GAME);

	game.make_enemy();

	//////////////////////////////////////////////////////////////////////////

	// Loop Start
	window.MainLoop();

	return TRUE;
}