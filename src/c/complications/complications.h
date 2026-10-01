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
  COMP_COUNT
} ComplicationId;

void complication_draw(ComplicationId id, GContext *ctx, const Slot *s);

// Label layout for corner complications (tune per screen).
#if defined(PBL_PLATFORM_GABBRO)
#define COMP_TEXT   11   // cap height of value labels
#define COMP_ICON   12   // icon size
#define COMP_THUMB  (-17) // radial offset of the label next to a thumb
#else
#define COMP_TEXT   8
#define COMP_ICON   18
#define COMP_THUMB  14
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

int heart_bpm(void);  // 0 = no reading
// Walked today, "3.9" in km or mi, and the step goal's progress in %.
void distance_text(char *buf, size_t n);
int step_pct(void);
GColor aqi_color(int v);  // EPA band of a US AQI; white for unknown (<0)
// Ground elevation in the unit setting, no unit; false and "--" before any weather.
bool elevation_text(char *buf, size_t n);

// The four subdials inside the dial ring, around center `c`, as picked in
// g_settings.center. Each id has its own subdial design, not the corner one.
void center_draw(GContext *ctx, GPoint c);

// Corner layout: the slot is the whole complication. Gauges put a label at
// both ends, bars one on the left; labels eat into the arc, not past it.
// Icons sit outside the arc, toward the corner.
void comp_fill_gauge(GContext *ctx, const Slot *s, int pct, GColor fill, GColor track,
                     const char *label, const char *icon);  // icon NULL: none
// Label along the arc at an end (0 or 100); trims that end of *s to make room.
void comp_end_label(GContext *ctx, Slot *s, int end, const char *txt);
void comp_icon(GContext *ctx, const Slot *s, const char *icon, GColor color);
