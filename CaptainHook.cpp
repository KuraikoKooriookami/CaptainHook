#include <SDL.h>
#include<vector> 
#include<SDL_image.h>

using namespace std;
extern "C"

struct WorldState {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* image;
	SDL_Texture* texture;
	SDL_FPoint playerPosition = { 100, 100 };
	double speedX = 0;
	double speedY = 0;
	bool ground = false;
	double movement = 0;
};

struct velocity {
	double MAXSPEED = 1000;
	double MOVEMENT = 10;
	double JUMP = 10;
	double GRAVITY = 1;
};

void initWorldState(WorldState& ws) {
	ws.window = SDL_CreateWindow("Captain Hook", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 850, 600, SDL_WINDOW_SHOWN);
	ws.renderer = SDL_CreateRenderer(ws.window, -1, SDL_RENDERER_PRESENTVSYNC);
	SDL_Init(SDL_INIT_EVERYTHING);

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		exit(0);
	}

	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
		SDL_Quit();
		exit(0);
	}
}

void readEvents(WorldState& ws) {
	SDL_PumpEvents();
	const Uint8* state = SDL_GetKeyboardState(NULL);
	velocity v;
	if (state[SDL_SCANCODE_LEFT]) {
		ws.movement = v.MOVEMENT * -1;
	}
	else {
		if (state[SDL_SCANCODE_RIGHT]) {
			ws.movement = v.MOVEMENT;
		}
		else {
			if (ws.movement > 0.1) {
				ws.movement -= 0.1;
			}
			else {
				if (ws.movement < -0.1) {
					ws.movement += 0.1;
				}
				else {
					ws.movement = 0;
				}
			}
		}
	}
	if (state[SDL_SCANCODE_UP]) {
		ws.speedY = v.JUMP * -1;
		ws.ground = false;
	}
	if (ws.ground == false) {
		ws.speedY += v.GRAVITY;
	}
}
bool checkCollision(const WorldState& ws, const SDL_FRect& b) {
	SDL_Rect a = { ws.playerPosition.x, ws.playerPosition.y, 50, 50 };
	return (a.x < b.x + b.w &&
		a.x + a.w > b.x &&
		a.y < b.y + b.h &&
		a.y + a.h > b.y);
}

void mutateWorldState(WorldState& ws, const vector<SDL_FRect>& obstacles) {
	ws.speedX += ws.movement;
	ws.playerPosition.x += ws.speedX;
	ws.speedX -= ws.movement;
	ws.playerPosition.y += ws.speedY;

	ws.ground = false;
	for (const auto& obstacle : obstacles) {
		if (checkCollision(ws, obstacle)) {
			ws.ground = true;
			ws.speedY = 0;
			ws.playerPosition.y = obstacle.y - 50; // Adjust player position to be on top of the obstacle
			break;
		}
	}
}


void renderGraphics(WorldState& ws, const vector<SDL_FRect>& obstacles) {
	SDL_SetRenderDrawColor(ws.renderer, 0, 0, 0, 255);
	SDL_RenderClear(ws.renderer);

	SDL_SetRenderDrawColor(ws.renderer, 255, 0, 0, 255);
	for (const auto& obstacle : obstacles) {
		SDL_Rect obstacleRect = { static_cast<int>(obstacle.x), static_cast<int>(obstacle.y), static_cast<int>(obstacle.w), static_cast<int>(obstacle.h) };
		SDL_RenderFillRect(ws.renderer, &obstacleRect);
	}

	SDL_SetRenderDrawColor(ws.renderer, 0, 0, 255, 255);
	SDL_Rect playerRect = { ws.playerPosition.x, ws.playerPosition.y, 50, 50 };
	SDL_RenderFillRect(ws.renderer, &playerRect);
	SDL_RenderPresent(ws.renderer);
}

int main(int argc, char* args[])
{
	WorldState worldState;
	initWorldState(worldState);
	vector<SDL_FRect> obstacles = {
		{0, 550, 850, 50}, // Example ground object
		{200, 400, 100, 50}, // Example obstacle
		{400, 300, 150, 50}  // Another example obstacle
	};

	while (true) {
		readEvents(worldState);
		mutateWorldState(worldState, obstacles);
		renderGraphics(worldState, obstacles);
	}

	SDL_DestroyTexture(worldState.texture);
	SDL_DestroyRenderer(worldState.renderer);
	IMG_Quit();
	SDL_Quit();
	return 0;
}