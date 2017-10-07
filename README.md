# Simple Radio Frequency Encoding for the Arduino & Digispark

Transmission speed for bytes is around 20Bytes/s, however, if multiple bytes are transformed into an unsigned long first it is possible to send around 66Bytes/s.

## Setting Up the devices
### The transmitter: (XD-FST)
*	Connect the Ground (GND) to the Ground on your device.
*	Connect the VCC to the 5V Pin on your device
*	Connect the Data pin to the Pin on your device you want to transmit from.
### The receiver: (XD-RF-5V)
*	Whilst the receiver has four pins you only need to use three. (Either of the data pins can be chosen)
*	Connect the Ground (GND) to the Ground on your device.
*	Connect the VCC to the 5V Pin on your device
*	Then connect the Data pin to the pin on your device that has a hardware interrupt.
### Known Usable Pins: (Receiving only as transmitting can use any digital pin)
*	Digispark (Standard not Pro):
  *	Only Pin 2 has a hardware interrupt, however, correlates to Pin 0 in the Arduino Library. Therefore, to use Pin 2, you must set Pin 0 to be the listening pin.
*	Arduino Nano (atmega328p)
  * Pins 2 & 3 have hardware interrupts these are labelled pin D2 & D3 respectively on the Nano.

## Extra Project Info

### Constraints:
*	Only possible to have to states either HIGH or LOW, so varying the voltage could not be used to encode more data. This is due to the Arduino Library only detecting two states.
*	When resting for more than 300ms transmitting a LOW signal, the next signal will have a great deal of interference on the receiving end. (This was detected using an oscilloscope)

### Existing Encodings
The two main types of encoding I considered where:
*	Manchester Encoding (as per IEEE 802.3) – This consists of transmitting a `0` as a `10` and a `1` as `01`
*	Return to Zero – The signal rests at `0` and the transmission is a `0` or `1`
Both types would be ineffective on their own due to the amount of inference received.
After implementing a simple Return to Zero Encoding Algorithm I learnt it is not sufficient for transmitting a large quantity of data using the transmitters and receivers due to bit drifting occurring between the state changes vs continuous LOW/HIGH streaks. Additionally, the Transmitter takes 20us to change state, thus, over large streaks of HIGH or LOW there can be more bits detected than transmitted if the receiver isn’t aware of this. However, a state change on the receiver’s end takes 10us, thus, transmitting a `010` at a rate of 8bits/ms would result in the `1` having a peak length of 16% less than it should.

### The Interference
If the transmitter is low for >300ms, a large amount of interference received. The actual transmission doesn’t seem to be affected by the inference, however, the transmission itself needs to be clearly identifiable from the interference. After analysing a large quantity of this type of interference it seems that the signal does not stay at a LOW state for a long a time and the HIGH states can be haphazard. Therefore, a begin signal with a large LOW state was used so it can be identified clearly from the interference.

### Encoding Used
My Resulting Encoding uses the following format:
1.	Begin Signal
2.	Reliability Type
3.	Data Type Contained
4.	Data
5.	End Signal
6.	Data
7.	End Signal
8.	Data
9.	End Signal

* The reliability type, data type, and data are all in Manchester Encoding.
* The Begin Signal – This starts at a HIGH for 10ms, then goes LOW for 10ms and then HIGH again for another 10ms.
* Reliability Type – This is a two-bit field, of which two types have been defined, the other two are left for custom implementations.
* Data Type – This is also a two-bit field, it has 3 defined types (byte, int, or long), the other is a custom data type for other implementations.
* Data – The data transmitted in Manchester Encoding.
* End Signal – Consists of a HIGH for 4 bits
