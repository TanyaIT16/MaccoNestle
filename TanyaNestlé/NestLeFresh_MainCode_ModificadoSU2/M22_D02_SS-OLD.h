
void D02SS_Initialize() {
  // Attach each function to the state machine
  D02SS.AttachNewState(D02_NoDefined, &D02_NoDefinedRun, &D02_NoDefinedStart, &D02_NoDefinedEnd, 0, TIMEOUT);
  D02SS.AttachNewState(D02_Standby, &D02_StandbyRun, &D02_StandbyStart, &D02_StandbyEnd, 0, TIMEOUT);
  D02SS.AttachNewState(D02_GOtoPickBag, &D02_GOtoPickBagRun, &D02_GOtoPickBagStart, &D02_GOtoPickBagEnd, 0, TIMEOUT);
  D02SS.AttachNewState(D02_GoToPatform, &D02_GoToPatformRun, &D02_GoToPatformStart, &D02_GoToPatformEnd, 0, TIMEOUT);
  D02SS.AttachNewState(D02_OpenBag, &D02_OpenBagRun, &D02_OpenBagStart, &D02_OpenBagEnd, 0, TIMEOUT);
  D02SS.AttachNewState(D02_Weight, &D02_WeightRun, &D02_WeightStart, &D02_WeightEnd, 0, TIMEOUT);
  D02SS.AttachNewState(D02_PrintToket, &D02_PrintTiketRun, &D02_PrintTiketStart, &D02_PrintTiketEnd, 0, TIMEOUT);
  D02SS.AttachNewState(D02_FoldBag, &D02_FoldBagRun, &D02_FoldBagStart, &D02_FoldBagEnd, 0, TIMEOUT);
  D02SS.AttachNewState(D02_PickTicket, &D02_PickTicketRun, &D02_PickTicketStart, &D02_PickTicketEnd, 0, TIMEOUT);
  D02SS.AttachNewState(D02_PlaceTicket, &D02_PlaceTicketRun, &D02_PlaceTicketStart, &D02_PlaceTicketEnd, 0, TIMEOUT);
  D02SS.AttachNewState(D02_ReleaseTicket, &D02_ReleaseTicketRun, &D02_ReleaseTicketStart, &D02_ReleaseTicketEnd, 0, TIMEOUT);
  D02SS.AttachNewState(D02_PackaginFinish, &D02_PackaginFinishRun, &D02_PackaginFinishStart, &D02_PackaginFinishEnd, 0, TIMEOUT);
}


// Function prototypes for D02_NoDefined
ERROR_Type D02_NoDefinedStart() {}
ERROR_Type D02_NoDefinedRun() {}
ERROR_Type D02_NoDefinedEnd() {}

// Function prototypes for D02_Standby
ERROR_Type D02_StandbyStart() {
  Serial.println("Entering StandBy");
  __SetAllToStandByD02();

  //Parking Positions Arms
  D02.arm0.MoveX(80);
  D02.arm0.MoveY(10);

  D02.arm1.MoveX(10);
  D02.arm1.MoveY(50);

  // Parking Position Servos
  D02.arm0.TurnTheta(180);
  D02.arm1.TurnTheta(0);

  // Turn OFF Vacuum
  D02.arm0.TurnOffVac(0);
  D02.arm1.TurnOffVac(0);
  D02.arm1.TurnOffVac(1);

  //Parking Position Folder
  D02.foldBag.MoveJoint(false, 180);
  D02.foldBag.CloseGrip();
  D02.foldBag.ExtendPiston();
  D02.foldBag.RetractPiston();


  return NO_ERROR;
}
ERROR_Type D02_StandbyRun() {
  D02.foldBag.RetractPiston();
  if (D02.status == STATUS_D02_PACKAGING) {
    //Recieve Command to start Packaging
  }
  return NO_ERROR;
}
ERROR_Type D02_StandbyEnd() {
  Serial.println("Leaving StandBy");

  //Go to Pick Bag Arm0 Position (a bit Down)
  D02.arm0.MoveY(10);
  D02.arm0.MoveX(10);
  D02.arm0.TurnTheta(105);
  D02.arm0.TurnOffVac(false);

  return NO_ERROR;
}

