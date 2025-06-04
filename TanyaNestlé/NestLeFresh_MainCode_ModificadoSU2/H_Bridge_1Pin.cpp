#include "H_Bridge.h"


H_Bridge_1Pin::H_Bridge_1Pin()  // Constructor
{
}
bool H_Bridge_1Pin::Attach(byte _pin0) {

  this->HbState = HB_STOPPED;
  if (_pin0 != defPin) {
    pinMode(_pin0, OUTPUT);
    this->Pins.pin0 = _pin0;
  }
  SetState(this->HbState);
}

