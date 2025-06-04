/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 ServicePlatform.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  25/09/22
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
ServicePlatform contains
    - 1 SmartStepper (CAN-Bus)
    - 1 Servo
************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#pragma once
#ifndef _ServicePlatformUnit
#define _ServicePlatformUnit

#include "ID_Def.h"
#include "Assets.h"

#ifndef _SmartMotor
#include "SmartMotor.h"
#endif

#ifndef _HBridge
#include "HBridge.h"
#endif

enum DoorStates {
  Door_NoDef = -1,
  Door_Closed,    // A1S1 >> action
  Door_Released,  // A0S0 >> timed
  Door_Opened,    // A0S0
  Door_Error
};

class ServicePlatformUnit {  // Class ServicePlatform Unit, Handle the service platform and the locking door (1 smartSteper-CANBus, 1 H-Bridge)

public:
  __ID ID;  // Own ID

  ServicePlatformUnit();  // Constructor

  void Attach(ID_Definition _ID, ID_Definition _IDdL, ID_Definition _IDpS, byte _dL_Pin, byte _pB_Pin1, byte _pB_Pin2, byte _mD_Pin, byte _sD_Pin, byte _bH_Pin, CANmannager *_can);  // Initialize the service platform (ID's / Pins)
  void ExtendPlatform();                                                                                                                                                              // Extend the platform to serve the product
  void RetractPlatform();                                                                                                                                                             // Retract the platform to prepare the product
  void MovePlatform(float _x);                                                                                                                                                        // Move the platform to a defired amount of distance
  void GoHome();
  unsigned GetPlatformPos();                      // Return the platform position
  void SetPlatformPoints(bool _eR, unsigned _x);  // Set the platform service and prepare points (INPUTS: extender-retractedFLAG, distance)
  void OpenDoor();                          // Open the service LockDoor
  void CloseDoor();                         // Close the service LockDoor
  bool GetDoorState();
  bool GetMainDoorState();
  bool GetServiceDoorState();
  bool GetBagSensorRead();  // Get the electromagnet Lock state

private:
  int extendPos = 0, retractPos = 0;
  byte mainDoor_InputPin;
  byte serviceDoor_InputPin;
  byte bagSensor_InputPin;
  DoorStates doorState = Door_NoDef;
  HBridge doorLock;
  HBridge pusherBag;
  CANmannager *can;
  SliderMannager platformSlider;
};

#endif