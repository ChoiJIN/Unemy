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

	void setCurrent(Current *current);
	double calcAccel();	// size�� ���� �ӵ��� ���
	void push(Direction direction, double push_ratio);
};

