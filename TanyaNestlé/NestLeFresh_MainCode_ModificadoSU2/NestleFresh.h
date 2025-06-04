/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 NESTLE FRESH.h
                                                  1.0.00a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  02/04/24
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


#ifndef __AVR_ATmega2560__
#define NANO
#endif

#define ASIG_PIN 2
#define BSIG_PIN 3
#define NSIG_PIN 4

#define DIR_PIN 6
#define ENA_PIN 5
#define STEP_PIN 7


#define ENDSWITCH_PIN 8

/////////////////////////////////////////
//

///////////////////////////////////////

#define CS_PIN 53
#define MISO_PIN 50
#define MOSI_PIN 51
#define SCLK_PIN 52

#define TIMEOUT 120000

#define PISTON_EXTRUDER_OPEN false
#define PISTON_EXTRUDER_CLOSE (!PISTON_EXTRUDER_OPEN)


/////////////////////////////////////////STATUSES/////////////////////////////////////////



enum SSD01Statuses {
  STATUS_D01_NO_DEF,
  STATUS_D01_STANDBY,
  STATUS_D01_EXTRUDING,
  STATUS_D01_WAITNG_SELECTOR,
  STATUS_D01_SELECTING,
  STATUS_D01_FINISHED,
  STATUS_D01_SECURE_MODE,

};

enum SSD02Statuses {
  STATUS_D02_NO_DEF,
  STATUS_D02_STANDBY,
  STATUS_D02_PACKAGING,
  STATUS_D02_WAITNG_GRIND,
  STATUS_D02_WAITNG_WPC,
  STATUS_D02_FINISHING,
  STATUS_D02_FINISHED,
  STATUS_D02_SECURE_MODE,
  STATUS_D02_SERVICE_MODE,
  STATUS_D02_PURGE_MODE,
  STATUS_D02_PURGED_MODE,
};


//
enum MainActions {
  ACTION_NO_DEF = '_',          // No action defined
  ACTION_RUN_LOOP = 'r',        // Run loop action
  ACTION_INITIALIZE = 'i',      // Initialize Action
  ACTION_MOVE_EXTRUDER = 'e',   // Move extruder action
  ACTION_MOVE_PISTON = 'p',     // Move piston action
  ACTION_MIXER = 'm',           // Mixer action
  ACTION_SELECTOR = 's',        // Selector action
  ACTION_GOHOME = 'h',          // GoHome action
  ACTION_GRINDER = 'g',         // Grinder action
  ACTION_SLIDER = 'l',          // Slider action
  ACTION_PACKAGINGSERVO = 'a',  // Packaging servo action
  ACTION_FOLDPISTONS = 'f',     // Fold pistons action
  ACTION_SERVICEDOOR = 'd',     // Service door action
  ACTION_VACUUM_VALVE = 'v',    // Vacuum valve action
  ACTION_COMPRESOR = 'c',       // Compressor action
  ACTION_KILLSELF = 'k',
  ACTION_ACTIVATE_SERVOS = 'z',
  ACTION_WPC = 'w',
  ACTION_DEACTIVATE_SERVOS = 'q',
  ACTION_RUN_PART = 'y',
};



/////////////////////////////////////////STATES/////////////////////////////////////////

enum D01States {

  D01_NoDefined = -1,
  D01_Standby,
  D01_Extrude,
  D01_GrindSetup,
  D01_DoMixing,
  D01_DoSelect,
  D01_DoGrind,
  D01_Finish,
  D01_SecureMode,
  D01_ServiceMode
};

enum D02States {

