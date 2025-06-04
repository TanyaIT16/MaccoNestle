#include "HardwareSerial.h"
#include "TimerOne.h"
/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 StepperSmart.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#include "Arduino.h"
#include "stepperDriver.h"

volatile byte StepperDriver::numberOfSteppers = 0;
volatile byte StepperDriver::_tick;
volatile StepperParameter StepperDriver::motorParameter;

volatile StepperSignal StepperDriver::signal[MAX_ALLOWED_STEPPERS];

//// STATIC METHODS  ////

void StepperDriver::__advanceStepISR()  // ISR to handle the the steps signals
{
  for (int i = 0; i < StepperDriver::numberOfSteppers; i++) {
    StepperDriver::_tick++;

    if (StepperDriver::signal[i]._IsSteps2Do) {
      if (!((int)(StepperDriver::_tick + 1) % StepperDriver::signal[i]._speedDivider)) {
        if (StepperDriver::signal[i]._steps2Do > 0) {
          StepperDriver::signal[i]._status = !StepperDriver::signal[i]._status;
          digitalWrite(StepperDriver::signal[i]._stepPin, StepperDriver::signal[i]._status);
          // digitalWrite(7, StepperDriver::signal[i]._status);
          StepperDriver::signal[i]._steps2Do--;
        } else {
          StepperDriver::signal[i]._status = 0;
          StepperDriver::signal[i]._IsSteps2Do = false;
        }
      }
    }
  }
  __AccelerationEvent();
}

void StepperDriver::__AccelerationEvent() {
  if ((ACC_DEFAULT > 0) && (StepperDriver::motorParameter.motorSpeed.mainSpeed > StepperDriver::motorParameter.motorSpeed.maxSpeed)) {
    StepperDriver::motorParameter.motorSpeed.mainSpeed -= ACC_DEFAULT;
    Timer1.setPeriod(motorParameter.motorSpeed.mainSpeed);
    //StepperDriver::UpdateTimerInterrupt(StepperDriver::motorParameter.motorSpeed.mainSpeed);
  }
}
void StepperDriver::StartTimerInterrupt()  // Start the timer ISR() for the stepper
{
  Timer1.initialize(StepperDriver::motorParameter.motorSpeed.mainSpeed);
  Timer1.attachInterrupt(__advanceStepISR);
}
void StepperDriver::UpdateTimerInterrupt(unsigned _speed)  // Update the timer ISR() for the stepper to a new freq
{
  Timer1.setPeriod(_speed);
}
void StepperDriver::UpdateTimerInterrupt()  // Update the timer ISR() for the stepper to default freq
{
  Timer1.setPeriod(StepperDriver::motorParameter.motorSpeed.mainSpeed);
}
void StepperDriver::HoldTimerInterrupt()  // Hold On the timer ISR() for the stepper
{
  Timer1.detachInterrupt();
}

void StepperDriver::SetMainSpeed(unsigned _speed) {
  if (_speed > StepperDriver::motorParameter.motorSpeed.maxSpeed && _speed < StepperDriver::motorParameter.motorSpeed.minSpeed) {
    StepperDriver::motorParameter.motorSpeed.mainSpeed = _speed;
  }
}
void StepperDriver::SetMinSpeed(unsigned _speed) {
  if (_speed > 0)
    StepperDriver::motorParameter.motorSpeed.minSpeed = _speed;
}
void StepperDriver::SetMaxSpeed(unsigned _speed) {
  if (_speed > 0 && _speed < StepperDriver::motorParameter.motorSpeed.minSpeed)
    StepperDriver::motorParameter.motorSpeed.maxSpeed = _speed;
  else
    StepperDriver::motorParameter.motorSpeed.maxSpeed = StepperDriver::motorParameter.motorSpeed.minSpeed;
}

void StepperDriver::SetMainAcc(unsigned _speed) {
  StepperDriver::motorParameter.motorSpeed.mainAcc = _speed;
}



//// METHODS ////

StepperDriver::StepperDriver() {
  SetSpeedDivider(1);
  if (StepperDriver::numberOfSteppers == 0) {
    StartTimerInterrupt();
  }

  if (StepperDriver::numberOfSteppers < MAX_ALLOWED_STEPPERS) {
    StepperDriver::numberOfSteppers++;
    Serial.println("SetMotor");
    this->stepperNumber = StepperDriver::numberOfSteppers;
  } else {
    //this->~StepperDriver();
  }
}

StepperDriver::~StepperDriver() {
  //SetSpeedDivider(1);
}

void StepperDriver::SetInvertLogic(bool invertEna, bool invertDir) {
  this->motorPins.invertLogic = ((invertEna << 1) & ENA_INVERT_MASK) + invertDir;
}

