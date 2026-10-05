// Settings model, shared by index.js, weather.js and the settings page
// (scripts/inline-config.mjs inlines this file into config.html, so keep it
// plain ES5 with no requires).

var SETTINGS_KEY = 'ultra-settings';  // last saved page result

// Values match ComplicationId in src/c/complications/complications.h. `group`
// is its <optgroup> in the pickers; None has none and stays last.
var COMPLICATIONS = [
  { group: 'Activity', label: 'Steps', value: 1 },
  { group: 'Activity', label: 'Distance', value: 9 },
  { group: 'Activity', label: 'Calories (total)', value: 29 },
  { group: 'Activity', label: 'Calories (active)', value: 30 },
  { group: 'Activity', label: 'Active minutes', value: 34 },
  { group: 'Activity', label: 'Heart rate', value: 8 },
  { group: 'Activity', label: 'Sleep', value: 33 },
  { group: 'Weather', label: 'Temperature', value: 2 },
  { group: 'Weather', label: 'Chance of rain', value: 4 },
  { group: 'Weather', label: 'Humidity', value: 13 },
  { group: 'Weather', label: 'Wind', value: 25 },
  { group: 'Weather', label: 'Air quality', value: 7 },
  { group: 'Weather', label: 'Air quality (gauge)', value: 27 },
  { group: 'Weather', label: 'UV index', value: 11 },
  { group: 'Weather', label: 'UV index (gauge)', value: 28 },
  { group: 'Time and date', label: 'Digital time', value: 31 },
  { group: 'Time and date', label: 'Time zone', value: 32 },
  { group: 'Time and date', label: '.beat time', value: 24 },
  { group: 'Time and date', label: 'Calendar', value: 6 },
  { group: 'Sun and moon', label: 'Sunrise / sunset', value: 23 },
  { group: 'Sun and moon', label: 'Moon phase', value: 35 },
  { group: 'Place', label: 'Location', value: 36 },
  { group: 'Place', label: 'Elevation', value: 10 },
  { group: 'Watch', label: 'Battery', value: 3 },
  { group: 'Watch', label: 'Text', value: 14 },
  { label: 'None', value: 0 }
];

// Subdials have their own designs, so their own list.
var CENTER_COMPLICATIONS = [
  { group: 'Activity', label: 'Distance', value: 9 },
  { group: 'Activity', label: 'Calories (total)', value: 29 },
  { group: 'Activity', label: 'Calories (active)', value: 30 },
  { group: 'Activity', label: 'Active minutes', value: 34 },
  { group: 'Activity', label: 'Heart rate', value: 8 },
  { group: 'Activity', label: 'Sleep', value: 33 },
  { group: 'Weather', label: 'Weather', value: 12 },
  { group: 'Weather', label: 'Temperature', value: 2 },
  { group: 'Weather', label: 'Chance of rain', value: 4 },
  { group: 'Weather', label: 'Humidity', value: 13 },
  { group: 'Weather', label: 'Wind', value: 25 },
  { group: 'Weather', label: 'Air quality', value: 7 },
  { group: 'Weather', label: 'Air quality (gauge)', value: 27 },
  { group: 'Weather', label: 'UV index', value: 11 },
  { group: 'Weather', label: 'UV index (gauge)', value: 28 },
  { group: 'Time and date', label: 'Digital time', value: 31 },
  { group: 'Time and date', label: 'Time zone', value: 32 },
  { group: 'Time and date', label: '.beat time', value: 24 },
  { group: 'Time and date', label: 'Calendar', value: 6 },
  { group: 'Time and date', label: 'Calendar (plain)', value: 26 },
  { group: 'Sun and moon', label: 'Sunrise / sunset', value: 23 },
  { group: 'Sun and moon', label: 'Moon phase', value: 35 },
  { group: 'Place', label: 'Location', value: 36 },
  { group: 'Place', label: 'Elevation', value: 10 },
  { group: 'Watch', label: 'Battery', value: 3 },
  { group: 'Watch', label: 'Text', value: 14 },
  { label: 'None', value: 0 }
];

