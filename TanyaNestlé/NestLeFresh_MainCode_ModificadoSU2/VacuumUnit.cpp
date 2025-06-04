/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 VacuumUnit.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#ifndef _VacuumUnit
#include <Arduino.h>
#include "VacuumUnit.h"

// Store the total number of VacuumUnits
byte VacuumUnit::nVacuumUnit = 0;

// Class VacuumUnit, Derived from Hbridge with a digitalVacuumSensor. Handles an electrovalve for vacuum
VacuumUnit::VacuumUnit() {
}

// Initialize the VacuumUnit (IDvacuum, IDSensor , PinVacuum , PinSensor )
void VacuumUnit::Attach(ID_Definition _vacID, ID_Definition _sensID, byte _vacPin, byte _sensPin) {

  if (_vacPin != defPin) {
    this->pin0 = _vacPin;
    pinMode(_vacPin, OUTPUT);
  }
  if (_sensPin != defPin) {
    this->sensPin = _sensPin;
    pinMode(_sensPin, INPUT);
  }
  
  
  this->ID.AttachID(_vacID, T_Vaacum);
  this->IDsens.AttachID(_sensID, T_DigSensor);
  this->SetEna(1);
}

// Read the Value of the vacuumSensor (Returns True if Vacuum)
bool VacuumUnit::ReadVacuum() {
  return (true);
  if (this->sensPin != defPin){
    return digitalRead(this->sensPin);
    int a= digitalRead(this->sensPin);
    //Serial.println("AAAAAAAAAAAAAAAAAAAAA " + String(a));
  }
  else
  {
    return 0;
  }

}

// Returns the PhisicalPin of the VacuumSensor
byte VacuumUnit::GetVacuumPin() {
  return this->sensPin;
}

// Returns the Number of VacuumUnits Registered
byte VacuumUnit::GetTypeCount() {
  return VacuumUnit::nVacuumUnit;
}

#endif
