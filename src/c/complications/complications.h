#pragma once
#include "../draw.h"

// Every complication is one draw function in its own file, handed the Slot
// (arc geometry) it lives in. It draws its gauge with the slot_* helpers and
// places labels with slot_point(), so any complication fits any corner.
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
  COMP_CALENDAR,
  COMP_AQI,
  COMP_HEART,
  COMP_DISTANCE,
  COMP_ELEVATION,
  COMP_UV,
  COMP_WEATHER,   // subdial only
  COMP_HUMIDITY,
  COMP_CUSTOM,    // the user's text
  COMP_API,       // first of API_MAX custom API complications, see api.c
  COMP_API_LAST = COMP_API + API_MAX - 1,
  COMP_SUN,       // sunrise and sunset
  COMP_BEAT,      // Swatch .beat time
  COMP_WIND,
  COMP_CALENDAR_PLAIN, // subdial only: no page behind it
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
  COMP_COUNT
} ComplicationId;

void complication_draw(ComplicationId id, GContext *ctx, const Slot *s);

// Label layout for corner complications (tune per screen).
#if defined(PBL_PLATFORM_GABBRO)
#define COMP_TEXT   11   // cap height of value labels
#define COMP_ICON   12   // icon size
#else
#define COMP_TEXT   8
#define COMP_ICON   18
#define COMP_THUMB  14   // radial offset of the label beside the arc; gabbro has none
#endif
#define COMP_GAP    4    // px between an arc end and its label/icon

#define COMP_TRACK  GColorOxfordBlue

void comp_steps_draw(GContext *ctx, const Slot *s);
void comp_temp_draw(GContext *ctx, const Slot *s);
void comp_battery_draw(GContext *ctx, const Slot *s);
void comp_rain_draw(GContext *ctx, const Slot *s);
void comp_calendar_draw(GContext *ctx, const Slot *s);
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
void comp_custom_draw(GContext *ctx, const Slot *s, const char *txt);  // not in the tables: needs its text
void comp_zone_draw(GContext *ctx, const Slot *s, int offset, const char *name);  // nor this: minutes from UTC, and its name
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
#else
#define SUB_D      31
#define SUB_R      16
#define SUB_TEXT   8
#define SUB_SMALL  6
#define SUB_LOW    10
#endif
#define SUB_T      3

// Ring open at the bottom, filling clockwise from 8 o'clock.
Slot center_ring(GPoint c);
void center_gauge(GContext *ctx, GPoint c, int pct, GColor fill);

void center_temp_draw(GContext *ctx, GPoint c);
void center_battery_draw(GContext *ctx, GPoint c);
void center_rain_draw(GContext *ctx, GPoint c);
void center_calendar_draw(GContext *ctx, GPoint c);
void center_calendar_plain_draw(GContext *ctx, GPoint c);
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
void center_sleep_draw(GContext *ctx, GPoint c);
void center_active_draw(GContext *ctx, GPoint c);
void center_moon_draw(GContext *ctx, GPoint c);
void center_location_draw(GContext *ctx, GPoint c);
void center_custom_draw(GContext *ctx, GPoint c, const char *txt);
void center_zone_draw(GContext *ctx, GPoint c, int offset, const char *name);
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
// The same index as a range gauge: the bands' colors along the arc, the thumb
// where `v` falls in its band, no min and max. `top` ends the last band.
void comp_band_gauge_draw(GContext *ctx, const Slot *s, int v, const Band *bands, int top, const char *caption);
void center_band_gauge_draw(GContext *ctx, GPoint c, int v, const Band *bands, int top, const char *caption);
// No bar: an icon and a text (heart, elevation, wind).
void comp_icon_text(GContext *ctx, const Slot *s, const char *icon, GColor color, const char *txt);
// Label along the arc at an end (0 or 100); trims that end of *s to make room.
void comp_end_label(GContext *ctx, Slot *s, int end, const char *txt);
// Gabbro trims the left end of *s for the icon; call it before the label.
void comp_icon(GContext *ctx, Slot *s, const char *icon, GColor color);
