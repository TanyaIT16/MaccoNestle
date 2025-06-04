/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 ID_Def.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  25/09/22
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
ID_Def:
defines a bunch of data for the M22-D01 Nestle FreshBeans machine

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#pragma once

#ifndef _ID_Def
#define _ID_Def
#include <Arduino.h>

//////////////////////////////////////////////////// Define The ID's for the diferent modules //////////////////////////////////////////////////
enum SliderIndex{
  Slider11 = 0,
  Slider12 = 1,
  Slider21 = 2,
  Slider22 = 3,
  SliderSP = 4,
};

enum ExtruderIndex{
  extruder0 = 0,
  extruder1 = 1,
  extruder2 = 2,
  extruder3 = 3,
  extruder4 = 4,
  extruder5 = 5,
};

enum ID_Definition {
  ID_NODEF = 0x00,             // Not Defined Unit
  ID_Master = 0x01,            // Arduino Master Controller
  ID_CommunicationDev = 0x02,  // Comunication Device ESP32
  ID_Extruder = 0x10,          // Extruder
  ID_E0 = 0x11,                // Extruder 0
  ID_E1 = 0x12,                // Extruder 1
  ID_E2 = 0x13,                // Extruder 2
  ID_E3 = 0x14,                // Extruder 3
  ID_E4 = 0x15,                // Extruder 4
  ID_E5 = 0x16,                // Extruder 5
  ID_HBridgeE0 = 0x17,         // Piston Extruder 0
  ID_HBridgeE1 = 0x18,         // Piston Extruder 1
  ID_HBridgeE2 = 0x19,         // Piston Extruder 2
  ID_HBridgeE3 = 0x1A,         // Piston Extruder 3
  ID_HBridgeE4 = 0x1B,         // Piston Extruder 4
  ID_HBridgeE5 = 0x1C,         // Piston Extruder 5
  ID_Selector = 0x20,          // Selector
  ID_HBMixerSelector = 0x21,   // Selector Mixer
  ID_ServoSelector0 = 0x22,    // Servo Bypass Selector
  ID_ServoSelector1 = 0x23,    // Servo Grinder Selector
  ID_Grinder = 0x30,           // Grinder
  ID_ServoGrindSize = 0x31,    // ServoSizeGrinder
  ID_HBridgeGrind = 0x32,      // ServoPowerGrinder
  ID_HBridgeVibrator = 0x33,   // ServoVibratorGrinder
  ID_Arm1 = 0x40,              // Arm1
  ID_MotorSlider11 = 0x41,     // Arm1 Slider 1
  ID_MotorSlider12 = 0x42,     // Arm1 Slider 2
  ID_ServoEndEff1 = 0x43,      // Arm1 Servo1
  ID_VaacumEndEff1 = 0x44,     // Arm1 Vaacum 1
  ID_Arm2 = 0x50,              // Arm2
  ID_MotorSlider21 = 0x6A,     // Arm2 Slider 1 //original 0x51
  ID_MotorSlider22 = 0x52,     // Arm2 Slider 2
  ID_ServoEndEff2 = 0x53,      // Arm2 Servo1
  ID_VaacumEndEff21 = 0x54,    // Arm2 Vaacum 1
  ID_VaacumEndEff22 = 0x55,    // Arm2 Vaacum 2
  ID_Fold = 0x60,              // FoldBag System
  ID_ServoFold1 = 0x61,        // Servo1 FoldBag
  ID_ServoFold2 = 0x62,        // Servo2 FoldBag
  ID_HB_FoldExtensor = 0x63,
  ID_Printer = 0x68,                 // Printer Unit
  ID_Wheight = 0x69,                 // Weigth Machine Unit
  ID_MotorServicePlatform = 0x7A,    // Slider Service Platform //original 0x6A
  ID_HBridgeServicePlatform = 0x6B,  // HB Door Service
  ID_Compressor = 0x70,              // Compresor Unit
  ID_ServoCompressorPurge = 0x71,    // Compressor Purge Servo 1
  ID_HBridgeCompressorTurn = 0x72,   // Compressor Power HB1
  ID_Ligths = 0x80,                  // Lights System
  ID_LedStrip1 = 0x81                // Led Strip 1
};

////////////////////////////// Define the type of module are an ID//////////////////////////////////////////////////
enum ObjectTYPE {
  T_NODEF = 0x00,            // Not defined Type
  T_AMEGA = 0x01,            // MEGA2556
  T_ESP_Eth = 0x02,          // ESP32
  T_HBridge = 0x03,          // H-Bridge unit
  T_Vaacum = 0x04,           // Vaacum Unit (H-Bridge with vaacum sensor)
  T_ServoUnit = 0x05,        // Servo Unit (PWM)
  T_SmartMotor = 0x06,       // Smartmotor Controller (Master CAN-Bus)
  T_SmartStepper = 0x07,     // Smartmotor Reciever (Slave CAN-Bus)
  T_DigSensor = 0x08,        // Digital input Sensor
  T_Extrusor = 0x09,         // Extrusor Unit
  T_Selector = 0x0A,         // Selector Unit
  T_Grinder = 0x0B,          // Grinder Unit
  T_Arm = 0x0C,              // ArmUnit
  T_Fold = 0x0D,             // FoldSystemUnit
  T_Weight = 0x0E,           // WeightMachine Unit
  T_Printer = 0x0F,          // Printer Unit
  T_ServicePlatform = 0x10,  // ServicePlatform Unit
  T_Compressor = 0x11        // Compressor Unit
};

