
#include "StepSmart.h"
#include "ID_Def.h"
#include "Assets.h"
#include "CANmannager.h"
#include <SPI.h>
#include "StateMachine.h"
#include "FoldUnit.h"
#include "HBridge.h"
#include "VacuumUnit.h"
#include "ArmUnit.h"

#include "ServicePlatformUnit.h"
#include "CompressorUnit.h"
#include "SerialMannager.h"
//#include "Serial_Macco.h"
#include "SelectorUnit.h"
#include "S_Timer_Redux.h"

#include "NestleFresh.h"

#define GRAMS_PER_REVOLUTION 13.0
#define SERIAL_PERIOD 5000
#define GRAMS_PURGE_CYCLE 25
#define MAX_RETRIES 8


///////////////////////////Example Order/////////////////////////////
/*
f2
#rs0/0/100/0/0/0/36/
#rs100/0/0/0/0/0/64/
#rs100/0/100/100/0/100/36/
44
*/

enum Periods_Seconds {
  Period_default = 1,
  Period_servo = 1,
  Period_Mixing = 5,
  Period_ByPassing = 10,
  Period_Grinding = 10,
  Period_Serving = 5,
  Period_OpenGrip = 5
};

enum Periods_TimeOuts_mSeconds {
  Timeout_Disabled = 0,
  Timeout_Default = 0,
  Timeout_Extruding = 0,
  Timeout_Packaging = 0,
  Timeout_WPC = 0,

};


void (*resetFunc)(void) = 0;
__ID ID;
Master__ID mastID;

byte _IDnum = ID_Master;
ObjectTYPE _IDtype = T_AMEGA;

CANmannager can = CANmannager(_IDnum);
ERROR_Type mainError = NO_ERROR;

S_Timer_Redux timerZero;
S_Timer_Redux timerOnaOne;
S_Timer_Redux timerSerial;
byte counter = 0;
byte counter1 = 0;
unsigned long auxCounter = 0;
MainStatuses MainStatus = STATUS_MAIN_STANDBY;
CounterACK counterACK;
//SerialMannager serialDebug;



ERROR_Type __DummyFunction() {}


StateMachine D01SS(D01_SecureMode);
StateMachine D02SS(D02_SecureMode);

#include "M22_D01_SS.h"
#include "M22_D02_SS.h"


ERROR_Type InitializeD01() {

  D01.E0.Attach(ID_E0, &can);
  D01.E1.Attach(ID_E1, &can);
  D01.E2.Attach(ID_E2, &can);
  D01.E3.Attach(ID_E3, &can);
  D01.E4.Attach(ID_E4, &can);
  D01.E5.Attach(ID_E5, &can);

  D01.E0.SetGramsPerRevolution(GRAMS_PER_REVOLUTION);
  D01.E1.SetGramsPerRevolution(GRAMS_PER_REVOLUTION);
  D01.E2.SetGramsPerRevolution(GRAMS_PER_REVOLUTION);
  D01.E3.SetGramsPerRevolution(GRAMS_PER_REVOLUTION);
  D01.E4.SetGramsPerRevolution(GRAMS_PER_REVOLUTION);
  D01.E5.SetGramsPerRevolution(GRAMS_PER_REVOLUTION);

  // Pistons
  D01.Piston0.Attach(ID_HBridgeE0, Pin_HBE00, Pin_HBE01);
  D01.Piston1.Attach(ID_HBridgeE1, Pin_HBE10, Pin_HBE11);
  D01.Piston2.Attach(ID_HBridgeE2, Pin_HBE20, Pin_HBE21);
  D01.Piston3.Attach(ID_HBridgeE3, Pin_HBE30, Pin_HBE31);
  D01.Piston4.Attach(ID_HBridgeE4, Pin_HBE40, Pin_HBE41);
  D01.Piston5.Attach(ID_HBridgeE5, Pin_HBE50, Pin_HBE51);

  // Servo Selector ***** Change for Selector Custom Type ....
  D01.Selector.Attach(ID_Selector, ID_ServoSelector0, ID_ServoSelector1, ID_HBridgeGrind, ID_ServoGrindSize, ID_HBMixerSelector, Pin_SelGrinderPWM, Pin_SelByPassPWM, Pin_PowGrinder, Pin_SizeGrinderPWM, Pin_PowVibrator, 255, Pin_SelBeansDoor);
  byte value = 0;
  D01.status = STATUS_D01_STANDBY;

  D01SS.SetNextState(0);
  D01SS.RestartState();
  D01SS.PrintStates();
  D01SS.RestartState();
}

