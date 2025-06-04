/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 CompresorUnit.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  25/09/22
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
Compresor Unit contains
  - 1 Servo 
  - 2 H-Bridge

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#pragma once
#ifndef _CompressorUnit
#define _CompressorUnit

#include "ID_Def.h"
#include "Assets.h"

#ifndef _ServoUnitOld
#include "ServoUnit.h"
#endif

#ifndef _HBridge
#include "HBridge.h"
#endif



class CompressorUnit { // Class Compresor unit, Managge the Compresor, H-Bridge to turn on/off And Servo to periodically do the purge of the moisture.

public:
__ID ID; // Own ID

CompressorUnit(); // Constructor

void Attach(ID_Definition _ID,ID_Definition _IDpowC,ID_Definition _IDpurgeC,byte _powC_Pin,byte _purgeC_Pin); // Initialize the compresor unit (ID's - Pin's)

void TurnOnCompressor(); // Set the compressor ON
void TurnOffCompressor(); // Set the compressor OFF
void TurnOnPurge(); // Set the purge valve OPEN
void TurnOfPurge(); //Set the purge valve CLOSE
void SetPurgeAngle(unsigned _angle); // Set the angle for open the purgeValve (INPUT: angle)

bool GetCompresorState(); // Return the compresor Power State
bool GetPurgeState(); // Return the purge valve State

private:

unsigned purgeAngle = 180; // Angle to Open the purge valve
HBridge powerComp; // Compresor relay
ServoUnit purgeComp; // Compresor purge valve

};

#endif