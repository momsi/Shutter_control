# Shutter control

This repo contains all code used for the new shutter control unit for the Arc-PVD in the NETZ 2.48 lab.

## Operation
### Before use while processing
* Switch on the power supply
* confirm correct shutter movement
* calibrate the shutter if necessary 
  * adjust the open position by rotating the motor by hand
  * open and close the shutter
  * adjust position again

### Manual Mode
In manual mode the shutter opens by depressing the trigger switch. The time the shutter is open will be shown in seconds on the display. Close the shutter by depressing the trigger switch again. The diplay will reset to "manual".

### Auto mode
In auto mode, the shutter opens and closes automatically after a predetermined time.

To set the time, use the keypad:
* reset the timer by pressing \*
* enter the desired time in seconds
* confirm by pressing \#
* the set time will be shown in the display

To open the shutter, press the trigger switch. The shutter opens and the time open will be counted down in the display. The shutter closes automatically when the timer reaches 0.

### Emergency abort
The red emergency abort switch is only active when the shutter is opened. This button is intended to be used in case the program is unresponsive, __this is not a "_close the sutter_" switch__. 
The emergency abort switch s attached as a hardware interrupt and triggers the following routine:

1. Attempt to close the shutter
2. Disable motor power (so the motor can be closed manually in case of hardware failure)
3. Display ABORT
4. Delete and reset all variables
5. Reset the program

Again: __This is not a "_close the sutter_" switch__ and only to be used in case of 

## Hardware Conponents

Components    |  Used
------------- | -------------
Voltage regulator:   | L7806CV
Controller:          | Arduino Nano (ATmega328p)
Stepper controller:  | A4988
Stepper motor:       | QSH4218-35-10-027
Display              | SSD1306 (128x64)

Also some generic switches and a 4x4 Keypad

## Pinout / wiring
### Arduino
Pin    |  Connected to
------------- | -------------
D5-D12 | Keypad (left to right)
D4 | Mode Switch
D3 | Trigger
D2 | Emergency Abort
Upper GND | GND: Mode-Switch, Trigger, Emergency Abort 
A0 | Stepper Controller DIR 
A1 | Stepper Controller STEP
A2 | Stepper Controller ENABLE
A4 | Display SDA
A5 | Display SCL
5V | Display VCC, Stepper Controller VDD
Lower GND | Display GND, Stepper Controller GND
Vin | 6V supply from voltage regulator

### Voltage regulator

Pin    |  Connected to
------------- | -------------
1 (left) | 12V IN, Stepper Controller VMOT
2 (middle) | Arduino GND, Stepper Controller Motor GND
3 (right) | Arduino Vin

### Stepper Controller

Pin    |  Connected to
------------- | -------------
ENABLE | Arduino A2
RST | short to SLEEP
SLEEP | short to RST
STEP  | Arduino A1
DIR  | Arduino A0
VMOT | 12V IN
GND | Voltage regulator GND
2A, 2B | Motor Coil 2
1A, 1B | Motor Coil 1
VDD | Arduino 5V
GND | Arduino GND

### Display 

Pin    |  Connected to
------------- | -------------
VCC | Arduino 5V
GND | Arduino GND
SDA | Arduino A4
SCL | Arduino A5
