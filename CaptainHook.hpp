#include "Game.hpp"

class MyGame;
class MyGameState;

class PlayerState;
class IntroState;
class PlasmaState;
class MapState;
class CameraState;
class ShooterState;

// MyGameState-Index
// Why not in the classes themselves? For reusability!
enum class MyGS : uint8_t
{
	Player,
};


template <typename TGameState = IGameState, typename TState = uint8_t>
class Game : public IGame
{
	using Base = IGame;

protected:
	/// Members / Fields
	vector<Owned<TGameState>> _allStates;

public:
	/// Ctors & Dtor
	using Base::Base;

	/// Getters & Setters: non-virtual first, followed by (pure) virtual/override
	[[nodiscard]] TGameState& currentState()                  override { return *_allStates[currentStateIndex()]; }
	[[nodiscard]] const     TGameState& currentState()   const          override { return *_allStates[currentStateIndex()]; }
	[[nodiscard]] constexpr usize        numberOfStates() const noexcept override { return _allStates.size(); }

	/// Methods: non-virtual first, followed by (pure) virtual/override
	template <typename... TS, typename TC>
		requires (sizeof...(TS) == 0) // Pre-C++20: typename std::enable_if_t<sizeof...(TS) == 0>
	constexpr void AddStates(TC& game) {}

	template <typename T, typename... TS, typename TC>
	constexpr void AddStates(TC& game)
	{
		//_allStates.emplace_back( make_unique<T>( (typename T::Game*)*this ) );
		//AddStates<TS...>( static_cast<typename T::Game*>(*this) );
		_allStates.emplace_back(make_unique<T>(game));
		AddStates<TS...>(game);
	}

	void ReplaceState(TState state) { Base::ReplaceState((u8)state); }
	void PushState(TState state) { Base::PushState((u8)state); }
	//void PopState()                   { Base::PopState(); }
};

// abstract
template <typename TGame = Game<IGameState, uint8_t>>
class GameState : public IGameState
{
public:
	using Base = IGameState;

protected:
	/// Members / Fields
	TGame& _game;

public:
	/// Ctors & Dtor
	explicit GameState(TGame& game) noexcept : _game(game) {}
	~GameState()               noexcept override = default;

	/// Getters & Setters: non-virtual first, followed by (pure) virtual/override
	[[nodiscard]] Window* window()   const noexcept { return _game.window(); }
	[[nodiscard]] Renderer* renderer() const noexcept { return _game.renderer(); }
};

// abstract
class MyGameState : public GameState<MyGame>
{
	using Base = GameState;

public:
	// ctor
	using Base::Base;
};

class MyGame final : public Game<MyGameState, MyGS>
{
	using Base = Game;

public:
	MyGame();

	bool HandleEvent(const SDL_Event& event) override;
};

class PlayerState : public MyGameState
{
	using Base = MyGameState;

	protected:
		struct velocity {
			double MAXSPEED = 1000;
			double MOVEMENT = 5;
			double AIR_FRICTION = 0.95;
			double GROUND_FRICTION = 0.5;
			double JUMP = 20;
			double GRAVITY = 1;
		};

	public:
		struct PlayerProperties {
			SDL_FPoint playerPosition = { 100, 100 };
			double speedX = 0;
			double speedY = 0;
			bool ground = false;
			int jumps = 0;
			double movement = 0;
		};
		PlayerProperties player;
		vector<SDL_FRect> obstacles = {
			{0, 550, 850, 50}, // Example ground object
			{200, 400, 100, 50}, // Example obstacle
			{400, 300, 150, 50}  // Another example obstacle
		};
		// ctor
		using Base::Base;

		void Init() override;
		void Destroy() override;

		bool Input() override;
		bool HandleEvent(const SDL_Event& event) override;
		void Update(uint64_t frame, uint64_t totalMSec, float deltaT) override;
		void Render(uint64_t frame, uint64_t totalMSec, float deltaT) override;
		void RenderLayer(const SDL_Point windowSize, const SDL_FPoint camPos, const int index) const;
};