// The Time zone complication's zones, one picked per place showing it: the name
// the face shows, minutes from UTC, what it stands for. An abbreviation is a
// fixed offset: daylight saving is another row (PST, PDT). A fourth item is the
// saved value where two zones share a name.
var ZONE_ID = 32;  // COMP_ZONE
function zoneRows(group, rows) {
  return rows.map(function (r) {
    var m = Math.abs(r[1]), utc = r[1] ? (r[1] < 0 ? '−' : '+') + Math.floor(m / 60) + (m % 60 ? ':' + m % 60 : '') : '';
    return { group: group, value: r[3] || r[0], name: r[0], offset: r[1], label: r[0] + ' · ' + r[2] + ' · UTC' + utc };
  });
}
var ZONES = zoneRows('Americas', [
  ['HST', -600, 'Hawaii'], ['AKST', -540, 'Alaska'], ['AKDT', -480, 'Alaska Daylight'],
  ['PST', -480, 'Pacific'], ['PDT', -420, 'Pacific Daylight'], ['MST', -420, 'Mountain'], ['MDT', -360, 'Mountain Daylight'],
  ['CST', -360, 'Central'], ['CDT', -300, 'Central Daylight'], ['EST', -300, 'Eastern'], ['EDT', -240, 'Eastern Daylight'],
  ['AST', -240, 'Atlantic'], ['NST', -210, 'Newfoundland'], ['BRT', -180, 'Brasilia'], ['ART', -180, 'Argentina']
]).concat(zoneRows('Europe and Africa', [
  ['UTC', 0, 'Universal'], ['GMT', 0, 'Greenwich'], ['WET', 0, 'Western European'], ['BST', 60, 'British Summer'],
  ['CET', 60, 'Central European'], ['WAT', 60, 'West Africa'], ['CEST', 120, 'Central European Summer'],
  ['EET', 120, 'Eastern European'], ['CAT', 120, 'Central Africa'], ['SAST', 120, 'South Africa'],
  ['EEST', 180, 'Eastern European Summer'], ['MSK', 180, 'Moscow'], ['TRT', 180, 'Turkey'], ['EAT', 180, 'East Africa']
]), zoneRows('Asia', [
  ['GST', 240, 'Gulf'], ['PKT', 300, 'Pakistan'], ['IST', 330, 'India'], ['NPT', 345, 'Nepal'], ['ICT', 420, 'Indochina'],
  ['WIB', 420, 'Western Indonesia'], ['CST', 480, 'China', 'CST-CN'], ['HKT', 480, 'Hong Kong'], ['SGT', 480, 'Singapore'],
  ['PHT', 480, 'Philippines'], ['JST', 540, 'Japan'], ['KST', 540, 'Korea']
]), zoneRows('Australia and Pacific', [
  ['AWST', 480, 'Australian Western'], ['ACST', 570, 'Australian Central'], ['ACDT', 630, 'Australian Central Daylight'],
  ['AEST', 600, 'Australian Eastern'], ['AEDT', 660, 'Australian Eastern Daylight'],
  ['NZST', 720, 'New Zealand'], ['NZDT', 780, 'New Zealand Daylight']
]));

// The zone saved as `id`; UTC for one that isn't there.
function zone(id) {
  return ZONES.filter(function (z) { return z.value === id; })[0] || zone('UTC');
}

// Custom API complications (src/pkjs/api.js): settings.APIS[i] is complication
// API_ID + i in any place. Same numbers as COMP_API and API_MAX in complications.h.
var API_ID = 15, API_MAX = 8;
var API_TYPES = ['Text', 'Bar', 'Gauge'];  // index = type, see API_TEXT.. in complications/api.c

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

// Index = HANDS value, see HandStyle in src/c/settings.h
var HANDS = ['Line', 'Bar', 'Outline', 'Pointer', 'Sword', 'Dauphine', 'None'];

// Index = RING value, see RingStyle in src/c/settings.h
var RINGS = ['Default', 'Minimal', 'Sport', 'Chronograph'];

