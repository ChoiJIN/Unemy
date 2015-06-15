#include "Controller.h"

Controller::Controller()
{
}


Controller::~Controller()
{
}

void Controller::set_current(Current *current)
{
	this->current = current;
}

double Controller::calc_accel()
{
	// a = F / M
	return PUSH_FORCE / (double)current->me.size;
}

void Controller::push(Direction direction, double push_ratio)
{
	// 가속도 계산
	if (push_ratio > 1)
		push_ratio = 1.0;
	double accelDelta = calc_accel()*push_ratio;
	switch (direction)
	{
	case Controller::UP:
		current->me.ay -= accelDelta;
		break;
	case Controller::DOWN:
		current->me.ay += accelDelta;
		break;
	case Controller::LEFT:
		current->me.ax -= accelDelta;
		break;
	case Controller::RIGHT:
		current->me.ax += accelDelta;
		break;
	default:
		break;
	}
	//current->me.ax = 10.0;
	// 가속도로 속도 계산
	current->me.vx += current->me.ax;
	current->me.vy += current->me.ay;

	current->me.ax = 0;
	current->me.ay = 0;
}
