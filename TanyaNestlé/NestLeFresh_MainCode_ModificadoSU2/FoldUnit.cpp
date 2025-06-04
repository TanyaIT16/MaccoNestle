/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 FoldUnit.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



#ifndef _FoldUnit
#include "FoldUnit.h"
#include <Arduino.h>

// Class FoldUnit that controls the FoldBagUnit (2 PWM servos + 1 H-Bridge (Piston))
FoldUnit::FoldUnit() {}

//Initialize the FoldUnit (ID's / Pins)
void FoldUnit::Attach(ID_Definition _ID, ID_Definition _IDj1, ID_Definition _IDj2, ID_Definition _IDeF, byte _J1_Pin, byte _J2_Pin, byte _eX0_Pin, byte _eX1_Pin, byte _eF0_Pin, byte _eF1_Pin, int servo1ID, int servo2ID) {
  this->ID.AttachID(_ID, T_Fold);
  this->joint1.Attach(_IDj1, _J1_Pin, 180, 0, servo1ID);
  this->joint2.Attach(_IDj2, _J2_Pin,0,180, servo2ID);
  this->endEffector.Attach(_eF0_Pin, _eF1_Pin);
  this->extensor.Attach(_eX0_Pin, _eX1_Pin);
}

void FoldUnit::ActivateServos() {
  this->joint1.Activate();
  this->joint2.Activate();
}
void FoldUnit::DeactivateServos() {
  this->joint1.Deactivate();
  this->joint2.Deactivate();
}


void FoldUnit::Run() {
  this->joint1.Run();
  this->joint2.Run();
}


// Go to Folding Position
void FoldUnit::GotoFoldPos() {
  this->joint1.GoMax();
  this->joint2.GoMin();
}

// Fold The Bag
void FoldUnit::DoFold() {
  this->joint2.GoMax();
}

// Move Manually a single Joint (Servo) (INPUTS: ServoFlag, Angle)
void FoldUnit::GotoRestPos() {
  this->joint1.GoMin();
  this->joint2.GoMin();
  FoldUnit::RetractPiston();
}

// Return to Home position
void FoldUnit::MoveJoint(bool _j1j2, int _angle) {
  if (_j1j2) {
    this->joint2.SetPos(_angle, 1);//***************************Modificar esta
  } else {
    this->joint1.SetPos(_angle, 1);//***************************Modificar esta
  }
}

void FoldUnit::MoveJoint_maxSpeed(bool _j1j2, int _angle) {
  if (_j1j2) {
    this->joint2.SetPos(_angle);// ********************************Modificar esta
  } else {
    this->joint1.SetPos(_angle);//*********************************Modificar esta 
  }
}

//Retract the piston
void FoldUnit::RetractPiston() {
  // Serial.println("RetractPiston");
  this->extensor.SetState(HB_BACKWARD);
}

// Extend the piston
void FoldUnit::ExtendPiston() {
  // Serial.println("ExtendPiston");
  this->extensor.SetState(HB_FORWARD);
}

void FoldUnit::CloseGrip() {
  //Serial.println("RetractGrip");
  this->endEffector.SetState(HB_BACKWARD);

}  //Retract the piston
void FoldUnit::OpenGrip() {
  //Serial.println("ExtendGrip");
  this->endEffector.SetState(HB_FORWARD);
}  // Extend the piston

#endif
