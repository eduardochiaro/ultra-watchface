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
  [COMP_SUN]     = comp_sun_draw,
  [COMP_BEAT]    = comp_beat_draw,
  [COMP_WIND]    = comp_wind_draw,
  [COMP_AQI_GAUGE] = comp_aqi_gauge_draw,
  [COMP_UV_GAUGE] = comp_uv_gauge_draw,
  [COMP_CALORIES] = comp_calories_draw,
  [COMP_CALORIES_ACTIVE] = comp_calories_active_draw,
  [COMP_TIME]    = comp_time_draw,
  [COMP_ZONE]    = comp_zone_draw,
  [COMP_SLEEP]   = comp_sleep_draw,
  [COMP_ACTIVE]  = comp_active_draw,
  [COMP_MOON]    = comp_moon_draw,
};

void complication_draw(ComplicationId id, GContext *ctx, const Slot *s) {
  if (id >= COMP_API && id <= COMP_API_LAST) {
    comp_api_draw(ctx, s, id - COMP_API);
  } else if (id < COMP_COUNT && DRAW[id]) {
    DRAW[id](ctx, s);
  }
}

// Icons are private-use glyphs (U+E000 on, 0xEE in UTF-8); anything else is a
// name, at label size.
static int icon_size(const char *icon) {
  return (uint8_t)icon[0] == 0xEE ? COMP_ICON : COMP_TEXT;
}

#if defined(PBL_PLATFORM_GABBRO)
// Px between it and the label it leads (gabbro): a name needs a word space.
static int icon_gap(const char *icon) {
  return icon_size(icon) == COMP_ICON ? COMP_GAP : 2 * COMP_GAP;
}
#endif

int comp_fit(GContext *ctx, const char *txt, int size, int width) {
  int w = text_width(ctx, txt, size);  // grows with size
  return w > width ? size * width / w : size;
}

// comp_end_label at any size.
static void end_label(GContext *ctx, Slot *s, int end, const char *txt, int size) {
  int w = text_width(ctx, txt, size);
  text_draw_along(ctx, txt, slot_past(s, end, -w / 2), s->center, size, GColorWhite);
  slot_trim(s, end, w + COMP_GAP + s->thickness / 2);
}

// Bar: value label on the left, filling from the left. A long label shrinks to
// leave half the slot to the bar.
void comp_fill_gauge(GContext *ctx, const Slot *s, int pct, GColor fill, GColor track,
                     const char *label, const char *icon) {
  pct = clamp_i32(pct, 0, 100);
  int left = slot_left_end(s);
  Slot b = *s;
  if (icon) {
    comp_icon(ctx, &b, icon, fill);
  }
  end_label(ctx, &b, left, label, comp_fit(ctx, label, COMP_TEXT, slot_len(&b) / 2));
  slot_arc(ctx, &b, 0, 100, track);
  if (pct > 0) {
    slot_arc(ctx, &b, left, left ? 100 - pct : pct, fill);
  }
}

// Each shade runs on to `pct` over the one before, so its cap rounds the join.
// The arc from its start to `pct` in `n` shades spread evenly over the whole
// arc; `flip`: it starts at the 100 end.
static void shaded_arc(GContext *ctx, const Slot *s, bool flip, int pct, const uint8_t *fill, int n) {
  for (int i = 0; i < n; i++) {
    int p = i * 100 / n;
    if (p >= pct) {
      break;
    }
    if (i && fill[i] == fill[i - 1]) {
      continue;
    }
    slot_arc(ctx, s, flip ? 100 - p : p, flip ? 100 - pct : pct, (GColor){ .argb = fill[i] });
  }
}

void comp_range_draw(GContext *ctx, const Slot *slot, int pct, const char *min, const char *max,
                     const char *value, const uint8_t *fill, int n) {
  Slot b = *slot, *s = &b;
  int size = comp_fit(ctx, value, COMP_TEXT + 2, slot_len(slot));
#if defined(PBL_PLATFORM_GABBRO)
  text_draw_along(ctx, value, slot_point(s, 50, 0), s->center, size, GColorWhite);
#else
  if (pct < 0) {
    slot_arc(ctx, s, 0, 100, COMP_TRACK);
  } else {
    int lo = slot_left_end(s);
    comp_end_label(ctx, s, lo, min);
    comp_end_label(ctx, s, 100 - lo, max);
    shaded_arc(ctx, s, lo, 100, fill, n);
    slot_dot(ctx, s, lo ? 100 - pct : pct, s->thickness / 2, 2, GColorWhite, GColorBlack);
  }
  // Centered, not on the thumb: near min/max it would run off the arc end.
  text_draw_along(ctx, value, slot_point(s, 50, COMP_THUMB), s->center, size, GColorWhite);
#endif
}

