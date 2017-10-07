#ifndef _RF_Transmitter_H
#define _RF_Transmitter_H

#include "Arduino.h"
#include "RF_Types.h"

class RF_Transmitter {
public:
  reliability_mode mode;
  uint8_t transmitPin;
  uint16_t transmit_time;
  uint8_t transmitCode;

  RF_Transmitter(const reliability_mode t_mode, const uint8_t t_pin);

  void transmit(const uint8_t data);
  void transmit(const int data);
  void transmit(const long data);

private:
  void transmitBeginSignal();
  void transmitBit(const uint8_t & data, const uint8_t & bit);
  void transmitIntBit(const uint16_t & data, const uint8_t & bit);
  void transmitLongBit(const uint32_t & data, const uint8_t & bit);
  void transmitByte(const uint8_t & data);
  void transmitReliabilityType();
  void transmitDataTypeEncoding(const data_type & type);
  void transmitEndSignal();
};

#endif
