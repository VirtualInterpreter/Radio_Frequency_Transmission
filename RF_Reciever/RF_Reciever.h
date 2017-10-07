#ifndef _RF_Reciever_H
#define _RF_Reciever_H

#include "Arduino.h"
#include "RF_Types.h"

union data_properties {
  uint8_t setAll;
  struct {
    uint8_t hasData : 1;
    uint8_t validDataIndex : 1;
    uint8_t data_type : 2;
  };
};

union transmission_properties {
  uint16_t setAll;
  struct {
    uint8_t data_type : 3;
    uint8_t reliability : 3;
    uint8_t transmissionID : 5;
    uint8_t transmissionPartCounter : 5;
  };
};

union transmission_progress {
  uint8_t setAll;
  struct {
    uint8_t hasStartSignal : 1;
    uint8_t hasDataType : 1;
    uint8_t hasReliability : 1;
    uint8_t hasTransmissionID : 1;
    uint8_t hasSeperator : 1;
    uint8_t innerPeak : 1;
    uint8_t isBitOne : 1;
    uint8_t padding : 1;
  };
};

class RF_Reciever {
public:
  RF_Reciever();

  void setRecievingPin(const uint8_t pin);
  void startListening();
  
  bool hasByte() const;
  bool hasInt() const;
  bool hasLong() const;
  
  uint8_t getByteAndRemove();
  unsigned int getIntAndRemove();
  unsigned long getLongAndRemove();
  
  static void stopListening();
  static void recieve();

private:
  volatile static uint32_t dataRecieved[3];
  volatile static uint8_t recievePin;
  volatile static uint8_t transmit_time;
  volatile static long lastPeak;

  volatile static data_properties d_properties;
  volatile static transmission_properties t_properties;
  volatile static transmission_progress t_progress;

  volatile static uint8_t t_bitCounter;

  static void processingStartSignal(const uint16_t & timeSinceLastPeak);
  static uint8_t processByteRecieved();
  static bool hasIntegrity();
  static void decodeReliability();
  static void decodeDataType();
  static void decodeDataSegment();
};

#endif
