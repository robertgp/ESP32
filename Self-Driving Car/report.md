# Quest 4: Self-Driving Car  #
  
#### Roberto Garcia Plata, Christina Chimienti, Kevin Sinaga  ####
  
## Summary: ##  
This quest consisted on putting together a car and making it follow the walls of a track, turning left whenever it found a beacon. The car used DC motors to advance, and a couple sensors - one on the front and one on the side - to drive along the track set by the professor. It also included an IR beacon receiver that would tell the car to turn whenever it found the transmitting signal.  

## Criteria: ##
1. Our car successfully traverses line segments <br>
2. The car successfully makes turns <br>
3. The car detects and uses beacons for navigation <br>
4. The car does not traverse the full course in one go <br>
5. The car uses PID to remain at a certain distance from the wall <br>

## Solution Design: ##
The first part consisted of putting together the car and being able to drive it with an H-bridge. This required to follow the schematic of an H-bridge and to wire it correctly with our DC motors. After that was done it was necessary to use one sensor on the side and another one on the front so that it would avoid colliding against obstacles. 
For this car, it was chosen to use a Lidar on the side and a Longrange IR on the front. The sensor on the side was implemented with a tuned PID algorithm so that it would remain at a certain distance from the wall (30 cm). If it went too close or too far, it would correct its direction to be at 30cm from the wall at all time. 
Next, the car had to turn whenever it received a signal from a transmiting beacon. The car had an IR receiver that would be looking for this beacon signal with ID (0-3), and once it found it the car would turn left. To make the car turn closer to the signal (and not as soon as it received it) it worked together with the IR Rangefinder sensor on the front to determine that it was close enough to the wall and receiving the signal before it turned. 
The demo involved having the car go through a rectangular track with beacons on the corners. 
  
## Sketches & Photos: ##  
![Front view](https://user-images.githubusercontent.com/27366309/48802599-ada50200-ecdd-11e8-8ee0-6ca1ce374088.jpg)
![Side view](https://user-images.githubusercontent.com/27366309/48802602-aed62f00-ecdd-11e8-92be-a6b3cda9e838.jpg)
![Schematic](https://user-images.githubusercontent.com/27366309/48803773-e7c3d300-ece0-11e8-9619-b1f3ede70354.jpg)

  
## Modules & Tools: ##  
IR Beacon:<br>
https://github.com/BU-EC444/ec444-repo/blob/master/ir-election/main/ir-election.c <br>
Longrange IR:<br>
https://github.com/espressif/esp-idf/blob/master/examples/peripherals/adc/main/adc1_example_main.c<br>
DC Motors:<br>
https://github.com/espressif/esp-idf/blob/master/examples/peripherals/mcpwm/mcpwm_brushed_dc_control/main/mcpwm_brushed_dc_control_example.c<br>
Lidar:<br>
https://github.com/espressif/esp-idf/blob/master/examples/peripherals/uart_async_rxtxtasks/main/uart_async_rxtxtasks_main.c<br>
PID:<br>
http://whizzer.bu.edu/guides/design-patterns/dp-pid<br>

## Pins: 
Motor 1:  
GPIO1 --> 22  
GPIO2 --> 23  
PWM --> A0  
  
Motor 2:  
GPIO1 --> 14  
GPIO2 --> 32  
PWM --> 27  
  
IR_Rangefinder:  
GPIO --> A2/34   
  
LIDAR:  
RX --> 16  
TX --> 17  
  
IR Reciver:  
GPIO --> A3/39  
  
## Supporting Artifacts: ##
Video of car on track:
https://drive.google.com/file/d/1JbD0fBA-mWMfsAPz1bx225N1Xg4dCePb/view?usp=sharing

Explanation video:
https://drive.google.com/file/d/1gaAn6_chY1gXy7tLljrgk0x9b9uVUCUW/view?usp=sharing

Main Code:  
https://github.com/BU-EC444/Quest4-Team8-Chimienti-GarciaPlata-Sinaga/blob/master/Quest4.c  
