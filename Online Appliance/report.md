# Quest 3: Online Appliance  #
  
#### Roberto Garcia-Plata, Christina Chimienti, Kevin Sinaga  ####
  
## Summary: ##  
For this quest we had to control a household appliance using a website. We didn't have to build the actual appliance, but design and implement control and actuation. This required for us to use a servo as our actuator and a thermistor as a sensor. We created a simple website where you can press a button to get the current measurement from the thermistor and another button to activiate the servo. We also implemented a web camera to display footage of the appliance in the website as well.

## Criteria: ##
1. We are able to send	data	across	separate	IP	network	to control the servo <br>
2. We can receive	data	across	separate	IP	network	from remote	sensor	into	web	client <br>
3. We integrated the	Rpi	webcam	into	web	client <br>
4. The last step was to integrate all of these functions together using the esp32, rpi, webcam and website <br>

## Solution Design: ##
The first part to put together this quest was to set up the router. To do that we followed the steps detailed in the same skill and received some help from the professor. After the router was set up, we had to connect our esp32 to the wifi. This only required running the simple wifi code and after that it was ready to be connected. Once this was set up it was necessary to get the servo, the thermistor, and the webcam working separately. Once we got each individual component to work, we had to integrate them all in the esp32. In order to get information from the esp32 we had to use the http protocol. To get this started, all we had to do is run the simple http example and follow the instructions. After that we used the code available in the class repo as an example of how to use get and post methods. We defined two get methods to move the servo and get the infromation from the thermistor. This was implemented through a short html client website that has some buttons to do its required actions. One button will show you the value of the thermistor, and the other one moves the servo. The webcam is displayed in the website along with the buttons through ssh into the RPi. The last thing we did was set the internal and external ports so that you can control this device from a different router (controlling it from work when the device is at home). To do this we added some port forwarding to the router using Tomato.
  
## Sketches & Photos: ##  
![Port Forwarding](https://user-images.githubusercontent.com/27366309/47939388-8bc61580-debd-11e8-9e4a-b45f773e004c.png)
  
![Device List](https://user-images.githubusercontent.com/27366309/47939392-8ff23300-debd-11e8-8b17-195fe3a41376.png)
  
RaspPi & Webcam Setup
![](https://user-images.githubusercontent.com/19481167/47940046-c466ee80-debf-11e8-974b-7180cc7af2bf.JPG)
  
Esp32 Setup
![](https://user-images.githubusercontent.com/19481167/47940049-c5981b80-debf-11e8-9b76-cfca2bdd8127.JPG)
  
Webcam Feed
![](https://user-images.githubusercontent.com/19481167/47940050-c630b200-debf-11e8-837c-c51018805839.JPG)
  
## Modules & Tools: ##  
https://github.com/espressif/esp-idf/blob/master/examples/peripherals/adc/main/adc1_example_main.c
https://github.com/espressif/esp-idf/blob/master/examples/peripherals/mcpwm/mcpwm_servo_control/main/mcpwm_servo_control_example.c
https://github.com/espressif/esp-idf/blob/master/examples/protocols/http_server/simple/main/main.c
https://github.com/espressif/esp-idf/blob/master/examples/wifi/getting_started/station/main/station_example_main.c
https://github.com/BU-EC444/ec444-repo/blob/master/simple-server/main/main.c

## Supporting Artifacts: ##
Video:
  
https://drive.google.com/open?id=18kDTXbqAkkB4KA3yA8_MUQ92fKnv8FPQ

  
