/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 SerialMannager.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  25/09/22
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
*************************************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SERIALMANNAGER
#define SERIALMANNAGER

#include <Arduino.h>
#include "Position.h"
#include "SelectorUnit.h"

#define SEPARATION_CHARACTER '/'
#define ORDER_CHARACTER '#'
#define MAX_DATA_ALLOWED 7
#define DEFAULT_SERIAL_BAUDRATE 115200

enum Verbose_Level {
  VERBOSE_RUNTIME = 0,  // No verbose output
  VERBOSE_ERROR = 1,    // Only errors
  VERBOSE_WARNING = 2,  // Errors and warnings
  VERBOSE_INFO = 3,     // Errors, warnings, and informational messages
  VERBOSE_DEBUG = 4     // Errors, warnings, informational messages, and debug messages
};

enum MainStatuses {
  STATUS_MAIN_NO_DEF,
  STATUS_MAIN_STANDBY,
  STATUS_MAIN_RUNNING,
  STATUS_MAIN_HALT,
  STATUS_MAIN_MANUAL,
};

struct SerialOrder {
  char incomingChar = ' ';
  char auxChar = ' ';
  long serialValue[MAX_DATA_ALLOWED] = { 0 };
};

struct MainOrder {
  bool checkNewOrder = false;
  char incomingChar = ' ';
  char auxChar = ' ';
  unsigned grams[6] = { 0 };
  unsigned totalGrams = 0;
  PowderSize grindSize;
  SelectorState selectorState = SELECTOR_CLOSED;
  MainStatuses mainStatus = STATUS_MAIN_NO_DEF;
};

struct SerialData {

  SerialOrder order;
  String numericValueString = "";
};

class SerialMannager {

public:

  static unsigned numberOfSerial;


private:

  Verbose_Level verboseLevel = VERBOSE_DEBUG;

public:


  SerialMannager();

  void begin(unsigned _baudrate = DEFAULT_SERIAL_BAUDRATE);

  void SetVerboseLevel(Verbose_Level _level);

  void print(int VerboseLevel, const String& message);
  void println(int VerboseLevel, const String& message);

  void print(int VerboseLevel, const long& message);
  void println(int VerboseLevel, const long& message);

  void print(int VerboseLevel, const StepsUnits& message);
  void println(int VerboseLevel, const StepsUnits& message);

  void print(int VerboseLevel, const EncoderUnits& message);
  void println(int VerboseLevel, const EncoderUnits& message);

  MainOrder ReadData();
};

#endif