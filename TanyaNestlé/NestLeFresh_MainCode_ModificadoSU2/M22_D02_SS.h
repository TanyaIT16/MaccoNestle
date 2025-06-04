#include "HardwareSerial.h"
#include "Arduino.h"

#include "Serial_Macco.h"

void D02SS_Initialize() {
  // Attach each function to the state machine
  D02SS.AttachMainState(&D02_MainState);
  D02SS.AttachNewState(D02_NoDefined, &D02_NoDefinedRun, &D02_NoDefinedStart, &D02_NoDefinedEnd, 0, 0, Timeout_Disabled);
  D02SS.AttachNewState(D02_Init, &D02_InitRun, &D02_InitStart, &D02_InitEnd, 0, 0, Timeout_Packaging);
  D02SS.AttachNewState(D02_Standby, &D02_StandbyRun, &D02_StandbyStart, &D02_StandbyEnd, 0, 0, Timeout_Disabled);
  D02SS.AttachNewState(D02_GOtoPickBag, &D02_GOtoPickBagRun, &D02_GOtoPickBagStart, &D02_GOtoPickBagEnd, 0, 0, Timeout_Packaging);
  D02SS.AttachNewState(D02_GoToPatform, &D02_GoToPatformRun, &D02_GoToPatformStart, &D02_GoToPatformEnd, 0, 0, Timeout_Packaging);
  D02SS.AttachNewState(D02_OpenBag, &D02_OpenBagRun, &D02_OpenBagStart, &D02_OpenBagEnd, 0, 0, Timeout_Packaging);
  D02SS.AttachNewState(D02_Weight, &D02_WeightRun, &D02_WeightStart, &D02_WeightEnd, 0, 0, Timeout_WPC);
  D02SS.AttachNewState(D02_FoldBag, &D02_FoldBagRun, &D02_FoldBagStart, &D02_FoldBagEnd, 0, 0, Timeout_Packaging);
  D02SS.AttachNewState(D02_PickTicket, &D02_PickTicketRun, &D02_PickTicketStart, &D02_PickTicketEnd, 0, 0, Timeout_Packaging);
  D02SS.AttachNewState(D02_PlaceTicket, &D02_PlaceTicketRun, &D02_PlaceTicketStart, &D02_PlaceTicketEnd, 0, 0, Timeout_Packaging);
  D02SS.AttachNewState(D02_PackaginFinish, &D02_PackaginFinishRun, &D02_PackaginFinishStart, &D02_PackaginFinishEnd, 0, 0, Timeout_Packaging);
  D02SS.AttachNewState(D02_SecureMode, &D02_SecureModeRun, &D02_SecureModeStart, &D02_SecureModeEnd, 0, 0, Timeout_Disabled);
  D02SS.AttachNewState(D02_ServiceMode, &D02_ServiceModeRun, &D02_ServiceModeStart, &D02_ServiceModeEnd, 0, 0, Timeout_Disabled);
}

ERROR_Type D02_ActivationServos() {
  D02.arm0.ActivateServos();
  D02.arm1.ActivateServos();
  D02.foldBag.ActivateServos();
}

ERROR_Type D02_DeactivationServos() {
  D02.arm0.DeactivateServos();
  D02.arm1.DeactivateServos();
  D02.foldBag.DeactivateServos();
}

ERROR_Type D02_MainState() {  //Execute Always
  if (!D02.servicePlatform.GetMainDoorState()) {
    //D02SS.SetNextState();
    //D02SS.SetNextState();

  } else {
  }
  if (!D02.servicePlatform.GetBagSensorRead()) {
    D02SS.SetNextState(D02_SecureMode);
    D01SS.SetNextState(D01_SecureMode);
  } else {
  }
}


// Function prototypes for D02_NoDefined
ERROR_Type D02_NoDefinedStart() {}
ERROR_Type D02_NoDefinedRun() {
  D02SS.SetNextState(D02_Init);
}
ERROR_Type D02_NoDefinedEnd() {}

ERROR_Type D02_InitStart() {
  Serial.println("Init State");
  D02.status = STATUS_D02_STANDBY;
  D02.servicePlatform.RetractPlatform();
  D02.arm0.TurnOffVac(false);
}
ERROR_Type D02_InitRun() {
  if (D02.status == STATUS_D02_STANDBY) {
    Serial.println(D02.status);
    D02SS.SetNextState(D02_Standby);
  }
}
ERROR_Type D02_InitEnd() {}

