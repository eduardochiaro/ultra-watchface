#include "complications.h"
#include "../weather.h"
#include "../settings.h"

void comp_rain_draw(GContext *ctx, const Slot *s) {
  char buf[8] = "--";
  if (g_weather.valid) {
    snprintf(buf, sizeof(buf), "%d" CORNER_PCT, g_weather.rain);
  }
  comp_fill_gauge(ctx, s, g_weather.valid ? g_weather.rain : 0,
                  GColorPictonBlue, GColorOxfordBlue, buf, ICON_UMBRELLA);
}

// A thumb at the fill's end; "30%" shrinks to fit inside the ring, the umbrella under it.
void center_rain_draw(GContext *ctx, GPoint c) {
  char buf[8] = "--";
  if (g_weather.valid) {
    Slot s = center_ring(c);
    center_gauge(ctx, c, g_weather.rain, GColorPictonBlue);
    slot_dot(ctx, &s, clamp_i32(g_weather.rain, 0, 100), SUB_T / 2 + 1, 1, GColorCeleste, GColorBlack);
    snprintf(buf, sizeof(buf), "%d" SMALL_PCT, g_weather.rain);
  } else {
    center_gauge(ctx, c, 0, GColorPictonBlue);
  }
  center_fit_text(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT + 2, 2 * SUB_R - SUB_T - 4, GColorWhite);
  text_draw(ctx, ICON_UMBRELLA, GPoint(c.x, c.y + SUB_LOW), 9, GColorWhite);
}

// Darker the likelier rain is, by the quarter. Mono would make every blue the
// ink: grays instead, the track's dark gray left for none.
static const Band SHADES[] = {
  { 25, GColorCelesteARGB8 }, { 50, GColorPictonBlueARGB8 }, { 75, GColorBlueMoonARGB8 }, { INT16_MAX, GColorBlueARGB8 },
};
static const Band GRAYS[] = { { 50, GColorLightGrayARGB8 }, { INT16_MAX, GColorWhiteARGB8 } };

// Accent scheme, where every blue would be the accent: the background a
// quarter, half, three quarters of the way to it, then the accent itself;
// fixed(), so theme() leaves them be. 2 bits a channel: close ones can match.
static void accent_shades(Band out[4]) {
  GColor bg = { .argb = g_settings.bg }, ac = { .argb = g_settings.accent };
  for (int k = 1; k <= 4; k++) {
    GColor c = fixed(GColorBlack);
    c.r = (bg.r * (4 - k) + ac.r * k + 2) / 4;
    c.g = (bg.g * (4 - k) + ac.g * k + 2) / 4;
    c.b = (bg.b * (4 - k) + ac.b * k + 2) / 4;
    out[k - 1] = (Band){ k < 4 ? k * 25 : INT16_MAX, c.argb };
  }
}

// A section an hour, the one before this one to four on: its shade, the track
// for none or unknown. This hour's chance, -1 = unknown.
static int rain_shades(uint8_t argb[6]) {
  Band accent[4];
  const Band *shades = g_settings.scheme & SCHEME_MONO ? GRAYS : SHADES;
  if (g_settings.scheme == SCHEME_ACCENT) {
    accent_shades(accent);
    shades = accent;
  }
  for (int i = 0; i < 6; i++) {
    int p = weather_rain_hour(i - 1), b = 0;
    while (p > shades[b].to) {
      b++;
    }
    argb[i] = p > 0 ? shades[b].argb : 0;
  }
  return weather_rain_hour(0);
}

// Laid out like comp_rain_draw, AQI-like sections for the bar, left to right,
// this hour's bigger; its chance on the left end.
void comp_rain_hourly_draw(GContext *ctx, const Slot *s) {
  uint8_t argb[6];
  int now = rain_shades(argb), left = slot_left_end(s);
  char buf[8] = "--";
  if (now >= 0) {
    snprintf(buf, sizeof(buf), "%d" CORNER_PCT, now);
  }
  Slot b = *s;
  comp_icon(ctx, &b, ICON_UMBRELLA, GColorPictonBlue);
  comp_end_label(ctx, &b, left, buf);
  comp_sections(ctx, &b, left, argb, 6, 1);
}

// The same around the ring, over the top from 8 o'clock; laid out like center_rain_draw.
void center_rain_hourly_draw(GContext *ctx, GPoint c) {
  uint8_t argb[6];
  int now = rain_shades(argb);
  char buf[8] = "--";
  if (now >= 0) {
    snprintf(buf, sizeof(buf), "%d" SMALL_PCT, now);
  }
  Slot s = center_ring(c);
  comp_sections(ctx, &s, false, argb, 6, 1);
  center_fit_text(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT + 2, 2 * SUB_R - SUB_T - 4, GColorWhite);
  text_draw(ctx, ICON_UMBRELLA, GPoint(c.x, c.y + SUB_LOW), 9, GColorWhite);
}
