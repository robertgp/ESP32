# Quest 5: Smahrt Key  #
  
#### Roberto Garcia Plata, Christina Chimienti, Kevin Sinaga  ####
  
## Summary: ##  
This quest consisted on implementing a secure login system. There are multiple fobs which act as keys to the system with a red led indicated locked and a green led indicated unlocked. Each fob has a distinctive ID and code that will be sent to a single hub via IR TX/RX. The hub will then send a message via UDP that includes the fob id and the code it received together with its own hub id to the server. The server will check if the id is valid and upon verification it will add all the information it received along with a time stamp to a relational database. This database will then receive a query to output in a website all the previous logs (should they exist) of the fob that attempted to login. If all of these processes are successful, the fob that sent the initial message will have an LED turn on indicating that it has been granted access. 

## Criteria: ##
- We are able have our fob relay its id and code	to our security	hub >	the hub	sends	this id and	code	to the server > the server	responds to	the fob > the	fob	green	line turns	on.  
- We are able to log fob key access to the database.  
- The database is hosted on the Raspberry Pi.
- We are able to see and access real time active unlocked fobs as well as unlocked fob history and timestamp shown on our web interface.
- Our system design is able to accept over three fob keys.
- See section *Security Vulnerabilities* for proposed mitigation.


## Solution Design: ##
Our FOB was created using IR TX and HTTPD. It transmits a start byte, it's ID, and a code. These are transmitted only when the button is pressed and a hardware interrupt takes place. The red led remains off until a command from the server is sent via HTTPD and the green light is turned on, specified by the `ctrl_put_handler()` function in the FOB code. From there, the IR RX hub (0) which contains only a receiver pin, will constantly be communicating to the server sending either an empty message or one containing the FOB id, hub id, and code in a form that is parsed serverside. The hub is communicating to the server via UDP client and receives `okay` messages sent back from the server if they are connected. Since the server/db is hosted in the Pi, we used the Pi address `192.168.1.108` for UDP. 
  
The server was created using javascript. This server would be listening until it received a message from the hub. As soon as it received a message from the hub it would check if the message came with a valid FOB ID. If the ID was valid the server would insert that log attempt into the database (along with the time, hub id and code) and then query the database for other times that ID has attempted to login. The client can then press a button on the website that will show the previous recorded logins of that FOB if they exist, also indicating the FOB's presence. This works by sending a get request to our server that will get this data. Finally, the server would send a put request to the FOB that sent the message to turn its LED on if the code matches our set pin (4444). This is done through a put request using curl. 

## Security Vulnerabilities: ##
-   Since our authentication system consists of only checking for a security pin serverside, there are a lot of design choices that can be altered to make our system more secure. Here is one example situation: If the FOB esp32 has wifi capabilities and it's ip address is known as well as the exact address that is curled, technically anyone can sent a command to change the leds. To mitigate this potential situation, one could change the FOB code to only accept an HTTPD put request from a specific source. 
- From a cybersecurity perspective, an infected payload delivered to the server by the hub could easily be a problem because we did not put any mechanisms to prevent this in place. One fix to this would be to make sure that information received by the hub is actually coming from a known FOB. This could be implemented by having the FOB transmit more information to the hub, including maybe a secure passcode, ideally one that changes over time similar to an RSA token or another form of two-factor authentication.
  
## Sketches & Photos: ##  
Initial Schematic:  
![](https://user-images.githubusercontent.com/19481167/49899973-4507fb80-fe2b-11e8-8b6e-b0df40926615.JPG)  
Website:  
![](https://user-images.githubusercontent.com/19481167/49890454-16c9f200-fe12-11e8-972a-5291011f5102.png)  
Logs:  
![](https://user-images.githubusercontent.com/19481167/49890452-1598c500-fe12-11e8-96b9-8d2e690948d9.png)   
Green Light on FOB 1 with Hub & Rpi:  
![](https://user-images.githubusercontent.com/19481167/49890492-2e08df80-fe12-11e8-8116-dc19e7ee9e70.JPG)  
FOB 2:  
![](https://user-images.githubusercontent.com/19481167/49890494-2f3a0c80-fe12-11e8-9fcd-b8dcaf9623ed.JPG)  
  
## Modules & Tools: ##  
TingoDB: http://www.tingodb.com/  
IR RX/TX:  https://github.com/BU-EC444/ec444-repo/tree/master/ir-election  
UDP Client: https://github.com/espressif/esp-idf/blob/222a7118a92e227a7954e8dc5cd955b6788e0221/examples/protocols/sockets/udp_client/README.md  
HTTPD: https://github.com/espressif/esp-idf/tree/master/examples/protocols/http_server/simple  

  
## Supporting Artifacts: ##
Videos: <br>
Explanation: https://drive.google.com/file/d/1sSN8xcAcB8pJf_KiYmWVkWmwSP-TcmIe/view?usp=sharing <br>
Demo: https://drive.google.com/file/d/1xSaTNMCVNHRLcusffDgMIVqJ57998Jey/view?usp=sharing  
#### Code: ###  
Hub: https://github.com/BU-EC444/Quest5-Team8-Chimienti-GarciaPlata-Sinaga/blob/master/HUB.c  
Fob (1): https://github.com/BU-EC444/Quest5-Team8-Chimienti-GarciaPlata-Sinaga/blob/master/FOB.c  
Server: https://github.com/BU-EC444/Quest5-Team8-Chimienti-GarciaPlata-Sinaga/blob/master/server2.js  
DB & Server: https://github.com/BU-EC444/Quest5-Team8-Chimienti-GarciaPlata-Sinaga/blob/master/server.js   
Website: https://github.com/BU-EC444/Quest5-Team8-Chimienti-GarciaPlata-Sinaga/blob/master/client.html  
   
