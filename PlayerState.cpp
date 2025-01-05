#include "CaptainHook.hpp"

void PlayerState::Init()
{
	Base::Init();

	/*if (!_image)
	{
		_image = IMG_LoadTexture(renderer(), BasePath "asset/graphic/background.png");
		if (!_image)
			print(stderr, "IMG_LoadTexture failed: {}\n", IMG_GetError());
	}*/
}

bool PlayerState::HandleEvent(const SDL_Event& event)
{
	SDL_PumpEvents();
	const Uint8* state = SDL_GetKeyboardState(NULL);
	velocity v;
	if (state[SDL_SCANCODE_LEFT]) {
		player.movement = v.MOVEMENT * -1;
	}
	else {
		if (state[SDL_SCANCODE_RIGHT]) {
			player.movement = v.MOVEMENT;
		}
		else {
			player.movement = 0;
		}
	}
	if (player.speedX < player.movement && player.movement > 0) {
		player.speedX = player.movement;
	}
	if (player.speedX > player.movement && player.movement < 0) {
		player.speedX = player.movement;
	}
	switch (event.type)
	{
		case SDL_KEYDOWN:
			const SDL_Keysym& what_key = event.key.keysym;

			if (what_key.scancode == SDL_SCANCODE_UP && player.jumps < 2)
			{
				player.jumps++;
				player.speedY = v.JUMP * -1;
				player.ground = false;
			}
		break;
		default:
		break;
	}
	if (player.ground == false) {
		player.speedY += v.GRAVITY;
	}
	else {
		player.jumps = 0;
		if (player.speedX > v.GROUND_FRICTION) {
			player.speedX += v.GROUND_FRICTION * -1;
		}
		else {
			if (player.speedX < v.GROUND_FRICTION * -1) {
				player.speedX += v.GROUND_FRICTION;
			}
			else {
				player.speedX = 0;
			}
		}
	}

	/*case SDL_MOUSEBUTTONDOWN:
		//game.SetNextState( 1 );
		break;

	default: break; 
	} */

	return true;
}

bool CheckCollision(PlayerState::PlayerProperties player, const SDL_FRect& b) {
	SDL_Rect a = { player.playerPosition.x, player.playerPosition.y, 50, 50 };
	return (a.x < b.x + b.w &&
		a.x + a.w > b.x &&
		a.y < b.y + b.h &&
		a.y + a.h > b.y);
}

void PlayerState::Update(const uint64_t frame, const uint64_t totalMSec, const float deltaT)
{
	player.speedX += player.movement;
	player.playerPosition.x += player.speedX;
	player.speedX -= player.movement;
	player.playerPosition.y += player.speedY;

	player.ground = false;
	for (const auto& obstacle : obstacles) {
		if (CheckCollision(player, obstacle)) {
			if (player.speedY > 0) {
				player.playerPosition.y = obstacle.y - 50; // Adjust player position to be on top of the obstacle
				player.ground = true;
				player.speedY = 0;
			}
			else {
				player.playerPosition.y = obstacle.y + 50; // Adjust player position to be on top of the obstacle
				player.ground = false;
				player.speedY = 0;
			}
			break;
		}
	}
}
void PlayerState::Render(uint64_t frame, uint64_t totalMSec, float deltaT)
{
	SDL_SetRenderDrawColor(renderer(), 0, 0, 0, 255);
	SDL_RenderClear(renderer());

	SDL_SetRenderDrawColor(renderer(), 255, 0, 0, 255);
	for (const auto& obstacle : obstacles) {
		SDL_Rect obstacleRect = { static_cast<int>(obstacle.x), static_cast<int>(obstacle.y), static_cast<int>(obstacle.w), static_cast<int>(obstacle.h) };
		SDL_RenderFillRect(renderer(), &obstacleRect);
	}

	SDL_SetRenderDrawColor(renderer(), 0, 0, 255, 255);
	SDL_Rect playerRect = { player.playerPosition.x, player.playerPosition.y, 50, 50 };
	SDL_RenderFillRect(renderer(), &playerRect);
	SDL_RenderPresent(renderer());
}