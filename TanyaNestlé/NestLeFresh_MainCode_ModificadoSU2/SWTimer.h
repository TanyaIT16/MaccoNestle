/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 SWTimer.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  25/09/22
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
SmartMotor: Control a Transciever SmartStepper CAN-BUS

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#pragma once
#ifndef SWTIMER
#define SWTIMER

struct AlarmVar
{                           // Struct to hold an Alarm data
  unsigned long timeAl = 0; // Time to trigg an alarm
  bool trigged = false;     // Is the alarm Triggered
};

struct TimerVar
{                                           // Struct to hold Time related data
  unsigned long lastTime = 0, initTime = 0; // last time , intial time
  AlarmVar alarm0, alarm1;                  // alarm 0 and alarm 1 objects
};

class SWTimer_Redux_Base
{
public:
  SWTimer_Redux_Base();                                      // Constructor
  bool Check(bool _lastInitFLAG, unsigned long _alarmMilis); // Function to update a timer, lastTime=now , check Trigger (returns 1 if Alarm0 is trigger)
  bool Check(unsigned long _alarmMilis);
  void ResetTimer();
  unsigned long GetRunTime();
  unsigned long GetDeltaTime(); // Returns delta time since the last update
protected:
  virtual unsigned long __GetTime(); // return the actual time VIRTUAL FUNCTION
private:
  unsigned long lastTime = 0, initTime = 0;
};

class SWTimer_Redux_Millis :public SWTimer_Redux_Base {
  public:
  SWTimer_Redux_Millis();
  private:
unsigned long __GetTime(); // return the actual time VIRTUAL FUNCTION
};

class SWTimer_Redux_Micros :public SWTimer_Redux_Base {
  public:
  SWTimer_Redux_Micros();
  private:
unsigned long __GetTime(); // return the actual time VIRTUAL FUNCTION
};

class SWTimer
{ // Base Class of a timer // Not For Use

public:
  SWTimer(); // Constructor

  void AtachAlarms(unsigned _alarm0Time, unsigned _alarm1Time); // Function to Initialize the alarms of a timer (INPUT time1 , time 2)
  void ResetTimer();                                            // Function to retest a Timer, set delta to 0, triggers=false, lastTime = Now
  bool Update();                                                // Function to update a timer, lastTime=now , check Trigger (returns 1 if Alarm0 is trigger)
  bool CheckAlarm0();                                           //  Check if the Alarm 0 is trigger (return 1 if triggered)
  bool CheckAlarm1();                                           //  Check if the Alarm 1 is trigger (return 1 if triggered)
  unsigned long GetDeltaTime(bool _alarmSel);
  unsigned long GetDeltaTime(); // Returns delta time since the last update
  unsigned long GetLastTime();  // Return last update time
  unsigned long GetNowTime();   // Return now time
  bool GetTrigger0();           // Return if Alarm0 has triggered
  bool GetTrigger1();           // Return if Alarm1 has triggered
  void ResetTriggers();         // Reset the triggers of alarm0 & alarm1

  void SetAlarm0(unsigned _time); // Set alarm0 timer
  void SetAlarm1(unsigned _time); // Set alarm1 timer

protected:
  virtual unsigned long __GetTime(); // return the actual time VIRTUAL FUNCTION

  TimerVar time; // time related data
};

class SWTimerMicros : public SWTimer
{ // Class to use the Timer in MicroSeconds [uS] (works well up to 50ms[in uS])
public:
  SWTimerMicros();

private:
  unsigned long __GetTime(); // return the actual time in microseconds [uS]
};

class SWTimerMilis : public SWTimer
{ // Class to use the Timer in MicroSeconds [mS] (works for longer timers)
public:
  SWTimerMilis();

private:
  unsigned long __GetTime(); // return the actual time in microseconds [mS]
};

#endif