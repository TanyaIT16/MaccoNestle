/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 ServoUnit.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#ifndef _ServoUnitNew
#include <Arduino.h>
#include "Servo.h"
#include "ServoUnit2.h"

byte ServoUnit2::nServoUnits = 0;

// Class Servo Unit, Handles a servo PWM type (Uses Servo library)
ServoUnit2::ServoUnit2() {
  nServoUnits++;
}

// Initialize the servo unit (ID, PwmPin , maxAngle , minAngle)
void ServoUnit2::Attach(ID_Definition _ID, byte _pin, int _maxAngle, int _minAngle, int servoID) {

  if (!ID.GetIsInitialize()) {
    this->ID.AttachID(_ID, T_ServoUnit);
    this->pin = _pin;
    SetAngles(_maxAngle, _minAngle);
    //this->servo.attach(this->pin, this->minAngle, this->maxAngle);
    this->idServo = servoID;
  } else
    ID.ThrowError(ALREADY_REGISTERED_ID_WARNING);
}

// Initialize the servo unit with default angles 0-180 (ID, PwmPin)
void ServoUnit2::Attach(ID_Definition _ID, byte _pin) {//****************************************
  


  
}

void ServoUnit2::Attach(ID_Definition _ID, byte _pin, int _defAngle) {//*****************************
 
}

void ServoUnit2::Activate(){//*************
}
void ServoUnit2::Deactivate(){//***********
}


// Set the servo position to an angle (Inputs: Angle [º])

void ServoUnit2::SetPos(int _angle) {//************************Funcion que modifico para enviar posicion 
    
    int velocidad = 500; 
    Serial.println("*"+String(idServo) + "," + String(_angle) + "," + String(velocidad)+"*");//*******************Cambio realizado de puerto serie 
    //Serial1.print(" ");
    //Serial1.print(_angle);
    //Serial1.print(" ");
    //Serial1.println(velocidad);

}


void ServoUnit2::SetPos(int _angle, byte _step) {//*******Mirar si esta funcion se esta usando en vez de la anterior

}

void ServoUnit2::SetTimer(int _delay) {//**********************

}

void ServoUnit2::Run() {//*********************************
}



// Go to max angle settedUp
void ServoUnit2::GoMax() {
  this->SetPos(this->maxAngle);
}

// Go to min angle settedUp
void ServoUnit2::GoMin() {
  this->SetPos(this->minAngle);
}

// SetUp the Limits angles
void ServoUnit2::SetAngles(int _maxAngle, int _minAngle) {
  if (_maxAngle != defMaxAngle || _minAngle != defMinAngle)
    this->_isMap = true;
  else
    this->_isMap = false;
  this->maxAngle = _maxAngle;
  this->minAngle = _minAngle;
}

// SetUp the Limits angles Assumes min=0
void ServoUnit2::SetAngles(int _maxAngle) {
  SetAngles(_maxAngle, defMinAngle);
}

// Return the phisical Pin of the servo PWM
byte ServoUnit2::GetPin() {
}

// Return the actual ReferenceAngle
unsigned ServoUnit2::GetAngle() {
  return this->angle;
}

// Returns the Number of ServoUnits Registered
byte ServoUnit2::GetTypeCount() {
  return ServoUnit2::nServoUnits;
}



// Do a conversion betwen angles (INTERNAL USE ONLY)
int ServoUnit2::_MapAngle(int _angle) {

}



// Clamp the angle to limits (INTERNAL USE ONLY)
int ServoUnit2::_clampAngle(int _angle) {
  if (_angle > this->maxAngle)
    _angle = this->maxAngle;
  else if (_angle < this->minAngle)
    _angle = this->minAngle;
  return _angle;
}

#endif
