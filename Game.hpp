#pragma once
#include <SDL.h>
#include <vector>
#include <cassert>
#include <chrono>
#include "smartptr.hpp"

using namespace std;	
using Clock = chrono::high_resolution_clock;

template<typename T, typename TDel = default_delete<T>> using Owned = AutocastUnique<T, TDel>;
template<typename T>                                         using Shared = AutocastShared<T>;
template<typename T>                                         using WeakShare = AutocastWeakShare<T>;

class IGameState;

template <typename TGame/* = Game<IGameState>*/>
class GameState;

class IGame;

template <typename TGameState/* = IGameState*/, typename TState/* = u8*/>
class Game;

// Usually ImGui is only included during development,
// but defining IMGUI_ALSO_IN_OPTIMIZED or IMGUI_ALSO_IN_FINAL can make it available everywhere
#if defined( IMGUI )
#include <imgui.h>      // https://github.com/ocornut/imgui/blob/bb224c8aa1de1992c6ea3483df56fb04d6d1b5b6/examples/example_sdl2_sdlrenderer/main.cpp
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#define ImGuiOnly( ... )    __VA_ARGS__
#define IfImGui             if constexpr( true )
#else
#define ImGuiOnly( ... )    // nothing
#define IfImGui             if constexpr( false )
#endif

//class GenericGameState;
//class GenericGame;

// abstract, interface
// Enter and Exit can be called from being stacked-as-a child or by replaced-by-a sibling
class IGameState
{
public:
	/// Ctors & Dtor
	IGameState() noexcept = default;
	virtual ~IGameState() noexcept = default;

	IGameState(const IGameState&) = delete;
	IGameState(IGameState&&) = delete;
	IGameState& operator=(const IGameState&) = delete;
	IGameState&& operator=(IGameState&&) = delete;

	/// constexpr Getters, to override for configuration of derived states
	[[nodiscard]] virtual constexpr bool  isFPSLimited()    const noexcept { return true; } // Limit the FPS of this State
	[[nodiscard]] virtual constexpr bool  isFillingScreen() const noexcept { return true; } // Care does not work yet: For stacked states, a state that fills the screen will prevent states behind from rendering
	[[nodiscard]] virtual constexpr bool  isEventBubbling() const noexcept { return false; } // Care does not work yet: For stacked states, does this state propagate unhandled events further down the stack
	[[nodiscard]] virtual constexpr bool  isAlwaysUpdated() const noexcept { return false; } // Care does not work yet: For stacked states, does this state need to be updated, even if it is not on top
	[[nodiscard]] virtual constexpr SDL_Color clearColor()      const noexcept { return SDL_Color{ 0, 0, 0, SDL_ALPHA_OPAQUE }; } // With what color shall the screen be cleared

	/// Methods: non-virtual first, followed by (pure) virtual/override
	virtual void Init() {} // Called before Enter()ing, can be called multiple times
	virtual void Enter(bool isStacking) {} // Call when activating the state, either replacing the top state or stacking on top
	virtual void Exit(bool isStackedUpon) {} // Call when deactivating the state, either being stacked upon or being completely removed from the stack
	virtual void Destroy() {} // Could be called after Exit()ing

	// This is mostly being replaced by HandleEvent, only use for stateful inputs, is already pumped.
	// Do only `return true` in overloads if ALL inputs are handled,
	// Care: `return true` here would skip the whole `while(SDL_PollEvent())` loop
	//  and all calls to Game::HandleEvent and GameState::HandleEvent
	virtual constexpr bool Input() { return false; }

	// Returns a bool to know if the Event was handled, maybe useful in the future if GameStates could be stacked (not possible yet)
	virtual bool HandleEvent(const SDL_Event& event) = 0;
	virtual void Update(uint64_t framesSinceStart, uint64_t msSinceStart, float deltaT) = 0;
	virtual void Render(uint64_t framesSinceStart, uint64_t msSinceStart, float deltaTNeeded) = 0;
	ImGuiOnly(
		virtual void RenderUI(uint64_t framesSinceStart, uint64_t msSinceStart, float deltaTNeeded) {})
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
	[[nodiscard]] SDL_Window* window()   const noexcept { return _game.window(); }
	[[nodiscard]] SDL_Renderer* renderer() const noexcept { return _game.renderer(); }
};

