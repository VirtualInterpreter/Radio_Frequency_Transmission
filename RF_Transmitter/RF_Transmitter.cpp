/**
 * Copyright (c) 2017 Martin Birch.  All right reserved.
*/
#include "RF_Transmitter.h"
#include "Arduino.h"

volatile uint8_t RF_Transmitter::transmitPin = 0;
volatile uint16_t RF_Transmitter::transmit_time =  500;

RF_Transmitter::RF_Transmitter() { }

/**
* Total Length of transmition:
* 0.5*20 + 0.5*20 + 0.5*20 + 1 + (0.5*8 + 1 + 0.5*12 + 1) * 3 + 10 = 77 milliseconds
* Data is encapulated by two peaks, with one marking the begining
* and the other marking the end of the data stream.
*/
void RF_Transmitter::transmit(const uint8_t pin, const uint8_t data) {
  pinMode(pin, OUTPUT);
  RF_Transmitter::transmitPin = pin;
  transmitBeginSignal(); // Transmit Begin signal

  for (uint8_t repeat = 0; repeat < 3; ++repeat) {
    for (int i = 7; i >= 0; --i) {
      if ((data >> i) & 1) {
        transmitBit(RF_Transmitter::transmitPin, HIGH);
      } else {
        transmitBit(RF_Transmitter::transmitPin, LOW);
      }
    }

    transmitEndSignal();
  }
  delay(10);
}

/**
* Used to transmit a single bit, this method shouldn't be used on it's on to send a message.
*/
void RF_Transmitter::transmitBit(const uint8_t pin, const uint8_t data) {
  digitalWrite(pin, data);
  delayMicroseconds(RF_Transmitter::transmit_time);
}

/**
* Transmits a Long begin transmit signal,
* used when there are large gaps between
* transmissions as this results in a large
* amount of interferance.
*/
void RF_Transmitter::transmitBeginSignal() {
  digitalWrite(RF_Transmitter::transmitPin, HIGH);
  delayMicroseconds(RF_Transmitter::transmit_time * 20);
  digitalWrite(RF_Transmitter::transmitPin, LOW);
  delayMicroseconds(RF_Transmitter::transmit_time * 20);
  digitalWrite(RF_Transmitter::transmitPin, HIGH);
  delayMicroseconds(RF_Transmitter::transmit_time * 20);
  digitalWrite(RF_Transmitter::transmitPin, LOW);
  delayMicroseconds(RF_Transmitter::transmit_time * 1);
}

/**
* Marks the end of a byte transmitted, it does not
* necessarily mark the end of a transmission.
* (It has the length of 12 to distinguish it from other singals)
*/
void RF_Transmitter::transmitEndSignal() {
  digitalWrite(RF_Transmitter::transmitPin, LOW);
  delayMicroseconds(RF_Transmitter::transmit_time * 1);
  digitalWrite(RF_Transmitter::transmitPin, HIGH);
  delayMicroseconds(RF_Transmitter::transmit_time * 12);
  digitalWrite(RF_Transmitter::transmitPin, LOW);
  delayMicroseconds(RF_Transmitter::transmit_time * 1);
}