// Function prototypes for D02_Standby
ERROR_Type D02_StandbyStart() {
  Serial.println("D02Entering StandBy Packaging");
  __SetAllToStandByD02();
  D02SS.Counter_Reset(0);
  timerZero.reset();
  return NO_ERROR;
}
ERROR_Type D02_StandbyRun() {
  if (D02SS.counter == 0 && counterACK.GetSliderState(0) == ACK_standby && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby && counterACK.GetSliderState(2) == ACK_standby) {
    //Parking Positions Arms
    Serial.print("State 0, step");
    Serial.println(D02SS.counter);
    D02.servicePlatform.RetractPlatform();
    D01.Selector.CloseBeansDoor();
    //D02.arm0.MoveX(40);
    delay(1);
    //D02.arm0.MoveY(10);
    delay(1);
    //D02.arm1.MoveX(20);
    delay(1);
    //D02.arm1.MoveY(90);
    delay(1);
    // Parking Position Servos
    D02.arm0.TurnTheta(915);//******************915
    D02.arm1.TurnTheta(2000);//***************+10

    // Turn OFF Vacuum
    D02.arm0.TurnOffVac(0);
    D02.arm1.TurnOffVac(0);
    D02.arm1.TurnOffVac(1);

    //Parking Position Folder
    D02.foldBag.MoveJoint(false, 180);
    D02.foldBag.CloseGrip();
    D02.foldBag.RetractPiston();

    D02SS.counter = 1;
  }

  else if (D02SS.counter == 1) {
    if (D02.status == STATUS_D02_PACKAGING) {  //Recieve Comm8and to start Packaging
      // WAIT until all the sliders are stopped
      if ((counterACK.GetSliderState(0) == ACK_standby && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(2) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby)) {
        Serial.print("State 0, step");
        Serial.println(D02SS.counter);

        delay(1);
        timerZero.reset();
        D02SS.counter = 2;
      }
    }
  } else if (D02SS.counter == 2 && counterACK.GetSliderState(0) == ACK_standby && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(2) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby) {

    D02.arm0.MoveX(60);
    delay(1);
    D02.arm0.MoveY(10);
    delay(1);
    D02.arm1.MoveX(20);
    delay(1);
    D02.arm1.MoveY(90);
    delay(1);
    timerZero.reset();
    D02SS.SetNextState(D02_GOtoPickBag);
  } else {
    // JUST PASS!!
  }
  return NO_ERROR;
}
ERROR_Type D02_StandbyEnd() {
  Serial.println("Leaving StandBy");
  return NO_ERROR;
}

// Function prototypes for D02_GOtoPickBag
ERROR_Type D02_GOtoPickBagStart() {
  D02SS.Counter_Reset(0);
  timerZero.reset();
  Serial.println("D02Entering PickBag");
}

ERROR_Type D02_GOtoPickBagRun() {  //START THE CYCLE
  // go up with the arm0 to pick the bag (should be at 80 go to 20) && turn the arm up to point to the bag
  if (D02SS.counter == 0 && counterACK.GetSliderState(0) == ACK_standby && counterACK.GetSliderState(1) == ACK_standby && timerZero.Check(Period_default * 1000)) {
    Serial.print("State 1, step");  //Turn Theta , Up A0 Y=0
    D02_ActivationServos();
    Serial.println(D02SS.counter);
    D02.arm0.TurnTheta(2065);  //servo correction*************2065-98
    D02.arm0.MoveX(20);//estba en 40
    delay(1);
    D02.arm0.MoveY(1);
    timerZero.reset();
    D02SS.counter = 1;
  }
  // when the arm0 it's up -> turn on the vacuum and up to bag position
  else if (D02SS.counter == 1 && counterACK.GetSliderState(0) == ACK_standby && counterACK.GetSliderState(1) == ACK_standby) {
    Serial.print("State 1, step");  // Vacuum On && Up A0
    Serial.println(D02SS.counter);
    D02.arm0.TurnOnVac(0);
    //D02.arm0.GoHome(false);
    D02.arm0.MoveX(5);
    timerZero.reset();
    D02SS.counter = 2;
    // when the arm0 has picked the bag -> get down the bag a bit
  } else if (D02SS.counter == 2 && counterACK.GetSliderState(0) == ACK_standby && timerZero.Check(Period_default * 5000)) {
    Serial.print("State 1, step");  // Check for Vacuum
    Serial.println(D02SS.counter);
    timerZero.reset();
    if (1 or D02.arm0.IsVacSense(0)) {  // If Bag is picked PASS
      D02.arm0.MoveX(70);
      D02SS.counter = 3;
    } else {  // If Bag is not picked
      D02SS.Counter_Fail(COUNTER_ERROR, 0);
    }

  }
  //wait until position
  else if (D02SS.counter == 3 && counterACK.GetSliderState(0) == ACK_standby && timerZero.Check(8000)) {  // Original Time 8000
    Serial.print("State 1, step");
    Serial.println(D02SS.counter);
    if (1 or D02.arm0.IsVacSense(0)) {
      D02SS.counter = 5;
      timerZero.reset();
    } else {
      D02SS.Counter_Fail(COUNTER_ERROR, 3);
    }

  }
  // when the arm0 it's down -> get down to bottom & turn the arm && move left to the platform
  else if (D02SS.counter == 5 && counterACK.GetSliderState(0) == ACK_standby && counterACK.GetSliderState(1) == ACK_standby) {
    Serial.print("State 1, step");
    Serial.println(D02SS.counter);
    if (1 or D02.arm0.IsVacSense(0)) {
      D02.arm0.MoveX(88);
      delay(1);
      D02.arm0.MoveY(270);
      D02.arm0.TurnTheta(3118);//*************3118-38
      D02.arm1.TurnTheta(2075);
      timerZero.reset();
      D02SS.counter = 6;
    } else {
      D02SS.Counter_Fail(COUNTER_ERROR, 5);
    }

  } else if (D02SS.counter == 6 && counterACK.GetSliderState(0) == ACK_standby && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(2) == ACK_standby && timerZero.Check(5000)) {  //Original 5000 //Should got servo time
    if (1 or D02.arm0.IsVacSense(0)) {
      D02.arm1.MoveX(5);
      timerZero.reset();
      D02SS.SetNextState(D02_GoToPatform);
    } else {
      D02SS.Counter_Fail(COUNTER_ERROR, 6);
    }
  } else if (D02SS.counter == COUNTER_ERROR) {
    D02SS.SetNextState(D02_SecureMode);
  } else {
    // JUST PASS
  }
}



