#include "complications.h"
#include "../weather.h"

// WHO bands of a UV index.
static const Band BANDS[] = {
  { 2, GColorGreenARGB8 }, { 5, GColorYellowARGB8 }, { 7, GColorOrangeARGB8 },
  { 10, GColorRedARGB8 }, { INT16_MAX, GColorPurpleARGB8 },
};

// A section per WHO band, laid out like the AQI.
void comp_uv_draw(GContext *ctx, const Slot *s) {
  comp_band_draw(ctx, s, g_weather.valid ? g_weather.uv : -1, BANDS, "UV");
}

// UV index: its WHO bands around the ring.
void center_uv_draw(GContext *ctx, GPoint c) {
  center_band_draw(ctx, c, g_weather.valid ? g_weather.uv : -1, BANDS, "UV");
}

// The same bands as a range gauge.
void comp_uv_gauge_draw(GContext *ctx, const Slot *s) {
  comp_band_gauge_draw(ctx, s, g_weather.valid ? g_weather.uv : -1, BANDS, 11, "UV");
}

void center_uv_gauge_draw(GContext *ctx, GPoint c) {
  center_band_gauge_draw(ctx, c, g_weather.valid ? g_weather.uv : -1, BANDS, 11, "UV");
}
