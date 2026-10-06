#include <pebble.h>
#include "draw.h"
#include "settings.h"
#include "weather.h"
#include "complications/complications.h"

// Dial geometry, tuned per screen.
#if defined(PBL_PLATFORM_GABBRO)   // 260x260 round: corner arcs hug the edge
#define DIAL_R     102  // its outer ring stops 4px short of the corner labels
#define NUM_R_BIG  81   // 12/3/6/9
#define NUM_R      76   // the other numerals
#define NUM_BIG    18
#define NUM_SMALL  12
#define INNER_R    59   // ring around the subdials
#define CHRONO_NUM 9    // chronograph ring's hours
#define COMP_R     118
#define COMP_T     6
#else                              // emery 200x228: dial fills the width, arcs in the corners
#define DIAL_R     94
#define NUM_R_BIG  77
#define NUM_R      69
#define NUM_BIG    14
#define NUM_SMALL  9
#define INNER_R    54
#define CHRONO_NUM 8
#define COMP_R     108
#define COMP_T     6
#endif

// Minimal, sport and chronograph rings. Their outer edge is the default's outer circle.
#define RING_OUT   (DIAL_R + 4)
#define MIN_TICK   8    // minimal's ticks
#define CHR_BAND   22   // sport's white band
#define CHR_INSET  4    // the accent ring inside it; chronograph's band is that much wider instead
#define CHR_NUM_R  (RING_OUT - 15)
#define CHR_NUM    (NUM_SMALL - 2)  // sport's numerals
#define TACH_TICK  7    // tachymeter's ticks; its band fills the rest, a 2px gap between
#define TACH_NUM_R (RING_OUT - 18)
#define CMP_TICK   7    // compass's 30 degree ticks; the 5 degree ones are 3px shorter

#define COMP_SPAN  45   // degrees per corner complication, labels included
#define ACCENT     GColorChromeYellow
#define DEG(d)     DEG_TO_TRIGANGLE(d)
// Bar and outline hands: a stem out of the pin, then the bar.
#define HAND_STEM   12  // px from the center to the bar
#define HAND_STEM_W 2
#define HAND_W      8
#define HAND_EDGE   2   // outline thickness

#define PK_SETTINGS 10
#define CACHE_HEADROOM 4000   // bytes left free beside the face cache, and beside fctx
#define LOW_BATTERY    20     // % and under, off the charger: no seconds hand
#define SWEEP_MS       125    // a sweeping seconds hand's step: 8 a second, a 4 Hz movement's
// Shake twice to hide the hands. Tune on the wrist: one hard shake can tap more than once.
#define SHAKE_MIN_MS   200    // taps closer than this are the same shake
#define SHAKE_MAX_MS   1500   // the second shake comes within this of the first
#define SHAKE_HIDE_MS  5000   // how long the hands stay away

Settings g_settings = {
  .slots = { COMP_STEPS, COMP_TEMP, COMP_RAIN, COMP_BATTERY },
  .seconds = false,
  .step_goal = 10000,
  .bg = GColorBlackARGB8,
  .accent = GColorChromeYellowARGB8,
  .center = { COMP_CUSTOM, COMP_ELEVATION, COMP_WEATHER, COMP_CALENDAR },
  .center_text = { "PB" },
};

static Window *s_window;
static Layer *s_layer;
// The face minus the hands, copied in under them every tick and drawn again
// only when what it shows changed: a message, or a shown complication's stamp.
// Text is the costly part to draw. Kept as runs of one color, a row at a time:
// a face is mostly flat, some 11KB. fctx takes a byte per screen pixel while it
// draws, so the cache is let go before the face is drawn and made again after
// it. Hands fctx draws need that room on every tick: emery has 24KB beside it,
// gabbro 2KB, so there the cache is kept only with plain line hands.
static uint8_t *s_cache;   // NULL = no room: draw it all every tick
static bool s_stale = true;
// The seconds hand runs: the setting, unless quiet time or a low battery has it off.
static bool s_seconds;
static AppTimer *s_sweep;  // the sweeping seconds hand's next step
// The last tick: its second, and time_ms's ms then. The sweep counts on from
// there. The clock's seconds and its ms do not turn over together (the emulator
// is a third of a second apart), so the two read as one make the hand jump back.
static int s_tick_sec, s_tick_ms;
static AppTimer *s_hide;   // set while a double shake has the hands hidden
static uint32_t s_tap_ms;  // the last shake, in ms
static int s_min = -1;     // the minute s_stamp was taken in
static uint32_t s_stamp;

