// Settings model, shared by index.js, weather.js and the settings page
// (scripts/inline-config.mjs inlines this file into config.html, so keep it
// plain ES5 with no requires).

var SETTINGS_KEY = 'ultra-settings';  // last saved page result

// Values match ComplicationId in src/c/complications/complications.h. `group`
// is its <optgroup> in the pickers; None has none and stays last.
var COMPLICATIONS = [
  { group: 'Activity', label: 'Steps', value: 1 },
  { group: 'Activity', label: 'Distance', value: 9 },
  { group: 'Activity', label: 'Calories', value: 29 },
  { group: 'Activity', label: 'Active minutes', value: 34 },
  { group: 'Activity', label: 'Heart rate', value: 8 },
  { group: 'Activity', label: 'Sleep', value: 33 },
  { group: 'Weather', label: 'Weather', value: 12 },
  { group: 'Weather', label: 'Temperature', value: 2 },
  { group: 'Weather', label: 'Chance of rain', value: 4 },
  { group: 'Weather', label: 'Humidity', value: 13 },
  { group: 'Weather', label: 'Wind', value: 25 },
  { group: 'Weather', label: 'Air quality', value: 7 },
  { group: 'Weather', label: 'UV index', value: 11 },
  { group: 'Weather', label: 'Pressure', value: 44 },
  { group: 'Time and date', label: 'Digital time', value: 31 },
  { group: 'Time and date', label: 'Time zone', value: 32 },
  { group: 'Time and date', label: '.beat time', value: 24 },
  { group: 'Time and date', label: 'Date', value: 6 },
  { group: 'Time and date', label: 'Week', value: 37 },
  { group: 'Time and date', label: 'Progress', value: 38 },
  { group: 'Time and date', label: 'Days until / since', value: 41 },
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
  { group: 'Activity', label: 'Calories', value: 29 },
  { group: 'Activity', label: 'Active minutes', value: 34 },
  { group: 'Activity', label: 'Heart rate', value: 8 },
  { group: 'Activity', label: 'Sleep', value: 33 },
  { group: 'Weather', label: 'Weather', value: 12 },
  { group: 'Weather', label: 'Temperature', value: 2 },
  { group: 'Weather', label: 'Chance of rain', value: 4 },
  { group: 'Weather', label: 'Humidity', value: 13 },
  { group: 'Weather', label: 'Wind', value: 25 },
  { group: 'Weather', label: 'Air quality', value: 7 },
  { group: 'Weather', label: 'UV index', value: 11 },
  { group: 'Weather', label: 'Pressure', value: 44 },
  { group: 'Time and date', label: 'Digital time', value: 31 },
  { group: 'Time and date', label: 'Time zone', value: 32 },
  { group: 'Time and date', label: '.beat time', value: 24 },
  { group: 'Time and date', label: 'Date', value: 6 },
  { group: 'Time and date', label: 'Week', value: 37 },
  { group: 'Time and date', label: 'Progress', value: 38 },
  { group: 'Time and date', label: 'Days until / since', value: 41 },
  { group: 'Sun and moon', label: 'Sunrise / sunset', value: 23 },
  { group: 'Sun and moon', label: 'Moon phase', value: 35 },
  { group: 'Place', label: 'Location', value: 36 },
  { group: 'Place', label: 'Elevation', value: 10 },
  { group: 'Watch', label: 'Battery', value: 3 },
  { group: 'Watch', label: 'Text', value: 14 },
  { label: 'None', value: 0 }
];

// One pick in the pickers, its kinds in a second picker beside it. Each kind is
// a ComplicationId of its own, saved as the place's: the watch knows no families.
// The first is the one listed in COMPLICATIONS.
var VARIANTS = [
  [{ label: 'Today', value: 2 }, { label: 'Now', value: 46 }, { label: 'Feels like', value: 47 }], // Temperature
  [{ label: 'Today', value: 4 }, { label: 'Hourly', value: 39 }],     // Chance of rain
  [{ label: 'Sections', value: 7 }, { label: 'Gauge', value: 27 }],   // Air quality
  [{ label: 'Sections', value: 11 }, { label: 'Gauge', value: 28 }],  // UV index
  [{ label: 'Total', value: 29 }, { label: 'Active', value: 30 }],    // Calories
  [{ label: 'Icon', value: 12 }, { label: 'Icon + temp', value: 40 }], // Weather
  [{ label: 'Year', value: 38 }, { label: 'Month', value: 42 }, { label: 'Week', value: 45 }, { label: 'Day', value: 43 }] // Progress
];
// The listed complication `id` is a kind of; itself with no kinds.
function family(id) {
  var kinds = VARIANTS.filter(function (v) { return v.some(function (o) { return o.value === id; }); })[0];
  return kinds ? kinds[0].value : id;
}

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

