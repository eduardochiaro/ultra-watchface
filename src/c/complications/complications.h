#pragma once
#include "../draw.h"

// Every complication is one draw function in its own file, handed the Slot
// (arc geometry) it lives in. It draws its gauge with the slot_* helpers and
// places labels with slot_point(), so any complication fits any corner.
//
// Adding one: write complications/<name>.c, declare it below, add it to the
// enum and the table in complications.c, and add an option to COMPLICATIONS in
// src/pkjs/config.js with the same value.

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
void comp_custom_draw(GContext *ctx, const Slot *s, const char *txt);  // not in the tables: needs its text

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
void center_aqi_draw(GContext *ctx, GPoint c);
void center_heart_draw(GContext *ctx, GPoint c);
void center_distance_draw(GContext *ctx, GPoint c);
void center_elevation_draw(GContext *ctx, GPoint c);
void center_uv_draw(GContext *ctx, GPoint c);
void center_humidity_draw(GContext *ctx, GPoint c);
void center_custom_draw(GContext *ctx, GPoint c, const char *txt);

// Corner layout: the slot is the whole complication. Gauges put a label at
// both ends, bars one on the left; labels eat into the arc, not past it.
// Icons sit outside the arc, toward the corner; on gabbro they lead the bar,
// "ICON 30% BAR".
void comp_fill_gauge(GContext *ctx, const Slot *s, int pct, GColor fill, GColor track,
                     const char *label, const char *icon);  // icon NULL: none
// A bar with a caption and a value (AQI, UV).
void comp_value_gauge(GContext *ctx, const Slot *s, int pct, GColor fill, const char *caption,
                      const char *value);
// No bar: an icon and a text (heart, elevation).
void comp_icon_text(GContext *ctx, const Slot *s, const char *icon, GColor color, const char *txt);
// Label along the arc at an end (0 or 100); trims that end of *s to make room.
void comp_end_label(GContext *ctx, Slot *s, int end, const char *txt);
// Gabbro trims the left end of *s for the icon; call it before the label.
void comp_icon(GContext *ctx, Slot *s, const char *icon, GColor color);
