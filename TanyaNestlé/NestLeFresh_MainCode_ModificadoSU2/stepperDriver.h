/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 StepperDriver.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  29/03/24
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
StepperDriver: General Pourpose Stepper Driver Controller

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#pragma once
#ifndef STEPPERDRIVER
#define STEPPERSDRIVER

#include "Arduino.h"
#include "Wire.h"
#include <TimerOne.h>
#include "Position.h"


#ifndef MAX_ALLOWED_STEPPERS
#define MAX_ALLOWED_STEPPERS 6
#endif

#define SPEED_DEFAULT 3000
#define SPEED_DEVIATION 2500

#define ACC_DEFAULT 5
#define ACC_DEVIATION 0

#define ENA_INVERT_MASK 0b00000010
#define DIR_INVERT_MASK 0b00000001

enum StepperBehavior {
  MOTOR_NOCOMMAND,
  MOTOR_STOPED,
  MOTOR_DISABLED,
  MOTOR_FORWARD,
  MOTOR_BACKWARD,
  MOTOR_HOLDED
};

struct PositionTurns {
EncoderUnits encoder;

};

struct MotorData {
  StepperBehavior command;
  unsigned long steps;
};

struct MotorSpeed {
  unsigned mainSpeed = SPEED_DEFAULT;
  unsigned minSpeed = SPEED_DEFAULT;
  unsigned maxSpeed = SPEED_DEFAULT - SPEED_DEVIATION;
  unsigned mainAcc = ACC_DEFAULT;
};

struct StepperPins {
  byte enaPin;                    // number of phisical pin Enable-
  byte dirPin;                    // number of phisical pin Dir-
  byte stepPin;                   // number of phisical pin Step-
  byte invertLogic = 0b00000000;  // first bit = ENA invert logic ---- second bit = DIR invert direction logic
  byte speedDivider;
  unsigned stepsPerRevolution;
};

struct StepperParameter {
  StepperBehavior motorStatus = MOTOR_DISABLED;
  MotorSpeed motorSpeed;
};



struct StepperSignal {
  bool _IsSteps2Do = false;
  bool _status = LOW;
  unsigned long _steps2Do = 0;
  byte _stepPin;
  byte _speedDivider = 1;
};

class StepperDriver {
public:
  static volatile byte numberOfSteppers;
  static volatile byte _tick;
  static volatile StepperParameter motorParameter;

  static volatile StepperSignal signal[MAX_ALLOWED_STEPPERS];

  static void __advanceStepISR();

  static void __AccelerationEvent();
  static void StartTimerInterrupt();                  // Start the timer ISR() for the stepper
  static void UpdateTimerInterrupt(unsigned _speed);  // Update the timer ISR() for the stepper to a new freq
  static void UpdateTimerInterrupt();                 // Update the timer ISR() for the stepper to default freq
  static void HoldTimerInterrupt();                   // Hold On the timer ISR() for the stepper

  static void SetMainSpeed(unsigned _speed);
  static void SetMinSpeed(unsigned _speed);
  static void SetMaxSpeed(unsigned _speed);

  static void SetMainAcc(unsigned _speed);

private:

  byte stepperNumber;
  StepsUnits stepsBatch;
  StepperPins motorPins;
  StepperBehavior motorBehavior = MOTOR_DISABLED;

public:
  StepperDriver();
  ~StepperDriver();

  bool Attach(byte _stepPin, byte _dirPin, byte _enaPin, unsigned _stepsPerRevolution);
  void SetInvertLogic(bool invertEna, bool invertDir);
  void SetStepBatch(unsigned long _steps);
  void SetStepBatch(StepsUnits _stepsTurns);


  StepsUnits GetStepBatch();
  StepsUnits GetSteps2Do();
  bool GetIsSteps2Do();


  bool SetMotorBehavior(StepperBehavior _behavior);
  bool SetSpeedDivider(byte _speedDivider);

  StepperBehavior GetMotorBehavior(StepperBehavior _behavior);
  byte GetSpeedDivider(byte _speedDivider);
};


#endif