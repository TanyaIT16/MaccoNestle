#include "HardwareSerial.h"
#pragma once
#ifndef POSITION_HANDLER
#define POSITION_HANDLER

#include "Arduino.h"

#define TICKS_LAP 4096
#define STEPS_LAP 1600


class PositionBase {
public:
  // Conversion functions

protected:
  int laps;
  int ticks;

public:
  // Constructor
  PositionBase() {
  }

  int getLaps() const {
    return laps;
  }
  int getTicks() const {
    return ticks;
  }
};



///////// CLASS STEPS UNITS ///////////////

class StepsUnits : public PositionBase {
public:
static constexpr float ENCODER_TO_STEPS_FACTOR = (float)STEPS_LAP /  TICKS_LAP;
    // Constructors and other member functions...
    StepsUnits(int initialLaps = 0, long initialTicks = 0) {
        if (abs(initialTicks) >= STEPS_LAP) {
            this->laps = initialLaps + initialTicks / STEPS_LAP;   
            this->ticks = (initialTicks % STEPS_LAP);
        } else {
            this->laps = initialLaps;
            this->ticks = initialTicks;
        }
    }

    void operator=(StepsUnits other) {
        this->ticks = other.getTicks();
        this->laps = other.getLaps();
    }

    void operator=(long _ticks) {
        if (_ticks >= STEPS_LAP) {
            this->laps = _ticks / STEPS_LAP;   
            this->ticks = (_ticks % STEPS_LAP) - this->laps;
        } else {
            this->laps = 0;
            this->ticks = _ticks;
        }
    }

    StepsUnits operator-(long _ticks) const {
        return (*this - StepsUnits(0, _ticks));
    }

    StepsUnits operator+(long _ticks) const {
        return (*this + StepsUnits(0, _ticks));
    }

    // Overloaded addition operator
    StepsUnits operator+(const StepsUnits& other) const {
        int totalTicks = ticks + other.ticks;
        int totalLaps = laps + other.laps + (totalTicks / STEPS_LAP);
        totalTicks %= STEPS_LAP;
        return StepsUnits(totalLaps, totalTicks);
    }

    // Overloaded subtraction operator
    StepsUnits operator-(const StepsUnits& other) const {
        int totalTicks = ticks - other.ticks;
        int totalLaps = laps - other.laps;
        if (totalTicks < 0) {
            totalTicks += STEPS_LAP;
            totalLaps--;
        }
        return StepsUnits(totalLaps, totalTicks);
    }

    // Overloaded multiplication operator
    StepsUnits operator*(int factor) const {
        int totalTicks = ticks * factor;
        int totalLaps = laps * factor + (totalTicks / STEPS_LAP);
        totalTicks %= STEPS_LAP;
        return StepsUnits(totalLaps, totalTicks);
    }

    // Overloaded comparison operators
    bool operator==(const StepsUnits& other) const {
        return (laps == other.laps) && (ticks == other.ticks);
    }

    bool operator!=(const StepsUnits& other) const {
        return !(*this == other);
    }

    bool operator<(const StepsUnits& other) const {
        if (laps == other.laps) {
            return ticks < other.ticks;
        }
        return laps < other.laps;
    }

    bool operator<=(const StepsUnits& other) const {
        return (*this < other) || (*this == other);
    }

    bool operator>(const StepsUnits& other) const {
        return !(*this <= other);
    }

    bool operator>=(const StepsUnits& other) const {
        return !(*this < other);
    }

    // Overloaded comparison operators for comparison with a long integer value
    bool operator==(long value) const {
        return ((long)(laps * STEPS_LAP + ticks) == value);
    }

    bool operator!=(long value) const {
        return !(*this == value);
    }

    bool operator<(long value) const {
        return (laps * STEPS_LAP + ticks) < value;
    }

    bool operator<=(long value) const {
        return (laps * STEPS_LAP + ticks) <= value;
    }

    bool operator>(long value) const {
        return (laps * STEPS_LAP + ticks) > value;
    }

    bool operator>=(long value) const {
        return (laps * STEPS_LAP + ticks) >= value;
    }

    // Overloaded comparison operators for comparison with an integer value
    bool operator==(int value) const {
        return ((int)(laps * STEPS_LAP + ticks) == value);
    }

    bool operator!=(int value) const {
        return !(*this == value);
    }

    bool operator<(int value) const {
        return (laps * STEPS_LAP + ticks) < value;
    }

    bool operator<=(int value) const {
        return (laps * STEPS_LAP + ticks) <= value;
    }

    bool operator>(int value) const {
        return (laps * STEPS_LAP + ticks) > value;
    }

    bool operator>=(int value) const {
        return (laps * STEPS_LAP + ticks) >= value;
    }
};



///////// CLASS ENCODER UNITS ///////////////

class EncoderUnits : public PositionBase {
public:

