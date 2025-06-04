/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 VacuumUnit.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  25/09/22
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
Selector Unit contains
  - 1 HBridge (derived)
  - 1 Vacuum Sensor
************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#pragma once
#ifndef _VacuumUnit
#define _VacuumUnit

#include "ID_Def.h"
#include "Assets.h"

#ifndef _HBridge
#include "HBridge.h"
#endif

#ifndef defPin
#define defPin 256 // Default Not Asigned Pin
#endif

class VacuumUnit : public HBridge // Class VacuumUnit, Derived from Hbridge with a digitalVacuumSensor. Handles an electrovalve for vacuum
{
  // ID of vacuum derived from H-bridge
  __ID IDsens; // ID of the sensor
public:
  static byte nVacuumUnit; // Store the total number of VacuumUnits
  VacuumUnit();            // Constructor

  void Attach(ID_Definition _vacID, ID_Definition _sensID, byte _vacPin, byte _sensPin); // Initialize the VacuumUnit (IDvacuum, IDSensor , PinVacuum , PinSensor )
  bool ReadVacuum();                                                                     // Read the Value of the vacuumSensor (Returns True if Vacuum)
  byte GetVacuumPin();                                                                   // Returns the PhisicalPin of the VacuumSensor
  byte GetTypeCount();                                                                   // Returns the Number of VacuumUnits Registered

private:
  byte sensPin = defPin; // PhisicalPin of the Sensor
};

#endif