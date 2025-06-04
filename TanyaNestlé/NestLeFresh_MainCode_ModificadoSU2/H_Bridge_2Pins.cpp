/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 H_Bridge.cpp
                                                  1.0.00
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  29/04/24

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#ifndef _HBridge_2Pins
#include <Arduino.h>
#include "H_Bridge.h"

H_Bridge_2Pins::H_Bridge_2Pins() : H_Bridge_Base() // Constructor
{
}
bool H_Bridge_2Pins::Attach(byte _pin0, byte _pin1) {

  this->HbState = HB_STOPPED;
  if (_pin0 != defPin) {
    pinMode(_pin0, OUTPUT);
    this->Pins.pin0 = _pin0;
  }
  if (_pin1 != defPin) {
    pinMode(_pin1, OUTPUT);
    this->Pins.pin1 = _pin1;
  }
  SetState(this->HbState);
}

#endif