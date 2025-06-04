#include "HardwareSerial.h"
/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 StepSmart.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  29/03/24

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#include "Arduino.h"
#include "StepSmart_Extrusor.h"
//static StepSmartExtrusor::AS5600 as5600;

StepSmartExtrusor::StepSmartExtrusor() {
}

MotorState StepSmartExtrusor::Execute() {
  this->encoderPtr->getCumulativePosition();
  if (this->state == MOTOR_STATE_STANDBY) {
    this->extrusor.motor.SetMotorBehavior(MOTOR_DISABLED);
    if (this->parameters.steps_order > 0) {
      this->state = MOTOR_STATE_INITIALIZING;
    }
  } else if (this->state == MOTOR_STATE_INITIALIZING) {
    this->state = _Start();
    this->extrusor.motor.SetMotorBehavior(MOTOR_FORWARD);
  } else if (this->state == MOTOR_STATE_RUNNING) {
    this->state = _Run();

  } else if (this->state == MOTOR_STATE_STOPPED) {
    this->state = _End();
  } else if (this->state == MOTOR_STATE_COLLISION) {
    this->state = _Collision();
  } else if (this->state == MOTOR_STATE_RESTORED_COLLISION) {
    this->state = _RestoreCollision();
  } else {
    this->state = MOTOR_STATE_NOT_INITIALIZE;
  }

  return this->state;
}
bool StepSmartExtrusor::Attach(byte _stepPin, byte _dirPin, byte _enaPin, AS5600 *_as5600) {
  this->extrusor.motor.Attach(_stepPin, _dirPin, _enaPin, this->parameters.steps_per_revolution);
  this->encoderPtr = _as5600;
  this->state = MOTOR_STATE_STANDBY;
  this->extrusor.motor.StartTimerInterrupt();
  this->extrusor.motor.SetInvertLogic(false, false);
  this->extrusor.motor.SetSpeedDivider(1);
  this->extrusor.motor.SetMotorBehavior(MOTOR_FORWARD);
}


bool StepSmartExtrusor::SetSteps(StepsUnits _steps) {

  this->encoderPtr->resetCumulativePosition();
  this->encoderPtr->resetPosition();
  this->encoderPtr->resetCumulativePosition();
  this->encoderPtr->SetOffsetPos(this->encoderPtr->getCumulativePosition());



  this->variableData.stepsLeft = _steps;
  this->variableData.positionData.last = 0;
  this->variableData.positionData.main = 0;
  this->variableData.positionData.desired = 0;
  _PickPosition();
  this->parameters.steps_order = _steps;
  this->variableData.positionData.collisionCounter = 0;
  this->variableData.positionData.finalDesired = this->variableData.positionData.main + StepsUnitsToEncoderUnits(this->parameters.steps_order);
}


long StepSmartExtrusor::_PickPosition() {
  //EncoderPositionRedux position = this->extrusor.encoder.GetCount();
  this->variableData.positionData.last = this->variableData.positionData.main;
  this->variableData.positionData.main = this->encoderPtr->getCumulativePosition();
  Serial.print("Desired: ");
  Serial.print(this->variableData.positionData.desired.getTicks());
  Serial.print("\t Main: ");
  Serial.println(this->variableData.positionData.main.getTicks());
  StepsUnits _remindSteps = EncoderUnitsToStepsUnits((EncoderUnits(0, this->variableData.positionData.desired.getTicks() - this->variableData.positionData.main.getTicks())));

  return (_remindSteps.getLaps() * STEPS_LAP) + _remindSteps.getTicks();
}


MotorState StepSmartExtrusor::_Collision() {

  if (!this->extrusor.motor.GetIsSteps2Do()) {

    if (this->variableData.collisionState == COLLISION_COLLIDED) {
      this->variableData.collisionState = COLLISION_REVERSE;
      this->extrusor.motor.SetMotorBehavior(MOTOR_BACKWARD);
      //this->extrusor.motor.SetStepBatch(100);
      _ForceSetNewBatch(StepsUnits(0, this->parameters.reverseSteps));
      return MOTOR_STATE_COLLISION;
    }

    else if (this->variableData.collisionState == COLLISION_REVERSE) {
      this->variableData.collisionState = COLLISION_RECOVERING;
      this->extrusor.motor.SetMotorBehavior(MOTOR_FORWARD);
      _ForceSetNewBatch(StepsUnits(0, this->parameters.reverseSteps));
      this->variableData.stepsLeft = 0;

      return MOTOR_STATE_COLLISION;
    }

    else if (this->variableData.collisionState == COLLISION_RECOVERING) {
      this->variableData.collisionState = COLLISION_NO_COLLISION;
      this->variableData.stepsLeft = 0;



      return MOTOR_STATE_STOPPED;
    }
  } else {
    return MOTOR_STATE_COLLISION;
  }
}


MotorState StepSmartExtrusor::_RestoreCollision() {
  _PickPosition();
  this->variableData.stepsLeft = EncoderUnitsToStepsUnits(this->variableData.positionData.finalDesired - this->variableData.positionData.main);  //Reset Step Left
  this->variableData.positionData.desired = this->variableData.positionData.main;
  Serial.println("Recovered");
  return MOTOR_STATE_RUNNING;
}

bool StepSmartExtrusor::_CheckCollision() {
  long _po = _PickPosition();
  Serial.println(_po);
  if (abs(_po) < this->parameters.maxErrorPositionAllowed.getTicks()) {  // cuando main > desired+delta || main < desired-delta
    //this->variableData.stepsLeft = this->variableData.stepsLeft + EncoderUnitsToStepsUnits(this->variableData.positionData.desired - this->variableData.positionData.main);  // restore the undoSteps
    return false;
  } else return true;
}