static void line(GContext *ctx, GPoint a, GPoint b, int width, GColor color) {
  graphics_context_set_stroke_color(ctx, theme(color));
  graphics_context_set_stroke_width(ctx, width);
  graphics_draw_line(ctx, a, b);
}

// A picked color stays as picked; without one (0) it is `own`, which follows the scheme.
static GColor picked(uint8_t argb, GColor own) {
  return argb ? fixed((GColor){ .argb = argb }) : own;
}

// Sport, chronograph, tachymeter and compass: a band the minute hand runs over.
static bool banded(void) {
  return g_settings.ring == RING_SPORT || g_settings.ring == RING_CHRONO || g_settings.ring == RING_TACHY ||
         g_settings.ring == RING_COMPASS;
}

// Per ring: the radius of the empty center the subdials fill, and how far each hand runs.
typedef struct { int16_t center, hour, minute, second; } Reach;

static Reach reach(void) {
  if (g_settings.ring == RING_MINIMAL) {
    // Minute hand to the ticks, the hour hand 10px short of them.
    int in = RING_OUT - MIN_TICK;
    return (Reach){ in - 4, in - 10, in, RING_OUT - MIN_TICK / 2 };
  }
  if (banded()) {
    // Hour hand just short of the band or its inset ring, the minute hand over the band, seconds to its outer edge.
    int in = RING_OUT - CHR_BAND - CHR_INSET;
    return (Reach){ in, in - 3, RING_OUT - 4, RING_OUT };
  }
  // Hour hand just short of the inner ring, the minute hand up to the inner edge of 12/3/6/9.
  return (Reach){ INNER_R, INNER_R - 8, NUM_R_BIG - NUM_BIG / 2, DIAL_R - 10 };
}

static void ring_fill(GContext *ctx, GPoint c, int r, int thickness, GColor color) {
  graphics_context_set_fill_color(ctx, theme(color));
  graphics_fill_radial(ctx, GRect(c.x - r, c.y - r, 2 * r + 1, 2 * r + 1), GOvalScaleModeFitCircle, thickness, 0, TRIG_MAX_ANGLE);
}

// Ticks only: light gray minutes, white hours.
static void draw_minimal(GContext *ctx, GPoint c) {
  ray_ticks(ctx, c, 0, 60, RING_OUT - MIN_TICK, RING_OUT, 1, GColorLightGray);
  ray_ticks(ctx, c, 0, 12, RING_OUT - MIN_TICK, RING_OUT, 3, GColorWhite);
}