//////////////////////////////////////////////////// Define the ERROR codes//////////////////////////////////////////////////
enum ERROR_Type {
  NO_ERROR = 0x00,  // No error Message
  NEWORDER_INFO,
  RUNNING_INFO,   // Running Status
  HOLDED_INFO,    // Holded Status
  FINISHED_INFO,  // Finished Status
  CANBEANS_OK_INFO,
  CANBEANS_HALF_INFO,

  MASK_WARNING = 32,                     // Main Warning Message
  NOT_DEFINED_WARNING,                   // Warning Not defined option
  DATA_NOT_VALID_WARNING,                // Warning Not Valid Data
  ALREADY_REGISTERED_ID_WARNING = 0x23,  // Warning ID already registered
  CANSBEANS_ALARM_WARNING = 0x24,

  MASK_ERROR = 0x40,             // Main Error Message
  NOT_CONECTED_ERROR = 0x41,     // Error Not conected unit
  CANSBEANS_EMPTY_ERROR = 0x42,  //

  MASK_CRITICAL = 0x80,  // Main Critical Message

  CANSBEANS_NOT_ALLOWED_COUNT_CRITICAL = 0x82
};

//////////////////////////////////////////////////// Define The Parameters to modify stepperSmart //////////////////////////////////////////////////
/*enum SmartStepperPARAMETERS {  // Enum to classify the Stepper Parameters
  P_No_Defined_Parameter = 0x00,
  P_Speed,
  P_Acceleration,
  P_RefPos,
  P_RefSteps,
  P_Min_Speed,
  P_Max_Speed,
  P_Mode,
  P_Min_Acceleration,
  P_Max_Acceleration,
  P_Jam,
  P_Limit_Steps,
  P_Limit_Colisions,
  P_Speed_Error,
  P_Position_Error,
  P_Switch_Direction,
  P_Switch_Enable,
  P_StepBack_Collision,
  P_StepsPerRevolution,
  P_SyncSteps,

  P_MAXALLOWEDPARAMETER  // Do not write over this parameter
};
*/

enum SmartStepperPARAMETERS {  // Enum to classify the Stepper Parameters
  P_No_Defined_Parameter = 0x00,
  P_Speed,
  P_Acceleration,
  P_RefSteps,
  P_RefPos,
  P_Min_Speed,
  P_Max_Speed,
  P_Limit_Colisions,
  P_Position_Error,
  P_InvertLogic,
  P_StepReverse,

  P_MAXALLOWEDPARAMETER  // Do not write over this parameter
};

enum SmartStepperPARAMETERS_EEPROMAddr {  // Enum to classify the Stepper Parameters
  EEPROM_No_Defined_Parameter = -1,
  EEPROM_Speed = 0x64,
  EEPROM_Acceleration,
  EEPROM_RefPos,
  EEPROM_RefSteps,
  EEPROM_Min_Speed,
  EEPROM_Max_Speed,
  EEPROM_Mode,
  EEPROM_Min_Acceleration,
  EEPROM_Max_Acceleration,
  EEPROM_Jam,
  EEPROM_Limit_Steps,
  EEPROM_Limit_Colisions,
  EEPROM_Speed_Error,
  EEPROM_Position_Error,
  EEPROM_Switch_Direction,
  EEPROM_Switch_Enable,
  EEPROM_StepBack_Collision,
  EEPROM_StepsPerRevolution,
  EEPROM_SyncSteps,

  EEPROM_MAXALLOWEDPARAMETER = 0x200  // Do not write over this parameter
};


//////////////////////////////////////////////////// Define The Pin's for the diferent modules //////////////////////////////////////////////////
enum PinList {
  Pin_SelfKill = 13,

  Pin_SPI_MISO = 50,
  Pin_SPI_MOSI = 51,
  Pin_SPI_CS = 53,
  Pin_SPI_SCK = 52,

  Pin_HBE00 = 255,
  Pin_HBE01 = 255,
  Pin_HBE10 = 255,
  Pin_HBE11 = 255,
  Pin_HBE20 = 255,
  Pin_HBE21 = 255,
  Pin_HBE30 = 255,
  Pin_HBE31 = 255,
  Pin_HBE40 = 255,
  Pin_HBE41 = 255,
  Pin_HBE50 = 255,
  Pin_HBE51 = 255,

  Pin_SelByPassPWM = 3,   // 1
  Pin_SelGrinderPWM = 4,  // 2
  Pin_SelBeansDoor = 2,

  Pin_PowGrinder = 45,
  Pin_SizeGrinderPWM = 9,  // 3
  Pin_PowVibrator = 255,

  Pin_Arm0_TurnPWM = 255,  // 4 //**************Modificado
  Pin_Arm0_Vac11IN = 48,
  Pin_Arm0_HBVac11 = 23,

  Pin_Arm1_TurnPWM = 255,  // 5 //**************Modificado
  Pin_Arm1_Vac21IN = 46,
  Pin_Arm1_HBVac21 = 32,
  Pin_Arm1_Vac22IN = 44,
  Pin_Arm1_HBVac22 = 38,

  Pin_Fold0_TurnPWM = 255,   // 6 //*************Modificado
  Pin_Fold1_TurnPWM = 255, // old 9  // 7
  Pin_Fold_HB0 = 40,
  Pin_Fold_HB1 = 41,
  Pin_FoldEx_HB0 = 35,
  Pin_FoldEx_HB1 = 255,

  Pin_SP_HBDoor = 255,
  Pin_SP_HBPush1 = 30,
  Pin_SP_HBPush2 = 255,
  

  Pin_Compressor_PowHB = 255,
  Pin_Compressorc_PurgePWM = 5,  // 8

  Pin_MainDoor_INPUT = 20,
  Pin_ServiceDoor_INPUT = 21,
  Pin_BagSensor_INPUT = 22,

  Pin_NoDef = 0xFF
};

#endif
