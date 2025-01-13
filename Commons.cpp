#include <SDL.h>
#include<vector> 
#define TILESIZE 32

struct vector2 {
	float x, y;

	vector2& operator+=(const vector2& other) {
		this->x += other.x;
		this->y += other.y;
		return *this;
	}
};

struct velocity {
	double MAXSPEED = 10;
	double MOVEMENT = 7.5;
	double AIR_FRICTION = 0.95;
	double GROUND_FRICTION = 0.5;
	double JUMP = 15;
	double GRAVITY = 0.5;
};

struct SDL_FRect_P {
	SDL_FRect rect;
	int obstacleValue;
};