// A white band, ticks for the half minutes, minutes and hours. Sport: the hour
// ticks and a ring inside the band in the accent, 00..55 along it, upright at 00
// and 30. Chronograph: one ink on the band only, the band out to where the accent ring
// would end, 1..12 upright. Its ticks are shorter and its numerals sit midway
// between them and the band's inner edge, clear of both: "10", the widest across
// its radius, spans about twice its cap height.
static void draw_band(GContext *ctx, GPoint c) {
  bool sport = g_settings.ring == RING_SPORT;
  static const uint8_t TICK[2][3] = { { 6, 5, 3 }, { 10, 7, 4 } };  // hour, minute, half; [sport]
  // Ticks and numerals are black or white, whichever reads on the band.
  GColor band = picked(g_settings.band_color, GColorWhite), ink = ink_on(band), hours = ink;
  ring_fill(ctx, c, RING_OUT, sport ? CHR_BAND : CHR_BAND + CHR_INSET, band);
  if (sport) {
    hours = picked(g_settings.second_color, ACCENT);
    ring_fill(ctx, c, RING_OUT - CHR_BAND, CHR_INSET, hours);
    // The accent is the band's own color: a mono scheme, or picked the same.
    if (gcolor_equal(theme(hours), theme(band))) {
      hours = ink;
    }
  }
  // The hours go over their minute ticks.
  ray_ticks(ctx, c, DEG(3), 60, RING_OUT - TICK[sport][2], RING_OUT - 1, 1, ink);
  ray_ticks(ctx, c, 0, 60, RING_OUT - TICK[sport][1], RING_OUT - 1, 1, ink);
  ray_ticks(ctx, c, 0, 12, RING_OUT - TICK[sport][0], RING_OUT - 1, 3, hours);
  char buf[3];
  for (int h = 1; h <= 12; h++) {
    snprintf(buf, sizeof(buf), sport ? "%02d" : "%d", sport ? h * 5 % 60 : h);
    if (sport) {
      text_draw_arc(ctx, buf, c, DEG(h * 30), CHR_NUM_R, CHR_NUM, ink);
    } else {
      text_draw(ctx, buf, polar(c, DEG(h * 30), RING_OUT - (TICK[0][0] + CHR_BAND + CHR_INSET) / 2), CHRONO_NUM, ink);
    }
  }
}

// Ticks for the seconds around the edge, the hours heavier. Inside them a band
// in the accent: 10..60 along it, a dot between each.
static void draw_tachy(GContext *ctx, GPoint c) {
  ray_ticks(ctx, c, 0, 60, RING_OUT - TACH_TICK, RING_OUT, 1, GColorLightGray);
  ray_ticks(ctx, c, 0, 12, RING_OUT - TACH_TICK, RING_OUT, 3, GColorWhite);
  GColor band = picked(g_settings.band_color, picked(g_settings.second_color, ACCENT)), ink = ink_on(band);
  int out = RING_OUT - TACH_TICK - 2;
  ring_fill(ctx, c, out, out - (RING_OUT - CHR_BAND - CHR_INSET), band);
  graphics_context_set_fill_color(ctx, theme(ink));
  char buf[3];
  for (int n = 10; n <= 60; n += 10) {
    snprintf(buf, sizeof(buf), "%d", n);
    text_draw_arc(ctx, buf, c, DEG(n * 6), TACH_NUM_R, CHR_NUM, ink);
    graphics_fill_circle(ctx, polar(c, DEG(n * 6 - 30), TACH_NUM_R), 2);
  }
}

// A compass bezel on chronograph's band: a tick every 5 degrees, the 30s heavier,
// N E S W upright at the quarters, the degrees (30, 60, 120 .. 330) along it
// between. It does not turn: a bezel, not a compass.
static void draw_compass(GContext *ctx, GPoint c) {
  GColor band = picked(g_settings.band_color, GColorWhite), ink = ink_on(band);
  ring_fill(ctx, c, RING_OUT, CHR_BAND + CHR_INSET, band);
  ray_ticks(ctx, c, 0, 72, RING_OUT - CMP_TICK + 3, RING_OUT - 1, 1, ink);
  ray_ticks(ctx, c, 0, 12, RING_OUT - CMP_TICK, RING_OUT - 1, 3, ink);
  int r = RING_OUT - (CMP_TICK + CHR_BAND + CHR_INSET) / 2;
  char buf[4];
  for (int h = 0; h < 12; h++) {
    if (h % 3 == 0) {
      snprintf(buf, sizeof(buf), "%c", "NESW"[h / 3]);
      text_draw(ctx, buf, polar(c, DEG(h * 30), r), CHRONO_NUM + 2, ink);
    } else {
      snprintf(buf, sizeof(buf), "%d", h * 30);
      text_draw_arc(ctx, buf, c, DEG(h * 30), r, CHR_NUM, ink);
    }
  }
}

