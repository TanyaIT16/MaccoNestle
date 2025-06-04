/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 H_Bridge.h
                                                  1.0.00

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  29/04/24
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
H-Bridge: Control a H-Bridge bidirectional

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


#pragma once
#ifndef H_Bridge
#define H_Bridge

#include "Arduino.h"

#ifndef defPin
#define defPin 0xFF  // Pin not determinated by default
#endif

enum HBridge_Status {
  HB_STOPPED = 0b00,
  HB_FORWARD = 0b01,
  HB_BACKWARD = 0b10,
  HB_FORBIDEN = 0b11
};

struct HB_Pins {
  byte pin0 = defPin;
  byte pin1 = defPin;
  byte pinEna = defPin;
};

class H_Bridge_Base {

public:
  H_Bridge_Base();  // Constructor
  virtual bool Attach(byte _pin0, byte _pin1, byte _pinPwm);
  virtual bool SetForward();
  virtual bool SetBackward();
  virtual bool SetStop();
  virtual bool SetState(HBridge_Status _state);

protected:
  HBridge_Status HbState;
  HB_Pins Pins;
};

class H_Bridge_2Pins : public H_Bridge_Base {

public:
  H_Bridge_2Pins();  // Constructor
  virtual bool Attach(byte _pin0, byte _pin1);
};


class H_Bridge_1Pin : public H_Bridge_Base {

public:
  H_Bridge_1Pin();  // Constructor
  virtual bool Attach(byte _pin0);
};






#endif