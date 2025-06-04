/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 CounterACK.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  29/03/24
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
StepperDriver: General Pourpose Stepper Driver Controller

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#pragma once
#ifndef COUNTER_ACK
#define COUNTER_ACK

#include "Arduino.h"

#define NUMBER_OF_SLIDERS 5
#define NUMBER_OF_EXTRUDERS 6

enum ACK_State{
  ACK_noDef,
  ACK_standby,
  ACK_running,
  ACK_unknown,
  ACK_error
};

struct ACK_data{
ACK_State state =ACK_noDef;
byte ID = 0x00;
};


class CounterACK {


public:

static ACK_data slider_State[NUMBER_OF_SLIDERS];
static ACK_data extruder_State[NUMBER_OF_EXTRUDERS];

CounterACK();

void SetNewSliderACK(ACK_State _state,byte _ID, byte _index);
void SetNewExtruderACK(ACK_State _state,byte _ID, byte _index);
ACK_State GetExtruderState(byte _index);
ACK_State GetSliderState(byte _index);

void RemoveSliderState(byte _index);
void RemoveExtrusorState(byte _index);


private:

};

#endif