  D02_NoDefined = -1,
  D02_Init,            // S [SetAllToRest]  |R []  |E []  {StartPackagingEvent}
  D02_Standby,         // S [SetAllToRest]  |R []  |E []  {StartPackagingEvent}
  D02_GOtoPickBag,     // S [TurnONVac1 - Arm1ToPickUpBag]  |R []  |E [Arm2ToPlatform- TurnONVac2]  { IsVacuum1 OR TimeOut} [->]
  D02_GoToPatform,     // S [Arm1ToPlatform -]  |R []  |E []  {IsVacum2 OR TimeOut} [->]
  D02_OpenBag,         // S [Arm1OpenBag - Arm2OpenBag]  |R []  |E [PackagingBagInPlaceACK]  {TimeControlled} [->]
  D02_ReleaseBag,      // S [TurnOFFVacum1 - TurnOFFVacum2]  |R []  |E [Arm1GoHome - Arm2GoHome]  {!IsVacuum1 AND !IsVacuum2} [->]
  D02_Weight,          // S [WeigthEvent]  |R []  |E []  {WaitUntilWeightDeviceRespond}
  D02_PrintToket,      // S [TicketPrintEvent]  |R []  |E [FoldGotoFoldPos]  {TicketDeviceACK}
  D02_FoldBag,         // S [ExtendFoldPiston]  |R []  |E [DoFold]  {TimeToPiston}
  D02_PickTicket,      // S [Arm2GoToTiket- TurnONVac3]  |R []  |E []  {IsVacuum3 OR TimeOut}
  D02_PlaceTicket,     // S [Arm2GotoPlaceTicket]  |R []  |E []  {TimeControlled}
  D02_ReleaseTicket,   // S [AdvanceArm1toStickPlace]  |R []  |E [TurnOFFVacuum - RetractFoldPiston]  {TimeControlled}
  D02_PackaginFinish,  // S [PackagingFinishEvent]  |R []  |E []  {Pass} [0]
  D02_SecureMode,
  D02_ServiceMode

};









///////////////////////////////////////// HARDWARE /////////////////////////////////////////
struct M22MachineD01 {
  __ID ID;
  unsigned retryCounter = 0;
  MainOrder mainOrder;

  ExtrusorMannager E0;
  ExtrusorMannager E1;
  ExtrusorMannager E2;
  ExtrusorMannager E3;
  ExtrusorMannager E4;
  ExtrusorMannager E5;

  HBridge Piston0;
  HBridge Piston1;
  HBridge Piston2;
  HBridge Piston3;
  HBridge Piston4;
  HBridge Piston5;

  SelectorUnit Selector;

  SSD01Statuses status;

} D01;

struct M22MachineD02 {
  __ID ID;
  // CANmannager can =  CANmannager(ID_Master);
  unsigned retryCounter = 0;
  ArmUnit arm0;
  ArmUnit arm1;

  FoldUnit foldBag;

  ServicePlatformUnit servicePlatform;

  CompressorUnit compressor;

  //Variable

  byte status = STATUS_D02_NO_DEF;
  unsigned aux_counter = 0;

} D02;

////////////////////////////////////////Activation of Servos////////////////////////////////////////
ERROR_Type D01_ActivationServos();
ERROR_Type D02_ActivationServos();

///////////////////////////////////////Deactivation of Servos For Idle Pos //////////////////////////
ERROR_Type D01_DeactivationServos();
ERROR_Type D02_DeactivationServos();

/////////////////////////////////////////State Machine D01 /////////////////////////////////////////

ERROR_Type D01_MainState();

// Function prototypes for D01_NoDefined
ERROR_Type D01_NoDefinedStart();
ERROR_Type D01_NoDefinedRun();
ERROR_Type D01_NoDefinedEnd();

// Function prototypes for D01_StandBy
ERROR_Type D01_StandbyStart();
ERROR_Type D01_StandbyRun();
ERROR_Type D01_StandbyEnd();
// Function prototypes for D01_Extrude
ERROR_Type D01_ExtrudeStart();
ERROR_Type D01_ExtrudeRun();
ERROR_Type D01_ExtrudeEnd();
// Function prototypes for D01_GrindSetup
ERROR_Type D01_GrindSetupStart();
ERROR_Type D01_GrindSetupRun();
ERROR_Type D01_GrindSetupEnd();
// Function prototypes for D01_NoMixing
ERROR_Type D01_DoMixingStart();
ERROR_Type D01_DoMixingRun();
ERROR_Type D01_DoMixingEnd();
// Function prototypes for D01_Select
ERROR_Type D01_DoSelectStart();
ERROR_Type D01_DoSelectRun();
ERROR_Type D01_DoSelectEnd();
// Function prototypes for D01_Grind
ERROR_Type D01_DoGrindStart();
ERROR_Type D01_DoGrindRun();
ERROR_Type D01_DoGrindEnd();
// Function prototypes for D01_Finish
ERROR_Type D01_FinishStart();
ERROR_Type D01_FinishRun();
ERROR_Type D01_FinishEnd();

