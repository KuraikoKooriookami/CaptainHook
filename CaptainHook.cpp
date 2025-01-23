#include <SDL.h>
#include<vector> 
#include<SDL_image.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <SDL_ttf.h>
#include <cmath>

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
	SDL_Texture* textureText;
	TTF_Font* font;
	SDL_Texture* hookTexture;
	SDL_Texture* linkTexture;
	SDL_Texture* rodTexture;
	SDL_Texture* flagTexture;
	SDL_Texture* spikeTexture;
	SDL_Texture* playerTexture;
	SDL_FRect player = { 0*TILESIZE, 0*TILESIZE, TILESIZE, TILESIZE };
	SDL_FRect hurtbox = { 0 * TILESIZE, 0 * TILESIZE, 16, 16 };
	SDL_FPoint hookGoal = { 0, 0 };
	SDL_FPoint hookPosition = { 0, 0 };
	SDL_Rect spriteSheet[16][12] = { 0, 0, TILESIZE , TILESIZE };
	vector<vector<int>>  map;
	int stage = 1;
	vector2 playerVelocity = { 0, 0 };
	vector2 appliedForce = { 0, 0 };
	bool hookFlying = false;
	bool hookConnected = false;
	bool hookNoObstacleFound = true;
	bool respawn = false;
	int amountOfHookTicks = 1;
	float MAXHOOKLENGTH = 350;
	float HOOKFLYINGSPEED = 35;
	float HOOKSTRENGTH = 1.75;
	bool ground = false;
	int jumps = 0;
	double maxMovement = 0;
	int deaths = 0;
};

struct {
	int x;
	int y;
}
	spawnLocation;

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
	NOTSET = -1,
	EMPTY = 0,			//0
	EARTH = 1,						//1
	ULC_GRASS,			//6
	URC_GRASS,			//7
	DRC_GRASS,			//9
	DLC_GRASS,				//8
	LC_GRASS,				//11
	U_GRASS,					//2
	R_GRASS,					//3
	D_GRASS,					//4
	L_GRASS,					//5
	L_EARTH,					//18
	R_EARTH,					//16
	RC_GRASS,				//10
	ULCLONGER_GRASS, //23
	URCLONGER_GRASS, //24
	UL_GRASS,				//13
	UR_GRASS,				//12
	DLC_EARTH,				//19
	DRC_EARTH,				//20
	DR_GRASS = 21,				
	DL_GRASS = 22,				
	D_EARTH = 23,					
	DRCORNER_EARTH = 24, 
	DLCORNER_EARTH = 25, 
	UNHOOKABLE = 26,
	SPIKEBALL = 27,
	LSPEAR_TIP = 28,
	LSPEAR_ROD = 29,
	USPEAR_TIP = 30,
	DSPEAR_ROD= 31,
	RSPEAR_ROD = 35,
	RSPEAR_TIP = 36,
	USPEAR_ROD = 37,
	DSPEAR_TIP = 38,
	FENCE = 39,

	PLAYER = 42,
	FINISH_ROD = 43,
	FINISH_FLAG = 44,
};

vector<SDL_FRect_P> obstacles;

