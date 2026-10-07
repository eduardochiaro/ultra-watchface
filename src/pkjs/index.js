var PAGE = require('./page');  // config.html + config.js, built in by npm run build
var config = require('./config');
var getWeather = require('./weather');
var api = require('./api');
var send = require('./send');

var weatherTimer;
// The watch's battery saver has the face asleep: an hour at least between refreshes.
// ponytail: lost when pkjs restarts under a sleeping face, back at the next sleep;
// have the watch say it again on 'ready' if that hour of full rate matters.
var asleep = false;

// The weather timer, at the saved refresh interval.
function scheduleWeather() {
  clearInterval(weatherTimer);
  var minutes = config.savedSettings().WEATHER_FREQ;
  weatherTimer = setInterval(function() { getWeather(true); }, (asleep ? Math.max(minutes, 60) : minutes) * 60 * 1000);
}

// The watch falls asleep or wakes. Awake again: what it missed, now.
Pebble.addEventListener('appmessage', function(e) {
  if (!e.payload || e.payload.ASLEEP === undefined) { return; }
  asleep = !!e.payload.ASLEEP;
  scheduleWeather();
  api.start(asleep);
  if (!asleep) { getWeather(true); }
});

Pebble.addEventListener('ready', function() {
  getWeather();
  api.start();
  scheduleWeather();
});

// The page is handed over whole, with the saved settings, the last weather and
// API values, and the watch model written into it.
Pebble.addEventListener('showConfiguration', function() {
  var watch = Pebble.getActiveWatchInfo && Pebble.getActiveWatchInfo();
  var weather = null, apis = null;
  try { weather = JSON.parse(localStorage.getItem(getWeather.WEATHER_KEY)); } catch (e) {}
  try { apis = JSON.parse(localStorage.getItem(api.API_KEY)); } catch (e) {}
  var state = { settings: config.savedSettings(), weather: weather, api: apis, platform: watch && watch.platform };
  var json = JSON.stringify(state).replace(/</g, '\\u003c');
  var page = PAGE.replace('var STATE = null; //$$STATE$$', function() {
    return 'var STATE = ' + json + ';';
  });
  Pebble.openURL('data:text/html;charset=utf-8,' + encodeURIComponent(page));
});

// Settings first, then weather and the API complications: units and the
// complications themselves may have changed.
Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response) { return; }  // cancelled
  var settings;
  try {
    settings = config.withDefaults(JSON.parse(decodeURIComponent(e.response)));
  } catch (err) {
    return;
  }
  localStorage.setItem(config.SETTINGS_KEY, JSON.stringify(settings));
  scheduleWeather();
  send(config.toMessage(settings), function() {
    getWeather();
    api.start();
  }, function(err) {
    console.log('Settings send failed: ' + JSON.stringify(err));
  });
});
