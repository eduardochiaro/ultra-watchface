var PAGE = require('./page');  // config.html + config.js, built in by npm run build
var config = require('./config');
var getWeather = require('./weather');

Pebble.addEventListener('ready', function() {
  getWeather();
  setInterval(getWeather, 30 * 60 * 1000);
});

// The page is handed over whole, with the saved settings, the last weather
// and the watch model written into it.
Pebble.addEventListener('showConfiguration', function() {
  var watch = Pebble.getActiveWatchInfo && Pebble.getActiveWatchInfo();
  var weather = null;
  try { weather = JSON.parse(localStorage.getItem(getWeather.WEATHER_KEY)); } catch (e) {}
  var state = { settings: config.savedSettings(), weather: weather, platform: watch && watch.platform };
  var json = JSON.stringify(state).replace(/</g, '\\u003c');
  var page = PAGE.replace('var STATE = null; //$$STATE$$', function() {
    return 'var STATE = ' + json + ';';
  });
  Pebble.openURL('data:text/html;charset=utf-8,' + encodeURIComponent(page));
});

// Settings first, then weather: units may have changed, and two messages in
// flight at once would make the second fail.
Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response) { return; }  // cancelled
  var settings;
  try {
    settings = config.withDefaults(JSON.parse(decodeURIComponent(e.response)));
  } catch (err) {
    return;
  }
  localStorage.setItem(config.SETTINGS_KEY, JSON.stringify(settings));
  Pebble.sendAppMessage(config.toMessage(settings), getWeather, function(err) {
    console.log('Settings send failed: ' + JSON.stringify(err));
  });
});