static void draw_dial(GContext *ctx, GPoint c) {
  if (g_settings.ring == RING_MINIMAL) {
    return draw_minimal(ctx, c);
  }
  if (g_settings.ring == RING_COMPASS) {
    return draw_compass(ctx, c);
  }
  if (g_settings.ring == RING_TACHY) {
    return draw_tachy(ctx, c);
  }
  if (banded()) {
    return draw_band(ctx, c);
  }
  graphics_context_set_stroke_color(ctx, theme(GColorDarkGray));
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, c, INNER_R);

  for (int i = 0; i < 60; i++) {
    int32_t a = DEG(i * 6);
    bool hour = i % 5 == 0;
    line(ctx, polar(c, a, DIAL_R - 3), polar(c, a, DIAL_R - (hour ? 8 : 5)), 1,
         hour ? GColorLightGray : GColorDarkGray);
  }

  static const char *const ROMAN[] = { "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII" };
  char buf[3];
  for (int h = 1; h <= 12; h++) {
    int32_t a = DEG(h * 30);
    bool big = h % 3 == 0;
    if (big) {
      line(ctx, polar(c, a, DIAL_R + 1), polar(c, a, DIAL_R - 5), 3, picked(g_settings.second_color, ACCENT));
    }
    int r = big ? NUM_R_BIG : NUM_R, size = big ? NUM_BIG : NUM_SMALL;
    // Dark gray is too faint on white; the palette has nothing between it and black.
    GColor color = big || theme_light() ? GColorWhite : GColorLightGray;
    if (g_settings.ring == RING_ROMAN) {
      // Along the dial, feet to the center: upright, "VIII" would run into the ticks.
      text_draw_arc(ctx, ROMAN[h - 1], c, a, r, size, color);
    } else {
      snprintf(buf, sizeof(buf), "%d", h);
      text_draw(ctx, buf, polar(c, a, r), size, color);
    }
  }
  graphics_context_set_stroke_color(ctx, theme(GColorLightGray));
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, c, DIAL_R);

  graphics_context_set_stroke_color(ctx, theme(GColorDarkGray));
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, c, DIAL_R + 4);
}

// A step darker per channel: the dauphine hand's shaded facet.
static GColor shade(GColor c) {
  if (c.r) {
    c.r--;
  }
  if (c.g) {
    c.g--;
  }
  if (c.b) {
    c.b--;
  }
  return c;
}

// Hour or minute hand, `len` px long. `width` is the plain line's.
static void draw_hand(GContext *ctx, GPoint c, int32_t angle, int len, int width, GColor color) {
  if (g_settings.hands == HANDS_LINE) {
    line(ctx, c, polar(c, angle, len), width, color);
    return;
  }
  const int s = HAND_STEM, h = HAND_W / 2;
  if (g_settings.hands == HANDS_DAUPHINE) {
    // Widest HAND_W px out, a short tail under the pin. The whole kite shaded,
    // then one half over it: two halves side by side would leave a seam.
    const GPoint p[] = { { -h, 0 }, { HAND_W, -h - 1 }, { len, 0 }, { HAND_W, h + 1 } };
    ray_poly(ctx, c, angle, p, 4, shade(color));
    ray_poly(ctx, c, angle, p, 3, color);
    return;
  }
  line(ctx, c, polar(c, angle, HAND_STEM), HAND_STEM_W, color);
  if (g_settings.hands == HANDS_POINTER) {
    // The bar, its base corners cut, the last HAND_W px a point.
    const GPoint p[] = { { s, 2 - h }, { s + 2, -h }, { len - HAND_W, -h }, { len, 0 },
                         { len - HAND_W, h }, { s + 2, h }, { s, h - 2 } };
    ray_poly(ctx, c, angle, p, ARRAY_LENGTH(p), color);
    return;
  }
  if (g_settings.hands == HANDS_SWORD) {
    // A little wider than the bar, HAND_W px out of the stem; the tip is 2px, not a point.
    const GPoint p[] = { { s, 0 }, { s + HAND_W, -h - 1 }, { len, -1 }, { len, 1 }, { s + HAND_W, h + 1 } };
    ray_poly(ctx, c, angle, p, ARRAY_LENGTH(p), color);
    return;
  }
  ray_bar(ctx, c, angle, HAND_STEM, len, HAND_W, color);
  // Outline: the background over its inside, not see-through.
  if (g_settings.hands == HANDS_OUTLINE) {
    ray_bar(ctx, c, angle, HAND_STEM + HAND_EDGE, len - HAND_EDGE, HAND_W - 2 * HAND_EDGE, GColorBlack);
  }
}

