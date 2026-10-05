#include "complications.h"
#include "../weather.h"

// Where the phone is, as it last said with the weather (src/pkjs/weather.js):
// the city along the corner, a short code for it across the subdial. Text only,
// like the Text complication.
void comp_location_draw(GContext *ctx, const Slot *s) {
  comp_custom_draw(ctx, s, g_weather.place[0] ? g_weather.place : "--");
}

void center_location_draw(GContext *ctx, GPoint c) {
  center_custom_draw(ctx, c, g_weather.place_code[0] ? g_weather.place_code : "--");
}