ERROR_Type D02_GOtoPickBagEnd() {
  Serial.println("Leaving PickBag");
}

// Function prototypes for D02_GoToPatform
ERROR_Type D02_GoToPatformStart() {
  Serial.println("D02Starting GoToPlatform");

  // Turn the two arms to fit the end effector together, also turn vacuum 1 ON
  timerZero.reset();
  D02SS.Counter_Reset(0);
}

ERROR_Type D02_GoToPatformRun() {

  if (D02SS.counter == 0 && counterACK.GetSliderState(0) == ACK_standby && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(2) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby) {
    if (D02.arm0.IsVacSense(0) || true) {
      Serial.print("State 2, step");
      Serial.println(D02SS.counter);
      D02.arm0.TurnTheta(3118);//*****************-38
      D02.arm0.MoveX(92);
      delay(1);
     // D02.arm0.MoveY(380);
      delay(1);
      D02.arm1.MoveX(15);
      delay(1);
      D02.arm1.MoveY(0);
      D02.arm1.TurnOnVac(0);
      D02SS.counter = 1;
    } else {
      D02SS.Counter_Fail(COUNTER_ERROR, 0);
    }
  } else if (D02SS.counter == 1 && counterACK.GetSliderState(0) == ACK_standby && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(2) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby) {  // Original 5000 // Could be nothing
    if (1 or (!D02.arm0.IsVacSense(0) && !D02.arm1.IsVacSense(0))) {                                                                                                                                                                   //Both Fails
      D02SS.Counter_Fail(COUNTER_ERROR, COUNTER_ERROR);
    } else if (1 or (!D02.arm0.IsVacSense(0) || !D02.arm1.IsVacSense(0))) {  // Just One Fail
      D02SS.Counter_Fail(COUNTER_ERROR, 0);
    } else {  // nothing fails -- >> Continue
      D02SS.counter = 2;
      //D02SS.SetNextState(D02_OpenBag);
    }
  } else if (D02SS.counter == 2) {
    //D02.arm0.TurnOffVac(0);
    D02.arm0.TurnTheta(3118);//*********-38
    timerZero.reset();
    D02SS.counter = 3;
  } else if (D02SS.counter == 3 && timerZero.Check(500)) {
    D02.arm0.TurnOnVac(0);
    D02.arm1.MoveY(0);
    delay(1);
    D02.arm0.MoveY(270);

    D02SS.SetNextState(D02_OpenBag);
  } else if (D02SS.counter == COUNTER_ERROR) {
    D02SS.SetNextState(D02_SecureMode);
  } else {
    // JUST PASS
  }
}
ERROR_Type D02_GoToPatformEnd() {
  Serial.println("Leaving GoToPlatForm");
}

