
/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 GoHome.h
                                                  0.0.02a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  29/03/24
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
StepperSmart: Control a Reciever SmartStepper CAN-BUS (NANO)

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#pragma once

#ifndef GOHOME
#define GOHOME

#include "Arduino.h"
#include "stepperDriver.h"

enum GoHomeStatus {
  GOHOME_NOHOMMED,
  GOHOME_HOMMING,
  GOHOME_REACHEDHOME,
  GOHOME_RETRYING_BACK,
  GOHOME_RETRYING_FOWARD,
  GOHOME_HOMMED
};

struct Action {
  GoHomeStatus status = GOHOME_NOHOMMED;
  StepperBehavior command;
  byte speedDivider = 1;
  unsigned long steps;
};

struct GoHomeInfo {
  GoHomeStatus status;
  unsigned steps_Per_Revolution;
  byte endSwitchPin;
  Action action;
};


class GoHomeBase {

private:
  GoHomeInfo info;
public:

  GoHomeBase();
  void Attach(byte _endSwitchPin, unsigned _steps_Per_Revolution) {
    pinMode(_endSwitchPin, INPUT);
    this->info.endSwitchPin = _endSwitchPin;
    this->info.steps_Per_Revolution = _steps_Per_Revolution;
    Reset();
  }


  void Reset() {
    SetMode(GOHOME_NOHOMMED);
    SetAction(MOTOR_DISABLED, 0);
  }


  void SetAction(StepperBehavior _command, unsigned long _steps) {
    this->info.action = _SetAction(_command, _steps);
  }

  Action Run() {
    Action _nextAction = _SetAction();
    switch (this->info.status) {
      case GOHOME_NOHOMMED:
        _nextAction = _SetAction();
        break;
      case GOHOME_HOMMING:
        _nextAction = _SetAction(MOTOR_BACKWARD, this->info.steps_Per_Revolution / 2);
        if (_ReadEndSwitch()) {
          this->info.status = GOHOME_REACHEDHOME;
        }
        break;
      case GOHOME_REACHEDHOME:
        _nextAction = _SetAction();
        this->info.status = GOHOME_RETRYING_BACK;
        break;
      case GOHOME_RETRYING_BACK:
        _nextAction = _SetAction(MOTOR_FORWARD, this->info.steps_Per_Revolution / 4, 4);
        this->info.status = GOHOME_RETRYING_FOWARD;
        break;
      case GOHOME_RETRYING_FOWARD:
        if (_ReadEndSwitch()) {
          this->info.status = GOHOME_HOMMED;
          _nextAction = _SetAction();
        } else {
          _nextAction = _SetAction(MOTOR_BACKWARD, this->info.steps_Per_Revolution / 4, 4);
        }
        break;
      case GOHOME_HOMMED:
        _nextAction = _SetAction();
        break;
      default:
      _nextAction = _SetAction();
        break;
    }
    return _nextAction;


    return _nextAction;
  }


  void GoHome(){
    this->info.status = GOHOME_HOMMING;
  }


  Action GetAction() {
    return this->info.action;
  }


  void SetMode(GoHomeStatus _status) {
    this->info.status = _status;
  }

private:

  bool _ReadEndSwitch() {
    return !digitalRead(this->info.endSwitchPin);
  }

  Action _SetAction(StepperBehavior _command, unsigned long _steps, byte _speedDivider) {
    Action _action;
    _action.command = _command;
    _action.steps = _steps;
    _action.speedDivider = _speedDivider;
    return _action;
  }

  Action _SetAction(StepperBehavior _command, unsigned long _steps) {
    Action _action;
    _action.command = _command;
    _action.steps = _steps;
    _action.speedDivider = 1;
    return _action;
  }

  Action _SetAction() {
    return _SetAction(MOTOR_DISABLED, 0, 1);
  }
};

#endif