ERROR_Type InitializeD02() {
  D02.arm0.TurnTheta(180);
  D02.arm0.Attach(ID_Arm1, ID_MotorSlider11, ID_MotorSlider12, ID_ServoEndEff1, ID_VaacumEndEff1, ID_NODEF, ID_VaacumEndEff1, ID_NODEF, Pin_Arm0_TurnPWM, Pin_Arm0_HBVac11, Pin_NoDef, Pin_Arm0_Vac11IN, Pin_NoDef, &can, 180, 4);
  D02.arm1.Attach(ID_Arm2, ID_MotorSlider21, ID_MotorSlider22, ID_ServoEndEff2, ID_VaacumEndEff21, ID_VaacumEndEff22, ID_VaacumEndEff21, ID_VaacumEndEff22, Pin_Arm1_TurnPWM, Pin_Arm1_HBVac21, Pin_Arm1_HBVac22, Pin_Arm1_Vac21IN, Pin_Arm1_Vac22IN, &can, 0, 9);
  D02.foldBag.Attach(ID_Fold, ID_ServoFold1, ID_ServoFold2, ID_HB_FoldExtensor, Pin_Fold0_TurnPWM, Pin_Fold1_TurnPWM, Pin_FoldEx_HB0, Pin_FoldEx_HB1, Pin_Fold_HB0, Pin_Fold_HB1, 12, 255);
  D02.servicePlatform.Attach(ID_MotorServicePlatform, ID_HBridgeServicePlatform, ID_MotorServicePlatform, Pin_SP_HBDoor, Pin_SP_HBPush1, Pin_SP_HBPush2, Pin_MainDoor_INPUT, Pin_ServiceDoor_INPUT, Pin_BagSensor_INPUT, &can);
  D02.compressor.Attach(ID_Compressor, ID_HBridgeCompressorTurn, ID_ServoCompressorPurge, Pin_Compressor_PowHB, Pin_Compressorc_PurgePWM);
  D02.status = STATUS_D02_STANDBY;

  //D02.servicePlatform.RetractPlatform();

  D02SS.SetNextState(0);
  D02SS.RestartState();
  D02SS.PrintStates();
  D02SS.RestartState();
}

ERROR_Type __SetAllToStandByD01()  // Set All Devices to Rest Position
{
  D01.Piston0.SetState(PISTON_EXTRUDER_CLOSE);
  D01.Piston1.SetState(PISTON_EXTRUDER_CLOSE);
  D01.Piston2.SetState(PISTON_EXTRUDER_CLOSE);
  D01.Piston3.SetState(PISTON_EXTRUDER_CLOSE);
  D01.Piston4.SetState(PISTON_EXTRUDER_CLOSE);
  D01.Piston5.SetState(PISTON_EXTRUDER_CLOSE);

  D01.Selector.CloseSelectorDoor();
  D01.Selector.TurnOffGrinder();
  D01.Selector.TurnOffMixer();
  D01.Selector.SetGrinderSize(POWDERSIZE_FINE);
}