// Function prototypes for D02_OpenBag
ERROR_Type D02_OpenBagStart() {
  Serial.println("D02Starting OpenBag");
  timerZero.reset();
  D02SS.Counter_Reset(0);

  // Open the bag to be filled
}
ERROR_Type D02_OpenBagRun() {

  if (D02SS.counter == 0) {
    if (!D02.arm0.IsVacSense(0) && !D02.arm1.IsVacSense(0)) {  //Both Fails
      Serial.println("BothVacuumFAILS");
      D02SS.Counter_Fail(COUNTER_ERROR, COUNTER_ERROR);
    } else if (!D02.arm0.IsVacSense(0) || !D02.arm1.IsVacSense(0)) {  // Just One Fail
      D02SS.Counter_Fail(COUNTER_ERROR, 0);
      Serial.println("OneVacuumFAILS");
    } else {  // nothing fails
      Serial.print("State 3, step");
      Serial.println(D02SS.counter);


      if (D02SS.RetriesCount == 0) {
        D02.arm0.MoveY(130);     //was 160
       

        delay(1);
        D02.arm1.MoveY(95);

      } else {
        D02.arm0.MoveY(75);
        delay(1);
        D02.arm1.MoveY(95);
      }
      timerZero.reset();
      D02SS.counter = 1;
    }

  } else if (D02SS.counter == 1 && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby && timerZero.Check(Period_servo * 1000)) {
    Serial.print("State 3, step");
    Serial.println(D02SS.counter);
    if (!D02.arm0.IsVacSense(0) && !D02.arm1.IsVacSense(0)) {  //Both Fails
      D02SS.Counter_Fail(COUNTER_ERROR, COUNTER_ERROR);
    } else if (!D02.arm0.IsVacSense(0) || !D02.arm1.IsVacSense(0)) {  // Just One Fail
      //D02SS.SetNextState(D02_GoToPatform);
      //D02SS.counter = 5;
      D02SS.Counter_Fail(COUNTER_ERROR, 5);
    } else {
      //WAIT UNTIL GRIND EVENT
      D02.arm0.TurnTheta(3118);  //was 38
      D01.Selector.OpenBeansDoor();
      timerZero.reset();
      D02.foldBag.OpenGrip();
      D02.status = STATUS_D02_WAITNG_GRIND;
      D02SS.counter = 2;
    }

  } else if (D02SS.counter == 2 && timerZero.Check(Period_OpenGrip * 1000)) {  // Original 5000 // Should Wait OpenGripTime

    if (D01.status == STATUS_D01_FINISHED) {
      Serial.print("State 3, step");
      Serial.println(D02SS.counter);

      D02.arm0.MoveY(160);
      delay(1);
      D02.arm1.MoveY(15);
      D02SS.counter = 25;
      D02.aux_counter = 0;
      timerZero.reset();
    }
  }
  //shake bag to displace beans or powder
  /*
  else if (D02SS.counter == 25 && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(2) == ACK_standby && timerZero.Check(Period_OpenGrip * 1000)) {
    D02.arm1.TurnOffVac(0);
    delay(1);
    Serial.println("shake fase");
    D02.arm0.MoveY(0);
    timerZero.reset();
    D02SS.counter = 26;
  }
     else if (D02SS.counter == 26 && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(2) == ACK_standby && timerZero.Check(400)) {
    D02.arm1.TurnOffVac(0);
    delay(1);
    Serial.println("shake fase2");
    D02.arm0.MoveY(0);
      D02.arm0.TurnTheta(4102);//***********-50
    D02.aux_counter++;
    timerZero.reset();
    D02SS.counter = 27;

  }
  */

   else if (D02SS.counter == 27 && counterACK.GetSliderState(1) == ACK_standby && timerZero.Check(300)) {
    D02.arm0.TurnTheta(2461);//¿**************30
    timerZero.reset();
    if (D02.aux_counter < 2) {
      D02SS.counter = 26;
    } else {
      D02SS.counter = 28;
      D02.aux_counter = 0;
    }
  }
    /*
    else if (D02SS.counter == 28 && counterACK.GetSliderState(1) == ACK_standby && timerZero.Check(1000)) {
    Serial.print("shakeend");
    D02.arm0.TurnTheta(3118);
    D02.arm1.TurnOnVac(0);
    D02.arm0.MoveY(180);
    if (!D02.arm1.IsVacSense(0)){
          timerZero.reset();
    D02SS.counter = 3;
    }
    }*/

  else if (D02SS.counter == 3 && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(2) == ACK_standby) {  // Original 1500 // Could Be Bothing
    D02.arm0.MoveY(195);
    D02.arm0.TurnTheta(3118);
    D02.arm0.TurnOffVac(0);
    D02SS.counter = 4;
    timerZero.reset();
  }

  else if (D02SS.counter == 4 && timerZero.Check(Period_servo * 1500)) {  // Original 1500 // Could Be Bothing
    D02.foldBag.ExtendPiston();
    D02SS.SetNextState(D02_Weight);
  }


  else if (D02SS.counter == 5) {
    D02.arm0.TurnTheta(3118);
    if (!D02.arm1.IsVacSense(0)) {
      D02.arm1.TurnOffVac(0);
    } else if (!D02.arm0.IsVacSense(0)) {
      D02.arm0.TurnOffVac(0);
    }
    D02SS.counter = 7;
    timerZero.reset();
  } else if (D02SS.counter == 6 && timerZero.Check(1000)) {  // Original 500
    D02.arm0.TurnOnVac(0);
    D02.arm1.TurnOnVac(0);
    D02SS.counter = 8;
  } else if (D02SS.counter == 7 && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby) {  // Original 1500 // Could Be Bothing
    D02.arm0.MoveY(270);
    delay(1);
    D02.arm1.MoveY(0);
    D02.arm0.TurnOnVac(0);
    D02.arm1.TurnOnVac(0);
    D02SS.counter = 6;
  } else if (D02SS.counter == 8 && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby) {  // Original 1500 // Could Be Bothing
    D02SS.counter = 0;
  }

  else if (D02SS.counter == COUNTER_ERROR && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(2) == ACK_standby && timerZero.Check(1500)) {  // Original 1500 // Could Be Bothing
    Serial.println("COUNTER ERROR");
    D02SS.counter = 1;
  } else {
    // JUST PASS
  }
}
ERROR_Type D02_OpenBagEnd() {
}


