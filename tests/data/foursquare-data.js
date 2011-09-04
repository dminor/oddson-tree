/*
Copyright (c) 2011 Daniel Minor 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

var https = require('https');
var fs = require('fs');

/******************************************************************************
    Global Variables
*******************************************************************************/
var secret = JSON.parse(fs.readFileSync('secret.json', encoding='utf8')); 

/******************************************************************************
    Functions
*******************************************************************************/
function makeVenueSearchRequest(lat, lon, callback)
{ 
    var req = https.request({
        host: 'api.foursquare.com',
        path: '/v2/venues/search?ll=' + lat + ',' + lon +
            '&limit=50&client_id=' + secret.CLIENT_ID +
            '&client_secret=' + secret.CLIENT_SECRET +
            '&v=20110901'
    });

    req.on('response', function(response) {
        var data = '';

        response.on('data', function(chunk) {
            data += chunk; 
        });

        response.on('end', function() {
             callback(data);
        });
    });

    return req;
}

/******************************************************************************
    Main 
*******************************************************************************/
var req = makeVenueSearchRequest(45.4, -75.71, function(data) { 

    var json = JSON.parse(data);

    if (json && json.response && json.response.venues) {
        var venues = json.response.venues;
        for (var i = 0; i < venues.length; ++i) {
            console.log(venues[i].name,
                venues[i].location.lat,
                venues[i].location.lng,
                venues[i].location.distance,
                venues[i].stats.checkinsCount);
        }
    }
});

req.end();
