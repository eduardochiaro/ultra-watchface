// Settings model, shared by index.js, weather.js and the settings page
// (scripts/inline-config.mjs inlines this file into config.html, so keep it
// plain ES5 with no requires).

var SETTINGS_KEY = 'ultra-settings';  // last saved page result

// Values match ComplicationId in src/c/complications/complications.h.
var COMPLICATIONS = [
  { label: 'Steps', value: 1 },
  { label: 'Temperature', value: 2 },
  { label: 'Battery', value: 3 },
  { label: 'Chance of rain', value: 4 },
  { label: 'Humidity', value: 13 },
  { label: 'Calendar', value: 6 },
  { label: 'Heart rate', value: 8 },
  { label: 'Distance', value: 9 },
  { label: 'Air quality', value: 7 },
  { label: 'UV index', value: 11 },
  { label: 'Elevation', value: 10 },
  { label: 'Text', value: 14 },
  { label: 'None', value: 0 }
];

// Subdials have their own designs, so their own list.
var CENTER_COMPLICATIONS = [
  { label: 'Temperature', value: 2 },
  { label: 'Chance of rain', value: 4 },
  { label: 'Humidity', value: 13 },
  { label: 'Air quality', value: 7 },
  { label: 'UV index', value: 11 },
  { label: 'Weather', value: 12 },
  { label: 'Calendar', value: 6 },
  { label: 'Battery', value: 3 },
  { label: 'Heart rate', value: 8 },
  { label: 'Distance', value: 9 },
  { label: 'Elevation', value: 10 },
  { label: 'Text', value: 14 },
  { label: 'None', value: 0 }
];

var CORNERS = [
  { key: 'SLOT_TL', label: 'Top left' },
  { key: 'SLOT_TR', label: 'Top right' },
  { key: 'SLOT_BL', label: 'Bottom left' },
  { key: 'SLOT_BR', label: 'Bottom right' }
];

var CENTERS = [
  { key: 'CENTER_T', label: 'Top' },
  { key: 'CENTER_L', label: 'Left' },
  { key: 'CENTER_R', label: 'Right' },
  { key: 'CENTER_B', label: 'Bottom' }
];

// Index = SCHEME value, see SCHEME_LIGHT / SCHEME_MONO / SCHEME_ACCENT in src/c/settings.h
var SCHEMES = ['Black', 'White', 'White on black', 'Black on white', 'Accent'];
var SCHEME_ACCENT = 4;

// Mirrors g_settings in C. weather.js asks Open-Meteo for the chosen
// temperature unit; the watch converts distance and elevation. Colors are GColor8 argb (0xC0 = black,
// 0xF8 = chrome yellow).
var DEFAULTS = { SLOT_TL: 1, SLOT_TR: 2, SLOT_BL: 3, SLOT_BR: 4, CENTER_T: 2, CENTER_L: 4, CENTER_R: 7, CENTER_B: 6, SCHEME: 0, UNITS: 0, STEP_GOAL: 10000, SECONDS: 0,
  BG_COLOR: 0xC0, ACCENT_COLOR: 0xF8,
  // The Text complication's text, per place: TEXT_ + the SLOT_/CENTER_ suffix.
  TEXT_TL: '', TEXT_TR: '', TEXT_BL: '', TEXT_BR: '', TEXT_T: '', TEXT_L: '', TEXT_R: '', TEXT_B: '' };

// Only characters the watch font has. Corners fit 12, subdials 4 (TEXT_T etc.).
function cleanText(k, s) {
  return String(s).replace(/[^ %,\-./0-9:A-Za-z]/g, '').slice(0, k.length === 6 ? 4 : 12);
}

function value(k, v) {
  return typeof DEFAULTS[k] === 'string' ? cleanText(k, v) : Number(v);
}

function withDefaults(saved) {
  var s = {};
  for (var k in DEFAULTS) s[k] = saved && saved[k] !== undefined ? value(k, saved[k]) : DEFAULTS[k];
  return s;
}

function savedSettings() {
  try {
    return withDefaults(JSON.parse(localStorage.getItem(SETTINGS_KEY)));
  } catch (e) {
    return withDefaults();
  }
}

function toMessage(settings) {
  var msg = {};
  for (var k in DEFAULTS) msg[k] = value(k, settings[k]);
  return msg;
}

if (typeof module === 'object') {
  module.exports = {
    SETTINGS_KEY: SETTINGS_KEY, COMPLICATIONS: COMPLICATIONS, CENTER_COMPLICATIONS: CENTER_COMPLICATIONS,
    CORNERS: CORNERS, CENTERS: CENTERS, SCHEMES: SCHEMES, SCHEME_ACCENT: SCHEME_ACCENT,
    DEFAULTS: DEFAULTS, cleanText: cleanText, withDefaults: withDefaults, savedSettings: savedSettings, toMessage: toMessage
  };
}
