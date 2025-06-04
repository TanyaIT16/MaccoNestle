#include "TimerOne.h"
void D01SS_Initialize() {
  D01SS.AttachNewState(D01_NoDefined, &D01_NoDefinedRun, &D01_NoDefinedStart, &D01_NoDefinedEnd, 0, 0, Timeout_Disabled);
  D01SS.AttachNewState(D01_Standby, &D01_StandbyRun, &D01_StandbyStart, &D01_StandbyEnd, 0, 0, Timeout_Disabled);
  D01SS.AttachNewState(D01_Extrude, &D01_ExtrudeRun, &D01_ExtrudeStart, &D01_ExtrudeEnd, 0, 0, Timeout_Extruding);
  D01SS.AttachNewState(D01_GrindSetup, &D01_GrindSetupRun, &D01_GrindSetupStart, &D01_GrindSetupEnd, 0, 0, Timeout_Extruding);
  D01SS.AttachNewState(D01_DoMixing, &D01_DoMixingRun, &D01_DoMixingStart, &D01_DoMixingEnd, 0, 0, Timeout_Extruding);
  D01SS.AttachNewState(D01_DoSelect, &D01_DoSelectRun, &D01_DoSelectStart, &D01_DoSelectEnd, 0, 0, Timeout_Extruding);
  D01SS.AttachNewState(D01_DoGrind, &D01_DoGrindRun, &D01_DoGrindStart, &D01_DoGrindEnd, 0, 0, Timeout_Extruding);
  D01SS.AttachNewState(D01_Finish, &D01_FinishRun, &D01_FinishStart, &D01_FinishEnd, 0, 0, Timeout_Extruding);
  D01SS.AttachNewState(D01_SecureMode, &D01_SecureModeRun, &D01_SecureModeStart, &D01_SecureModeEnd, 0, 0, Timeout_Extruding);
}

ERROR_Type D01_ActivationServos() {
  D01.Selector.ActivateServos();
}

ERROR_Type D01_DeactivationServos() {
  D01.Selector.DeactivateServos();
}

// Function prototypes for D01_NoDefined
ERROR_Type D01_NoDefinedStart() {
  return NO_ERROR;
}

ERROR_Type D01_NoDefinedRun() {
  return NO_ERROR;
}

ERROR_Type D01_NoDefinedEnd() {
  return NO_ERROR;
}

// Function prototypes for D01_StandBy
ERROR_Type D01_StandbyStart() {
  //D01.Selector.CloseBeansDoor();
  D01.Selector.CloseSelectorDoor();
  D01.Selector.TurnOffGrinder();
  D01.Selector.DeactivateServos();
  D01.Selector.TurnOffMixer();
  return NO_ERROR;
}

ERROR_Type D01_StandbyRun() {

  return NO_ERROR;
}

ERROR_Type D01_StandbyEnd() {

  return NO_ERROR;
}

// Function prototypes for D01_Extrude
ERROR_Type D01_ExtrudeStart() {
  Serial.println("D01Starting Extrude");
  D01SS.Counter_Reset(0);
  timerOnaOne.reset();
  D01.Selector.ActivateServos();
  D01.Selector.CloseSelectorDoor();
  //D01.Selector.CloseBeansDoor();
  D01.Selector.TurnOnMixer();
  //counterACK.SetNewExtruderACK(ACK_standby, ID_E0, 0);
  //counterACK.SetNewExtruderACK(ACK_standby, ID_E2, 2);
  //counterACK.SetNewExtruderACK(ACK_standby, ID_E3, 3);
  //counterACK.SetNewExtruderACK(ACK_standby, ID_E5, 5);
  return NO_ERROR;
}

ERROR_Type D01_ExtrudeRun() {
  if (D01SS.counter == 0 && timerOnaOne.Check(Period_servo * 1000)) {
    Serial.println("Extruding");
    D01.E0.ExtrudeGrams(D01.mainOrder.grams[0]);
    delay(1);
    //D01.E1.ExtrudeGrams(D01.mainOrder.grams[1]);
    //delay(1);
    D01.E2.ExtrudeGrams(D01.mainOrder.grams[2]);
    delay(1);
    D01.E3.ExtrudeGrams(D01.mainOrder.grams[3]);
    delay(1);
    //D01.E4.ExtrudeGrams(D01.mainOrder.grams[4]);
    //delay(1);
    D01.E5.ExtrudeGrams(D01.mainOrder.grams[5]);
    delay(1);
    D01SS.counter = 1;

  } else if (D01SS.counter == 1 /*Should Check Extruders Are Finished*/) {
    D01SS.SetNextState(D01_GrindSetup);
  } else {
    // JUST PASS
  }

  return NO_ERROR;
}

ERROR_Type D01_ExtrudeEnd() {

  return NO_ERROR;
}

// Function prototypes for D01_GrindSetUp
ERROR_Type D01_GrindSetupStart() {
  Serial.println("D01Starting SetupGrind");
  D01SS.Counter_Reset(0);
  timerOnaOne.reset();
  return NO_ERROR;
}

ERROR_Type D01_GrindSetupRun() {
  if (D01SS.counter == 0) {
    if (D01.mainOrder.grindSize < POWDERSIZE_MAX) {
      D01.Selector.SetGrinderSize(D01.mainOrder.grindSize);
      Serial.print("Size:");
      Serial.println(D01.mainOrder.grindSize);
      //D01.Selector.TurnOffGrinder();
      Serial.println("Grind Select");
    } else {
      Serial.println("ByPass Select");
      //D01.Selector.TurnOffGrinder();
    }
    D01SS.counter = 1;
  } else if (D01SS.counter == 1 && counterACK.GetExtruderState(0) == ACK_standby && counterACK.GetExtruderState(2) == ACK_standby && counterACK.GetExtruderState(3) == ACK_standby && counterACK.GetExtruderState(5) == ACK_standby) {
    D01SS.SetNextState(D01_DoMixing);
  } else {
    // JUST PASS
  }

  return NO_ERROR;
}

