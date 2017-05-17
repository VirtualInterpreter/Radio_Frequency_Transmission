/**
 * Copyright (c) 2017 Martin Birch.  All right reserved.
*/
#include "RF_Reciever.h"
#include "Arduino.h"

volatile uint16_t RF_Reciever::dataRecieved[36];
volatile uint8_t RF_Reciever::positionInArray = 0;
volatile long RF_Reciever::lastPeak = 0;
volatile bool RF_Reciever::startSignalRecieved = false;
volatile uint8_t RF_Reciever::startBitCounter = 0;
volatile uint8_t RF_Reciever::transmissionPartCounter = 0;

volatile uint8_t RF_Reciever::byteRecieved;
volatile bool RF_Reciever::hasByte = false;

volatile uint8_t RF_Reciever::recievePin = 0;
volatile uint16_t RF_Reciever::transmit_time =  500;

RF_Reciever::RF_Reciever() { }

/**
* Set the pin that should be listened to
* @param pin The Pin that should be listened to for input
*/
void RF_Reciever::setRecievingPin(const uint8_t pin) {
  pinMode(pin, INPUT);
  RF_Reciever::recievePin = pin;
}

/**
* Starts listening for inputs
* For more information about known usable pins,
* please see the documentation of the class.
*
* When using the digispark replace the line in the method with:
* attachInterrupt(RF_Reciever::recievePin, recieve, CHANGE);
*/
void RF_Reciever::startListening() {
  #if defined( __AVR_ATtinyX5__ )
  attachInterrupt(RF_Reciever::recievePin, recieve, CHANGE);
  #else
  attachInterrupt(digitalPinToInterrupt(RF_Reciever::recievePin), recieve, CHANGE);
  #endif
}

/**
* Stop listening for inputs
*
* When using the digispark replace the line in the method with:
* detachInterrupt(RF_Reciever::recievePin);
*/
void RF_Reciever::stopListening() {
  #if defined( __AVR_ATtinyX5__ )
  detachInterrupt(RF_Reciever::recievePin);
  #else
  detachInterrupt(digitalPinToInterrupt(RF_Reciever::recievePin));
  #endif
}

/**
* This tests whether the peak detected is a end peak.
* This does not necessarily mean that the transimission has ended.
* @param peakLength The Time in Microseconds since the last peak and this one
*/
bool RF_Reciever::isEndSignal(const uint16_t & peakLength) {
  return peakLength > 11 * RF_Reciever::transmit_time;
}

/**
* Called at the end of a transmission to read
* what was transmitted. This method works by reading
* the peaks on the stack.
*/
uint8_t RF_Reciever::processByteRecieved() {
  uint8_t bytesRecieved[3];

  uint8_t byteDone = 0;
  for (uint8_t i = 0; i < 3; ++i) {
    --RF_Reciever::positionInArray; // Removed High Part of End Peak, it's not informative
    bytesRecieved[byteDone] = 0; // Reset Byte

    uint8_t bitsDone = 0; // Counter for the amount of bits in the byte formed
    uint8_t peakLength = RF_Reciever::dataRecieved[RF_Reciever::positionInArray - 1] / RF_Reciever::transmit_time;
    bitsDone = peakLength - 1; // 1 bit used in First Part of End Peak Signal

    --RF_Reciever::positionInArray; // Pop the item off the top of the stack

    bool isHigh = false; // Specifies if the bit being processed is a HIGH or LOW signal
    uint8_t escapeAfter = 0; // Number specific for transmitting data of size 8 i.e. a byte (to prevent infinte loop)
    while (bitsDone < 8 && escapeAfter < 14) {
      if (RF_Reciever::positionInArray > 0) {

        isHigh = !isHigh;
        peakLength = RF_Reciever::dataRecieved[RF_Reciever::positionInArray - 1] / RF_Reciever::transmit_time;
        --RF_Reciever::positionInArray;

        if (isHigh) {
          for (uint8_t i = 0; (i < peakLength) && (bitsDone < 8); ++i) {
            if (bitsDone == 0) {
              bytesRecieved[byteDone] = 1;
            } else {
              bytesRecieved[byteDone] |= 1 << (bitsDone);
            }
            ++bitsDone;
          }
        } else {
          bitsDone += peakLength;
        }
      }
      ++escapeAfter;
    }
    ++byteDone;
  }

  if (!hasIntegrity(bytesRecieved)) {
    RF_Reciever::hasByte = false;
    RF_Reciever::byteRecieved = 0;
  } else {
    RF_Reciever::hasByte = true;
  }

  return RF_Reciever::byteRecieved;
}

