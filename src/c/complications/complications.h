#pragma once
#include "../draw.h"

// Every complication is one draw function in its own file, handed the Slot
// (arc geometry) it lives in. It draws its gauge with the slot_* helpers and
// places labels with slot_point() and slot_mid(), so any complication fits any
// corner, or corners joined.
//
// Adding one: write complications/<name>.c, declare it below, add it to the
// enum and the table in complications.c, and add an option to COMPLICATIONS in
// src/pkjs/config.js with the same value.

#define API_MAX 8  // custom API complications; API_MAX in src/pkjs/config.js

// Values are persisted and sent by the config page: append only.
typedef enum {
  COMP_NONE = 0,
  COMP_STEPS,
  COMP_TEMP,
  COMP_BATTERY,
  COMP_RAIN,
  COMP_RETIRED_5, // was the sun gauge
  COMP_CALENDAR,  // the date, in the place's DateFormat
  COMP_AQI,
  COMP_HEART,
  COMP_DISTANCE,
  COMP_ELEVATION,
  COMP_UV,
  COMP_WEATHER,   // the conditions' icon; in a corner their name too
  COMP_HUMIDITY,
  COMP_CUSTOM,    // the user's text
  COMP_API,       // first of API_MAX custom API complications, see api.c
  COMP_API_LAST = COMP_API + API_MAX - 1,
  COMP_SUN,       // sunrise and sunset
  COMP_BEAT,      // Swatch .beat time
  COMP_WIND,
  COMP_CALENDAR_PLAIN, // retired: older saves, a subdial's DATE_WEEKDAY
  COMP_AQI_GAUGE, // the AQI and UV as range gauges, not sections
  COMP_UV_GAUGE,
  COMP_CALORIES,  // active and resting
  COMP_CALORIES_ACTIVE,
  COMP_TIME,      // digital time
  COMP_ZONE,      // the same in another time zone, one per place
  COMP_SLEEP,     // slept last night
  COMP_ACTIVE,    // active minutes today
  COMP_MOON,      // moon phase
  COMP_LOCATION,  // the city the phone is in
  COMP_WEEK,      // the week's days, lit up to today
  COMP_YEAR,      // how much of the year is gone
  COMP_RAIN_HOURLY, // the chance of rain by the hour, -1h to +4h
  COMP_WEATHER_TEMP, // COMP_WEATHER and the temperature
  COMP_COUNTDOWN, // days to or since the place's day
  COMP_MONTH,     // how much of the month is gone
  COMP_DAY,       // how much of the day is gone
  COMP_PRESSURE,  // sea-level pressure
  COMP_WEEK_PROGRESS, // how much of the week is gone, from g_settings.week_start
  COMP_TEMP_NOW,  // the temperature alone, large
  COMP_FEELS,     // what it feels like, large, the temperature smaller beside it
  COMP_HOURS,     // subdials only: the time's parts, "09"; _PLAIN: "9"
  COMP_HOURS_PLAIN,
  COMP_MINUTES,
  COMP_MINUTES_PLAIN,
  COMP_SECONDS,   // drawn over the cached face, see center_seconds_draw
  COMP_SECONDS_PLAIN,
  COMP_AMPM,      // subdials only: AM or PM, in either clock style
  COMP_COUNT
} ComplicationId;

// COMP_CALENDAR's formats, one per place, kept in g_settings.zone. Sent by the
// config page (DATE_FORMATS in src/pkjs/config.js): append only.
typedef enum {
  DATE_CALENDAR,  // boxes in a corner, a tear-off page in a subdial
  DATE_WEEKDAY,   // TUE 6
  DATE_MONTH,     // OCT 6
  DATE_FULL,      // TUE 6 OCT
  DATE_NUMERIC,   // 6/10/2026, the year under it in a subdial
  DATE_WEEK,      // WEEK 41
  DATE_YEARDAY,   // DAY 279
  DATE_YEAR,      // 2026
  DATE_COUNT
} DateFormat;

