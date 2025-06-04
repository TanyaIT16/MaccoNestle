/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 CounterACK.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#include "CounterACK.h"
ACK_data CounterACK::slider_State[NUMBER_OF_SLIDERS];
ACK_data CounterACK::extruder_State[NUMBER_OF_EXTRUDERS];

CounterACK::CounterACK() {}

void CounterACK::SetNewSliderACK(ACK_State _state, byte _ID , byte _index) {
  ACK_data newData;
  if(_index != 8){
  newData.state = _state;
  newData.ID = _ID;
  CounterACK::slider_State[_index]= newData;
  }
  else {
  newData.state = ACK_standby;
  newData.ID = _ID;
  CounterACK::slider_State[_index]= newData;
  }
}
void CounterACK::SetNewExtruderACK(ACK_State _state, byte _ID,byte _index) {
  ACK_data newData;
  newData.state = _state;
  newData.ID = _ID;
  CounterACK::extruder_State[_index]= newData;
}
ACK_State CounterACK::GetExtruderState(byte _index) {
  
  return CounterACK::extruder_State[_index].state;
}
ACK_State CounterACK::GetSliderState(byte _index) {
  return CounterACK::slider_State[_index].state;
}


void CounterACK::RemoveSliderState(byte _index) {
  ACK_data newData;
  CounterACK::slider_State[_index] = newData;
}
void CounterACK::RemoveExtrusorState(byte _index) {
  ACK_data newData;
  CounterACK::extruder_State[_index] = newData;
}