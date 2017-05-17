#ifndef _RF_Reciever_H
#define _RF_Reciever_H

#include "Arduino.h"

class RF_Reciever {
public:
  volatile static uint8_t byteRecieved;
  volatile static bool hasByte;

  volatile static uint8_t recievePin;
  volatile static uint16_t transmit_time;

  RF_Reciever();

  void setRecievingPin(const uint8_t pin);
  void startListening();
  static void stopListening();
  static void recieve();

private:
  volatile static uint16_t dataRecieved[36];
  volatile static uint8_t positionInArray;
  volatile static uint8_t startBitCounter;
  volatile static long lastPeak;
  volatile static bool startSignalRecieved;
  volatile static uint8_t transmissionPartCounter;

  static void processingStartSignal(const uint16_t & timeSinceLastPeak);
  static bool isEndSignal(const uint16_t & peakLength);
  static uint8_t processByteRecieved();
  static bool hasIntegrity(const uint8_t * bytes);
};

#endif
