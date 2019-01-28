/*
 * HyperPong version 1.2
 *
 * Student Name: Ayden Hull
 * Student Number: n9749675
 *
 * Notes:
 * - Game logic originally based on frame-rate independent logic
 * - Due to assignment specifications, the ball cannot move more
 *   than one screen unit in each direction every frame
 * - New game logic still frame-rate independent, but capped
 *
 * - Slowed down initial velocity in version 1.1
 * - Up and down arrows actually work now
 *
 * - Fixed paddle height
 * - Fixed countdown duration from 3 seconds to 0.3 seconds between countdowns in line with specification
 */

#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <cab202_graphics.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <curses.h>

#define FRAMES_PER_SECOND 60.0
#define SPEED_MULTIPLIER 1.0
#define RESUME_TIME 0.9

 // Standard gravitational parameter (G * M)
#define GREAT_ATRACTOR_FORCE 1800

#include "rng.h"
#include "utils.h"
#include "graphics_utils.h"
#include "pong.h"

char * header =
"    __  __                      ____                   "
"   / / / /_  ______  ___  _____/ __ \\____  ____  ____ _"
"  / /_/ / / / / __ \\/ _ \\/ ___/ /_/ / __ \\/ __ \\/ __ `/"
" / __  / /_/ / /_/ /  __/ /  / ____/ /_/ / / / / /_/ / "
"/_/ /_/\\__, / .___/\\___/_/  /_/    \\____/_/ /_/\\__, /  "
"      /____/_/                                /____/   ";

uint16_t header_width = 55;
uint16_t header_height = 6;

char * anomaly =
"               .               "
"               .               "
"               :               "
"               !               "
"               |               "
"              _|_              "
"            .` | `.            "
"-  --  --- -+-<#>-+- ---  --  -"
"            '._|_.'            "
"               T               "
"               |               "
"               !               "
"               :               "
"               .               ";

uint16_t anomaly_width = 31;
uint16_t anomaly_height = 14;

void game_create_state(PGameState * p_state) {
	if (*p_state != NULL) {
		free(*p_state);
	}

	*p_state = (PGameState)malloc(sizeof(GameState));

	memset(*p_state, 0, sizeof(GameState));

	// Initialize state; every member is set to zero/false/null automatically
	(*p_state)->lives = 3;
	(*p_state)->level = 1;

	(*p_state)->ticker_state = PAUSED;

	rng_init(&((*p_state)->rng));
	rng_seed((*p_state)->rng, nanotime());
}

// Calculates the upper and lower y-coordinate for a paddle, and updates the paddle position accordingly
void game_calc_paddle(PVec2d bounds, double * y, double h, uint16_t canv_width, uint16_t canv_height) {
	--h;

	// Paddle position is vertically centered
	int16_t tmp_u = (*y) - h / 2.0;
	int16_t tmp_d = tmp_u + h;

	// Upper coordinate
	if (tmp_u < 0) {
		tmp_u = 0;
		tmp_d = h;
	}

	// Lower coordinate
	if (tmp_d > canv_height - 1) {
		tmp_u = canv_height - 1 - h;
		tmp_d = canv_height - 1;
	}

	// Only return bounds if necessary
	if (bounds != NULL) {
		bounds->x = tmp_u;
		bounds->y = tmp_d;
	}

	// Update paddle position
	*y = tmp_u + (h / 2.0);
}

void game_rail_hit(PGameState state, uint8_t rail, int16_t offset, uint16_t rail_width) {
	if (offset > 0) {
		state->rail_state[rail][offset - 1] = true;
	}

	if (offset >= 0 && offset <= rail_width) {
		state->rail_state[rail][offset] = true;
	}

	if (offset < rail_width) {
		state->rail_state[rail][offset + 1] = true;
	}
}