void complication_draw(ComplicationId id, GContext *ctx, const Slot *s);
// What its drawing hangs on that no message brings: the time, health, the
// battery. The face is drawn again only when a shown one's stamp changes.
uint32_t complication_stamp(ComplicationId id, const struct tm *t);

// Label layout for corner complications (tune per screen).
#if defined(PBL_PLATFORM_GABBRO)
#define COMP_TEXT   11   // cap height of value labels
#define COMP_ICON   12   // icon size
#elif defined(PBL_PLATFORM_EMERY)
#define COMP_TEXT   8
#define COMP_ICON   18
#define COMP_THUMB  14   // radial offset of the label beside the arc; gabbro has none
#else                    // lite: no corners
#define COMP_TEXT   7
#define COMP_ICON   13
#define COMP_THUMB  10
#endif
#define COMP_GAP    4    // px between an arc end and its label/icon

#define COMP_TRACK  GColorOxfordBlue

void comp_steps_draw(GContext *ctx, const Slot *s);
void comp_temp_draw(GContext *ctx, const Slot *s);
void comp_temp_now_draw(GContext *ctx, const Slot *s);
void comp_feels_draw(GContext *ctx, const Slot *s);
void comp_battery_draw(GContext *ctx, const Slot *s);
void comp_rain_draw(GContext *ctx, const Slot *s);
void comp_heart_draw(GContext *ctx, const Slot *s);
void comp_distance_draw(GContext *ctx, const Slot *s);
void comp_aqi_draw(GContext *ctx, const Slot *s);
void comp_elevation_draw(GContext *ctx, const Slot *s);
void comp_uv_draw(GContext *ctx, const Slot *s);
void comp_humidity_draw(GContext *ctx, const Slot *s);
void comp_sun_draw(GContext *ctx, const Slot *s);
void comp_beat_draw(GContext *ctx, const Slot *s);
void comp_wind_draw(GContext *ctx, const Slot *s);
void comp_aqi_gauge_draw(GContext *ctx, const Slot *s);
void comp_uv_gauge_draw(GContext *ctx, const Slot *s);
void comp_calories_draw(GContext *ctx, const Slot *s);
void comp_calories_active_draw(GContext *ctx, const Slot *s);
void comp_time_draw(GContext *ctx, const Slot *s);
void comp_sleep_draw(GContext *ctx, const Slot *s);
void comp_active_draw(GContext *ctx, const Slot *s);
void comp_moon_draw(GContext *ctx, const Slot *s);
void comp_location_draw(GContext *ctx, const Slot *s);
void comp_week_draw(GContext *ctx, const Slot *s);
void comp_year_draw(GContext *ctx, const Slot *s);
void comp_month_draw(GContext *ctx, const Slot *s);
void comp_day_draw(GContext *ctx, const Slot *s);
void comp_week_progress_draw(GContext *ctx, const Slot *s);
void comp_pressure_draw(GContext *ctx, const Slot *s);
void comp_rain_hourly_draw(GContext *ctx, const Slot *s);
void comp_weather_draw(GContext *ctx, const Slot *s);
void comp_weather_temp_draw(GContext *ctx, const Slot *s);
void comp_custom_draw(GContext *ctx, const Slot *s, const char *txt);  // not in the tables: needs its text
void comp_zone_draw(GContext *ctx, const Slot *s, int offset, const char *name);  // nor this: minutes from UTC, and its name
void comp_date_draw(GContext *ctx, const Slot *s, int format);  // nor this: a DateFormat
void comp_countdown_draw(GContext *ctx, const Slot *s, int day, const char *name);  // nor this: days since the epoch, and its name
void comp_api_draw(GContext *ctx, const Slot *s, int i);  // i: 0..API_MAX-1

// Custom API complications: what the phone last sent for each (src/pkjs/api.js).
void api_init(void);
// True when the message carried one.
bool api_handle_message(DictionaryIterator *iter);

