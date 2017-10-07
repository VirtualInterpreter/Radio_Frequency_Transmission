/**
 * Copyright (c) 2017 Martin Birch.  All right reserved.
*/

/*
  Modes of Reliability:
  Unsafe - no repeats, minimum data seperation
  Safe - 3 repeats of data segments only, with clear data seperation
  Ultra Safe (Or overkill) - 3 repeats of the whole transmission, with clear data seperation

  Maximum amount of repeats is limited to 4 unless reciever transmission properties size is increased.
  */

/*
  Maximum data that can be transferred in a single continous segment is ~60 bits
*/

#include "RF_Transmitter.h"
#include "RF_Types.h"
#include "Arduino.h"

RF_Transmitter::RF_Transmitter(const reliability_mode t_mode, const uint8_t t_pin) {
  mode = t_mode;
  transmitPin = t_pin;
  transmit_time = 150;
  transmitCode = 0;
}

void RF_Transmitter::transmit(const uint8_t data) {
  switch (mode) {
    case t_unsafe:
      transmitBeginSignal();
      transmitReliabilityType();
      transmitDataTypeEncoding(data_byte);
      transmitByte(data);
      transmitEndSignal();
      break;
    case t_safe:
      transmitBeginSignal();
      transmitReliabilityType();
      transmitDataTypeEncoding(data_byte);
      
      for (uint8_t repeat = 0; repeat < 3; ++repeat) {
        transmitByte(data);
        transmitEndSignal();
      }

      break;
    default:
      break;
  }
}

/**
 * Size of int is 16 bits!
**/
void RF_Transmitter::transmit(const int data) {
  switch (mode) {
    case t_unsafe:
      transmitBeginSignal();
      transmitReliabilityType();
      transmitDataTypeEncoding(data_int);
      for (uint8_t x = 15; x >= 0 && x < 255; --x) {
        transmitIntBit(data, x);
      }
      transmitEndSignal();
      break;
    case t_safe:
      transmitBeginSignal();
      transmitReliabilityType();
      transmitDataTypeEncoding(data_int);

      for (uint8_t c = 0; c < 3; ++c) {
        for (uint8_t x = 15; x >= 0 && x < 255; --x) {
          transmitIntBit(data, x);
        }
        transmitEndSignal();
      }

      break;
    default:
      break;
  }
}

void RF_Transmitter::transmit(const long data) {
  switch (mode) {
    case t_unsafe:
      transmitBeginSignal();
      transmitReliabilityType();
      transmitDataTypeEncoding(data_long);
      for (uint8_t x = 31; x >= 0 && x < 255; --x) {
        transmitLongBit(data, x);
      }
      transmitEndSignal();
      break;
    case t_safe:
      transmitBeginSignal();
      transmitReliabilityType();
      transmitDataTypeEncoding(data_long);

      for (uint8_t repeat = 0; repeat < 3; ++repeat) {
        for (uint8_t x = 31; x >= 0 && x < 255; --x) {
          transmitLongBit(data, x);
        }
        transmitEndSignal();
      }

      break;
    default:
      break;
  }
}

/**
* Used to transmit a single bit, this method shouldn't be used on it's on to send a message.
* Least significant bit is 0.
*/
void RF_Transmitter::transmitBit(const uint8_t & data, const uint8_t & bit) {
  if ((data >> bit) & 1) {
    digitalWrite(transmitPin, LOW);
    delayMicroseconds(transmit_time);
    digitalWrite(transmitPin, HIGH);
    delayMicroseconds(transmit_time);
  } else {
    digitalWrite(transmitPin, HIGH);
    delayMicroseconds(transmit_time);
    digitalWrite(transmitPin, LOW);
    delayMicroseconds(transmit_time);
  }
}

void RF_Transmitter::transmitIntBit(const uint16_t & data, const uint8_t & bit) {
  if ((data >> bit) & 1) {
    digitalWrite(transmitPin, LOW);
    delayMicroseconds(transmit_time);
    digitalWrite(transmitPin, HIGH);
    delayMicroseconds(transmit_time);
  } else {
    digitalWrite(transmitPin, HIGH);
    delayMicroseconds(transmit_time);
    digitalWrite(transmitPin, LOW);
    delayMicroseconds(transmit_time);
  }
}

void RF_Transmitter::transmitLongBit(const uint32_t & data, const uint8_t & bit) {
  if ((data >> bit) & 1) {
    digitalWrite(transmitPin, LOW);
    delayMicroseconds(transmit_time);
    digitalWrite(transmitPin, HIGH);
    delayMicroseconds(transmit_time);
  } else {
    digitalWrite(transmitPin, HIGH);
    delayMicroseconds(transmit_time);
    digitalWrite(transmitPin, LOW);
    delayMicroseconds(transmit_time);
  }
}

void RF_Transmitter::transmitByte(const uint8_t & data) {
  for (uint8_t x = 7; x >= 0 && x < 255; --x) {
    transmitBit(data, x);
  }
}

/**
* Transmits a Long begin transmit signal,
* used when there are large gaps between
* transmissions as this results in a large
* amount of interferance.
*/
void RF_Transmitter::transmitBeginSignal() {
  digitalWrite(RF_Transmitter::transmitPin, HIGH);
  delayMicroseconds(RF_Transmitter::transmit_time * 65);
  digitalWrite(RF_Transmitter::transmitPin, LOW);
  delayMicroseconds(RF_Transmitter::transmit_time * 65);
  digitalWrite(RF_Transmitter::transmitPin, HIGH);
  delayMicroseconds(RF_Transmitter::transmit_time * 65);
  digitalWrite(RF_Transmitter::transmitPin, LOW);
  delayMicroseconds(RF_Transmitter::transmit_time * 1);
}

/**
* Marks the end of a byte transmitted, it does not
* necessarily mark the end of a transmission.
* (It has the length of 12 to distinguish it from other singals)
*/
void RF_Transmitter::transmitEndSignal() {
  digitalWrite(RF_Transmitter::transmitPin, HIGH);
  delayMicroseconds(RF_Transmitter::transmit_time * 6);
  digitalWrite(RF_Transmitter::transmitPin, LOW);
  delayMicroseconds(RF_Transmitter::transmit_time * 1);
}

void RF_Transmitter::transmitReliabilityType() {
  for (uint8_t y = 1; y >= 0 && y < 255; --y) {
    transmitBit(mode, y);
  }
}

void RF_Transmitter::transmitDataTypeEncoding(const data_type & type) {
  for (uint8_t z = 1; z >= 0 && z < 255; --z) {
    transmitBit(type, z);
  }
}
