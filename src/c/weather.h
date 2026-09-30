#pragma once
#include <pebble.h>

// Last Open-Meteo reading pushed by the phone (src/pkjs/weather.js), already in
// the configured units. Cached to persist storage so a reboot shows real data.
typedef struct {
  bool valid;
  int16_t temp, temp_min, temp_max;
  int16_t rain;              // today's max precipitation probability, %
  int16_t sunrise, sunset;   // today, minutes since local midnight
} Weather;

extern Weather g_weather;

void weather_init(void);
// True when the message carried weather (and g_weather was updated).
bool weather_handle_message(DictionaryIterator *iter);