ERROR_Type __SetAllToStandByD02()  // Set All Devices to Rest Position
{
  //SendMSG(&can, ID_MotorSlider11, MSG_GOHOME, BYTE, false, P_RefPos, 0);
  //SendMSG(&can, ID_MotorSlider12, MSG_GOHOME, BYTE, false, P_RefPos, 0);
  //SendMSG(&can, ID_MotorSlider21, MSG_GOHOME, BYTE, false, P_RefPos, 0);
  //SendMSG(&can, ID_MotorSlider22, MSG_GOHOME, BYTE, false, P_RefPos, 0);
  //SendMSG(&can, ID_MotorServicePlatform, MSG_GOHOME, BYTE, false, P_RefPos, 0);
  delay(1);
  D02.arm0.GoHome(true);
  delay(1);
  D02.arm0.GoHome(false);
  delay(1);
  D02.arm1.GoHome(true);
  delay(1);
  D02.arm1.GoHome(false);
  delay(10);

  D02.arm0.TurnTheta(180);
  D02.arm0.TurnOffVac(0);


  D02.arm1.TurnTheta(0);
  D02.arm1.TurnOffVac(0);
  D02.arm1.TurnOffVac(1);

  D02.foldBag.MoveJoint_maxSpeed(true, 180);
  D02.foldBag.MoveJoint_maxSpeed(false, 180);
  D02.foldBag.CloseGrip();
  D02.foldBag.ExtendPiston();
  D02.foldBag.RetractPiston();

  D02.servicePlatform.CloseDoor();
  // D02.servicePlatform.GoHome();

  D02.compressor.TurnOffCompressor();
  D02.compressor.TurnOfPurge();

  return NO_ERROR;
}

ERROR_Type Initialize()  // Initialization of all devices
{
  long _value = 0;
  byte _param = 0;
  SendMSG(&can, ID_E0, MSG_STOP, LONG, false, (byte)_param, _value);
  delay(1);
  //SendMSG(&can, ID_E1, MSG_STOP, LONG, false, (byte)_param, _value);
  //delay(1);
  SendMSG(&can, ID_E2, MSG_STOP, LONG, false, (byte)_param, _value);
  delay(1);
  SendMSG(&can, ID_E3, MSG_STOP, LONG, false, (byte)_param, _value);
  delay(1);
  //SendMSG(&can, ID_E4, MSG_STOP, LONG, false, (byte)_param, _value);
  //delay(1);
  SendMSG(&can, ID_E5, MSG_STOP, LONG, false, (byte)_param, _value);
  delay(1);

  SendMSG(&can, ID_MotorSlider11, MSG_STOP, LONG, false, (byte)_param, _value);
  delay(1);
  SendMSG(&can, ID_MotorSlider12, MSG_STOP, LONG, false, (byte)_param, _value);
  delay(1);
  SendMSG(&can, ID_MotorSlider21, MSG_STOP, LONG, false, (byte)_param, _value);
  delay(1);
  SendMSG(&can, ID_MotorSlider22, MSG_STOP, LONG, false, (byte)_param, _value);
  delay(1);
  SendMSG(&can, ID_MotorServicePlatform, MSG_STOP, LONG, false, (byte)_param, _value);
  delay(100);
  D01SS_Initialize();
  D02SS_Initialize();
  InitializeD01();
  InitializeD02();
  __SetAllToStandByD01();
  __SetAllToStandByD02();


  return NO_ERROR;
}

