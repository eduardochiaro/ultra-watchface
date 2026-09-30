#pragma once
#include <pebble.h>

// Corner slots, clockwise from top-left. Same order as the SLOT_* message keys.
typedef enum { SLOT_POS_TL, SLOT_POS_TR, SLOT_POS_BR, SLOT_POS_BL, SLOT_POS_COUNT } SlotPos;

// Color scheme bits. 0 black, 1 white, 2 white on black, 3 black on white.
enum { SCHEME_LIGHT = 1, SCHEME_MONO = 2 };

typedef struct {
  uint8_t slots[SLOT_POS_COUNT];   // ComplicationId per corner
  bool seconds;
  int32_t step_goal;
  uint8_t scheme;                  // appended: older saves read without it
} Settings;

extern Settings g_settings;
