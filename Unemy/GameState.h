#pragma once
#include <vector>

enum Screen {
	START, MENU, GAME, NETWORK
};

enum PlayerState {
	ALIVE, DEAD
};

struct Player {
	int size;
	int x, y;
	double vx, vy;
	double ax, ay;
	PlayerState state;
};

struct Current {
	Screen screen;
	
	int number;			// ���� ������ �÷��̾� ��
	std::vector<Player> players;
	Player me;			// �÷��� ����
};

const double GRAVITY_COEF = 9.81;
const int PUSH_FORCE = 2500;
const double DRAG_COEF = 0.1;
const double MAX_VELOCITY = 50;
const double DELTA_TIME = 0.02;
const int SIZE_NUMBER = 8;