// `sa`: the seconds hand's angle.
static void draw_hands(GContext *ctx, GPoint c, struct tm *t, int32_t sa) {
  int32_t ha = DEG((t->tm_hour % 12) * 30 + t->tm_min / 2);
  int32_t ma = DEG(t->tm_min * 6);
  GColor second = picked(g_settings.second_color, ACCENT);
  Reach r = reach();
  if (g_settings.hands != HANDS_NONE) {
    draw_hand(ctx, c, ha, r.hour, 6, picked(g_settings.hand_color, GColorWhite));
    // A 1px rim of background, so a white hand shows over the white band.
    static const int8_t RIM[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
    for (int i = 0; banded() && i < 4; i++) {
      draw_hand(ctx, GPoint(c.x + RIM[i][0], c.y + RIM[i][1]), ma, r.minute, 3, GColorBlack);
    }
    draw_hand(ctx, c, ma, r.minute, 3, picked(g_settings.minute_color, GColorWhite));
  } else if (!s_seconds) {
    return;  // nothing to pin
  }
  if (s_seconds) {
    line(ctx, polar(c, sa + DEG(180), DIAL_R * 25 / 100), polar(c, sa, r.second), 2, second);
  }
  graphics_context_set_fill_color(ctx, theme(second));
  graphics_fill_circle(ctx, c, 5);
  graphics_context_set_fill_color(ctx, theme(GColorBlack));
  graphics_fill_circle(ctx, c, 2);
}

// The frame buffer as (length, color) runs, none across a row's end, into `out`
// if there is one. Returns their size. Rows are clipped on round screens.
static int pack(GBitmap *fb, uint8_t *out) {
  int rows = gbitmap_get_bounds(fb).size.h, size = 0;
  for (int y = 0; y < rows; y++) {
    GBitmapDataRowInfo row = gbitmap_get_data_row_info(fb, y);
    for (int x = row.min_x; x <= row.max_x; size += 2) {
      int n = 1;
      while (n < 255 && x + n <= row.max_x && row.data[x + n] == row.data[x]) {
        n++;
      }
      if (out) {
        out[size] = n;
        out[size + 1] = row.data[x];
      }
      x += n;
    }
  }
  return size;
}

// The face just drawn into s_cache, if it leaves the hands their room.
static void save_cache(GContext *ctx) {
  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  if (!fb) {
    return;
  }
  GBitmapFormat format = gbitmap_get_format(fb);
  // Color screens: a byte per pixel.
  if (format == GBitmapFormat8Bit || format == GBitmapFormat8BitCircular) {
    GSize screen = gbitmap_get_bounds(fb).size;
    bool lines = g_settings.hands == HANDS_LINE || g_settings.hands == HANDS_NONE;
    int size = pack(fb, NULL);
    if ((int)heap_bytes_free() > size + CACHE_HEADROOM + (lines ? 0 : screen.w * screen.h)) {
      s_cache = malloc(size);
    }
    if (s_cache) {
      pack(fb, s_cache);
    }
  }
  graphics_release_frame_buffer(ctx, fb);
}

// s_cache back over the whole frame buffer.
static void draw_cache(GContext *ctx) {
  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  if (!fb) {
    return;
  }
  const uint8_t *p = s_cache;
  int rows = gbitmap_get_bounds(fb).size.h;
  for (int y = 0; y < rows; y++) {
    GBitmapDataRowInfo row = gbitmap_get_data_row_info(fb, y);
    for (int x = row.min_x; x <= row.max_x; p += 2) {
      memset(row.data + x, p[1], p[0]);
      x += p[0];
    }
  }
  graphics_release_frame_buffer(ctx, fb);
}

static void draw_face(GContext *ctx, GRect b, GPoint c) {
  graphics_context_set_fill_color(ctx, theme(GColorBlack));
  graphics_fill_rect(ctx, b, 0, GCornerNone);

  // Corner complications: the outer side is pinned at 10/2/4/8 o'clock and
  // each runs COMP_SPAN toward 12/6. a0 is the pinned end.
  static const int16_t SIDE[SLOT_POS_COUNT] = { 300, 60, 120, 240 };  // TL TR BR BL
  static const int8_t DIR[SLOT_POS_COUNT] = { 1, -1, 1, -1 };
  for (int i = 0; i < SLOT_POS_COUNT; i++) {
    Slot s = {
      .center = c, .radius = COMP_R, .thickness = COMP_T,
      .a0 = DEG(SIDE[i]),
      .a1 = DEG(SIDE[i] + DIR[i] * COMP_SPAN),
    };
    // Text and the time zone need their slot's string; the rest draw from what they measure.
    if (g_settings.slots[i] == COMP_CUSTOM) {
      comp_custom_draw(ctx, &s, g_settings.slot_text[i]);
    } else if (g_settings.slots[i] == COMP_ZONE) {
      comp_zone_draw(ctx, &s, g_settings.zone[i], g_settings.slot_text[i]);
    } else {
      complication_draw(g_settings.slots[i], ctx, &s);
    }
  }

  draw_dial(ctx, c);
  draw_zoom(c, reach().center * 100 / INNER_R);
  center_draw(ctx, c);
  draw_zoom(c, 100);
}

// Every shown complication's stamp in one. The hour is in it: the moon moves by
// it, and a face nothing else changes is still drawn fresh 24 times a day.
static uint32_t face_stamp(const struct tm *t) {
  uint32_t stamp = t->tm_hour + 1;
  for (int i = 0; i < SLOT_POS_COUNT; i++) {
    stamp = stamp * 31 + complication_stamp(g_settings.slots[i], t);
  }
  for (int i = 0; i < CENTER_POS_COUNT; i++) {
    stamp = stamp * 31 + complication_stamp(g_settings.center[i], t);
  }
  return stamp;
}

static void update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  GPoint c = grect_center_point(&b);
  graphics_context_set_antialiased(ctx, true);
  time_t now;
  uint16_t ms;
  time_ms(&now, &ms);
  struct tm t = *localtime(&now);  // copied: complications call localtime too
  // Once a minute, in seconds mode too: health is asked no more often.
  if (s_min != t.tm_min) {
    s_min = t.tm_min;
    uint32_t stamp = face_stamp(&t);
    s_stale = s_stale || stamp != s_stamp;
    s_stamp = stamp;
  }
  if (s_cache && !s_stale) {
    draw_cache(ctx);
  } else {
    free(s_cache);
    s_cache = NULL;
    draw_face(ctx, b, c);
    save_cache(ctx);
    s_stale = false;
  }
  // It sweeps over the cached face only: a face drawn whole 8 times a second would stall the watch.
  int32_t sa = DEG(t.tm_sec * 6);
  if (g_settings.sweep && s_cache) {
    sa = DEG(s_tick_sec * 6) + DEG(6) * ((ms - s_tick_ms + 1000) % 1000) / 1000;
  }
  if (!s_hide) {
    draw_hands(ctx, c, &t, sa);
  }
}

