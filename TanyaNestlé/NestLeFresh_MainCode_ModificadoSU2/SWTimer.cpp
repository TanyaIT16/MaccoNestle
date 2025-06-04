/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 SWTimer.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#include "Arduino.h"
#ifndef SWTIMER
#include "SWTimer.h"

SWTimer_Redux_Base::SWTimer_Redux_Base() // Constructor
{
  ResetTimer();
}

bool SWTimer_Redux_Base::Check(bool _lastInitFLAG, unsigned long _alarmMilis) // Function to update a timer, lastTime=now , check Trigger (returns 1 if Alarm0 is trigger)
{
  if (!_lastInitFLAG)
  {
    if (GetDeltaTime() > _alarmMilis)
    {
      this->lastTime = __GetTime();
      return true;
    }
  }
  else
  {
    if (GetRunTime() > _alarmMilis)
    {
      this->lastTime = __GetTime();
      return true;
    }
  }
  return false;
}

bool SWTimer_Redux_Base::Check(unsigned long _alarmMilis)
{
  if (GetDeltaTime() > _alarmMilis)
    {
      this->lastTime = __GetTime();
      return true;
    }
    else return false;
}

void SWTimer_Redux_Base::ResetTimer()
{
  this->initTime = this->lastTime = __GetTime();
}

unsigned long SWTimer_Redux_Base::GetRunTime() // Returns delta time since the last Restart
{
  return (__GetTime() - this->initTime);
}

unsigned long SWTimer_Redux_Base::GetDeltaTime() // Returns delta time since the last update
{
  return (__GetTime() - this->lastTime);
}

unsigned long SWTimer_Redux_Base::__GetTime() // return the actual time VIRTUAL FUNCTION
{
  return millis();
}

SWTimer_Redux_Millis::SWTimer_Redux_Millis() : SWTimer_Redux_Base()
{
}

unsigned long SWTimer_Redux_Millis::__GetTime()
{
  return millis();
}

SWTimer_Redux_Micros::SWTimer_Redux_Micros() : SWTimer_Redux_Base()
{
}

unsigned long SWTimer_Redux_Micros__GetTime()
{
  return micros();
}

SWTimer::SWTimer()
{ // Base Constructor of a timer
}

// Function to Initialize the alarms of a timer (INPUT time1 , time 2)
void SWTimer::AtachAlarms(unsigned _alarm0Time, unsigned _alarm1Time)
{
  SetAlarm0(_alarm0Time);
  SetAlarm1(_alarm1Time);
  ResetTimer();
  Update();
}

// Function to retest a Timer, set delta to 0, triggers=false, lastTime = Now
void SWTimer::ResetTimer()
{

  this->time.lastTime = __GetTime();
  this->time.initTime = __GetTime();
  ResetTriggers();
}

// Function to update a timer, lastTime=now , check Trigger (returns True if Alarm0 is trigger)
bool SWTimer::Update()
{
  CheckAlarm1();
  if (CheckAlarm0())
    this->time.lastTime = __GetTime();
  return this->time.alarm0.trigged;
}

//  Check if the Alarm 0 is trigger (return 1 if triggered)
bool SWTimer::CheckAlarm0()
{
  if (GetDeltaTime(1) > this->time.alarm0.timeAl)
  {
    this->time.alarm0.trigged = true;
    return 1;
  }
  else
    return 0;
}

//  Check if the Alarm 1 is trigger (return 1 if triggered)
bool SWTimer::CheckAlarm1()
{
  if (GetDeltaTime(1) > this->time.alarm1.timeAl)
  {
    this->time.alarm1.trigged = true;
    return 1;
  }
  else
    return 0;
}

// Returns delta time since the last update
unsigned long SWTimer::GetDeltaTime()
{
  return GetDeltaTime(1);
}

unsigned long SWTimer::GetDeltaTime(bool _alarmSel)
{
  if (!_alarmSel)
  {
    return (__GetTime() - this->time.lastTime);
  }
  else
  {
    return (__GetTime() - this->time.initTime);
  }
}

// Return last update time
unsigned long SWTimer::GetLastTime()
{
  return this->time.lastTime;
}

// Return now time
unsigned long SWTimer::GetNowTime()
{
  return __GetTime();
}

// Return if Alarm 0 has triggered
bool SWTimer::GetTrigger0()
{
  return this->time.alarm0.trigged;
}

// Return if Alarm 1 has triggered
bool SWTimer::GetTrigger1()
{
  return this->time.alarm1.trigged;
}

// Reset the triggers of alarm0 & alarm1
void SWTimer::ResetTriggers()
{
  this->time.alarm0.trigged = false;
  this->time.alarm1.trigged = false;
}

// Set alarm 0 timer
void SWTimer::SetAlarm0(unsigned _time)
{
  this->time.alarm0.timeAl = _time;
}

// Set alarm 1 timer
void SWTimer::SetAlarm1(unsigned _time)
{
  this->time.alarm1.timeAl = _time;
}

// Class to use the Timer in MicroSeconds [uS] (works well up to 50ms[in uS])
SWTimerMicros::SWTimerMicros()
{
}

// return the actual time in microseconds [uS]
unsigned long SWTimerMicros::__GetTime()
{
  return micros();
}

// Class to use the Timer in MicroSeconds [mS] (works for longer timers)
SWTimerMilis::SWTimerMilis()
{
}

// return the actual time in miliseconds [mS]
unsigned long SWTimerMilis::__GetTime()
{
  return millis();
}

#endif