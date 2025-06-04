/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 ARMUNIT.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#ifndef _ArmUnit
#include "ArmUnit.h"
#include <Arduino.h>

// Class ArmUnit Handles 2 Sliders (SmartMotor(Transiever)), 1 Servo Unit , 1 VacuumUnit (1 H-Bridge(1pin) + VacuuumSensor)
ArmUnit::ArmUnit() {
}

// Initialize an Arm unit (IS's , Pins)
void ArmUnit::Attach(ID_Definition _ID, ID_Definition _IDj1, ID_Definition _IDj2, ID_Definition _IDj3, ID_Definition _IDvac1, ID_Definition _IDvac2, ID_Definition _IDsens1, ID_Definition _IDsens2, byte _J3_Pin, byte _eF1_Pin, byte _eF2_Pin, byte _sens1_Pin, byte _sens2_Pin, CANmannager *_can, int _defServoAngle = 0,int _servoID = -1) {
  this->can = _can;
  this->ID.AttachID(_ID, T_Arm);
  this->joint1.Attach(_IDj1, this->can);
  this->joint2.Attach(_IDj2, this->can);
  this->joint3.Attach(_IDj3, _J3_Pin,0, _defServoAngle, _servoID);
  this->endEffector1.Attach(_IDvac1, _IDsens1, _eF1_Pin, _sens1_Pin);
  this->endEffector2.Attach(_IDvac2, _IDsens2, _eF2_Pin, _sens2_Pin);

  ResetSliders();
}

void ArmUnit::ActivateServos() {
  this->joint3.Activate();
}
void ArmUnit::DeactivateServos() {
  this->joint3.Deactivate();
}

void ArmUnit::ResetSliders() {
  // Do a GoHome function
  this->joint1.GoHome();
  this->joint2.GoHome();
}


void ArmUnit::Run() {
  this->joint3.Run();
}



void ArmUnit::GoTo(float _x, float _y, int _theta) {
  MoveX(_x);
  MoveX(_x);
  TurnTheta(_theta);
}

void ArmUnit::GoHome() {
  this->joint1.GoHome();
  delay(1);
  this->joint2.GoHome();
}

void ArmUnit::GoHome(bool _slider) {
  if (!_slider) {
    this->joint1.GoHome();
  } else {
    this->joint2.GoHome();
  }
}



void ArmUnit::GoToHome() {
  GoTo(0.0, 0.0, 0.0);
}

// Move Slider 1 to position (X)
void ArmUnit::MoveX(float _x) {
  this->joint1.GoToPosition(_x);
  // long _pos = (_x * 10);
  // SendMSG(this->can, this->ID.GetID(), MSG_PARAMETER, LONG, false, P_RefPos, _pos);
}

// Move Slider 2 to position (Y)
void ArmUnit::MoveY(float _y) {
  this->joint2.GoToPosition(_y);
}


// Turn ServoUnit (Theta) angle
void ArmUnit::TurnTheta(int _theta) {
  this->joint3.SetPos(_theta);//************************************Hay que modificar esta para enviar tambien ID
}
void ArmUnit::TurnThetaSlow(int _theta ,unsigned _steps) {
  this->joint3.SetPos(_theta);
}

// Turn On Vacuum valve
void ArmUnit::TurnOnVac(bool endEffSel) {
  if (!endEffSel)
    this->endEffector1.SetState(true);
  else
    this->endEffector2.SetState(true);
}

// Turn Off Vacuum Valve
void ArmUnit::TurnOffVac(bool endEffSel) {
  if (!endEffSel)
    this->endEffector1.SetState(false);
  else
    this->endEffector2.SetState(false);
}

// Returns the state of the vacuum valve
bool ArmUnit::GetVacState(bool endEffSel) {

  if (!endEffSel)
    return this->endEffector1.GetState(0);
  else
    return this->endEffector2.GetState(0);
}

// Returns the read of the vacuum Sensor
bool ArmUnit::IsVacSense(bool endEffSel) {
  //////CAUTION REMOVE ALWAYS TRUE
  //return true;  //////CAUTION REMOVE ALWAYS TRUE !!!!!!!!!!!!!!!!!
  if (!endEffSel)
    return this->endEffector1.ReadVacuum();
  else
    return this->endEffector2.ReadVacuum();
}

#endif
