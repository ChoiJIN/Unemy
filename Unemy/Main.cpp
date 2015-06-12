// BOOST
#include <boost/thread/thread.hpp>

// Network
#include "chat_client.hpp"
#include "chat_message.hpp"

// Standard
#include <windows.h>
#include <vector>
#include <string>

#include "CWindow.h"

#include "GameState.h"
#include "Controller.h"

using namespace std;

/************************************************************************/
/* Declaration                                                          */
/************************************************************************/
void load_images();						// 모든 이미지를 로딩 함.
void change_screen(Screen state);		// Screen 전환 함수
void change_background(Screen state);	// Background 전환 함수
char* background_from_enum(Screen state);	// Enum으로부터 Background의 파일 경로를 받아옴.

// Physics
void calculation();
void collision_detection();

// Network
void send_me(int size, int x, int y, double vx, double vy);
void receive_from_server();

/************************************************************************/
/* Global Variable                                                      */
/************************************************************************/
double my_time = 0;

CWindow window;
Current current;
Controller controller;

LPDIRECTDRAWSURFACE BackImage;
struct Color {
	int r, g, b;
};
Color colorKey = { 0, 255, 0 };	// 현재 컬러키가 먹히지 않음. 무조건 검은색으로..

chat_client* c;

/************************************************************************/
/* Resources                                                            */
/************************************************************************/
vector<LPDIRECTDRAWSURFACE> backgroundImages;
vector<LPDIRECTDRAWSURFACE> unitImages;

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
		change_screen(Screen::GAME);
		break;
	case WM_DESTROY:
		window.ReleaseAll();
		PostQuitMessage(0);
		break;

	case WM_TIMER:
		if (tleft)
			controller.push(Controller::LEFT, lr_push += DELTA_PUSH);
		else if (tright)
			controller.push(Controller::RIGHT, lr_push += DELTA_PUSH);

		if (tup)
			controller.push(Controller::UP, td_push += DELTA_PUSH);
		else if (tdown)
			controller.push(Controller::DOWN, td_push += DELTA_PUSH);

		// Physics
		calculation();
		collision_detection();

		send_me(current.me.size, current.me.x, current.me.y, current.me.vx, current.me.vy);

		receive_from_server();

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
	BackScreen->BltFast(0, 0, BackImage, &BackRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
	//////////////////////////////////////////////////////////////////////////

	// Draw me
	RECT me_rect;
	me_rect.left = current.me.x - current.me.size;
	me_rect.top = current.me.y - current.me.size;
	me_rect.right = current.me.x + current.me.size;
	me_rect.bottom = current.me.y + current.me.size;
	RECT im_rect = { 0, 0, 80, 80 };

	BackScreen->Blt(&me_rect, unitImages[4], &im_rect, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	//BackScreen->BltFast(100, 100, testi, &im_rect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);

	for (int i = 0; i < current.players.size(); i++)
	{
		Enemy e = current.players[i];
		RECT enemy_rect = { e.x - e.size, e.y - e.size, e.x + e.size, e.y + e.size };
		BackScreen->Blt(&enemy_rect, unitImages[4], &im_rect, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	}

	//////////////////////////////////////////////////////////////////////////
	// TEXT
	HDC hdc;
	BackScreen->GetDC(&hdc);
	string str = "pos: (";
	str += to_string(current.me.x);
	str += ", ";
	str += to_string(current.me.y);
	str += ")";
	TextOut(hdc, 50, 50, str.c_str(), str.size());
	str = "velocity: (";
	str += to_string(current.me.vx);
	str += ", ";
	str += to_string(current.me.vy);
	str += ")";
	TextOut(hdc, 50, 70, str.c_str(), str.size());
	str = "Number: ";
	str += to_string(current.players.size());
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
	try
	{
		//////////////////////////////////////////////////////////////////////////
		// Network SETTING
		//////////////////////////////////////////////////////////////////////////
		const char ip[] = "222.112.27.129";
		const char port[] = "5166";

		boost::asio::io_service io_service;

		tcp::resolver resolver(io_service);
		tcp::resolver::query query(ip, port);
		tcp::resolver::iterator iterator = resolver.resolve(query);

		c = new chat_client(io_service, iterator);

		boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));


		//////////////////////////////////////////////////////////////////////////
		// DirectDraw SETTING
		//////////////////////////////////////////////////////////////////////////
		// Window Initialization
		window.SetGameProc(GameProc);
		window.SetWndProc(WndProc);
		window.Init(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

		load_images();

		controller.setCurrent(&current);

		//////////////////////////////////////////////////////////////////////////
		// Game SETTING
		//////////////////////////////////////////////////////////////////////////
		//BackImage = backgroundImages[2];
		change_screen(Screen::MENU);

		current.me.size = 10;
		current.me.x = 300;
		current.me.y = 300;


		//////////////////////////////////////////////////////////////////////////
		// Loop Start
		//////////////////////////////////////////////////////////////////////////
		window.MainLoop();

		c->close();
		t.join();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return TRUE;
}

/************************************************************************/
/* Implementation                                                       */
/************************************************************************/
void change_screen(Screen state)
{
	if (current.screen == state) {
		// do nothing
	}
	else {
		current.screen = state;
		change_background(current.screen);
	}
}

void change_background(Screen state)
{
	BackImage = backgroundImages[state];
}

void change_my_size(int size)
{

}

char* background_from_enum(Screen state)
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

void load_images()
{


	int background_number = 3;		// number of backgrounds
	char *background_files[] = {
		"images/background/back.bmp",
		"images/background/menu.bmp",
		"images/Background/universe.bmp"
	};
	for (int i = 0; i < background_number; i++)
	{
		LPDIRECTDRAWSURFACE temp;
		temp = DDLoadBitmap(window.DirectOBJ, background_files[i], 0, 0);
		DDSetColorKey(temp, RGB(colorKey.r, colorKey.g, colorKey.b));
		backgroundImages.push_back(temp);
	}

	int unit_number = 1;
	int size_number = 5;
	char* unit_files[] = {
		"images/units/basic/"
	};
	char* size_files[] = {
		"s1.bmp",
		"s2.bmp",
		"s3.bmp",
		"s4.bmp",
		"test.bmp"
	};
	for (int i = 0; i < unit_number; i++)
	{
		for (int j = 0; j < size_number; j++)
		{
			LPDIRECTDRAWSURFACE temp;

			string str(unit_files[i]);
			str += size_files[j];

			temp = DDLoadBitmap(window.DirectOBJ, str.c_str(), 0, 0);
			DDSetColorKey(temp, RGB(colorKey.r, colorKey.g, colorKey.b));
			unitImages.push_back(temp);
		}
	}
}

void calculation()
{
	my_time += DELTA_TIME;

	// 마찰력 연산
	if (current.me.vx != 0) {
		if (abs(current.me.vx) < abs(GRAVITY_COEF*DRAG_COEF*(abs(current.me.vx) / current.me.vx) * current.me.size))
			current.me.vx = 0;
		else
			current.me.vx -= GRAVITY_COEF*DRAG_COEF*(abs(current.me.vx) / current.me.vx) * current.me.size;
	}

	if (current.me.vy != 0) {
		if (abs(current.me.vy) < abs(GRAVITY_COEF*DRAG_COEF*(abs(current.me.vy) / current.me.vy) * current.me.size))
			current.me.vy = 0;
		else
			current.me.vy -= GRAVITY_COEF*DRAG_COEF*(abs(current.me.vy) / current.me.vy) * current.me.size;
	}

	int maxVelocity = (SIZE_NUMBER * 10 / current.me.size) * MAX_VELOCITY;
	if (abs(current.me.vx) > maxVelocity)
		current.me.vx = maxVelocity * (abs(current.me.vx) / current.me.vx);

	if (abs(current.me.vy) > maxVelocity)
		current.me.vy = maxVelocity * (abs(current.me.vy) / current.me.vy);

	// 속도로 위치 연산
	current.me.x += (int)(current.me.vx*DELTA_TIME);
	current.me.y += (int)(current.me.vy*DELTA_TIME);

}

int win_offset = 30;
void collision_detection()
{
	int offset = current.me.size + win_offset;

	if (current.me.x > window.WIN_WIDTH - offset) {
		current.me.x = window.WIN_WIDTH - offset;
		current.me.vx = 0;
	}
	else if (current.me.x < offset) {
		current.me.x = offset;
		current.me.vx = 0;
	}

	if (current.me.y > window.WIN_HEIGHT - offset) {
		current.me.y = window.WIN_HEIGHT - offset;
		current.me.vy = 0;
	}
	else if (current.me.y < offset) {
		current.me.y = offset;
		current.me.vy = 0;
	}
}


void send_me(int size, int x, int y, double vx, double vy)
{
	using namespace std; // For strlen and memcpy.
	chat_message msg;

	int vx1 = (int)vx;
	int vx2 = (int)(vx * 100) % 100;

	int vy1 = (int)vy;
	int vy2 = (int)(vy * 100) % 100;

	sprintf(msg.body(), "%04d%04d%04d%02d%02d%02d%02d", size, x, y, vx1, vx2, vy1, vy2);
	msg.body_length(strlen(msg.body()));

	//memcpy(msg.body(), line, msg.body_length());
	msg.encode_header();
	c->write(msg);
}

void receive_from_server()
{
	current.players = c->get_enemies();
}