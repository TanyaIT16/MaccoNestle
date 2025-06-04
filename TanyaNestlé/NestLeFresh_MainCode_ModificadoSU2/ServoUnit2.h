/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 ServoUnit.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  25/09/22
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
Servo Unit: Control a PWM servo motor

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#pragma once
#ifndef _ServoUnitNew
#define _ServoUnit

#include "ID_Def.h"
#include "Assets.h"
#include "SWTimer.h"

#ifndef maxServo
#define maxServo 16
#endif

#ifndef defPin
#define defPin 0xFF
#endif

#define defMinAngle 0
#define defMaxAngle 180
#define degTurn 360

//#include "Servo.h"

class ServoUnit2
{ // Class Servo Unit, Handles a servo PWM type (Uses Servo library)
public:
    static byte nServoUnits; // Store the total number of ServoUnits
    __ID ID;                 // Owns ID

    ServoUnit2(); // Constructor

    void Attach(ID_Definition _ID, byte _pin, int _maxAngle, int _minAngle, int servoID); // Initialize the servo unit (ID, PwmPin , maxAngle , minAngle)
    void Attach(ID_Definition _ID, byte _pin);                                                           // Initialize the servo unit with default angles 0-180 (ID, PwmPin)
    void Attach(ID_Definition _ID, byte _pin, int _defAngle);                                                           // Initialize the servo unit with default angles 0-180 (ID, PwmPin)
    void Activate();
    void Deactivate();
    void SetPos(int _angle);
    void SetPos(int _angle,byte steps);                                                                              // Set the servo position to an angle (Inputs: Angle [º])
    void GoMax();                                                                                        // Go to max angle settedUp
    void GoMin();                                                                                        // Go to min angle settedUp
    void SetAngles(int _maxAngle, int _minAngle);                                                        // SetUp the Limits angles
    void SetAngles(int _maxAngle);
    void SetTimer(int _delay);                                                                       // SetUp the Limits angles Asumes min = 0;
    byte GetPin();                                                                                       // Return the phisical Pin of the servo PWM
    unsigned GetAngle();                                                                                 // Return the actual ReferenceAngle
    byte GetTypeCount();  
    void Run();                                                                               // Returns the Number of ServoUnits Registered

private:
    //Servo servo;                 // Servo type , handle the HWtimers for the PWM signal
    int _MapAngle(int _angle);   // Do a conversion betwen angles (INTERNAL USE ONLY)
    int _clampAngle(int _angle); // Clamp the angle to limits (INTERNAL USE ONLY)
    int idServo = -1;
    int servoSpeed = 10;
    bool _isMap = false;                         // Flag to use or not the map function
    byte pin = defPin;                           // Stores the phisicalPin of the PWM
    int angle = 0, maxAngle = 270, minAngle = 0, refAngle, timerStep=8; // Store the angles data
    byte steps= 0 , angleStep = 0;
    SWTimer_Redux_Millis timer0;
};

#endif