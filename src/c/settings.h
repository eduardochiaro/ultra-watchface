#pragma once
#include <pebble.h>

// Corner slots, clockwise from top-left. Same order as the SLOT_* message keys.
typedef enum { SLOT_POS_TL, SLOT_POS_TR, SLOT_POS_BR, SLOT_POS_BL, SLOT_POS_COUNT } SlotPos;
// Subdials, same order as the CENTER_* message keys.
typedef enum { CENTER_POS_T, CENTER_POS_L, CENTER_POS_R, CENTER_POS_B, CENTER_POS_COUNT } CenterPos;

// Color scheme bits. 0 black, 1 white, 2 white on black, 3 black on white.
// SCHEME_ACCENT is a value, not a bit: custom background + accent colors.
enum { SCHEME_LIGHT = 1, SCHEME_MONO = 2, SCHEME_ACCENT = 4 };

typedef struct {
  uint8_t slots[SLOT_POS_COUNT];   // ComplicationId per corner
  bool seconds;
  int32_t step_goal;
  uint8_t scheme;                  // appended: older saves read without it
  uint8_t bg, accent;              // GColor8 argb, used by SCHEME_ACCENT
  // Appended. imperial lands on older saves' trailing pad byte, which was 0.
  bool imperial;                   // distance and elevation units
  uint8_t center[CENTER_POS_COUNT]; // ComplicationId per subdial
  // COMP_CUSTOM text per corner and subdial; font glyphs only (config.js cleans it).
  char slot_text[SLOT_POS_COUNT][13];
  char center_text[CENTER_POS_COUNT][5];  // a monogram: 4 at most
} Settings;

extern Settings g_settings;