// Frame-rate independent game processing
void game_tick(PGameState state, double delta, uint64_t curr_time) {
	delta *= SPEED_MULTIPLIER;

	uint16_t scr_width = screen_width();
	uint16_t scr_height = screen_height();

	double canv_width = scr_width - 2;
	double canv_height = scr_height - 4;
	double canv_x = 1;
	double canv_y = 3;
	double canv_cx = (double)canv_width / 2.0;
	double canv_cy = (double)canv_height / 2.0;

	double paddle_height = (double)(canv_height - 1) / 2.0;

	if (paddle_height > 7.0) {
		paddle_height = 7.0;
	}

	Vec2d ball_position = vec2d(round(state->position.x), round(state->position.y));

	uint16_t rail_width = (double)canv_width / 2.0;
	uint16_t rail_left = ((double)canv_width - (double)rail_width) / 2.0;
	uint16_t rail_offset = (double)canv_height / 3.0;

	// Calculate paddle coordinates for bounce mechanics
	Vec2d paddle_bounds[2];

	game_calc_paddle(&paddle_bounds[PADDLE_COMPUTER], &state->paddle[PADDLE_COMPUTER], paddle_height, canv_width, canv_height);
	game_calc_paddle(&paddle_bounds[PADDLE_PLAYER], &state->paddle[PADDLE_PLAYER], paddle_height, canv_width, canv_height);

	if (state->resume_timer > 0 && (curr_time - state->resume_timer) > (NANOSECONDS_PER_SECOND * RESUME_TIME)) {
		state->resume_timer = 0;
	}

	if (game_is_paused(state)) {
		// Draw base layout
		draw_rect(0, 0, scr_width, scr_height, '*', BORDER_ALL);

		draw_glyph(0, 1, 0, 0, DS_CENTER_HORIZ, header, header_width, header_height);

		draw_string2(3, 8, 0, 0, DS_NONE, "Student Name: Ayden Hull");
		draw_string2(3, 9, 0, 0, DS_NONE, "Student Number: n9749675");

		draw_string2(3, 11, 0, 0, DS_NONE, "q      = Quit");
		draw_string2(3, 12, 0, 0, DS_NONE, "h or p = Help/pause (this screen)");
		draw_string2(3, 13, 0, 0, DS_NONE, "d      = Debug (info & auto paddle)");

		draw_string2(3, 15, 0, 0, DS_NONE, "w or ^ = Move paddle up");
		draw_string2(3, 16, 0, 0, DS_NONE, "s or v = Move paddle down");
		draw_string2(3, 17, 0, 0, DS_NONE, "l      = Advance level");

		// Flash paused message
		if (curr_time / NANOSECONDS_PER_MILLISECOND / HALF_SECOND_MS % 2 == 0) {
			draw_string2(0, 0, 2, -1, DS_LEFT | DS_BOTTOM, "PAUSED");
		}

		draw_string2(0, 0, -1, -1, DS_RIGHT | DS_BOTTOM, "Press any key to continue...");
	}

	if (game_is_running(state)) {
		// Draw base layout
		draw_rect(0, 0, scr_width, scr_height, '*', BORDER_ALL);
		draw_line(0, 2, scr_width, 2, '*');

		// Create a status string that displays data from the current game state
		// - Evenly spaces data according to screen width

		// Calculate lives string
		char * lives = msprintf("Lives: %d ", state->lives);
		char * score = msprintf("Score: %d ", state->score);
		char * level = msprintf("Level: %d ", state->level);

		// Calculate time string
		uint32_t time_d = state->timer;
		uint32_t time_d_m = time_d / SECONDS_PER_MINUTE;
		uint32_t time_d_s = time_d - (time_d_m * SECONDS_PER_MINUTE);

		char * d_time = msprintf("Time: %d:%02d ", time_d_m, time_d_s);

		// Calculate overall status string
		int space_count = (scr_width - 2.0 - strlen(lives) - strlen(score) - strlen(level) - strlen(d_time)) / 4;
		draw_string2f(1, 1, 0, 0, DS_NONE, " %s%*s%s%*s%s%*s%s", lives, space_count, "", score, space_count, "", level, space_count, "", d_time);

		// Clear unused dynamically allocated memory
		free(lives);
		free(score);
		free(level);
		free(d_time);

		// Initialize if necessary
		if (!state->game_current) {
			state->timer = 0.0;

			state->paddle[PADDLE_COMPUTER] = (double)canv_height / 2.0;
			state->paddle[PADDLE_PLAYER] = (double)canv_height / 2.0;

			state->ga_enabled = false;

			// Rail persists between lives
			if (state->level != 4) {
				state->rail_enabled = false;
			}

			// Create angle between -45 and 45 degrees
			double angle = fremap(rng_dnext(state->rng), 0.0, 1.0, -(double)M_PI / 4.0, (double)M_PI / 4.0);

			state->velocity.x = (double)FRAMES_PER_SECOND / 2.0 * cos(angle);
			state->velocity.y = (double)FRAMES_PER_SECOND / 2.0 * sin(angle);

			// Spawn ball in center
			state->position.x = (double)canv_width / 2.0;
			state->position.y = (double)canv_height / 2.0;

			state->game_current = true;
		}
	}

	if (game_is_running(state) && !game_is_resuming(state)) {
		state->timer += delta;

		if (state->level == 3 && state->timer >= 5.0 && !state->ga_enabled) {
			// Create a great attractor
			state->ga_sgp = GREAT_ATRACTOR_FORCE;
			state->ga_enabled = true;
		}

		// Enable the rail if necessary
		if (state->level == 4 && !state->rail_enabled) {
			state->rail_enabled = true;

			// Free last rail state if necessary
			if (state->rail_state != NULL) {
				if (state->rail_state[RAIL_TOP] != NULL) {
					free(state->rail_state[RAIL_TOP]);

					state->rail_state[RAIL_TOP] = NULL;
				}

				if (state->rail_state[RAIL_BOTTOM] != NULL) {
					free(state->rail_state[RAIL_BOTTOM]);

					state->rail_state[RAIL_BOTTOM] = NULL;
				}

				free(state->rail_state);

				state->rail_state = NULL;
			}

			// Initialise rail state array
			state->rail_state = (bool **)malloc(sizeof(bool *) * 2);

			memset(state->rail_state, 0, sizeof(bool *) * 2);

			// Initialise top rail
			state->rail_state[RAIL_TOP] = (bool *)malloc(sizeof(bool) * rail_width);

			memset(state->rail_state[RAIL_TOP], 0, sizeof(bool) * rail_width);

			// Initialise bottom rail
			state->rail_state[RAIL_BOTTOM] = (bool *)malloc(sizeof(bool) * rail_width);

			memset(state->rail_state[RAIL_BOTTOM], 0, sizeof(bool) * rail_width);
		}

		// Modify velocity for great attractor
		if (state->ga_enabled) {
			double r = sqrt(pow(state->position.x - canv_cx, 2.0) + pow(state->position.y - canv_cy, 2.0));
			double a = state->ga_sgp / pow(r, 2.0);

			double angle = atan2(state->position.y - canv_cy, state->position.x - canv_cx) + M_PI;

			Vec2d acceleration;

			acceleration.x = a * cos(angle);
			acceleration.y = a * sin(angle);

			state->velocity.x += acceleration.x * delta;
			state->velocity.y += acceleration.y * delta;

			// Prevent ball from getting trapped in orbit
			// - Ball hasn't been stuck in orbit before
			// - However... it can do a few arcs around it before it reaches a paddle
			// - This should prevent it
			double velocity_angle = atan2(state->velocity.y, state->velocity.x) + M_PI;
			double velocity_radius = sqrt(pow(state->velocity.x, 2.0) + pow(state->velocity.y, 2.0));
			double escape_velocity = sqrt(2.0 * state->ga_sgp / r);

			if (velocity_radius < escape_velocity) {
				state->velocity.x = escape_velocity * cos(velocity_angle);
				state->velocity.y = escape_velocity * sin(velocity_angle);
			}
		}

		Vec2d last_position = state->position;

		state->position.x += state->velocity.x * delta;
		state->position.y += state->velocity.y * delta;

		// Move at most one screen unit
		if (abs(state->position.x - last_position.x) > 1) {
			state->position.x = last_position.x + fsign(state->position.x - last_position.x) * 1.0;
		}

		if (abs(state->position.y - last_position.y) > 1) {
			state->position.y = last_position.y + fsign(state->position.y - last_position.y) * 1.0;
		}

		ball_position.x = round(state->position.x);
		ball_position.y = round(state->position.y);

		// Lock computer paddle coordinates to ball
		if (state->level != 1) {
			state->paddle[PADDLE_COMPUTER] = state->position.y;

			game_calc_paddle(&paddle_bounds[PADDLE_COMPUTER], &state->paddle[PADDLE_COMPUTER], paddle_height, canv_width, canv_height);
		}

		// Update player paddle (DEBUG)
		if (state->debug) {
			state->paddle[PADDLE_PLAYER] = state->position.y;

			game_calc_paddle(&paddle_bounds[PADDLE_PLAYER], &state->paddle[PADDLE_PLAYER], paddle_height, canv_width, canv_height);
		}

		// Bounce horizontally
		// Right side
		if (ball_position.x >= canv_width - 4.0 && state->velocity.x > 0.0) {
			// Exact position the ball hits at
			Vec2d intercept = vec2d_intercept_y_at_x(ball_position, canv_width - 3.0);

			// Check if ball hit paddle
			if (intercept.y >= paddle_bounds[PADDLE_PLAYER].x && intercept.y <= paddle_bounds[PADDLE_PLAYER].y) {
				++state->score;

				// Check if ball hit ends of paddle, but there's space between the border
				if ((intercept.y < (paddle_bounds[PADDLE_PLAYER].x + 1.0) || intercept.y >(paddle_bounds[PADDLE_PLAYER].y - 1.0)) && paddle_bounds[PADDLE_PLAYER].x != 0 && paddle_bounds[PADDLE_PLAYER].y != canv_height - 1) {
					state->velocity.y *= -1.0;
				} else {
					state->velocity.x *= -1.0;

					state->position.x = ball_position.x;
					state->position.y = ball_position.y;
				}
			} else if (state->position.x >= canv_width - 1.0) {
				state->game_current = false;

				if (state->lives > 0) {
					--state->lives;

					game_pause(state);
				} else {
					game_stop(state);
				}
			}
		}

		// Left side
		// Only enable if level == 1 (computer not playing)
		if (state->velocity.x < 0.0) {
			if (state->level != 1 && ball_position.x <= 3.0) {
				state->velocity.x *= -1.0;

				state->position.x = ball_position.x;
				state->position.y = ball_position.y;
			} else if (ball_position.x <= 0.0) {
				state->velocity.x *= -1.0;
				state->position.x = 0.0;
			}
		}

		// Bounce vertically
		// Bottom side
		if (ball_position.y >= canv_height - 1.0 && state->velocity.y > 0.0) {
			state->velocity.y *= -1.0;

			state->position.y = canv_height - 1.0;
		}

		// Top side
		if (ball_position.y <= 0.0 && state->velocity.y < 0.0) {
			state->velocity.y *= -1.0;

			state->position.y = 0.0;
		}

		// Bounce on rails
		if (state->rail_enabled && ball_position.x >= rail_left && ball_position.x <= rail_left + rail_width) {
			int16_t rail_state_offset = ball_position.x - rail_left;

			// Top rail above
			if (ball_position.y >= rail_offset && ball_position.y < rail_offset + 1 && state->velocity.y > 0 && !state->rail_state[RAIL_TOP][rail_state_offset]) {
				state->velocity.y *= -1.0;

				game_rail_hit(state, RAIL_TOP, rail_state_offset, rail_width);
			}

			// Top rail below
			if (ball_position.y > rail_offset - 1 && ball_position.y <= rail_offset && state->velocity.y < 0 && !state->rail_state[RAIL_TOP][rail_state_offset]) {
				state->velocity.y *= -1.0;

				game_rail_hit(state, RAIL_TOP, rail_state_offset, rail_width);
			}

			// Bottom rail above
			uint16_t rail_bottom_offset = canv_height - rail_offset;

			if (ball_position.y > rail_bottom_offset - 1 && ball_position.y <= rail_bottom_offset && state->velocity.y > 0 && !state->rail_state[RAIL_BOTTOM][rail_state_offset]) {
				state->velocity.y *= -1.0;

				game_rail_hit(state, RAIL_BOTTOM, rail_state_offset, rail_width);
			}

			// Bottom rail below
			if (ball_position.y > rail_bottom_offset && ball_position.y <= rail_bottom_offset + 1 && state->velocity.y < 0 && !state->rail_state[RAIL_BOTTOM][rail_state_offset]) {
				state->velocity.y *= -1.0;

				game_rail_hit(state, RAIL_BOTTOM, rail_state_offset, rail_width);
			}
		}
	}

	if (game_is_running(state)) {
		// Draw anomaly
		if (state->ga_enabled) {
			draw_glyph(canv_cx + canv_x, canv_cy + canv_y, -(anomaly_width / 2.0), -(anomaly_height / 2.0), DS_NONE, anomaly, anomaly_width, anomaly_height);
		}

		// Draw computer paddle
		if (state->level != 1) {
			draw_line(2 + canv_x, paddle_bounds[PADDLE_COMPUTER].x + canv_y, 2 + canv_x, paddle_bounds[PADDLE_COMPUTER].y + canv_y, '|');
		}

		// Draw user paddle
		draw_line(canv_width - 1 - 2 + canv_x, paddle_bounds[PADDLE_PLAYER].x + canv_y, canv_width - 1 - 2 + canv_x, paddle_bounds[PADDLE_PLAYER].y + canv_y, '|');

		// Draw rails
		if (state->rail_enabled) {
			for (uint16_t x = 0; x < rail_width; x++) {
				if (!state->rail_state[RAIL_TOP][x]) {
					draw_char(rail_left + x + canv_x, rail_offset + canv_y, '=');
				}

				if (!state->rail_state[RAIL_BOTTOM][x]) {
					draw_char(rail_left + x + canv_x, canv_height - rail_offset + canv_y, '=');
				}
			}
		}

		// Draw ball
		draw_char(ball_position.x + canv_x, ball_position.y + canv_y, 'O');
	}

	if (game_is_resuming(state)) {
		uint64_t curr_time = nanotime();
		double remaining_time = RESUME_TIME - ((double)curr_time - (double)state->resume_timer) / (double)NANOSECONDS_PER_SECOND;

		char format_text[] = "Starting in %.1f...";

		draw_string2f(0, 0, 0, 0, DS_CENTER, format_text, remaining_time);
	}

	if (state->debug) {
		double fps = 1L / delta;

		draw_string2f(0, 0, 0, 0, DS_CENTER_HORIZ | DS_BOTTOM, " s: %02.2f, %02.2f * v: %02.2f, %02.2f * FPS: %02.0f ", state->position.x, state->position.y, state->velocity.x, state->velocity.y, fps);
	}
}

