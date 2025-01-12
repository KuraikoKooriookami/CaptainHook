#include <SDL.h>
#include<vector> 
#include<SDL_image.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#define SCREEN_WIDTH 850 *1.5
#define SCREEN_HEIGHT  600 *1.5
#define TILESIZE 32

using namespace std;
extern "C"

struct vector2 {
	float x, y;    
	
	vector2& operator+=(const vector2& other) {
		this->x += other.x;
		this->y += other.y;
		return *this;
	}
};

struct WorldState {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* image;
	SDL_Texture* texture;
	SDL_FRect player = { 2*TILESIZE, 2*TILESIZE, TILESIZE, TILESIZE };
	SDL_FPoint hookGoal = { 0, 0 };
	SDL_FPoint hookPosition = { 0, 0 };
	SDL_Rect spriteSheet[16][12] = { 0, 0, TILESIZE , TILESIZE };
	vector<vector<int>>  map;
	vector2 playerVelocity = { 0, 0 };
	vector2 appliedForce = { 0, 0 };
	bool hookFlying = false;
	bool hookConnected = false;
	bool hookNoObstacleFound = false;
	int amountOfHookTicks = 1;
	float MAXHOOKLENGTH = 1000;
	float HOOKFLYINGSPEED = 25;
	float HOOKSTRENGHT = 5;
	float currentHookLength = 0;
	double hookLength = 0;
	double speedX = 0;
	double speedY = 0;
	bool ground = false;
	int jumps = 0;
	double movement = 0;

};

struct {
	float x;
	float y;
} camera = { 0, 0, };

struct velocity {
	double MAXSPEED = 10;
	double MOVEMENT = 7.5;
	double AIR_FRICTION = 0.95;
	double GROUND_FRICTION = 0.5;
	double JUMP = 15;
	double GRAVITY = 0.5;
};

enum obstacleID {
	EMPTY,	//1
	EARTH,		//2
	UGRASS,	//3
	RGRASS,	//4
	DGRASS,	//5
	LGRASS,	//6
	KIRBY1,	
	KIRBY2,
	DUDE1,
	DUDE2,

};

vector<SDL_FRect> obstacles;

void drawObstacle(const WorldState& ws, SDL_FRect destRect,int obstacleValue)
{
	obstacleID obstacle = static_cast<obstacleID>(obstacleValue);

	switch (obstacle) {
	case (EMPTY):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[6][0], &destRect);    break;
		break;
	case (EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[1][0], &destRect);    break;
		break;
	case (UGRASS):
		printf("Object: UGRASS\n");
		break;
	case (RGRASS):
		printf("Object: RGRASS\n");
		break;
	case (DGRASS):
		printf("Object: DGRASS\n");
		break;
	case (LGRASS):
		printf("Object: LGRASS\n");
		break;
	case (KIRBY1):
		printf("Object: KIRBY1\n");
		break;
	case (KIRBY2):
		printf("Object: KIRBY2\n");
		break;
	case (DUDE1):
		printf("Object: DUDE1\n");
		break;
	case (DUDE2):
		printf("Object: DUDE2\n");
		break;
	default:
		printf("Unknown object\n");
		break;
	}

}

void getMap(WorldState& ws, vector<SDL_FRect>& obstacles, const string& level)
{
	cout << "Loading level file: " << level << endl;
	ifstream file(level);
	string line;

	if (file.is_open()) {
		int row = 0;
		while (getline(file, line)) {
			stringstream ss(line);
			string number;
			vector<int> rowData;

			// Strip '{' and '}' and other non-digit characters
			line.erase(remove_if(line.begin(), line.end(), [](unsigned char c) {
				return c == '{' || c == '}' || c == ' ';
				}), line.end());

			// Now split the line by commas
			stringstream rowStream(line);
		
			while (getline(rowStream, number, ',')) {
					// Remove non-digit characters (if any left after erase)
					number.erase(remove_if(number.begin(), number.end(),
					[](unsigned char c) { return !isdigit(c); }),
					number.end());

					if (!number.empty()) {
					{
						int value = stoi(number);
						rowData.push_back(value);

						//if (value > 0) {
						//	SDL_FRect rect = {
						//		static_cast<float>(rowData.size()) * TILESIZE,
						//		static_cast<float>(row ) * TILESIZE,
						//		TILESIZE,
						//		TILESIZE
						//	};
						//	obstacles.push_back(rect);
						//}
					}
				}
			}
			ws.map.push_back(rowData);
			row++;
		}
		file.close();
	}
	else {
		cerr << "No file: " << level << " found" << endl;
	}
	/* Optionally print the levelData for verification
	std::cout << "Level data loaded:" << std::endl;
	for (const auto& row : ws.map) {
		for (const auto& value : row) {
			std::cout << value << " ";
		}
		std::cout << std::endl;
	}*/
}

