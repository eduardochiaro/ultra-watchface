#include "complications.h"
#include "../weather.h"

// WHO bands of a UV index.
static const Band BANDS[] = {
  { 2, GColorGreenARGB8 }, { 5, GColorYellowARGB8 }, { 7, GColorOrangeARGB8 },
  { 10, GColorRedARGB8 }, { INT16_MAX, GColorPurpleARGB8 },
};

// 0..11 along the bar in the WHO band's color, laid out like the AQI.
void comp_uv_draw(GContext *ctx, const Slot *s) {
  comp_band_draw(ctx, s, g_weather.valid ? g_weather.uv : -1, 11, BANDS, "UV");
}

// UV index, 0..11 around the ring in its WHO band color.
void center_uv_draw(GContext *ctx, GPoint c) {
  center_band_draw(ctx, c, g_weather.valid ? g_weather.uv : -1, 11, BANDS, "UV");
}
