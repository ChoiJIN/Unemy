#pragma once
#include <vector>

enum Screen {
	START, MENU, GAME, NETWORK
};

enum PlayerState {
	ALIVE, DEAD
};

struct Player {
	int id;
	int size;	// size is related movement
	int x, y;	// current position
	double vx, vy;	// current velocity
	double ax, ay;	// current acceleration
	PlayerState state;
};

struct Enemy {
	int id;
	int size;
	int x, y;
	PlayerState state;
};

struct Current {
	Screen screen;
	
	int number;					// 나를 제외한 플레이어 수
	std::vector<Enemy> players;	// 네트워크 플레이어
	Player me;					// 플레이 유저
};

const double GRAVITY_COEF = 9.81;
const int PUSH_FORCE = 2500;
const double DRAG_COEF = 0.1;
const double MAX_VELOCITY = 50;
const double DELTA_TIME = 0.02;
const int SIZE_NUMBER = 8;