void loadLevelAndDraw(WorldState& ws, vector<SDL_FRect>& obstacles)
{
	obstacles.clear();
	for (int row = 0; row < ws.map.size(); row++){
		for (int col = 0; col < ws.map[row].size(); col++){
			int value = ws.map[row][col];

			if (value > 0) {
				SDL_FRect rect = {
					col * TILESIZE,
					row * TILESIZE,
					TILESIZE,
					TILESIZE
				};
				if (obstacles.empty() || obstacles.back().x != rect.x || obstacles.back().y != rect.y) {
					obstacles.push_back(rect);
				}
				drawObstacle(ws, rect, value);
			}
		}
	}
}


void initWorldState(WorldState& ws) {
	ws.window = SDL_CreateWindow("Captain Hook", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	ws.renderer = SDL_CreateRenderer(ws.window, -1, SDL_RENDERER_PRESENTVSYNC);

	char* basePath = SDL_GetBasePath();
	string texturePath = string(basePath) + "../../Images/grass_main.png";
	ws.texture = IMG_LoadTexture(ws.renderer, texturePath.c_str());
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Init(IMG_INIT_PNG);

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("Ey ne Fehlermeldung w�re ganz nett");
		exit(0);
	}

	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
		printf("Ey ne Fehlermeldung w�re ganz nett");
		SDL_Quit();
		exit(0);
	}

	if (!ws.texture) {
		printf("Keine Texture gefunden");
		exit(0);
	}
	string file = string(basePath)+"../../levels/map.txt";
	getMap(ws, obstacles, file);
	loadLevelAndDraw(ws, obstacles);
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
	ws.appliedForce = { 0,0 };

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
	/*if (ws.playerVelocity.x < ws.movement && ws.movement > 0) {
		ws.playerVelocity.x = ws.movement;
	}
	if (ws.playerVelocity.x > ws.movement && ws.movement < 0) {
		ws.playerVelocity.x = ws.movement;
	}*/

	while (SDL_PollEvent(&event)){
		if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_UP && ws.jumps < 2) {
			ws.jumps++;
			ws.playerVelocity.y = v.JUMP * -1;
			ws.ground = false;
		}
		if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) 
		{
			ws.hookGoal.x = event.button.x + camera.x;
			ws.hookGoal.y = event.button.y + camera.y;
			ws.hookNoObstacleFound = false;
			ws.hookFlying = true;
			ws.hookConnected = false;
			ws.amountOfHookTicks = 1;
		}
		if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT) {
			ws.hookFlying = false;
			ws.hookNoObstacleFound = true;
			ws.hookConnected = false;
			ws.amountOfHookTicks = 1;
		}
	}
}

void handle_camera(WorldState& ws) {
	camera.x = (ws.player.x + ws.player.w / 2) - SCREEN_WIDTH  /  2;
	camera.y = (ws.player.y + ws.player.h / 2) - SCREEN_HEIGHT / 2;
	
	/*if (camera.x < 0)
	{
		camera.x = 0;
	}
	if (camera.y < 0)
	{
		camera.y = 0;
	}*/
}

