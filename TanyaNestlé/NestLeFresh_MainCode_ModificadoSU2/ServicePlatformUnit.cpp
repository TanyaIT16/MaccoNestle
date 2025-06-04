/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 ServicePlatform.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#ifndef _ServicePlatformUnit
#include "ServicePlatformUnit.h"
#include <Arduino.h>

// Class ServicePlatform Unit, Handle the service platform and the locking door (1 smartSteper-CANBus, 1 H-Bridge)
ServicePlatformUnit::ServicePlatformUnit() {
}

// Initialize the service platform (ID's / Pins)
void ServicePlatformUnit::Attach(ID_Definition _ID, ID_Definition _IDdL, ID_Definition _IDpS, byte _dL_Pin, byte _pB_Pin1, byte _pB_Pin2, byte _mD_Pin, byte _sD_Pin, byte _bH_Pin, CANmannager *_can) {
  this->can = _can;
  this->ID.AttachID(_ID, T_ServicePlatform);
  this->platformSlider.Attach(_IDpS, this->can);
  this->doorLock.Attach(_ID, _dL_Pin);
  this->pusherBag.Attach(_ID, _pB_Pin1, _pB_Pin2);
  mainDoor_InputPin = _mD_Pin;
  pinMode(_mD_Pin, INPUT);
  serviceDoor_InputPin = _sD_Pin;
  pinMode(_sD_Pin, INPUT);
  bagSensor_InputPin = _bH_Pin;
  pinMode(_bH_Pin, INPUT);
}

// Extend the platform to serve the product
void ServicePlatformUnit::ExtendPlatform() {
  this->pusherBag.SetState(true);
}

// Retract the platform to prepare the product
void ServicePlatformUnit::RetractPlatform() {
  this->pusherBag.SetState(false);
  //this->platformSlider.SetValue(this->retractPos);
}

// Move the platform to a defired amount of distance
void ServicePlatformUnit::SetPlatformPoints(bool _eR, unsigned _x) {
  if (_eR) {
    this->retractPos = _x;
  } else {
    this->extendPos = _x;
  }
}

void ServicePlatformUnit::GoHome() {
  this->platformSlider.GoHome();
}

// Return the platform position (INPUT: position)
void ServicePlatformUnit::MovePlatform(float _x) {
  //this->platformSlider.SetValue(_x);
  this->platformSlider.GoToPosition(_x);
}

// Set the platform service and prepare points (INPUTS: extender-retractedFLAG, distance)
unsigned ServicePlatformUnit::GetPlatformPos() {
  unsigned _pos = 0;
  // this->platformSlider.
  return _pos;
}

// Open the service LockDoor
void ServicePlatformUnit::OpenDoor() {
  this->doorLock.SetState(true);
}

// Close the service LockDoor
void ServicePlatformUnit::CloseDoor() {
  this->doorLock.SetState(false);
}

// Get the electromagnet Lock state
bool ServicePlatformUnit::GetDoorState() {
  return true;
  return this->doorLock.GetState(0);
}

bool ServicePlatformUnit::GetMainDoorState() {
  return true;
  return digitalRead(mainDoor_InputPin);
}
bool ServicePlatformUnit::GetServiceDoorState() {
  return true;
  return digitalRead(serviceDoor_InputPin);
}
bool ServicePlatformUnit::GetBagSensorRead() {
  return true;
  return digitalRead(bagSensor_InputPin);
}


#endif