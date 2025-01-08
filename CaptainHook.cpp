#include <SDL.h>
#include<vector> 
#include<SDL_image.h>
#include <iostream>

#define SCREEN_WIDTH 850
#define SCREEN_HEIGHT  600

using namespace std;
extern "C"

struct WorldState {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* image;
	SDL_Texture* texture;
	SDL_FRect player = { 100, 100, 50, 50 };
	SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	SDL_FPoint hookGoal = { 0, 0 };
	SDL_FPoint hookPosition = { 0, 0 };
	bool hookFlying = false;
	bool hookConnected = false;
	bool hookNotConnected = false;
	int amountOfHookTicks = 1;
	float MAXHOOKLENGTH = 1000;
	float HOOKFLYINGSPEED = 20;
	float currentHookLength = 0;
	double hookLength = 0;
	double speedX = 0;
	double speedY = 0;
	bool ground = false;
	int jumps = 0;
	double movement = 0;
};

struct vector2 {
	float x, y;
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

vector2 calculateDirection(const SDL_FRect& player, const SDL_FPoint& hookGoal) {
	float centerX = player.x + player.w / 2.0f;
	float centerY = player.y + player.h / 2.0f;
	float dx = hookGoal.x - centerX;
	float dy = hookGoal.y - centerY;
	float magnitude = sqrt(dx * dx + dy * dy);
	return { dx / magnitude, dy / magnitude };
}

void readEvents(WorldState& ws, const vector<SDL_FRect>& obstacles) {
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
		if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) 
		{
			ws.hookGoal.x = event.button.x;
			ws.hookGoal.y = event.button.y;
			ws.hookNotConnected = false;
			ws.hookFlying = true;
			ws.amountOfHookTicks = 1;
		}
		if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT) {
			ws.hookFlying = false;
			if (ws.hookConnected) {
				ws.hookNotConnected = true;
			}
			ws.hookConnected = false;
			ws.amountOfHookTicks = 1;
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

	if (ws.hookFlying && !ws.hookConnected && !ws.hookNotConnected) {
		vector2 direction = calculateDirection(ws.player, ws.hookGoal);

		ws.hookPosition.x = ws.player.x + ws.player.w / 2 + (direction.x * (ws.HOOKFLYINGSPEED * ws.amountOfHookTicks));
		ws.hookPosition.y = ws.player.y + ws.player.h / 2 + (direction.y * (ws.HOOKFLYINGSPEED * ws.amountOfHookTicks));
		ws.amountOfHookTicks++;
		SDL_FRect currentRect = { ws.hookPosition.x, ws.hookPosition.y, 1, 1 };
		for (const auto& obstacle : obstacles) {
			if (SDL_HasIntersectionF(&currentRect, &obstacle)) {
				ws.hookConnected = true;
				ws.hookFlying = false;
				ws.hookGoal = ws.hookPosition;
				ws.hookPosition = ws.hookPosition;
				break;
			}
		}
		if (!ws.hookConnected && abs(ws.hookPosition.x - ws.hookGoal.x) < 2 && abs(ws.hookPosition.y - ws.hookGoal.y) < 2) {
			ws.hookNotConnected = true;
			ws.hookGoal = ws.hookPosition;
			ws.hookFlying = false;
		}
	}
	else {
		if (ws.hookNotConnected) {
			vector2 direction = calculateDirection(ws.player, ws.hookGoal);
			ws.amountOfHookTicks--;
			ws.hookPosition.x = ws.player.x + ws.player.w/2 + (direction.x * (ws.HOOKFLYINGSPEED * ws.amountOfHookTicks));
			ws.hookPosition.y = ws.player.y + ws.player.h/2 + (direction.y * (ws.HOOKFLYINGSPEED * ws.amountOfHookTicks));
			if (!ws.hookConnected && abs(ws.hookPosition.x - (float(ws.player.x) + float(ws.player.w) / 2)) < 2 
				&& abs(ws.hookPosition.y - (float(ws.player.y) + float(ws.player.h) / 2)) < 2) {
				ws.hookNotConnected = false;
			}
		}
	}
	ws.ground = false;
	ws.player.y += ws.speedY;
	for (const auto& obstacle : obstacles) {
		if (SDL_HasIntersectionF(&ws.player, &obstacle)) {
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
		if (SDL_HasIntersectionF(&ws.player, &obstacle)) {
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
	if (ws.hookFlying || ws.hookConnected || ws.hookNotConnected) {
		SDL_SetRenderDrawColor(ws.renderer, 0, 255, 0, 255);
		SDL_RenderDrawLine(ws.renderer, ws.player.x + ws.player.w / 2, ws.player.y + ws.player.h / 2, ws.hookPosition.x, ws.hookPosition.y);
	}

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
		readEvents(worldState, obstacles);
		mutateWorldState(worldState, obstacles);
		//handle_camera(worldState);
		renderGraphics(worldState, obstacles);
	}

	SDL_DestroyTexture(worldState.texture);
	SDL_DestroyRenderer(worldState.renderer);
	IMG_Quit();
	SDL_Quit();
	return 0;
}