int step_pct(void);  // today's steps toward the goal, in %

// The four subdials inside the dial ring, around center `c`, as picked in
// g_settings.center. Each id has its own subdial design, not the corner one:
// center_<name>_draw, next to comp_<name>_draw in <name>.c.
void center_draw(GContext *ctx, GPoint c);

// Subdial layout (tune per screen).
#if defined(PBL_PLATFORM_GABBRO)
#define SUB_D      35   // subdial center from the face center
#define SUB_R      17   // ring centerline
#define SUB_TEXT   9    // value
#define SUB_SMALL  7    // caption under it
#define SUB_LOW    11   // caption's y below the subdial center, in the ring's gap
#define SUB_T      3
#elif defined(PBL_PLATFORM_EMERY)
#define SUB_D      31
#define SUB_R      16
#define SUB_TEXT   8
#define SUB_SMALL  6
#define SUB_LOW    10
#define SUB_T      3
#elif defined(PBL_PLATFORM_CHALK)  // emery's at 88%
#define SUB_D      27
#define SUB_R      14
#define SUB_TEXT   7
#define SUB_SMALL  6
#define SUB_LOW    9
#define SUB_T      3
#else                              // 144x168: emery's at 70%
#define SUB_D      22
#define SUB_R      11
#define SUB_TEXT   6
#define SUB_SMALL  5
#define SUB_LOW    7
#define SUB_T      2
#endif

// Ring open at the bottom, filling clockwise from 8 o'clock.
Slot center_ring(GPoint c);
void center_gauge(GContext *ctx, GPoint c, int pct, GColor fill);

void center_temp_draw(GContext *ctx, GPoint c);
void center_temp_now_draw(GContext *ctx, GPoint c);
void center_feels_draw(GContext *ctx, GPoint c);
void center_battery_draw(GContext *ctx, GPoint c);
void center_rain_draw(GContext *ctx, GPoint c);
void center_aqi_draw(GContext *ctx, GPoint c);
void center_heart_draw(GContext *ctx, GPoint c);
void center_distance_draw(GContext *ctx, GPoint c);
void center_elevation_draw(GContext *ctx, GPoint c);
void center_uv_draw(GContext *ctx, GPoint c);
void center_humidity_draw(GContext *ctx, GPoint c);
void center_sun_draw(GContext *ctx, GPoint c);
void center_beat_draw(GContext *ctx, GPoint c);
void center_wind_draw(GContext *ctx, GPoint c);
void center_aqi_gauge_draw(GContext *ctx, GPoint c);
void center_uv_gauge_draw(GContext *ctx, GPoint c);
void center_calories_draw(GContext *ctx, GPoint c);
void center_calories_active_draw(GContext *ctx, GPoint c);
void center_time_draw(GContext *ctx, GPoint c);
void center_hours_draw(GContext *ctx, GPoint c);
void center_hours_plain_draw(GContext *ctx, GPoint c);
void center_minutes_draw(GContext *ctx, GPoint c);
void center_minutes_plain_draw(GContext *ctx, GPoint c);
void center_ampm_draw(GContext *ctx, GPoint c);
// Whether a subdial shows the seconds.
bool center_has_seconds(void);
// They change every tick, the cached face doesn't: drawn over it, in `color`,
// "--" unless `running`. Under the same draw_zoom as center_draw.
void center_seconds_draw(GContext *ctx, GPoint c, int sec, bool running, GColor color);
void center_sleep_draw(GContext *ctx, GPoint c);
void center_active_draw(GContext *ctx, GPoint c);
void center_moon_draw(GContext *ctx, GPoint c);
void center_location_draw(GContext *ctx, GPoint c);
void center_week_draw(GContext *ctx, GPoint c);
void center_year_draw(GContext *ctx, GPoint c);
void center_month_draw(GContext *ctx, GPoint c);
void center_day_draw(GContext *ctx, GPoint c);
void center_week_progress_draw(GContext *ctx, GPoint c);
void center_pressure_draw(GContext *ctx, GPoint c);
void center_rain_hourly_draw(GContext *ctx, GPoint c);
void center_weather_draw(GContext *ctx, GPoint c);
void center_weather_temp_draw(GContext *ctx, GPoint c);
void center_custom_draw(GContext *ctx, GPoint c, const char *txt);
void center_zone_draw(GContext *ctx, GPoint c, int offset, const char *name);
void center_date_draw(GContext *ctx, GPoint c, int format);
void center_countdown_draw(GContext *ctx, GPoint c, int day, const char *name);
void center_api_draw(GContext *ctx, GPoint c, int i);
// `size`, or the largest below it at which `txt` fits `width` px.
int comp_fit(GContext *ctx, const char *txt, int size, int width);
// Text centered on c, `size` at most, shrunk to fit `width`.
void center_fit_text(GContext *ctx, const char *txt, GPoint c, int size, int width, GColor color);

