/**
 * Copyright (c) 2017 Martin Birch.  All right reserved.
 * Needs at least 35 Bytes of RAM
*/
#include "RF_Reciever.h"
#include "Arduino.h"

volatile uint32_t RF_Reciever::dataRecieved[3];
volatile uint8_t RF_Reciever::recievePin = 0;
volatile uint8_t RF_Reciever::transmit_time =  150;
volatile long RF_Reciever::lastPeak = 0;

volatile data_properties RF_Reciever::d_properties = { setAll: 0 };
volatile transmission_properties RF_Reciever::t_properties = { setAll: 0 };
volatile transmission_progress RF_Reciever::t_progress = { setAll: 0 };

volatile uint8_t RF_Reciever::t_bitCounter = 0;

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
*/
void RF_Reciever::stopListening() {
  #if defined( __AVR_ATtinyX5__ )
  detachInterrupt(RF_Reciever::recievePin);
  #else
  detachInterrupt(digitalPinToInterrupt(RF_Reciever::recievePin));
  #endif
}

bool RF_Reciever::hasByte() const {
  return RF_Reciever::d_properties.hasData && RF_Reciever::d_properties.data_type == data_byte;
}

bool RF_Reciever::hasInt() const {
  return RF_Reciever::d_properties.hasData && RF_Reciever::d_properties.data_type == data_int;
}

bool RF_Reciever::hasLong() const {
  return RF_Reciever::d_properties.hasData && RF_Reciever::d_properties.data_type == data_long;
}

uint8_t RF_Reciever::getByteAndRemove() {
  RF_Reciever::d_properties.setAll = 0;
  return (uint8_t) RF_Reciever::dataRecieved[RF_Reciever::d_properties.validDataIndex];
}

unsigned int RF_Reciever::getIntAndRemove() {
  int number = 0;
  for (uint8_t i = 15; i >= 0 && i < 255; --i) {
    if ((RF_Reciever::dataRecieved[RF_Reciever::d_properties.validDataIndex] >> i) & 1) {
      number |= 1 << i;
    }
  }
  RF_Reciever::d_properties.setAll = 0;
  return (unsigned int) RF_Reciever::dataRecieved[RF_Reciever::d_properties.validDataIndex];
}

unsigned long RF_Reciever::getLongAndRemove() {
  long number = 0;
  for (uint8_t i = 31; i >= 0 && i < 255; --i) {
    if ((RF_Reciever::dataRecieved[RF_Reciever::d_properties.validDataIndex] >> i) & 1) {
      number |= 1 << i;
    }
  }
  RF_Reciever::d_properties.setAll = 0;
  return number;
}

/**
* Checks that the message has been recieved correctly.
* During the transmission the byte sent is repeated an
* odd number of times. If there is only one malformed byte
* then the transmission is considered a success.
*/
bool RF_Reciever::hasIntegrity() {
  RF_Reciever::d_properties.hasData = false;

  if (RF_Reciever::t_properties.reliability == t_safe) {
    return (RF_Reciever::dataRecieved[0] == RF_Reciever::dataRecieved[1]) ||
      (RF_Reciever::dataRecieved[1] == RF_Reciever::dataRecieved[2]) ||
      (RF_Reciever::dataRecieved[0] == RF_Reciever::dataRecieved[2]);
  } else {
    return true;
  }

}

