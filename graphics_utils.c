#include <stdint.h>
#include <cab202_graphics.h>
#include <string.h>

#include <stdarg.h>
#include <stdlib.h>

#include "utils.h"
#include "graphics_utils.h"

// Private method to offset coordinates
void offset_coords(uint16_t * p_x, uint16_t * p_y, int16_t x_offset, int16_t y_offset, uint8_t flags, uint16_t width, uint16_t height) {
	uint16_t x = *p_x;
	uint16_t y = *p_y;

	if ((flags & DS_LEFT) && !(flags & DS_RIGHT)) {
		x = 0;
	} else if (flags & DS_RIGHT) {
		x = screen_width() - width;
	}

	if ((flags & DS_TOP) && !(flags && DS_BOTTOM)) {
		y = 0;
	} else if (flags & DS_BOTTOM) {
		y = screen_height() - height;
	}

	if ((flags & DS_LEFT) && (flags & DS_RIGHT)) {
		x = (screen_width() - width) / 2;
	}

	if ((flags & DS_TOP) && (flags & DS_BOTTOM)) {
		y = (screen_height() - height) / 2;
	}

	x += x_offset;
	y += y_offset;

	*p_x = x;
	*p_y = y;
}

// Utility method to draw a rectangle
void draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char c, uint8_t flags) {
	--w;
	--h;

	if (flags & BORDER_TOP) {
		draw_line(x + 0, y + 0, x + w, y + 0, c);
	}

	if (flags & BORDER_RIGHT) {
		draw_line(x + w, y + 0, x + w, y + h, c);
	}

	if (flags & BORDER_BOTTOM) {
		draw_line(x + 0, y + h, x + w, y + h, c);
	}

	if (flags & BORDER_LEFT) {
		draw_line(x + 0, y + 0, x + 0, y + h, c);
	}
}

void draw_string2(uint16_t x, uint16_t y, int16_t x_offset, int16_t y_offset, uint8_t flags, char * str) {
	offset_coords(&x, &y, x_offset, y_offset, flags, strlen(str), 1);

	draw_string(x, y, str);
}

// Creates a formatted string and uses draw_string2 to position it
void draw_string2f(uint16_t x, uint16_t y, int16_t x_offset, int16_t y_offset, uint8_t flags, char * str, ...) {
	va_list args;
	va_start(args, str);

	char * formatted = vmsprintf(str, args);

	va_end(args);
	
	draw_string2(x, y, x_offset, y_offset, flags, formatted);

	free(formatted);
}

void draw_glyph(uint16_t x, uint16_t y, int16_t x_offset, int16_t y_offset, uint8_t flags, char * glyph, uint16_t width, uint16_t height) {
	offset_coords(&x, &y, x_offset, y_offset, flags, width, height);

	for (uint16_t ly = 0; ly < height; ly++) {
		for (uint16_t lx = 0; lx < width; lx++) {
			draw_char(lx + x, ly + y, glyph[ly * width + lx]);
		}
	}
}
