#pragma once
#include <windows.h>

#include <string>
#include <vector>

#include <ddraw.h>
#include <dsound.h>

#include "DDUTIL.H"
#include "DSUTIL.H"

#include "CWindow.h"

#include "GameState.h"

using namespace std;

class Game
{
private:
	LPDIRECTDRAWSURFACE BackImage;

	Current current;
	CWindow* window;

	int color_key_r = 0;
	int color_key_g = 255;
	int color_key_b = 0;

	int player_count = 0;

public:
	Game();
	~Game();

	//////////////////////////////////////////////////////////////////////////
	// GAME - SETUP
	void game_init(CWindow*);
	void load_images();
	// 
	void change_screen(Screen state);
	void change_background(Screen state);
	void me_calculation();
	void enemy_calculation();
	// GAME - ENEMY
	bool is_enemy_exist();
	void make_enemy();

private:
	//////////////////////////////////////////////////////////////////////////
	// GAME - hidden funtion
	void core_calculation(Player& p);
	int new_id();
	void add_player();
	//////////////////////////////////////////////////////////////////////////

public:
	//////////////////////////////////////////////////////////////////////////
	// GETTER
	Current* get_current();
	LPDIRECTDRAWSURFACE get_back_image();
	LPDIRECTDRAWSURFACE get_me_image();
	//////////////////////////////////////////////////////////////////////////
	// SETTER
	void set_cwindow(CWindow* window);
	//////////////////////////////////////////////////////////////////////////
	// INFO
	RECT get_me_rect();
	string get_me_info();
	string get_game_info();
	//////////////////////////////////////////////////////////////////////////

private:
	//////////////////////////////////////////////////////////////////////////
	// Resource                                                             */
	vector<LPDIRECTDRAWSURFACE> background_images;
	vector<LPDIRECTDRAWSURFACE> unit_images;
	//////////////////////////////////////////////////////////////////////////
};

