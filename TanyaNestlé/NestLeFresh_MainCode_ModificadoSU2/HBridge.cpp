#include "HardwareSerial.h"
/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 HBridge.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#ifndef _HBridge
#include <Arduino.h>
#include "HBridge.h"

byte HBridge::nHBridges = 0;  // Initialize GlobalCounter H-Bridges

// Class H-Bridge to control a bidirectional DC motor, valve or actuator
HBridge::HBridge() {
  nHBridges++;
}

// Initialize the H-Bridge with the user parameters (ID, 2 Pins)
bool HBridge::Attach(ID_Definition _ID, byte _pin0, byte _pin1) {
  this->ID.AttachID(_ID, T_HBridge);
  if (_pin0 != defPin) {
    pinMode(_pin0, OUTPUT);
    this->pin0 = _pin0;
  }
  if (_pin1 != defPin) {
    pinMode(_pin1, OUTPUT);
    this->pin1 = _pin1;
  }
  this->SetEna(true);
  this->SetState(false);
}

// Initialize the H-Bridge with the user parameters (ID, 1 Pin)
bool HBridge::Attach(ID_Definition _ID, byte _pin0) {
  this->ID.AttachID(_ID, T_HBridge);
  if (_pin0 != defPin) {
    pinMode(_pin0, OUTPUT);
    this->pin0 = _pin0;
  }
  this->SetEna(true);
  this->SetState(false);
  this->pin1 = defPin;
}

// Set the direction of flow of the H-bridge (Reverse-Foward)
bool HBridge::SetState(bool _dir) {
  if (this->dir != _dir) {
    this->dir = _dir;
    if (this->pin0 != defPin)
      digitalWrite(this->pin0, LOW);
    if (this->pin1 != defPin)
      digitalWrite(this->pin1, LOW);
    if (this->ena) {
      //Serial.println(_dir);
      if (this->pin0 != defPin)
        digitalWrite(this->pin0, _dir);
      if (this->pin1 != defPin)
        digitalWrite(this->pin1, !_dir);
    }
  }

  return _dir;
}

// returns the state of the H-Bridge (INPUTS: dir-enaFLAG) (return 1 foward OR On; 0 backward OR Off)
bool HBridge::GetState(bool _hx) {
  if (_hx)
    return this->ena;
  else
    return this->dir;
}

// returns the phisical pin attached to the H-Bridge (INPUTS: PinSelFLAG)
byte HBridge::GetPin(bool _px) {
  if (!_px)
    return this->pin0;
  else
    return this->pin1;
}

// Set the enable-disable of H-bridge (OFF-ON)
void HBridge::SetEna(bool _ena) {
  this->ena = _ena;
  if (!this->ena) {
    if (pin0 != defPin)
      digitalWrite(this->pin0, LOW);
    if (pin1 != defPin)
      digitalWrite(this->pin1, LOW);
  }
}

// Returns the Number of VacuumUnits Registered
byte HBridge::GetTypeCount() {
  return HBridge::nHBridges;
}

HBridgePWM::HBridgePWM()
  : HBridge() {
}


bool HBridgePWM::Attach(ID_Definition _ID, byte _pin0, byte _pin1, byte _pinPwm)  // Initialize the H-Bridge with the user parameters (ID, 2 Pins)
{
  this->ID.AttachID(_ID, T_HBridge);
  if (_pin0 != defPin) {
    pinMode(_pin0, OUTPUT);
    this->pin0 = _pin0;
  } else
    this->pin1 = defPin;
  this->pwmPin = _pinPwm;
  pinMode(_pinPwm, OUTPUT);
}

bool HBridgePWM::Attach(ID_Definition _ID, byte _pin0, byte _pinPwm)  // Initialize the H-Bridge with the user parameters (ID, 1 Pins)
{
  this->ID.AttachID(_ID, T_HBridge);
  if (_pin0 != defPin) {
    pinMode(_pin0, OUTPUT);
    this->pin0 = _pin0;
  }

  this->pin1 = defPin;
  this->pwmPin = _pinPwm;
  pinMode(_pinPwm, OUTPUT);
}

void HBridgePWM::SetSpeed(byte _speed) {
  if (pwmPin != 0xFF)
    this->pwmValue = _speed;
  analogWrite(this->pwmPin, this->pwmValue);
}

int HBridgePWM::GetSpeed() {
  return this->pwmValue;
}

#endif  // END H-BRIDGE