// Function prototypes for D02_Weight
ERROR_Type D02_WeightStart() {
  Serial.println("D02Starting WeightEvent");
  D02SS.Counter_Reset(0);
  timerZero.reset();
}
ERROR_Type D02_WeightRun() {
  if (D02SS.counter == 0 && timerZero.Check(5000)) {  // Original 3000 // Should Be Something (Maybe 1000)
    D01.Selector.CloseBeansDoor();
    Serial.print("State 4, step");
    Serial.println(D02SS.counter);
    D02.arm0.TurnOffVac(0);
    D02.arm1.TurnOffVac(0);
    D02.arm1.TurnOffVac(1);
    D02.arm0.MoveY(50);
    D02.arm1.MoveY(90);

    timerZero.reset();
    D02.status = STATUS_D02_WAITNG_WPC;
    Serial.println("#WP");  // send WPC start
    D02SS.counter = 1;
  } else if (D02SS.counter == 1 && timerZero.Check(0)) {  // Original 1000 // Not Used
    Serial.print("State 4, step");
    Serial.println(D02SS.counter);
    // SEND TO PRINTER
    Serial.println("Send Command start WPC");
    D02SS.counter = 2;
  } else if (D02SS.counter == 2) {
    Serial.print("State 4, step");
    Serial.println(D02SS.counter);
    //D02.arm0.MoveX(80);
    delay(1);
    //D02.arm0.MoveY(150);
    timerZero.reset();
    //Wait until printer it's done
    D02SS.counter = 3;
  } else if (D02SS.counter == 3 && counterACK.GetSliderState(1) == ACK_standby && timerZero.Check(Period_default * 1000) && D02.status == STATUS_D02_WAITNG_WPC) {  // WAIT UNTIL WPC IT's DONE
    Serial.print("State 4, step");
    Serial.println(D02SS.counter);
    //D02.arm0.MoveX(60);
    D02SS.SetNextState(D02_FoldBag);
  } else {
    // JUST PASS
  }
}
ERROR_Type D02_WeightEnd() {}

// Function prototypes for D02_FoldBag
ERROR_Type D02_FoldBagStart() {
  Serial.println("D02Starting Fold Bag");
  D02SS.Counter_Reset(0);
  timerZero.reset();
}
ERROR_Type D02_FoldBagRun() {

  if (D02SS.counter == 0) {
    Serial.print("State 5, step");
    Serial.println(D02SS.counter);
    D02.arm0.TurnTheta(3118);
    D02.arm0.MoveY(230);
    D02.arm1.MoveY(90);

    //D02.arm0.MoveX(5);
    timerZero.reset();
    D02SS.counter = 1;
  }

  if (D02SS.counter == 1 && counterACK.GetSliderState(0) == ACK_standby && counterACK.GetSliderState(2) == ACK_standby && timerZero.Check(5000)) {  // Original 5000 // Could Be Nothing
    Serial.print("State 5, step");
    Serial.println(D02SS.counter);

    D02.arm1.MoveY(90);
    D02.arm1.TurnTheta(989);
    D02.foldBag.CloseGrip();
    timerZero.reset();
    D02SS.counter = 2;
  } else if (D02SS.counter == 2 && timerZero.Check(Period_OpenGrip * 1000)) {  // Original 2000 // Should Be CloseGrip Time
    Serial.print("State 5, step");
    Serial.println(D02SS.counter);
    D02.foldBag.MoveJoint(false, 10);
    timerZero.reset();
    D02SS.counter = 3;
  } else if (D02SS.counter == 3 && timerZero.Check(Period_servo * 2000)) {  // Original 10e3 // Could be ServoTime
    Serial.print("State 5, step");
    Serial.println(D02SS.counter);
    D02.arm0.MoveX(60);
    timerZero.reset();
    D02SS.counter = 4;
  } else if (D02SS.counter == 4) {
    Serial.print("State 5, step");
    Serial.println(D02SS.counter);
    D02SS.SetNextState(D02_PickTicket);
  } else {
    // JUST PASS
  }
}
ERROR_Type D02_FoldBagEnd() {}

