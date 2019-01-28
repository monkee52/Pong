#ifndef RNG_H_

#define RNG_H_

#include <stdint.h>

#define ROTL64(x, n) (((x) << (n)) | ((x) >> (64 - (n))))

// RNG
typedef struct {
	uint64_t s[2];
} RngState, *PRngState;

void rng_init(PRngState * p_state);
void rng_seed(PRngState state, uint64_t seed);

uint64_t rng_next(PRngState state);
double rng_dnext(PRngState state);

#endif