// Corner layout: the slot is the whole complication. Gauges put a label at
// both ends, bars one on the left; labels eat into the arc, not past it.
// Icons sit outside the arc, toward the corner; on gabbro they lead the bar,
// "ICON 30% BAR". An `icon` may also be a short name (API complications),
// drawn as text.
void comp_fill_gauge(GContext *ctx, const Slot *s, int pct, GColor fill, GColor track,
                     const char *label, const char *icon);  // icon NULL: none
// Range gauge (temp, API gauge): min and max at the ends, a thumb at pct, the
// value by the arc's middle, where an icon would be, shrunk to fit the slot.
// pct < 0: unknown, a bare track. Gabbro: no gauge, only the value on the arc's middle.
// `fill` is the arc's color: `n` GColor8 argb shades spread evenly, min to max.
void comp_range_draw(GContext *ctx, const Slot *s, int pct, const char *min, const char *max,
                     const char *value, const uint8_t *fill, int n);
// Same in a subdial: min and max under the value. `name` (or NULL) goes above it.
void center_range_draw(GContext *ctx, GPoint c, int pct, const char *min, const char *max,
                       const char *value, const char *name, const uint8_t *fill, int n);
// A banded index (AQI, UV): a section per band, lit in their own colors up to
// the band the value `v` is in, with a caption and the value; all unlit and
// "--" for unknown (v < 0). `to` is a band's upper bound, the last one INT16_MAX.
typedef struct { int16_t to; uint8_t argb; } Band;
void comp_band_draw(GContext *ctx, const Slot *s, int v, const Band *bands, const char *caption);
void center_band_draw(GContext *ctx, GPoint c, int v, const Band *bands, const char *caption);
// `n` sections along *s, 2px apart, in GColor8 `argb`, 0 for the track; the
// first at the 100 end if `flip`. Section `mark` (or -1) is drawn 2px thicker.
void comp_sections(GContext *ctx, const Slot *s, bool flip, const uint8_t *argb, int n, int mark);
// The same index as a range gauge: the bands' colors along the arc, the thumb
// where `v` falls in its band, no min and max. `top` ends the last band.
void comp_band_gauge_draw(GContext *ctx, const Slot *s, int v, const Band *bands, int top, const char *caption);
void center_band_gauge_draw(GContext *ctx, GPoint c, int v, const Band *bands, int top, const char *caption);
// No bar: an icon and a text (heart, elevation, wind).
void comp_icon_text(GContext *ctx, const Slot *s, const char *icon, GColor color, const char *txt);
// Label along the arc at an end (0 or 100); trims that end of *s to make room.
void comp_end_label(GContext *ctx, Slot *s, int end, const char *txt);
// The same at any size and color.
void comp_label(GContext *ctx, Slot *s, int end, const char *txt, int size, GColor color);
// Gabbro trims the left end of *s for the icon; call it before the label.
void comp_icon(GContext *ctx, Slot *s, const char *icon, GColor color);