void mutateWorldState(WorldState& ws, const vector<SDL_FRect>& obstacles) {
	vector2 hookPull = { 0,0 };
	vector2 speedVector = { 0,0 };
	velocity v;	
	if (ws.hookFlying && !ws.hookConnected && !ws.hookNoObstacleFound) {
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
				break;
			}
		}
		if (!ws.hookConnected && abs(ws.hookPosition.x - ws.hookGoal.x) < 2 && abs(ws.hookPosition.y - ws.hookGoal.y) < 2) {
			ws.hookNoObstacleFound = true;
			//ws.hookGoal = ws.hookPosition;
			ws.hookFlying = false;
		}
	}
	/*else {
		if (ws.hookNoObstacleFound) {
			vector2 direction = calculateDirection(ws.player, ws.hookGoal);
			ws.amountOfHookTicks--;
			ws.hookPosition.x = ws.player.x + ws.player.w/2 + (direction.x * (ws.HOOKFLYINGSPEED * ws.amountOfHookTicks));
			ws.hookPosition.y = ws.player.y + ws.player.h/2 + (direction.y * (ws.HOOKFLYINGSPEED * ws.amountOfHookTicks));
			if (!ws.hookConnected && abs(ws.hookPosition.x - (float(ws.player.x) + float(ws.player.w) / 2)) < 2 
				&& abs(ws.hookPosition.y - (float(ws.player.y) + float(ws.player.h) / 2)) < 2) {
				ws.hookNoObstacleFound = false;
			}
		}
	}*/	
	
	if (!ws.ground && !ws.hookConnected) {
		ws.appliedForce.y += v.GRAVITY;
		ws.appliedForce.x *= v.AIR_FRICTION;
	}
	else if (ws.ground){
		ws.jumps = 0;
		if (ws.playerVelocity.x > v.GROUND_FRICTION) {
			ws.playerVelocity.x += v.GROUND_FRICTION * -1;
		}
		else {
			ws.playerVelocity.x += v.GROUND_FRICTION;
		}
		//ws.playerVelocity.x = 0;
	}

	if (ws.hookConnected) {
		hookPull = calculateDirection(ws.player, ws.hookGoal);
		if (hookPull.y < 0) {
			//stronger pull upwards
			speedVector.y += hookPull.y * ws.HOOKSTRENGHT * 1.5;
		}
		else
			speedVector.y += hookPull.y * ws.HOOKSTRENGHT * 0.85;
		if (hookPull.x < 0 && ws.movement < 0) {
			//dampen pull when moving in opposite direction
			speedVector.x += hookPull.x * ws.HOOKSTRENGHT * 0.9;
		}
		else
			speedVector.x += hookPull.x * ws.HOOKSTRENGHT;

		/*
		if (speedVector.x > 0 && speedVector.x > ws.appliedForce.x) {
			ws.appliedForce.x += speedVector.x;
		}		
		if (speedVector.x < 0 && speedVector.x < ws.appliedForce.x) {
			ws.appliedForce.x += speedVector.x;
		}
		if (speedVector.y > 0 && speedVector.y > ws.appliedForce.y) {
			ws.appliedForce.y += speedVector.y;
			ws.appliedForce.y += v.GRAVITY *-1;
		}
		if (speedVector.y < 0 && speedVector.y < ws.appliedForce.y) {
			ws.appliedForce.y += speedVector.y;
			ws.appliedForce.y += v.GRAVITY;
		}*/
		ws.appliedForce += speedVector;

		// Allow upward movement when connected to the hook
		if (speedVector.y < 0) {
			ws.playerVelocity.y += speedVector.y - v.GRAVITY;
		}
		else {
			ws.playerVelocity.y += speedVector.y + v.GRAVITY;
		}
	}

	ws.ground = false;

	ws.playerVelocity += ws.appliedForce;
	ws.playerVelocity.x = SDL_clamp(ws.playerVelocity.x, -v.MAXSPEED, v.MAXSPEED);
	ws.playerVelocity.y = SDL_clamp(ws.playerVelocity.y, -v.MAXSPEED, v.MAXSPEED);
	if (ws.playerVelocity.x < 1 && ws.playerVelocity.x > -1) {
		ws.playerVelocity.x = 0;
	}
	ws.player.y += ws.playerVelocity.y;

	for (const auto& obstacle : obstacles) {
		if (SDL_HasIntersectionF(&ws.player, &obstacle)) {
			if (ws.playerVelocity.y > 0) {
				ws.player.y = obstacle.y - ws.player.h;
				ws.ground = true;
				ws.playerVelocity.y = 0;
			}
			else {
				ws.player.y = obstacle.y + obstacle.h;
				ws.ground = false;
				ws.playerVelocity.y = 0;
			}
		}
	}

	ws.player.x += ws.playerVelocity.x + ws.movement;
	//ws.speedX -= ws.movement;
	for (const auto& obstacle : obstacles) {
		if (SDL_HasIntersectionF(&ws.player, &obstacle)) {
			if (ws.playerVelocity.x+ws.movement > 0) {
				ws.player.x = obstacle.x - ws.player.w;
				ws.playerVelocity.x = 0;
			}
			else {
				ws.player.x = obstacle.x + obstacle.w;
				ws.playerVelocity.x = 0;
			}
		}
	}
}

