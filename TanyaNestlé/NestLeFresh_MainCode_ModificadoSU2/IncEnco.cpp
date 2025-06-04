/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 IncEnco.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#include "Arduino.h"
#ifndef INCENCO
#include "IncEnco.h"

volatile bool IncEnco::dirSwitch;                                            // STATIC direction behaivorFlag of meausurment
volatile int IncEnco::pos;                                                   // STATIC position of the encoder [encoder units]
volatile int IncEnco::turns;                                                 // STATIC turns done by the encoder
volatile bool IncEnco::dir;                                                  // STATIC main direction
volatile bool IncEnco::sigA, IncEnco::sigB, IncEnco::sigN, IncEnco::lastAB;  // Signal level of encoder pins A,B,N
volatile bool IncEnco::IsEndSwitch;                                          // Flag to use and endSwitch or not
//volatile byte IncEnco::endSwitchPin;                        //Phisical pin of the ensSwitch


// Class Incremental Encoder object
IncEnco::IncEnco() {
  IncEnco::dirSwitch = true;
}

// Method to Initialize an incremental encoder
void IncEnco::Attach(byte _endSwitchPin) {
  pinMode(A_PIN, INPUT);
  pinMode(B_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(A_PIN), IncEnco::__EventA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(B_PIN), IncEnco::__EventB, CHANGE);
  pinMode(N_PIN, INPUT);
  //pinMode(_endSwitchPin, INPUT);
  //IncEnco::endSwitchPin = _endSwitchPin;
  this->sigA = digitalRead(A_PIN);
  this->sigB = digitalRead(B_PIN);
  this->sigN = digitalRead(N_PIN);
}

void IncEnco::Attach() {
  pinMode(A_PIN, INPUT);
  pinMode(B_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(A_PIN), IncEnco::__EventA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(B_PIN), IncEnco::__EventB, CHANGE);
  pinMode(N_PIN, INPUT);
  this->sigA = digitalRead(A_PIN);
  this->sigB = digitalRead(B_PIN);
  this->sigN = digitalRead(N_PIN);
}

// Remove the encoder object from memory
void IncEnco::Dettach() {
  detachInterrupt(digitalPinToInterrupt(A_PIN));
  detachInterrupt(digitalPinToInterrupt(B_PIN));
}

// Return the counts (position) of the encoder
int IncEnco::GetCount() {
  return IncEnco::pos;
}

// Return the turns done by the encoder (Based on N signals)
int IncEnco::GetTurns() {
  return this->turns;
}

// Return the current direction of meausurment
bool IncEnco::GetDir() {
  return this->dir;
}

// Return the steps per revolution of the encoder
unsigned IncEnco::GetStepsPerRevolution() {
  return this->StepsPerRevolution;
}

// Set the steps per revolution of the encoder (INPUT: stepsPerRevolution)
void IncEnco::SetStepPerRevolution(unsigned _stepPerRevolution) {
  this->StepsPerRevolution = _stepPerRevolution;
}

// Set the dirSwitch Parameter to a value (INPUT: dir_Flag)
void IncEnco::SetdirSwitch(bool _dirSwitch) {
  this->dirSwitch = _dirSwitch;
}

// Set the dirSwitch Parameter to default
void IncEnco::SetdirSwitch() {
  this->dirSwitch = !this->dirSwitch;
}

//Reset the encoder to Defaults
void IncEnco::Reset() {
  IncEnco::pos = 0;
  IncEnco::turns = 0;
}

// Check if the endSwitch pin has triggered
bool IncEnco::Check_EndSwitch() {
  //return IncEnco::IsEndSwitch;
return false;
}

// STATIC ISR() handle the event change on pin A
void IncEnco::__EventA() {
  //Serial.println("a");
  // if(digitalRead(IncEnco::endSwitchPin)){
  //   IncEnco::IsEndSwitch=false;
  // }
  //  else{
  //    IncEnco::IsEndSwitch=true;
  //  }
  IncEnco::sigA = digitalRead(A_PIN);
  if (lastAB) {
    lastAB = false;
    if (IncEnco::sigA) {
      if (!IncEnco::sigB) {
        IncEnco::pos++;
        IncEnco::dir = IncEnco::dirSwitch;
      }  // Determines the direction of the encoder
      else {
        IncEnco::pos--;
        IncEnco::dir = !IncEnco::dirSwitch;
      }
    } else {
      if (IncEnco::sigB) {
        IncEnco::pos++;
        IncEnco::dir = IncEnco::dirSwitch;
      }  // Determines the direction of the encoder
      else {
        IncEnco::pos--;
        IncEnco::dir = !IncEnco::dirSwitch;
      }
    }
  }
}

// STATIC ISR() handle the event change on pin B
void IncEnco::__EventB() {
  //Serial.println("b");
  if (!lastAB) {
    lastAB = true;
    //if (digitalRead(IncEnco::endSwitchPin)) {
    //   IncEnco::IsEndSwitch = false;
    // }
    IncEnco::sigB = digitalRead(B_PIN);
    IncEnco::sigN = digitalRead(N_PIN);
    if (IncEnco::sigB) {
      if (IncEnco::sigA) {
        IncEnco::pos++;
        IncEnco::dir = IncEnco::dirSwitch;
      }  // Determines the direction of the encoder
      else {
        IncEnco::pos--;
        IncEnco::dir = !IncEnco::dirSwitch;
      }
    } else {
      if (!IncEnco::sigA) {
        IncEnco::pos++;
        IncEnco::dir = IncEnco::dirSwitch;
      }  // Determines the direction of the encoder
      else {
        IncEnco::pos--;
        IncEnco::dir = !IncEnco::dirSwitch;
      }
    }

    if (IncEnco::sigN) {
      if (IncEnco::dir == IncEnco::dirSwitch)
        IncEnco::turns++;
      else
        IncEnco::turns--;
    }
  }
}


#endif