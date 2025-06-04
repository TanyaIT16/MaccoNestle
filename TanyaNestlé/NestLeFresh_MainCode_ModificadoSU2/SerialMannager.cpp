/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 SerialMannager.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#include "SerialMannager.h"

unsigned SerialMannager::numberOfSerial = 0;


SerialMannager::SerialMannager() {
  SerialMannager::numberOfSerial++;
}

void SerialMannager::begin(unsigned _baudrate) {}

void SerialMannager::SetVerboseLevel(Verbose_Level _level) {
  this->verboseLevel = _level;
}

void SerialMannager::print(int _verboseLevel, const String& message) {
  if (_verboseLevel <= this->verboseLevel) {
    Serial.print(message);
  }
}
void SerialMannager::println(int _verboseLevel, const String& message) {
  if (_verboseLevel <= this->verboseLevel) {
    Serial.println(message);
  }
}

void SerialMannager::print(int _verboseLevel, const long& message) {
  if (_verboseLevel <= this->verboseLevel) {
    Serial.print(message);
  }
}

void SerialMannager::println(int _verboseLevel, const long& message) {
  if (_verboseLevel <= this->verboseLevel) {
    Serial.println(message);
  }
}


void SerialMannager::print(int VerboseLevel, const StepsUnits& message) {
}
void SerialMannager::println(int VerboseLevel, const StepsUnits& message) {
}

void SerialMannager::print(int VerboseLevel, const EncoderUnits& message) {
}
void SerialMannager::println(int VerboseLevel, const EncoderUnits& message) {
}

MainOrder SerialMannager::ReadData() {
  SerialData _data;
  MainOrder _newOrder;
  unsigned counter = 0;

  if (Serial.available() > 0) {
    print(VERBOSE_DEBUG, "NewSerial:");
    _data.order.incomingChar = Serial.read();  // Read the first character
    print(VERBOSE_DEBUG, _data.order.incomingChar);
    print(VERBOSE_DEBUG, "\t");
    if (Serial.available() > 0) {
      _data.order.auxChar = Serial.read();  // Read the second character
      _newOrder.checkNewOrder = true;
      print(VERBOSE_DEBUG, _data.order.auxChar);
      print(VERBOSE_DEBUG, "\t");

      while (Serial.available() > 0) {
        char numericChar = Serial.read();

        if (isDigit(numericChar) || numericChar == '-') {
          _data.numericValueString += numericChar;

        } else if ((numericChar == SEPARATION_CHARACTER && counter < MAX_DATA_ALLOWED) || (numericChar == '\n')) {
          _data.order.serialValue[counter] = _data.numericValueString.toInt();
          _data.numericValueString = "";
          print(VERBOSE_DEBUG, _data.order.serialValue[counter]);
          print(VERBOSE_DEBUG, SEPARATION_CHARACTER);
          counter++;
        }
      }
    }
  }
  _newOrder.auxChar=_data.order.auxChar;
  _newOrder.incomingChar = _data.order.incomingChar;
  for(int i=0; i<6 ; i++){
  _newOrder.grams[i]=_data.order.serialValue[i];
  }
  _newOrder.grindSize=_data.order.serialValue[6];
  if(_newOrder.grindSize == POWDERSIZE_MAX){
    _newOrder.selectorState = SELECTOR_BYPASS;
  }
  else{
    _newOrder.selectorState = SELECTOR_GRINDER;
  }
  return _newOrder;
}
