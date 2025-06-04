/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 StepSmart.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  02/04/24
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
SteppSmart: Control a Reciever SmartStepper CAN-BUS (NANO)

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#pragma once
#ifndef STEPPERSMART_EXTRUSOR
#define STEPPERSMART_EXTRUSOR
#include "Arduino.h"
#include "StepSmart.h"

class StepSmartExtrusor : public StepSmartBase {
public:
  AS5600 *encoderPtr;

  StepSmartExtrusor();

  
  MotorState Execute();
  bool Attach(byte _stepPin, byte _dirPin, byte _enaPin, AS5600 *_as5600);
  bool SetSteps(StepsUnits _steps);

private:


  MotorState _Collision();
  MotorState _RestoreCollision();

  bool _CheckCollision();
  long _PickPosition();
 
};

#endif