// The Date complication's formats, one picked per place showing it. Index =
// value, see DateFormat in src/c/complications/complications.h: append only.
var DATE_ID = 6;  // COMP_CALENDAR
var DATE_FORMATS = ['Calendar', 'Weekday and day', 'Month and day', 'Full date', 'Numeric', 'Week number', 'Day of year', 'Year'];
var PLAIN_ID = 26;  // COMP_CALENDAR_PLAIN, retired: a subdial's Date as weekday and day

// The settings preview's, here to be tested: calendar.c's year_days(), tm_yday, iso_week() and week_number().
function yearDays(y) { return y % 4 === 0 && (y % 100 !== 0 || y % 400 === 0) ? 366 : 365; }
function yearDay(d) { return Math.round((Date.UTC(d.getFullYear(), d.getMonth(), d.getDate()) - Date.UTC(d.getFullYear(), 0, 1)) / 86400000); }
// ISO 8601: weeks start on Monday, and a week is of the year its Thursday is in.
function isoWeek(d) {
  var y = d.getFullYear(), thu = yearDay(d) - (d.getDay() + 6) % 7 + 3;
  if (thu < 0) {
    thu += yearDays(y - 1);
  } else if (thu >= yearDays(y)) {
    thu -= yearDays(y);
  }
  return Math.trunc(thu / 7) + 1;
}
// The week starting `start` days after Monday: ISO from Monday; from another day, week 1 is the one 1 January is in.
function weekNumber(d, start) {
  if (!start) {
    return isoWeek(d);
  }
  var yd = yearDay(d), jan1 = ((d.getDay() + 6 - start) % 7 - yd % 7 + 7) % 7;
  return Math.trunc((yd + jan1) / 7) + 1;
}

// The Week starts on setting: days after Monday. Monday, Sunday, Saturday.
var WEEK_STARTS = [{ label: 'Monday', value: 0 }, { label: 'Sunday', value: 6 }, { label: 'Saturday', value: 5 }];

