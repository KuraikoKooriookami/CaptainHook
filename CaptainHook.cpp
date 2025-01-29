#include <SDL.h>
#include<vector> 
#include<SDL_image.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <SDL_ttf.h>
#include <cmath>
#include <SDL_mixer.h>

#define SCREEN_WIDTH 850 *1.5
#define SCREEN_HEIGHT  580 *1.5
#define TILESIZE 32

using namespace std;
extern "C"
bool isRunning = true;

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
	int enemyId;
};

class Enemy {
public:
	SDL_FRect enemyRect;
	vector2 enemyVelocity = { 0, 0 };
	int enemyID;
	SDL_Point initialPosition;
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
	SDL_Texture* playermonke;
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
	int hookEnemy = 0;
	int amountOfHookTicks = 1;
	float MAXHOOKLENGTH = 350;
	float HOOKFLYINGSPEED = 35;
	float HOOKSTRENGTH = 1.45;
	bool limitedHook = false;
	int maxHookDuration = 50;
	bool ground = false;
	int jumps = 0;
	double maxMovement = 0;
	int deaths = 0;
	int killed = 0;
	int messageTimer = 250;
	string achievementMessage = "";
	bool showAchievement = false;
	Mix_Chunk* soundEffect;
	vector<Enemy> enemies;
	int enemyAmount = 0;
	bool cameraToggle = false;
	bool isFKeyPressed = false;
	bool isDead = false;
	bool respawn = false;
	int alpha =  255;
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
	double AIRSLIDING = 0.95;
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
	CRUMBLING,
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
	MONKAS = 46,
	EMPTYREPLACEMENT = 60,
};

vector<SDL_FRect_P> obstacles;
vector<SDL_FRect_P> obstaclesSorted;

void drawObstacle(const WorldState& ws, SDL_FRect destRect,int obstacleValue)
{
	obstacleID obstacle = static_cast<obstacleID>(obstacleValue);
	destRect.x = ceil(destRect.x);
	destRect.y = ceil(destRect.y);
	switch (obstacle) {
	case (NOTSET):
	case(EMPTYREPLACEMENT):
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
	case (CRUMBLING):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[6][2], &destRect);    break;//done
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
	case(MONKAS):
		SDL_RenderCopyF(ws.renderer, ws.texture, &ws.spriteSheet[4][6], &destRect); break;
	default:
		break;
	}

}

void getMap(WorldState& ws, const string& level)
{
	if (ws.stage == 5) {
		ws.achievementMessage = "Achievement Unlocked: You Completed the Game";
		ws.showAchievement = true;
	}
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

void playSound(WorldState& ws,string path) {
	string basePath = "sound/";
	string soundPath = basePath + path;
	ifstream file(soundPath);
	ws.soundEffect = Mix_LoadWAV(soundPath.c_str());
	char* base_path = SDL_GetBasePath();
	if (!ws.soundEffect) {
		std::cerr << "Failed to load sound effect: " << Mix_GetError() << std::endl;
		return;
	}
	Mix_PlayChannel(-1, ws.soundEffect, 0);
}

void loadLevelAndDraw(WorldState& ws, vector<SDL_FRect_P>& obstacles)
{
	ws.enemies.clear();
	ws.enemyAmount = 0;
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
			if (ws.map[row][col] == 46) {
				ws.enemyAmount++;
				Enemy newEnemy;
				newEnemy.enemyRect = { (float)col * TILESIZE, (float)row * TILESIZE, TILESIZE, TILESIZE };
				newEnemy.enemyID = ws.enemyAmount;
				newEnemy.initialPosition = { col, row };
				ws.enemies.push_back(newEnemy);
			}

			if (value > 0) {
				SDL_FRect_P rect_p = {
					{
						col* TILESIZE,
						row* TILESIZE,
						TILESIZE,
						TILESIZE
					},
					value,
					value == 46 ? ws.enemyAmount : 0
				};
				SDL_FRect rect = rect_p.rect;
				if (obstacles.empty() || obstacles.back().rect.x != rect.x || obstacles.back().rect.y != rect.y) {
					obstacles.push_back(rect_p);
				}
				drawObstacle(ws, rect, value);
			}
		}
	}
	obstaclesSorted.clear();
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
	ws.limitedHook = false;
	ws.maxHookDuration = 50;
	ws.hookFlying = false;
	ws.hookEnemy = false;
	ws.isDead = false;
	ws.alpha = 255;
}

