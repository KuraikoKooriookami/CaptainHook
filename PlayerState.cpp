#include "Commons.cpp"


struct PlayerState {
	SDL_FRect player = { 2 * TILESIZE, 2 * TILESIZE, TILESIZE, TILESIZE };
	vector2 playerVelocity = { 0, 0 };
	bool isOnGround = false;
	int jumps = 0;
	double movement = 0;
};

class Player
{
    public:
        static Player& getInstance()
        {
            static Player instance; 
            return instance;
        }
        Player(Player const&) = delete;
        void operator=(Player const&) = delete;
		static void updatePlayerVelocity(vector2 appliedForce) {
			playerState.playerVelocity += appliedForce;
			playerState.playerVelocity.x = SDL_clamp(playerState.playerVelocity.x, -v.MAXSPEED, v.MAXSPEED);
			playerState.playerVelocity.y = SDL_clamp(playerState.playerVelocity.y, -v.MAXSPEED, v.MAXSPEED);
		}
		static int getPlayerMovement() {
			return playerState.movement;
		}
		static bool GetIsOnGround() {
			return playerState.isOnGround;
		}
		static SDL_FRect getPlayerCharacter() {
			return playerState.player;
		}
		static void readEvents() {
			const Uint8* state = SDL_GetKeyboardState(NULL);
			SDL_Event event;

			if (state[SDL_SCANCODE_A]) {
				playerState.movement = v.MOVEMENT * -1;
			}
			else {
				if (state[SDL_SCANCODE_D]) {
					playerState.movement = v.MOVEMENT;
				}
				else {
					playerState.movement = 0;
				}
			}
			/*if (ws.playerVelocity.x < ws.movement && ws.movement > 0) {
				ws.playerVelocity.x = ws.movement;
			}
			if (ws.playerVelocity.x > ws.movement && ws.movement < 0) {
				ws.playerVelocity.x = ws.movement;
			}*/

			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_W && playerState.jumps < 2) {
					playerState.jumps++;
					playerState.playerVelocity.y = v.JUMP * -1;
					playerState.isOnGround = false;
				}
				if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT)
				{
					/*ws.hookGoal.x = event.button.x + camera.x;
					ws.hookGoal.y = event.button.y + camera.y;
					ws.hookNoObstacleFound = false;
					ws.hookFlying = true;
					ws.hookConnected = false;
					ws.amountOfHookTicks = 1;*/
				}
				if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT) {
					/*ws.hookFlying = false;
					ws.hookNoObstacleFound = true;
					ws.hookConnected = false;
					ws.amountOfHookTicks = 1;*/
				}
			}
		}
		static vector2 mutatePlayerState(vector2 appliedForce, bool hookConnected) {
			velocity v;
			if (!playerState.isOnGround && !hookConnected) {
				appliedForce.y += v.GRAVITY;
				appliedForce.x *= v.AIR_FRICTION;
			}
			else if (playerState.isOnGround) {
				playerState.jumps = 0;
				playerState.playerVelocity.x *= v.GROUND_FRICTION;
				if (abs(playerState.playerVelocity.x) < 1)
					playerState.playerVelocity.x = 0;
			}
			return appliedForce;
		}
		static void checkPlayerCollision(const std::vector<SDL_FRect_P>& obstacles) {
			playerState.isOnGround = false;
			/*if (ws.playerVelocity.x < 1 && ws.playerVelocity.x > -1) {
				ws.playerVelocity.x = 0;
			}*/
			playerState.player.y += playerState.playerVelocity.y;

			for (const auto& obstacle : obstacles) {
				SDL_FRect obstacleRect = obstacle.rect;
				if (SDL_HasIntersectionF(&playerState.player, &obstacleRect)) {
					if (playerState.playerVelocity.y > 0) {
						playerState.player.y = obstacleRect.y - playerState.player.h;
						playerState.isOnGround = true;
						playerState.playerVelocity.y = 0;
					}
					else {
						playerState.player.y = obstacleRect.y + obstacleRect.h;
						playerState.isOnGround = false;
						playerState.playerVelocity.y = 0;
					}
				}
			}

			playerState.player.x += playerState.playerVelocity.x + playerState.movement;
			//ws.speedX -= ws.movement;
			for (const auto& obstacle : obstacles) {
				SDL_FRect obstacleRect = obstacle.rect;
				if (SDL_HasIntersectionF(&playerState.player, &obstacleRect)) {
					if (playerState.playerVelocity.x+ playerState.movement > 0) {
						playerState.player.x = obstacleRect.x - playerState.player.w;
						playerState.playerVelocity.x = 0;
					}
					else {
						playerState.player.x = obstacleRect.x + obstacleRect.w;
						playerState.playerVelocity.x = 0;
					}
				}
			} 
		}
    private:
		Player() {};
		static PlayerState playerState;
		static velocity v;
};
PlayerState Player::playerState;
velocity Player::v;