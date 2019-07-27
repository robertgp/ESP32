# Quest 2: Swiss Army Tape Measure  #
  
#### Roberto Garcia-Plata, Christina Chimienti, Kevin Sinaga  ####
  
## Summary: ##  
The goal of this quest was to simultaneously implement multiple measuring devices in our kit.
Once implemented, the incoming data from these devices was charted in a graphical way via JavaScript.
In order to fulfill this, we used the Ultrasonic, LIDAR, IR, and Wheel Speed sensors via UART and ADC on the ESP32.  

## Criteria: ##
1) IR Sensor input in meters  
2) LIDAR Sensor input in meters  
3) Ultrasonic Range Sensor 1 input in meters  
4) Ultrasonic Range Sensor 2 input in meters  
5) Wheel Speed Sensor input in meters/sec  
6) Results of IR, LIDAR, and Ultrasonic graphed using Canvas.js & Node.js  

## Solution Design: ##
  GPIO Pins:  
  * A2/34- IR  
  * RX/16- LIDAR_TX  
  * TX/17- LIDAR_RX  
  * 32/A7- Echo Ultrasonic 2  
  * 14/A6- Trig Ultrasonic 2  
    
  Focused on UART module for LIDAR, and Ultrasonic 2 Sensors.
  Focused on ADC module for IR, Ultrasonic 1 and Wheel Speed Sensors.
  Use of RX/TX and intaking data from multiple GPIO pins simultaneously. One of the challenges we first faced was understanding the receiving signal and transmitting signal and how they function in our various sensors.
  
  The method followed to come up with the final solution was to first make every sensor work individually. Once we had all sensors working, we tried to plot the data from Lidar using Canvas.js and Node.js. This worked immediately since we were given the code to plot these values. Then it was necessary to make sure we could plot all sensors simultaneously, so we modified the code given to us to accomodate for this. Once these modification were made the last thing was to be able to tell where the information was coming from, and we did that by adding a unique identifier to every sensor when they send the data. The solution is all the previous steps put together. 
  
## Sketches & Photos: ##  
-LIDAR, IR, Ultrasonic Range Sensor 2  
![](https://user-images.githubusercontent.com/19481167/47171301-5ffc3a80-d2d6-11e8-8e64-e93368067ad0.JPG)
![](https://user-images.githubusercontent.com/19481167/47171305-61c5fe00-d2d6-11e8-8848-61a679a7f4f7.JPG)
![](https://user-images.githubusercontent.com/19481167/47171306-62f72b00-d2d6-11e8-87bc-b918d3851598.JPG)  
  
-Ultrasonic Range Sensor 1  
![](https://user-images.githubusercontent.com/19481167/47172270-fdf10480-d2d8-11e8-8941-2b85c9396ef2.JPG)  
  
-Plot  
![](https://user-images.githubusercontent.com/19481167/47173024-ee72bb00-d2da-11e8-9cd1-d7e1029056d0.png)
![](https://user-images.githubusercontent.com/19481167/47173027-ef0b5180-d2da-11e8-91b2-9a29f6cedcc0.png)
  
-Schematic
![](https://user-images.githubusercontent.com/19481167/47189828-6a392b80-d30c-11e8-930a-67164c2c1bab.JPG)
  
## Modules & Tools: ##  
https://github.com/espressif/esp-idf/blob/master/examples/peripherals/adc  
https://github.com/espressif/esp-idf/tree/master/examples/peripherals/uart_async_rxtxtasks 
https://github.com/JRodrigoTech/Ultrasonic-HC-SR04/tree/master/Ultrasonic  

## Supporting Artifacts: ##
Video:
  https://drive.google.com/open?id=1X4pZ3bo3nc8BfIXDxGuCPgJVKahlsmCi  

  
