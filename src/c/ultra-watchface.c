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
#define COMP_R     118
#define COMP_T     6
#else                              // emery 200x228: dial fills the width, arcs in the corners
#define DIAL_R     94
#define NUM_R_BIG  77
#define NUM_R      69
#define NUM_BIG    14
#define NUM_SMALL  9
#define INNER_R    54
#define COMP_R     108
#define COMP_T     6
#endif

#define COMP_SPAN  45   // degrees per corner complication, labels included
#define ACCENT     GColorChromeYellow
#define DEG(d)     DEG_TO_TRIGANGLE(d)

#define PK_SETTINGS 10
#define CACHE_HEADROOM 40000  // bytes left for fctx after the seconds cache

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
// Seconds mode: the face minus the hands, drawn once a minute and copied in
// every second. Text is the costly part to draw. NULL = no memory, draw it all.
static GBitmap *s_cache;
static int s_cache_min = -1;  // -1 = stale

static void line(GContext *ctx, GPoint a, GPoint b, int width, GColor color) {
  graphics_context_set_stroke_color(ctx, theme(color));
  graphics_context_set_stroke_width(ctx, width);
  graphics_draw_line(ctx, a, b);
}

static void draw_dial(GContext *ctx, GPoint c) {
  graphics_context_set_stroke_color(ctx, theme(GColorDarkGray));
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, c, INNER_R);

  for (int i = 0; i < 60; i++) {
    int32_t a = DEG(i * 6);
    bool hour = i % 5 == 0;
    line(ctx, polar(c, a, DIAL_R - 3), polar(c, a, DIAL_R - (hour ? 8 : 5)), 1,
         hour ? GColorLightGray : GColorDarkGray);
  }

  char buf[3];
  for (int h = 1; h <= 12; h++) {
    int32_t a = DEG(h * 30);
    bool big = h % 3 == 0;
    if (big) line(ctx, polar(c, a, DIAL_R + 1), polar(c, a, DIAL_R - 5), 3, ACCENT);
    snprintf(buf, sizeof(buf), "%d", h);
    text_draw(ctx, buf, polar(c, a, big ? NUM_R_BIG : NUM_R), big ? NUM_BIG : NUM_SMALL,
              // Dark gray is too faint on white; the palette has nothing between it and black.
              big || theme_light() ? GColorWhite : GColorLightGray);
  }
  graphics_context_set_stroke_color(ctx, theme(GColorLightGray));
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, c, DIAL_R);

  graphics_context_set_stroke_color(ctx, theme(GColorDarkGray));
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, c, DIAL_R + 4);
}

static void draw_hands(GContext *ctx, GPoint c, struct tm *t) {
  int32_t ha = DEG((t->tm_hour % 12) * 30 + t->tm_min / 2);
  int32_t ma = DEG(t->tm_min * 6);
  line(ctx, c, polar(c, ha, DIAL_R * 38 / 100), 6, GColorWhite);
  line(ctx, c, polar(c, ma, DIAL_R * 65 / 100), 3, GColorWhite);
  if (g_settings.seconds) {
    int32_t sa = DEG(t->tm_sec * 6);
    line(ctx, polar(c, sa + DEG(180), DIAL_R * 25 / 100), polar(c, sa, DIAL_R - 10), 2, ACCENT);
  }
  graphics_context_set_fill_color(ctx, theme(ACCENT));
  graphics_fill_circle(ctx, c, 5);
  graphics_context_set_fill_color(ctx, theme(GColorBlack));
  graphics_fill_circle(ctx, c, 2);
}

static void save_cache(GContext *ctx) {
  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  if (!fb) return;
  GRect r = gbitmap_get_bounds(fb);
  // fctx allocates while drawing text: leave it room. Emery keeps ~48KB with
  // the cache; gabbro's 67KB cache would leave ~26KB and crash it.
  // ponytail: gabbro gets no cache; cache only the corners if it needs one.
  if (!s_cache && (int)heap_bytes_free() > r.size.w * r.size.h + CACHE_HEADROOM)
    s_cache = gbitmap_create_blank(r.size, gbitmap_get_format(fb));
  // 8-bit formats: a byte per pixel. Rows are clipped on round screens.
  for (int y = 0; s_cache && y < r.size.h; y++) {
    GBitmapDataRowInfo src = gbitmap_get_data_row_info(fb, y), dst = gbitmap_get_data_row_info(s_cache, y);
    memcpy(dst.data + src.min_x, src.data + src.min_x, src.max_x - src.min_x + 1);
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
    // Text needs its slot's string; the rest draw from what they measure.
    if (g_settings.slots[i] == COMP_CUSTOM) comp_custom_draw(ctx, &s, g_settings.slot_text[i]);
    else complication_draw(g_settings.slots[i], ctx, &s);
  }

  draw_dial(ctx, c);
  center_draw(ctx, c);
}

