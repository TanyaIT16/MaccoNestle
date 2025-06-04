/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 CompresorUnit.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


#ifndef _CompressorUnit
#include "CompressorUnit.h"
#include <Arduino.h>

CompressorUnit::CompressorUnit()
{
}

void CompressorUnit::Attach(ID_Definition _ID, ID_Definition _IDpowC, ID_Definition _IDpurgeC, byte _powC_Pin, byte _purgeC_Pin)
{
  this->ID.AttachID(_ID, T_Compressor);
  this->powerComp.Attach(_IDpowC, _powC_Pin);
  this->purgeComp.Attach(_IDpurgeC, _purgeC_Pin);
}

// Set the compressor ON
void CompressorUnit::TurnOnCompressor()
{
  this->powerComp.SetState(true);
}

// Set the compressor OFF
void CompressorUnit::TurnOffCompressor()
{
  this->powerComp.SetState(false);
}

// Set the purge valve OPEN
void CompressorUnit::TurnOnPurge()
{
  this->purgeComp.SetPos(this->purgeAngle);
}

// Set the purge valve CLOSE
void CompressorUnit::TurnOfPurge()
{
  this->purgeComp.GoMin();
}

// Return the compresor Power State
bool CompressorUnit::GetCompresorState()
{
  return this->powerComp.GetState(0);
}

// Return the purge valve State
bool CompressorUnit::GetPurgeState()
{
  return this->purgeComp.GetAngle();
}

// Set the angle for open the purgeValve (INPUT: angle)
void CompressorUnit::SetPurgeAngle(unsigned _angle)
{
  this->purgeAngle = _angle;
}

#endif