// The countdown's day, one picked per place showing it, "2026-12-25" or "".
// The watch gets it as days since 1970-01-01, in an int16: up to 2059-09-18; 0 for none.
var COUNT_ID = 41;  // COMP_COUNTDOWN
function countDay(iso) {
  var m = /^(\d{4})-(\d\d)-(\d\d)$/.exec(iso || ''), day = m ? Date.UTC(+m[1], m[2] - 1, +m[3]) / 86400000 : 0;
  return day > 0 && day <= 32767 ? day : 0;
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

// JOIN bits: a corner joined to the next one clockwise, across the gap between
// them. JOIN_* in settings.h; emery joins the top and the bottom only.
var JOINS = [
  { label: 'Join the top corners', value: 1 },
  { label: 'Join the right corners', value: 2, round: true },
  { label: 'Join the bottom corners', value: 4 },
  { label: 'Join the left corners', value: 8, round: true }
];
var CLOCKWISE = ['SLOT_TL', 'SLOT_TR', 'SLOT_BR', 'SLOT_BL'];

// The corners as runs, each clockwise from the first whose gap before it is
// open, with the corner whose complication it shows: the one at its left end,
// else at its bottom. All four make one ring (`ring`), its labels at 6 o'clock.
// The loop in draw_face() and run_owner() in ultra-watchface.c.
function runs(join, round) {
  join &= round ? 15 : 5;
  var ring = join === 15;
  if (ring) {
    join = 11;
  }
  function left(i) { return i === 0 || i === 3; }
  var out = [];
  for (var i = 0; i < 4; i++) {
    if (join & (1 << (i + 3) % 4)) {
      continue;
    }
    var run = [i];
    while (join & (1 << run[run.length - 1])) {
      run.push((run[run.length - 1] + 1) % 4);
    }
    var last = run[run.length - 1];
    var own = left(i) !== left(last) ? (left(i) ? i : last) : (i >= 2 ? i : last);
    out.push({ corners: run.map(function (k) { return CLOCKWISE[k]; }), owner: CLOCKWISE[own], ring: ring });
  }
  return out;
}

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
var RINGS = ['Default', 'Minimal', 'Sport', 'Chronograph', 'Roman', 'Tachymeter', 'Compass'];

// Mirrors g_settings in C. weather.js asks Open-Meteo for the chosen
// temperature unit; the watch converts distance and elevation. Colors are GColor8 argb (0xC0 = black,
// 0xF8 = chrome yellow); a hand or band color of 0 follows the scheme. LANG: 0
// follows the watch's language, else a LANG_CODES index + 1 (i18n.js).
var DEFAULTS = { SLOT_TL: 1, SLOT_TR: 2, SLOT_BL: 3, SLOT_BR: 4, CENTER_T: 14, CENTER_L: 10, CENTER_R: 12, CENTER_B: 6, SCHEME: 0, UNITS: 0, STEP_GOAL: 10000, SECONDS: 0, SWEEP: 0, SHAKE_HIDE: 0, SAVER: 0, SQUARE: 0, LANG: 0, WEEK_START: 0, JOIN: 0,
  BG_COLOR: 0xC0, ACCENT_COLOR: 0xF8, HANDS: 0, RING: 0, BAND_COLOR: 0, HAND_COLOR: 0, MINUTE_COLOR: 0, SECOND_COLOR: 0,
  // The Time zone complication's zone, per place: ZONE_ + the SLOT_/CENTER_ suffix, a ZONES value.
  ZONE_TL: 'UTC', ZONE_TR: 'UTC', ZONE_BL: 'UTC', ZONE_BR: 'UTC', ZONE_T: 'UTC', ZONE_L: 'UTC', ZONE_R: 'UTC', ZONE_B: 'UTC',
  // The Date complication's format, per place: DATE_ + the SLOT_/CENTER_ suffix, a DATE_FORMATS index.
  // On the phone: the watch gets it as the ZONE_ of a place showing the date.
  DATE_TL: 0, DATE_TR: 0, DATE_BL: 0, DATE_BR: 0, DATE_T: 0, DATE_L: 0, DATE_R: 0, DATE_B: 0,
  // The countdown's day, per place: COUNT_ + the SLOT_/CENTER_ suffix. Its name is the place's TEXT_.
  // On the phone: the watch gets it as the ZONE_ of a place showing the countdown, a countDay().
  COUNT_TL: '', COUNT_TR: '', COUNT_BL: '', COUNT_BR: '', COUNT_T: '', COUNT_L: '', COUNT_R: '', COUNT_B: '',
  // The Text complication's text, per place: TEXT_ + the SLOT_/CENTER_ suffix.
  TEXT_TL: '', TEXT_TR: '', TEXT_BL: '', TEXT_BR: '', TEXT_T: 'PB', TEXT_L: '', TEXT_R: '', TEXT_B: '',
  // Weather: minutes between refreshes, a WEATHER_FREQS value, and where it is for:
  // { name, label, lat, lon } picked in Settings, or null for the phone's own position. Both stay on the phone.
  WEATHER_FREQ: 30, WEATHER_PLACE: null,
  // Custom API complications. Stays on the phone: the watch gets what to draw (api.js).
  APIS: [] };
var WEATHER_FREQS = [15, 30, 60, 120, 180];

// name is the city, what the Location complication shows; label tells it from its namesakes.
function cleanPlace(p) {
  var lat = p ? Number(p.lat) : NaN, lon = p ? Number(p.lon) : NaN;
  if (!(Math.abs(lat) <= 90) || !(Math.abs(lon) <= 180)) {
    return null;
  }
  return { name: String(p.name || '').slice(0, 40), label: String(p.label || p.name || '').slice(0, 80), lat: lat, lon: lon };
}

// Only characters the watch font has, `n` bytes at most: the watch keeps
// UTF-8, an accented letter takes 2. One the font lacks loses its accent: "ś" is "s".
// Those with no accent to lose are in BASE: "ł" is "l", not dropped.
var FONT = /[ %,\-./0-9:A-Za-zÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÑÒÓÔÕÖØÙÚÛÜßàáâãäåæçèéêëìíîïñòóôõöøùúûüŒœ]/;
var BASE = { 'Đ': 'D', 'đ': 'd', 'Ð': 'D', 'ð': 'd', 'Ħ': 'H', 'ħ': 'h', 'ı': 'i', 'Ł': 'L', 'ł': 'l' };
function clean(s, n) {
  var out = '', bytes = 0;
  s = String(s);
  (s.normalize ? s.normalize('NFC') : s).split('').every(function (c) {
    c = FONT.test(c) ? c : BASE[c] || (c.normalize ? c.normalize('NFD')[0] : '');
    if (!FONT.test(c)) {
      return true;
    }
    bytes += c > '\x7f' ? 2 : 1;
    out += bytes <= n ? c : '';
    return bytes < n;
  });
  return out;
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
  if (k === 'WEATHER_PLACE') {
    return cleanPlace(v);
  }
  if (k === 'WEATHER_FREQ') {
    return WEATHER_FREQS.indexOf(Number(v)) < 0 ? DEFAULTS[k] : Number(v);
  }
  if (k.indexOf('ZONE_') === 0) {
    return zone(String(v)).value;
  }
  if (k.indexOf('DATE_') === 0) {
    return DATE_FORMATS[v] ? Number(v) : 0;
  }
  if (k.indexOf('COUNT_') === 0) {
    return countDay(v) ? String(v) : '';
  }
  if (k === 'JOIN') {
    return Number(v) & 15;
  }
  if (k === 'WEEK_START') {
    return WEEK_STARTS.some(function (w) { return w.value === Number(v); }) ? Number(v) : 0;
  }
  return typeof DEFAULTS[k] === 'string' ? cleanText(k, v) : Number(v);
}

function withDefaults(saved) {
  var s = {};
  for (var k in DEFAULTS) {
    s[k] = value(k, saved && saved[k] !== undefined ? saved[k] : DEFAULTS[k]);
  }
  CENTERS.forEach(function (p) {
    if (s[p.key] === PLAIN_ID) {
      s[p.key] = DATE_ID;
      s['DATE_' + p.key.split('_')[1]] = 1;
    }
  });
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
    if (k !== 'APIS' && k.indexOf('WEATHER_') !== 0 && k.indexOf('DATE_') !== 0 && k.indexOf('COUNT_') !== 0) {  // the phone's own
      msg[k] = value(k, settings[k]);
    }
  }
  // The watch gets a zone as its offset, and its name as the text of a place showing it.
  // A place showing the date gets its format there, one showing the countdown its day.
  CORNERS.concat(CENTERS).forEach(function (p) {
    var at = p.key.split('_')[1], z = zone(msg['ZONE_' + at]);
    msg['ZONE_' + at] = msg[p.key] === DATE_ID ? value('DATE_' + at, settings['DATE_' + at]) :
      msg[p.key] === COUNT_ID ? countDay(settings['COUNT_' + at]) : z.offset;
    if (msg[p.key] === ZONE_ID) {
      msg['TEXT_' + at] = z.name;
    }
  });
  return msg;
}

if (typeof module === 'object') {
  module.exports = {
    SETTINGS_KEY: SETTINGS_KEY, COMPLICATIONS: COMPLICATIONS, CENTER_COMPLICATIONS: CENTER_COMPLICATIONS,
    CORNERS: CORNERS, CENTERS: CENTERS, SCHEMES: SCHEMES, SCHEME_ACCENT: SCHEME_ACCENT, HANDS: HANDS, RINGS: RINGS, VARIANTS: VARIANTS, family: family, ZONES: ZONES, zone: zone, DATE_ID: DATE_ID, DATE_FORMATS: DATE_FORMATS, yearDays: yearDays, yearDay: yearDay, isoWeek: isoWeek,
    weekNumber: weekNumber, WEEK_STARTS: WEEK_STARTS, JOINS: JOINS, runs: runs, COUNT_ID: COUNT_ID, countDay: countDay,
    API_ID: API_ID, API_MAX: API_MAX, API_TYPES: API_TYPES, WEATHER_FREQS: WEATHER_FREQS,
    DEFAULTS: DEFAULTS, clean: clean, cleanText: cleanText, withDefaults: withDefaults, savedSettings: savedSettings, toMessage: toMessage
  };
}
