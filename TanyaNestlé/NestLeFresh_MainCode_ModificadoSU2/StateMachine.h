/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 StateMachine.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  25/09/22
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
Handles a state machine type
************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#pragma once
#ifndef _StateMachine
#define _StateMachine

#include "Assets.h"

#include "S_Timer_Redux.h"

#define DEFAULT_TIMEOUT 120000
#define MAX_STATES 64
#define MAX_RETRIES 8
#define COUNTER_ERROR 255

enum StateMachineStatus {
  SS_NotInitialize,
  SS_StandBy,
  SS_Initializing,
  SS_Running,
  SS_Finishing,
  SS_Finished,
  SS_SecureMode
};

struct State {
  unsigned timeOut;
  ERROR_Type (*stateStart_Ptr)() = NULL;
  ERROR_Type (*state_Ptr)() = NULL;
  ERROR_Type (*stateEnd_Ptr)() = NULL;
};


class StateMachine {
private:
  /* data */
  State listStates[MAX_STATES];  // Main list of states
  ERROR_Type (*mainState_Ptr)() = NULL;
  byte numState = 0, actualState = 0, nextState = 0, timeOutState;
  StateMachineStatus status = SS_NotInitialize;
  S_Timer_Redux timer0;
public:
  __ID ID;  // Own ID
  byte counter;
  byte RetriesCount = 0;
  S_Timer_Redux timerUser;
  StateMachine(byte _timeOutState = (MAX_STATES));  // Constructor Initialize State Machine

  ~StateMachine();  // Destructor
  byte Counter_Fail(byte _failCounter, byte _retryCounter);
  byte Counter_Reset(byte _init);
  ERROR_Type Run();  // Run the actual State (Also OneShot Start / End methods)
  ERROR_Type SetNextState(byte _index);
  byte GetState();
  byte GetSafeState();
  byte GetNextState();                                                                                                                                                                       // Change to a new state ->Search if the state exist if not Return Error
  ERROR_Type AttachNewState(byte _index, ERROR_Type (*_state_Ptr)(), ERROR_Type (*_stateStart_Ptr)(), ERROR_Type (*_stateEnd_Ptr)(), byte _safeState, byte _nextState, unsigned _timeOut0);  // Add a new state to the list
  ERROR_Type AttachMainState(ERROR_Type (*_state_Ptr)());  // Add a main state to the list
  ERROR_Type AvanceState();
  ERROR_Type DeleteState(byte _index);
  ERROR_Type DeleteState();  // Delete If exist, a state, else Return False
  ERROR_Type PrintStates();
  void RestartState();  // ReInitialize current state
  void StopsState();    // Stops (without EndAction),  state (Doesen't change actual state, just stop it)
  void EndState();
  bool CheckState(byte _index);
  // Finalize an state (with EnsAction),  state (Doesen't change actual state, just finish it)

private:
  State *__CreateNewState();                                                                                                                                                                               // Handles the creation of a new state
  ERROR_Type __FillStateDate(State *&_auxState, byte _index, ERROR_Type (*_state_Ptr)(), ERROR_Type (*_stateStart_Ptr)(), ERROR_Type (*_stateEnd_Ptr)(), unsigned long _alarm0, unsigned long _timeOut0);  // Fill the data for a State
  State *__SearchState(State *_list, byte _indexState);                                                                                                                                                    // Search for a particular index State, return NULL if not found
  bool __DeleteState(State *&_list, byte _indexState);                                                                                                                                                     // Delete a particular index State, return False if not found
  ERROR_Type __CallBack(ERROR_Type (*_action_Ptr)());                                                                                                                                                      // Function to call another function (INPUT: a pointer to a function)
  bool __ChangeState();
  bool __CheckTimeOut();
  bool __CheckAlarm0();
  // Check if a new state it's available, if true then finish the actualState to initialize a new one
};

#endif