// Open-Meteo, no key needed. Units are picked here (temperature_unit) so the
// watch only ever displays what it gets.

var savedSettings = require('./config').savedSettings;
var WEATHER_KEY = 'ultra-weather';  // last message sent, for the settings preview

function buildUrl(lat, lon, imperial) {
  return 'https://api.open-meteo.com/v1/forecast?latitude=' + lat +
    '&longitude=' + lon +
    '&current=temperature_2m,relative_humidity_2m,uv_index,weather_code,is_day' +
    '&daily=temperature_2m_min,temperature_2m_max,precipitation_probability_max' +
    '&forecast_days=1&timezone=auto' +
    (imperial ? '&temperature_unit=fahrenheit' : '');
}

function aqiUrl(lat, lon) {
  return 'https://air-quality-api.open-meteo.com/v1/air-quality?latitude=' + lat +
    '&longitude=' + lon + '&current=us_aqi';
}

// WMO weather code -> icon index on the watch (ICON_SUN.. in draw.h), -1 = unknown.
function condition(code, day) {
  if (code === 0) return day ? 0 : 1;               // clear
  if (code === 1 || code === 2) return day ? 2 : 3; // partly cloudy
  if (code === 3) return 4;                          // overcast
  if (code === 45 || code === 48) return 5;          // fog
  if (code >= 95) return 8;                          // thunderstorm
  if ((code >= 71 && code <= 77) || code === 85 || code === 86) return 7;  // snow
  if (code >= 51 && code <= 82) return 6;            // drizzle, rain, showers
  return -1;
}

// aqi: Open-Meteo air-quality response, or null. -1 = unknown.
function buildMessage(data, aqi) {
  var d = data.daily;
  var a = aqi && aqi.current && aqi.current.us_aqi;
  return {
    TEMP: Math.round(data.current.temperature_2m),
    TEMP_MIN: Math.round(d.temperature_2m_min[0]),
    TEMP_MAX: Math.round(d.temperature_2m_max[0]),
    RAIN: Math.round(d.precipitation_probability_max[0] || 0),
    AQI: typeof a === 'number' ? Math.round(a) : -1,
    CONDITION: condition(data.current.weather_code, data.current.is_day === 1),
    HUMIDITY: typeof data.current.relative_humidity_2m === 'number' ? Math.round(data.current.relative_humidity_2m) : -1,
    UV: typeof data.current.uv_index === 'number' ? Math.round(data.current.uv_index) : -1,
    ELEVATION: Math.round(data.elevation || 0)  // metres either way: the watch converts
  };
}

function fetchJson(url, ok, fail) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    if (xhr.status !== 200) { return fail('HTTP ' + xhr.status); }
    try { ok(JSON.parse(xhr.responseText)); } catch (e) { fail(e.message); }
  };
  xhr.onerror = function() { fail('network error'); };
  xhr.ontimeout = function() { fail('timeout'); };
  xhr.timeout = 10000;
  xhr.open('GET', url, true);
  xhr.send();
}

// skipSame: only send when it differs from the last message, so the watch
// isn't woken over Bluetooth (and doesn't rewrite flash) for nothing. Only
// the timer passes it: a fresh start may have lost the watch's copy.
function getWeather(skipSame) {
  var imperial = savedSettings().UNITS === 1;

  navigator.geolocation.getCurrentPosition(function(pos) {
    var lat = pos.coords.latitude, lon = pos.coords.longitude;
    fetchJson(buildUrl(lat, lon, imperial), function(data) {
      // Air quality is a separate API; weather still goes out without it.
      fetchJson(aqiUrl(lat, lon), function(aqi) { send(data, aqi); }, function(err) {
        console.log('AQI fetch failed: ' + err);
        send(data, null);
      });
    }, function(err) {
      console.log('Weather fetch failed: ' + err);
    });
  }, function(err) {
    console.log('Location failed: ' + err.message);
  }, { timeout: 15000, maximumAge: 30 * 60 * 1000 });

  function send(data, aqi) {
    var msg = buildMessage(data, aqi);
    msg.imperial = imperial;
    var json = JSON.stringify(msg);
    if (skipSame === true && json === localStorage.getItem(WEATHER_KEY)) { return; }
    localStorage.setItem(WEATHER_KEY, json);
    delete msg.imperial;
    Pebble.sendAppMessage(msg, function() {
      console.log('Weather sent');
    }, function(err) {
      console.log('Weather send failed: ' + JSON.stringify(err));
    });
  }
}

module.exports = getWeather;
module.exports.condition = condition;
module.exports.WEATHER_KEY = WEATHER_KEY;
