/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 Serial_Macco.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  24/07/24
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#pragma once

#ifdef SERIAL_MACCO
#define SERIAL_MACCO

#include "SerialMannager.h"

void SerialSend(char _order, char _auxchar, int data[MAX_DATA_ALLOWED] = {0}, int dataUsed = 0) {
  if (dataUsed <= MAX_DATA_ALLOWED && dataUsed >= 0) {
    String _serialData = "";
    _serialData = ORDER_CHARACTER;
    _serialData += _order;
    _serialData += _order;
    if (dataUsed != 0) {
      for (int i = 0; i <= dataUsed - 1; i++) {
        _serialData += data[i];
        _serialData += SEPARATION_CHARACTER;
      }
    }

    Serial.println(_serialData);
  }
}


#endif