void center_range_draw(GContext *ctx, GPoint c, int pct, const char *min, const char *max,
                       const char *value, const char *name, const uint8_t *fill, int n) {
  Slot s = center_ring(c);
  if (pct < 0) {
    slot_arc(ctx, &s, 0, 100, COMP_TRACK);
  } else {
    shaded_arc(ctx, &s, false, 100, fill, n);
    slot_dot(ctx, &s, pct, SUB_T / 2 + 1, 1, GColorWhite, GColorBlack);
    // The ring's gap fits about 3 characters a side.
    if (strlen(min) <= 3 && strlen(max) <= 3) {
      text_draw(ctx, min, GPoint(c.x - 7, c.y + SUB_LOW), SUB_SMALL, GColorLightGray);
      text_draw(ctx, max, GPoint(c.x + 7, c.y + SUB_LOW), SUB_SMALL, GColorLightGray);
    }
  }
  if (name) {
    text_draw(ctx, name, GPoint(c.x, c.y - 7), SUB_SMALL - 1, GColorWhite);
  }
  center_fit_text(ctx, value, GPoint(c.x, c.y + (name ? 2 : -1)), SUB_TEXT, 2 * SUB_R - SUB_T - 4, GColorWhite);
}

// A section per band, 2px apart: lit in their own colors up to the band `v` is
// in, the rest track. `flip`: the first is at the 100 end.
static void band_sections(GContext *ctx, const Slot *s, bool flip, int v, const Band *bands) {
  int n = 1, lit = -1;
  while (bands[n - 1].to != INT16_MAX) {
    n++;
  }
  if (v >= 0) {
    lit = 0;
    while (v > bands[lit].to) {
      lit++;
    }
  }
  int32_t span = s->a1 - s->a0;
  int32_t inset = span * (s->thickness + 2) / (2 * slot_len(s));  // its cap and half the gap
  for (int i = 0; i < n; i++) {
    int at = flip ? n - 1 - i : i;
    Slot b = *s;
    b.a0 = s->a0 + span * at / n + inset;
    b.a1 = s->a0 + span * (at + 1) / n - inset;
    slot_arc(ctx, &b, 0, 100, i <= lit ? (GColor){ .argb = bands[i].argb } : COMP_TRACK);
  }
}

// Sections captioned at their left end, the value curved by the middle. Gabbro
// has no room beside the arc: caption and value both lead, "AQI 42 SECTIONS".
void comp_band_draw(GContext *ctx, const Slot *slot, int v, const Band *bands, const char *caption) {
  char value[12] = "--";
  if (v >= 0) {
    snprintf(value, sizeof(value), "%d", v);
  }
  Slot s = *slot;
  int left = slot_left_end(&s);
#if defined(PBL_PLATFORM_GABBRO)
  char label[16];
  snprintf(label, sizeof(label), "%s %s", caption, value);
#else
  const char *label = caption;
  text_draw_along(ctx, value, slot_point(slot, 50, COMP_THUMB), slot->center, COMP_TEXT + 2, GColorWhite);
#endif
  end_label(ctx, &s, left, label, comp_fit(ctx, label, COMP_TEXT, slot_len(&s) / 2));
  band_sections(ctx, &s, left, v, bands);
}

void center_band_draw(GContext *ctx, GPoint c, int v, const Band *bands, const char *caption) {
  char buf[12] = "--";
  if (v >= 0) {
    snprintf(buf, sizeof(buf), "%d", v);
  }
  Slot s = center_ring(c);
  band_sections(ctx, &s, false, v, bands);
  text_draw(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT, GColorWhite);
  text_draw(ctx, caption, GPoint(c.x, c.y + SUB_LOW), SUB_SMALL, GColorWhite);
}

#define BANDS_MAX 8

