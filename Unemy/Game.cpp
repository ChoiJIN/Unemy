#include "Game.h"


Game::Game()
{
}


Game::~Game()
{
}

//////////////////////////////////////////////////////////////////////////
// GAME - SETUP
void Game::game_init(CWindow* window)
{
	set_cwindow(window);

	current.me.state = PlayerState::ALIVE;
	current.me.size = 10;
	current.me.x = 200;
	current.me.y = 200;
}

void Game::load_resources()
{
	LPDIRECTDRAWSURFACE temp;

	int background_number = 3;		// number of backgrounds
	char *background_files[] = {
		"images/background/universe.bmp",
		"images/background/win.bmp",
		"images/background/dead.bmp"
	};
	for (int i = 0; i < background_number; i++)
	{
		temp = DDLoadBitmap(window->DirectOBJ, background_files[i], 0, 0);
		DDSetColorKey(temp, RGB(color_key_r, color_key_g, color_key_b));
		background_images.push_back(temp);
	}

	int unit_number = 1;
	int size_number = 2;
	char* unit_files[] = {
		"images/units/basic/"
	};
	char* size_files[] = {
		"me.bmp",
		"enemy.bmp"
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

	char* sounds_directory[] = {
		"sounds/bgm"
	};

	int sound_number = 1;
	char* sound_files[] = {
		"/Reverse-Rebirth.wav"
	};

	for (int i = 0; i<sound_number; i++)
	{
		string str(sounds_directory[0]);
		str += sound_files[i];
		Sound[i] = SndObjCreate(window->SoundOBJ, str.c_str(), 1);
	}
	
}

//////////////////////////////////////////////////////////////////////////
// GAME - PLAY
void Game::elapse()
{
	this->global_count++;

	if (!is_alive())
	{
		if (this->revival_count > 100) {
			revive_me();
			this->revival_count = 0;
		}
		this->revival_count++;
	}

	if (is_alive() && current.me.size == UNI_MAX_SIZE)
	{
		if (win_count > 1000) {
			game_over();
		}
		this->win_count++;
	}
}

void Game::change_screen(Screen::Enum state)
{
	if (current.screen == state) {
		// do nothing
	}
	else {
		current.screen = state;
		change_background(current.screen);
	}
}

void Game::change_background(Screen::Enum state)
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

double player_disance(Player p1, Player p2)
{
	return sqrt(pow(p1.x-p2.x,2.0) + pow(p1.y-p2.y,2.0));
}

void Game::collision_detect()
{
	int x_offset = 10;
	// between me and walls
	if (current.me.x - current.me.size < x_offset)
		current.me.x = x_offset + current.me.size;
	else if (current.me.x + current.me.size > window->width - x_offset)
		current.me.x = window->width - x_offset - current.me.size;

	if (current.me.y - current.me.size < 30)
		current.me.y = 30 + current.me.size;
	else if (current.me.y + current.me.size > window->height - x_offset)
		current.me.y = window->height - x_offset - current.me.size;

	// between me and plares
	//int number = get_player_number();
	for (Player p : current.players)
	{
		if (player_disance(current.me, p) < current.me.size + p.size)
		{
			consume(p);
			break;
		}
	}

	// between players

}

void Game::play_bgm(int number)
{
	SndObjPlay(Sound[number], DSBPLAY_LOOPING);
}

bool Game::is_alive()
{
	return current.me.state == PlayerState::ALIVE;
}

//////////////////////////////////////////////////////////////////////////
// GAME - ENEMY
bool Game::is_enemy_exist()
{
	return !current.players.empty();
}

bool Game::is_enemy_time()
{
	if (global_count % enemy_make_time == 0)
		return true;
	else
		return false;
}

void Game::make_enemy()
{
	srand((unsigned int)time((time_t)NULL));
	int size = 0;
	while (size == 0){
		size = rand() % 30 + 10;
	}
	int x = rand() % window->width;
	int y = rand() % window->height;
	Player new_player = { size, x, y };

	add_player(new_player);
}

//////////////////////////////////////////////////////////////////////////
// GAME - hidden function

void Game::core_calculation(Player& p)
{
	// ������ ����
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

	if (p.size == 0) return;

	double max_velocity = ((SIZE_NUMBER) * MAX_VELOCITY);
	//double max_velocity = 10*sqrt((SIZE_NUMBER * 10.0 - p.size + 10));
	//int max_velocity = MAX_VELOCITY - p.size;
	if (abs(p.vx) > max_velocity)
		p.vx = max_velocity * (abs(p.vx) / p.vx);

	if (abs(p.vy) > max_velocity)
		p.vy = max_velocity * (abs(p.vy) / p.vy);

	// �ӵ��� ��ġ ����
	p.x += (int)(p.vx*DELTA_TIME);
	p.y += (int)(p.vy*DELTA_TIME);
}

int Game::new_id()
{
	return player_count++;
}

void Game::add_player(Player p)
{
	current.players.push_back(p);
}

void Game::remove_player(Player p)
{
	vector<Player>& sugar = current.players;
	sugar.erase(remove(sugar.begin(), sugar.end(), p), sugar.end());
}

void Game::consume(Player p)
{
	remove_player(p);

	if (current.me.size + p.size >= UNI_MAX_SIZE)
	{
		if (current.me.size == UNI_MAX_SIZE)
			// BOMB!
			kill_me();
		current.me.size = UNI_MAX_SIZE;
	}
	else
	{
		current.me.size += p.size;
	}
}

void Game::kill_me()
{
	current.me.state = PlayerState::DEAD;
	game_over();
}

void Game::revive_me()
{
	int offset = 30;
	current.me.state = PlayerState::ALIVE;
	current.me.size = 10;
	current.me.x = rand() % (window->width - offset) + offset;
	current.me.y = rand() % (window->height - offset) + offset;

	win_count = 0;

	change_screen(Screen::GAME);
}

void Game::game_over()
{
	if (current.me.state == PlayerState::ALIVE && current.me.size == UNI_MAX_SIZE)
	{
		//std::this_thread::sleep_for(chrono::seconds(1));
		change_screen(Screen::WIN);
	}
	if (current.me.state == PlayerState::DEAD)
	{
		change_screen(Screen::DEAD);
	}
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

LPDIRECTDRAWSURFACE Game::get_enemy_image()
{
	return unit_images[1];
}

LPDIRECTDRAWSURFACE Game::get_me_image()
{
	return unit_images[0];
}

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

RECT Game::get_player_rect(int index)
{
	int left = current.players[index].x - current.players[index].size;
	int right = current.players[index].x + current.players[index].size;
	int top = current.players[index].y - current.players[index].size;
	int bottom = current.players[index].y + current.players[index].size;
	RECT rect = {
		left,
		top,
		right,
		bottom
	};
	return rect;
}

int Game::get_player_number()
{
	return current.players.size();
}

//////////////////////////////////////////////////////////////////////////
// SETTER
void Game::set_cwindow(CWindow* window)
{
	this->window = window;
}

//////////////////////////////////////////////////////////////////////////
// INFO
std::string Game::get_me_info()
{
	string str = "pos: (";
	str += to_string(current.me.x);
	str += ", ";
	str += to_string(current.me.y);
	str += ")";
	str += "  ";
	str += "velocity: (";
	str += to_string(current.me.vx*DELTA_TIME);
	str += ", ";
	str += to_string(current.me.vy*DELTA_TIME);
	str += ")";
	str += "  ";
	str += "size: (";
	str += to_string(current.me.size);
	str += ")";

	return str;
}

std::string Game::get_game_info()
{
	string str = "number: ";
	str += to_string(current.players.size());
	str += "  ";
	str += "count before win: ";
	str += to_string(win_count);
	str += " / 1000";

	return str;
}

