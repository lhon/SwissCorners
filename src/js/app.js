// ModernSwiss Watchface - PebbleKit JS with Clay + weather
var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

console.log('ModernSwiss: JS app initialized');

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.onerror = function () {
    console.log('Weather XHR error');
  };
  xhr.open(type, url);
  xhr.send();
};

// Parse Open-Meteo local time "2024-01-15T07:32" to minutes past midnight
function parseTimeToMinutes(isoLocal) {
  if (!isoLocal || isoLocal.indexOf('T') < 0) {
    return -1;
  }
  var parts = isoLocal.split('T')[1].split(':');
  return parseInt(parts[0], 10) * 60 + parseInt(parts[1], 10);
}

function locationSuccess(pos) {
  // Always fetch Celsius; watch converts based on TempUnit setting
  var url = 'https://api.open-meteo.com/v1/forecast?' +
    'latitude=' + pos.coords.latitude +
    '&longitude=' + pos.coords.longitude +
    '&current=temperature_2m,weather_code' +
    '&daily=sunrise,sunset' +
    '&temperature_unit=celsius' +
    '&timezone=auto' +
    '&forecast_days=1';

  xhrRequest(url, 'GET', function (responseText) {
    var json = JSON.parse(responseText);
    var temperature = Math.round(json.current.temperature_2m);
    var weatherCode = json.current.weather_code;
    var sunrise = parseTimeToMinutes(json.daily.sunrise[0]);
    var sunset = parseTimeToMinutes(json.daily.sunset[0]);

    Pebble.sendAppMessage({
      'Temperature': temperature,
      'WeatherCode': weatherCode,
      'Sunrise': sunrise,
      'Sunset': sunset
    },
      function () { console.log('Weather info sent'); },
      function () { console.log('Error sending weather info'); }
    );
  });
}

function locationError(err) {
  console.log('Error requesting location: ' + (err && err.message ? err.message : err));
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    { timeout: 15000, maximumAge: 60000 }
  );
}

Pebble.addEventListener('ready', function () {
  console.log('PebbleKit JS ready');
  getWeather();
});

Pebble.addEventListener('appmessage', function (e) {
  if (e.payload && e.payload.RequestWeather !== undefined) {
    getWeather();
  }
});

Pebble.addEventListener('webviewclosed', function () {
  getWeather();
});
