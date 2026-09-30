#pragma once
#include <pebble.h>

// Corner slots, clockwise from top-left. Same order as the SLOT_* message keys.
typedef enum { SLOT_POS_TL, SLOT_POS_TR, SLOT_POS_BR, SLOT_POS_BL, SLOT_POS_COUNT } SlotPos;

typedef struct {
  uint8_t slots[SLOT_POS_COUNT];   // ComplicationId per corner
  bool seconds;
  int32_t step_goal;
} Settings;

extern Settings g_settings;