// Function prototypes for D02_PickTicket
ERROR_Type D02_PickTicketStart() {
  Serial.println("D02 Starting Pick Ticket");
  D02SS.Counter_Reset(0);
  timerZero.reset();
}
ERROR_Type D02_PickTicketRun() {
  if (D02SS.counter == 0 && counterACK.GetSliderState(2) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby) {
    Serial.print("Counter: ");
    Serial.println(D02SS.counter);
    D02.arm1.TurnTheta(42);
    D02.arm1.MoveX(40);
    delay(1);
    D02.arm1.MoveY(95);
    //D02.arm1.TurnOnVac(1);
    D02SS.counter = 1;
    timerZero.reset();


  } else if (D02SS.counter == 1 && counterACK.GetSliderState(2) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby && timerZero.Check(Period_servo * 1000)) {  // Original 1500 // Should Be ServoTime
    D02.arm1.TurnOnVac(1);
    Serial.println(D02SS.counter);
    D02SS.counter = 2;
    timerZero.reset();

  } else if (D02SS.counter == 2 && counterACK.GetSliderState(2) == ACK_standby && timerZero.Check(500)) {

    if (D02.arm1.IsVacSense(1)) {
      //D02.arm1.MoveY(95);
      //delay(10);
      D02SS.counter = 9;
      D02.arm1.MoveX(30);
      timerZero.reset();
    } else {
      D02SS.Counter_Fail(COUNTER_ERROR, 7);
    }

  } else if (D02SS.counter == 9 && counterACK.GetSliderState(2) == ACK_standby && timerZero.Check(1000)) {

    if (D02.arm1.IsVacSense(1)) {
      D02.arm1.MoveY(95);
      delay(100);
      D02SS.counter = 3;
      D02.arm1.MoveX(1);
      timerZero.reset();
    } else {
      D02SS.Counter_Fail(COUNTER_ERROR, 7);
    }

  } else if (D02SS.counter == 3 && counterACK.GetSliderState(2) == ACK_standby && timerZero.Check(Period_default * 1000)) {  // Original 5000 // Could Be Nothing
    if (D02.arm1.IsVacSense(1)) {
      D02.arm1.TurnTheta(110);
      timerZero.reset();
      Serial.println(D02SS.counter);
      D02SS.counter = 4;
    } else {
      D02SS.Counter_Fail(COUNTER_ERROR, 7);
    }

  } else if (D02SS.counter == 4 && timerZero.Check(Period_servo * 1000)) {  // Original 1000 // Should Be ServoTime
    D02.arm1.MoveY(0);
    delay(1);
    D02.arm0.MoveX(92);
    Serial.println(D02SS.counter);
    timerZero.reset();
    D02SS.counter = 5;
  } else if (D02SS.counter == 5 && counterACK.GetSliderState(3) == ACK_standby && counterACK.GetSliderState(0) == ACK_standby) {  // Original 2000 // Could Be Nothing
    D02.arm0.TurnTheta(4659);//******************
    D02.arm0.MoveY(260);
    timerZero.reset();
    Serial.println(D02SS.counter);
    D02SS.counter = 6;
  } else if (D02SS.counter == 6 && counterACK.GetSliderState(1) == ACK_standby && timerZero.Check(Period_servo * 1000)) {  // Original 2000 // Should Be ServoTime
    timerZero.reset();
    Serial.println(D02SS.counter);
    D02SS.SetNextState(D02_PlaceTicket);


  } else if (D02SS.counter == 7 && counterACK.GetSliderState(2) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby) {  // Original 2000 // Could Be Nothing
    timerZero.reset();
    D02.arm1.MoveX(30);
    Serial.println(D02SS.counter);
    D02SS.counter = 8;
  } else if (D02SS.counter == 8 && counterACK.GetSliderState(2) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby && timerZero.Check(2000)) {  // Original 2000 // Could Be Nothing
    timerZero.reset();
    Serial.println(D02SS.counter);
    D02SS.counter = 0;
  }


  else if (D02SS.counter == COUNTER_ERROR) {
    timerZero.reset();
    D02SS.SetNextState(D02_SecureMode);
  }
}
ERROR_Type D02_PickTicketEnd() {}

