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
	vector2& operator*=(const double other) {
		this->x *= other;
		this->y *= other;
		return *this;
	}
};

struct SDL_FRect_P {
	SDL_FRect rect;
	int obstacleValue;
};

struct WorldState {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* image;
	SDL_Texture* texture;
	SDL_Texture* texture2;
	SDL_Texture* hookTexture;
	SDL_Texture* linkTexture;
	SDL_Texture* playerTexture;
	SDL_FRect player = { 2*TILESIZE, 2*TILESIZE, TILESIZE, TILESIZE };
	SDL_FPoint hookGoal = { 0, 0 };
	SDL_FPoint hookPosition = { 0, 0 };
	SDL_Rect spriteSheet[16][12] = { 0, 0, TILESIZE , TILESIZE };
	vector<vector<int>>  map;
	vector2 playerVelocity = { 0, 0 };
	vector2 appliedForce = { 0, 0 };
	bool hookFlying = false;
	bool hookConnected = false;
	bool hookNoObstacleFound = true;
	int amountOfHookTicks = 1;
	float MAXHOOKLENGTH = 350;
	float HOOKFLYINGSPEED = 35;
	float HOOKSTRENGTH = 1.75;
	bool ground = false;
	bool canJump = true;
	double maxMovement = 0;

};

struct {
	float x;
	float y;
} 
camera = { 0, 0, };

struct velocity {
	double MAXSPEED = 20;
	double GROUNDMOVEMENT = 7.5;
	double AIRMOVEMENT = 4;
	double JUMP = 12;
	double GRAVITY = 0.5;
	double SLIDING = 0.87;
	double MOVEMENTACCELERATION = 0.5;
};

enum obstacleID { //UDLR = Directions, C = curved
	EMPTY,					//0
	EARTH,						//1
	U_GRASS,					//2
	R_GRASS,					//3
	D_GRASS,					//4
	L_GRASS,					//5
	ULC_GRASS,			//6
	URC_GRASS,			//7
	DLC_GRASS,				//8
	DRC_GRASS,			//9
	RC_GRASS,				//10
	LC_GRASS,				//11
	UR_GRASS,				//12
	UL_GRASS,				//13
	DL_GRASS,				//14
	DR_GRASS,				//15
	R_EARTH,					//16
	D_EARTH,					//17
	L_EARTH,					//18
	DLC_EARTH,				//19
	DRC_EARTH,				//20
	DLCORNER_EARTH, //21
	DRCORNER_EARTH, //22
	URCLONGER_GRASS, //23
	ULCLONGER_GRASS, //24
	UNHOOKABLE, //25

};

vector<SDL_FRect_P> obstacles;

void drawObstacle(const WorldState& ws, SDL_FRect destRect,int obstacleValue)
{
	obstacleID obstacle = static_cast<obstacleID>(obstacleValue);

	switch (obstacle) {
	case (EMPTY):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[6][0], &destRect);    break;
	case (EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[1][0], &destRect);    break;
	case (U_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[0][1], &destRect);    break;
	case (R_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[1][1], &destRect);    break;
	case (D_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[2][1], &destRect);    break;
	case (L_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[3][1], &destRect);    break;
	case (ULC_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[4][0], &destRect);    break;
	case (URC_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[5][0], &destRect);    break;
	case (DLC_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[3][2], &destRect);    break;
	case (DRC_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[2][2], &destRect);    break;
	case (RC_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[6][2], &destRect);    break;
	case (LC_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[6][1], &destRect);    break;
	case (UR_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[1][3], &destRect);    break;
	case (UL_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[0][3], &destRect);    break;
	case (DL_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[2][10], &destRect);    break;
	case (DR_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[2][10], &destRect);    break;
	case (R_EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[5][1], &destRect);    break;
	case (D_EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[4][3], &destRect);    break;
	case (L_EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[4][1], &destRect);    break;
	case (DLC_EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[4][2], &destRect);    break;
	case (DRC_EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[5][2], &destRect);    break;
	case (DLCORNER_EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[6][3], &destRect);    break;
	case (DRCORNER_EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[5][3], &destRect);    break;
	case (ULCLONGER_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[0][2], &destRect);    break;
	case (URCLONGER_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[1][2], &destRect);    break;

	case (UNHOOKABLE):
		SDL_RenderCopyF(ws.renderer, ws.texture2, &ws.spriteSheet[0][1], &destRect);	break;
	default:
		printf("Unknown object\n");
		break;
	}

}

