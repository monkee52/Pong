#ifndef UTILS_H_

#define UTILS_H_

#include <stdint.h>
#include <stdarg.h>

// General 2 dimensional vector
typedef struct {
	double x;
	double y;
} Vec2d, *PVec2d;

// General rectangle
typedef struct {
	Vec2d tl; // Top left
	Vec2d br; // Bottom right
} Rect, *PRect;

Rect create_rect(double tl_x, double tl_y, double br_x, double br_y);

uint64_t microtime();
uint64_t nanotime();

double fsign(double x);
double fclamp(double x, double low, double high);
double fremap(double s, double x1, double y1, double x2, double y2);

char * vmsprintf(char * format, va_list va1);
char * msprintf(char * format, ...);

Vec2d vec2d(double x, double y);
Vec2d vec2dremap(Vec2d s, Rect range_in, Rect range_out);
Vec2d vec2d_intercept_y_at_x(Vec2d point, double x);
Vec2d vec2d_intercept_x_at_y(Vec2d point, double y);
double vec2d_intercept_time(Vec2d p0, Vec2d p1, Vec2d v);

#define SECONDS_PER_MINUTE           60

#define NANOSECONDS_PER_SECOND       UINT64_C(1000000000)
#define NANOSECONDS_PER_MILLISECOND  UINT64_C(1000000)
#define NANOSECONDS_PER_MICROSECOND  UINT64_C(1000)

#define MICROSECONDS_PER_SECOND      UINT64_C(1000000)
#define MICROSECONDS_PER_MILLISECOND UINT64_C(1000)

#define HALF_SECOND_MS 500

#endif