  // Conversion factor from StepsUnits to EncoderUnits
  static constexpr float STEPS_TO_ENCODER_FACTOR =  (float)TICKS_LAP / STEPS_LAP;

  // Constructor
  EncoderUnits(int initialLaps = 0, long initialTicks = 0) {

    if (abs(initialTicks) >= TICKS_LAP) {
    this->laps = initialLaps + initialTicks/ TICKS_LAP;   
    this->ticks= (initialTicks % TICKS_LAP);
    } else {
      this->laps = initialLaps;
      this->ticks = initialTicks;
    }
  }

  // Overloaded addition operator
  EncoderUnits operator+(const EncoderUnits& other) const {
    int totalTicks = ticks + other.ticks;
    int totalLaps = laps + other.laps + (totalTicks / TICKS_LAP);
    totalTicks %= TICKS_LAP;
    EncoderUnits _pos(totalLaps, totalTicks);

    return _pos;
  }

  // Overloaded subtraction operator
  EncoderUnits operator-(const EncoderUnits& other) const {
    int totalTicks = ticks - other.ticks;
    int totalLaps = laps - other.laps;
    if (totalTicks < 0) {
      totalTicks += TICKS_LAP;
      totalLaps--;
    }
    return EncoderUnits(totalLaps, totalTicks);
  }

  // Overloaded multiplication operator
  EncoderUnits operator*(int factor) const {
    int totalTicks = ticks * factor;
    int totalLaps = laps * factor + (totalTicks / TICKS_LAP);
    totalTicks %= TICKS_LAP;
    return EncoderUnits(totalLaps, totalTicks);
  }

  void operator=(EncoderUnits other){
    this->ticks = other.getTicks();
    this->laps = other.getLaps();
  }

  void operator=(long _ticks){
    if(_ticks >= TICKS_LAP){
    this->laps = _ticks/ TICKS_LAP;   
    this->ticks= (_ticks % TICKS_LAP)-this->laps;
    }
    else{
      this->laps=0;
      this->ticks=_ticks;
    }
  }

  EncoderUnits operator-(long _ticks) const {
    return (*this - EncoderUnits(0, _ticks));
  }

  EncoderUnits operator+(long _ticks) const {
    return (*this + EncoderUnits(0, _ticks));
  }


  // Overloaded comparison operators

  bool operator==(const EncoderUnits& other) const {
    return (laps == other.laps) && (ticks == other.ticks);
  }

  bool operator!=(const EncoderUnits& other) const {
    return !(*this == other);
  }

  bool operator<(const EncoderUnits& other) const {
    if (laps == other.laps) {
      return ticks < other.ticks;
    }
    return laps < other.laps;
  }

  bool operator<=(const EncoderUnits& other) const {
    return (*this < other) || (*this == other);
  }

  bool operator>(const EncoderUnits& other) const {
    return !(*this <= other);
  }

  bool operator>=(const EncoderUnits& other) const {
    return !(*this < other);
  }

  // Overloaded comparison operators for comparison with a long integer value
  bool operator==(long value) const {
    return ((long)(laps * TICKS_LAP + ticks) == value);
  }

  bool operator!=(long value) const {
    return !(*this == value);
  }

  bool operator<(long value) const {
    return (laps * TICKS_LAP + ticks) < value;
  }

  bool operator<=(long value) const {
    return (laps * TICKS_LAP + ticks) <= value;
  }

  bool operator>(long value) const {
    return (laps * TICKS_LAP + ticks) > value;
  }

  bool operator>=(long value) const {
    return (laps * TICKS_LAP + ticks) >= value;
  }

  /// FOR INT_16

  bool operator==(int value) const {
    return ((int)(laps * TICKS_LAP + ticks) == value);
  }

  bool operator!=(int value) const {
    return !(*this == value);
  }

  bool operator<(int value) const {
    return (laps * TICKS_LAP + ticks) < value;
  }

  bool operator<=(int value) const {
    return (laps * TICKS_LAP + ticks) <= value;
  }

  bool operator>(int value) const {
    return (laps * TICKS_LAP + ticks) > value;
  }

  bool operator>=(int value) const {
    return (laps * TICKS_LAP + ticks) >= value;
  }
};



// External Functions


static StepsUnits EncoderUnitsToStepsUnits(const EncoderUnits& encoderUnits) {
  int totalLaps = encoderUnits.getLaps();
  int remainingTicks = (float)(encoderUnits.getTicks() * StepsUnits::ENCODER_TO_STEPS_FACTOR);
  return StepsUnits(totalLaps, remainingTicks);
}

static EncoderUnits StepsUnitsToEncoderUnits(const StepsUnits& stepsUnits) {
  int totalLaps = stepsUnits.getLaps();
  int remainingTicks = ((float)stepsUnits.getTicks() * EncoderUnits::STEPS_TO_ENCODER_FACTOR);
  return EncoderUnits(totalLaps, remainingTicks);
}

#endif