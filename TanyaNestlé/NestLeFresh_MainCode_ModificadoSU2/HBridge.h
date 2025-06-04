/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 HBridge.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  25/09/22
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
H-Bridge: Control a H-Bridge bidirectional

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#pragma once
#ifndef _HBridge
#define _HBridge

#include "ID_Def.h"
#include "Assets.h"

#ifndef defPin
#define defPin 0xFF // Pin not determinated by default
#endif

class HBridge
{ // Class H-Bridge to control a bidirectional DC motor, valve or actuator

public:
  static byte nHBridges;             // Stores the total amount of H-Bridge on the system
  __ID ID;                           // Store owns ID
  byte pin0 = defPin, pin1 = defPin; // Pins to control the H-bridge

  HBridge(); // Constructor

  bool Attach(ID_Definition _ID, byte _pin0, byte _pin1); // Initialize the H-Bridge with the user parameters (ID, 2 Pins)
  bool Attach(ID_Definition _ID, byte _pin0);             // Initialize the H-Bridge with the user parameters (ID, 1 Pins)
  bool SetState(bool _dir);                               // Set the direction of flow of the H-bridge (Reverse-Foward)
  void SetEna(bool _ena);                                 // Set the enable-disable of H-bridge (OFF-ON)
  bool GetState(bool _hx);                                // returns the state of the H-Bridge (INPUTS: dir-enaFLAG) (return 1 foward OR On; 0 backward OR Off)
  byte GetPin(bool _px);                                  // returns the phisical pin attached to the H-Bridge (INPUTS: PinSelFLAG)                                  // Returns the stateERROR of the H-Bridge
  byte GetTypeCount();                                    // Returns the Number of VacuumUnits Registered

private:
  bool dir = true; // Direction Flag
  bool ena = true; // Enable Flag
};


class HBridgePWM : public HBridge
{
public:
  HBridgePWM();
  bool Attach(ID_Definition _ID, byte _pin0, byte _pin1,byte _pinPwm); // Initialize the H-Bridge with the user parameters (ID, 2 Pins)
  bool Attach(ID_Definition _ID, byte _pin0, byte _pinPwm); 
  void SetSpeed(byte _speed);
  int GetSpeed();

private:
  byte pwmValue = 0;
  byte pwmPin = 0xFF;
};

#endif // End H-Bridge Class