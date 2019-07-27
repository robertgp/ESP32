/* This is an instantiation of a server that we designed:

-- to run on a local machine (laptop)
-- creates a web server that will communicate with the local USB port
-- serves a webpage to a client that will plot using a javscript chart library

-- server side

October 2018 -- Emily Lam
*/


// Modules
var SerialPort = require('serialport');
var Readline = require('@serialport/parser-readline')
var express = require('express');
var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);

// Open serial port
var port = new SerialPort('/dev/tty.SLAB_USBtoUART', {baudRate: 115200});

// Read data from serial (this works)
/*
var read_data;
var msg;
const parser = port.pipe(new Readline({ delimiter: '\r\n' }))
parser.on('data', function (data) {
  read_data = data;
  console.log('Read:', read_data);
  msg = parseInt(read_data);  // Convert to int
  io.emit('message', msg);
});
*/

// Read from sensors
var read_ir;
var num_ir;
var read_lidar;
var num_lidar;
var read_u;
var num_u;
const parser = port.pipe(new Readline({ delimiter: '\r\n' }))
parser.on('data', function (data) {
  if (data.includes("IR")){
    read_ir = data;
    var inter_ir = read_ir.replace("IR","");
    console.log('Read IR:', inter_ir);
    num_ir = parseInt(inter_ir);  // Convert to int
    io.emit('ir', num_ir);
  }
  else if (data.includes("L")){
    read_lidar = data;
    var inter_lidar = read_lidar.replace("L","");
    console.log('Read L:', inter_lidar);
    num_lidar = parseInt(inter_lidar);  // Convert to int
    io.emit('lidar', num_lidar);
  }
  else if (data.includes("U")){
    read_u = data;
    var inter_u = read_u.replace("U","");
    console.log('Read U:', inter_u);
    num_u = parseInt(inter_u);  // Convert to int
    io.emit('ultra', num_u);
  }
});


// // Test stuff --> no serial port
// var i = 0;
// setInterval( function() {
//   console.log('Read:', i);
//   io.emit('message', i);
//   i++;
//   if (i > 10) {i=0;}
// }, 1000);

// Points to index.html to serve webpage
app.get('/', function(req, res){
  res.sendFile(__dirname + '/index.html');
});

// User socket connection
io.on('connection', function(socket){
  console.log('a user connected');
  socket.on('disconnect', function(){
    console.log('user disconnected');
  });
});

// Listening on localhost:3000
http.listen(3000, function() {
  console.log('listening on *:3000');
});
