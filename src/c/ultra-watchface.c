#include <pebble.h>
#include "draw.h"
#include "settings.h"
#include "weather.h"
#include "complications/complications.h"

// Dial geometry, tuned per screen.
#if defined(PBL_PLATFORM_GABBRO)   // 260x260 round: corner arcs hug the edge
#define DIAL_R     88
#define NUM_R_BIG  69   // 12/3/6/9
#define NUM_R      72   // the other numerals
#define NUM_BIG    16
#define NUM_SMALL  10
#define INNER_R    51   // ring around the sun gauge
#define COMP_R     118
#define COMP_T     6
#define SUN_R      30
#define SUN_T      7
#else                              // emery 200x228: dial fills the width, arcs in the corners
#define DIAL_R     94
#define NUM_R_BIG  77
#define NUM_R      69
#define NUM_BIG    14
#define NUM_SMALL  9
#define INNER_R    54
#define COMP_R     108
#define COMP_T     6
#define SUN_R      32
#define SUN_T      7
#endif

#define COMP_SPAN  45   // degrees per corner complication, labels included
#define ACCENT     GColorChromeYellow
#define DEG(d)     DEG_TO_TRIGANGLE(d)

#define PK_SETTINGS 10

Settings g_settings = {
  .slots = { COMP_STEPS, COMP_TEMP, COMP_RAIN, COMP_BATTERY },
  .seconds = false,
  .step_goal = 10000,
};

static Window *s_window;
static Layer *s_layer;

static GPoint polar(GPoint c, int32_t angle, int r) {
  return GPoint(c.x + sin_lookup(angle) * r / TRIG_MAX_RATIO,
                c.y - cos_lookup(angle) * r / TRIG_MAX_RATIO);
}

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
              big || (g_settings.scheme & SCHEME_LIGHT) ? GColorWhite : GColorLightGray);
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

static void update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  GPoint c = grect_center_point(&b);
  graphics_context_set_antialiased(ctx, true);
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
    complication_draw(g_settings.slots[i], ctx, &s);
  }

  draw_dial(ctx, c);

  Slot sun = { .center = c, .radius = SUN_R, .thickness = SUN_T,
               .a0 = DEG(-90), .a1 = DEG(90) };
  complication_draw(COMP_SUN, ctx, &sun);

  time_t now = time(NULL);
  draw_hands(ctx, c, localtime(&now));
}

static void tick_handler(struct tm *t, TimeUnits changed) {
  layer_mark_dirty(s_layer);
}

static void subscribe_ticks(void) {
  tick_timer_service_subscribe(g_settings.seconds ? SECOND_UNIT : MINUTE_UNIT, tick_handler);
}

// The settings page sends ints; older saves sent selects as strings.
static int32_t tuple_int(Tuple *t) {
  return t->type == TUPLE_CSTRING ? atoi(t->value->cstring) : t->value->int32;
}

static void inbox_received(DictionaryIterator *iter, void *context) {
  if (!weather_handle_message(iter)) {
    const uint32_t slot_keys[SLOT_POS_COUNT] = {
      MESSAGE_KEY_SLOT_TL, MESSAGE_KEY_SLOT_TR, MESSAGE_KEY_SLOT_BR, MESSAGE_KEY_SLOT_BL,
    };
    for (int i = 0; i < SLOT_POS_COUNT; i++) {
      Tuple *t = dict_find(iter, slot_keys[i]);
      if (t) g_settings.slots[i] = clamp_i32(tuple_int(t), 0, COMP_COUNT - 1);
    }
    Tuple *t;
    if ((t = dict_find(iter, MESSAGE_KEY_SECONDS))) g_settings.seconds = tuple_int(t);
    if ((t = dict_find(iter, MESSAGE_KEY_STEP_GOAL))) g_settings.step_goal = tuple_int(t);
    if ((t = dict_find(iter, MESSAGE_KEY_SCHEME))) g_settings.scheme = clamp_i32(tuple_int(t), 0, 3);
    persist_write_data(PK_SETTINGS, &g_settings, sizeof(g_settings));
    subscribe_ticks();
  }
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
  app_message_open(256, 64);

  app_event_loop();
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}
