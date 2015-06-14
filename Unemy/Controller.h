#pragma once
#include "GameState.h"

class Controller
{
private:
	Current *current;

public:
	enum Direction {
		UP, DOWN, LEFT, RIGHT
	};

	Controller();
	~Controller();

	void set_current(Current *current);
	double calc_accel();	// size에 따른 속도를 계산
	void push(Direction direction, double push_ratio);
};