void initWorldState(WorldState& ws) {
	ws.window = SDL_CreateWindow("Captain Hook", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	ws.renderer = SDL_CreateRenderer(ws.window, -1, SDL_RENDERER_PRESENTVSYNC);

	string basePath = SDL_GetBasePath();
	basePath = basePath + "../../";
	string texturePath = basePath + "/Images/SpriteSheet.png";
	string monkeyPath = basePath + "/Images/MonkeBad.png";
	string texturePathHook = basePath + "/Images/hook.png";
	string texturePathChain = basePath + "/Images/link.png";
	string fontPath = basePath + "/monogram.ttf";
	ws.texture = IMG_LoadTexture(ws.renderer, texturePath.c_str());
	ws.hookTexture = IMG_LoadTexture(ws.renderer, texturePathHook.c_str());
	ws.linkTexture = IMG_LoadTexture(ws.renderer, texturePathChain.c_str());
	ws.playermonke = IMG_LoadTexture(ws.renderer, texturePath.c_str());

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
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		printf("Failed to initialize SDL: %s\n", SDL_GetError());
		exit(0);
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("Failed to initialize SDL_mixer: %s\n", Mix_GetError());
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
	if (!ws.isDead) {
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

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN && (event.key.keysym.scancode == SDL_SCANCODE_W || event.key.keysym.scancode == SDL_SCANCODE_SPACE)) {
				if (ws.jumps == 1) {
					playSound(ws, "dblJump.wav");
					ws.jumps = 2;
					ws.ground = false;
					ws.playerVelocity.y = v.JUMP * -1;
				}
				else if (ws.jumps == 0) {
					ws.ground = false;
					ws.jumps = 1;
					ws.playerVelocity.y = v.JUMP * -1;
				}
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
				ws.hookEnemy = 0;
				ws.limitedHook = false;
				ws.maxHookDuration = 50;
				ws.amountOfHookTicks = 1;
			}
			if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_F) {
				if (!ws.isFKeyPressed) {
					ws.cameraToggle = !ws.cameraToggle;
					ws.isFKeyPressed = true;
				}
			}
			if (event.type == SDL_KEYUP && event.key.keysym.scancode == SDL_SCANCODE_F) {
				ws.isFKeyPressed = false;
			}
		}
	}

	if (state[SDL_SCANCODE_LCTRL] && state[SDL_SCANCODE_C]) {
		isRunning = false;
	}

	if (state[SDL_SCANCODE_LCTRL] && state[SDL_SCANCODE_R]) {
		resetPlayer(ws);
	}
}

void getMousePosition(int& mouseX, int& mouseY) {
	SDL_GetMouseState(&mouseX, &mouseY);
}

void handle_camera(WorldState& ws) {
	camera.x = (ws.player.x + ws.player.w / 2) - SCREEN_WIDTH  /  2;
	camera.y = (ws.player.y + ws.player.h / 2) - SCREEN_HEIGHT / 2;
	if (ws.cameraToggle) {
		int mouseX, mouseY;
		getMousePosition(mouseX, mouseY);
		camera.x += mouseX - (SCREEN_WIDTH / 2);
		camera.y += mouseY - (SCREEN_HEIGHT / 2);
	}
	
	/*if (camera.x < 0)
	{
		camera.x = 0;
	}
	if (camera.y < 0)
	{
		camera.y = 0;
	}*/
}

Enemy* getEnemyByID(vector<Enemy>& enemies, int id) {
	for (auto& e : enemies) {
		if (e.enemyID == id) {
			return &e;
		}
	}
	return nullptr;
}