// The bands' colors into `fill`, their count into *n; where `v` falls along
// them at even widths, in %, -1 for unknown.
static int band_pct(int v, const Band *bands, int top, uint8_t fill[BANDS_MAX], int *n) {
  int count = 0;
  do {
    fill[count] = bands[count].argb;
  } while (bands[count++].to != INT16_MAX && count < BANDS_MAX);
  *n = count;
  if (v < 0) {
    return -1;
  }
  int i = 0, lo = 0;
  while (v > bands[i].to) {
    lo = bands[i++].to;
  }
  int hi = bands[i].to == INT16_MAX ? top : bands[i].to;
  return clamp_i32((i * 100 + (v - lo) * 100 / (hi - lo)) / count, 0, 100);
}

// Laid out like comp_band_draw, the sections one shaded arc with a thumb.
void comp_band_gauge_draw(GContext *ctx, const Slot *slot, int v, const Band *bands, int top, const char *caption) {
  uint8_t fill[BANDS_MAX];
  int n, pct = band_pct(v, bands, top, fill, &n);
  char value[12] = "--";
  if (v >= 0) {
    snprintf(value, sizeof(value), "%d", v);
  }
  Slot s = *slot;
  int left = slot_left_end(&s);
#if defined(PBL_PLATFORM_GABBRO)
  char label[16];
  snprintf(label, sizeof(label), "%s %s", caption, value);
#else
  const char *label = caption;
  text_draw_along(ctx, value, slot_point(slot, 50, COMP_THUMB), slot->center, COMP_TEXT + 2, GColorWhite);
#endif
  end_label(ctx, &s, left, label, comp_fit(ctx, label, COMP_TEXT, slot_len(&s) / 2));
  if (pct < 0) {
    slot_arc(ctx, &s, 0, 100, COMP_TRACK);
    return;
  }
  shaded_arc(ctx, &s, left, 100, fill, n);
  slot_dot(ctx, &s, left ? 100 - pct : pct, s.thickness / 2, 2, GColorWhite, GColorBlack);
}

// The caption above the value, no min and max.
void center_band_gauge_draw(GContext *ctx, GPoint c, int v, const Band *bands, int top, const char *caption) {
  uint8_t fill[BANDS_MAX];
  int n, pct = band_pct(v, bands, top, fill, &n);
  char value[8] = "--";
  if (v >= 0) {
    snprintf(value, sizeof(value), "%d", v);
  }
  center_range_draw(ctx, c, pct, "", "", value, caption, fill, n);
}

// No bar: the text curved along the arc's middle, the icon toward the corner.
// Gabbro: "ICON 72 BPM" centered on the arc. A long text shrinks to fit the slot.
void comp_icon_text(GContext *ctx, const Slot *s, const char *icon, GColor color, const char *txt) {
  Slot b = *s;
#if defined(PBL_PLATFORM_GABBRO)
  int left = slot_left_end(&b);
  int lead = text_width(ctx, icon, icon_size(icon)) + icon_gap(icon);
  int size = comp_fit(ctx, txt, COMP_TEXT, slot_len(&b) - lead);
  slot_trim(&b, left, (slot_len(&b) - lead - text_width(ctx, txt, size)) / 2);
  comp_icon(ctx, &b, icon, color);
  end_label(ctx, &b, left, txt, size);
#else
  text_draw_along(ctx, txt, slot_point(s, 50, 0), s->center, comp_fit(ctx, txt, COMP_TEXT + 2, slot_len(s)), GColorWhite);
  comp_icon(ctx, &b, icon, color);
#endif
}

// Label laid along the arc at an end; the arc is trimmed to make room so
// label + arc stay inside the slot's original span.
void comp_end_label(GContext *ctx, Slot *s, int end, const char *txt) {
  end_label(ctx, s, end, txt, COMP_TEXT);
}

// Just past the arc, toward the screen corner. Gabbro has no corners: the icon
// goes along the arc at its left end, which is trimmed like for a label.
void comp_icon(GContext *ctx, Slot *s, const char *icon, GColor color) {
  int size = icon_size(icon);
#if defined(PBL_PLATFORM_GABBRO)
  int end = slot_left_end(s), w = text_width(ctx, icon, size);
  text_draw_along(ctx, icon, slot_past(s, end, -w / 2), s->center, size, color);
  slot_trim(s, end, w + icon_gap(icon));
#else
  text_draw(ctx, icon, slot_corner(s, COMP_GAP + COMP_ICON / 2), size, color);
#endif
}
