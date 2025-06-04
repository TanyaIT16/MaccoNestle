

#pragma once
#ifndef TRANSLATE_PARAMETER
#define TRANSLATE_PARAMETER

#include "mcp2515.h"


class TranslateParameter {

public:
  TranslateParameter();

  // Method to extract an UNSIGNED value from a can_frame
  unsigned Msg2UNSIGNED(can_frame &_canMsg) {
    byte _data[sizeof(unsigned)];
    for (int i = 0; i < sizeof(unsigned); i++) {
      _data[i] = _canMsg.data[3 + i];
    }

    unsigned *_dataPtr = (unsigned *)&_data[0];
    return (unsigned)*_dataPtr;
  }

  // Method to extract a LONG value from a can_frame
  long Msg2LONG(can_frame &_canMsg) {
    byte _data[sizeof(long)];
    for (int i = 0; i < sizeof(long); i++) {
      //_data[(sizeof(long))-(i+1)] = _canMsg.data[3 + i];
      _data[i] = _canMsg.data[3 + i];
    }

    long *_dataPtr = (long *)&_data[0];
    return (long)*_dataPtr;
  }

  // Method to extract a BYTE value from a can_frame
  byte Msg2BYTE(can_frame &_canMsg) {
    return _canMsg.data[3];
  }

  // Method to extract a BOOL value from a can_frame
  bool Msg2BOOL(can_frame &_canMsg) {
    return _canMsg.data[3] != 0;
  }

  // Method to extract a FLOAT value from a can_frame
  float Msg2FLOAT(can_frame &_canMsg) {
    byte _data[sizeof(float)];
    for (int i = 0; i < sizeof(float); i++) {
      _data[i] = _canMsg.data[3 + i];
    }

    float *_dataPtr = (float *)&_data[0];
    return (float)*_dataPtr;
  }

  EncoderUnits Msg2EncoderUnits(can_frame &_canMsg) {
    int _laps = 0;
    int _ticks = 0;
    int *_dataPtr;
    byte _data[sizeof(int)];


    _data[0] = _canMsg.data[3];
    _data[1] = _canMsg.data[4];
    _dataPtr = (int *)&_data[0];
    _laps = (int)*_dataPtr;

    _data[0] = _canMsg.data[5];
    _data[1] = _canMsg.data[6];
    _dataPtr = (int *)&_data[0];
    _ticks = (int)*_dataPtr;

    return EncoderUnits(_laps, _ticks);
  }

  StepsUnits Msg2StepsUnits(can_frame &_canMsg) {
    int _laps = 0;
    int _ticks = 0;
    int *_dataPtr;
    byte _data[sizeof(int)];


    _data[0] = _canMsg.data[3];
    _data[1] = _canMsg.data[4];
    _dataPtr = (int *)&_data[0];
    _laps = (int)*_dataPtr;

    _data[0] = _canMsg.data[5];
    _data[1] = _canMsg.data[6];
    _dataPtr = (int *)&_data[0];
    _ticks = (int)*_dataPtr;

    return StepsUnits(_laps, _ticks);
  }
};

#endif