static bool seconds_wanted(void) {
  if (!g_settings.seconds || quiet_time_is_active()) {
    return false;
  }
  BatteryChargeState b = battery_state_service_peek();
  return b.is_plugged || b.charge_percent > LOW_BATTERY;
}

static void subscribe_ticks(void);

// Quiet time and the battery are looked at once a minute, with or without seconds.
static void tick_handler(struct tm *t, TimeUnits changed) {
  uint16_t ms;
  time_ms(NULL, &ms);
  s_tick_sec = t->tm_sec;
  s_tick_ms = ms;
  if ((changed & MINUTE_UNIT) && seconds_wanted() != s_seconds) {
    subscribe_ticks();
  }
  layer_mark_dirty(s_layer);
}

static void sweep_step(void *data) {
  s_sweep = app_timer_register(SWEEP_MS, sweep_step, NULL);
  if (s_cache && !s_hide) {
    layer_mark_dirty(s_layer);
  }
}

static void hide_end(void *data) {
  s_hide = NULL;
  layer_mark_dirty(s_layer);
}

// Two shakes in a row hide the hands for a while, pin and seconds too: the subdials show whole.
static void tap_handler(AccelAxisType axis, int32_t direction) {
  time_t now;
  uint16_t ms;
  time_ms(&now, &ms);
  uint32_t at = (uint32_t)now * 1000 + ms, gap = at - s_tap_ms;
  if (gap < SHAKE_MIN_MS) {
    return;
  }
  s_tap_ms = at;
  if (gap > SHAKE_MAX_MS) {
    return;
  }
  // A third shake keeps them away longer.
  if (s_hide) {
    app_timer_cancel(s_hide);
  }
  s_hide = app_timer_register(SHAKE_HIDE_MS, hide_end, NULL);
  layer_mark_dirty(s_layer);
}

