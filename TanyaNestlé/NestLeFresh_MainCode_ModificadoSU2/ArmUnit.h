/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 ARMUNIT.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  25/09/22
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
Arm Unit contains
  - 1 Servo 
  - 2 Sliders
  - 1 Vacuum Unit 
    * 1 H-Bridge
    * 1 VacSensor

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#pragma once
#ifndef _ArmUnit
#define _ArmUnit

#include "ID_Def.h"
#include "Assets.h"
#include "CANmannager.h"

#ifndef _SmartMotor
#include "SmartMotor.h"
#endif

#ifndef _ServoUnitNew
#include "ServoUnit2.h"//***********************************
#endif

#ifndef _VacuumUnit
#include "VacuumUnit.h"
#endif

#include <Arduino.h>

#include "CounterACK.h"

class ArmUnit  // Class ArmUnit Handles 2 Sliders (SmartMotor(Transiever)), 1 Servo Unit , 1 VacuumUnit (1 H-Bridge(1pin) + VacuuumSensor)
{
public:
  __ID ID;  // Owns ID
  //CounterACK counterSlider;
  ArmUnit();  // Constructor

  void Attach(ID_Definition _ID, ID_Definition _IDj1, ID_Definition _IDj2, ID_Definition _IDj3, ID_Definition _IDvac1, ID_Definition _IDvac2, ID_Definition _IDsens1, ID_Definition _IDsens2, byte _J3_Pin, byte _eF1_Pin, byte _eF2_Pin, byte _sens1_Pin, byte _sens2_Pin, CANmannager *_can, int _defServoAngle, int _servoID);  // Initialize an Arm unit (IS's , Pins)
  void ActivateServos();
  void DeactivateServos();
  void GoTo(float _x, float _y, int _theta);  // Moves the Arm to (X,Y,Theta) position
  void ResetSliders();
  void GoToHome();
  void GoHome();
  void GoHome(bool _slider);
  void MoveX(float _x);               // Move Slider 1 to position (X)
  void MoveY(float _y);               // Move Slider 2 to position (Y)
  void TurnTheta(int _theta);        // Turn ServoUnit (Theta) angle
    void TurnThetaSlow(int _theta,unsigned _steps);        // Turn ServoUnit (Theta) angle
  void TurnOnVac(bool endEffSel);    // Turn On Vacuum valve
  void TurnOffVac(bool endEffSel);   // Turn Off Vacuum Valve
  bool GetVacState(bool endEffSel);  // Returns the state of the vacuum valve
  bool IsVacSense(bool endEffSel);
  void Run();  // Returns the read of the vacuum Sensor

private:

  long x = 0, y = 0;  // Positions X , Y of the sliders
  int theta;          // Angle Theta of the servo

  CANmannager *can;

  SliderMannager joint1;                  // SmartMotor Joint 1
  SliderMannager joint2;                  // SmartMotor Joint 2
  ServoUnit2 joint3;                       // ServoUnit Joint 3
  VacuumUnit endEffector1, endEffector2;  // VaccumUnit EndEffector0
};

#endif