SDL_FRect_P* getObstacleByEnemyId(vector<SDL_FRect_P>& obstacles, int enemyId) {
	for (auto& o : obstacles) {
		if (o.enemyId == enemyId) {
			return &o;
		}
	}
	return nullptr;
}

void checkEnemyCollision(WorldState& ws, vector<SDL_FRect_P>& obstacles) {
	velocity v;
	ws.enemies.erase(
		std::remove_if(ws.enemies.begin(), ws.enemies.end(), [&](Enemy& enemy) {
			SDL_FRect_P* o = getObstacleByEnemyId(obstacles, enemy.enemyID);
			if (abs(enemy.enemyVelocity.x) < 0.5)
				enemy.enemyVelocity.x = 0;
			enemy.enemyVelocity.x *= v.AIRSLIDING;
			enemy.enemyVelocity.y += v.GRAVITY;
			enemy.enemyVelocity.x = SDL_clamp(enemy.enemyVelocity.x, -v.MAXSPEED, v.MAXSPEED);
			enemy.enemyVelocity.y = SDL_clamp(enemy.enemyVelocity.y, -v.MAXSPEED, v.MAXSPEED);
			enemy.enemyRect.y += enemy.enemyVelocity.y;
			for (const auto& obstacle : obstacles) {
				SDL_FRect obstacleRect = obstacle.rect;
				if (SDL_HasIntersectionF(&enemy.enemyRect, &obstacleRect)) {
					if (obstacle.obstacleValue != SPIKEBALL && obstacle.obstacleValue != USPEAR_TIP && obstacle.obstacleValue != RSPEAR_TIP && obstacle.obstacleValue != DSPEAR_TIP && obstacle.obstacleValue != LSPEAR_TIP && obstacle.obstacleValue != MONKAS && obstacle.obstacleValue != EMPTYREPLACEMENT) {
						if (enemy.enemyVelocity.y > 0) {
							enemy.enemyRect.y = obstacleRect.y - enemy.enemyRect.h;
							enemy.enemyVelocity.y = 0;
						}
						else {
							enemy.enemyRect.y = obstacleRect.y + obstacleRect.h;
							enemy.enemyVelocity.y = 0;
						}
					}
					else {
						if (obstacle.obstacleValue != MONKAS && obstacle.obstacleValue != EMPTYREPLACEMENT) {
							if (o) {
								o->obstacleValue = 60;
								o->enemyId = 0;
							}
							ws.map[enemy.initialPosition.y][enemy.initialPosition.x] = 60;
							if (enemy.enemyID == ws.hookEnemy) {
								ws.hookEnemy = 0;
								ws.maxHookDuration = 50;
							}

							if (ws.killed == 1 || ws.killed == 5 || ws.killed == 10 || ws.killed == 17) {
								if (ws.killed == 1)
									ws.achievementMessage = "Achievement Unlocked: You killed your First enemy!";
								else {
									if (ws.killed == 17) {
										ws.achievementMessage = "Achievement Unlocked: You killed all the enemies!";
									}
									else {
										ws.achievementMessage = "Achievement Unlocked: You killed " + to_string(ws.killed) + " enemies!";
									}
								}
								ws.showAchievement = true;
							}
							playSound(ws, "died.wav");
							return true;
						}
					}
				}
			}
			if (SDL_HasIntersectionF(&enemy.enemyRect, &ws.player)) {
				if (enemy.enemyVelocity.y > 0) {
					enemy.enemyRect.y = ws.player.y - enemy.enemyRect.h;
					enemy.enemyVelocity.y = 0;
				}
				else {
					enemy.enemyRect.y = ws.player.y + ws.player.h;
					enemy.enemyVelocity.y = 0;
				}
			}
			enemy.enemyRect.x += enemy.enemyVelocity.x;
			for (const auto& obstacle : obstacles) {
				SDL_FRect obstacleRect = obstacle.rect;
				if (SDL_HasIntersectionF(&enemy.enemyRect, &obstacleRect)) {
					if (obstacle.obstacleValue != SPIKEBALL && obstacle.obstacleValue != USPEAR_TIP && obstacle.obstacleValue != RSPEAR_TIP && obstacle.obstacleValue != DSPEAR_TIP && obstacle.obstacleValue != LSPEAR_TIP && obstacle.obstacleValue != MONKAS && obstacle.obstacleValue != EMPTYREPLACEMENT) {
						if (enemy.enemyVelocity.x > 0) {
							enemy.enemyRect.x = obstacleRect.x - enemy.enemyRect.w;
							enemy.enemyVelocity.x = 0;
						}
						else {
							if (enemy.enemyVelocity.x < 0) {
								enemy.enemyRect.x = obstacleRect.x + obstacleRect.w;
								enemy.enemyVelocity.x = 0;
							}
						}
					}
					else {
						if (obstacle.obstacleValue != MONKAS && obstacle.obstacleValue != EMPTYREPLACEMENT) {
							if (o) {
								o->obstacleValue = 60;
								o->enemyId = 0;
							}
							if (enemy.enemyID == ws.hookEnemy) {
								ws.hookEnemy = 0;
								ws.maxHookDuration = 50;
							}
							ws.map[enemy.initialPosition.y][enemy.initialPosition.x] = 60;

							if (ws.killed == 1 || ws.killed == 5 || ws.killed == 10 || ws.killed == 17) {
								if (ws.killed == 1)
									ws.achievementMessage = "Achievement Unlocked: You killed your First enemy!";
								else {
									if (ws.killed == 17) {
										ws.achievementMessage = "Achievement Unlocked: You killed all the enemies!";
									}
									else {
										ws.achievementMessage = "Achievement Unlocked: You killed " + to_string(ws.killed) + " enemies!";
									}
								}
								ws.showAchievement = true;
							}
							playSound(ws, "died.wav");
							return true;
						}
					}
				}
			}
			if (SDL_HasIntersectionF(&enemy.enemyRect, &ws.player)) {
				if (enemy.enemyVelocity.x > 0) {
					enemy.enemyRect.x = ws.player.x - enemy.enemyRect.w;
					enemy.enemyVelocity.x = 0;
				}
				else {
					enemy.enemyRect.x = ws.player.x + ws.player.w;
					enemy.enemyVelocity.x = 0;
				}
			}
			if (o) {
				o->rect = enemy.enemyRect;
			}
			return false;
			}),
		ws.enemies.end()
	);
}