// abstract, pseudo interface (contains fields)
class IGame
{
protected:
	/// Types
	enum class NextStateOp : uint8_t
	{
		None = 0,
		Replace,    // Modifies the GameStateIndex on top of the stack
		Push,       //   Adds a new GameStateIndex on top of the stack
		Pop,        //  Removes the GameStateIndex on top of the stack
	};

	/// Members / Fields
	Owned<SDL_Window>   _window;
	Owned<SDL_Renderer> _renderer;
	uint64_t        _framesSinceStart = 0;
	uint64_t        _msSinceStart = 0;

	vector<uint8_t>    _stateStack;      // Not a std::stack since it's necessary to look inside without de-stacking
	uint8_t              _stateNextVal;
	NextStateOp     _stateNextOp = NextStateOp::None;

	float             _scalingFactor;
	bool            _isRunning = true;

public:
	/// Ctors & Dtor
	// scalingFactor:
	//  -1.0f (or unspecified) for autoscaling,
	//   1.0f for "no" scaling,
	//  integer floating point values for pixel perfect scaling
	explicit IGame(
		const char* windowTitle = "SDL Game",
		const float  scalingFactor = -1.0f,
		const SDL_Point  requestedSize = SDL_Point{ 640, 360 },
		const bool   vSync = true) noexcept;
	virtual ~IGame() noexcept;

	IGame(const IGame&) = delete;
	IGame(IGame&&) = delete;
	IGame& operator=(const IGame&) = delete;
	IGame&& operator=(IGame&&) = delete;

	/// Getters & Setters: non-virtual first, followed by (pure) virtual/override
	[[nodiscard]] constexpr bool       isRunning()         const noexcept { return _isRunning; }
	[[nodiscard]] SDL_Window* window()            const noexcept { return _window.get(); } // even though this is a pointer, it is usually not null
	[[nodiscard]] SDL_Renderer* renderer()          const noexcept { return _renderer.get(); } // even though this is a pointer, it is usually not null
	[[nodiscard]] float        scalingFactor()     const { return _scalingFactor; }
	[[nodiscard]] constexpr bool       isStateChanging()   const noexcept { return _stateNextOp != NextStateOp::None; }
	[[nodiscard]] uint8_t         currentStateIndex() const { assert(!_stateStack.empty()); return _stateStack.back(); }

	[[nodiscard]] virtual constexpr       IGameState& currentState() = 0;
	[[nodiscard]] virtual constexpr const IGameState& currentState()   const = 0;
	[[nodiscard]] virtual constexpr       size_t         numberOfStates() const noexcept = 0;

	[[nodiscard]] virtual constexpr bool isStateIndexValid(uint8_t index) const noexcept
	{
		return (size_t)index < numberOfStates();
	}

	/// Methods: non-virtual first, followed by (pure) virtual/override
	// Care: Currently not possible to Push multiple States in one Frame
	void PushState(uint8_t state);
	// Care: Currently not possible to Pop multiple States in one Frame (might quickly be problematic)
	void PopState();
	void ReplaceState(uint8_t state);
protected:
	//void ChangeState();
private:
	ImGuiOnly(
		void CreateImGui(Renderer* renderer, Window* window);)

public:
	void ChangeState();
	virtual int  Run();
	virtual void Input();
	virtual bool HandleEvent(const SDL_Event& event);
	virtual void Update(float deltaT);
	virtual void Render(float deltaTNeeded);
	ImGuiOnly(
		virtual void RenderUI(f32 deltaTNeeded);)


		/// Performance
		/// TODO: refactor into a game-component class

public:
	// All output msec per frame, see: http://renderingpipeline.com/2013/02/fps-vs-msecframe/
	enum class PerformanceDrawMode : uint8_t
	{
		None,
		Title,      // averaged and only updated every 250ms
		OStream,    // averaged and only updated every 1000ms
	};

	void SetPerfDrawMode(PerformanceDrawMode mode) noexcept { _perfDrawMode = mode; }

protected:
	Clock::time_point           _lastPerfInfoTime = Clock::now();
	Clock::duration            _accumulatedNeeded = Clock::duration::zero();
	uint32_t                 _lastPerfInfoFrame = 0;
	PerformanceDrawMode _perfDrawMode = PerformanceDrawMode::None;

	[[nodiscard]]
	float AverageMSecPerFrame() const;
	void  ResetPerformanceInfo(Clock::time_point current);
	void  OutputPerformanceInfo(Clock::time_point current, Clock::duration needed);
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
	[[nodiscard]] constexpr size_t        numberOfStates() const noexcept override { return _allStates.size(); }

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