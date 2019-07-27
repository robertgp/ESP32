// Required module
var dgram = require('dgram');
var http = require('http');
var url = require('url');
var Db = require('tingodb')().Db,
assert = require('assert');

var express = require('express');
var app = express();

var incoming;
var final_log;
var final_presence;

// Port and IP
var PORT = 8080;
var HOST = '192.168.1.108';

// Create socket
var server = dgram.createSocket('udp4');

//create database wherever this file is on the Pi
var db = new Db('./', {});
// ONLY NEED TO DO ONCE
db.createCollection("smahrt_keys", function(err, res) {
  if (err) throw err;
  console.log("Collection created!");
  db.close();
});

// Create server
server.on('listening', function () {
    var address = server.address();
    console.log('UDP Server listening on ' + address.address + ":" + address.port);
});

// On connection, print out received message
server.on('message', function (message, remote) {

    var message_str = remote.address + ':' + remote.port + '/' + message;
    console.log(String(message));
    
    var fob_id = String(message).charAt(4);
    var hub_id = String(message).charAt(10);
    var code_id = String(message).substr(17,20);
    
    //mongodb type: will give current time
    var tim = new Date();

    final_presence = "Fob with ID" + fob_id + "has been recorder at" + tim;

    if (typeof fob_id !== 'undefined'){
        
        incoming = { Time: tim, Fob_ID: fob_id, Hub_ID: hub_id, Code: code_id };
        console.log("This is the url parsed: ");
        console.log(incoming);
                
        //INSERT- one at a time preferably, careful with inserting same data
        db.collection("smahrt_keys").insert(incoming, function(err, res) {
        if (err) throw err;
        db.close();
        });
        
        //query by ID to get time, also if ID exists in database, then it has checked in!
        var query = { Fob_ID: fob_id };
        db.collection("smahrt_keys").find(query).toArray(function(err, result) {
        if (err) throw err;
        console.log("The query from db is: \n");
        console.log(result);
        final_log = result;
        db.close();
        });
        
    } // end of if statement

    
    // Send Ok acknowledgement
    server.send("Everything was OK!",remote.port,remote.address,function(error){
        if(error){
            console.log('MEH!');
        }
        else{
            console.log('Sent OK!');

            var exec = require('child_process').exec, child;
            child = exec('curl -X PUT -d 1 192.168.1.123:80/ctrl',
                function (error, stdout, stderr) {
                    if (error !== null) {
                        console.log('exec error: ' + error);
                    }
                });
        }
    });

    //server.send(incoming, )
    
});

app.get('/', function(req, res){
    res.send(final_log);
  });
  app.listen(3000);

app.get('/presence', function(req, res){
    res.send(final_presence);
  });
  app.listen(8000);

// Bind server to port and IP
server.bind(PORT, HOST);