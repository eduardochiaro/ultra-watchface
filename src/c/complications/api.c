#include "complications.h"

// Custom API complications. The phone fetches each one's URL and fills in its
// patterns (src/pkjs/api.js); the watch keeps the result and draws it.

#define PK_API 40  // + index: a key each, all of them would not fit one

enum { API_TEXT, API_BAR, API_GAUGE };  // API_TYPES in src/pkjs/config.js

typedef struct {
  uint8_t type;
  int8_t pct;           // bar and gauge: the value between min and max, -1 = unknown
  char name[5];
  char text[13];
  char min[7], max[7];  // gauge end labels
} Api;

static Api s_api[API_MAX];

void api_init(void) {
  for (int i = 0; i < API_MAX; i++) persist_read_data(PK_API + i, &s_api[i], sizeof(Api));
}

bool api_handle_message(DictionaryIterator *iter) {
  Tuple *t = dict_find(iter, MESSAGE_KEY_API_INDEX);
  if (!t) return false;
  int i = t->value->int32;
  if (i < 0 || i >= API_MAX) return true;
  Api *a = &s_api[i];
  if ((t = dict_find(iter, MESSAGE_KEY_API_TYPE))) a->type = t->value->int32;
  if ((t = dict_find(iter, MESSAGE_KEY_API_PCT))) a->pct = clamp_i32(t->value->int32, -1, 100);
  const struct { uint32_t key; char *dst; size_t size; } texts[] = {
    { MESSAGE_KEY_API_NAME, a->name, sizeof(a->name) },
    { MESSAGE_KEY_API_TEXT, a->text, sizeof(a->text) },
    { MESSAGE_KEY_API_MIN,  a->min,  sizeof(a->min) },
    { MESSAGE_KEY_API_MAX,  a->max,  sizeof(a->max) },
  };
  for (unsigned n = 0; n < ARRAY_LENGTH(texts); n++) {
    if ((t = dict_find(iter, texts[n].key)) && t->type == TUPLE_CSTRING)
      strncpy(texts[n].dst, t->value->cstring, texts[n].size - 1);  // the last byte stays 0
  }
  persist_write_data(PK_API + i, a, sizeof(Api));
  return true;
}

static const char *api_text(const Api *a) {
  return a->text[0] ? a->text : "--";  // nothing received yet
}

// Text: like heart rate. Bar: like rain. Gauge: like temperature. The name
// goes where their icons do; the gauge has its value there, so "NAME value".
void comp_api_draw(GContext *ctx, const Slot *s, int i) {
  const Api *a = &s_api[i];
  if (a->type == API_GAUGE) {
    char buf[sizeof(a->name) + sizeof(a->text)];
    snprintf(buf, sizeof(buf), "%s%s%s", a->name, a->name[0] ? " " : "", api_text(a));
    comp_range_draw(ctx, s, a->pct, a->min, a->max, buf);
  }
  else if (a->type == API_BAR) comp_fill_gauge(ctx, s, a->pct, GColorChromeYellow, COMP_TRACK, api_text(a), a->name);
  else comp_icon_text(ctx, s, a->name, GColorChromeYellow, api_text(a));
}

void center_api_draw(GContext *ctx, GPoint c, int i) {
  const Api *a = &s_api[i];
  int inner = 2 * SUB_R - SUB_T - 4;  // inside the ring
  if (a->type == API_GAUGE) {
    center_range_draw(ctx, c, a->pct, a->min, a->max, api_text(a), a->name);
  } else if (a->type == API_BAR) {
    // The value in the ring, the name in its gap.
    center_gauge(ctx, c, a->pct, GColorChromeYellow);
    center_fit_text(ctx, api_text(a), GPoint(c.x, c.y - 1), SUB_TEXT + 2, inner, GColorWhite);
    text_draw(ctx, a->name, GPoint(c.x, c.y + SUB_LOW), SUB_SMALL, GColorWhite);
  } else {
    // No ring: the name as a caption over the value.
    text_draw(ctx, a->name, GPoint(c.x, c.y - SUB_R + 5), SUB_SMALL, GColorChromeYellow);
    center_fit_text(ctx, api_text(a), GPoint(c.x, c.y + 2), SUB_TEXT + 2, 2 * SUB_R, GColorWhite);
  }
}