bool ProcessSerialMSG(MainOrder _newOrder) {
  if (_newOrder.checkNewOrder == true) {
    switch (_newOrder.incomingChar) {
      case ACTION_RUN_PART:
        // Handle Run Loop action

        D01.status = STATUS_D01_EXTRUDING;
        D01SS.SetNextState(_newOrder.grams[0]);
        D01.mainOrder = _newOrder;
        D02.status = STATUS_D02_PACKAGING;
        D02SS.SetNextState(_newOrder.grams[1]);

        Serial.println("PartCommand");
        break;
      case ACTION_WPC:
        // Handle WPC action
        if (_newOrder.auxChar == 'p') {
          if (D02.status == STATUS_D02_WAITNG_WPC) {
            D02.status = STATUS_D02_FINISHING;
          }
        }
        break;
      case ACTION_RUN_LOOP:
        // Handle Run Loop action
        if (_newOrder.auxChar == 's') {
          if (D02.status == STATUS_D02_STANDBY && D01.status == STATUS_D01_STANDBY) {
            D01.status = STATUS_D01_EXTRUDING;
            D01SS.SetNextState(D01_Extrude);
            D01.mainOrder = _newOrder;
            D02.status = STATUS_D02_PACKAGING;
            Serial.println("#RS2");
            Serial.println("StartCommand");
          } else {
            Serial.println("StartFailed");
          }
        } else if (_newOrder.auxChar == 'p') {
          Serial.println("Do purge of extruder ");

          if (D02.status == STATUS_D02_SERVICE_MODE && _newOrder.grams[0] == 0) {
            D01.mainOrder = _newOrder;
            D02.status = STATUS_D02_PURGE_MODE;
          } else if (D02.status == STATUS_D02_SERVICE_MODE && _newOrder.grams[0] == 1) {
            D01.mainOrder = _newOrder;
            D02.status = STATUS_D02_PURGED_MODE;
          }

        } else if (_newOrder.auxChar == 'y') {
          D01.status = STATUS_D01_EXTRUDING;
          D01SS.SetNextState(_newOrder.grams[0]);
          D01.mainOrder = _newOrder;
          D02.status = STATUS_D02_PACKAGING;
          D02SS.SetNextState(_newOrder.grams[1]);

          Serial.println("PartCommand");
          ;
        }
        break;
      case ACTION_MOVE_EXTRUDER:
        // Handle Move Extruder action
        switch (_newOrder.auxChar) {
          case '0':
            // Handle digit 0
            D01.E0.ExtrudeGrams(_newOrder.grams[0]);
            break;
          case '1':
            // Handle digit 1
            //D01.E1.ExtrudeGrams(_newOrder.grams[0]);
            break;
          case '2':
            // Handle digit 2
            D01.E2.ExtrudeGrams(_newOrder.grams[0]);
            break;
          case '3':
            // Handle digit 3
            D01.E3.ExtrudeGrams(_newOrder.grams[0]);
            break;
          case '4':
            // Handle digit 4
            //D01.E4.ExtrudeGrams(_newOrder.grams[0]);
            break;
          case '5':
            // Handle digit 5
            D01.E5.ExtrudeGrams(_newOrder.grams[0]);
            break;
          case '6':
            Serial.print("E0: ");
            Serial.print(counterACK.GetExtruderState(0));
            Serial.print("\t E1: ");
            Serial.print(counterACK.GetExtruderState(1));
            Serial.print("\t E2: ");
            Serial.print(counterACK.GetExtruderState(2));
            Serial.print("\t E3: ");
            Serial.print(counterACK.GetExtruderState(3));
            Serial.print("\t E4: ");
            Serial.print(counterACK.GetExtruderState(4));
            Serial.print("\t E5: ");
            Serial.println(counterACK.GetExtruderState(5));
            break;
          case '9':
            // Handle digit 5
            D01.E0.ExtrudeGrams(_newOrder.grams[0]);
            D01.E1.ExtrudeGrams(_newOrder.grams[1]);
            D01.E2.ExtrudeGrams(_newOrder.grams[2]);
            D01.E3.ExtrudeGrams(_newOrder.grams[3]);
            D01.E4.ExtrudeGrams(_newOrder.grams[4]);
            D01.E5.ExtrudeGrams(_newOrder.grams[5]);
            break;
          default:
            Serial.println("ExtrudeOrderFailed");
            break;
        }
        break;
      case ACTION_MOVE_PISTON:
        // Handle Move Piston action
        switch (_newOrder.auxChar) {
          case '0':
            // Handle digit 0
            D01.Piston0.SetState(_newOrder.grams[0]);
            break;
          case '1':
            // Handle digit 1
            D01.Piston1.SetState(_newOrder.grams[0]);
            break;
          case '2':
            // Handle digit 2
            D01.Piston2.SetState(_newOrder.grams[0]);
            break;
          case '3':
            // Handle digit 3
            D01.Piston3.SetState(_newOrder.grams[0]);
            break;
          case '4':
            // Handle digit 4
            D01.Piston4.SetState(_newOrder.grams[0]);
            break;
          case '5':
            // Handle digit 5
            D01.Piston5.SetState(_newOrder.grams[0]);
            break;
          case '9':
            // Handle digit 5
            D01.Piston0.SetState(_newOrder.grams[0]);
            D01.Piston1.SetState(_newOrder.grams[1]);
            D01.Piston2.SetState(_newOrder.grams[2]);
            D01.Piston3.SetState(_newOrder.grams[3]);
            D01.Piston4.SetState(_newOrder.grams[4]);
            D01.Piston5.SetState(_newOrder.grams[5]);
            break;
          default:
            Serial.println("ExtrudeOrderFailed");
            break;
        }
        break;
      case ACTION_MIXER:
        // Handle Mixer action
        break;
      case ACTION_GOHOME:
        // Handle Mixer action
        switch (_newOrder.auxChar) {
          case '0':
            // Handle digit 0
            D02.arm0.GoHome(false);
            break;
          case '1':
            // Handle digit 1
            D02.arm0.GoHome(true);
            break;
          case '2':
            // Handle digit 2
            D02.arm1.GoHome(false);
            break;
          case '3':
            // Handle digit 3
            D02.arm1.GoHome(true);
            break;
          case '9':
            // Handle digit 5
            D02.arm0.GoHome(false);
            D02.arm0.GoHome(true);
            D02.arm1.GoHome(false);
            D02.arm1.GoHome(true);
            break;
          default:
            Serial.println("SliderOrderFailed");
            break;
        }
        break;
      case ACTION_ACTIVATE_SERVOS:
        Serial.println("Activate Servos");
        D01_ActivationServos();
        D02_ActivationServos();
        break;
      case ACTION_DEACTIVATE_SERVOS:
        Serial.println("Deactivate Servos");
        D01_DeactivationServos();
        D02_DeactivationServos();
        break;
      case ACTION_SELECTOR:
        // Handle Selector action
        switch (_newOrder.auxChar) {
          case SELECTOR_CLOSED:
            D01.Selector.CloseSelectorDoor();
            break;
          case SELECTOR_BYPASS:
            D01.Selector.OpenByPassDoor();
            break;
          case SELECTOR_GRINDER:
            D01.Selector.OpenGrinderDoor();
            break;
          case '0':
            D01.Selector.TuneServoSelector(false, _newOrder.grams[0]);
            break;
          case '1':
            D01.Selector.TuneServoSelector(true, _newOrder.grams[0]);
            break;
          case 'v':
            D01.Selector.SetBeansDoor(_newOrder.grams[0]);
            break;
          default:
            break;
        }
        break;
      case ACTION_GRINDER:
        // Handle Grinder action
        switch (_newOrder.auxChar) {
          case GRINDER_OFF:
            D01.Selector.TurnOffGrinder();
            Serial.println("GrindOFF");
            break;
          case GRINDER_ON:
            Serial.println("GrindON");
            D01.Selector.TurnOnGrinder();
            break;
          case GRINDER_SIZE:
            Serial.println("SelectSize");
            D01.Selector.SetGrinderSize(_newOrder.grams[0]);
          default:
            break;
        }
        break;
      case ACTION_SLIDER:
        // Handle Slider action
        switch (_newOrder.auxChar) {
          case '0':
            // Handle digit 0
            Serial.println("Move Slider11");
            D02.arm0.MoveX(_newOrder.grams[0]);
            break;
          case '1':
            // Handle digit 1
            D02.arm0.MoveY(_newOrder.grams[0]);
            break;
          case '2':
            // Handle digit 2
            D02.arm1.MoveX(_newOrder.grams[0]);
            break;
          case '3':
            // Handle digit 3
            D02.arm1.MoveY(_newOrder.grams[0]);
            break;
          case '4':
            // Handle digit 4
            Serial.print("S11: ");
            Serial.print(counterACK.GetSliderState(0));
            Serial.print("\t S12: ");
            Serial.print(counterACK.GetSliderState(1));
            Serial.print("\t S21: ");
            Serial.print(counterACK.GetSliderState(2));
            Serial.print("\t S22: ");
            Serial.print(counterACK.GetSliderState(3));
            Serial.print("\t SP: ");
            Serial.println(counterACK.GetSliderState(4));
            break;

          case '9':
            // Handle digit 5
            D02.arm0.MoveX(_newOrder.grams[0]);
            D02.arm0.MoveY(_newOrder.grams[1]);
            D02.arm1.MoveX(_newOrder.grams[2]);
            D02.arm1.MoveY(_newOrder.grams[3]);
            break;
          default:
            Serial.println("SliderOrderFailed");
            break;
        }
        break;
      case ACTION_PACKAGINGSERVO:
        // Handle Packaging Servo action
        switch (_newOrder.auxChar) {
          case '0':
            // Handle digit 0
            D02.arm0.TurnTheta(_newOrder.grams[0]);
            break;
          case '1':
            // Handle digit 1
            D02.arm1.TurnTheta(_newOrder.grams[0]);
            break;
          case '2':
            // Handle digit 2
            Serial.println("move joint Fold0");
            D02.foldBag.MoveJoint_maxSpeed(false, _newOrder.grams[0]);
            break;
          case '3':
            // Handle digit 3
            Serial.println("move joint Fold1");
            D02.foldBag.MoveJoint_maxSpeed(true, _newOrder.grams[0]);
            break;
          case '4':
            // Handle digit 2
            Serial.println("move joint Fold0");
            D02.foldBag.MoveJoint(false, _newOrder.grams[0]);
            break;
          case '5':
            // Handle digit 3
            Serial.println("move joint Fold1");
            D02.foldBag.MoveJoint(true, _newOrder.grams[0]);
            break;
          case '6':
            // Handle digit 4

            break;

          case '9':
            // Handle digit 5
            D02.arm0.TurnTheta(_newOrder.grams[0]);
            D02.arm1.TurnTheta(_newOrder.grams[1]);
            D02.foldBag.MoveJoint_maxSpeed(false, _newOrder.grams[2]);
            D02.foldBag.MoveJoint_maxSpeed(true, _newOrder.grams[3]);
            break;
          default:
            Serial.println("PackagingServoFailed");
            break;
        }
        break;
      case ACTION_FOLDPISTONS:
        // Handle Fold Pistons action
        Serial.println("foldAction");
        switch (_newOrder.auxChar) {
          case '0':
            // Handle digit 0
            D02.foldBag.RetractPiston();
            break;
          case '1':
            // Handle digit 1
            D02.foldBag.ExtendPiston();
            break;
          case '2':
            // Handle digit 0

            D02.foldBag.CloseGrip();
            break;
          case '3':
            // Handle digit 1
            D02.foldBag.OpenGrip();
            break;
          case '4':
            // Handle digit 1
            D02.servicePlatform.ExtendPlatform();
            break;
          case '5':
            // Handle digit 1
            D02.servicePlatform.RetractPlatform();
            break;
        }
        break;
      case ACTION_SERVICEDOOR:
        // Handle Service Door action
        break;
      case ACTION_VACUUM_VALVE:
        // Handle Vacuum Valve action
        switch (_newOrder.auxChar) {
          case 's':
            Serial.print("Vacuum A0-0: ");
            Serial.print(D02.arm0.IsVacSense(0));
            Serial.print("\t Vacuum A1-0: ");
            Serial.print(D02.arm1.IsVacSense(0));
            Serial.print("\t Vacuum A1-1: ");
            Serial.println(D02.arm1.IsVacSense(1));
            //A0-0 -3
            break;
          case '0':
            // Handle digit 0
            D02.arm0.TurnOnVac(false);
            break;
          case '1':
            // Handle digit 1
            D02.arm0.TurnOffVac(false);
            break;
          case '2':
            // Handle digit 0
            D02.arm1.TurnOnVac(false);
            break;
          case '3':
            // Handle digit 1
            D02.arm1.TurnOffVac(false);
            break;
          case '4':
            // Handle digit 0
            D02.arm1.TurnOnVac(true);
            break;
          case '5':
            // Handle digit 1
            D02.arm1.TurnOffVac(true);
            break;
          case '8':
            // Handle digit 3
            D02.arm1.TurnOnVac(false);
            D02.arm1.TurnOnVac(false);
            D02.arm1.TurnOnVac(true);
            break;
          case '9':
            // Handle digit 3
            D02.arm1.TurnOffVac(false);
            D02.arm1.TurnOffVac(false);
            D02.arm1.TurnOffVac(true);
            break;
        }
        break;
      case ACTION_COMPRESOR:
        // Handle Compressor action
        break;
      case ACTION_KILLSELF:
        // Handle SelfKill Action
        Serial.println("SelfReset");
        D02_DeactivationServos();
        delay(1000);
        resetFunc();
        break;
      case ACTION_INITIALIZE:
        // Handle Compressor action
        Serial.println("Initialize");
        if (_newOrder.auxChar == 'a') {
          D02SS_Initialize();
          D01SS_Initialize();
          D02SS.SetNextState(0);
          D01SS.SetNextState(0);
          __SetAllToStandByD01();
          __SetAllToStandByD02();
          Initialize();
        } else if (_newOrder.auxChar == '1') {
          __SetAllToStandByD01();
        } else if (_newOrder.auxChar == '2') {
          __SetAllToStandByD02();
        } else if (_newOrder.auxChar == 's') {
          if (D02.status == STATUS_D02_SECURE_MODE) {
            D02.status = STATUS_D02_STANDBY;
            D02SS.SetNextState(D02_Init);
          }
          if (D01.status == STATUS_D01_SECURE_MODE) {
            D01.status = STATUS_D01_STANDBY;
            D01SS.SetNextState(0);
          }
        } else {
        }
        break;
      case ACTION_NO_DEF:
        // Handle No Defined action
        break;
      default:
        // Handle Unknown action
        Serial.println("Failed Incomming Serial");
        return false;
        break;
    }
    return true;
  }

  else {
    //Serial.println("Failed Incomming Serial");
    return false;
  }
}

