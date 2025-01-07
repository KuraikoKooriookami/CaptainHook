#include <SDL.h>
#include<vector> 
#include<SDL_image.h>

#define SCREEN_WIDTH 850
#define SCREEN_HEIGHT  600

using namespace std;
extern "C"

struct WorldState {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* image;
	SDL_Texture* texture;
	SDL_Rect player = { 100, 100, 50, 50 };
	SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	double speedX = 0;
	double speedY = 0;
	bool ground = false;
	int jumps = 0;
	double movement = 0;
};

struct velocity {
	double MAXSPEED = 1000;
	double MOVEMENT = 5;
	double AIR_FRICTION = 0.95;
	double GROUND_FRICTION = 0.5;
	double JUMP = 20;
	double GRAVITY = 1;
};

void initWorldState(WorldState& ws) {
	ws.window = SDL_CreateWindow("Captain Hook", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
	SDL_Event event;
	velocity v;
	if (state[SDL_SCANCODE_LEFT]) {
		ws.movement = v.MOVEMENT * -1;
	}
	else {
		if (state[SDL_SCANCODE_RIGHT]) {
			ws.movement = v.MOVEMENT;
		}
		else {
			ws.movement = 0;
		}
	}
	if (ws.speedX < ws.movement && ws.movement > 0) {
		ws.speedX = ws.movement;
	}
	if (ws.speedX > ws.movement && ws.movement < 0) {
		ws.speedX = ws.movement;
	}

	while (SDL_PollEvent(&event)){
		if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_UP && ws.jumps < 2) {
			ws.jumps++;
			ws.speedY = v.JUMP * -1;
			ws.ground = false;
		}
	}
	if (ws.ground == false) {
		ws.speedY += v.GRAVITY;
	}
	else {
		ws.speedY = 0;
		ws.jumps = 0;
		if (ws.speedX > v.GROUND_FRICTION) {
			ws.speedX += v.GROUND_FRICTION * -1;
		}
		else {
			if (ws.speedX < v.GROUND_FRICTION * -1) {
				ws.speedX += v.GROUND_FRICTION;
			}
			else {
				ws.speedX = 0;
			}
		}
	}
}
bool checkCollision(const WorldState& ws, const SDL_FRect& b) {
	SDL_Rect a = ws.player;
	return (a.x < b.x + b.w &&
		a.x + a.w > b.x &&
		a.y < b.y + b.h &&
		a.y + a.h > b.y);
}

void resolveCollision(WorldState& ws, const SDL_FRect& obstacle) {

	// Determine the overlap in each direction
	float overlapLeft = (ws.player.x + ws.player.w) - obstacle.x;
	float overlapRight = (obstacle.x + obstacle.w) - ws.player.x;
	float overlapTop = (ws.player.y + ws.player.h) - obstacle.y;
	float overlapBottom = (obstacle.y + obstacle.h) - ws.player.y;

	// Find the minimum overlap to resolve collision
	float minOverlap = std::min({ overlapLeft, overlapRight, overlapTop, overlapBottom });

	if (minOverlap == overlapLeft) {
		// Colliding from the left
		ws.player.x = obstacle.x - ws.player.w;
	}
	else if (minOverlap == overlapRight) {
		// Colliding from the right
		ws.player.x = obstacle.x + obstacle.w;
	}
	else if (minOverlap == overlapTop) {
		// Colliding from the top
		ws.player.y = obstacle.y - ws.player.h;
	}
	else if (minOverlap == overlapBottom) {
		// Colliding from the bottom
		ws.player.y = obstacle.y + obstacle.h;
	}
}

void handle_camera(WorldState& ws) {
	ws.camera.x = (ws.player.x + 50 / 2) - SCREEN_WIDTH  /  2;
	ws.camera.y = (ws.player.y + 50 / 2) - SCREEN_HEIGHT / 2;
	
	if (ws.camera.x < 0)
	{
		ws.camera.x = 0;
	}
	if (ws.camera.y < 0)
	{
		ws.camera.y = 0;
	}
}

void mutateWorldState(WorldState& ws, const vector<SDL_FRect>& obstacles) {

	ws.ground = false;
	ws.player.y += ws.speedY;
	for (const auto& obstacle : obstacles) {
		if (checkCollision(ws, obstacle)) {
			if (ws.speedY > 0) {
				ws.player.y = obstacle.y - ws.player.h; // Adjust player position to be on top of the obstacle
				ws.ground = true;
				ws.speedY = 0;
			}
			else {
				ws.player.y = obstacle.y + obstacle.h; // Adjust player position to be on top of the obstacle
				ws.ground = false;
				ws.speedY = 0;
			}
		}
	}
	ws.speedX += ws.movement;
	ws.player.x += ws.speedX;
	ws.speedX -= ws.movement;
	for (const auto& obstacle : obstacles) {
		if (checkCollision(ws, obstacle)) {
			if (ws.speedX > 0) {
				ws.player.x = obstacle.x - ws.player.w; // Adjust player position to be on top of the obstacle
				ws.speedX = 0;
			}
			else {
				ws.player.x = obstacle.x + obstacle.w; // Adjust player position to be on top of the obstacle
				ws.speedX = 0;
			}
		}
	}
}


void renderGraphics(WorldState& ws, const vector<SDL_FRect>& obstacles) {
	SDL_SetRenderDrawColor(ws.renderer, 0, 0, 0, 255);
	SDL_RenderClear(ws.renderer);

	SDL_SetRenderDrawColor(ws.renderer, 255, 0, 0, 255);
	for (const auto& obstacle : obstacles) {
		SDL_Rect obstacleRect = { static_cast<int>(obstacle.x - ws.camera.x), static_cast<int>(obstacle.y - ws.camera. y), static_cast<int>(obstacle.w), static_cast<int>(obstacle.h) };
		SDL_RenderFillRect(ws.renderer, &obstacleRect);
	}

	SDL_SetRenderDrawColor(ws.renderer, 0, 0, 255, 255);
	SDL_Rect playerRect = { ws.player.x - ws.camera.x, ws.player.y - ws.camera.y, 50, 50 };
	SDL_RenderFillRect(ws.renderer, &playerRect);
	SDL_RenderPresent(ws.renderer);
	
}

int main(int argc, char* args[])
{
	WorldState worldState;
	initWorldState(worldState);
	vector<SDL_FRect> obstacles = {
		{0, 550, 1050, 50}, // Example ground object
		{200, 400, 100, 50}, // Example obstacle
		{400, 300, 350, 50},  // Another example obstacle
		{850, 200, 50, 300}  // Wall example obstacle
	};

	while (true) {
		readEvents(worldState);
		mutateWorldState(worldState, obstacles);
		handle_camera(worldState);
		renderGraphics(worldState, obstacles);
	}

	SDL_DestroyTexture(worldState.texture);
	SDL_DestroyRenderer(worldState.renderer);
	IMG_Quit();
	SDL_Quit();
	return 0;
}