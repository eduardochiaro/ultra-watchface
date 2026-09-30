// Open-Meteo, no key needed. Units are picked here (temperature_unit) so the
// watch only ever displays what it gets.

var savedSettings = require('./config').savedSettings;
var WEATHER_KEY = 'ultra-weather';  // last message sent, for the settings preview

function buildUrl(lat, lon, imperial) {
  return 'https://api.open-meteo.com/v1/forecast?latitude=' + lat +
    '&longitude=' + lon +
    '&current=temperature_2m' +
    '&daily=temperature_2m_min,temperature_2m_max,precipitation_probability_max,sunrise,sunset' +
    '&forecast_days=1&timezone=auto' +
    (imperial ? '&temperature_unit=fahrenheit' : '');
}

// "2026-09-30T06:52" (local, timezone=auto) -> minutes since midnight.
function minutes(iso) {
  if (!iso) { return 0; }   // polar day / night
  return parseInt(iso.slice(11, 13), 10) * 60 + parseInt(iso.slice(14, 16), 10);
}

function buildMessage(data) {
  var d = data.daily;
  return {
    TEMP: Math.round(data.current.temperature_2m),
    TEMP_MIN: Math.round(d.temperature_2m_min[0]),
    TEMP_MAX: Math.round(d.temperature_2m_max[0]),
    RAIN: Math.round(d.precipitation_probability_max[0] || 0),
    SUNRISE: minutes(d.sunrise[0]),
    SUNSET: minutes(d.sunset[0])
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

function getWeather() {
  var imperial = savedSettings().UNITS === 1;

  navigator.geolocation.getCurrentPosition(function(pos) {
    fetchJson(buildUrl(pos.coords.latitude, pos.coords.longitude, imperial), function(data) {
      var msg = buildMessage(data);
      msg.imperial = imperial;
      localStorage.setItem(WEATHER_KEY, JSON.stringify(msg));
      delete msg.imperial;
      Pebble.sendAppMessage(msg, function() {
        console.log('Weather sent');
      }, function(err) {
        console.log('Weather send failed: ' + JSON.stringify(err));
      });
    }, function(err) {
      console.log('Weather fetch failed: ' + err);
    });
  }, function(err) {
    console.log('Location failed: ' + err.message);
  }, { timeout: 15000, maximumAge: 30 * 60 * 1000 });
}

module.exports = getWeather;
module.exports.buildMessage = buildMessage;
module.exports.WEATHER_KEY = WEATHER_KEY;
