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
void loadImages();						// 모든 이미지를 로딩 함.
void changeScreen(Screen state);		// Screen 전환 함수
void changeBackground(Screen state);	// Background 전환 함수
char* backgroundFromEnum(Screen state);	// Enum으로부터 Background의 파일 경로를 받아옴.

// Physics
void calculation();
void collisionDetection();

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
Color colorKey = { 0, 0, 0 };	// 현재 컬러키가 먹히지 않음. 무조건 검은색으로..

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

chat_client* c;
const size_t DATA_LENGTH = 10;

void send_position(int x, int y)
{
	using namespace std; // For strlen and memcpy.
	chat_message msg;
	sprintf(msg.body(), "%d %d", x, y);
	msg.body_length(strlen(msg.body()));

	//memcpy(msg.body(), line, msg.body_length());
	msg.encode_header();
	c->write(msg);
}

void receive_position()
{
	//chat_message msg = c->get_message();


}

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
		collisionDetection();

		send_position(current.me.x, current.me.y);

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
	RECT meRect = { 0, 0, 16, 16 };
	BackScreen->BltFast(current.me.x, current.me.y, unitImages[3], &meRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);

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
		const char ip[] = "127.0.0.1";
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

		loadImages();

		controller.setCurrent(&current);

		//////////////////////////////////////////////////////////////////////////
		// Game SETTING
		//////////////////////////////////////////////////////////////////////////
		BackImage = backgroundImages[0];
		changeScreen(Screen::MENU);

		current.me.size = 10;
		current.me.x = 100;
		current.me.y = 100;


		//////////////////////////////////////////////////////////////////////////
		// Loop Start
		//////////////////////////////////////////////////////////////////////////
		window.MainLoop();

// 		char line[chat_message::max_body_length + 1];
// 		while (std::cin.getline(line, chat_message::max_body_length + 1))
// 		{
// 			using namespace std; // For strlen and memcpy.
// 			chat_message msg;
// 			msg.body_length(strlen(line));
// 			memcpy(msg.body(), line, msg.body_length());
// 			msg.encode_header();
// 			c.write(msg);
// 		}

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
void collisionDetection()
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