void getMap(WorldState& ws, const string& level)
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

void loadLevelAndDraw(WorldState& ws, vector<SDL_FRect_P>& obstacles)
{
	obstacles.clear();
	for (int row = 0; row < ws.map.size(); row++){
		for (int col = 0; col < ws.map[row].size(); col++){
			int value = ws.map[row][col];

			if (value > 0) {
				SDL_FRect_P rect_p = {
					{
						col* TILESIZE,
						row* TILESIZE,
						TILESIZE,
						TILESIZE
					},
					value
				};
				SDL_FRect rect = rect_p.rect;
				if (obstacles.empty() || obstacles.back().rect.x != rect.x || obstacles.back().rect.y != rect.y) {
					obstacles.push_back(rect_p);
				}
				drawObstacle(ws, rect, value);
			}
		}
	}
}


void initWorldState(WorldState& ws) {
	ws.window = SDL_CreateWindow("Captain Hook", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	ws.renderer = SDL_CreateRenderer(ws.window, -1, SDL_RENDERER_PRESENTVSYNC);

	string basePath = SDL_GetBasePath();
	basePath = basePath + "../../";
	string texturePath = basePath + "/Images/grass_main.png";
	string texturePath2 = basePath + "/Images/generic_unhookable.png";
	string texturePath3 = basePath + "/Images/monkey.png";
	string texturePathHook = basePath + "/Images/hook.png";
	string texturePathChain = basePath + "/Images/link.png";
	ws.texture = IMG_LoadTexture(ws.renderer, texturePath.c_str());
	ws.texture2 = IMG_LoadTexture(ws.renderer, texturePath2.c_str());
	ws.playerTexture = IMG_LoadTexture(ws.renderer, texturePath3.c_str());
	ws.hookTexture = IMG_LoadTexture(ws.renderer, texturePathHook.c_str());
	ws.linkTexture = IMG_LoadTexture(ws.renderer, texturePathChain.c_str());


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
	string file = string(basePath)+"/levels/map.txt";
	getMap(ws, file);
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

void readEvents(WorldState& ws, const vector<SDL_FRect_P>& obstacles) {
	SDL_PumpEvents();
	const Uint8* state = SDL_GetKeyboardState(NULL);
	SDL_Event event;
	velocity v;
	ws.appliedForce = { 0,0 };
	double movement = 0;

	if (!ws.ground) {
		movement = v.AIRMOVEMENT;
	}
	else {
		movement = v.GROUNDMOVEMENT;
	}

	if (state[SDL_SCANCODE_A]) {
		ws.maxMovement = movement * -1;
	}
	else {
		if (state[SDL_SCANCODE_D]) {
			ws.maxMovement = movement;
		}
		else {
			ws.maxMovement = 0;
		}
	}

	if (state[SDL_SCANCODE_LCTRL] && state[SDL_SCANCODE_C]) {
		SDL_DestroyWindow(ws.window);
		SDL_DestroyRenderer(ws.renderer);
		SDL_Quit();
		exit(0);
	}

	while (SDL_PollEvent(&event)){
		if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_W && ws.canJump) {
			if (!ws.ground) {
				ws.canJump = false;
				ws.ground = false;
				cout << "DOUBLE JUMPED" << endl;
			}
			else {
				ws.ground = false;
				cout << "JUMPED" << endl;
			}
			ws.playerVelocity.y = v.JUMP * -1;
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

void mutateWorldState(WorldState& ws, const vector<SDL_FRect_P>& obstacles, double deltaT) {
	vector2 hookPull = { 0,0 };
	vector2 speedVector = { 0,0 };
	velocity v;	
	if (ws.hookFlying && !ws.hookConnected && !ws.hookNoObstacleFound) {
		vector2 direction = calculateDirection(ws.player, ws.hookGoal);
		ws.hookPosition.x = ws.player.x + ws.player.w / 2 + (direction.x * ws.HOOKFLYINGSPEED * ws.amountOfHookTicks);// * deltaT;
		ws.hookPosition.y = ws.player.y + ws.player.h / 2 + (direction.y * ws.HOOKFLYINGSPEED * ws.amountOfHookTicks);// * deltaT;
		ws.amountOfHookTicks++;
		SDL_FRect currentRect = { ws.hookPosition.x, ws.hookPosition.y, 1, 1 };
		//calculate hook length
		double hookLength = sqrt(pow(ws.hookPosition.x - ws.player.x + ws.player.w / 2, 2) + pow(ws.hookPosition.y - ws.player.y + ws.player.h / 2, 2));
		if (hookLength > ws.MAXHOOKLENGTH) {
			ws.hookFlying = false;
			ws.hookNoObstacleFound = true;
		}
		else {
			for (const auto& obstacle : obstacles) {
				SDL_FRect obstacleRect = obstacle.rect;
				if (SDL_HasIntersectionF(&currentRect, &obstacleRect)) {
					if (obstacle.obstacleValue == UNHOOKABLE) {
						ws.hookNoObstacleFound = true;
						ws.hookFlying = false;
						break;
					}
					ws.hookConnected = true;
					ws.hookFlying = false;
					ws.hookGoal = ws.hookPosition;
					if (ws.playerVelocity.y * direction.y > 0)
						ws.playerVelocity.y = ws.playerVelocity.y * 0.5;
					if (ws.playerVelocity.x * direction.x > 0)
						ws.playerVelocity.x = ws.playerVelocity.x * 0.75;
					break;
				}
			}
		}
	}

	if (ws.hookConnected) {
		hookPull = calculateDirection(ws.player, ws.hookGoal);
		if (hookPull.y < 0) {
			//stronger pull upwards
			speedVector.y += hookPull.y * 1.25;
		}
		else
			speedVector.y += hookPull.y * 0.85;
		if (hookPull.x * ws.maxMovement < 0) {
			//dampen pull when moving in opposite direction
			speedVector.x += hookPull.x *0.77;
		}
		else
			speedVector.x += hookPull.x *1.2;

		speedVector.x *= ws.HOOKSTRENGTH;

		ws.playerVelocity += speedVector;
	}

	//right movement
	if (ws.maxMovement > 0) {
		if (ws.playerVelocity.x < ws.maxMovement) {
			ws.playerVelocity.x += v.MOVEMENTACCELERATION;
			if (ws.playerVelocity.x > ws.maxMovement)
				ws.playerVelocity.x = ws.maxMovement;
		}
	}
	//left movement
	else if (ws.maxMovement < 0) {
		if (ws.playerVelocity.x > ws.maxMovement) {
			ws.playerVelocity.x += v.MOVEMENTACCELERATION * -1;
			if (ws.playerVelocity.x < ws.maxMovement)
				ws.playerVelocity.x = ws.maxMovement;
		}
	}
	else {
		if (!ws.hookConnected) {
			ws.playerVelocity.x *= v.SLIDING;
			if (abs(ws.playerVelocity.x) < 1)
				ws.playerVelocity.x = 0;
		}
	}


	ws.playerVelocity.y += v.GRAVITY;

	ws.playerVelocity.x = SDL_clamp(ws.playerVelocity.x, -v.MAXSPEED, v.MAXSPEED);
	ws.playerVelocity.y = SDL_clamp(ws.playerVelocity.y, -v.MAXSPEED, v.MAXSPEED);

	ws.player.y += ws.playerVelocity.y;

	for (const auto& obstacle : obstacles) {
		SDL_FRect obstacleRect = obstacle.rect;
		if (SDL_HasIntersectionF(&ws.player, &obstacleRect)) {
			if (ws.playerVelocity.y > 0) {
				ws.player.y = obstacleRect.y - ws.player.h;
				ws.ground = true;
				ws.canJump = true;
				ws.playerVelocity.y = 0;
			}
			else {
				ws.player.y = obstacleRect.y + obstacleRect.h;
				ws.ground = false;
				ws.playerVelocity.y = 0;
			}
		}
	}

	ws.player.x += ws.playerVelocity.x;
	for (const auto& obstacle : obstacles) {
		SDL_FRect obstacleRect = obstacle.rect;
		if (SDL_HasIntersectionF(&ws.player, &obstacleRect)) {
			if (ws.playerVelocity.x > 0) {
				ws.player.x = obstacleRect.x - ws.player.w;
				ws.playerVelocity.x = 0;
			}
			else {
				if (ws.playerVelocity.x < 0){
					ws.player.x = obstacleRect.x + obstacleRect.w;
					ws.playerVelocity.x = 0;
				}
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

void renderGraphics(WorldState& ws, const vector<SDL_FRect_P>& obstacles) {
	SDL_SetRenderDrawColor(ws.renderer, 0, 0, 0, 255);
	SDL_RenderClear(ws.renderer);
	if (ws.hookFlying || ws.hookConnected || !ws.hookNoObstacleFound) {
		
		float hookLength = sqrt(pow(ws.hookPosition.x - ws.player.x + ws.player.w / 2, 2) + pow(ws.hookPosition.y - ws.player.y + ws.player.h / 2, 2));
		float angle = atan2(ws.hookGoal.y - ws.player.y, ws.hookGoal.x - ws.player.x) * 180 / M_PI;
		for (float i = hookLength; i > 0; i -= TILESIZE/2) {
			if (i == hookLength){
				SDL_Rect hookRect = { ws.player.x + ws.player.w / 4 + (ws.hookPosition.x - ws.player.x) * i / hookLength - camera.x, ws.player.y + ws.player.h / 4 + (ws.hookPosition.y - ws.player.y) * i / hookLength - camera.y, TILESIZE / 2, TILESIZE / 2 };
				SDL_RenderCopyEx(ws.renderer, ws.hookTexture, &ws.spriteSheet[0][0], &hookRect, angle + 90, NULL, SDL_FLIP_NONE);
			}
			else {
				SDL_Rect linkRect = { ws.player.x + ws.player.w / 4 + (ws.hookPosition.x - ws.player.x) * i / hookLength - camera.x, ws.player.y + ws.player.h / 4 + (ws.hookPosition.y - ws.player.y) * i / hookLength - camera.y, TILESIZE / 2, TILESIZE / 2 };
				SDL_RenderCopyEx(ws.renderer, ws.linkTexture, &ws.spriteSheet[0][0], &linkRect, angle + 90, NULL, SDL_FLIP_NONE);
			}
		}		
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
				SDL_FRect_P rect_p = obstacles[i];
				i++;

				SDL_FRect rect = rect_p.rect;
				rect.x -= camera.x;
				rect.y -= camera.y;
				
				if (rect.x + rect.w > 0 && rect.x < SCREEN_WIDTH && rect.y + rect.h > 0 && rect.y < SCREEN_HEIGHT) {
					drawObstacle(ws, rect, ws.map[row][col]);
				}
			}
		}
	}


	SDL_Rect playerRect = { ws.player.x - camera.x, ws.player.y - camera.y, ws.player.w, ws.player.h };
	SDL_Rect monkey = { 0, 0, 256, 256 };
	SDL_RenderCopy(ws.renderer, ws.playerTexture, &monkey, &playerRect);
	SDL_RenderPresent(ws.renderer);
	
}

int main(int argc, char* args[])
{
	WorldState worldState;
	initSpriteSheet(worldState);
	initWorldState(worldState);
	Uint64 NOW = SDL_GetPerformanceCounter();
	Uint64 LAST = 0;
	double deltaT = 0;

	while (true) {
		LAST = NOW;
		NOW = SDL_GetPerformanceCounter();

		deltaT = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());
		readEvents(worldState, obstacles);
		mutateWorldState(worldState, obstacles, deltaT);
		handle_camera(worldState);
		renderGraphics(worldState, obstacles);
	}

	SDL_DestroyTexture(worldState.texture);
	SDL_DestroyRenderer(worldState.renderer);
	IMG_Quit();
	SDL_Quit();
	return 0;
}