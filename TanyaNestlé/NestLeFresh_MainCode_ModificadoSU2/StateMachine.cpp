#include "HardwareSerial.h"

/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 StateMachine.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#ifndef _StateMachine
//#include "HardwareSerial.h" // ???
#include "StateMachine.h"
/// CPP FILE

// Constructor Initialize State Machine
StateMachine::StateMachine(byte _timeOutState = (MAX_STATES)) {
  ID.AttachID(ID_Master, T_AMEGA);
  //SetNextState(0);               // Initialize the machine to 0 State (Or ERROR if not exist)
  this->timer0.reset();     // Initialize the timer0 (Not user accecible)
  this->timerUser.reset();  // Initialize time user (This timer it's fully handled by the user, anything of this timer modify the state machine)
  this->timeOutState = _timeOutState;
}

// Destructor
StateMachine::~StateMachine() {
}

ERROR_Type StateMachine::AttachMainState(ERROR_Type (*_state_Ptr)()){  // Add a main state to the list
  this->mainState_Ptr = _state_Ptr;

}
// Add a new state to the list
ERROR_Type StateMachine::AttachNewState(byte _index, ERROR_Type (*_state_Ptr)(), ERROR_Type (*_stateStart_Ptr)(), ERROR_Type (*_stateEnd_Ptr)(), byte _safeState, byte _nextState, unsigned _timeOut0 = 0) {
  //this->numState ++ ;

  if (_index < MAX_STATES) {
    this->listStates[_index].stateStart_Ptr = _stateStart_Ptr;
    this->listStates[_index].state_Ptr = _state_Ptr;
    this->listStates[_index].stateEnd_Ptr = _stateEnd_Ptr;
    this->listStates[_index].timeOut = _timeOut0;
  }
}

byte StateMachine::Counter_Fail(byte _failCounter,byte _retryCounter){
this->RetriesCount ++;
      if (this->RetriesCount <= MAX_RETRIES) { // Retry
        counter = _retryCounter;
      } else { // FAIL ORDER
        counter = _failCounter;
      }

}

byte StateMachine::Counter_Reset(byte _init = 0){
this->RetriesCount=0;
this->counter = _init;
}

byte StateMachine::GetState() {
  return this->actualState;
}

byte StateMachine::GetSafeState() {
}

byte StateMachine::GetNextState() {
}

// Run the actual State (Also OneShot Start / End methods)
ERROR_Type StateMachine::Run() {
  //Check if actual state it's valid
  __ChangeState();
  if(this->mainState_Ptr != NULL){
    __CallBack(this->mainState_Ptr); // Execute Always Function
  }
  
  if (CheckState(this->actualState)) {

    switch (this->status) {
      case SS_StandBy:
        break;

      case SS_Initializing:
        __CallBack(this->listStates[this->actualState].stateStart_Ptr);
        this->timer0.reset();
        this->status = SS_Running;
        break;

      case SS_Running:
        //Serial.println("Running...");
        switch (__CallBack(this->listStates[this->actualState].state_Ptr)) {
          case NO_ERROR:
            break;
          case RUNNING_INFO:
            break;
          case FINISHED_INFO:
            break;
          case HOLDED_INFO:
            break;
          default:
            break;
        }

        if (this->timer0.Check(this->listStates[this->actualState].timeOut) && this->listStates[this->actualState].timeOut != 0)  // If a TimeOUT ocurs during Runtime -> Halt the machine -> Go to State 0 (Or initial)
        {
          Serial.println("TimeOUT");
          if(this->timeOutState < MAX_STATES){
          this->SetNextState(this->timeOutState);
          this->timer0.reset();
          }
        }

        break;

      case SS_Finishing:
        this->status = SS_Finished;
        __CallBack(this->listStates[this->actualState].stateEnd_Ptr);
        break;

      case SS_Finished:
        Serial.print("Switch to state: ");
        Serial.println(this->nextState);
        if (this->actualState == this->nextState)
          this->status = SS_StandBy;
        break;

      case SS_NotInitialize:
        // ERROR
        break;

      default:
        // ERROR
        break;
    }


  }

  else {
    //FAIL STATE MACHINE
    Serial.println("ERROR, State Corrupt");
  }
  /*
  if (this->actualState != NULL) {

    switch (this->status) {
      case SS_StandBy:
        break;

      case SS_Initializing:
        __CallBack(this->actualState->stateStart_Ptr);
        if (this->actualState->alarm0 != 0 || this->actualState->timeOut != 0)  // If it's a time to set -> Set It
        {
          this->timer0.ResetTimer();
        }
        this->status = SS_Running;
        break;

      case SS_Running:
      //Serial.println("Running...");
        switch (__CallBack(this->actualState->state_Ptr)) {
          case NO_ERROR:
            break;
          case RUNNING_INFO:
            break;
          case FINISHED_INFO:
            if (this->nextState->nextListState != NULL)
              this->nextState = this->nextState->nextListState;
            else
              this->nextState = this->listStates;
            break;
          case HOLDED_INFO:
            break;
          default:
            this->nextState = this->listStates;
            break;
        }

        if (!__CheckTimeOut())  // If a TimeOUT ocurs during Runtime -> Halt the machine -> Go to State 0 (Or initial)
        {
          //Serial.println("TimeOUT");
          __CheckAlarm0();  // If alarm time has pased Out (AND NOT TimeOut Ocurs) -> Change to next available state
        }

        break;

      case SS_Finishing:
        this->status = SS_Finished;
        __CallBack(this->actualState->stateEnd_Ptr);

        break;

      case SS_Finished:
        Serial.print("Switch to state: ");
        Serial.println(this->nextState->index);
        if (this->actualState == this->nextState)
          this->status = SS_StandBy;
        break;

      case SS_NotInitialize:
        // ERROR
        break;

      default:
        // ERROR
        break;
    }
  } else
    this->actualState = this->nextState;
  __ChangeState();
*/
}

