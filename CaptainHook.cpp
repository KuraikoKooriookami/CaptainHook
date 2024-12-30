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
	bool ground = false;
};

struct velocity {
	double MOVEMENT = 10;
	double JUMP = 30;
	double GRAVITY = 10;
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
		ws.playerPosition.x -= v.MOVEMENT;
	}
	if (state[SDL_SCANCODE_RIGHT]) {
		ws.playerPosition.x += v.MOVEMENT;
	}
	if (state[SDL_SCANCODE_UP]) {
		ws.playerPosition.y -= v.JUMP;
	}
	if (ws.ground == false) {
		ws.playerPosition.y += v.GRAVITY;
	}
}

void renderGraphics(WorldState& ws) {
	SDL_SetRenderDrawColor(ws.renderer, 0, 0, 0, 255);
	SDL_RenderClear(ws.renderer);

	//SDL_RenderCopy(ws.renderer, ws.texture, NULL, NULL);	

	SDL_SetRenderDrawColor(ws.renderer, 0, 0, 255, 255);
	SDL_Rect playerRect = { ws.playerPosition.x, ws.playerPosition.y, 50, 50 };
	SDL_RenderFillRect(ws.renderer, &playerRect);
	SDL_RenderPresent(ws.renderer);
}

int main(int argc, char* args[])
{
	WorldState worldState;
	initWorldState(worldState);

	while (true) {
		readEvents(worldState);
		//TODO: mutateWorldState(worldState);
		renderGraphics(worldState);
	}

	SDL_DestroyTexture(worldState.texture);
	SDL_DestroyRenderer(worldState.renderer);
	IMG_Quit();
	SDL_Quit();
	return 0;
}