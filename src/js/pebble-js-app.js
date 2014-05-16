var Global = {
  apiKey:            '',
  configurationUrl:  'http://jaredbiehler.github.io/NPR-Station-Finder/config/',
  updateInProgress:  false,
  updateWaitTimeout: 2 * 60 * 1000, // two minutes in ms
  lastUpdateAttempt: new Date(),
  maxRetry:          3,
  retryWait:         750,
  config:            {
    zip: ''
  }
};

var ack  = function () { console.log("Pebble ACK sendAppMessage");};
var nack = function (data, retry) {
  retry = typeof retry !== 'undefined' ? retry : 0;
  retry++;
  if (retry >= Global.maxRetry) {
    console.warn("Pebble NACK sendAppMessage max exceeded");
    return;
  }
  console.warn("Pebble NACK sendAppMessage retryCount:"+retry+" data:"+JSON.stringify(data));
  if (data) {
    setTimeout(function(){
          Pebble.sendAppMessage(data, ack, function(e){
            nack(data, retry);
          });
    }, Global.retryWait + Math.floor(Math.random() * Global.retryWait));
  }
};

var updateNprData = function () {
  if (Global.updateInProgress &&
    new Date().getTime() < Global.lastUpdateAttempt.getTime() + Global.updateWaitTimeout) {
    console.log("Update already started in the last "+(Global.updateWaitTimeout/60000)+" minutes");
    return;
  }
  Global.updateInProgress  = true;
  Global.lastUpdateAttempt = new Date();

  if (Global.config.zip !== null && Global.config.zip.length > 0) {
    fetchNprDataViaZip();
  } else {
    var locationOptions = { "timeout": 15000, "maximumAge": 60000 };
    navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
  }
};

var locationSuccess = function (pos) {
  var coordinates = pos.coords;
  console.log("Got coordinates: " + JSON.stringify(coordinates));
  fetchNprDataViaGPS(coordinates.latitude, coordinates.longitude);

};

var locationError = function (err) {
  var message = 'Location error (' + err.code + '): ' + err.message;
  console.warn(message);
  Pebble.sendAppMessage({ "error": "Loc unavailable" }, ack, nack);
  Global.updateInProgress = false;
};

var fetchNprDataViaZip = function() {
   var url   = 'http://api.npr.org/v2/stations/search/'+Global.config.zip+'?apiKey='+Global.apiKey;

  console.log('URL: ' + url);

  getJson(url, function(err, response) {

    try {
      if (err) {
        throw err;
      }

      var stationsFound = 0
        , stationsData  = {};

      for (var i = 0; i < response.length; i++) {

        var stationData = {
          call:      response[i].call,
          frequency: response[i].frequency,
          strength:  response[i].strength,
          band:      response[i].band,
          guid:      response[i].guid
        };

        stationsFound++;

        if (stationsFound === 1) {
          stationsData.primary_call      = stationData.call;
          stationsData.primary_frequency = ' '+stationData.frequency.slice(-5);
          stationsData.primary_band      = stationData.band;
          stationsData.primary_strength  = parseInt(stationData.strength);
          fetchStationStreams(stationData.guid, true);
        } else if (stationsFound === 2) {
          stationsData.secondary_call      = stationData.call;
          stationsData.secondary_frequency = ' '+stationData.frequency.slice(-5);
          stationsData.secondary_band      = stationData.band;
          stationsData.secondary_strength  = parseInt(stationData.strength);
          fetchStationStreams(stationData.guid, false);
        } else {
          break;
        }
      }
      console.log("Station Data: "+JSON.stringify(stationsData));

      Pebble.sendAppMessage(stationsData, ack, function(e){
        nack(stationsData);
      });

    } catch (e) {
      console.warn("Could not find station streams in response: " + e.message);
    }
    Global.updateInProgress = false;
  });
}

var fetchNprDataViaGPS = function(latitude, longitude) {

  var url   = 'http://api.npr.org/stations?lat='+latitude+'&lon='+longitude+'&apiKey='+Global.apiKey+'&format=json';

  console.log('URL: ' + url);

  getJson(url, function(err, response) {

    try {
      if (err) {
        throw err;
      }

      var stations = response.station
        , stationsFound = 0
        , stationsData  = {};

      for (var i = 0; i < stations.length; i++) {
        if (stations[i].memberStatus.$text !== 'Member') {
          continue;
        }

        var stationData = {
          call:      stations[i].callLetters.$text,
          frequency: stations[i].frequency.$text,
          strength:  stations[i].signal.strength,
          band:      stations[i].band.$text,
          guid:      stations[i].guid.$text
        };

        stationsFound++;

        if (stationsFound === 1) {
          stationsData.primary_call      = stationData.call;
          stationsData.primary_frequency = ' '+stationData.frequency.slice(-5);
          stationsData.primary_band      = stationData.band;
          stationsData.primary_strength  = parseInt(stationData.strength);
          fetchStationStreams(stationData.guid, true);
        } else if (stationsFound === 2) {
          stationsData.secondary_call      = stationData.call;
          stationsData.secondary_frequency = ' '+stationData.frequency.slice(-5);
          stationsData.secondary_band      = stationData.band;
          stationsData.secondary_strength  = parseInt(stationData.strength);
          fetchStationStreams(stationData.guid, false);
        } else {
          break;
        }
      }
      console.log("Station Data: "+JSON.stringify(stationsData));

      Pebble.sendAppMessage(stationsData, ack, function(e){
        nack(stationsData);
      });
    
    } catch (e) {
      console.warn("Could not find stations data in response: " + e.message);
      Pebble.sendAppMessage({ "error": "HTTP Error" }, ack, nack);
    }
    Global.updateInProgress = false;
  });
};

