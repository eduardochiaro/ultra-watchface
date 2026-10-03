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

GPoint polar(GPoint c, int32_t angle, int r);  // r px from c at a TRIG angle
GPoint slot_point(const Slot *s, int pct, int dr);
int slot_len(const Slot *s);  // px along the centerline
// Point `px` pixels beyond an arc end (end: 0 = a0, 100 = a1) along its circle.
GPoint slot_past(const Slot *s, int end, int px);
// Pull an end `px` pixels in toward the other end.
void slot_trim(Slot *s, int end, int px);
// `px` past the arc's outer edge on the line to the screen corner nearest its
// middle. Rect screens only.
GPoint slot_corner(const Slot *s, int px);
// The end (0 or 100) further left on screen.
int slot_left_end(const Slot *s);
void slot_arc(GContext *ctx, const Slot *s, int from_pct, int to_pct, GColor color);
// Arc segment with square ends, corners rounded by `r` px.
void slot_box(GContext *ctx, const Slot *s, int from_pct, int to_pct, int r, GColor color);
// Antialiased disc, flat below `cut` px from the center (cut = r: whole disc).
void disc_fill(GContext *ctx, GPoint c, int r, int cut, GColor color);
// Rounded bar on the ray from `c` at `angle`, from r0 to r1 px out, `w` wide.
void ray_bar(GContext *ctx, GPoint c, int32_t angle, int r0, int r1, int w, GColor color);
void slot_dot(GContext *ctx, const Slot *s, int pct, int r, GColor fill, GColor ring);

// Icons are font glyphs (resources/icons, U+E000 on): draw them with
// text_draw, `size` being the square they fill. Same order as ICONS in
// scripts/gen-svg-font.js.
#define ICON_HEART    "\uE000"
#define ICON_RUNNER   "\uE001"
#define ICON_BOLT     "\uE002"
#define ICON_UMBRELLA "\uE003"
#define ICON_ARROW    "\uE004"
// Weather conditions, in the order of CONDITION in src/pkjs/weather.js.
#define ICON_SUN        "\uE005"
#define ICON_MOON       "\uE006"
#define ICON_SUN_CLOUD  "\uE007"
#define ICON_MOON_CLOUD "\uE008"
#define ICON_CLOUD      "\uE009"
#define ICON_FOG        "\uE00A"
#define ICON_RAIN       "\uE00B"
#define ICON_SNOW       "\uE00C"
#define ICON_STORM      "\uE00D"
#define ICON_DROP     "\uE00E"
// Text-sized, inside a label: sunrise and sunset.
#define ICON_UP       "\uE00F"
#define ICON_DOWN     "\uE010"
#define ICON_WIND     "\uE011"

// Text centered on `c` (cap height `size` px). Text and icons are clamped so
// they stay on screen.
void text_draw(GContext *ctx, const char *txt, GPoint c, int size, GColor color);
// Same, curved along the circle around `center` (edge labels). Not clamped.
void text_draw_along(GContext *ctx, const char *txt, GPoint c, GPoint center, int size,
                     GColor color);
int text_width(GContext *ctx, const char *txt, int size);

// Maps a color from the black scheme (what all drawing code is written in) to
// the active scheme. The draw helpers apply it; direct graphics_* calls must too.
// On a light background grays are inverted and bright yellow is darkened.
// Accent scheme: black is the background, grays go dark or light to contrast
// with it, bright colors become the accent and dark ones a gray track.
GColor theme(GColor c);
// Whether the scheme's background is light (grays are inverted).
bool theme_light(void);
// A color theme() leaves alone, for things that keep their real-world look.
// ponytail: marked by alpha 2; nothing here draws translucent.
GColor fixed(GColor c);
// White or black, whichever reads on `fill` once both are themed.
GColor ink_on(GColor fill);

int32_t clamp_i32(int32_t v, int32_t lo, int32_t hi);
