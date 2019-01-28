#ifndef PONG_H_

#define PONG_H_

#include <stdint.h>

#define PADDLE_COMPUTER 0
#define PADDLE_PLAYER   1

#define RAIL_TOP        0
#define RAIL_BOTTOM     1

// Game states
typedef enum {
	STOPPED,
	PAUSED,
	RUNNING
} GameTickerState;

// Store the state of the current game
typedef struct {
	// Keep track of player progress
	double timer;
	uint8_t lives;
	uint16_t score;
	uint8_t level;

	// Keep track of current game state (e.g. playing, paused)
	GameTickerState ticker_state;
	uint64_t resume_timer;
	bool game_current;
	bool debug;

	// Keep a global RNG
	PRngState rng;

	// Ball mechanics
	Vec2d position;
	Vec2d velocity;

	// Paddle mechanics
	double paddle[2];

	// "Great Attractor" mechanics;
	double ga_sgp;
	bool ga_enabled;

	bool ** rail_state;
	bool rail_enabled;
} GameState, *PGameState;

void game_calc_paddle(PVec2d bounds, double * y, double h, uint16_t canv_width, uint16_t canv_height);

void game_create_state(PGameState * p_state);
void game_tick(PGameState state, double delta, uint64_t curr_time);
void game_loop(PGameState state, bool * continue_flag);

void game_pause(PGameState state);
void game_resume(PGameState state, uint64_t * last_time);
void game_stop(PGameState state);
void game_restart(PGameState state);

bool game_is_stopped(PGameState state);
bool game_is_paused(PGameState state);
bool game_is_running(PGameState state);
bool game_is_resuming(PGameState state);

#endif
