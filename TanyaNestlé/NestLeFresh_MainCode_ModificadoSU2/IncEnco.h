/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 IncEnco.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  25/09/22
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
IncEnco: Control a incremental encoder (Quatrature A,B) with number of turn signal N

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

 
#pragma once
#ifndef INCENCO
#define INCENCO

#include "Wire.h"
#include <TimerOne.h>
#include <EEPROM.h>

#define A_PIN 2 // Definition of pin A of the encoder
#define B_PIN 3 // Definition of Pin B of the encoder
#define N_PIN 4 // Definition of Pin N of the encoder

#define ENCODERSTEPS 2000 //Definition of steps per revolution of the encoder

 

class IncEnco { // Class Incremental Encoder object
public:

static volatile bool dirSwitch ; // STATIC direction behaivorFlag of meausurment
static volatile int pos;// STATIC position of the encoder [encoder units]
static volatile int turns; // STATIC turns done by the encoder
static volatile bool dir; // STATIC main direction
static volatile bool sigA, sigB, sigN, lastAB; // Signal level of encoder pins A,B,N
static volatile bool IsEndSwitch; // Flag to use and endSwitch or not
static volatile byte endSwitchPin; //Phisical pin of the ensSwitch

private:

unsigned StepsPerRevolution = ENCODERSTEPS; // Steps per revolution of the encoder

public:

static void __EventA(); // STATIC ISR() handle the event change on pin A
static void __EventB(); // STATIC ISR() handle the event change on pin B

IncEnco(); // Constructor

void Attach(byte _endSwitchPin); // Method to Initialize an incremental encoder
void Attach(); // Method to Initialize an incremental encoder
void Dettach(); // Remove the encoder object from memory
int GetCount(); // Return the counts (position) of the encoder
int GetTurns(); // Return the turns done by the encoder (Based on N signals)
bool GetDir(); // Return the current direction of meausurment
bool Check_EndSwitch(); // Check if the endSwitch pin has triggered
unsigned GetStepsPerRevolution(); // Return the steps per revolution of the encoder
void SetdirSwitch(); // Set the dirSwitch Parameter to default
void SetdirSwitch(bool _dirSwitch); // Set the dirSwitch Parameter to a value (INPUT: dir_Flag)
void SetStepPerRevolution(unsigned _stepPerRevolution);  // Set the steps per revolution of the encoder (INPUT: stepsPerRevolution)
void Reset(); //Reset the encoder to Defaults

};
#endif