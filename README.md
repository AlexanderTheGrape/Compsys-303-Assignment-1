# Compsys-303-Assignment-1

This assignment is about designing a traffic light controller on a Nios II processor using the Altera DE2-115 FPGA board.

There are 4 modes of the traffic light controller:
  - Mode 1:
    This mode is just a basic traffic light controller which controls a 4-way intersection. 
    The traffic light cycles between 6 states such that both directions are never in a Green-Green 
    or Yellow-Yellow or Yellow-Green state as these would be unsafe. There are 6 timeout values which
    control the time between state transitions.
  
  - Mode 2:
    This mode is an extension of mode 1 with the addition of pedestrian lights. There are also two button 
    inputs (Key0 and Key1) in this mode for the pedestrian lights - see 'Basic Controls' below. If a pedestrian 
    input is detected in either direction, the corresponding pedestrian light will be turned on for the duration of
    green traffic light in that direction - see 'LEDs' below.
  
  - Mode 3:
    This mode is an extension of mode 2 with the addition of configurable timeout values. A switch has been added
    as an input for this mode - see 'Other Switch Uses' below.
      - Assumptions:
        1. If six valid inputs are not received, the input is ignored ('Invalid Input' is displayed on PuTTY) and the
           traffic light controller progress with the last timeout values it has.
        2. Upon receipt of 6 valid inputs, the timeout values are updated and then held until new ones are provided.
      - Things to note when using PuTTY:
        1. Use 'control' + 'J' to send the end-of-packet character ('\n')
  
  - Mode 4:
    This mode is an extension of mode 3 with the addition of a red light camera. A button (Key2) has been added as an
    input for this mode - see 'Button Uses' below.
      - Assumptions:
        1. If a car enters in a Red-Green or Green-Red state, it is assumed that the car is entering from the green 
           light direction.
        2. A snapshot will be taken after 2 seconds of the camera being active regardless of the timeout values of 
           each state.
        3. A snapshot will not be taken if a car, which entered on a yellow light, exits within two seconds regardless 
           of the state it leaves in.
  
  
Basic Controls:
  - Selecting between modes:
    Switch 0 = mode 1 (when set high)
    Switch 1 = mode 2 (when set high)
    Switch 2 = mode 3 (when set high)
    Switch 3 = mode 4 (when set high)
    
    Default mode (upon start-up) = mode 1
    Most recently selected mode holds when all switches set low
    
  - Button Uses:
    Key 0 = North-South pedestrian light (Mode 2, 3 and 4)
    Key 1 = East-West pedestrian light (Mode 2, 3 and 4)
    Key 2 = Car entering and exiting an intersection (Mode 4)
    
  - Other Switch Uses:
    Switch 17 (set high) = Indicates new state timeout values are to be entered (Mode 3 and 4)
    Switch 17 (set low) = Continue using current state timeout values (Mode 3 and 4)
    
LEDs:
  LEDG0 = North-South Green Light
  LEDG1 = North-South Yellow Light
  LEDG2 = North-South Red Light
  LEDG3 = East-West Green Light
  LEDG4 = East-West Yellow Light
  LEDG5 = East-West Red Light
  LEDG6 = North-South Pedestrian Light
  LEDG7 = East-West Pedestrian Light
    
