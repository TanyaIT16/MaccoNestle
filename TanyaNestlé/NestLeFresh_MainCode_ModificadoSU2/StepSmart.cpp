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
#include "StepSmart.h"

StepSmartBase::StepSmartBase() {

  this->variableData.goHomeState = GOHOME_NOHOMMED;
}

MotorState StepSmartBase::Execute() {
  //Serial.println(this->state);
  //this->encoderPtr->getCumulativePosition();
  if (this->state == MOTOR_STATE_STANDBY) {
    _SetMotorBehavior (MOTOR_DISABLED);
    if (this->parameters.steps_order > 0) {
      this->state = MOTOR_STATE_INITIALIZING;
    }
  } else if (this->state == MOTOR_STATE_INITIALIZING) {
    this->state = _Start();
    _SetMotorBehavior (MOTOR_FORWARD);
  } else if (this->state == MOTOR_STATE_GOHOME) {
    this->state = _GoHome();
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
bool StepSmartBase::Attach(byte _stepPin, byte _dirPin, byte _enaPin) {
  this->extrusor.motor.Attach(_stepPin, _dirPin, _enaPin, this->parameters.steps_per_revolution);
  this->state = MOTOR_STATE_STANDBY;
  this->extrusor.motor.StartTimerInterrupt();
  this->extrusor.motor.SetInvertLogic(true, false);
  this->extrusor.motor.SetSpeedDivider(1);
  this->variableData.mainBehavior = MOTOR_FORWARD;

  _SetMotorBehavior (MOTOR_FORWARD);
}

void StepSmartBase::SetParameter(SmartStepperPARAMETERS _param, byte _value[4]) {
}


void StepSmartBase::SetMaxSpeed(unsigned _maxSpeed) {
  // Implement the method to set the maximum speed
  this->extrusor.motor.SetMaxSpeed(_maxSpeed);
}

void StepSmartBase::SetMinSpeed(unsigned _minSpeed) {
  // Implement the method to set the minimum speed
  this->extrusor.motor.SetMinSpeed(_minSpeed);
}

void StepSmartBase::SetAcceleration(unsigned _acc) {
  // Implement the method to set the acceleration
  this->extrusor.motor.SetMainAcc(_acc);
}

void StepSmartBase::SetLimitCollisionsEvent(unsigned _collisionMax) {
  // Implement the method to set the collision event limit
  this->parameters.maxCollisionCounter = _collisionMax;
}

void StepSmartBase::SetPositionError(EncoderUnits _PositionError) {
  // Implement the method to set the position error
  this->parameters.maxErrorPositionAllowed = _PositionError;
}

void StepSmartBase::SetInvertLogic(byte _invertLogic) {
  // Implement the method to set the invert logic
  this->extrusor.motor.SetInvertLogic((_invertLogic >> 1), (_invertLogic & 1));

}

void StepSmartBase::SetStepsReverse(unsigned _StepReverse) {
  // Implement the method to set the steps reverse
  this->parameters.reverseSteps =_StepReverse;
}

void StepSmartBase::SetStepsBatch(long _steps) {
  // Implement the method to set the steps batch
  this->parameters.steps_per_batch = _steps;
}

void StepSmartBase::SetMotorBehavior (StepperBehavior _behavior){
  _SetMotorBehavior (_behavior);
}

bool StepSmartBase::SetPosition(long _position){

Serial.print("Go to Position: ");
Serial.println(_position);
  long _distanceTravel = _position - this->variableData.positionOrder;
  if(_distanceTravel < 0){
    this->variableData.mainBehavior = MOTOR_BACKWARD;
    this->SetSteps(_TransformPosition(abs(_distanceTravel)));
  }
  else{
    this->variableData.mainBehavior = MOTOR_FORWARD;
    this->SetSteps(_TransformPosition(abs(_distanceTravel)));
  }
  this->variableData.positionOrder = _position;


}

bool StepSmartBase::SetSteps(StepsUnits _steps) {
  this->variableData.stepsLeft = _steps;
  this->variableData.positionData.last = 0;
  this->variableData.positionData.main = 0;
  this->variableData.positionData.desired = 0;
  _PickPosition();
  this->parameters.steps_order = _steps;
  this->variableData.positionData.collisionCounter = 0;
  this->variableData.positionData.finalDesired = this->variableData.positionData.main + StepsUnitsToEncoderUnits(this->parameters.steps_order);
}

void StepSmartBase::GoHome() {
  return this->state;
}

long StepSmartBase::_PickPosition() {
  this->variableData.positionData.last = 0;
  this->variableData.positionData.main = 0;
  this->variableData.positionData.desired = 0;
  this->variableData.positionData.finalDesired = 0;
  return 0;
}

MotorState StepSmartBase::_Start() {
  Serial.println("Start");
  //this->variableData.stepsLeft = this->parameters.steps_order;
  this->variableData.positionData.desired = this->variableData.positionData.main;
  if (_CheckGoHome()) {
    return _GoHome();
  }
  Serial.println("Running");
  return MOTOR_STATE_RUNNING;
}

MotorState StepSmartBase::_End() {
  Serial.println("End");

  _PickPosition();
  this->variableData.stepsLeft = EncoderUnitsToStepsUnits(this->variableData.positionData.finalDesired - this->variableData.positionData.main);

  Serial.print("Steps Left to Goal :");
  Serial.print(this->variableData.stepsLeft.getLaps());
  Serial.print(" laps \t");
  Serial.print(this->variableData.stepsLeft.getTicks());
  Serial.print(" steps \t");

  if (this->variableData.stepsLeft < EncoderUnitsToStepsUnits(this->parameters.maxErrorPositionAllowed)) {
    this->parameters.steps_order = 0;
    this->variableData.stepsLeft = 0;
    return MOTOR_STATE_STANDBY;
  }

  else {
    return MOTOR_STATE_INITIALIZING;
  }

  return MOTOR_STATE_STANDBY;
}

MotorState StepSmartBase::_Collision() {

  if (!this->extrusor.motor.GetIsSteps2Do()) {

    if (this->variableData.collisionState == COLLISION_COLLIDED) {
      this->variableData.collisionState = COLLISION_REVERSE;
      _SetMotorBehavior (MOTOR_BACKWARD);
      //this->extrusor.motor.SetStepBatch(100);
      _ForceSetNewBatch(StepsUnits(0, this->parameters.reverseSteps));
      return MOTOR_STATE_COLLISION;
    }

    else if (this->variableData.collisionState == COLLISION_REVERSE) {
      this->variableData.collisionState = COLLISION_RECOVERING;
      _SetMotorBehavior (MOTOR_FORWARD);
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

bool StepSmartBase::_CheckGoHome() {

  return false;
}

MotorState StepSmartBase::_GoHome() {

  return MOTOR_STATE_INITIALIZING;
}

MotorState StepSmartBase::_RestoreCollision() {
  _PickPosition();
  this->variableData.stepsLeft = EncoderUnitsToStepsUnits(this->variableData.positionData.finalDesired - this->variableData.positionData.main);  //Reset Step Left
  this->variableData.positionData.desired = this->variableData.positionData.main;
  Serial.println("Recovered");
  return MOTOR_STATE_RUNNING;
}

bool StepSmartBase::_CheckCollision() {
  return false;
}
MotorState StepSmartBase::_Run() {
  //Check steps2Do

  if (this->variableData.stepsLeft > 0) {
    if (!this->extrusor.motor.GetIsSteps2Do()) {

      if (this->variableData.stepsLeft == this->parameters.steps_order) {
        //_ForceSetNewBatch(400);
        _PickPosition();
        this->variableData.stepBatch = _SetNewBatch();
        Serial.println("doFirstbatch");
        return MOTOR_STATE_RUNNING;
      } else {

        if (_CheckCollision()) {
          this->variableData.collisionState = COLLISION_COLLIDED;
          return MOTOR_STATE_COLLISION;
          //return MOTOR_STATE_STOPPED;
        } else {
          Serial.println("dobatch");
          this->variableData.stepBatch = _SetNewBatch();
          return MOTOR_STATE_RUNNING;
        }
      }

    } else {
      return MOTOR_STATE_RUNNING;
    }
  } else if (!this->extrusor.motor.GetIsSteps2Do()) return MOTOR_STATE_STOPPED;  // end stepsLeft
  else return MOTOR_STATE_RUNNING;
  //Check
}

void StepSmartBase::_SetMotorBehavior (StepperBehavior _behavior){
  if(this->variableData.mainBehavior == MOTOR_FORWARD){
    if(_behavior == MOTOR_FORWARD){
      this->extrusor.motor.SetMotorBehavior(MOTOR_FORWARD);
    }
    else if (_behavior == MOTOR_BACKWARD){
      this->extrusor.motor.SetMotorBehavior(MOTOR_BACKWARD);
    }
  }
  else if (this->variableData.mainBehavior == MOTOR_BACKWARD) {
    if(_behavior == MOTOR_FORWARD){
      this->extrusor.motor.SetMotorBehavior(MOTOR_BACKWARD);
    }
    else if (_behavior == MOTOR_BACKWARD){
      this->extrusor.motor.SetMotorBehavior(MOTOR_FORWARD);
    }
  }
  else {
    this->extrusor.motor.SetMotorBehavior(_behavior);
  }

}

StepsUnits StepSmartBase::_SetNewBatch() {
  //_PickPosition();
  if (this->variableData.stepsLeft >= this->parameters.steps_per_batch) {

    this->extrusor.motor.SetStepBatch(this->parameters.steps_per_batch);
    this->variableData.positionData.desired = this->variableData.positionData.desired + StepsUnitsToEncoderUnits(StepsUnits(0, this->parameters.steps_per_batch));
    this->variableData.stepsLeft = this->variableData.stepsLeft - this->parameters.steps_per_batch;
    Serial.println(this->variableData.stepsLeft.getTicks());
    return StepsUnits(0, this->parameters.steps_per_batch);

  } else {
    this->extrusor.motor.SetStepBatch((unsigned long)this->variableData.stepsLeft.getTicks());

    this->variableData.positionData.desired = this->variableData.positionData.desired + StepsUnitsToEncoderUnits(this->variableData.stepsLeft);
    Serial.println(this->variableData.stepsLeft.getTicks());
    this->variableData.stepsLeft = 0;
    //return _stepsBatch;
  }
}

StepsUnits StepSmartBase::_ForceSetNewBatch(StepsUnits _steps) {
  this->extrusor.motor.SetStepBatch((unsigned long)_steps.getTicks());
  return _steps;
}

StepsUnits StepSmartBase::_TransformPosition(long _position){
  return StepsUnits(0,(long)(_position * STEPS_PER_dMM));
}