static void update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  GPoint c = grect_center_point(&b);
  graphics_context_set_antialiased(ctx, true);
  time_t now = time(NULL);
  struct tm t = *localtime(&now);  // copied: complications call localtime too
  if (s_cache && s_cache_min == t.tm_min) {
    graphics_draw_bitmap_in_rect(ctx, s_cache, b);
  } else {
    draw_face(ctx, b, c);
    if (g_settings.seconds) {
      save_cache(ctx);
      s_cache_min = t.tm_min;
    }
  }
  draw_hands(ctx, c, &t);
}

static void tick_handler(struct tm *t, TimeUnits changed) {
  layer_mark_dirty(s_layer);
}

static void subscribe_ticks(void) {
  s_cache_min = -1;
  if (!g_settings.seconds && s_cache) {
    gbitmap_destroy(s_cache);
    s_cache = NULL;
  }
  tick_timer_service_subscribe(g_settings.seconds ? SECOND_UNIT : MINUTE_UNIT, tick_handler);
}

// Every corner and subdial: its complication and its COMP_CUSTOM text.
#define PLACE(key, text_key, id, text) \
  { MESSAGE_KEY_##key, MESSAGE_KEY_##text_key, &g_settings.id, g_settings.text, sizeof(g_settings.text) }

static void inbox_received(DictionaryIterator *iter, void *context) {
  if (!weather_handle_message(iter)) {
    const struct { uint32_t key, text_key; uint8_t *id; char *text; size_t size; } places[] = {
      PLACE(SLOT_TL, TEXT_TL, slots[SLOT_POS_TL], slot_text[SLOT_POS_TL]),
      PLACE(SLOT_TR, TEXT_TR, slots[SLOT_POS_TR], slot_text[SLOT_POS_TR]),
      PLACE(SLOT_BR, TEXT_BR, slots[SLOT_POS_BR], slot_text[SLOT_POS_BR]),
      PLACE(SLOT_BL, TEXT_BL, slots[SLOT_POS_BL], slot_text[SLOT_POS_BL]),
      PLACE(CENTER_T, TEXT_T, center[CENTER_POS_T], center_text[CENTER_POS_T]),
      PLACE(CENTER_L, TEXT_L, center[CENTER_POS_L], center_text[CENTER_POS_L]),
      PLACE(CENTER_R, TEXT_R, center[CENTER_POS_R], center_text[CENTER_POS_R]),
      PLACE(CENTER_B, TEXT_B, center[CENTER_POS_B], center_text[CENTER_POS_B]),
    };
    Tuple *t;
    for (unsigned i = 0; i < ARRAY_LENGTH(places); i++) {
      if ((t = dict_find(iter, places[i].key))) *places[i].id = clamp_i32(t->value->int32, 0, COMP_COUNT - 1);
      if ((t = dict_find(iter, places[i].text_key)) && t->type == TUPLE_CSTRING)
        strncpy(places[i].text, t->value->cstring, places[i].size - 1);
    }
    if ((t = dict_find(iter, MESSAGE_KEY_SECONDS))) g_settings.seconds = t->value->int32;
    if ((t = dict_find(iter, MESSAGE_KEY_UNITS))) g_settings.imperial = t->value->int32;
    if ((t = dict_find(iter, MESSAGE_KEY_STEP_GOAL))) g_settings.step_goal = t->value->int32;
    if ((t = dict_find(iter, MESSAGE_KEY_SCHEME))) g_settings.scheme = clamp_i32(t->value->int32, 0, 4);
    if ((t = dict_find(iter, MESSAGE_KEY_BG_COLOR))) g_settings.bg = t->value->int32 | 0xC0;  // opaque
    if ((t = dict_find(iter, MESSAGE_KEY_ACCENT_COLOR))) g_settings.accent = t->value->int32 | 0xC0;
    persist_write_data(PK_SETTINGS, &g_settings, sizeof(g_settings));
    subscribe_ticks();
  }
  s_cache_min = -1;
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
  if (s_cache) gbitmap_destroy(s_cache);
  s_cache = NULL;
  draw_deinit();
}

int main(void) {
  if (persist_exists(PK_SETTINGS)) persist_read_data(PK_SETTINGS, &g_settings, sizeof(g_settings));
  weather_init();

  s_window = window_create();
  window_set_background_color(s_window, theme(GColorBlack));
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  subscribe_ticks();
  app_message_register_inbox_received(inbox_received);
  app_message_open(512, 64);  // settings with all 8 texts: ~290 bytes

  app_event_loop();
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}