void checkPlayerCollision(WorldState& ws, vector<SDL_FRect_P>& obstacles) {

	ws.player.y += ws.playerVelocity.y;
	ws.hurtbox.y = ws.player.y + 4;

	if (ws.playerVelocity.y != 0 && ws.ground == true) {
		ws.jumps = 1;
		ws.ground = false;
	}

	for (const auto& obstacle : obstacles) {
		SDL_FRect obstacleRect = obstacle.rect;
		if (SDL_HasIntersectionF(&ws.player, &obstacleRect)) {
			if (obstacle.obstacleValue != SPIKEBALL && obstacle.obstacleValue != USPEAR_TIP && obstacle.obstacleValue != RSPEAR_TIP && obstacle.obstacleValue != DSPEAR_TIP && obstacle.obstacleValue != LSPEAR_TIP && obstacle.obstacleValue != EMPTYREPLACEMENT) {
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
				if (obstacle.obstacleValue >= 43 && obstacle.obstacleValue != 60 && obstacle.obstacleValue != 46) {
					if (obstacle.obstacleValue == FINISH_FLAG || obstacle.obstacleValue == FINISH_ROD) {
						ws.stage++;
						string basePath = SDL_GetBasePath();
						basePath = basePath + "../../";
						string file = string(basePath) + "/levels/map" + to_string(ws.stage) + ".txt";
						ws.map.clear();
						getMap(ws, file);
						loadLevelAndDraw(ws, obstacles);
					}
					resetPlayer(ws);
					break;
				}
			}
		}
		if (SDL_HasIntersectionF(&ws.hurtbox, &obstacleRect)) {
			if (obstacle.obstacleValue == SPIKEBALL || obstacle.obstacleValue == USPEAR_TIP || obstacle.obstacleValue == RSPEAR_TIP || obstacle.obstacleValue == DSPEAR_TIP || obstacle.obstacleValue == LSPEAR_TIP) {
				if (!ws.isDead) {
					ws.deaths++;
					playSound(ws, "died.wav");
					if (ws.deaths == 10 || ws.deaths == 100) {
						ws.achievementMessage = "Achievement Unlocked: You died " + to_string(ws.deaths) + " times!";
						ws.showAchievement = true;
					}
					ws.isDead = true;
				}
			}
		}
	}
	ws.player.x += ws.playerVelocity.x;
	ws.hurtbox.x = ws.player.x + 4;

	for (const auto& obstacle : obstacles) {
		SDL_FRect obstacleRect = obstacle.rect;
		if (SDL_HasIntersectionF(&ws.player, &obstacleRect)) {
			if (obstacle.obstacleValue != SPIKEBALL && obstacle.obstacleValue != USPEAR_TIP && obstacle.obstacleValue != RSPEAR_TIP && obstacle.obstacleValue != DSPEAR_TIP && obstacle.obstacleValue != LSPEAR_TIP && obstacle.obstacleValue != EMPTYREPLACEMENT) {
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
				if (obstacle.obstacleValue >= 43 && obstacle.obstacleValue != 60 && obstacle.obstacleValue != 46) {
					if (obstacle.obstacleValue == FINISH_FLAG || obstacle.obstacleValue == FINISH_ROD) {
						ws.stage++;
						string basePath = SDL_GetBasePath();
						basePath = basePath + "../../";
						string file = string(basePath) + "/levels/map" + to_string(ws.stage) + ".txt";
						ws.map.clear();
						getMap(ws, file);
						loadLevelAndDraw(ws, obstacles);
					}
					resetPlayer(ws);
					break;
				}
			}
		}
		if (SDL_HasIntersectionF(&ws.hurtbox, &obstacleRect)) {
			if (obstacle.obstacleValue == SPIKEBALL || obstacle.obstacleValue == USPEAR_TIP || obstacle.obstacleValue == RSPEAR_TIP || obstacle.obstacleValue == DSPEAR_TIP || obstacle.obstacleValue == LSPEAR_TIP) {
				if (!ws.isDead) {
					ws.deaths++;
					playSound(ws, "died.wav");
					if (ws.deaths == 10 || ws.deaths == 100) {
						ws.achievementMessage = "Achievement Unlocked: You died " + to_string(ws.deaths) + " times!";
						ws.showAchievement = true;
					}
					ws.isDead = true;
				}
			}
		}
	}
}

