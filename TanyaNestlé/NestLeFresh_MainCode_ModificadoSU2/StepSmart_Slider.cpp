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
#include "StepSmart_Slider.h"
#define ENDSWITCH_PIN 4


StepSmartSlider::StepSmartSlider() {
  pinMode(ENDSWITCH_PIN, INPUT);
}


void StepSmartSlider::GoHome() {
  this->state = MOTOR_STATE_GOHOME;
}

bool StepSmartSlider::_CheckGoHome() {
  //Serial.println("GoHomeCheck");
  if (this->variableData.goHomeState != GOHOME_HOMMED) {
    this->variableData.goHomeState = GOHOME_NOHOMMED;
    return true;
  } else return false;
}

bool StepSmartSlider::_CheckEndSwitch() {
  return digitalRead(ENDSWITCH_PIN);
}

MotorState StepSmartSlider::_GoHome() {

  if (!this->extrusor.motor.GetIsSteps2Do()) {
    

    // NO HOMMED
    if (this->variableData.goHomeState == GOHOME_NOHOMMED) {
      if (_CheckEndSwitch()) {
        Serial.println("Hommed");
        //this->variableData.goHomeState = GOHOME_REACHEDHOME;
        this->variableData.goHomeState = GOHOME_HOMMED;
      } else {
        _SetMotorBehavior (MOTOR_BACKWARD);
        _ForceSetNewBatch(StepsUnits(0, 10));
        this->variableData.goHomeState = GOHOME_HOMMING;
      }
    }
    // HOMMING
    else if (this->variableData.goHomeState == GOHOME_HOMMING) {
      //Serial.println("Homming");
      if (_CheckEndSwitch()) {
        //Serial.println("GoHome_HomeTick");
        this->variableData.goHomeState = GOHOME_REACHEDHOME;
        //this->variableData.goHomeState = GOHOME_HOMMED;

      } else {
        this->variableData.goHomeState = GOHOME_NOHOMMED;
      }
    }

    // 1rst REACH HOME
    else if (this->variableData.goHomeState == GOHOME_REACHEDHOME) {
    //Serial.println("ReachHome");
      _SetMotorBehavior (MOTOR_FORWARD);
      _ForceSetNewBatch(StepsUnits(0, 800));
      this->extrusor.motor.SetSpeedDivider(3);
      this->variableData.goHomeState = GOHOME_RETRYING_BACK;
    }

    //RETRYING BACKWARD
    else if (this->variableData.goHomeState == GOHOME_RETRYING_BACK) {
      //Serial.println("Backing");
      _SetMotorBehavior (MOTOR_BACKWARD);
      _ForceSetNewBatch(StepsUnits(0, 10));
      this->variableData.goHomeState = GOHOME_RETRYING_FOWARD;
    }

    // RETRY FORWARD
    else if (this->variableData.goHomeState == GOHOME_RETRYING_FOWARD) {
      //Serial.println("Check");
      if (_CheckEndSwitch()) {
        //Serial.println("GoHome_HomedTick");
        this->variableData.goHomeState = GOHOME_HOMMED;

      } else {
        this->variableData.goHomeState = GOHOME_RETRYING_BACK;
      }
    }

    // HOMMED
    else if (this->variableData.goHomeState == GOHOME_HOMMED) {
      _SetMotorBehavior (MOTOR_FORWARD);
      this->variableData.positionOrder = 0;
      this->extrusor.motor.SetSpeedDivider(1);
      return MOTOR_STATE_RUNNING;
    }

    //Default Answer
    return MOTOR_STATE_GOHOME;
  }

  //Running GoHome Answer
  else
    return MOTOR_STATE_GOHOME;
}