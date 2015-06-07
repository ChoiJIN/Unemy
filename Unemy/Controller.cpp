#include "Controller.h"

Controller::Controller()
{
}


Controller::~Controller()
{
}

void Controller::setCurrent(Current *current)
{
	this->current = current;
}

double Controller::calcAccel()
{
	// a = F / M
	return PUSH_FORCE / (double)current->me.size;
}

void Controller::push(Direction direction, double push_ratio)
{
	// ���ӵ� ���
	double accelDelta = calcAccel()*push_ratio;
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

	// ���ӵ��� �ӵ� ���
	current->me.vx += current->me.ax;
	current->me.vy += current->me.ay;

	current->me.ax = 0;
	current->me.ay = 0;
}
