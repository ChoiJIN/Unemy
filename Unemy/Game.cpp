#include "Game.h"


Game::Game()
{
}


Game::~Game()
{
}

//////////////////////////////////////////////////////////////////////////
// GAME SETUP
void Game::game_init(CWindow* window)
{
	set_cwindow(window);

	current.me.size = 10;
	current.me.x = 200;
	current.me.y = 200;

}

void Game::load_images()
{
	LPDIRECTDRAWSURFACE temp;

	int background_number = 3;		// number of backgrounds
	char *background_files[] = {
		"images/background/back.bmp",
		"images/background/menu.bmp",
		"images/background/universe.bmp"
	};
	for (int i = 0; i < background_number; i++)
	{
		temp = DDLoadBitmap(window->DirectOBJ, background_files[i], 0, 0);
		DDSetColorKey(temp, RGB(color_key_r, color_key_g, color_key_b));
		background_images.push_back(temp);
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
			string str(unit_files[i]);
			str += size_files[j];

			temp = DDLoadBitmap(window->DirectOBJ, str.c_str(), 0, 0);
			DDSetColorKey(temp, RGB(color_key_r, color_key_g, color_key_b));
			unit_images.push_back(temp);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// GAME - SETUP
void Game::change_screen(Screen state)
{
	if (current.screen == state) {
		// do nothing
	}
	else {
		current.screen = state;
		change_background(current.screen);
	}
}

void Game::change_background(Screen state)
{
	BackImage = background_images[state];
}

void Game::me_calculation()
{
	core_calculation(current.me);
}

void Game::enemy_calculation()
{
	for each (Player p in current.players)
	{
		core_calculation(p);
	}
}

//////////////////////////////////////////////////////////////////////////
// GAME - ENEMY
bool Game::is_enemy_exist()
{
	return !current.players.empty();
}


void Game::make_enemy()
{

}

//////////////////////////////////////////////////////////////////////////
// GAME - hidden function

void Game::core_calculation(Player& p)
{
	// 마찰력 연산
	if (p.vx != 0) {
		if (abs(p.vx) < abs(GRAVITY_COEF*DRAG_COEF*(abs(p.vx) / p.vx) * p.size))
			p.vx = 0;
		else
			p.vx -= GRAVITY_COEF*DRAG_COEF*(abs(p.vx) / p.vx) * p.size;
	}

	if (p.vy != 0) {
		if (abs(p.vy) < abs(GRAVITY_COEF*DRAG_COEF*(abs(p.vy) / p.vy) * p.size))
			p.vy = 0;
		else
			p.vy -= GRAVITY_COEF*DRAG_COEF*(abs(p.vy) / p.vy) * p.size;
	}

	int maxVelocity = (int)((SIZE_NUMBER * 10 / p.size) * MAX_VELOCITY);
	if (abs(p.vx) > maxVelocity)
		p.vx = maxVelocity * (abs(p.vx) / p.vx);

	if (abs(p.vy) > maxVelocity)
		p.vy = maxVelocity * (abs(p.vy) / p.vy);

	// 속도로 위치 연산
	p.x += (int)(p.vx*DELTA_TIME);
	p.y += (int)(p.vy*DELTA_TIME);
}

int Game::new_id()
{
	return player_count++;
}

void Game::add_player()
{
	int id = new_id();

}


//////////////////////////////////////////////////////////////////////////
// GETTER
Current* Game::get_current()
{
	return &current;
}

LPDIRECTDRAWSURFACE Game::get_back_image()
{
	return BackImage;
}

LPDIRECTDRAWSURFACE Game::get_me_image()
{
	return unit_images[4];
}

//////////////////////////////////////////////////////////////////////////
// SETTER
void Game::set_cwindow(CWindow* window)
{
	this->window = window;
}

//////////////////////////////////////////////////////////////////////////
// INFO
RECT Game::get_me_rect()
{
	int left = current.me.x - current.me.size;
	int right = current.me.x + current.me.size;
	int top = current.me.y - current.me.size;
	int bottom = current.me.y + current.me.size;
	RECT rect = {
		left,
		top,
		right,
		bottom
	};
	return rect;
}

std::string Game::get_me_info()
{
	string str = "pos: (";
	str += to_string(current.me.x);
	str += ", ";
	str += to_string(current.me.y);
	str += ")";
	str += "\n";
	str += "velocity: (";
	str += to_string(current.me.vx);
	str += ", ";
	str += to_string(current.me.vy);
	str += ")";

	return str;
}

std::string Game::get_game_info()
{
	string str = "number: ";
	str += current.players.size();

	return str;
}

