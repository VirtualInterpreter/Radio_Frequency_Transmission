# Radio_Frequency_Transmission
This library contains code for the XD-RF-5V &amp; XD-FST to allow for radio communication between two devices.
It is designed for transmitting a small amount of data, whilst using only a small amount of memory.

## General Information:
* If you are planning to make your own library or modify this one be aware that if you send a LOW Signal for greater than <300~350ms, the next high Signal you send will have a great deal of interference before it.
* This library is only able to send about 12.5 B/s at the current time. This is was due to the following constraints:
  1. The Digispark only has 256 bytes of memory and as I store the peaks and process them at the end of the transmission, the transmission needs to be small or it will use a large amount of memory.
  2. The Digispark only runs at 16 Mhz so the processing of the interrupts need to be very fast, hence the transmission is largely processed at the end of the transmission.
* The transmitter and reciever have been split up into different files to allow for more efficient use of memory.


## How to set up your devices:

### The transmitter: (XD-FST aka The three pins one)

* Connect the Ground (GND) to the Ground on your device.
* Connect the VCC to the 5V Pin on your device
* Connect the Data pin to the Pin on your device you want to transmit from.

### The reciever: (XD-RF-5V aka the four pins one)

* Whilst the reciever has four pins you only need to use three. (Either of the data pins can be chosen)
* Connect the Ground (GND) to the Ground on your device.
* Connect the VCC to the 5V Pin on your device
* Then connect the Data pin to the pin on your device that has a hardware interrupt.


## How to use:
### Known Usable Pins: (Receiving)
* Digispark (Standard not Pro):
  * Only Pin 2 has a hardware interrupt, however, correlates to Pin 0 in the Arduino Library.
  * Therefore to use Pin 2, you must set Pin 0 to be the listening pin.
* Arduino Nano (atmega328p)
  * Pins 2 & 3 have hardware interrupts, these are labelled pin D2 & D3 respectively on the Nano.


## Format of Transmission:
* The current transmission speed for a signal bit is set to 500 microseconds
* To distinguish the transmission from any interference I send a starting signal in the following format: 
  * 20xHigh, 20xLOW, 20xHIGH, 1xLOW. Where each 20x represents 20 bits transferred.
* The byte is transmitted as standard HIGH or LOW, with an End Peak at the end which has the format:
  * 1xLOW, 12xHIGH, 1xLOW
* The second stage of is repeat twice more, so byte and the end peak is sent is sent 3 times in total.
* A transmission of `00110011` would result in the following signal:
  * (20xH, 20xL, 20xH, 1xL), (2xL, 2xH, 2xL, 2xH, (1xL, 12xH, 1xL))x3 


## Links you might find useful
[RC-Switch](https://github.com/sui77/rc-switch)
[Project using RC-Switch](https://sites.google.com/site/summerfuelrobots/arduino-sensor-tutorials/rf-wireless-transmitter-receiver-module-433mhz-for-arduino)

# Copyright (c) 2017 Martin Birch.  All right reserved.
