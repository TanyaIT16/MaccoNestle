/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 SmartMotor.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#ifndef _SmartMotor
#include <Arduino.h>
#include "SmartMotor.h"
//#include "StepperSmart.h"


byte SmartMotor::nSmartMotors = 0;
byte SmartMotor::IDMaster = ID_Master;


SmartMotor::SmartMotor() {
  this->motorNumber = nSmartMotors;
  nSmartMotors++;
}

void SmartMotor::Attach(ID_Definition _ID, CANmannager *_can) {
  this->ID.AttachID(_ID, T_SmartMotor);
  this->can = _can;
}

void SmartMotor::SetParam(SmartStepperPARAMETERS _param, int32_t _value) {
  SendMSG(this->can, this->ID.GetID(), MSG_PARAMETER, LONG, false, (byte)_param, _value);
}




ExtrusorMannager::ExtrusorMannager(float _gramsPerRev)
  : SmartMotor() {
  this->gramsPerRevolution = _gramsPerRev;
}

void ExtrusorMannager::ExtrudeGrams(unsigned _grams) {
  if (_grams != 0 || true) {
    SendStepsMSG(this->can, this->ID.GetID(), MSG_PARAMETER, LONG, false, P_RefSteps, _TransformGramsToSteps(_grams));
    byte _index;
    switch (this->ID.GetID()) {
      case ID_E0:
        _index = extruder0;
        break;
      case ID_E1:
        _index = extruder1;
        break;
      case ID_E2:
        _index = extruder2;
        break;
      case ID_E3:
        _index = extruder3;
        break;
      case ID_E4:
        _index = extruder4;
        break;
      case ID_E5:
        _index = extruder5;
        break;
    }
    counterACK.SetNewExtruderACK(ACK_unknown, this->ID.GetID(), _index);
    //counterACK.SetNewExtruderACK(ACK_standby, this->ID.GetID(), _index);
  }
}
void ExtrusorMannager::SetGramsPerRevolution(const float _gramsPerRev) {
  this->gramsPerRevolution = _gramsPerRev;
}
float ExtrusorMannager::GetGramsPerRevolution() {
  return this->gramsPerRevolution;
}
StepsUnits ExtrusorMannager::_TransformGramsToSteps(unsigned _grams) {
  Serial.print("grams ");
  Serial.println(_grams);
  long _steps = (float)(_grams / this->gramsPerRevolution) * 1600;
  return StepsUnits(0, _steps);
}




SliderMannager::SliderMannager()
  : SmartMotor() {
}
void SliderMannager::GoToPosition(float milimeters) {
  long _pos = (milimeters * 10);
  SendMSG(this->can, this->ID.GetID(), MSG_PARAMETER, LONG, false, P_RefPos, _pos);
  //Serial.println("GoTO movement");
  byte _index;
  switch (this->ID.GetID()) {
    case ID_MotorSlider11:
      _index = Slider11;
      break;
    case ID_MotorSlider12:
      _index = Slider12;
      break;
    case ID_MotorSlider21:
      _index = Slider21;
      break;
    case ID_MotorSlider22:
      _index = Slider22;
      break;
    case ID_MotorServicePlatform:
      _index = SliderSP;
      break;
  }
  counterACK.SetNewSliderACK(ACK_unknown, this->ID.GetID(), _index);
  //counterACK.SetNewSliderACK(ACK_standby, this->ID.GetID(), _index);
}

void SliderMannager::GoHome() {
  delay(1);
  SendMSG(this->can, this->ID.GetID(), MSG_GOHOME, BYTE, false, P_RefPos, 0);
  delay(1);
  byte _index;
  switch (this->ID.GetID()) {
    case ID_MotorSlider11:
      _index = Slider11;
      break;
    case ID_MotorSlider12:
      _index = Slider12;
      break;
    case ID_MotorSlider21:
      _index = Slider21;
      break;
    case ID_MotorSlider22:
      _index = Slider22;
      break;
    case ID_MotorServicePlatform:
      _index = SliderSP;
      break;
  }
  //counterACK.SetNewSliderACK(ACK_unknown, this->ID.GetID(), _index);
  counterACK.SetNewSliderACK(ACK_standby, this->ID.GetID(), _index);
}

#endif