var fetchStationStreams = function(guid, isPrimary) {

  var url = 'http://api.npr.org/v2/stations/'+guid+'/streams?apiKey='+Global.apiKey;

  console.log('URL: ' + url);

  getJson(url, function(err, response) {

    try {
      if (err) {
        throw err;
      }

      var streamGuid = null;
      for (var i = 0; i < response.length; i++) {
        for (var j = 0; j < response[i].station.length; j++) {
          if (response[i].station[j].primary_stream) {
            streamGuid = response[i].guid;
          }
        }
      }

      if (streamGuid !== null) {
        console.log("Station stream guid: "+streamGuid);
        fetchStreamProgram(streamGuid, isPrimary);
      }

    } catch (e) {
      console.warn("Could not find station streams in response: " + e.message);
    }
  });
};

var fetchStreamProgram = function (guid, isPrimary) {

  var url = 'http://api.npr.org/v2/streams/'+guid+'/times/now/episodes?apiKey='+Global.apiKey;

  console.log('URL: ' + url);

  getJson(url, function(err, response) {

    try {
      if (err) {
        throw err;
      }

      var program = null;
      for (var i = 0; i < response.length; i++) {
        if (typeof response[i].program !== "undefined" &&
            typeof response[i].program.name !== "undefined") {
          program = response[i].program.name;
        }
      }

      if (program !== null) {
        console.log("Stream program name: "+program);
        var data = isPrimary ? {primary_program: program} : {secondary_program: program};
        Pebble.sendAppMessage(data, ack, function(){
          nack(data);
        });
      }

    } catch (e) {
      console.warn("Could not find program data in response: " + e.message);
    }
  });
}

var getJson = function(url, callback) {
  try {
    var req = new XMLHttpRequest();
    req.open('GET', url, true);
    req.onload = function(e) {
      if (req.readyState == 4) {
        if(req.status == 200) {
          try {
            //console.log(req.responseText);
            var response = JSON.parse(req.responseText);
            callback(null, response);
          } catch (e) {
            callback(e.message);
          }
        } else {
          callback("Error request status not 200, status: "+req.status);
        }
      }
    };
    req.send(null);
  } catch(e) {
    callback("Unable to GET JSON: "+e.message);
  }
};


var serialize = function (obj) {
  var str = [];
  for(var p in obj)
    if (obj.hasOwnProperty(p)) {
      str.push(encodeURIComponent(p) + "=" + encodeURIComponent(obj[p]));
    }
  return str.join("&");
};

// Setup Pebble Event Listeners
Pebble.addEventListener("ready", function(e) {
  console.log("Starting ...");
  var data = { "js_ready": true };
  Pebble.sendAppMessage(data, ack, function(e){
    nack(data);
  });
});

Pebble.addEventListener("appmessage", function(data) {
  console.log("Got a message - Starting request ... " + JSON.stringify(data));
  try {
    if (typeof data.payload.zip !== "undefined"){
      Global.config.zip = data.payload.zip;
    }
    updateNprData();
  } catch (e) {
    console.warn("Could not retrieve data sent from Pebble: "+e.message);
  }
});

Pebble.addEventListener("showConfiguration", function (e) {
  var url = Global.configurationUrl+'?'+serialize(Global.config);
  console.log('Configuration requested using url: '+url);
  Pebble.openURL(url);
});

Pebble.addEventListener("webviewclosed", function(e) {

   // Android Hack - for whatever reason this event is always firing on Android with a message of 'CANCELLED'

  if (e.response && e.response !== 'CANCELLED') {
    try {
      var settings = JSON.parse(decodeURIComponent(e.response));

      // Android 'cancel' sends a blank object
      if (Object.keys(settings).length <= 0) {
        return;
      }

      if (typeof settings.zip !== "undefined") {
        Global.config.zip = settings.zip;
      } else {
        Global.config.zip = '';
      }
      
      console.log("Settings received: "+JSON.stringify(settings));
      Pebble.sendAppMessage(Global.config, ack, function(){
        nack(Global.config);
      });
      updateNprData();
    } catch(ex) {
      console.warn("Unable to parse response from configuration:"+ex.message);
    }
  }
});