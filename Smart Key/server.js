// http://localhost:8080/?fob=1&hub=0&code=0001

var http = require('http');
var url = require('url');
var Db = require('tingodb')().Db,
assert = require('assert');

//create database wherever this file is on the Pi
var db = new Db('./', {});

// ONLY NEED TO DO ONCE
db.createCollection("smahrt_keys", function(err, res) {
  if (err) throw err;
  console.log("Collection created!");
  db.close();
});

// Create the server for the query
http.createServer(function (req, res) {
    res.writeHead(200, {'Content-Type': 'text/html'});
    var adr = req.url;
    
    var q = url.parse(adr, true); // parse the url
    var info = q.query; // split the url into an object
    
    
    //mongodb type: will give current time
    var tim = new Date();

    
    //Create object with the info that is being passed
    if (typeof info.fob !== 'undefined'){

        var incoming = { Time: tim, Fob_ID: info.fob, Hub_ID: info.hub, Code: info.code };
        console.log("This is the url parsed: ");
        console.log(incoming);
        
        //INSERT- one at a time preferably, careful with inserting same data
        db.collection("smahrt_keys").insert(incoming, function(err, res) {
        if (err) throw err;
        db.close();
        });

        //query by ID to get time, also if ID exists in database, then it has checked in!
        var query = { Fob_ID: info.fob };
        db.collection("smahrt_keys").find(query).toArray(function(err, result) {
        if (err) throw err;
        console.log("The query from db is: \n");
        console.log(result);
        db.close();
        });

    } // end of if statement

    res.end();
}).listen(8080);

