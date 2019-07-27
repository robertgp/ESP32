//will need: npm install tingodb, mongodb --save


//actual quest database:

var Db = require('tingodb')().Db,
assert = require('assert');


//create database whereever this file is on the Pi
var db = new Db('./', {});
// ONLY NEED TO DO ONCE
db.createCollection("smahrt_keys", function(err, res) {
  if (err) throw err;
  console.log("Collection created!");
  db.close();
});

//mongodb type: will give current time
var tim = new Date();

//pass through ID & Code (3444)
var incoming = { Time: tim, Fob_ID: "1", Hub_ID: "0", Code: "3444" };

console.log(incoming);

//INSERT- one at a time preferably, careful with inserting same data
db.collection("smahrt_keys").insert(incoming, function(err, res) {
  if (err) throw err;
  console.log("Number of documents inserted: " + res.insertedCount);
  db.close();
});

//query by ID to get time, also if ID exists in database, then it has checked in!
var query = { ID: "1" };
db.collection("smahrt_keys").find(query).toArray(function(err, result) {
  if (err) throw err;
  console.log(result);
  db.close();
});