void initSpriteSheet(WorldState& ws) {
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 11; j++)
		{
			ws.spriteSheet[i][j].x = 64 * i;
			ws.spriteSheet[i][j].y = 64 * j;
			ws.spriteSheet[i][j].w = 64;
			ws.spriteSheet[i][j].h = 64;
		}
	}
}

void renderGraphics(WorldState& ws, const vector<SDL_FRect>& obstacles) {
	SDL_SetRenderDrawColor(ws.renderer, 0, 0, 0, 255);
	SDL_RenderClear(ws.renderer);
	if (ws.hookFlying || ws.hookConnected || !ws.hookNoObstacleFound) {
		SDL_SetRenderDrawColor(ws.renderer, 0, 255, 0, 255);
		SDL_RenderDrawLine(ws.renderer, ws.player.x - camera.x + ws.player.w / 2, ws.player.y - camera.y + ws.player.h / 2, ws.hookPosition.x - camera.x, ws.hookPosition.y - camera.y);
	}

	SDL_SetRenderDrawColor(ws.renderer, 255, 0, 0, 255);
	//for (const auto& obstacle : obstacles) {
	//	SDL_FRect rect = obstacle;
	//	rect.x = rect.x - camera.x;
	//	rect.y = rect.y - camera.y;

	//	// Check if the obstacle is within the camera's bounds (screen view)
	//	if (rect.x + rect.w > 0 && rect.x < SCREEN_WIDTH && rect.y + rect.h > 0 && rect.y < SCREEN_HEIGHT) {
	//		int obstacleValue = ws.map[static_cast<int>(rect.x/TILESIZE)][static_cast<int>(rect.y/TILESIZE)];
	//		drawObstacle(ws, rect, obstacleValue);
	//		//SDL_RenderFillRectF(ws.renderer, &rect);
	//	}
	//}

	int mapHeight = ws.map.size();
	int i = 0;
	for (int row = 0; row < mapHeight; row++) {
		int mapWidth = ws.map[row].size();
		for (int col = 0; col < mapWidth; col++) {
			int tileValue = ws.map[row][col];
			if (tileValue > 0) {
				SDL_FRect rect = obstacles[i];
				i++;

				rect.x -= camera.x;
				rect.y -= camera.y;
				
				if (rect.x + rect.w > 0 && rect.x < SCREEN_WIDTH && rect.y + rect.h > 0 && rect.y < SCREEN_HEIGHT) {
					drawObstacle(ws, rect, ws.map[row][col]);
				}
			}
		}
	}


	SDL_SetRenderDrawColor(ws.renderer, 0, 0, 255, 255);
	SDL_Rect playerRect = { ws.player.x - camera.x, ws.player.y - camera.y, ws.player.w, ws.player.h };
	SDL_RenderFillRect(ws.renderer, &playerRect);
	SDL_RenderPresent(ws.renderer);
	
}

int main(int argc, char* args[])
{
	WorldState worldState;
	initSpriteSheet(worldState);
	initWorldState(worldState);
	 //obstacles = {
		//{0, 550, 1050, TILESIZE}, // Example ground object
		//{200, 400, 100, TILESIZE}, // Example obstacle
		//{400, 300, 350, TILESIZE},  // Another example obstacle
		//{850, 200, TILESIZE, 300},  // Wall example obstacle
		//{750, 850, 1050, TILESIZE }, // Example ground object
		//{1500, 650, TILESIZE, TILESIZE }, // Example ground object
	//};

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