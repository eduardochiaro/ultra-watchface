#include "complications.h"
#include "../settings.h"
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

// A thumb at the fill's end; the % sits by the umbrella, not the value.
static void rain(GContext *ctx, GPoint c) {
  char buf[8] = "--";
  if (g_weather.valid) {
    Slot s = ring(c);
    gauge(ctx, c, g_weather.rain, GColorPictonBlue);
    slot_dot(ctx, &s, clamp_i32(g_weather.rain, 0, 100), SUB_T / 2 + 1, GColorCeleste, GColorBlack);
    snprintf(buf, sizeof(buf), "%d", g_weather.rain);
  } else {
    gauge(ctx, c, 0, GColorPictonBlue);
  }
  text_draw(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT + 2, GColorWhite);
  text_draw(ctx, ICON_UMBRELLA, GPoint(c.x - 2, c.y + SUB_LOW), 9, GColorWhite);
  text_draw(ctx, "%", GPoint(c.x + 5, c.y + SUB_LOW + 2), SUB_SMALL - 2, GColorWhite);
}

// US AQI, 0..300 around the ring in the color of its EPA band.
static void aqi(GContext *ctx, GPoint c) {
  int v = g_weather.valid ? g_weather.aqi : -1;
  GColor color = aqi_color(v);
  char buf[8] = "--";
  if (v >= 0) snprintf(buf, sizeof(buf), "%d", v);
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
  text_draw(ctx, DAYS[t->tm_wday], GPoint(c.x, c.y + (cut - r) / 2), SUB_SMALL - 1, ink_on(GColorRed));
  char buf[3];
  snprintf(buf, sizeof(buf), "%d", t->tm_mday);
  text_draw(ctx, buf, GPoint(c.x, c.y + (cut + r) / 2), SUB_TEXT + 2, fixed(GColorBlack));
}

// Charge around the ring, a bolt below: yellow while charging.
static void battery(GContext *ctx, GPoint c) {
  BatteryChargeState b = battery_state_service_peek();
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", b.charge_percent);
  gauge(ctx, c, b.charge_percent, b.charge_percent <= 20 ? GColorRed : GColorGreen);
  text_draw(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT + 1, GColorWhite);
  text_draw(ctx, ICON_BOLT, GPoint(c.x, c.y + SUB_LOW + 1), 12, b.is_charging ? GColorChromeYellow : GColorWhite);
}

// No ring: a heart on a dark red glow, the caption above, the reading over its tip.
static void heart(GContext *ctx, GPoint c) {
  int bpm = heart_bpm();
  char buf[12] = "--";
  if (bpm > 0) snprintf(buf, sizeof(buf), "%d", bpm);
  disc_fill(ctx, c, SUB_R + 1, SUB_R + 1, GColorBulgarianRose);
  text_draw(ctx, "BPM", GPoint(c.x, c.y - SUB_R + 5), SUB_SMALL - 1, GColorMelon);
  text_draw(ctx, ICON_HEART, c, SUB_R + 4, GColorRed);
  text_draw(ctx, buf, GPoint(c.x, c.y + SUB_R / 2 + 1), SUB_TEXT, GColorWhite);
}

// Step goal around the ring (see comp_distance_draw), runner in the gap.
static void distance(GContext *ctx, GPoint c) {
  char buf[12];
  distance_text(buf, sizeof(buf));
  gauge(ctx, c, step_pct(), GColorChromeYellow);
  text_draw(ctx, g_settings.imperial ? "MI" : "KM", GPoint(c.x, c.y - 7), SUB_SMALL - 1, GColorWhite);
  text_draw(ctx, buf, GPoint(c.x, c.y + 2), SUB_TEXT + 1, GColorWhite);
  text_draw(ctx, ICON_RUNNER, GPoint(c.x, c.y + SUB_LOW + 2), 10, GColorChromeYellow);
}

// Ground height where the phone is: arrow and unit above a red pill holding
// the number, bars fading below.
static void elevation(GContext *ctx, GPoint c) {
  char buf[12];
  elevation_text(buf, sizeof(buf));
  const char *unit = g_settings.imperial ? "FT" : "M";
  int ux = c.x + 3, ax = ux - text_width(ctx, unit, SUB_SMALL) / 2 - 5, ay = c.y - 11;
  text_draw(ctx, ICON_ARROW, GPoint(ax, ay), 11, GColorRed);
  text_draw(ctx, unit, GPoint(ux, ay), SUB_SMALL, GColorWhite);
  graphics_context_set_fill_color(ctx, theme(GColorRed));
  graphics_fill_rect(ctx, GRect(c.x - SUB_R + 1, c.y - 5, 2 * SUB_R - 2, 12), 4, GCornersAll);
  text_draw(ctx, buf, GPoint(c.x, c.y + 1), SUB_TEXT, ink_on(GColorRed));
  static const struct { int8_t hw; uint8_t argb; } BARS[] = {
    { 10, GColorDarkCandyAppleRedARGB8 }, { 7, GColorBulgarianRoseARGB8 }, { 4, GColorBulgarianRoseARGB8 },
  };
  for (unsigned i = 0; i < ARRAY_LENGTH(BARS); i++) {
    graphics_context_set_fill_color(ctx, theme((GColor){ .argb = BARS[i].argb }));
    graphics_fill_rect(ctx, GRect(c.x - BARS[i].hw, c.y + 10 + 3 * i, 2 * BARS[i].hw, 2), 0, GCornerNone);
  }
}

typedef void (*Subdial)(GContext *ctx, GPoint c);

static const Subdial SUBDIAL[COMP_COUNT] = {
  [COMP_TEMP] = temp, [COMP_RAIN] = rain, [COMP_AQI] = aqi, [COMP_CALENDAR] = date,
  [COMP_BATTERY] = battery, [COMP_HEART] = heart, [COMP_DISTANCE] = distance, [COMP_ELEVATION] = elevation,
};

void center_draw(GContext *ctx, GPoint c) {
  const GPoint at[CENTER_POS_COUNT] = {
    GPoint(c.x, c.y - SUB_D), GPoint(c.x - SUB_D, c.y), GPoint(c.x + SUB_D, c.y), GPoint(c.x, c.y + SUB_D),
  };
  for (int i = 0; i < CENTER_POS_COUNT; i++) {
    uint8_t id = g_settings.center[i];
    if (id < COMP_COUNT && SUBDIAL[id]) SUBDIAL[id](ctx, at[i]);
  }
}
