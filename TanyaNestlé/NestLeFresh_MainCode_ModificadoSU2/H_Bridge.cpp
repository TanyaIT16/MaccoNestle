/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 H_Bridge.cpp
                                                  1.0.00
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  29/04/24

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#ifndef _HBridge_Base
#include <Arduino.h>
#include "H_Bridge.h"

H_Bridge_Base::H_Bridge_Base()  // Constructor
{
}
bool H_Bridge_Base::Attach(byte _pin0, byte _pin1, byte _pinPwm) {

  this->HbState = HB_STOPPED;
  if (_pin0 != defPin) {
    pinMode(_pin0, OUTPUT);
    this->Pins.pin0 = _pin0;
  }
  if (_pin1 != defPin) {
    pinMode(_pin1, OUTPUT);
    this->Pins.pin1 = _pin1;
  }
  if (_pinPwm != defPin) {
    pinMode(_pinPwm, OUTPUT);
    this->Pins.pin0 = _pinPwm;
  }
  SetState(this->HbState);
}
bool H_Bridge_Base::SetForward() {
  SetState(HB_FORWARD);
}
bool H_Bridge_Base::SetBackward() {
  SetState(HB_BACKWARD);
}
bool H_Bridge_Base::SetStop() {
  SetState(HB_STOPPED);
}
bool H_Bridge_Base::SetState(HBridge_Status _state) {
  switch (_state) {
    case HB_STOPPED:
      digitalWrite(this->Pins.pin0, LOW);
      digitalWrite(this->Pins.pin1, LOW);
      break;
    case HB_FORWARD:
      digitalWrite(this->Pins.pin0, HIGH);
      digitalWrite(this->Pins.pin1, LOW);
      break;
    case HB_BACKWARD:
      digitalWrite(this->Pins.pin0, LOW);
      digitalWrite(this->Pins.pin1, HIGH);
      break;
    default:
      digitalWrite(this->Pins.pin0, LOW);
      digitalWrite(this->Pins.pin1, LOW);
      break;
  }
}

#endif