/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 SelectorUnit.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  26/03/24

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#include "SelectorUnit.h"


// Class ServicePlatform Unit, Handle the service platform and the locking door (1 smartSteper-CANBus, 1 H-Bridge)
SelectorUnit::SelectorUnit() {
}

// Initialize the service platform (ID's / Pins)
/*Pin_SelByPassPWM = 3,   // 1
  Pin_SelGrinderPWM = 4,  // 2

  Pin_PowGrinder = 42,
  Pin_SizeGrinderPWM = 2,  // 3
  Pin_PowVibrator = 43,*/
void SelectorUnit::Attach(ID_Definition _ID, ID_Definition _IDgD, ID_Definition _IDbD, ID_Definition _IDgP, ID_Definition _IDgS, ID_Definition _IDmP, byte _gD_Pin, byte _bD_Pin, byte _gP_Pin, byte _gS_Pin, byte _mP_Pin0, byte _mP_Pin1, byte _eD_Pin)  // Initialize the service platform (ID's / Pins)
{
  this->ID.AttachID(_ID, T_Selector);
  this->ByPassDoor.Attach(_IDbD, _bD_Pin, MAX_ANGLE_DOOR_B);
  this->GrinderDoor.Attach(_IDgD, _gD_Pin, MIN_ANGLE_DOOR_G);
  this->BeansDoor.Attach(_ID, _eD_Pin, CLOSED_ANGLE_BEANS_DOOR);
  this->GrindSize.Attach(_IDgS, _gS_Pin, POWDERSIZE_FINE);

  this->Grinder.Attach(_IDgP, _gP_Pin);
  this->Mixer.Attach(_IDmP, _mP_Pin0, _mP_Pin1);
  CloseBeansDoor();
  CloseSelectorDoor();
}

void SelectorUnit::ActivateServos() {
  this->ByPassDoor.Activate();
  this->GrinderDoor.Activate();
  this->GrindSize.Activate();
  this->BeansDoor.Activate();
}
void SelectorUnit::DeactivateServos() {
  this->ByPassDoor.Deactivate();
  this->GrinderDoor.Deactivate();
  this->GrindSize.Deactivate();
  this->BeansDoor.Deactivate();
}

void SelectorUnit::CloseBeansDoor() {
  this->BeansDoor.SetPos(CLOSED_ANGLE_BEANS_DOOR);
}
void SelectorUnit::SetBeansDoor(unsigned _angle) {
  this->BeansDoor.SetPos(_angle);
}
void SelectorUnit::OpenBeansDoor() {
  this->BeansDoor.SetPos(OPEN_ANGLE_BEANS_DOOR);
}


void SelectorUnit::CloseSelectorDoor() {
  this->ByPassDoor.SetPos(MAX_ANGLE_DOOR_B);
  this->GrinderDoor.SetPos(MIN_ANGLE_DOOR_G);
}
void SelectorUnit::OpenGrinderDoor() {

  this->ByPassDoor.SetPos(MIN_ANGLE_DOOR_B);
  this->GrinderDoor.SetPos(MIN_ANGLE_DOOR_G);
}
void SelectorUnit::OpenByPassDoor() {
  this->ByPassDoor.SetPos(MAX_ANGLE_DOOR_B);
  this->GrinderDoor.SetPos(MAX_ANGLE_DOOR_G);
}

void SelectorUnit::TuneServoSelector(bool _servo, unsigned _angle) {
  if (_servo) {
    this->ByPassDoor.SetPos(_angle);
  } else {
    this->GrinderDoor.SetPos(_angle);
  }
}


void SelectorUnit::ToogleMixer() {
}
void SelectorUnit::TurnOnGrinder() {
  this->Grinder.SetState(true);
}
void SelectorUnit::TurnOffGrinder() {
  this->Grinder.SetState(false);
}

void SelectorUnit::TurnOnMixer() {
  this->Mixer.SetState(false);
}
void SelectorUnit::TurnOffMixer() {
  this->Mixer.SetState(false);
}

void SelectorUnit::SetGrinderSize(byte _size) {
  if (_size > POWDERSIZE_MAX)
    _size = POWDERSIZE_MAX;
  int _angle = map(_size, 0, POWDERSIZE_MAX, MIN_ANGLE_SIZE, MAX_ANGLE_SIZE);
  this->GrindSize.SetPos(_angle);
}