// Function prototypes for D01_SecureMode
ERROR_Type D01_SecureModeStart();
ERROR_Type D01_SecureModeRun();
ERROR_Type D01_SecureModeEnd();
// Function prototypes for D01_ServiceMode
ERROR_Type D01_ServiceModeStart();
ERROR_Type D01_ServiceModeRun();
ERROR_Type D01_ServiceModeEnd();


/////////////////////////////////////////State Machine D02 /////////////////////////////////////////
ERROR_Type D02_MainState();

// Function prototypes for D02_NoDefined
ERROR_Type D02_NoDefinedStart();
ERROR_Type D02_NoDefinedRun();
ERROR_Type D02_NoDefinedEnd();

// Function prototypes for D02_Standby
ERROR_Type D02_InitStart();
ERROR_Type D02_InitRun();
ERROR_Type D02_InitEnd();

// Function prototypes for D02_Standby
ERROR_Type D02_StandbyStart();
ERROR_Type D02_StandbyRun();
ERROR_Type D02_StandbyEnd();

// Function prototypes for D02_GOtoPickBag
ERROR_Type D02_GOtoPickBagStart();
ERROR_Type D02_GOtoPickBagRun();
ERROR_Type D02_GOtoPickBagEnd();

// Function prototypes for D02_GoToPatform
ERROR_Type D02_GoToPatformStart();
ERROR_Type D02_GoToPatformRun();
ERROR_Type D02_GoToPatformEnd();

// Function prototypes for D02_OpenBag
ERROR_Type D02_OpenBagStart();
ERROR_Type D02_OpenBagRun();
ERROR_Type D02_OpenBagEnd();

// Function prototypes for D02_Weight
ERROR_Type D02_WeightStart();
ERROR_Type D02_WeightRun();
ERROR_Type D02_WeightEnd();

// Function prototypes for D02_PrintTiket
ERROR_Type D02_PrintTiketStart();
ERROR_Type D02_PrintTiketRun();
ERROR_Type D02_PrintTiketEnd();

// Function prototypes for D02_FoldBag
ERROR_Type D02_FoldBagStart();
ERROR_Type D02_FoldBagRun();
ERROR_Type D02_FoldBagEnd();

// Function prototypes for D02_PickTicket
ERROR_Type D02_PickTicketStart();
ERROR_Type D02_PickTicketRun();
ERROR_Type D02_PickTicketEnd();

// Function prototypes for D02_PlaceTicket
ERROR_Type D02_PlaceTicketStart();
ERROR_Type D02_PlaceTicketRun();
ERROR_Type D02_PlaceTicketEnd();

// Function prototypes for D02_ReleaseTicket
ERROR_Type D02_ReleaseTicketStart();
ERROR_Type D02_ReleaseTicketRun();
ERROR_Type D02_ReleaseTicketEnd();

// Function prototypes for D02_PackaginFinish
ERROR_Type D02_PackaginFinishStart();
ERROR_Type D02_PackaginFinishRun();
ERROR_Type D02_PackaginFinishEnd();

// Function prototypes for D02_PackaginSecureMode
ERROR_Type D02_SecureModeStart();
ERROR_Type D02_SecureModeRun();
ERROR_Type D02_SecureModeEnd();

// Function prototypes for D02_PackaginServiceMode
ERROR_Type D02_ServiceModeStart();
ERROR_Type D02_ServiceModeRun();
ERROR_Type D02_ServiceModeEnd();

//This function continuously checks for incoming serial data. If it finds a character that matches one of the specified commands ('s', 'h', 'v', 'p', 'm', 'd'), it reads the numeric value following that character and stores it in the parameterValue variable. You can then process the command and its associated value in the switch-case block based on the first character.