MainOrder ReadData() {
  //SerialData _data;
  String numericValueString = "";
  MainOrder _newOrder;
  unsigned counter = 0;

  if (Serial.available() > 0) {

    if (Serial.read() == ORDER_CHARACTER) {
      Serial.print("NewSerial:");
      _newOrder.incomingChar = Serial.read();  // Read the first character
      delay(1);
      Serial.print(_newOrder.incomingChar);
      Serial.print("\t");
      if (Serial.available() > 0) {
        _newOrder.auxChar = Serial.read();  // Read the second character
        delay(1);
        _newOrder.checkNewOrder = true;
        Serial.print(_newOrder.auxChar);
        Serial.print("\t");

        while (Serial.available() > 0) {
          char numericChar = Serial.read();

          if (isDigit(numericChar) || numericChar == '-') {
            numericValueString += numericChar;

          } else if ((numericChar == SEPARATION_CHARACTER || numericChar == '\n') && counter < MAX_DATA_ALLOWED) {
            if (counter == (MAX_DATA_ALLOWED - 1)) {
              _newOrder.grindSize = numericValueString.toInt();
              numericValueString = "";
              if (_newOrder.grindSize == POWDERSIZE_MAX) {
                _newOrder.selectorState = SELECTOR_BYPASS;
              } else {
                _newOrder.selectorState = SELECTOR_GRINDER;
              }
            } else {
              _newOrder.grams[counter] = numericValueString.toInt();
              numericValueString = "";
              Serial.print(_newOrder.grams[counter]);
              Serial.print(SEPARATION_CHARACTER);
            }

            counter++;
          }

          delay(1);
        }
        Serial.println("");
      }
      return _newOrder;
    }
  }
}