void StepperDriver::SetStepBatch(unsigned long _steps) {
  this->stepsBatch = _steps;
  StepperDriver::signal[this->stepperNumber - 1]._steps2Do = _steps;
  StepperDriver::signal[this->stepperNumber - 1]._IsSteps2Do = true;
}

void StepperDriver::SetStepBatch(StepsUnits _stepsTurns) {
  unsigned long _steps;
  _steps = _stepsTurns.getLaps() * STEPS_LAP + _stepsTurns.getTicks();
  this->stepsBatch = _stepsTurns;
  StepperDriver::signal[this->stepperNumber - 1]._steps2Do = _steps;
  StepperDriver::signal[this->stepperNumber - 1]._IsSteps2Do = true;
}

StepsUnits StepperDriver::GetStepBatch() {
  return this->stepsBatch;
}


bool StepperDriver::GetIsSteps2Do() {
  return StepperDriver::signal[this->stepperNumber - 1]._IsSteps2Do;
}

StepsUnits StepperDriver::GetSteps2Do() {
  return StepsUnits(0, StepperDriver::signal[this->stepperNumber - 1]._steps2Do);
}

bool StepperDriver::Attach(byte _stepPin, byte _dirPin, byte _enaPin, unsigned _stepsPerRevolution) {
  this->motorPins.stepPin = _stepPin;
  this->motorPins.dirPin = _dirPin;
  this->motorPins.enaPin = _enaPin;
  this->motorPins.stepsPerRevolution = _stepsPerRevolution;

  StepperDriver::signal[this->stepperNumber - 1]._stepPin = _stepPin;

  pinMode(_dirPin, OUTPUT);
  pinMode(_enaPin, OUTPUT);
  pinMode(_stepPin, OUTPUT);
  return 1;
}
bool StepperDriver::SetMotorBehavior(StepperBehavior _behavior) {
  if (this->motorBehavior != _behavior) {
    if (this->motorBehavior == MOTOR_HOLDED) {
    }

    this->motorBehavior = _behavior;

    bool invertEna = ((this->motorPins.invertLogic & ENA_INVERT_MASK)>> 1);
    bool invertDir = (this->motorPins.invertLogic & DIR_INVERT_MASK);

    switch (_behavior) {
      case MOTOR_STOPED:
        StepperDriver::motorParameter.motorSpeed.mainSpeed = StepperDriver::motorParameter.motorSpeed.minSpeed;
        digitalWrite(this->motorPins.enaPin, invertEna);
        break;
      case MOTOR_DISABLED:
        StepperDriver::motorParameter.motorSpeed.mainSpeed = StepperDriver::motorParameter.motorSpeed.minSpeed;
        digitalWrite(this->motorPins.enaPin, invertEna);
        break;
      case MOTOR_FORWARD:
        StepperDriver::motorParameter.motorSpeed.mainSpeed = StepperDriver::motorParameter.motorSpeed.minSpeed;
        digitalWrite(this->motorPins.enaPin, !invertEna);
        digitalWrite(this->motorPins.dirPin, invertDir);
        break;
      case MOTOR_BACKWARD:
        StepperDriver::motorParameter.motorSpeed.mainSpeed = StepperDriver::motorParameter.motorSpeed.minSpeed;
        digitalWrite(this->motorPins.enaPin, !invertEna);
        digitalWrite(this->motorPins.dirPin, !invertDir);
        break;
      case MOTOR_HOLDED:
        StepperDriver::motorParameter.motorSpeed.mainSpeed = StepperDriver::motorParameter.motorSpeed.minSpeed;
        digitalWrite(this->motorPins.enaPin, !invertEna);
        StepperDriver::signal[(this->stepperNumber - 1)]._speedDivider = 0;
        break;
      default:
        StepperDriver::motorParameter.motorSpeed.mainSpeed = StepperDriver::motorParameter.motorSpeed.minSpeed;
        digitalWrite(this->motorPins.enaPin, invertEna);
        return 0;
        break;
    }
    return 1;
  }
  return 0;
}
bool StepperDriver::SetSpeedDivider(byte _speedDivider) {
  if (_speedDivider > 0) {
    this->motorPins.speedDivider = _speedDivider;
    StepperDriver::signal[(this->stepperNumber - 1)]._speedDivider = _speedDivider;
    return 1;
  } else return 0;
}

StepperBehavior StepperDriver::GetMotorBehavior(StepperBehavior _behavior) {
  return this->motorBehavior;
}
byte StepperDriver::GetSpeedDivider(byte _speedDivider) {
  return StepperDriver::signal[(this->stepperNumber - 1)]._speedDivider;
}