// Change to a new state ->Search if the state exist if not Return Error
ERROR_Type StateMachine::SetNextState(byte _index) {

  if (_index < MAX_STATES) {
    this->nextState = _index;

    if (this->actualState == this->nextState) {
      if (this->status == SS_StandBy)
        this->status = SS_Initializing;
      if (this->status == SS_Running) {
        this->status = SS_Finishing;
      }
    }
  }
  /*
  State *_auxState = this->listStates;
  _auxState = __SearchState(this->listStates, _index);
  if (_auxState != NULL) {
    Serial.println("ChangeState");
    this->nextState = _auxState;
    if (this->actualState == this->nextState) {
      if (this->status == SS_StandBy)
        this->status = SS_Initializing;
      if (this->status == SS_Running) {
        // this->status = SS_Finishing;
      }
    }
  } else
    Serial.println("Not Found State");
*/
}

// ReInitialize current state
void StateMachine::RestartState() {
  this->status = SS_Initializing;
}

ERROR_Type StateMachine::AvanceState() {
  SetNextState(this->actualState + 1);
}

// Stops (without EndAction),  state (Doesen't change actual state, just stop it)
void StateMachine::StopsState() {
  this->status = SS_StandBy;
}

// Finalize an state (with EnsAction),  state (Doesen't change actual state, just finish it)
void StateMachine::EndState() {
  this->status = SS_Finishing;
}

bool StateMachine::CheckState(byte _index) {

  if (_index < MAX_STATES) {
    if (this->listStates[_index].stateStart_Ptr != NULL && this->listStates[_index].state_Ptr != NULL && this->listStates[_index].stateEnd_Ptr != NULL) {
      return true;
    }
  }
  return false;
}

// Delete If exist, a state, else Return False
ERROR_Type StateMachine::DeleteState(byte _index) {
  if (_index < MAX_STATES) {
    this->listStates[_index].stateStart_Ptr = NULL;
    this->listStates[_index].state_Ptr = NULL;
    this->listStates[_index].stateEnd_Ptr = NULL;
  }
}


ERROR_Type StateMachine::PrintStates() {
  Serial.print("List of Available States:");
  for (int i = 0; i < MAX_STATES; i++) {
    if (CheckState(i)) {
      Serial.print(i);
      Serial.print(", ");
    }
  }
  Serial.println();
}


// Function to call another function (INPUT: a pointer to a function)
ERROR_Type StateMachine::__CallBack(ERROR_Type (*_action_Ptr)()) {
  return _action_Ptr();
}

// Check if a new state it's available, if true then finish the actualState to initialize a new one
bool StateMachine::__ChangeState() {

  if (this->actualState != this->nextState && this->nextState < MAX_STATES) {
    switch (this->status) {
      case SS_StandBy:
        this->actualState = this->nextState;
        this->status = SS_Initializing;
        return true;
        break;
      case SS_Initializing:
        break;
      case SS_Running:
        this->status = SS_Finishing;
        break;

      case SS_Finishing:

        break;
      case SS_Finished:
        this->actualState = this->nextState;
        this->status = SS_Initializing;
        return true;
        break;
      case SS_NotInitialize:
        return false;
        break;

      default:
        return false;
        break;
    }
  }
  return false;
}


#endif