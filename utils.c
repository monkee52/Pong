#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>

#include "utils.h"

Rect create_rect(double tl_x, double tl_y, double br_x, double br_y) {
	Rect o;

	o.tl.x = tl_x;
	o.tl.y = tl_y;
	o.br.x = br_x;
	o.br.y = br_y;

	return o;
}

// Returns the current time as microseconds since unix epoch
// - https://stackoverflow.com/questions/11604336/microtime-equivalent-for-c-and-c
uint64_t microtime() {
	struct timeval res;

	gettimeofday(&res, NULL);

	return ((uint64_t)res.tv_sec * MICROSECONDS_PER_SECOND) + res.tv_usec;
}

uint64_t nanotime() {
	struct timespec res;

	clock_gettime(CLOCK_REALTIME, &res);

	return ((uint64_t)res.tv_sec * NANOSECONDS_PER_SECOND) + (uint64_t)res.tv_nsec;
}

// Returns the sign of a double
double fsign(double x) {
	if (x < 0.0) {
		return -1.0;
	}

	if (x > 0.0) {
		return 1.0;
	}

	return 0.0;
}

// Returns a double clamped between two values
double fclamp(double x, double low, double high) {
	return fmin(high, fmax(low, x));
}

// Convert a double from one range to another
double fremap(double s, double x1, double y1, double x2, double y2) {
	return x2 + (s - x1) * (y2 - x2) / (y1 - x1);
}

char * vmsprintf(char * format, va_list va1) {
	va_list va2;

	va_copy(va2, va1);

	size_t length = vsnprintf(NULL, 0, format, va2);

	va_end(va2);

	char * output = (char *)malloc(sizeof(char) * (length + 1));

	vsnprintf(output, length + 1, format, va1);

	return output;
}

char * msprintf(char * format, ...) {
	va_list va;

	va_start(va, format);

	char * output = vmsprintf(format, va);

	va_end(va);

	return output;
}

Vec2d vec2d(double x, double y) {
	Vec2d o;

	o.x = x;
	o.y = y;

	return o;
}

// Scale a 2d vector from one rectangle to another
Vec2d vec2dremap(Vec2d s, Rect range_in, Rect range_out) {
	Vec2d o;

	o.x = fremap(s.x, range_in.tl.x, range_in.br.x, range_out.tl.x, range_out.br.x);
	o.y = fremap(s.y, range_in.tl.y, range_in.br.y, range_out.tl.y, range_out.br.y);

	return o;
}

// Given a vector, calculate a new y-value for a given x that is along the path of the vector
Vec2d vec2d_intercept_y_at_x(Vec2d point, double x) {
	Vec2d o;

	o.x = x;
	o.y = x * point.y / point.x;

	return o;
}

// Given a vector, calculate a new x-value for a given y that is along the path of the vector
Vec2d vec2d_intercept_x_at_y(Vec2d point, double y) {
	Vec2d o;

	o.x = y * point.x / point.y;
	o.y = y;

	return o;
}

double vec2d_dot(Vec2d a, Vec2d b) {
	return a.x * b.x + a.y * b.y;
}

double vec2d_magnitude(Vec2d point) {
	return sqrt(pow(point.x, 2.0) + pow(point.y, 2.0));
}

// Given two vectors and a velocity, determine the time it will take to cross
double vec2d_intercept_time(Vec2d p0, Vec2d p1, Vec2d v) {
	// Check if line is parallel (they'll never meet if they are)
	if (p0.x != 0.0 && p1.x != 0.0 && p0.y / p0.x == p1.y / p1.x) {
		return INFINITY;
	}

	double dist = vec2d_magnitude(p1) - vec2d_magnitude(p0);
	double t = dist / vec2d_magnitude(v);

	return t;
}
