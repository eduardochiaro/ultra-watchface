#pragma once
#include <pebble.h>

// The hour before the fetch's, it, and 7 more (weather.js): the hourly rain's
// 6 sections, -1h to +4h, still fill 3 hours on from a fetch.
#define RAIN_HOURS 9

// Last Open-Meteo reading pushed by the phone (src/pkjs/weather.js), already in
// the configured units. Cached to persist storage so a reboot shows real data.
typedef struct {
  bool valid;
  int16_t temp, temp_min, temp_max;
  int16_t rain;              // today's max precipitation probability, %
  int16_t aqi;               // US AQI now, -1 = unknown
  int16_t uv;                // UV index now, -1 = unknown
  int16_t humidity;          // relative humidity now, %, -1 = unknown
  int16_t condition;         // ICON_SUN.. order (weather.js CONDITION), -1 = unknown
  int16_t elevation;         // ground at the phone's position, m
  int16_t sunrise, sunset;   // today's, minutes after local midnight; equal = unknown
  int16_t wind;              // speed now, km/h (the watch converts), -1 = unknown
  int16_t wind_dir;          // where it blows from, degrees clockwise from north
  // Appended: empty on older saves.
  char place[17];            // the city at the phone's position, "" = unknown
  char place_code[5];        // a short code for it, for the subdial
  // Appended: 0 on older saves, no forecast.
  int32_t rain_start;        // the hour rain_hours[0] is for, hours since the epoch
  uint8_t rain_hours[RAIN_HOURS]; // precipitation probability per hour, %
  // Appended: 0 on older saves, in the old size's padding.
  int16_t pressure;          // at sea level now, hPa (the watch converts), <= 0 = unknown
  // Appended: FEELS_UNKNOWN on older saves.
  int16_t feels;             // apparent temperature now
} Weather;

#define FEELS_UNKNOWN INT16_MIN

extern Weather g_weather;

// The chance of rain `dh` hours from this one, %; -1 = unknown.
int weather_rain_hour(int dh);

// The icon glyph (ICON_SUN.. in draw.h) for the conditions now, NULL if unknown.
const char *weather_icon(void);
// Their name, short, in the face's language: "CLOUDY", "PT CLOUDY". Only when weather_icon() has one.
const char *weather_name(void);

void weather_init(void);
// True when the message carried weather (and g_weather was updated).
bool weather_handle_message(DictionaryIterator *iter);
