#ifndef GRAPHICS_UTILS_H_

#define GRAPHICS_UTILS_H_

#include <stdint.h>
#include <stdarg.h>

#define BORDER_NONE	    0x00
#define BORDER_TOP      0x01
#define BORDER_RIGHT    0x02
#define BORDER_BOTTOM   0x04
#define BORDER_LEFT	    0x08

#define BORDER_HORIZ    (BORDER_TOP | BORDER_BOTTOM)
#define BORDER_VERT	    (BORDER_RIGHT | BORDER_LEFT)
#define BORDER_ALL      (BORDER_HORIZ | BORDER_VERT)

#define DS_NONE         0x00
#define DS_TOP          0x01
#define DS_RIGHT        0x02
#define DS_BOTTOM       0x04
#define DS_LEFT         0x08

#define DS_CENTER_HORIZ	(DS_LEFT | DS_RIGHT)
#define DS_CENTER_VERT  (DS_TOP | DS_BOTTOM)
#define DS_CENTER       (DS_CENTER_HORIZ | DS_CENTER_VERT)

void draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char c, uint8_t flags);
void draw_string2(uint16_t x, uint16_t y, int16_t x_offset, int16_t y_offset, uint8_t flags, char * str);
void draw_glyph(uint16_t x, uint16_t y, int16_t x_offset, int16_t y_offset, uint8_t flags, char * glpyh, uint16_t width, uint16_t height);

void draw_string2f(uint16_t x, uint16_t y, int16_t x_offset, int16_t y_offset, uint8_t flags, char * str, ...);

#endif