// Function prototypes for D02_PlaceTicket
ERROR_Type D02_PlaceTicketStart() {
  Serial.println("D02Starting Place Ticket");
  D02SS.Counter_Reset(0);
  timerZero.reset();
}
ERROR_Type D02_PlaceTicketRun() {
  if (D02SS.counter == 0) {  // Original 3000 // Could Be Nothing
    Serial.println(D02SS.counter);
    D02.foldBag.MoveJoint(false, 40);
    D02.arm1.MoveY(40);
    D02.arm1.TurnOffVac(1);
    delay(1);
    D02.arm0.MoveY(230);
    D02SS.counter = 1;
    timerZero.reset();
  } else if (D02SS.counter == 1 && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby && timerZero.Check(Period_servo * 1000)) {  // Original 1000 // Should be something
    D02.foldBag.MoveJoint(false, 90);
    D02.foldBag.OpenGrip();
    D02SS.counter = 2;
    timerZero.reset();
  } else if (D02SS.counter == 2 && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby && timerZero.Check(max(Period_servo * 1000, Period_OpenGrip * 1000))) {  // Original 1000 // Should be max(servoTime, openGripTime)
    D02.arm0.MoveY(260);
    delay(1);
    D02.arm1.MoveY(0);
    timerZero.reset();
    D02SS.counter = 3;
  } else if (D02SS.counter == 3 && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby) {  // Original 1000 // Should be max(servoTime, openGripTime)
    D02.arm0.MoveY(1);
    delay(1);
    D02.arm1.MoveY(90);
    D02SS.SetNextState(D02_PackaginFinish);
    timerZero.reset();
  } else {
    //JUST PASS!!
  }
}
ERROR_Type D02_PlaceTicketEnd() {}

// Function prototypes for D02_PackaginFinish
ERROR_Type D02_PackaginFinishStart() {
  Serial.println("D02Finish Full Cycle");
  D02.arm0.TurnOffVac(0);
  D02.arm1.TurnOffVac(0);
  D02.arm1.TurnOffVac(1);
  D02.arm0.TurnTheta(915);//*******60
  D02SS.Counter_Reset(0);

  timerZero.reset();
}
ERROR_Type D02_PackaginFinishRun() {
  if (D02SS.counter == 0 && timerZero.Check(Period_default * 1000)) {  // Original 1000 // Should Be Something
    Serial.println("Finish0");
    D02.foldBag.RetractPiston();
    D02.servicePlatform.ExtendPlatform();
    D02.servicePlatform.OpenDoor();
    // OPEN MAIN DOOR
    D02SS.counter = 1;
    timerZero.reset();
  } else if (D02SS.counter == 1 && timerZero.Check(Period_Serving * 1000)) {  // Original 5000 // SHOULD BE SERVING TIME
    timerZero.reset();
    Serial.println("Finish1");
    D02.servicePlatform.CloseDoor();
    D02SS.counter = 2;


  } else if (D02SS.counter == 2 && timerZero.Check(Period_Serving * 1000)) {  // Original 5000 // SHOULD BE SERVING TIME
    timerZero.reset();
    Serial.println("Finish2");
    if (D02.servicePlatform.GetServiceDoorState() || true) {
      D02SS.counter = 3;
    } else {
      D02SS.Counter_Fail(COUNTER_ERROR, 2);
    }
  } else if (D02SS.counter == 3 && timerZero.Check(Period_default * 1000)) {
    Serial.println("Finish3");
    long _value = 0;
    byte _param = 0;
    D02.servicePlatform.RetractPlatform();
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

    D01_DeactivationServos();
    D02_DeactivationServos();
    Serial.println("END OF CYCLE");
    //resetFunc();
    D02SS.counter = 2;
    D02SS.SetNextState(D02_Standby);
    D01SS.SetNextState(D01_Standby);
    D02.status = STATUS_D02_STANDBY;
    D01.status = STATUS_D01_STANDBY;
    Serial.println("#RS1");
  } else if (D02SS.counter == COUNTER_ERROR) {  // Original 5000 // SHOULD BE SERVING TIME
    timerZero.reset();
    Serial.println("FinishBAG FAILED");
    D02SS.SetNextState(D02_SecureMode);
  }
}
ERROR_Type D02_PackaginFinishEnd() {
}

