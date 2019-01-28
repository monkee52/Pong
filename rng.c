#include <stdlib.h>
#include <string.h>

#include "rng.h"

void rng_init(PRngState * p_state) {
	*p_state = (PRngState)malloc(sizeof(RngState));

	memset(*p_state, 0, sizeof(RngState));
}

void rng_seed(PRngState state, uint64_t seed) {
	uint64_t z = (seed += UINT64_C(0x9E3779B97F4A7C15));

	z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);

	state->s[0] = z ^ (z >> 31);

	z = (seed += UINT64_C(0x9E3779B97F4A7C15));

	z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);

	state->s[1] = z ^ (z >> 31);
}

uint64_t rng_next(PRngState state) {
	uint64_t s0 = state->s[0];
	uint64_t s1 = state->s[1];
	uint64_t sum = s0 + s1;

	s1 ^= s0;

	state->s[0] = ROTL64(s0, 55) ^ s1 ^ (s1 << 14);
	state->s[1] = ROTL64(s1, 36);

	return sum;
}

double rng_dnext(PRngState state) {
	return (double)((long double)rng_next(state) / (long double)0xffffffffffffffff);
}
