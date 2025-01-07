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
	SDL_FPoint hookGoal = { 0, 0 };
	bool hook = false;
	bool hookConnected = false;
	bool noConnection = false;
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


vector2 calculateDirection(const SDL_Rect& player, const SDL_FPoint& hookGoal) {
	float centerX = player.x + player.w / 2.0f;
	float centerY = player.y + player.h / 2.0f;
	float dx = hookGoal.x - centerX;
	float dy = hookGoal.y - centerY;
	float magnitude = sqrt(dx * dx + dy * dy);
	return { dx / magnitude, dy / magnitude };
}

SDL_Point checkHookInterception(const SDL_Rect player, const SDL_MouseButtonEvent& button, const vector<SDL_FRect>& obstacles) {
	SDL_Point hookGoal = { button.x, button.y };

	// Berechne den Mittelpunkt des Spielers
	float playerCenterX = player.x + player.w / 2.0f;
	float playerCenterY = player.y + player.h / 2.0f;

	for (const auto& obstacle : obstacles) {
		// Berechne die Grenzen des Hindernisses
		float left = obstacle.x;
		float right = obstacle.x + obstacle.w;
		float top = obstacle.y;
		float bottom = obstacle.y + obstacle.h;

		// Berechne die Richtung des Hakens
		float dx = hookGoal.x - playerCenterX;
		float dy = hookGoal.y - playerCenterY;

		// Überprüfe, ob der Haken das Hindernis schneidet
		if ((playerCenterX < right && hookGoal.x > left) || (playerCenterX > left && hookGoal.x < right)) {
			if ((playerCenterY < bottom && hookGoal.y > top) || (playerCenterY > top && hookGoal.y < bottom)) {
				// Berechne den Schnittpunkt
				if (dx != 0) {
					float slope = dy / dx;
					float intercept = playerCenterY - slope * playerCenterX;

					if (dx > 0) {
						hookGoal.x = left;
					}
					else {
						hookGoal.x = right;
					}
					hookGoal.y = slope * hookGoal.x + intercept;
				}
				else {
					hookGoal.y = (dy > 0) ? top : bottom;
				}
				return hookGoal;
			}
		}
	}

	return hookGoal;
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
			//ws.hookGoal = checkHookInterception(ws.player,event.button, obstacles);
			ws.hookGoal.x = event.button.x;
			ws.hookGoal.y = event.button.y;
			ws.noConnection = false;
			ws.hook = true;
		}
		if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT) {
			ws.hook = false;
			ws.hookConnected = false;
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

	if (ws.hook && !ws.hookConnected && !ws.noConnection) {
		vector2 direction = calculateDirection(ws.player, ws.hookGoal);
		SDL_FPoint currentPosition = { ws.player.x + ws.player.w / 2, ws.player.y + ws.player.h / 2 };
		while (!ws.hookConnected) {
			currentPosition.x += direction.x;
			currentPosition.y += direction.y;
			SDL_FRect currentRect = { currentPosition.x, currentPosition.y, 1, 1 };
			for (const auto& obstacle : obstacles) {
				if (SDL_HasIntersectionF(&currentRect, &obstacle)) {
					ws.hookConnected = true;
					ws.hookGoal = currentPosition;
					break;
				}
			}
			if (!ws.hookConnected && abs(currentPosition.x - ws.hookGoal.x) < 2 && abs(currentPosition.y - ws.hookGoal.y) < 2) {
				ws.noConnection = true;
				break;
			}
		}
		ws.hookGoal = currentPosition;
	}
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
	if (ws.hook && ws.hookConnected) {
		SDL_SetRenderDrawColor(ws.renderer, 0, 255, 0, 255);
		SDL_RenderDrawLine(ws.renderer, ws.player.x + ws.player.w / 2, ws.player.y + ws.player.h / 2, ws.hookGoal.x, ws.hookGoal.y);
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
		handle_camera(worldState);
		renderGraphics(worldState, obstacles);
	}

	SDL_DestroyTexture(worldState.texture);
	SDL_DestroyRenderer(worldState.renderer);
	IMG_Quit();
	SDL_Quit();
	return 0;
}