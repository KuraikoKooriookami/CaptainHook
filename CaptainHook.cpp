#include <SDL.h>
#include<vector> 
#include<SDL_image.h>
#include "CaptainHook.hpp"

using namespace std;
extern "C"

MyGame::MyGame()
	: Base("Example SDL Game", 3.0f, SDL_Point{ 640, 360 })
	//	: Base( "Example SDL Game", 1.0f, Point { 1280, 960 }, !doBenchmark )
{
	AddStates<
		PlayerState>(*this);

	// Set initial State
	PushState(MyGS::Player);

	/*IfNotFinal
		SetPerfDrawMode(PerformanceDrawMode::Title);*/
}
/*struct WorldState {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* image;
	SDL_Texture* texture;
	SDL_FPoint playerPosition = { 100, 100 };
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
			if (ws.speedY > 0) {
				ws.playerPosition.y = obstacle.y - 50; // Adjust player position to be on top of the obstacle
				ws.ground = true;
				ws.speedY = 0;
			}
			else {
				ws.playerPosition.y = obstacle.y + 50; // Adjust player position to be on top of the obstacle
				ws.ground = false;
				ws.speedY = 0;
			}
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
}*/
IGame::IGame(const char* windowTitle, float scalingFactor, const SDL_Point logicalSize, const bool vSync) noexcept
{
	if (SDL_Init(SDL_INIT_EVERYTHING))
	{
		exit(1);
	}
	// Recalculate scalingFactor dynamically
	/*if (scalingFactor == -1.0f)
	{
		SDL_DisplayMode dm;
		SDL_GetDesktopDisplayMode(0, &dm);
		int xFac = dm.w / logicalSize.x;
		int yFac = dm.h / logicalSize.y;
		scalingFactor = (float)std::max(1, std::min(xFac, yFac) - 1);
	} 

	Point requestedSize = toI(toF(logicalSize) * scalingFactor); */
	_window = SDL_CreateWindow(
		windowTitle,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		/*requestedSize.x,
		requestedSize.y,*/
		850,
		600,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if (_window == nullptr)
	{
		exit(6);
	}

	_renderer = SDL_CreateRenderer(
		_window,
		-1,
		SDL_RENDERER_ACCELERATED
		| (vSync
			? SDL_RENDERER_PRESENTVSYNC
			: 0));

	if (_renderer == nullptr)
	{
		exit(7);
	}

	/*if (scalingFactor != 1.0f)
	{
		// TODO: test if this works as intended
		const bool isIntegerScaling = (scalingFactor == nearbyintf(scalingFactor));
		if (isIntegerScaling)
		{
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
			SDL_RenderSetIntegerScale(_renderer, SDL_TRUE);
		}
		// HACK: Does not work because ImGUI would not work then, it in fact turns this here off :(
		//SDL_SetHint( SDL_HINT_MOUSE_RELATIVE_SCALING, "1" );
		//SDL_RenderSetLogicalSize( _renderer, logicalSize.x, logicalSize.y );
	}
	_scalingFactor = scalingFactor;*/
	//ImGuiOnly(CreateImGui(_renderer, _window); )
}

void IGame::ChangeState()
{
	if (_stateNextOp == NextStateOp::None)
	{
		//assert( false && "Change called with nextOp == None" );
		return;
	}

	if (_stateNextOp == NextStateOp::Pop
		|| _stateNextOp == NextStateOp::Replace)
	{
		currentState().Exit(_stateNextOp == NextStateOp::Pop);

		// When to call this?
		//currentState().Destroy();

		_stateStack.pop_back();
	}

	if (_stateNextOp == NextStateOp::Push
		|| _stateNextOp == NextStateOp::Replace)
	{
		assert(isStateIndexValid(_stateNextVal) && "Invalid State");

		_stateStack.push_back(_stateNextVal);

		currentState().Init();

		currentState().Enter(_stateNextOp == NextStateOp::Push);
	}

	_stateNextOp = NextStateOp::None;
	_stateNextVal = 0;
}

void IGame::Input()
{
	SDL_PumpEvents();

	if (currentState().Input())
		return;

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		// First try if BaseGame wants to handle the input (globals), then pass it to the GameState
		if (HandleEvent(event))
		{
			continue;
		}
		else
		{
			// Returns a bool to tell if the Event was expected and handled, if false, then this return could bubble up to a stacked GameState (future feature)
			currentState().HandleEvent(event);
		}
	}
}
void IGame::Update(const float deltaT)
{
	currentState().Update(_framesSinceStart, _msSinceStart, deltaT);
}

void IGame::Render(const float deltaTNeeded)
{
	const SDL_Color clear = currentState().clearColor();
	SDL_SetRenderDrawColor(_renderer, clear.r, clear.g, clear.b, clear.a);
	SDL_RenderClear(_renderer);

	/*// This is placed before the GameState::Render call, to also allow calls to ImGui inside Render (although most ImGui calls should be in RenderUI)
#if IMGUI
	//FPoint oldScale;
	//SDL_RenderGetScale(_renderer, &oldScale.x, &oldScale.y);
	const ImGuiIO& io = ImGui::GetIO();
	SDL_RenderSetScale(_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
	ImGui_ImplSDLRenderer2_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	//SDL_RenderSetScale( _renderer, oldScale.x, oldScale.y );
#endif*/

	currentState().Render(_framesSinceStart, _msSinceStart, deltaTNeeded);

	//ImGuiOnly(RenderUI(deltaTNeeded); )

	SDL_RenderPresent(_renderer);
}

int main(int argc, char* args[])
{
	MyGame game;
	/*WorldState worldState;
	initWorldState(worldState);
	vector<SDL_FRect> obstacles = {
		{0, 550, 850, 50}, // Example ground object
		{200, 400, 100, 50}, // Example obstacle
		{400, 300, 150, 50}  // Another example obstacle
	};
	*/
	game.ChangeState();
	while (true) {
		game.Input();
		game.Update(0);
		game.Render(0);
	}

	//SDL_DestroyTexture(worldState.texture);
	//SDL_DestroyRenderer(worldState.renderer);
	IMG_Quit();
	SDL_Quit();
	return 0;
}