void game_pause(PGameState state) {
	state->ticker_state = PAUSED;
}

void game_resume(PGameState state, uint64_t * last_time) {
	state->ticker_state = RUNNING;
	state->resume_timer = nanotime();

	// Update last time to prevent frame jump
	if (last_time != NULL) {
		*last_time = state->resume_timer;
	}
}

void game_stop(PGameState state) {
	state->ticker_state = STOPPED;
}

void game_restart(PGameState state) {
	game_pause(state);

	state->game_current = false;
	state->lives = 3;
	state->level = 1;
	state->score = 0;
}

bool game_is_stopped(PGameState state) {
	return state->ticker_state == STOPPED;
}

bool game_is_paused(PGameState state) {
	return state->ticker_state == PAUSED;
}

bool game_is_running(PGameState state) {
	return state->ticker_state == RUNNING;
}

bool game_is_resuming(PGameState state) {
	return game_is_running(state) && state->resume_timer > 0;
}

// Main game loop
void game_loop(PGameState state, bool * continue_flag) {
	uint64_t last_time = nanotime();

	while (!game_is_stopped(state)) {
		uint64_t curr_time = nanotime();
		uint64_t time_diff = curr_time - last_time;

		last_time = curr_time;

		clear_screen();

		double delta = (double)time_diff / (double)NANOSECONDS_PER_SECOND;

		game_tick(state, delta, curr_time);

		show_screen();

		// Control handling code
		int c = get_char();

		if (c == 'd') {
			state->debug = !state->debug;
		} else if (c == 'q') {
			game_stop(state);

			*continue_flag = false;
		} else if (c != ERR && game_is_paused(state)) {
			game_resume(state, &last_time);
		} else if (game_is_running(state)) {
			if (c == 'h' || c == 'p') {
				game_pause(state);
			} else if (c == 'l') {
				++state->level;

				if (state->level > 4.0) {
					state->level = 1.0;
				}

				if (state->level < 1.0) {
					state->level = 4.0;
				}

				state->game_current = false;

				game_pause(state);
			} else if ((c == 'w' || c == KEY_UP) && !game_is_resuming(state)) {
				--state->paddle[PADDLE_PLAYER];
			} else if ((c == 's' || c == KEY_DOWN) && !game_is_resuming(state)) {
				++state->paddle[PADDLE_PLAYER];
			}
		}

		int64_t sl = NANOSECONDS_PER_SECOND / FRAMES_PER_SECOND;

		if (sl > 0L) {
			struct timespec req;

			req.tv_sec = sl / NANOSECONDS_PER_SECOND;
			req.tv_nsec = sl % NANOSECONDS_PER_SECOND;

			nanosleep(&req, NULL);
		}
	}
}

