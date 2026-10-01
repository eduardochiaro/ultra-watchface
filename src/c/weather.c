#include "weather.h"

#define PK_WEATHER 25  // bumped when Weather changes shape

Weather g_weather;

void weather_init(void) {
  persist_read_data(PK_WEATHER, &g_weather, sizeof(g_weather));
}

bool weather_handle_message(DictionaryIterator *iter) {
  Tuple *t = dict_find(iter, MESSAGE_KEY_TEMP);
  if (!t) return false;
  g_weather.temp = t->value->int32;
  const struct { uint32_t key; int16_t *dst; } fields[] = {
    { MESSAGE_KEY_TEMP_MIN, &g_weather.temp_min },
    { MESSAGE_KEY_TEMP_MAX, &g_weather.temp_max },
    { MESSAGE_KEY_RAIN,     &g_weather.rain },
    { MESSAGE_KEY_AQI,      &g_weather.aqi },
    { MESSAGE_KEY_UV,       &g_weather.uv },
    { MESSAGE_KEY_HUMIDITY, &g_weather.humidity },
    { MESSAGE_KEY_CONDITION, &g_weather.condition },
    { MESSAGE_KEY_ELEVATION, &g_weather.elevation },
  };
  for (unsigned i = 0; i < ARRAY_LENGTH(fields); i++) {
    Tuple *f = dict_find(iter, fields[i].key);
    if (f) *fields[i].dst = f->value->int32;
  }
  g_weather.valid = true;
  persist_write_data(PK_WEATHER, &g_weather, sizeof(g_weather));
  return true;
}
