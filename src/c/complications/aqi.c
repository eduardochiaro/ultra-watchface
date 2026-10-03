#include "complications.h"
#include "../weather.h"

// EPA bands of a US AQI.
static const Band BANDS[] = {
  { 50, GColorGreenARGB8 }, { 100, GColorYellowARGB8 }, { 150, GColorOrangeARGB8 },
  { 200, GColorRedARGB8 }, { 300, GColorPurpleARGB8 }, { INT16_MAX, GColorDarkCandyAppleRedARGB8 },
};

// A section per band, "AQI" at their end; the value curved outside the middle.
// Gabbro: "AQI 42 SECTIONS".
void comp_aqi_draw(GContext *ctx, const Slot *s) {
  comp_band_draw(ctx, s, g_weather.valid ? g_weather.aqi : -1, BANDS, "AQI");
}

// US AQI: its EPA bands around the ring.
void center_aqi_draw(GContext *ctx, GPoint c) {
  center_band_draw(ctx, c, g_weather.valid ? g_weather.aqi : -1, BANDS, "AQI");
}

// The same bands as a range gauge.
void comp_aqi_gauge_draw(GContext *ctx, const Slot *s) {
  comp_band_gauge_draw(ctx, s, g_weather.valid ? g_weather.aqi : -1, BANDS, 500, "AQI");
}

void center_aqi_gauge_draw(GContext *ctx, GPoint c) {
  center_band_gauge_draw(ctx, c, g_weather.valid ? g_weather.aqi : -1, BANDS, 500, "AQI");
}
