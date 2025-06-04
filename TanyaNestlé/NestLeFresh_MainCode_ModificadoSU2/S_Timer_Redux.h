#include "Arduino.h"

#ifndef S_TIMER_REDUX
#define S_TIMER_REDUX

class S_Timer_Redux {
public:
  // Constructor
  S_Timer_Redux() {
    lastResetTime = initTime = __GetTime();
  }

  // Method to check if the timeout has occurred
  bool Check(unsigned long _TimeOut) {
    if (GetDeltaTime() >= _TimeOut) {
      // Timeout has occurred
      return true;
    }
    // Timeout has not occurred yet
    return false;
  }

  // Method to reset the timer
  void reset() {
    lastResetTime = __GetTime();
  }


  unsigned long GetDeltaTime() {
    return (__GetTime() - lastResetTime);
  }

  unsigned long GetInitTime() {
    return (__GetTime() - initTime);
  }
protected:

  virtual unsigned long __GetTime() {
    return millis();
  }

private:
  unsigned long lastResetTime, initTime;  // Store the last reset time
};



#endif