void SerialStatusMSG() {
  //“#ST00/101101/11110/11/1
  if (timerSerial.Check(SERIAL_PERIOD)) {
    String serialMSG = "#ST";
    //serialMSG += String(mainError,HEX);
    serialMSG += String(D01SS.GetState(), HEX);
    serialMSG += String(D02SS.GetState(), HEX);
    serialMSG += SEPARATION_CHARACTER;
    for (int i = 0; i < 6; i++) {
      serialMSG += String(counterACK.GetExtruderState(i));
    }
    serialMSG += SEPARATION_CHARACTER;
    for (int i = 0; i < 5; i++) {
      serialMSG += String(counterACK.GetSliderState(i));
    }
    serialMSG += SEPARATION_CHARACTER;
    serialMSG += String(D02.arm0.IsVacSense(0));
    serialMSG += String(D02.arm1.IsVacSense(0));

    //serialMSG += String(D02.servicePlatform.GetMainDoorState());
    //serialMSG += String(D02.servicePlatform.GetServiceDoorState());
    serialMSG += SEPARATION_CHARACTER;
    serialMSG += String(D02.servicePlatform.GetBagSensorRead());
    Serial.println(serialMSG);
    timerSerial.reset();
  }
}

void mainRun() {
  ProcessSerialMSG(ReadData());
  SerialStatusMSG();
  D02.arm0.Run();
  D02.arm1.Run();
  D02.foldBag.Run();
}

