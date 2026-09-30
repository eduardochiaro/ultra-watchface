#pragma once
#include <pebble.h>

// Drawing kit shared by the dial and every complication.
//
// A Slot is an arc on a circle around `center`. Angles are Pebble TRIG angles
// (0 = 12 o'clock, clockwise). Positions along it are given in percent:
// 0 = a0, 100 = a1. Values outside 0..100 run past the ends, which is where
// the labels usually go.
typedef struct {
  GPoint center;
  int16_t radius;     // arc centerline
  int16_t thickness;
  int32_t a0, a1;     // a0 = where a gauge starts filling from
} Slot;

void draw_init(GSize screen);  // loads the vector font
void draw_deinit(void);

GPoint slot_point(const Slot *s, int pct, int dr);
// Point `px` pixels beyond an arc end (end: 0 = a0, 100 = a1) along its circle.
GPoint slot_past(const Slot *s, int end, int px);
// Pull an end `px` pixels in toward the other end.
void slot_trim(Slot *s, int end, int px);
// The end (0 or 100) further left on screen.
int slot_left_end(const Slot *s);
void slot_arc(GContext *ctx, const Slot *s, int from_pct, int to_pct, GColor color);
void slot_dot(GContext *ctx, const Slot *s, int pct, int r, GColor fill, GColor ring);

// ponytail: placeholder until real icons land; swap for a gdraw_command_image.
void icon_block(GContext *ctx, GPoint c, int size, GColor color);

// Text centered on `c` (cap height `size` px). Text and icons are clamped so
// they stay on screen.
void text_draw(GContext *ctx, const char *txt, GPoint c, int size, GColor color);
// Same, curved along the circle around `center` (edge labels). Not clamped.
void text_draw_along(GContext *ctx, const char *txt, GPoint c, GPoint center, int size,
                     GColor color);
int text_width(GContext *ctx, const char *txt, int size);

// Maps a color from the black scheme (what all drawing code is written in) to
// the active scheme. The draw helpers apply it; direct graphics_* calls must too.
GColor theme(GColor c);

int32_t clamp_i32(int32_t v, int32_t lo, int32_t hi);
