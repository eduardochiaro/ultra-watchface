var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var getWeather = require('./weather');

var clay = new Clay(clayConfig, null, { autoHandleEvents: false });

Pebble.addEventListener('ready', function() {
  getWeather();
  setInterval(getWeather, 30 * 60 * 1000);
});

Pebble.addEventListener('showConfiguration', function() {
  Pebble.openURL(clay.generateUrl());
});

// Settings first, then weather: units may have changed, and two messages in
// flight at once would make the second fail.
Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response) { return; }
  Pebble.sendAppMessage(clay.getSettings(e.response), getWeather, function(err) {
    console.log('Settings send failed: ' + JSON.stringify(err));
  });
});
