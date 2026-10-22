#include "complications.h"
#include "../settings.h"
#include "../weather.h"

// Sea-level pressure. In a subdial on a barometer's scale, PRESS_LO to PRESS_HI
// hPa: low and stormy in blue to high and fair in yellow. Imperial: inHg, to 2 decimals.
#define PRESS_LO 970
#define PRESS_HI 1050

static const uint8_t SHADES[] = {
  GColorBlueMoonARGB8, GColorPictonBlueARGB8, GColorCelesteARGB8, GColorIcterineARGB8,
};

// The scale's ends and the value; the thumb's %, -1 for unknown.
static int pressure_range(char min[8], char max[8], char now[12]) {
  int p = g_weather.valid ? g_weather.pressure : 0;
  if (g_settings.imperial) {
    strcpy(min, "28.6");
    strcpy(max, "31.0");
  } else {
    snprintf(min, 8, "%d", PRESS_LO);
    snprintf(max, 8, "%d", PRESS_HI);
  }
  if (p <= 0) {
    strcpy(now, "--");
    return -1;
  }
  if (g_settings.imperial) {
    int in = (p * 2953 + 500) / 1000;  // hundredths of an inHg
    snprintf(now, 12, "%d.%02d", in / 100, in % 100);
  } else {
    snprintf(now, 12, "%d", p);
  }
  return clamp_i32((p - PRESS_LO) * 100 / (PRESS_HI - PRESS_LO), 0, 100);
}

static const char *unit(void) {
  return g_settings.imperial ? "inHg" : "hPa";
}

// No gauge: a corner is too short for the scale's ends. The value along the
// arc, the unit for an icon; gabbro: one line, "1013 hPa".
void comp_pressure_draw(GContext *ctx, const Slot *s) {
  char min[8], max[8], now[12];
  pressure_range(min, max, now);
#if defined(PBL_PLATFORM_GABBRO)
  char buf[20];
  snprintf(buf, sizeof(buf), "%s %s", now, unit());
  text_draw_along(ctx, buf, slot_mid(s, 0), s->center, comp_fit(ctx, buf, COMP_TEXT + 2, slot_len(s)), GColorWhite);
#else
  comp_icon_text(ctx, s, unit(), GColorPictonBlue, now);
#endif
}

// The unit above the value; the ends are too long for the ring's gap.
void center_pressure_draw(GContext *ctx, GPoint c) {
  char min[8], max[8], now[12];
  int pct = pressure_range(min, max, now);
  center_range_draw(ctx, c, pct, min, max, now, unit(), SHADES, ARRAY_LENGTH(SHADES));
}
