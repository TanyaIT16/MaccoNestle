/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 SmartMotor.h
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
# ifndef _SmartMotor
# define _SmartMotor

#include "ID_Def.h"
#include "Assets.h"
#include "CANmannager.h"
#include "Position.h"

#ifndef STEPPERSMART
#include "StepSmart.h"
#endif

#include "CounterACK.h"

class SmartMotor { // Class SmartMotor to Handle the Transmiter part of a SmartStepper-CANBUS
public:
static byte nSmartMotors; //Store the total number of nSmartMotors = nSmartSteppers
static byte IDMaster;  
CounterACK counterACK;
__ID ID; //Own ID
SmartMotor(); // Constructor

void Attach(ID_Definition _ID, CANmannager *_can) ; // Initialize a smartMotor (ID)
void SetMode (byte _mode); // Set the SmartMotor mode
void SetParam (SmartStepperPARAMETERS _param, int32_t _value); // SetParameter of SmartMotor
void SetSteps (StepsUnits _value); // Set a value
void SetPosition(long _distance);
void GetHWStatus ();


protected:
byte motorNumber = 0;
CANmannager* can;



};

class ExtrusorMannager : public SmartMotor {
private:
float gramsPerRevolution = 16;
StepsUnits _TransformGramsToSteps(unsigned _grams);

public:
ExtrusorMannager(float _gramsPerRev = 10.0);
void ExtrudeGrams(unsigned _grams);
void SetGramsPerRevolution(const float _gramsPerRev);
float GetGramsPerRevolution();

};

class SliderMannager : public SmartMotor {
private:
float milimeterPerRevolution;

public:
SliderMannager();
void GoToPosition(float milimeters);
void GoHome();


};


#endif