int main(int argc, char * argv[]) {
	PGameState state = NULL;

	bool keep_playing = true;

	while (keep_playing) {
		game_create_state(&state);

		setup_screen();

		game_loop(state, &keep_playing);

		cleanup_screen();

		// Cleanup
		if (state != NULL) {
			// Random number generator
			if (state->rng != NULL) {
				free(state->rng);

				state->rng = NULL;
			}

			// Rails
			if (state->rail_state != NULL) {
				if (state->rail_state[RAIL_TOP] != NULL) {
					free(state->rail_state[RAIL_TOP]);

					state->rail_state[RAIL_TOP] = NULL;
				}

				if (state->rail_state[RAIL_BOTTOM] != NULL) {
					free(state->rail_state[RAIL_BOTTOM]);

					state->rail_state[RAIL_BOTTOM] = NULL;
				}

				free(state->rail_state);

				state->rail_state = NULL;
			}

			// State
			free(state);

			state = NULL;
		}

		// Prompt user to play again
		bool valid = false;

		while (!valid && keep_playing) {
			printf("Game Over!\r\n");
			printf("Play again (Y/N)? ");
			fflush(stdout);

			char x = wait_char();

			if (x != ERR && toupper(x) == 'Y') {
				valid = true;
				keep_playing = true;
			} else if (x != ERR && toupper(x) == 'N') {
				valid = true;
				keep_playing = false;
			}
		}
	}

	printf("Goodbye!\r\n");

	return 0;
}