/**
* Tests to see whether the begining of the message has been reached,
* and sets the appropirate flags if it has.
*/
void RF_Reciever::processingStartSignal(const uint16_t & timeSinceLastPeak) {
  // Begin Part if transmission

  if (RF_Reciever::t_bitCounter == 0) {
    // First Part of begin signal check it is high
    if (digitalRead(RF_Reciever::recievePin) == HIGH) {
      ++RF_Reciever::t_bitCounter;
    }
  } else {
    if (timeSinceLastPeak > RF_Reciever::transmit_time * 64) {
      ++RF_Reciever::t_bitCounter;

      if (RF_Reciever::t_bitCounter == 4) {
        RF_Reciever::t_progress.setAll = 0;
        RF_Reciever::t_progress.hasStartSignal = true;
        RF_Reciever::d_properties.setAll = 0;
        RF_Reciever::dataRecieved[0] = 0;
        RF_Reciever::dataRecieved[1] = 0;
        RF_Reciever::dataRecieved[2] = 0;

        RF_Reciever::t_properties.setAll = 0;
        RF_Reciever::t_bitCounter = 0;
      }

    } else {
      RF_Reciever::t_bitCounter = 0;
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

  if (!RF_Reciever::t_progress.hasStartSignal) {
    RF_Reciever::processingStartSignal(timeSinceLastPeak);
  } else {
    const int peakLength = timeSinceLastPeak / transmit_time;

    if (peakLength >= 6 && digitalRead(recievePin) == LOW) {
      // Is End Peak
      ++RF_Reciever::t_properties.transmissionPartCounter;
      RF_Reciever::t_progress.hasSeperator = true;
      RF_Reciever::t_bitCounter = 0;
    } else {
      if (peakLength >= 2) {
        if (RF_Reciever::t_bitCounter == 0 && RF_Reciever::t_progress.hasSeperator) {
          //Message started low and went high
          RF_Reciever::t_progress.hasSeperator = false;
          RF_Reciever::t_progress.innerPeak = false;
          RF_Reciever::t_progress.isBitOne = true;
        } else {
          RF_Reciever::t_progress.isBitOne = !RF_Reciever::t_progress.isBitOne;
        }
      } else {
        // Same type of bit
        if (RF_Reciever::t_bitCounter == 0 && RF_Reciever::t_progress.hasSeperator) {
          //Message started high and went low
          RF_Reciever::t_progress.hasSeperator = false;
          RF_Reciever::t_progress.innerPeak = true;
          RF_Reciever::t_progress.isBitOne = false;
        } else {
          RF_Reciever::t_progress.innerPeak = !RF_Reciever::t_progress.innerPeak;
        }
      }

      if (!RF_Reciever::t_progress.innerPeak) {
        if (!RF_Reciever::t_progress.hasReliability) {
          decodeReliability();
        } else if (!RF_Reciever::t_progress.hasDataType) {
          decodeDataType();
        } else if (RF_Reciever::t_progress.hasDataType) {
          decodeDataSegment();
        }
      }

    }
  }

  RF_Reciever::lastPeak = currentTime;
}

void RF_Reciever::decodeReliability() {

  if (RF_Reciever::t_progress.isBitOne) {
    RF_Reciever::t_properties.reliability |= 1 << (1 - RF_Reciever::t_bitCounter);
  }

  ++RF_Reciever::t_bitCounter;

  if (RF_Reciever::t_bitCounter >= 2) {
    RF_Reciever::t_progress.hasReliability = true;
    RF_Reciever::t_bitCounter = 0;
  }

}

void RF_Reciever::decodeDataType() {
  
  if (RF_Reciever::t_progress.isBitOne) {
    RF_Reciever::t_properties.data_type |= 1 << (1 - RF_Reciever::t_bitCounter);
  }

  ++RF_Reciever::t_bitCounter;

  if (RF_Reciever::t_bitCounter >= 2) {
    RF_Reciever::t_progress.hasDataType = true;
    RF_Reciever::t_bitCounter = 0;
  }

}

void RF_Reciever::decodeDataSegment() {
  switch (RF_Reciever::t_properties.data_type) {
    case data_byte:
      if (RF_Reciever::t_progress.isBitOne) {
        RF_Reciever::dataRecieved[RF_Reciever::t_properties.transmissionPartCounter] |= 1 << (7 - RF_Reciever::t_bitCounter);
      }
      break;
    case data_int:
      if (RF_Reciever::t_progress.isBitOne) {
        RF_Reciever::dataRecieved[RF_Reciever::t_properties.transmissionPartCounter] |= 1 << (15 - RF_Reciever::t_bitCounter);
      }
      break;
    case data_long:
      if (RF_Reciever::t_progress.isBitOne) {
        RF_Reciever::dataRecieved[RF_Reciever::t_properties.transmissionPartCounter] |= 1 << (31 - RF_Reciever::t_bitCounter);
      }
      break;
    default:
      break;
  }

  ++RF_Reciever::t_bitCounter;

  if (RF_Reciever::t_properties.reliability == t_unsafe ||
    (RF_Reciever::t_properties.reliability == t_safe && RF_Reciever::t_properties.transmissionPartCounter >= 3)) {

    if (!hasIntegrity()) {
      RF_Reciever::d_properties.hasData = false;
    } else {
      RF_Reciever::d_properties.hasData = true;
      RF_Reciever::d_properties.data_type = RF_Reciever::t_properties.data_type;
    }

    RF_Reciever::t_progress.setAll = 0;
    RF_Reciever::t_properties.setAll = 0;
    RF_Reciever::t_bitCounter = 0;
  }

}
