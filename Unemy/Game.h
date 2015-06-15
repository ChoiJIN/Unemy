#pragma once

#include <windows.h>

#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <thread>
#include <chrono>

#include <mmsystem.h>
#include <ddraw.h>
#include <dsound.h>

#include "CWindow.h"

#include "GameState.h"

using namespace std;

class Game
{
private:
	Current current;
	CWindow* window;

	const int color_key_r = 0;
	const int color_key_g = 255;
	const int color_key_b = 0;

	int player_count = 0;
	int global_count = 0;
	int revival_count = 0;
	int win_count = 0;

	const int UNI_MAX_SIZE = 80;


public:
	Game();
	~Game();

	//////////////////////////////////////////////////////////////////////////
	// GAME - SETUP
	void game_init(CWindow*);
	void load_resources();
	//////////////////////////////////////////////////////////////////////////
	// GAME - PLAY
	void elapse();
	void change_screen(Screen state);
	void change_background(Screen state);
	void me_calculation();
	void enemy_calculation();
	void collision_detect();
	void play_bgm(int number);
	bool is_alive();
	// GAME - ENEMY
	bool is_enemy_exist();
	bool is_make_enemy_time();
	void make_enemy();

private:
	//////////////////////////////////////////////////////////////////////////
	// GAME - hidden funtion
	void core_calculation(Player& p);
	int new_id();
	void add_player(Player p);
	void remove_player(Player p);
	// Consumes p
	void consume(Player p);
	void kill_me();
	void revive_me();
	//
	void game_over();
	//
	//////////////////////////////////////////////////////////////////////////

public:
	//////////////////////////////////////////////////////////////////////////
	// GETTER
	Current* get_current();
	LPDIRECTDRAWSURFACE get_back_image();
	LPDIRECTDRAWSURFACE get_me_image();

	RECT get_me_rect();
	RECT get_player_rect(int);
	int get_player_number();
	//////////////////////////////////////////////////////////////////////////
	// SETTER
	void set_cwindow(CWindow* window);
	//////////////////////////////////////////////////////////////////////////
	// INFO
	string get_me_info();
	string get_game_info();
	//////////////////////////////////////////////////////////////////////////

private:
	//////////////////////////////////////////////////////////////////////////
	LPDIRECTDRAWSURFACE BackImage;

	// Images
	vector<LPDIRECTDRAWSURFACE> background_images;
	vector<LPDIRECTDRAWSURFACE> unit_images;

	// Sounds
	HSNDOBJ Sound[10];

	//////////////////////////////////////////////////////////////////////////
};

