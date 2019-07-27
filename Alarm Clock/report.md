<h1> Quest 1: Psuedo-Mechanical Alarm Clock  </h1>

<h3> 1. Summary </h3>
The purpose of this quest was to create a pseudo-mechanical digital alarm clock. This required to use the esp32 board to connect 2 servos, a 14-segment alphanumeric display, and use UART to communicate to it. The project was divided equally amongst the team members to ensure agile development. 

<h3> 2. Evaluation Criteria </h3> 
The alarm clock works as specified with the sole issue of the UART not being the main form of communication between the esp32 and the user. The servos work and are accurate to display the time units they have to display (ticking every second and every minute). The digital display works correctly: it displays time accurately and blinks when the alarm is triggered. 

<h3> 3. Solution Design </h3>

**Servos:**  
The Servos arms are configured so that the second hand is split up into 60 units with a full sweep equating to one minute.  
The minute hand is split into 60 units and a full sweep equating to one hour.
We based our servo functionality on the esp-idf example given (https://github.com/espressif/esp-idf/tree/master/examples/peripherals/mcpwm/mcpwm_servo_control)  
We modified it to first work with one servo, then together with the second servo.
We set the maximum angle attainable by both servos to be 180 degrees. We would change the angle by 3 degrees (to make it 60 ticks) every time we looped, using count to determine the angle. Count was incremented by 3 as specified before. One loop was nested inside the other which meant the inner one had to finish (a whole minute had to pass) before the outer one could tick once. We made sure these ticks measured the corrects units of time, and we saw they did when we got the digital part of the clock working. Both servos and clock were in sinc. 

**User Interface:**
The user interface was attempted to be handled through UART. This way the user could communicate with the clock through the monitor. The goal for this was to have the user set the time and the alarm without having to change the code. 

**Alphanumeric Display:**    
Our display was based on the modified example code given in class (i2c.c). We initially had a hard time producing the correct output through the I2C as the Adafruit Bitmap took sometime to understand so that we could use it correctly. When we finally understood it we were able to send something to the display in hopes of seeing a number, but random parts of the display would like up. We later discovered that it was necessary to send 0x00 after every number to switch the space we wre writting to on the display. In the end, we sent an array of size 8: the even elements had the correct bitmaps to display numbers, and the odd elements had the value 0x00. We then created an array with all the correct bitmaps so we could change the values every second. 

**Clock:**  
In order to make the clock it was necessary to have the alphanumeric display working beforehand. In order to display the right time we used a for loop used to loop through the tenth units of the hour. Instead of using multiple for loops, we decided to make the payload easier for the processor and leave it at one. To make this work we used several if statements inside the loop to check for the values of the hours and minutes (represented as i, j, k, l). If none of the if statements were true then the program would just increment the values of i, j, or k depending on the previous value that they had. 

**Alarm:**  
The alarm was set after the clock was correctly set up. We created a boolean variable to determine whether the alarm was triggered or not. To check if the alarm had to be triggered, we would compare the values of i, j, k, and l that we currenlty had (the time the clock was displaying) to the values set by the alarm. If they matched, then the alarm variable would be set to true, and the display would flash for 3 seconds to make it clear that the alarm was triggered. 


<h3> 4. Sketches and Photos </h3>  
  
![](https://user-images.githubusercontent.com/27366309/46504336-a728fc80-c7fb-11e8-952e-08d144a19baa.jpg)  
  
![](https://user-images.githubusercontent.com/19481167/46507641-8a92c180-c807-11e8-8c6c-4b35a9a6f19b.jpg)  
  
<h3> 5. Concepts, modules, tools, source code </h3>
-GPIO (gpio_example_main.c)  
-Stopwatch (i2c_example_main.c)  
-14-segment alphanumeric display (i2c_example_main.c)  
-Servo (mcpwm_servo_control_example.c)  

Note: All source code was taken from the esp-idf examples library  

<h3> 6. Supporting Artifacts </h3>
  
Link to Explanation Video: https://drive.google.com/open?id=111w_IQBnn8tT8kg5PwkOQXDdTmbFWS2O  
  
Link to Short Alarm Functionality: https://drive.google.com/open?id=1DRj41KT4wN8hgj6F1e7avaOPDmyNyfqq

Github solution: https://github.com/BU-EC444/Chimienti-GarciaPlata-Sinaga/blob/report_quest1/alarm_clock.c
