/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 FoldUnit.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  25/09/22
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
Fold Unit contains
  - 2 Servo 
  - 1 H-Bridge

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#pragma once
#ifndef _FoldUnit
#define _FoldUnit

#include "ID_Def.h"
#include "Assets.h"

#ifndef _HBridge
#include "HBridge.h"
#include "H_Bridge.h"
#endif

#ifndef _ServoUnitNew
#include "ServoUnit2.h"
#endif


class FoldUnit {  // Class FoldUnit that controls the FoldBagUnit (2 PWM servos + 1 H-Bridge (Piston))

public:
  __ID ID;  //Own ID

  FoldUnit();  // constructor

  void Attach(ID_Definition _ID, ID_Definition _IDj1, ID_Definition _IDj2, ID_Definition _IDeF, byte _J1_Pin, byte _J2_Pin, byte _eX0_Pin, byte _eX1_Pin, byte _eF0_Pin, byte _eF1_Pin, int servo1ID, int servo2ID);  //Initialize the FoldUnit (ID's / Pins)
  void ActivateServos();
  void DeactivateServos();
  void GotoFoldPos();                      // Go to Folding Position
  void DoFold();                           // Fold The Bag
  void MoveJoint(bool _j1j2, int _angle);  // Move Manually a single Joint (Servo) (INPUTS: ServoFlag, Angle)
  void MoveJoint_maxSpeed(bool _j1j2, int _angle);
  void GotoRestPos();    // Return to Home position
  void RetractPiston();  //Retract the piston
  void ExtendPiston();   // Extend the piston
  void CloseGrip();      //Retract the piston
  void OpenGrip();       // Extend the piston
  void Run();

private:
  H_Bridge_2Pins endEffector, extensor;  // Piston object H-Bridge
  ServoUnit2 joint1, joint2;       // Joint Objects Servo0,Servo1

};



#endif