void mutateWorldState(WorldState& ws, vector<SDL_FRect_P>& obstacles, double deltaT) {
	vector2 hookPull = { 0,0 };
	vector2 speedVector = { 0,0 };
	velocity v;
	for (int i = 0; i <= ws.HOOKFLYINGSPEED; i += 7) {
		if (ws.hookFlying && !ws.hookConnected && !ws.hookNoObstacleFound) {
			vector2 direction = calculateDirection(ws.player, ws.hookGoal);
			ws.hookPosition.x = ws.player.x + ws.player.w / 2 + (direction.x * i * ws.amountOfHookTicks);// * deltaT;
			ws.hookPosition.y = ws.player.y + ws.player.h / 2 + (direction.y * i * ws.amountOfHookTicks);// * deltaT;
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
							playSound(ws, "leadhook.wav");
							break;
						}
						if (obstacle.obstacleValue == 46) {
							ws.hookEnemy = obstacle.enemyId;
							ws.hookGoal = ws.hookPosition;
							ws.limitedHook = true;
							playSound(ws, "hook.wav");
							ws.hookNoObstacleFound = true;
							ws.hookFlying = false;
							break;
						}
						if (obstacle.obstacleValue >= 28) {
							continue;
						}
						if (obstacle.obstacleValue == 20) {
							ws.limitedHook = true;
						}
						playSound(ws, "hook.wav");
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
		else {
			break;
		}
	}
	if (ws.hookFlying) {
		ws.amountOfHookTicks++;
	}

	if (ws.hookConnected) {
		hookPull = calculateDirection(ws.player, ws.hookGoal);
		if (hookPull.y < 0) {
			//stronger pull upwards
			speedVector.y += hookPull.y * 1.27;
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
	if (ws.hookEnemy > 0) {
		Enemy* e = getEnemyByID(ws.enemies, ws.hookEnemy);
		if (e) {
			vector2 enemyPull = calculateDirection(e->enemyRect, { ws.player.x + ws.player.w / 2, ws.player.y + ws.player.h / 2 });
			if (enemyPull.y < 0) {
				//stronger pull upwards
				e->enemyVelocity.y += enemyPull.y * 1.27;
			}
			else
				e->enemyVelocity.y += enemyPull.y * 0.85;
			e->enemyVelocity.x += enemyPull.x * 1.2;
			//slower pull for enemies
			e->enemyVelocity.x *= ws.HOOKSTRENGTH / 1.7;
		}
	}
	if (ws.limitedHook) {
		ws.maxHookDuration--;
		if (ws.maxHookDuration == 0) {
			ws.maxHookDuration = 50;
			ws.hookConnected = false;
			ws.limitedHook = false;
			ws.hookEnemy = 0;
			ws.hookNoObstacleFound = true;
		}
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
				ws.playerVelocity.x *= v.AIRSLIDING;

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
	
	checkPlayerCollision(ws, obstacles);
	checkEnemyCollision(ws, obstacles);
	if (ws.isDead){
		ws.playerVelocity = { 0, 0 };
		if (ws.alpha > 0) {
			ws.alpha -= 6;
		}

		else {
			SDL_Delay(20);
			ws.isDead = false;
			ws.respawn = true;
		}
	}
	
	if (ws.respawn) {
		resetPlayer(ws);
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


void setText(WorldState& ws, const string& text, float positionX, float positionY, float width, float height, SDL_Color color, bool addCameraOffset) {
	SDL_Surface* surfaceMessage = TTF_RenderText_Solid(ws.font, text.c_str(), color);
	SDL_Texture* Message = SDL_CreateTextureFromSurface(ws.renderer, surfaceMessage);
	SDL_FRect Message_rect;
	if (addCameraOffset) {
		Message_rect = { positionX - camera.x, positionY - camera.y, width, height };
	}
	else {
		Message_rect = { positionX, positionY, width, height };
	}
	SDL_RenderCopyF(ws.renderer, Message, NULL, &Message_rect);
	SDL_FreeSurface(surfaceMessage);
	SDL_DestroyTexture(Message);
}


void renderGraphics(WorldState& ws, const vector<SDL_FRect_P>& obstacles) {
	SDL_Color Black = { 0, 0, 0, 0 };
	SDL_SetRenderDrawColor(ws.renderer, 89, 181, 226, 0);
	SDL_RenderClear(ws.renderer);
	if (ws.hookFlying || ws.hookConnected || !ws.hookNoObstacleFound || ws.hookEnemy > 0) {
		if (ws.hookEnemy > 0) {
			Enemy* e = getEnemyByID(ws.enemies, ws.hookEnemy);
			if (e) {
				ws.hookGoal = { e->enemyRect.x + e->enemyRect.w / 2, e->enemyRect.y + e->enemyRect.h / 2 };
				ws.hookPosition = { e->enemyRect.x + e->enemyRect.w / 2, e->enemyRect.y + e->enemyRect.h / 2 };
			}
		}

		float hookLength = sqrt(pow((ws.hookPosition.x - TILESIZE / 2) - ws.player.x + ws.player.w / 2, 2) + pow((ws.hookPosition.y - TILESIZE / 2) - ws.player.y + ws.player.h / 2, 2));
		float angle = atan2((ws.hookGoal.y - TILESIZE / 2) - ws.player.y, (ws.hookGoal.x - TILESIZE / 2) - ws.player.x) * 180 / M_PI;
		for (float i = hookLength; i > 0; i -= TILESIZE / 2) {
			if (i == hookLength) {
				SDL_FRect hookRect = { ws.player.x + ws.player.w / 4 + ((ws.hookPosition.x - TILESIZE / 2) - ws.player.x) * i / hookLength - camera.x, ws.player.y + ws.player.h / 4 + ((ws.hookPosition.y - TILESIZE / 2) - ws.player.y) * i / hookLength - camera.y, TILESIZE / 2, TILESIZE / 2 };
				SDL_RenderCopyExF(ws.renderer, ws.hookTexture, &ws.spriteSheet[0][0], &hookRect, angle + 90, NULL, SDL_FLIP_NONE);
			}
			else {
				SDL_FRect linkRect = { ws.player.x + ws.player.w / 4 + ((ws.hookPosition.x - TILESIZE / 2) - ws.player.x) * i / hookLength - camera.x, ws.player.y + ws.player.h / 4 + ((ws.hookPosition.y - TILESIZE / 2) - ws.player.y) * i / hookLength - camera.y, TILESIZE / 2, TILESIZE / 2 };
				SDL_RenderCopyExF(ws.renderer, ws.linkTexture, &ws.spriteSheet[0][0], &linkRect, angle + 90, NULL, SDL_FLIP_NONE);
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
		setText(ws, "Press 'A' or 'D' to Walk", 200, 1150, 300, 50, Black, true);
		setText(ws, "Press 'W' or 'Space' to Jump", 700, 1150, 300, 50, Black, true);
		setText(ws, "Avoid These", 700, 1450, 300, 50, Black, true);
		setText(ws, "Press 'w' or 'Space' while in Air", 1475, 1150, 300, 50, Black, true);
		setText(ws, "Rightclick to Hook", 1200, 850, 300, 50, Black, true);
		setText(ws, "You cant Hook Iron Objects", 875, 600, 300, 50, Black, true);
		setText(ws, "You can Hook Through here", 1475, 600, 300, 50, Black, true);
		setText(ws, "The Goal is to reach the Flag!", 900, 50, 300, 50, Black, true);
		setText(ws, "You can grab and kill those!", 200, 50, 300, 50, Black, true);
	}
	setText(ws, "Deaths: " + to_string(ws.deaths), 20, 40, 200, 50, Black, false);
	if (ws.showAchievement) {
		if (ws.messageTimer == 0) {
			ws.showAchievement = false;
			ws.messageTimer = 250;
		}
		setText(ws, ws.achievementMessage, 20, 800, 500, 50, Black, false);
		ws.messageTimer--;
	}
	if (ws.stage == 2) {
		setText(ws, "Move and quickly grab above you", 200, 50, 300, 50, Black, true);
		setText(ws, "Can't see? Try pressing 'F'", 900, 400, 300, 50, Black, true);
		setText(ws, "You can hold on to these only temporary", 1100, 1400, 400, 50, Black, true);
		setText(ws, "Enjoy!", 700, 750, 300, 50, Black, true);
	}

	SDL_Rect playerRect = { ws.player.x - camera.x, ws.player.y - camera.y, ws.player.w, ws.player.h };
	SDL_Rect monkey = { 0 * 64, 6 * 64, 64, 64 };
	if (ws.jumps == 2) {
		monkey.x = 3 * 64;
	}

	SDL_SetTextureAlphaMod(ws.playermonke, ws.alpha);
	SDL_RenderCopy(ws.renderer, ws.playermonke, &monkey, &playerRect);
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


	while (isRunning) {
		
		Uint64 start = SDL_GetPerformanceCounter();
		
		readEvents(worldState, obstacles);
		mutateWorldState(worldState, obstacles, deltaT);
		handle_camera(worldState);
		renderGraphics(worldState, obstacles);
	Uint64 end = SDL_GetPerformanceCounter();
	float passedTime = (end / start) / ((float)SDL_GetPerformanceFrequency() * 1000.0f);
	SDL_Delay(floor(16.666f - passedTime));
	}

	SDL_DestroyTexture(worldState.texture);
	SDL_DestroyRenderer(worldState.renderer);
	Mix_CloseAudio();
	IMG_Quit();
	SDL_Quit();
	return 0;
}