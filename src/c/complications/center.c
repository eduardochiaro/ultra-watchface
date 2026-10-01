#include "complications.h"
#include "../settings.h"
#include "../weather.h"

Slot center_ring(GPoint c) {
  return (Slot){ .center = c, .radius = SUB_R, .thickness = SUB_T,
                 .a0 = DEG_TO_TRIGANGLE(-120), .a1 = DEG_TO_TRIGANGLE(120) };
}

void center_gauge(GContext *ctx, GPoint c, int pct, GColor fill) {
  Slot s = center_ring(c);
  slot_arc(ctx, &s, 0, 100, COMP_TRACK);
  if (pct > 0) slot_arc(ctx, &s, 0, clamp_i32(pct, 0, 100), fill);
}

// Conditions now as one icon, no ring: text color, the accent in that scheme.
static void conditions(GContext *ctx, GPoint c) {
  static const char *const ICON[] = {
    ICON_SUN, ICON_MOON, ICON_SUN_CLOUD, ICON_MOON_CLOUD, ICON_CLOUD, ICON_FOG, ICON_RAIN, ICON_SNOW, ICON_STORM,
  };
  int i = g_weather.valid ? g_weather.condition : -1;
  // Any bright color themes to the accent; white stays the text color.
  GColor color = g_settings.scheme == SCHEME_ACCENT ? GColorChromeYellow : GColorWhite;
  if (i >= 0 && i < (int)ARRAY_LENGTH(ICON)) text_draw(ctx, ICON[i], c, 7 * SUB_R / 4, color);  // 28px on emery
  else text_draw(ctx, "--", c, SUB_TEXT, color);
}

typedef void (*Subdial)(GContext *ctx, GPoint c);

static const Subdial SUBDIAL[COMP_COUNT] = {
  [COMP_TEMP] = center_temp_draw, [COMP_RAIN] = center_rain_draw, [COMP_AQI] = center_aqi_draw,
  [COMP_CALENDAR] = center_calendar_draw, [COMP_BATTERY] = center_battery_draw, [COMP_HEART] = center_heart_draw,
  [COMP_DISTANCE] = center_distance_draw, [COMP_ELEVATION] = center_elevation_draw, [COMP_UV] = center_uv_draw,
  [COMP_WEATHER] = conditions, [COMP_HUMIDITY] = center_humidity_draw,
};

void center_draw(GContext *ctx, GPoint c) {
  const GPoint at[CENTER_POS_COUNT] = {
    GPoint(c.x, c.y - SUB_D), GPoint(c.x - SUB_D, c.y), GPoint(c.x + SUB_D, c.y), GPoint(c.x, c.y + SUB_D),
  };
  for (int i = 0; i < CENTER_POS_COUNT; i++) {
    uint8_t id = g_settings.center[i];
    if (id == COMP_CUSTOM) center_custom_draw(ctx, at[i], g_settings.center_text[i]);
    else if (id < COMP_COUNT && SUBDIAL[id]) SUBDIAL[id](ctx, at[i]);
  }
}
