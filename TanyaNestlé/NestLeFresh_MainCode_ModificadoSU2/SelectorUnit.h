/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 SelectorUnit.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  26/03/24
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
Selector contains
    - 1 Hbridge (Mixer)
    -1 Relay (GrinderPow)
    - 2 Servo (Doors)
    -1 Servo (Size)
************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#pragma once
#ifndef _SelectorUnit
#define _SelectorUnit

#include "ID_Def.h"
#include "Assets.h"

#ifndef _ServoUnitOld
#include "ServoUnit.h"
#endif

#ifndef _HBridge
#include "HBridge.h"
#endif



#define MIN_ANGLE_DOOR_B 50
#define MAX_ANGLE_DOOR_B 160

#define MIN_ANGLE_DOOR_G 10
#define MAX_ANGLE_DOOR_G 95

#define MIN_ANGLE_SIZE 0
#define MAX_ANGLE_SIZE 180

#define CLOSED_ANGLE_BEANS_DOOR 38
#define OPEN_ANGLE_BEANS_DOOR 20

enum SelectorState{
  SELECTOR_CLOSED = 'c',
  SELECTOR_GRINDER = 'g',
  SELECTOR_BYPASS = 'b' 
};

enum GrinderState{
  GRINDER_OFF = '0',
  GRINDER_ON = '1',
  GRINDER_SIZE = 's'
};

enum PowderSize {
  POWDERSIZE_MIN = 0,
  POWDERSIZE_FINE = 8,
  POWDERSIZE_MID = 16,
  POWDERSIZE_COARSE = 24,
  POWDERSIZE_MAX = 63,
};


class SelectorUnit {  // Class Selector Unit, handles the behaivor of the selector, open and close the selector doors, manage the mixer, grinder and grinder size

public:
  __ID ID;  // Own ID

  SelectorUnit();                                                                                                                                                                                                                 // Constructor
                                                                                                                                                                                                                                  /*Pin_SelByPassPWM = 3,   // 1
  Pin_SelGrinderPWM = 4,  // 2

  Pin_PowGrinder = 42,
  Pin_SizeGrinderPWM = 2,  // 3
  Pin_PowVibrator = 43,*/
  void Attach(ID_Definition _ID, ID_Definition _IDgD, ID_Definition _IDbD, ID_Definition _IDgP, ID_Definition _IDgS, ID_Definition _IDmP, byte _gD_Pin, byte _bD_Pin, byte _gP_Pin, byte _gS_Pin, byte _mP_Pin0, byte _mP_Pin1, byte _eD_Pin);  // Initialize the service platform (ID's / Pins)
  void ActivateServos();
  void DeactivateServos();
  void CloseBeansDoor();
  void SetBeansDoor(unsigned _angle);
  void OpenBeansDoor();
  void CloseSelectorDoor();
  void OpenGrinderDoor();
  void OpenByPassDoor();
  void TuneServoSelector(bool _servo, unsigned _angle);
  void ToogleMixer();
  void TurnOnMixer();
  void TurnOffMixer();
  void TurnOnGrinder();
  void TurnOffGrinder();
  void SetGrinderSize(byte _size);


private:

  HBridge Mixer;
  HBridge Grinder;
  ServoUnit GrinderDoor, ByPassDoor, GrindSize , BeansDoor;
};

#endif