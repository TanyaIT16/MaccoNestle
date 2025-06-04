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
#ifndef STEPPERSMART
#define STEPPERSMART
#include "Arduino.h"
#include "AS5600macco.h"
#include "stepperDriver.h"
#include "MotorCommand.h"
#include "ID_Def.h"
#include "TranslateParameters.h"

#define STEPS_PER_REVOLUTION 1600
#define TICKS_PER_REVOLUTION 4096
#define SPEED_DIVIDER 1
#define ENCODER_ERROR_POSITION 128
#define TIMEOUT 6e4
#define MAX_COLLISION_COUNTER 10
#define STEPS_PER_dMM 27.83   //Steps per 1/100mm

enum MotorState {
  MOTOR_STATE_NOT_INITIALIZE,
  MOTOR_STATE_INITIALIZING,
  MOTOR_STATE_GOHOME,
  MOTOR_STATE_STANDBY,
  MOTOR_STATE_RUNNING,
  MOTOR_STATE_COLLISION,
  MOTOR_STATE_RESTORED_COLLISION,
  MOTOR_STATE_STOPPED
};

enum GoHomeStates {
  GOHOME_NOHOMMED,
  GOHOME_HOMMING,
  GOHOME_REACHEDHOME,
  GOHOME_RETRYING_BACK,
  GOHOME_RETRYING_FOWARD,
  GOHOME_HOMMED
};

enum CollisionState {
  COLLISION_NO_COLLISION,
  COLLISION_COLLIDED,
  COLLISION_REVERSE,
  COLLISION_RECOVERING
};

struct MOTOR_STATE {
  MotorState mainState;
  MotorState nextState;
};



struct PositionData {
  EncoderUnits main;
  EncoderUnits last;
  EncoderUnits desired;
  EncoderUnits finalDesired;
  unsigned collisionCounter = 0;
};



struct Parameters {
  long steps_per_revolution = STEPS_LAP;
  long steps_per_batch = STEPS_LAP / 2;
  StepsUnits steps_order = 0;
  EncoderUnits maxErrorPositionAllowed = EncoderUnits(0, ENCODER_ERROR_POSITION);
  //float syncFactor = (float)TICK_PER_REVOLUTION / (float)STEPS_PER_REVOLUTION;
  unsigned reverseSteps = STEPS_PER_REVOLUTION / 4;
  unsigned maxCollisionCounter = MAX_COLLISION_COUNTER;
};
struct VariableData {
  StepperBehavior mainBehavior;
  long positionOrder =0;
  StepsUnits stepsLeft;
  StepsUnits stepBatch;
  PositionData positionData;
  CollisionState collisionState;
  GoHomeStates goHomeState;
};

struct Extrusor {
  StepperDriver motor;
  //AS5600 as5600;
  MotorData data;
  unsigned long commandSteps;
};

/*

  P_Speed,
  P_Acceleration,
  P_RefSteps,
  P_Min_Speed,
  P_Max_Speed,
  P_Limit_Colisions,
  P_Position_Error,
  P_InvertLogic,
  P_StepReverse,

*/

class StepSmartBase {
public:
  StepSmartBase();

  virtual MotorState Execute();
  virtual bool Attach(byte _stepPin, byte _dirPin, byte _enaPin);
  virtual bool SetSteps(StepsUnits _steps);
  virtual bool SetPosition(long _distance);
  virtual void GoHome();

  virtual void SetParameter(SmartStepperPARAMETERS _param, byte _value[4]);
  virtual void SetMaxSpeed(unsigned _maxSpeed);
  virtual void SetMinSpeed(unsigned _minSpeed);
  virtual void SetAcceleration(unsigned _acc);
  virtual void SetLimitCollisionsEvent(unsigned _collisionEvent);
  virtual void SetPositionError(EncoderUnits _PositionError);
  virtual void SetInvertLogic(byte _invertLogic);
  virtual void SetStepsReverse (unsigned _StepReverse);
  virtual void SetStepsBatch (long _steps);
  virtual void SetMotorBehavior (StepperBehavior _behavior);

protected:

  MotorState state;
  Extrusor extrusor;
  VariableData variableData;
  Parameters parameters;
  MotorCommands motorCommandList, motorCorrectionList;



  MotorState _Start();
  MotorState _End();
  MotorState _Run();
  virtual MotorState _Collision();
  virtual MotorState _RestoreCollision();

  virtual bool _CheckCollision();
  virtual long _PickPosition();

  virtual bool _CheckGoHome();
  virtual MotorState _GoHome();

  void _SetMotorBehavior (StepperBehavior _behavior);

  StepsUnits _SetNewBatch();
  StepsUnits _ForceSetNewBatch(StepsUnits _steps);
  StepsUnits _TransformPosition(long _position);
};

#endif