// Mirrors g_settings in C. weather.js asks Open-Meteo for the chosen
// temperature unit; the watch converts distance and elevation. Colors are GColor8 argb (0xC0 = black,
// 0xF8 = chrome yellow); a hand or band color of 0 follows the scheme.
var DEFAULTS = { SLOT_TL: 1, SLOT_TR: 2, SLOT_BL: 3, SLOT_BR: 4, CENTER_T: 14, CENTER_L: 10, CENTER_R: 12, CENTER_B: 6, SCHEME: 0, UNITS: 0, STEP_GOAL: 10000, SECONDS: 0,
  BG_COLOR: 0xC0, ACCENT_COLOR: 0xF8, HANDS: 0, RING: 0, BAND_COLOR: 0, HAND_COLOR: 0, MINUTE_COLOR: 0, SECOND_COLOR: 0,
  // The Time zone complication's zone, per place: ZONE_ + the SLOT_/CENTER_ suffix, a ZONES value.
  ZONE_TL: 'UTC', ZONE_TR: 'UTC', ZONE_BL: 'UTC', ZONE_BR: 'UTC', ZONE_T: 'UTC', ZONE_L: 'UTC', ZONE_R: 'UTC', ZONE_B: 'UTC',
  // The Text complication's text, per place: TEXT_ + the SLOT_/CENTER_ suffix.
  TEXT_TL: '', TEXT_TR: '', TEXT_BL: '', TEXT_BR: '', TEXT_T: 'PB', TEXT_L: '', TEXT_R: '', TEXT_B: '',
  // Custom API complications. Stays on the phone: the watch gets what to draw (api.js).
  APIS: [] };

// Only characters the watch font has, `n` at most.
function clean(s, n) {
  return String(s).replace(/[^ %,\-./0-9:A-Za-z]/g, '').slice(0, n);
}

// Corners fit 12, subdials 4 (TEXT_T etc.).
function cleanText(k, s) {
  return clean(s, k.length === 6 ? 4 : 12);
}

// header, text, min and max may hold {{path.to[0].value}} patterns, filled from
// the response. title names it in Settings; header is what the face shows, 4
// characters once filled. Both were one `name` once.
function cleanApis(list) {
  return (Array.isArray(list) ? list : []).slice(0, API_MAX).map(function (a) {
    a = a || {};
    return {
      url: String(a.url || '').trim(),
      freq: a.freq > 0 ? Math.max(1, Math.round(a.freq)) : 10,  // minutes, 1 at least
      type: Math.min(API_TYPES.length - 1, Math.max(0, Math.round(Number(a.type)) || 0)),
      title: String(a.title == null ? a.name || '' : a.title).trim(),
      header: String(a.header == null ? a.name || '' : a.header).trim(),
      text: String(a.text || ''),
      min: String(a.min == null ? 0 : a.min).trim(),
      max: String(a.max == null ? 100 : a.max).trim()
    };
  });
}

function value(k, v) {
  if (k === 'APIS') {
    return cleanApis(v);
  }
  if (k.indexOf('ZONE_') === 0) {
    return zone(String(v)).value;
  }
  return typeof DEFAULTS[k] === 'string' ? cleanText(k, v) : Number(v);
}

function withDefaults(saved) {
  var s = {};
  for (var k in DEFAULTS) {
    s[k] = value(k, saved && saved[k] !== undefined ? saved[k] : DEFAULTS[k]);
  }
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
  for (var k in DEFAULTS) {
    if (k !== 'APIS') {
      msg[k] = value(k, settings[k]);
    }
  }
  // The watch gets a zone as its offset, and its name as the text of a place showing it.
  CORNERS.concat(CENTERS).forEach(function (p) {
    var at = p.key.split('_')[1], z = zone(msg['ZONE_' + at]);
    msg['ZONE_' + at] = z.offset;
    if (msg[p.key] === ZONE_ID) {
      msg['TEXT_' + at] = z.name;
    }
  });
  return msg;
}

if (typeof module === 'object') {
  module.exports = {
    SETTINGS_KEY: SETTINGS_KEY, COMPLICATIONS: COMPLICATIONS, CENTER_COMPLICATIONS: CENTER_COMPLICATIONS,
    CORNERS: CORNERS, CENTERS: CENTERS, SCHEMES: SCHEMES, SCHEME_ACCENT: SCHEME_ACCENT, HANDS: HANDS, RINGS: RINGS, ZONES: ZONES, zone: zone,
    API_ID: API_ID, API_MAX: API_MAX, API_TYPES: API_TYPES,
    DEFAULTS: DEFAULTS, clean: clean, cleanText: cleanText, withDefaults: withDefaults, savedSettings: savedSettings, toMessage: toMessage
  };
}
