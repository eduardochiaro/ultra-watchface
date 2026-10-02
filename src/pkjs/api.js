// Custom API complications: each one in use fetches its URL on its own timer
// and sends the watch what to draw. Patterns are filled in here, so the watch
// only ever displays what it gets.

var config = require('./config');
var fetchJson = require('./weather').fetchJson;
var send = require('./send');
var API_KEY = 'ultra-api';  // last message per complication, for the settings preview

// lookup({ a: { b: [{ c: 1 }] } }, 'a.b[0].c') -> 1; undefined when a step is missing.
function lookup(data, path) {
  var keys = path.replace(/\[(\w+)\]/g, '.$1').split('.');
  for (var i = 0; i < keys.length && data != null; i++) {
    if (keys[i]) { data = data[keys[i]]; }
  }
  return data;
}

// '{{a.b}} kW' -> '12.5 kW'. Numbers keep 2 decimals; anything missing is '--'.
function fill(template, data) {
  return String(template).replace(/\{\{(.*?)\}\}/g, function(m, path) {
    var v = lookup(data, path.trim());
    if (typeof v === 'number') { return String(Math.round(v * 100) / 100); }
    return typeof v === 'string' || typeof v === 'boolean' ? String(v) : '--';
  });
}

function number(s) {
  var m = /-?\d+(\.\d+)?/.exec(s);
  return m ? Number(m[0]) : NaN;
}

// A gauge's end label: 4 characters or so.
function short(n) {
  var a = Math.abs(n);
  return a >= 10000 ? Math.round(n / 1000) + 'k' : String(a >= 100 ? Math.round(n) : Math.round(n * 10) / 10);
}

// data: the parsed response, or null when there is none ('--' on the watch).
function buildMessage(api, i, data) {
  var msg = { API_INDEX: i, API_TYPE: api.type, API_NAME: api.name, API_TEXT: '--', API_PCT: -1, API_MIN: '', API_MAX: '' };
  if (data == null) { return msg; }
  msg.API_TEXT = config.clean(fill(api.text, data), 12);
  if (api.type === 0) { return msg; }
  // Bar and gauge: the text's first pattern, placed between min and max.
  var v = number(fill((/\{\{.*?\}\}/.exec(api.text) || [api.text])[0], data));
  var min = number(fill(api.min, data)), max = number(fill(api.max, data));
  if (max > min && !isNaN(v)) {
    msg.API_PCT = Math.max(0, Math.min(100, Math.round((v - min) * 100 / (max - min))));
    msg.API_MIN = short(min);
    msg.API_MAX = short(max);
  }
  return msg;
}

var timers = [], sent = [], run = 0;

// skipSame: only send what differs from the last message, and keep the last
// value when the fetch fails. Only the timers pass it.
function refresh(api, i, skipSame) {
  var mine = run;
  function done(data) {
    if (mine !== run) { return; }  // settings changed while fetching
    var msg = buildMessage(api, i, data);
    if (skipSame === true && (data == null || JSON.stringify(msg) === JSON.stringify(sent[i]))) { return; }
    sent[i] = msg;
    localStorage.setItem(API_KEY, JSON.stringify(sent));
    send(msg, null, function(err) {
      console.log('API ' + i + ' send failed: ' + JSON.stringify(err));
      sent[i] = null;
    });
  }
  if (!/^https?:\/\//i.test(api.url)) { return done(null); }
  fetchJson(api.url, done, function(err) {
    console.log('API ' + i + ' fetch failed: ' + err);
    done(null);
  });
}

// (Re)starts the timers from the saved settings. Complications no place shows
// are left alone.
// ponytail: one request per complication; share them per URL if many read the same endpoint.
function start() {
  var settings = config.savedSettings();
  var places = config.CORNERS.concat(config.CENTERS);
  timers.forEach(clearInterval);
  sent = [];
  run++;
  timers = settings.APIS.map(function(api, i) {
    if (!places.some(function(p) { return settings[p.key] === config.API_ID + i; })) { return 0; }
    refresh(api, i);
    return setInterval(function() { refresh(api, i, true); }, api.freq * 60 * 1000);
  });
}

module.exports = { start: start, buildMessage: buildMessage, API_KEY: API_KEY };
