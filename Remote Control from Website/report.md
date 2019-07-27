# Quest 6: Race to find the Beacons/Escape the Course!  #
  
#### Roberto Garcia Plata, Christina Chimienti, Kevin Sinaga  ####
  
## Summary: ##  
This quest required to control a car using a website. This website needed to have controls to move the car forward, backward, left, right, and to stop it. Since the user is telling the car how to move, it was necessary to add a webcam to the car whose video would be shown in the website along with the car controls. The purpose of building the car like this is so the car goes around a course collecting pieces of code from beacons. Once it has all four pieces of the code it would concatenate them and send them as a message to the website. This message is an URL to another website and that is how you escape the course. 


## Criteria: ##
- Car	is controlled remotely from a website and	drives successfully with forward, right, left and backward controls
- Car visits every beacons and decodes the message
- Video and car controls are incorporated in the same website
- Front collision avoidance using an ultrasonic sensor
- Message is catenated in program and received in browser (see photo)
- Bonus feature for doing a fast forward driving to cover large distances faster


## Solution Design: ##
The first thing that we built was the website to control the car. The website has 5 buttons to control it: forward, left, right, backward and stop. This buttons were made using form actions. These simple forms would send a get request to an IP address with an action specified in the url. This would be received by the car that had wifi and http modules in it. The wifi allowed it to connect to the same router that our computer was using and the http module would create a server. This server used `httpd_uri_t` to trigger certain handlers according to the url that was passed to it from the get request in our website. These handlers would in turn trigger a specific function which is what allowed the car to move in the directions that we specified. The website also displayed the video feed of a webcam that was connected to a raspberry pi. There were two things that we needed to change on the pi to get livestream from the webcam and that was `framerate` = 100 and `stream_max_rate` = 100. This way you can see where the car is going at the same time that you are controlling it. We chose a bird's eye view when positioning the webcam. The last thing we incorporated in the car to make it move adequately was a front ultrasonic sensor to avoid collisions. This senosor makes the car stop when its at a 20cm distance from any obstacle. 
  
In order to receive the beacons properly, we started with the example given `ir-car-beacon-escape.c`. The receive task was modified to have seen flags per beacon (seen under the received rxID). If a beacon is seen for the first time, the incoming message is saved once and one flag is set to true. This is done for all four beacons and all four flags. After receiving four flags, the four messages are then concatenated in order and sent to /message using form actions. The message will only show if all four beacons are hit. 

## Sketches & Photos: ##  
Website: 
![](https://user-images.githubusercontent.com/19481167/50028263-13269e80-ffbd-11e8-9dce-11ffcb370b1f.PNG)  

Webcam:  
![](https://user-images.githubusercontent.com/19481167/50028234-f7bb9380-ffbc-11e8-976c-c82085a35c1f.png)  
  
Message:  
![](https://user-images.githubusercontent.com/19481167/50028237-f9855700-ffbc-11e8-87bc-b560eb1d23ad.png)  
![](https://user-images.githubusercontent.com/19481167/50028268-19b51600-ffbd-11e8-9942-ac2d84513a42.PNG)  
  
Website--> /message:  
![](https://user-images.githubusercontent.com/27366309/50026850-43b80980-ffb8-11e8-9ce6-8282b0adc4d0.png)  
Car:  
![](https://user-images.githubusercontent.com/19481167/50028246-073adc80-ffbd-11e8-9085-b52a1746bf14.PNG)  
  
## Pins:    ##
- Receiver   -->  39  
- Ultrasonic -->   Trigger 17   Echo 16   
- LED        -->  15  
  
## Modules & Tools: ##  
Ultrasonic Sensor: https://github.com/espressif/esp-idf/tree/master/examples/peripherals/uart <br>
Car Motors: https://github.com/espressif/esp-idf/tree/master/examples/peripherals/mcpwm/mcpwm_brushed_dc_control <br>
HTTPD: https://github.com/espressif/esp-idf/tree/master/examples/protocols/http_server/simple <br>
IR RX: https://github.com/BU-EC444/ec444-repo/tree/master/ir-car-beacon-escape
  
## Supporting Artifacts: ##
### Videos ###
Demo 1/2: https://drive.google.com/file/d/1E3_RQS_nDAPVp7gev561HtdnYKh8fYbe/view?usp=sharing <br>
Demo 2/2: https://drive.google.com/file/d/1z573GoQwTnQQrDj2V0e6RBmIi8CVZ8VL/view?usp=sharing

### Code ###
Website: https://github.com/BU-EC444/Quest6-Team8-Chimienti-GarciaPlata-Sinaga/blob/master/remote_website.html <br>
Car: https://github.com/BU-EC444/Quest6-Team8-Chimienti-GarciaPlata-Sinaga/blob/master/REMOTE_CAR_BEACON.c
