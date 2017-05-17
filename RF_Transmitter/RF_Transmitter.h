#ifndef _RF_Transmitter_H
#define _RF_Transmitter_H

#include "Arduino.h"

class RF_Transmitter {
public:
  volatile static uint8_t transmitPin;
  volatile static uint16_t transmit_time;

  RF_Transmitter();

  void transmit(const uint8_t pin, const uint8_t data);

private:
  void transmitBeginSignal();
  void transmitBit(const uint8_t pin, const uint8_t data);
  void transmitEndSignal();
};

#endif