static void subscribe_shake(void) {
  if (g_settings.shake_hide) {
    accel_tap_service_subscribe(tap_handler);
  } else {
    accel_tap_service_unsubscribe();
  }
}

static void subscribe_ticks(void) {
  s_seconds = seconds_wanted();
  tick_timer_service_subscribe(s_seconds ? SECOND_UNIT : MINUTE_UNIT, tick_handler);
  // Quiet time and a low battery stop the sweep with the seconds hand.
  if (s_sweep) {
    app_timer_cancel(s_sweep);
    s_sweep = NULL;
  }
  if (s_seconds && g_settings.sweep) {
    s_sweep = app_timer_register(SWEEP_MS, sweep_step, NULL);
  }
}

// Every corner and subdial, in g_settings.zone order: its complication, its
// COMP_CUSTOM text or COMP_ZONE name, and its COMP_ZONE offset.
#define PLACE(key, text_key, zone_key, id, text) \
  { MESSAGE_KEY_##key, MESSAGE_KEY_##text_key, MESSAGE_KEY_##zone_key, &g_settings.id, g_settings.text, sizeof(g_settings.text) }

static void inbox_received(DictionaryIterator *iter, void *context) {
  if (!weather_handle_message(iter) && !api_handle_message(iter)) {
    const struct { uint32_t key, text_key, zone_key; uint8_t *id; char *text; size_t size; } places[] = {
      PLACE(SLOT_TL, TEXT_TL, ZONE_TL, slots[SLOT_POS_TL], slot_text[SLOT_POS_TL]),
      PLACE(SLOT_TR, TEXT_TR, ZONE_TR, slots[SLOT_POS_TR], slot_text[SLOT_POS_TR]),
      PLACE(SLOT_BR, TEXT_BR, ZONE_BR, slots[SLOT_POS_BR], slot_text[SLOT_POS_BR]),
      PLACE(SLOT_BL, TEXT_BL, ZONE_BL, slots[SLOT_POS_BL], slot_text[SLOT_POS_BL]),
      PLACE(CENTER_T, TEXT_T, ZONE_T, center[CENTER_POS_T], center_text[CENTER_POS_T]),
      PLACE(CENTER_L, TEXT_L, ZONE_L, center[CENTER_POS_L], center_text[CENTER_POS_L]),
      PLACE(CENTER_R, TEXT_R, ZONE_R, center[CENTER_POS_R], center_text[CENTER_POS_R]),
      PLACE(CENTER_B, TEXT_B, ZONE_B, center[CENTER_POS_B], center_text[CENTER_POS_B]),
    };
    Tuple *t;
    for (unsigned i = 0; i < ARRAY_LENGTH(places); i++) {
      if ((t = dict_find(iter, places[i].key))) {
        *places[i].id = clamp_i32(t->value->int32, 0, COMP_COUNT - 1);
      }
      if ((t = dict_find(iter, places[i].text_key)) && t->type == TUPLE_CSTRING) {
        strncpy(places[i].text, t->value->cstring, places[i].size - 1);
      }
      if ((t = dict_find(iter, places[i].zone_key))) {
        g_settings.zone[i] = t->value->int32;
      }
    }
    if ((t = dict_find(iter, MESSAGE_KEY_SECONDS))) {
      g_settings.seconds = t->value->int32;
    }
    if ((t = dict_find(iter, MESSAGE_KEY_SWEEP))) {
      g_settings.sweep = t->value->int32;
    }
    if ((t = dict_find(iter, MESSAGE_KEY_SHAKE_HIDE))) {
      g_settings.shake_hide = t->value->int32;
    }
    if ((t = dict_find(iter, MESSAGE_KEY_UNITS))) {
      g_settings.imperial = t->value->int32;
    }
    if ((t = dict_find(iter, MESSAGE_KEY_STEP_GOAL))) {
      g_settings.step_goal = t->value->int32;
    }
    if ((t = dict_find(iter, MESSAGE_KEY_SCHEME))) {
      g_settings.scheme = clamp_i32(t->value->int32, 0, 4);
    }
    if ((t = dict_find(iter, MESSAGE_KEY_BG_COLOR))) {
      g_settings.bg = t->value->int32 | 0xC0;  // opaque
    }
    if ((t = dict_find(iter, MESSAGE_KEY_ACCENT_COLOR))) {
      g_settings.accent = t->value->int32 | 0xC0;
    }
    if ((t = dict_find(iter, MESSAGE_KEY_HANDS))) {
      g_settings.hands = clamp_i32(t->value->int32, 0, HANDS_COUNT - 1);
    }
    if ((t = dict_find(iter, MESSAGE_KEY_RING))) {
      g_settings.ring = clamp_i32(t->value->int32, 0, RING_COUNT - 1);
    }
    // 0 stays 0: the scheme's color.
    if ((t = dict_find(iter, MESSAGE_KEY_HAND_COLOR))) {
      g_settings.hand_color = t->value->int32 ? t->value->int32 | 0xC0 : 0;
    }
    if ((t = dict_find(iter, MESSAGE_KEY_MINUTE_COLOR))) {
      g_settings.minute_color = t->value->int32 ? t->value->int32 | 0xC0 : 0;
    }
    if ((t = dict_find(iter, MESSAGE_KEY_SECOND_COLOR))) {
      g_settings.second_color = t->value->int32 ? t->value->int32 | 0xC0 : 0;
    }
    if ((t = dict_find(iter, MESSAGE_KEY_BAND_COLOR))) {
      g_settings.band_color = t->value->int32 ? t->value->int32 | 0xC0 : 0;
    }
    persist_write_data(PK_SETTINGS, &g_settings, sizeof(g_settings));
    subscribe_ticks();
    subscribe_shake();
  }
  s_stale = true;
  layer_mark_dirty(s_layer);
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect b = layer_get_bounds(root);
  draw_init(b.size);
  s_layer = layer_create(b);
  layer_set_update_proc(s_layer, update_proc);
  layer_add_child(root, s_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_layer);
  free(s_cache);
  s_cache = NULL;
  draw_deinit();
}

int main(void) {
  if (persist_exists(PK_SETTINGS)) {
    persist_read_data(PK_SETTINGS, &g_settings, sizeof(g_settings));
  }
  weather_init();
  api_init();

  s_window = window_create();
  window_set_background_color(s_window, theme(GColorBlack));
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  subscribe_ticks();
  subscribe_shake();
  app_message_register_inbox_received(inbox_received);
  app_message_open(512, 64);  // settings with all 8 texts: ~440 bytes

  app_event_loop();
  tick_timer_service_unsubscribe();
  accel_tap_service_unsubscribe();
  window_destroy(s_window);
}
