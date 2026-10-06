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
  if (pct > 0) {
    slot_arc(ctx, &s, 0, clamp_i32(pct, 0, 100), fill);
  }
}

// Conditions now as one icon, no ring: text color, the accent in that scheme.
static void conditions(GContext *ctx, GPoint c) {
  const char *icon = weather_icon();
  // Any bright color themes to the accent; white stays the text color.
  GColor color = g_settings.scheme == SCHEME_ACCENT ? GColorChromeYellow : GColorWhite;
  if (icon) {
    text_draw(ctx, icon, c, 7 * SUB_R / 4, color);  // 28px on emery
  } else {
    text_draw(ctx, "--", c, SUB_TEXT, color);
  }
}

typedef void (*Subdial)(GContext *ctx, GPoint c);

static const Subdial SUBDIAL[COMP_COUNT] = {
  [COMP_TEMP] = center_temp_draw, [COMP_RAIN] = center_rain_draw, [COMP_AQI] = center_aqi_draw,
  [COMP_BATTERY] = center_battery_draw, [COMP_HEART] = center_heart_draw,
  [COMP_DISTANCE] = center_distance_draw, [COMP_ELEVATION] = center_elevation_draw, [COMP_UV] = center_uv_draw,
  [COMP_WEATHER] = conditions, [COMP_HUMIDITY] = center_humidity_draw,
  [COMP_SUN] = center_sun_draw, [COMP_BEAT] = center_beat_draw,
  [COMP_WIND] = center_wind_draw,
  [COMP_AQI_GAUGE] = center_aqi_gauge_draw, [COMP_UV_GAUGE] = center_uv_gauge_draw,
  [COMP_CALORIES] = center_calories_draw, [COMP_CALORIES_ACTIVE] = center_calories_active_draw,
  [COMP_TIME] = center_time_draw, [COMP_SLEEP] = center_sleep_draw,
  [COMP_ACTIVE] = center_active_draw, [COMP_MOON] = center_moon_draw,
  [COMP_LOCATION] = center_location_draw,
  [COMP_WEEK] = center_week_draw, [COMP_YEAR] = center_year_draw,
};

void center_draw(GContext *ctx, GPoint c) {
  const GPoint at[CENTER_POS_COUNT] = {
    GPoint(c.x, c.y - SUB_D), GPoint(c.x - SUB_D, c.y), GPoint(c.x + SUB_D, c.y), GPoint(c.x, c.y + SUB_D),
  };
  for (int i = 0; i < CENTER_POS_COUNT; i++) {
    uint8_t id = g_settings.center[i];
    if (id == COMP_CUSTOM) {
      center_custom_draw(ctx, at[i], g_settings.center_text[i]);
    } else if (id == COMP_ZONE) {
      center_zone_draw(ctx, at[i], g_settings.zone[SLOT_POS_COUNT + i], g_settings.center_text[i]);
    } else if (id == COMP_CALENDAR) {
      center_date_draw(ctx, at[i], g_settings.zone[SLOT_POS_COUNT + i]);
    } else if (id == COMP_CALENDAR_PLAIN) {
      center_date_draw(ctx, at[i], DATE_WEEKDAY);
    } else if (id >= COMP_API && id <= COMP_API_LAST) {
      center_api_draw(ctx, at[i], id - COMP_API);
    } else if (id < COMP_COUNT && SUBDIAL[id]) {
      SUBDIAL[id](ctx, at[i]);
    }
  }
}