/**
* Checks that the message has been recieved correctly.
* During the transmission the byte sent is repeated an
* odd number of times. If there is only one malformed byte
* then the transmission is considered a success.
* @param bytes A pointer to the array of decoded bytes
*/
bool RF_Reciever::hasIntegrity(const uint8_t * bytes) {
  RF_Reciever::hasByte = false;
  uint8_t integrityErrors = 0;

  for (uint8_t i = 1; i < 3; ++i) {
    if (*bytes != *(bytes + i)) {
      ++integrityErrors;
    } else {
      RF_Reciever::byteRecieved = *(bytes + i);
    }
  }

  if (integrityErrors > 1) {
    integrityErrors = 0;
    if (*(bytes) != *(bytes + 2)) {
      ++integrityErrors;
    }
    for (uint8_t i = 2; i < 3; ++i) {
      if (*(bytes + 1) != *(bytes + i)) {
        ++integrityErrors;
      } else {
        RF_Reciever::byteRecieved = *(bytes + i);
      }
    }
  }

  return integrityErrors <= 1;
}

/**
* Tests to see whether the begining of the message has been reached,
* and sets the appropirate flags if it has.
*/
void RF_Reciever::processingStartSignal(const uint16_t & timeSinceLastPeak) {
  // Begin Part if transmission

  if (RF_Reciever::startBitCounter == 0) {
    // First Part of begin signal check it is high
    if (digitalRead(recievePin) == HIGH) {
      ++RF_Reciever::startBitCounter;
    }
  } else {
    if (timeSinceLastPeak > 9750) { // 500us transmission time * 20 bits - 250 delay factor
      ++RF_Reciever::startBitCounter;

      if (RF_Reciever::startBitCounter == 4) {
        RF_Reciever::startSignalRecieved = true;
        RF_Reciever::dataRecieved[RF_Reciever::positionInArray++] = timeSinceLastPeak;
      }

    } else {
      RF_Reciever::startBitCounter = 0;
    }
  }
}

/**
* This method listens for hardware interrupts on the pin.
* When the start of a valid transmission is recieved, it
* is recorded onto the stack. And then decoded at the end of
* the transmission.
*/
void RF_Reciever::recieve() {
  const long currentTime = micros();
  const uint16_t timeSinceLastPeak = currentTime - RF_Reciever::lastPeak;

  if (!RF_Reciever::startSignalRecieved) {
    RF_Reciever::processingStartSignal(timeSinceLastPeak);
  } else {
    // Middle of tranmission
    RF_Reciever::dataRecieved[RF_Reciever::positionInArray++] = timeSinceLastPeak;
    if (RF_Reciever::isEndSignal(timeSinceLastPeak)) {
      ++RF_Reciever::transmissionPartCounter;
      if (RF_Reciever::transmissionPartCounter == 3) {
        // Message Recieved
        RF_Reciever::startSignalRecieved = false;
        RF_Reciever::transmissionPartCounter = 0;
        RF_Reciever::startBitCounter = 0;
        RF_Reciever::processByteRecieved();
        RF_Reciever::positionInArray = 0; // Reset stack
      }
    }
  }

  if (RF_Reciever::positionInArray >= 35) {
    RF_Reciever::startSignalRecieved = false;
    RF_Reciever::transmissionPartCounter = 0;
    RF_Reciever::startBitCounter = 0;
    RF_Reciever::positionInArray = 0;
  }

  RF_Reciever::lastPeak = currentTime;
}