void drawObstacle(const WorldState& ws, SDL_FRect destRect,int obstacleValue)
{
	obstacleID obstacle = static_cast<obstacleID>(obstacleValue);
	destRect.x = ceil(destRect.x);
	destRect.y = ceil(destRect.y);
	switch (obstacle) {
	case (NOTSET):
	case (EMPTY):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[0][0], &destRect);    break; //done
	case (EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[1][0], &destRect);    break; //done
	case (ULC_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[2][0], &destRect);    break;//done
	case (URC_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[3][0], &destRect);    break;//done
	case (DRC_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[4][0], &destRect);    break;//done
	case (DLC_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[5][0], &destRect);    break;//done
	case (LC_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[6][0], &destRect);    break;//done
	case (U_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[0][1], &destRect);    break; //done
	case (R_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[1][1], &destRect);    break; //done
	case (D_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[2][1], &destRect);    break;//done
	case (L_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[3][1], &destRect);    break;//done
	case (L_EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[4][1], &destRect);    break;//done
	case (R_EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[5][1], &destRect);    break;//done
	case (RC_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[6][1], &destRect);    break;//done
	case (ULCLONGER_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[0][2], &destRect);    break;
	case (URCLONGER_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[1][2], &destRect);    break;
	case (UL_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[2][2], &destRect);    break;//done
	case (UR_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[3][2], &destRect);    break;//done
	case (DLC_EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[4][2], &destRect);    break;//done
	case (DRC_EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[5][2], &destRect);    break;//done
	case (DR_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[0][3], &destRect);    break;
	case (DL_GRASS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[1][3], &destRect);    break;
	case (D_EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[2][3], &destRect);    break;//done
	case (DRCORNER_EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[3][3], &destRect);    break;//done
	case (DLCORNER_EARTH):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[4][3], &destRect);    break;//done
	case (UNHOOKABLE):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[5][3], &destRect);	break;
	case(SPIKEBALL):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[6][3], &destRect); break;
	case(LSPEAR_TIP):
			SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[0][4], &destRect); break;
	case(LSPEAR_ROD):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[1][4], &destRect); break;
	case(USPEAR_TIP):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[2][4], &destRect); break;
	case(DSPEAR_ROD):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[3][4], &destRect); break;
	case(RSPEAR_ROD):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[0][5], &destRect); break;
	case(RSPEAR_TIP):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[1][5], &destRect); break;
	case(USPEAR_ROD):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[2][5], &destRect); break;
	case(DSPEAR_TIP):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[3][5], &destRect); break;
	case(FENCE):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[4][5], &destRect); break;
	case (FINISH_ROD):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[1][6], &destRect); break;
	case(FINISH_FLAG):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[2][6], &destRect); break;
	default:
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
					[](unsigned char c) { return !isdigit(c) && c != '-'; }),
					number.end());

					if (!number.empty()) {
					{
						int value = stoi(number);
						rowData.push_back(value);

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
}

void loadLevelAndDraw(WorldState& ws, vector<SDL_FRect_P>& obstacles)
{
	obstacles.clear();
	for (int row = 0; row < ws.map.size(); row++){
		for (int col = 0; col < ws.map[row].size(); col++){
			int value = ws.map[row][col];

			if (ws.map[row][col] == 42) {
				spawnLocation.x = col * TILESIZE;
				spawnLocation.y = row * TILESIZE;
				ws.player.x = spawnLocation.x;
				ws.player.y = spawnLocation.y;
				ws.hurtbox.x = ws.player.x + 4;
				ws.hurtbox.y = ws.player.y + 4;
				continue;
			}

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

void resetPlayer(WorldState& ws) {
	ws.playerVelocity.x = 0;
	ws.playerVelocity.y = 0;
	ws.player.x = spawnLocation.x;
	ws.player.y = spawnLocation.y;
	ws.hurtbox.x = ws.player.x + 4;
	ws.hurtbox.y = ws.player.y + 4;
	ws.respawn = false;
	ws.hookConnected = false;
	ws.hookNoObstacleFound = true;
	ws.hookFlying = false;
}

void initWorldState(WorldState& ws) {
	ws.window = SDL_CreateWindow("Captain Hook", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	ws.renderer = SDL_CreateRenderer(ws.window, -1, SDL_RENDERER_PRESENTVSYNC);

	string basePath = SDL_GetBasePath();
	basePath = basePath + "../../";
	string texturePath = basePath + "/Images/SpriteSheet_Copy.png";
	string texturePathHook = basePath + "/Images/hook.png";
	string texturePathChain = basePath + "/Images/link.png";
	string fontPath = basePath + "/monogram.ttf";
	ws.texture = IMG_LoadTexture(ws.renderer, texturePath.c_str());
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
	if (TTF_Init() == -1) {
		printf("Failed to initialize TTF: %s\n", TTF_GetError());
		exit(0);
	}
	ws.font = TTF_OpenFont(fontPath.c_str(), 24);
	if (!ws.font) {
		printf("Failed to load font: %s\n", TTF_GetError());
		exit(0);
	}

	if (!ws.texture) {
		printf("Keine Texture gefunden");
		exit(0);
	}
	string file = string(basePath) + "/levels/map" + to_string(ws.stage) + ".txt";
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

	if (state[SDL_SCANCODE_LCTRL] && state[SDL_SCANCODE_D]) {
		resetPlayer(ws);
	}

	while (SDL_PollEvent(&event)){
		if (event.type == SDL_KEYDOWN && (event.key.keysym.scancode == SDL_SCANCODE_W || event.key.keysym.scancode == SDL_SCANCODE_SPACE)) {
			if (ws.jumps == 1) {
				ws.jumps = 2;
				ws.ground = false;
				ws.playerVelocity.y = v.JUMP * -1;
			}
			else if(ws.jumps == 0){
				ws.ground = false;
				ws.jumps = 1;
				ws.playerVelocity.y = v.JUMP * -1;
			}
			//ws.playerVelocity.y = v.JUMP * -1;
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

void mutateWorldState(WorldState& ws, vector<SDL_FRect_P>& obstacles, double deltaT) {
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
					if (obstacle.obstacleValue == UNHOOKABLE || obstacle.obstacleValue == SPIKEBALL) {
						ws.hookNoObstacleFound = true;
						ws.hookFlying = false;
						break;
					}
					if (obstacle.obstacleValue >= 28) {
						continue;
					}
					ws.hookConnected = true;
					ws.hookFlying = false;
					ws.hookGoal = ws.hookPosition;
					if (ws.playerVelocity.y * direction.y > 0)
						ws.playerVelocity.y = ws.playerVelocity.y * 0.4;
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
			speedVector.x += hookPull.x * 0.77;
		}
		else
			speedVector.x += hookPull.x * 1.2;

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
			if (!ws.ground) {
				ws.playerVelocity.x *= v.SLIDING * 1.1;

			}
			else {
				ws.playerVelocity.x *= v.SLIDING;
			}
			if (abs(ws.playerVelocity.x) < 1)
				ws.playerVelocity.x = 0;
		}
	}


	ws.playerVelocity.y += v.GRAVITY;

	ws.playerVelocity.x = SDL_clamp(ws.playerVelocity.x, -v.MAXSPEED, v.MAXSPEED);
	ws.playerVelocity.y = SDL_clamp(ws.playerVelocity.y, -v.MAXSPEED, v.MAXSPEED);

	ws.player.y += ws.playerVelocity.y;
	ws.hurtbox.y = ws.player.y + 4;

	if (ws.playerVelocity.y != 0 && ws.ground == true) {
		ws.jumps = 1;
		ws.ground = false;
	}

	for (const auto& obstacle : obstacles) {
		SDL_FRect obstacleRect = obstacle.rect;
		if (SDL_HasIntersectionF(&ws.player, &obstacleRect)) {
			if (obstacle.obstacleValue != SPIKEBALL && obstacle.obstacleValue != USPEAR_TIP && obstacle.obstacleValue != RSPEAR_TIP && obstacle.obstacleValue != DSPEAR_TIP && obstacle.obstacleValue != LSPEAR_TIP) {
				if (ws.playerVelocity.y > 0) {
					ws.player.y = obstacleRect.y - ws.player.h;
					ws.hurtbox.y = ws.player.y + 4;
					ws.ground = true;
					ws.jumps = 0;
					ws.playerVelocity.y = 0;
				}
				else {
					ws.player.y = obstacleRect.y + obstacleRect.h;
					ws.hurtbox.y = ws.player.y + 4;
					ws.ground = false;
					ws.playerVelocity.y = 0;
				}
				if (obstacle.obstacleValue >= 43) {
					if (obstacle.obstacleValue == FINISH_FLAG || obstacle.obstacleValue == FINISH_ROD) {
						ws.stage++;
						string basePath = SDL_GetBasePath();
						basePath = basePath + "../../";
						string file = string(basePath) + "/levels/map" + to_string(ws.stage) + ".txt";
						ws.map.clear();
						getMap(ws, file);
						loadLevelAndDraw(ws, obstacles);
						ws.deaths--;
					}
					ws.deaths++;
					resetPlayer(ws);
					break;
				}
			}
		}
		if (SDL_HasIntersectionF(&ws.hurtbox, &obstacleRect)) {
			if (obstacle.obstacleValue == SPIKEBALL || obstacle.obstacleValue == USPEAR_TIP || obstacle.obstacleValue == RSPEAR_TIP || obstacle.obstacleValue == DSPEAR_TIP || obstacle.obstacleValue == LSPEAR_TIP) {
				ws.deaths++;
				resetPlayer(ws);
			}
		}
	}

	ws.player.x += ws.playerVelocity.x;
	ws.hurtbox.x = ws.player.x + 4;

	for (const auto& obstacle : obstacles) {
		SDL_FRect obstacleRect = obstacle.rect;
		if (SDL_HasIntersectionF(&ws.player, &obstacleRect)) {
			if (obstacle.obstacleValue != SPIKEBALL && obstacle.obstacleValue != USPEAR_TIP && obstacle.obstacleValue != RSPEAR_TIP && obstacle.obstacleValue != DSPEAR_TIP && obstacle.obstacleValue != LSPEAR_TIP) {
				if (ws.playerVelocity.x > 0) {
					ws.player.x = obstacleRect.x - ws.player.w;
					ws.hurtbox.x = ws.player.x + 4;
					ws.playerVelocity.x = 0;
				}
				else {
					if (ws.playerVelocity.x < 0) {
						ws.player.x = obstacleRect.x + obstacleRect.w;
						ws.hurtbox.x = ws.player.x + 4;
						ws.playerVelocity.x = 0;
					}
				}
				if (obstacle.obstacleValue >= 43) {
					if (obstacle.obstacleValue == FINISH_FLAG || obstacle.obstacleValue == FINISH_ROD) {
						ws.stage++;
						string basePath = SDL_GetBasePath();
						basePath = basePath + "../../";
						string file = string(basePath) + "/levels/map" + to_string(ws.stage) + ".txt";
						ws.map.clear();
						getMap(ws, file);
						loadLevelAndDraw(ws, obstacles);
						ws.deaths--;
					}
					ws.deaths++;
					resetPlayer(ws);
					break;
				}
			}
		}
		if (SDL_HasIntersectionF(&ws.hurtbox, &obstacleRect)) {
			if (obstacle.obstacleValue == SPIKEBALL || obstacle.obstacleValue == USPEAR_TIP || obstacle.obstacleValue == RSPEAR_TIP || obstacle.obstacleValue == DSPEAR_TIP || obstacle.obstacleValue == LSPEAR_TIP) {
				ws.deaths++;
				resetPlayer(ws);
			}
		}
	}
}

void initSpriteSheet(WorldState& ws) {
	for (int i = 0; i < 7; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			ws.spriteSheet[i][j].x = 64 * i;
			ws.spriteSheet[i][j].y = 64 * j;
			ws.spriteSheet[i][j].w = 64;
			ws.spriteSheet[i][j].h = 64;
		}
	}
}

void renderGraphics(WorldState& ws, const vector<SDL_FRect_P>& obstacles) {
	SDL_Color Black = { 0, 0, 0, 0 };
	SDL_SetRenderDrawColor(ws.renderer, 89, 181, 226, 0);
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

	int mapHeight = ws.map.size();
	int i = 0;
	for (int row = 0; row < mapHeight; row++) {
		int mapWidth = ws.map[row].size();
		for (int col = 0; col < mapWidth; col++) {
			int tileValue = ws.map[row][col];
			if (tileValue > 0 && tileValue != 42) {
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

	if (ws.stage == 1)
	{
		SDL_Surface* surfaceMessage = TTF_RenderText_Solid(ws.font, "Press 'A' or 'D' to Walk", Black);
		SDL_Texture* Message = SDL_CreateTextureFromSurface(ws.renderer, surfaceMessage);
		SDL_FRect Message_rect = { 200-camera.x, 1150-camera.y, 300, 50 };
		SDL_RenderCopyF(ws.renderer, Message, NULL, &Message_rect);
		surfaceMessage = TTF_RenderText_Solid(ws.font, "Press 'W' to Jump", Black);
		Message = SDL_CreateTextureFromSurface(ws.renderer, surfaceMessage);
		Message_rect = { 700 - camera.x, 1150 - camera.y, 300, 50 };
		SDL_RenderCopyF(ws.renderer, Message, NULL, &Message_rect);
		SDL_FreeSurface(surfaceMessage); 
		SDL_DestroyTexture(Message);     
		surfaceMessage = TTF_RenderText_Solid(ws.font, "Avoid These", Black);
		Message = SDL_CreateTextureFromSurface(ws.renderer, surfaceMessage);
		Message_rect = { 700 - camera.x, 1450 - camera.y, 300, 50 };
		SDL_RenderCopyF(ws.renderer, Message, NULL, &Message_rect);
		SDL_FreeSurface(surfaceMessage);
		SDL_DestroyTexture(Message);     
		surfaceMessage = TTF_RenderText_Solid(ws.font, "Press 'W' while in Air", Black);
		Message = SDL_CreateTextureFromSurface(ws.renderer, surfaceMessage);
		Message_rect = { 1475 - camera.x, 1150 - camera.y, 300, 50 };
		SDL_RenderCopyF(ws.renderer, Message, NULL, &Message_rect);
		SDL_FreeSurface(surfaceMessage);
		SDL_DestroyTexture(Message);     
		surfaceMessage = TTF_RenderText_Solid(ws.font, "Rightclick to Hook", Black);
		Message = SDL_CreateTextureFromSurface(ws.renderer, surfaceMessage);
		Message_rect = {  1200 - camera.x, 850 - camera.y, 300, 50 };
		SDL_RenderCopyF(ws.renderer, Message, NULL, &Message_rect);
		SDL_FreeSurface(surfaceMessage);
		SDL_DestroyTexture(Message);     
		surfaceMessage = TTF_RenderText_Solid(ws.font, "You cant Hook Iron Objects", Black);
		Message = SDL_CreateTextureFromSurface(ws.renderer, surfaceMessage);
		Message_rect = { 875 - camera.x, 600 - camera.y, 300, 50 };
		SDL_RenderCopyF(ws.renderer, Message, NULL, &Message_rect);
		SDL_FreeSurface(surfaceMessage);
		SDL_DestroyTexture(Message);     
		surfaceMessage = TTF_RenderText_Solid(ws.font, "You can Hook Through here", Black);
		Message = SDL_CreateTextureFromSurface(ws.renderer, surfaceMessage);
		Message_rect = { 1475 - camera.x, 600 - camera.y, 300, 50 };
		SDL_RenderCopyF(ws.renderer, Message, NULL, &Message_rect);
		SDL_FreeSurface(surfaceMessage);
		SDL_DestroyTexture(Message);     
		surfaceMessage = TTF_RenderText_Solid(ws.font, "The Goal is to reach the Flag!", Black);
		Message = SDL_CreateTextureFromSurface(ws.renderer, surfaceMessage);
		Message_rect = { 900 - camera.x, 50 - camera.y, 300, 50 };
		SDL_RenderCopyF(ws.renderer, Message, NULL, &Message_rect);
		SDL_FreeSurface(surfaceMessage);
		SDL_DestroyTexture(Message);     
	}
	string deathText = "Deaths: " + to_string(ws.deaths);
	SDL_Surface* deathMessage = TTF_RenderText_Solid(ws.font, deathText.c_str(), Black);
	SDL_Texture* message = SDL_CreateTextureFromSurface(ws.renderer, deathMessage);
	SDL_FRect death_rect = { 20, 10, 200, 50 };
	SDL_RenderCopyF(ws.renderer, message, NULL, &death_rect);
	SDL_FreeSurface(deathMessage);
	SDL_DestroyTexture(message);

	SDL_Rect playerRect = { ws.player.x - camera.x, ws.player.y - camera.y, ws.player.w, ws.player.h };
	SDL_Rect monkey = { 0 * 64, 6 * 64, 64, 64};
	SDL_RenderCopy(ws.renderer, ws.texture, &monkey, &playerRect);
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
		/*LAST = NOW;
		NOW = SDL_GetPerformanceCounter();

		deltaT = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());*/
		readEvents(worldState, obstacles);
		mutateWorldState(worldState, obstacles, deltaT);
		handle_camera(worldState);
		renderGraphics(worldState, obstacles);
	}

	SDL_DestroyTexture(worldState.texture);
	SDL_DestroyTexture(worldState.textureText);
	SDL_DestroyRenderer(worldState.renderer);
	IMG_Quit();
	SDL_Quit();
	return 0;
}