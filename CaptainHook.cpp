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

int main(int argc, char* args[])
{
	WorldState worldState;

	while (true) {
		initWorldState(worldState);
		SDL_PumpEvents();
		//TODO: ReadEvents();
		//TODO: mutateWorldState(worldState);
		//TODO: renderGraphics();

		SDL_RenderPresent(worldState.renderer);
	}

	SDL_DestroyTexture(worldState.texture);
	SDL_DestroyRenderer(worldState.renderer);
	IMG_Quit();
	SDL_Quit();
	return 0;
}