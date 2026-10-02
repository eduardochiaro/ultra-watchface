#include "complications.h"
#include "../weather.h"

// EPA bands of a US AQI.
static const Band BANDS[] = {
  { 50, GColorGreenARGB8 }, { 100, GColorYellowARGB8 }, { 150, GColorOrangeARGB8 },
  { 200, GColorRedARGB8 }, { 300, GColorPurpleARGB8 }, { INT16_MAX, GColorBulgarianRoseARGB8 },
};

// 0..300 along the bar in the band's color, "AQI" at its end; the value curved
// outside its middle, where the battery puts its percentage. Gabbro: "AQI 42 BAR".
void comp_aqi_draw(GContext *ctx, const Slot *s) {
  comp_band_draw(ctx, s, g_weather.valid ? g_weather.aqi : -1, 300, BANDS, "AQI");
}

// US AQI, 0..300 around the ring in the color of its EPA band.
void center_aqi_draw(GContext *ctx, GPoint c) {
  center_band_draw(ctx, c, g_weather.valid ? g_weather.aqi : -1, 300, BANDS, "AQI");
}