// Function prototypes for D02_GOtoPickBag
ERROR_Type D02_GOtoPickBagStart() {
  counter = 0;
  timerZero.ResetTimer();
  Serial.println("Entering PickBag");
}
ERROR_Type D02_GOtoPickBagRun() {

  if (counter == 0) {
    if (timerZero.Check(3000)) {  // Arm0 to pick the bag
      Serial.println("...PickBag");

      // Turn On Vacuum 0 & Go Upper limit to pick the bag
      D02.arm0.TurnOnVac(0);
      D02.arm0.MoveX(10);

      timerZero.ResetTimer();
      counter++;
    }
  }
  if (counter == 1) {              // Position the bag above and rigth to the weight platform
    if (D02.arm0.IsVacSense(0)) {  // Turn On Vacuum Try to catch the bag
      Serial.println("Bag Picked");


      // Move X position of both arms to packaging position
      D02.arm0.MoveX(100);
      D02.arm1.MoveX(100);

      //Start to extend the folder mechanism
      D02.foldBag.MoveJoint(false, 175);
      D02.foldBag.OpenGrip();
      D02.foldBag.ExtendPiston();

      timerZero.ResetTimer();
      counter++;

    } else if (timerZero.Check(5000)) {  // If Vacuum Fails
      Serial.println("Vacuum Failed");
      //When vacuum FAILS, go to StandBy state
      D02SS.SetNextState(D02_Standby);
      timerZero.ResetTimer();
    }
  }

  if (counter == 2) {             //
    if (timerZero.Check(3500)) {  // When Vacuum its triggered (So bag it's picked)
      // Close the Arm0 to the center
      D02.arm0.MoveY(20);
      D02SS.SetNextState(D02_Standby);
      //D02SS.SetNextState(D02_GoToPatform);
    }
  }
}



ERROR_Type D02_GOtoPickBagEnd() {
  Serial.println("Leaving PickBag");
}

// Function prototypes for D02_GoToPatform
ERROR_Type D02_GoToPatformStart() {
  Serial.println("Starting GoToPlatform");

  // Turn the two arms to fit the end effector together, also turn vacuum 1 ON
  D02.arm0.TurnTheta(40);
  D02.arm1.TurnTheta(38);
  D02.arm1.TurnOnVac(1);

  timerZero.ResetTimer();
  counter = 0;
}

ERROR_Type D02_GoToPatformRun() {


  if (!D02.arm0.IsVacSense(0)) {  // Check If vacuum 0 still being triggered
    D02SS.SetNextState(D02_Standby);
    timerZero.ResetTimer();
  }

  if (counter == 0) {  // Fit the two arms together, so vacuum 1 could catch the other side of the bag
    if (timerZero.Check(2500)) {
      // Close the Arm 1 endeffector to the center
      D02.arm1.MoveY(0);

      timerZero.ResetTimer();
      counter++;
    }
  }

  if (counter == 1) {  // Close the Arm 0 to the center
    if (timerZero.Check(3000)) {

      //Collision between the two end effectors
      D02.arm0.MoveY(80);

      timerZero.ResetTimer();
      counter++;
    }
  }
  if (counter == 2) {  // Check Vacuum State
    Serial.print("Vacuum Measurement 1:");
    Serial.print(D02.arm1.IsVacSense(0));
    Serial.print("\t Vacuum Measurement 2:");
    Serial.println(D02.arm1.IsVacSense(1));
    if (D02.arm1.IsVacSense(0)) {

      // Check Vacuum 1 State
      D02SS.SetNextState(D02_OpenBag);
    }
  }
}
ERROR_Type D02_GoToPatformEnd() {
  Serial.println("Leaving GoToPlatForm");
}

// Function prototypes for D02_OpenBag
ERROR_Type D02_OpenBagStart() {
  Serial.println("Starting OpenBag");
  timerZero.ResetTimer();
  counter = 0;

  // Open the bag to be filled
  D02.arm0.MoveY(1800);
  D02.arm1.MoveY(900);
}
ERROR_Type D02_OpenBagRun() {

  if (counter == 0) {
    if (timerZero.Check(5000)) {  //CAUTION !!!! This Line Is ACK Grinder Command !!!!!!!!

      //Release Vacuum 1
      D02.arm1.TurnOffVac(1);
      // Move Arm1 to ticket printer position
      D02.arm1.MoveX(3100);
      D02.arm1.MoveY(1000);
      timerZero.ResetTimer();
      counter = 1;
    }
  }
  if (counter == 1) {
    // Send data to printer

    //Release Vacuum 0 (leave the bag alone)
    D02.arm0.TurnOffVac(0);
    D02.arm1.TurnOffVac(0);
    D02.arm0.MoveY(1000);
    D02.arm0.MoveX(2000);

    timerZero.ResetTimer();
    counter = 2;
  }
  if (counter == 2) {
    if (timerZero.Check(500)) {
      // Lift Arm0 to fold in proper height
      //D02.arm0.MoveX(2000);

      counter = 3;
      timerZero.ResetTimer();
    }
  }
  if (counter == 3) {  // Start Folding Process
    if (timerZero.Check(2500)) {
      // Close the folding grip
      D02.foldBag.CloseGrip();

      //Turn Arm1 out of folding space
      D02.arm1.TurnTheta(45);

      counter = 4;
      timerZero.ResetTimer();
      Serial.println("CloseGrip");
      D02SS.SetNextState(D02_Weight);
    }
  }
}
ERROR_Type D02_OpenBagEnd() {
  //
}