void setup() {

  Serial.begin(115200);//***************************Cambio realizado
  Serial.print("Start System : ");
  Serial.print("MEGA");

  ID.AttachID(_IDnum, _IDtype);
  can.CanSetID(_IDnum);
  can.SetMode(NORMAL_MODE);
  delay(500);
  Initialize();
  Serial.println("#RS1");
  /* 
   D01.E0.ExtrudeGrams(0);
    delay(1);
    //D01.E1.ExtrudeGrams(0);
    //delay(1);
    D01.E2.ExtrudeGrams(0);
    delay(1);
    D01.E3.ExtrudeGrams(0);
    delay(1);
    //D01.E4.ExtrudeGrams(0);
    //delay(1);
    D01.E5.ExtrudeGrams(0);
    delay(1);
  */
  delay(3000);
  // while (counterACK.GetSliderState(0) == ACK_standby && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(2) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby && counterACK.GetSliderState(4) == ACK_standby) {
  //WAIT UNTIL ALL IT's INITIALIZE
  //CHECK TIMEOUT ON RELEASE!!
  //}
}

void canEvento() {
  if (can.ReciveMsg() == MCP2515::ERROR_OK) {
    can_frame canMsg = can.ReadLastMsg();
    {
      CanMsg_Type msgType = canMsg.data[COMMANDORDER] & AUXCOMMANDMASK;
      switch (msgType) {
        case MSG_ACK:
          // Handle ACK message
          break;
        case MSG_ERROR:
          // Handle ERROR message
          break;
        case MSG_STATE:
          //Serial.println("SetACKState_Reset");
          byte _index = 255;
          switch (canMsg.data[PARAMORDER]) {
            case ID_MotorSlider11:
              //Serial.println("Extruder0");
              _index = Slider11;
              break;
            case ID_MotorSlider12:
              _index = Slider12;
              break;
            case ID_MotorSlider21:
              _index = Slider21;
              break;
            case ID_MotorSlider22:
              _index = Slider22;
              break;
            case ID_MotorServicePlatform:
              _index = SliderSP;
              break;
          }
          if (_index != 255) {
            //Serial.println("Restarted");
            counterACK.SetNewSliderACK(canMsg.data[3], canMsg.data[IDSENDERORDER], _index);
          } else {
            switch (canMsg.data[PARAMORDER]) {
              case ID_E0:
                _index = extruder0;
                break;
              case ID_E1:
                _index = extruder1;
                break;
              case ID_E2:
                _index = extruder2;
                break;
              case ID_E3:
                _index = extruder3;
                break;
              case ID_E4:
                _index = extruder4;
                break;
              case ID_E5:
                _index = extruder5;
                break;
            }
            counterACK.SetNewExtruderACK(canMsg.data[3], canMsg.data[IDSENDERORDER], _index);
          }
          break;
        default:
          // Handle unexpected message type
          break;
      }
    }
  }
}


void loop() {

  D01SS.Run();
  D02SS.Run();
  mainRun();
  canEvento();
}

// -- END OF FILE --