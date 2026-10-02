#include "complications.h"

typedef void (*ComplicationDraw)(GContext *ctx, const Slot *s);

static const ComplicationDraw DRAW[COMP_COUNT] = {
  [COMP_STEPS]   = comp_steps_draw,
  [COMP_TEMP]    = comp_temp_draw,
  [COMP_BATTERY] = comp_battery_draw,
  [COMP_RAIN]    = comp_rain_draw,
  [COMP_CALENDAR] = comp_calendar_draw,
  [COMP_HEART]   = comp_heart_draw,
  [COMP_DISTANCE] = comp_distance_draw,
  [COMP_AQI]     = comp_aqi_draw,
  [COMP_ELEVATION] = comp_elevation_draw,
  [COMP_UV]      = comp_uv_draw,
  [COMP_HUMIDITY] = comp_humidity_draw,
};

void complication_draw(ComplicationId id, GContext *ctx, const Slot *s) {
  if (id < COMP_COUNT && DRAW[id]) DRAW[id](ctx, s);
}

// Bar: value label on the left, filling from the left.
void comp_fill_gauge(GContext *ctx, const Slot *s, int pct, GColor fill, GColor track,
                     const char *label, const char *icon) {
  pct = clamp_i32(pct, 0, 100);
  int left = slot_left_end(s);
  Slot b = *s;
  if (icon) comp_icon(ctx, &b, icon, fill);
  comp_end_label(ctx, &b, left, label);
  slot_arc(ctx, &b, 0, 100, track);
  if (pct > 0) slot_arc(ctx, &b, left, left ? 100 - pct : pct, fill);
}

// Bar captioned at its end, the value curved by its middle. Gabbro has no room
// beside the arc: caption and value both lead the bar, "AQI 42 BAR".
static void comp_value_gauge(GContext *ctx, const Slot *s, int pct, GColor fill, const char *caption,
                             const char *value) {
#if defined(PBL_PLATFORM_GABBRO)
  char buf[16];
  snprintf(buf, sizeof(buf), "%s %s", caption, value);
  comp_fill_gauge(ctx, s, pct, fill, COMP_TRACK, buf, NULL);
#else
  comp_fill_gauge(ctx, s, pct, fill, COMP_TRACK, caption, NULL);
  text_draw_along(ctx, value, slot_point(s, 50, COMP_THUMB), s->center, COMP_TEXT + 2, GColorWhite);
#endif
}

static GColor band_color(int v, const Band *bands) {
  if (v < 0) return GColorWhite;
  while (v > bands->to) bands++;
  return (GColor){ .argb = bands->argb };
}

void comp_band_draw(GContext *ctx, const Slot *s, int v, int max, const Band *bands, const char *caption) {
  char buf[12] = "--";
  if (v >= 0) snprintf(buf, sizeof(buf), "%d", v);
  comp_value_gauge(ctx, s, v >= 0 ? v * 100 / max : 0, band_color(v, bands), caption, buf);
}

void center_band_draw(GContext *ctx, GPoint c, int v, int max, const Band *bands, const char *caption) {
  GColor color = band_color(v, bands);
  char buf[12] = "--";
  if (v >= 0) snprintf(buf, sizeof(buf), "%d", v);
  center_gauge(ctx, c, v * 100 / max, color);
  text_draw(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT, color);
  text_draw(ctx, caption, GPoint(c.x, c.y + SUB_LOW), SUB_SMALL, GColorWhite);
}

// No bar: the text curved along the arc's middle, the icon toward the corner.
// Gabbro: "ICON 72 BPM" centered on the arc.
void comp_icon_text(GContext *ctx, const Slot *s, const char *icon, GColor color, const char *txt) {
  Slot b = *s;
#if defined(PBL_PLATFORM_GABBRO)
  int left = slot_left_end(&b);
  int w = text_width(ctx, icon, COMP_ICON) + COMP_GAP + text_width(ctx, txt, COMP_TEXT);
  slot_trim(&b, left, (slot_len(&b) - w) / 2);
  comp_icon(ctx, &b, icon, color);
  comp_end_label(ctx, &b, left, txt);
#else
  text_draw_along(ctx, txt, slot_point(s, 50, 0), s->center, COMP_TEXT + 2, GColorWhite);
  comp_icon(ctx, &b, icon, color);
#endif
}

// Label laid along the arc at an end; the arc is trimmed to make room so
// label + arc stay inside the slot's original span.
void comp_end_label(GContext *ctx, Slot *s, int end, const char *txt) {
  int w = text_width(ctx, txt, COMP_TEXT);
  text_draw_along(ctx, txt, slot_past(s, end, -w / 2), s->center, COMP_TEXT, GColorWhite);
  slot_trim(s, end, w + COMP_GAP + s->thickness / 2);
}

// Just past the arc, toward the screen corner. Gabbro has no corners: the icon
// goes along the arc at its left end, which is trimmed like for a label.
void comp_icon(GContext *ctx, Slot *s, const char *icon, GColor color) {
#if defined(PBL_PLATFORM_GABBRO)
  int end = slot_left_end(s), w = text_width(ctx, icon, COMP_ICON);
  text_draw_along(ctx, icon, slot_past(s, end, -w / 2), s->center, COMP_ICON, color);
  slot_trim(s, end, w + COMP_GAP);
#else
  text_draw(ctx, icon, slot_corner(s, COMP_GAP + COMP_ICON / 2), COMP_ICON, color);
#endif
}