// Function prototypes for D02_Weight
ERROR_Type D02_WeightStart() {
  Serial.println("Starting WeightEvent");

  counter = 0;
  timerZero.ResetTimer();
}
ERROR_Type D02_WeightRun() {

  if (counter == 0) {  //Do Fold
    if (timerZero.Check(3000)) {
      // Turn The fork to fold the bag
      D02.foldBag.MoveJoint(true, 00);
      D02.foldBag.MoveJoint(false, 00);
      Serial.println("Fold");
      timerZero.ResetTimer();
      counter = 1;
    }
  }

  if (counter == 1) {             //Do Fold
    if (timerZero.Check(2000)) {  // Turn The fork to fold the bag
      D02.arm1.TurnTheta(105);
      timerZero.ResetTimer();
      Serial.println("TurnToTicket");
      timerZero.ResetTimer();
      counter = 2;
    }
  }
  if (counter == 2 && timerZero.Check(1000)) {
    Serial.println("PickTicket");
    D02.arm1.TurnOnVac(false);
    D02.arm1.MoveY(1100);
    D02.arm1.MoveX(5000);

    timerZero.ResetTimer();
    counter++;
  }

  if (counter == 3 && timerZero.Check(1000)) {
    D02.arm1.MoveX(6500);

    timerZero.ResetTimer();
    counter++;
  }

  if (counter == 4 && (D02.arm1.IsVacSense(false) || D02.arm1.IsVacSense(true))) {

    D02.arm1.MoveX(2000);
    D02.arm1.TurnTheta(108);

    timerZero.ResetTimer();
    counter++;
  }

  if (counter == 5 && timerZero.Check(1000)) {
    Serial.println("PutTicket");
    D02.arm1.MoveY(10);

    timerZero.ResetTimer();
    counter++;
  }

  if (counter == 6 && timerZero.Check(1000)) {
    D02.arm1.TurnOffVac(false);

    timerZero.ResetTimer();
    counter++;
  }
}
ERROR_Type D02_WeightEnd() {}

// Function prototypes for D02_PrintTiket
ERROR_Type D02_PrintTiketStart() {}
ERROR_Type D02_PrintTiketRun() {}
ERROR_Type D02_PrintTiketEnd() {}

// Function prototypes for D02_FoldBag
ERROR_Type D02_FoldBagStart() {}
ERROR_Type D02_FoldBagRun() {}
ERROR_Type D02_FoldBagEnd() {}

// Function prototypes for D02_PickTicket
ERROR_Type D02_PickTicketStart() {}
ERROR_Type D02_PickTicketRun() {}
ERROR_Type D02_PickTicketEnd() {}

// Function prototypes for D02_PlaceTicket
ERROR_Type D02_PlaceTicketStart() {}
ERROR_Type D02_PlaceTicketRun() {}
ERROR_Type D02_PlaceTicketEnd() {}

// Function prototypes for D02_ReleaseTicket
ERROR_Type D02_ReleaseTicketStart() {}
ERROR_Type D02_ReleaseTicketRun() {}
ERROR_Type D02_ReleaseTicketEnd() {}

// Function prototypes for D02_PackaginFinish
ERROR_Type D02_PackaginFinishStart() {}
ERROR_Type D02_PackaginFinishRun() {}
ERROR_Type D02_PackaginFinishEnd() {}

// D02_NoDefined,D02_Standby,D02_GOtoPickBag,D02_GoToPatform,D02_OpenBag,D02_Weight,D02_PrintTiket,D02_FoldBag,D02_PickTicket,D02_PlaceTicket,D02_ReleaseTicket,D02_PackaginFinish,