// Function prototypes for D02_SecureMode
ERROR_Type D02_SecureModeStart() {
  Serial.println("#RS0");
  D01.Selector.CloseBeansDoor();
  D02SS.Counter_Reset(0);
  timerZero.reset();
}
ERROR_Type D02_SecureModeRun() {
  if (D02SS.counter == 0) {
    Serial.print("State Secure MODE");
    Serial.println(D02SS.counter);
    D02.arm0.MoveX(80);
    delay(1);
    D02.arm0.MoveY(10);
    delay(1);
    D02.arm1.MoveX(20);
    delay(1);
    D02.arm1.MoveY(90);
    delay(1);
    // Parking Position Servos
    D02.arm0.TurnTheta(915);
    D02.arm1.TurnTheta(2000);

    // Turn OFF Vacuum
    D02.arm0.TurnOffVac(0);
    D02.arm1.TurnOffVac(0);
    D02.arm1.TurnOffVac(1);

    //Parking Position Folder
    D02.foldBag.MoveJoint(false, 180);
    D02.foldBag.CloseGrip();
    D02.foldBag.RetractPiston();
    D02.servicePlatform.RetractPlatform();
    D01SS.SetNextState(D01_SecureMode);

    D02SS.counter = 1;
  }

  else if (D02SS.counter == 1 && counterACK.GetSliderState(0) == ACK_standby && counterACK.GetSliderState(1) == ACK_standby && counterACK.GetSliderState(3) == ACK_standby && counterACK.GetSliderState(2) == ACK_standby) {
    D01_DeactivationServos();
    D02_DeactivationServos();
    D02SS.counter = 2;
  } else if (D02SS.counter == 2) {
    D02.status = STATUS_D02_SECURE_MODE;
  }
}
ERROR_Type D02_SecureModeEnd() {
  D02.status = STATUS_D02_STANDBY;
}

// Function prototypes for D02_ServiceMode
ERROR_Type D02_ServiceModeStart() {
  D02.status = STATUS_D02_SECURE_MODE;
  D01SS.SetNextState(D01_SecureMode);
  D01.Selector.CloseBeansDoor();
  Serial.println("#RS0");
  Serial.println("Service MODE");
  D02SS.Counter_Reset(0);
  timerZero.reset();
}
ERROR_Type D02_ServiceModeRun() {
  unsigned grams_purge = 0;

  if (D02.servicePlatform.GetMainDoorState() && timerZero.Check(Period_default * 100)) {
    D02SS.counter++;
    timerZero.reset();
  }

  if (!D02.servicePlatform.GetMainDoorState()) {
    D02SS.counter = 0;
    D01.Selector.OpenByPassDoor();
    //D01.Selector.OpenBeansDoor();
  }

  if (D02SS.counter >= 10) {
    resetFunc();
  }


  if (D02.status == STATUS_D02_PURGE_MODE) {
    D01.Selector.OpenByPassDoor();
    //D01.Selector.OpenBeansDoor();
    grams_purge = GRAMS_PURGE_CYCLE * 5;

    if (grams_purge != 0) {

      switch (D01.mainOrder.grams[1]) {
        case 0:
          D01.E0.ExtrudeGrams(grams_purge);
          break;
        case 1:
          //D01.E1.ExtrudeGrams(grams_purge);
          break;
        case 2:
          D01.E2.ExtrudeGrams(grams_purge);
          break;
        case 3:
          D01.E3.ExtrudeGrams(grams_purge);
          break;
        case 4:
          //D01.E4.ExtrudeGrams(grams_purge);
          break;
        case 5:
          D01.E5.ExtrudeGrams(grams_purge);
          break;
        default:
          break;
      }
      D02.status = STATUS_D02_SERVICE_MODE;
      grams_purge = 0;
    }
  }

  if (D02.status == STATUS_D02_PURGED_MODE) {
    grams_purge = GRAMS_PURGE_CYCLE;
    D01.Selector.OpenByPassDoor();
    //D01.Selector.OpenBeansDoor();
    if (grams_purge != 0) {

      switch (D01.mainOrder.grams[1]) {
        case 0:
          D01.E0.ExtrudeGrams(grams_purge);
          break;
        case 1:
          //D01.E1.ExtrudeGrams(grams_purge);
          break;
        case 2:
          D01.E2.ExtrudeGrams(grams_purge);
          break;
        case 3:
          D01.E3.ExtrudeGrams(grams_purge);
          break;
        case 4:
          //D01.E4.ExtrudeGrams(grams_purge);
          break;
        case 5:
          D01.E5.ExtrudeGrams(grams_purge);
          break;
        default:
          break;
      }
      grams_purge = 0;
      D02.status = STATUS_D02_SERVICE_MODE;
    }
  }
}
ERROR_Type D02_ServiceModeEnd() {
  D02.status = STATUS_D02_STANDBY;
}

// D02_NoDefined,D02_Standby,D02_GOtoPickBag,D02_GoToPatform,D02_OpenBag,D02_Weight,D02_PrintTiket,D02_FoldBag,D02_PickTicket,D02_PlaceTicket,D02_ReleaseTicket,D02_PackaginFinish,