ERROR_Type D01_GrindSetupEnd() {

  return NO_ERROR;
}


// Function prototypes for D01_Mixing
ERROR_Type D01_DoMixingStart() {
  Serial.println("D01Starting DoMixing");
  //Turn On Mixer
  D01.Selector.TurnOnMixer();
  D01SS.Counter_Reset(0);
  timerOnaOne.reset();
  return NO_ERROR;
}

ERROR_Type D01_DoMixingRun() {
  if (D01SS.counter == 0 && timerOnaOne.Check(Period_Mixing * 1000)) {
    //Wait Mixing
    D01.Selector.TurnOffMixer();
    Serial.println("Mixing Done");
    timerOnaOne.reset();
    D01SS.counter = 1;
    //Wait Until Bag It's ready...
  } else if (D01SS.counter == 1 && counterACK.GetExtruderState(0) == ACK_standby && counterACK.GetExtruderState(2) == ACK_standby && counterACK.GetExtruderState(3) == ACK_standby && counterACK.GetExtruderState(5) == ACK_standby /*Should Check Extruders Are Finished*/) {
    if (D02.status == STATUS_D02_WAITNG_GRIND) {
      //When Bag It's Ready -->> Continue
      D01SS.counter = 2;
    }
  } else if (D01SS.counter == 2) {
    D01SS.SetNextState(D01_DoSelect);
  } else {
    // JUST PASS
  }
  return NO_ERROR;
}

ERROR_Type D01_DoMixingEnd() {

  return NO_ERROR;
}

// Function prototypes for D01_Select
ERROR_Type D01_DoSelectStart() {
  Serial.println("D01Starting DoSelect");
  D01SS.Counter_Reset(0);
  timerOnaOne.reset();
  return NO_ERROR;
}

ERROR_Type D01_DoSelectRun() {
  if (D01SS.counter == 0) {
    ///Open Release Beans Door
    //D01.Selector.OpenBeansDoor();
    if (D01.mainOrder.grindSize < POWDERSIZE_MAX) {
      Serial.println("GrinderSelector");
      Serial.print("Size:");
      Serial.println(D01.mainOrder.grindSize);
      D01.Selector.OpenGrinderDoor();
      D01SS.counter = 1;
    } else {
      D01.Selector.OpenByPassDoor();
      Serial.println("BypassSelector");
      timerOnaOne.reset();
      D01SS.counter = 1;
    }


  } else if (D01SS.counter == 1 && timerOnaOne.Check (5000)) {
    D01SS.SetNextState(D01_DoGrind);
  } else {
    // JUST PASS
  }
  return NO_ERROR;
}

ERROR_Type D01_DoSelectEnd() {

  return NO_ERROR;
}

// Function prototypes for D01_Grind
ERROR_Type D01_DoGrindStart() {
  Serial.println("D01Starting Grinding");
  D01SS.Counter_Reset(0);
  //D01.Selector.OpenBeansDoor();
  timerOnaOne.reset();
  return NO_ERROR;
}

ERROR_Type D01_DoGrindRun() {
  unsigned wait_Period =  (10000);
  if (D01SS.counter == 0) {
    
    if (D01.mainOrder.grindSize < POWDERSIZE_MAX) {
      
      D01.Selector.TurnOnGrinder();
      Serial.println("Grinding");
      Serial.print("Size:");
      Serial.println(D01.mainOrder.grindSize);
      timerOnaOne.reset();
    } else {
      
      Serial.println("ByPassing");
      //D01.Selector.TurnOffGrinder();
      timerOnaOne.reset();
    }
    D01SS.counter = 1;
  } else if (D01SS.counter == 1 && timerOnaOne.Check(wait_Period)) {
    D01.Selector.TurnOffGrinder();
    timerOnaOne.reset();
    D01SS.counter = 2;
  } else if (D01SS.counter == 2) {

    D01SS.SetNextState(D01_Finish);
  } else {
    // JUST PASS
  }
  return NO_ERROR;
}

ERROR_Type D01_DoGrindEnd() {

  return NO_ERROR;
}

// Function prototypes for D01_Finish
ERROR_Type D01_FinishStart() {
  Serial.println("Finishing D01");
  D01.status = STATUS_D01_FINISHED;
  D01SS.Counter_Reset(0);
  //D01.Selector.CloseBeansDoor();
  timerOnaOne.reset();
  D01SS.SetNextState(D01_Standby);
  return NO_ERROR;
}

ERROR_Type D01_FinishRun() {

  return NO_ERROR;
}

ERROR_Type D01_FinishEnd() {

  return NO_ERROR;
}


ERROR_Type D01_SecureModeStart() {
  Serial.println("SecureMode D01");
  D01.status = STATUS_D01_SECURE_MODE;
  D01SS.Counter_Reset(0);
  timerOnaOne.reset();
  D01.Selector.CloseBeansDoor();
  D01.Selector.CloseSelectorDoor();
  D01.Selector.TurnOffGrinder();
  D01.Selector.DeactivateServos();
  D01.Selector.TurnOffMixer();
  D02SS.SetNextState(D02_SecureMode);
  //  D01SS.SetNextState(D01_Standby);
  return NO_ERROR;
}

ERROR_Type D01_SecureModeRun() {

  return NO_ERROR;
}

ERROR_Type D01_SecureModeEnd() {
  Serial.println("Leaving SecureMode D01");
  return NO_ERROR;
}