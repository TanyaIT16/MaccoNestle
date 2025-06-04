/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 ServoUnit.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#ifndef _ServoUnitOld
#include <Arduino.h>
#include "Servo.h"
#include "ServoUnit.h"

byte ServoUnit::nServoUnits = 0;

// Class Servo Unit, Handles a servo PWM type (Uses Servo library)
ServoUnit::ServoUnit() {
  nServoUnits++;
}

// Initialize the servo unit (ID, PwmPin , maxAngle , minAngle)
void ServoUnit::Attach(ID_Definition _ID, byte _pin, int _maxAngle, int _minAngle) {

  if (!ID.GetIsInitialize()) {
    this->ID.AttachID(_ID, T_ServoUnit);
    this->pin = _pin;
    SetAngles(_maxAngle, _minAngle);
    //this->servo.attach(this->pin, this->minAngle, this->maxAngle);

  } else
    ID.ThrowError(ALREADY_REGISTERED_ID_WARNING);
}

// Initialize the servo unit with default angles 0-180 (ID, PwmPin)
void ServoUnit::Attach(ID_Definition _ID, byte _pin) {
  if (!ID.GetIsInitialize()) {
    this->ID.AttachID(_ID, T_ServoUnit);
    this->pin = _pin;
    //this->servo.attach(_pin);
  }
}

void ServoUnit::Attach(ID_Definition _ID, byte _pin, int _defAngle) {
  this->refAngle = _defAngle;
  //this->servo.write(this->refAngle);
  Attach(_ID, _pin);
  //this->servo.write(this->refAngle);
  //delay(1);
}

void ServoUnit::Activate(){
  this->servo.write(this->refAngle);
  this->servo.attach(this->pin);
  this->servo.write(this->refAngle);
}
void ServoUnit::Deactivate(){
  this->servo.write(this->refAngle);
  delay(1000);
  this->servo.detach();
}

// Set the servo position to an angle (Inputs: Angle [º])
void ServoUnit::SetPos(int _angle) {
  if (_isMap)
    _angle = _MapAngle(_angle);
  // ServoUnit::servo[this->n].write(_angle);
  this->servo.write(_angle);
}


void ServoUnit::SetPos(int _angle, byte _step) {
  _angle = _MapAngle(_angle);
  this->refAngle = _angle;
  this->timer0.ResetTimer();
  if (_step > 0) {
    this->steps = _step;
  } else {
    this->angle = _angle;
  }
}

void ServoUnit::SetTimer(int _delay) {
  this->timerStep = _delay;
}

void ServoUnit::Run() {
  int deltaAngle = this->refAngle - this->angle;
  if (this->timer0.Check(this->timerStep) && this->timerStep > 0) {
    this->timer0.ResetTimer();
    //Serial.println(deltaAngle);
    if (deltaAngle > 0) {

      if (abs(deltaAngle) > this->steps) {
        this->angle += this->steps;
      } else {
        this->angle++;
      }
    } else if (deltaAngle < 0) {
      if (abs(deltaAngle) > this->steps) {
        this->angle -= this->steps;
      } else {
        this->angle--;
      }
    } else {
    }
    this->servo.write(this->angle);
  }
}



// Go to max angle settedUp
void ServoUnit::GoMax() {
  this->SetPos(this->maxAngle);
}

// Go to min angle settedUp
void ServoUnit::GoMin() {
  this->SetPos(this->minAngle);
}

// SetUp the Limits angles
void ServoUnit::SetAngles(int _maxAngle, int _minAngle) {
  if (_maxAngle != defMaxAngle || _minAngle != defMinAngle)
    this->_isMap = true;
  else
    this->_isMap = false;
  this->maxAngle = _maxAngle;
  this->minAngle = _minAngle;
}

// SetUp the Limits angles Assumes min=0
void ServoUnit::SetAngles(int _maxAngle) {
  SetAngles(_maxAngle, defMinAngle);
}

// Return the phisical Pin of the servo PWM
byte ServoUnit::GetPin() {
  return this->pin;
}

// Return the actual ReferenceAngle
unsigned ServoUnit::GetAngle() {
  return this->angle;
}

// Returns the Number of ServoUnits Registered
byte ServoUnit::GetTypeCount() {
  return ServoUnit::nServoUnits;
}

// Do a conversion betwen angles (INTERNAL USE ONLY)
int ServoUnit::_MapAngle(int _angle) {
  if (_isMap) {
    _angle = map(_angle, this->minAngle, this->maxAngle, defMinAngle, defMaxAngle);
  }
  return _angle;
}

// Clamp the angle to limits (INTERNAL USE ONLY)
int ServoUnit::_clampAngle(int _angle) {
  if (_angle > this->maxAngle)
    _angle = this->maxAngle;
  else if (_angle < this->minAngle)
    _angle = this->minAngle;
  return _angle;
}

#endif