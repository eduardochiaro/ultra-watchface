#include "complications.h"
#include "../weather.h"

#if defined(PBL_PLATFORM_GABBRO)
#define SUB_D      29   // subdial center from the face center
#define SUB_R      14   // ring centerline
#else
#define SUB_D      31
#define SUB_R      16
#endif
#define SUB_T      3
#define SUB_TEXT   8    // value
#define SUB_SMALL  6    // caption under it
#define SUB_LOW    10   // caption's y below the subdial center, in the ring's gap

// Ring open at the bottom, filling clockwise from 8 o'clock.
static Slot ring(GPoint c) {
  return (Slot){ .center = c, .radius = SUB_R, .thickness = SUB_T,
                 .a0 = DEG_TO_TRIGANGLE(-120), .a1 = DEG_TO_TRIGANGLE(120) };
}

static void gauge(GContext *ctx, GPoint c, int pct, GColor fill) {
  Slot s = ring(c);
  slot_arc(ctx, &s, 0, 100, COMP_TRACK);
  if (pct > 0) slot_arc(ctx, &s, 0, clamp_i32(pct, 0, 100), fill);
}

// Today's range in one color, a thumb at now, min and max under the value.
static void temp(GContext *ctx, GPoint c) {
  Slot s = ring(c);
  const Weather *w = &g_weather;
  char buf[12] = "--";
  if (!w->valid) {
    slot_arc(ctx, &s, 0, 100, COMP_TRACK);
  } else {
    int span = w->temp_max - w->temp_min;
    slot_arc(ctx, &s, 0, 100, GColorChromeYellow);
    slot_dot(ctx, &s, span > 0 ? clamp_i32((w->temp - w->temp_min) * 100 / span, 0, 100) : 50,
             SUB_T / 2 + 1, GColorWhite, GColorBlack);
    snprintf(buf, sizeof(buf), "%d", w->temp_min);
    text_draw(ctx, buf, GPoint(c.x - 7, c.y + SUB_LOW), SUB_SMALL, GColorLightGray);
    snprintf(buf, sizeof(buf), "%d", w->temp_max);
    text_draw(ctx, buf, GPoint(c.x + 7, c.y + SUB_LOW), SUB_SMALL, GColorLightGray);
    snprintf(buf, sizeof(buf), "%d°", w->temp);
  }
  text_draw(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT, GColorWhite);
}

// Canopy and stem.
static void umbrella(GContext *ctx, GPoint c) {
  graphics_context_set_fill_color(ctx, theme(GColorWhite));
  graphics_fill_radial(ctx, GRect(c.x - 4, c.y - 3, 9, 9), GOvalScaleModeFitCircle, 5,
                       DEG_TO_TRIGANGLE(-90), DEG_TO_TRIGANGLE(90));
  graphics_context_set_stroke_color(ctx, theme(GColorWhite));
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(c.x, c.y + 1), GPoint(c.x, c.y + 4));
}

static void rain(GContext *ctx, GPoint c) {
  char buf[8] = "--";
  if (g_weather.valid) snprintf(buf, sizeof(buf), "%d%%", g_weather.rain);
  gauge(ctx, c, g_weather.valid ? g_weather.rain : 0, GColorPictonBlue);
  text_draw(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT, GColorWhite);
  umbrella(ctx, GPoint(c.x, c.y + SUB_LOW - 1));
}

// US AQI, 0..300 around the ring in the color of its EPA band.
static void aqi(GContext *ctx, GPoint c) {
  static const struct { int16_t to; uint8_t argb; } BANDS[] = {
    { 50, GColorGreenARGB8 }, { 100, GColorYellowARGB8 }, { 150, GColorOrangeARGB8 },
    { 200, GColorRedARGB8 }, { 300, GColorPurpleARGB8 }, { INT16_MAX, GColorBulgarianRoseARGB8 },
  };
  int v = g_weather.valid ? g_weather.aqi : -1;
  GColor color = GColorWhite;
  char buf[8] = "--";
  if (v >= 0) {
    unsigned i = 0;
    while (v > BANDS[i].to) i++;
    color = (GColor){ .argb = BANDS[i].argb };
    snprintf(buf, sizeof(buf), "%d", v);
  }
  gauge(ctx, c, v * 100 / 300, color);
  text_draw(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT, color);
  text_draw(ctx, "AQI", GPoint(c.x, c.y + SUB_LOW), SUB_SMALL, GColorWhite);
}

// A tear-off page: weekday on a red header (the accent or mono ink in those
// schemes), the day below in black on white whatever the scheme.
static void date(GContext *ctx, GPoint c) {
  static const char *const DAYS[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int r = SUB_R + SUB_T / 2, cut = -r / 3;
  if (theme_light()) disc_fill(ctx, c, r + 1, r + 1, GColorLightGray);  // edge on white
  disc_fill(ctx, c, r, r, fixed(GColorWhite));
  disc_fill(ctx, c, r, cut, GColorRed);
  // Contrast with the header: theme(Black) is light iff the scheme is.
  GColor ink = color_light(theme(GColorRed)) == theme_light() ? GColorWhite : GColorBlack;
  text_draw(ctx, DAYS[t->tm_wday], GPoint(c.x, c.y + (cut - r) / 2), SUB_SMALL - 1, ink);
  char buf[3];
  snprintf(buf, sizeof(buf), "%d", t->tm_mday);
  text_draw(ctx, buf, GPoint(c.x, c.y + (cut + r) / 2), SUB_TEXT + 2, fixed(GColorBlack));
}

void center_draw(GContext *ctx, GPoint c) {
  temp(ctx, GPoint(c.x, c.y - SUB_D));
  rain(ctx, GPoint(c.x - SUB_D, c.y));
  aqi(ctx, GPoint(c.x + SUB_D, c.y));
  date(ctx, GPoint(c.x, c.y + SUB_D));
}
