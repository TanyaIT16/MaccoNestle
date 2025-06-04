#include "HardwareSerial.h"
/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 CANmannager.h
                                                  0.1.1

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  25/09/22
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
CANMannager: handle the CAN-Bus communication task (MCP2515 -> SPI )

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#pragma once
#ifndef _CANmannagger
#define _CANmannagger

#include "ID_Def.h"
#include <SPI.h>
#include "mcp2515.h"
#include "SWTimer.h"
#include "List.h"
#include "Position.h"
//#include "Parameters.h"

#define STANDAR_HEADER
// #define EXTENDED_HEADER
#define _subIDLen 2

/*
#define SUBIDMASKLONG 0b10000000
#define SUBIDMASKINT 0b01100000
#define SUBIDMASKISCHAR 0b00010000
#define SUBIDMASKBYTE 0b00001111

#define SUBIDSHIFTLONG 7
#define SUBIDSHIFTINT 5
#define SUBIDSHIFTISCHAR 4
#define SUBIDSHIFTBYTE 0
*/
#define AUXCOMMANDMASK 0x0F  // FIRST 4 bits [0-3] // 16 Types of msg
#define DATATYPEMASK 0x70    // 3BITS [4-6] // 8 Types of data
#define ACKREQUIREDMASK 0x80 // LAST bit [7] // ACK required bit
#define CANIDMASK 0x7F0

#define IDSENDERORDER 0
#define COMMANDORDER 1
#define PARAMORDER 2
#define DATAHEADERBITS 3


/// Select the CS pin of the SPI in the uC
#ifdef __AVR_ATmega2560__
#define _DEFAULT_SPI_PIN 53
#else
#define _DEFAULT_SPI_PIN 10
#endif

#define _DEFAULT_SPI_CLOCK 10000000

enum DataTypes
{                        // Enum to clasiffy the CAN-Data types
    BYTE = 0x00,         // 1
    CHAR = 0x01,         // 1
    INT = 0x02,          // 2
    UNSIGNED = 0x03,     // 2
    LONG = 0x04,         // 4
    UNSIGNEDLONG = 0x05, // 4
    FLOAT = 0x06,        // 4
    BOOL = 0x07,     // 8 // 1

    DATATYPE_NODEF = 0x08, // n-a // FORBIDEN
};           

enum CanModes
{ // Enum to clasiffy the CAN modes
  NORMAL_MODE,
  LISTENONLYMODE,
  LOOPBACKMODE,
  CONFIGMODE
};


enum CanMsg_Type
{
  MSG_ACK = 0x00,
  MSG_STATE ,
  MSG_ERROR,
  MSG_MODE,
  MSG_START,
  MSG_STOP,
  MSG_PARAMETER,
  MSG_GOHOME,

  MSG_NODEF = 0x10
};

struct CommandChain
{
  bool isACKreq = false;
  CanMsg_Type codeMSG = MSG_NODEF;
  DataTypes dataType = DATATYPE_NODEF;
};

struct PendingACK
{
  uint8_t deviceID;
  byte aux_char;
  SWTimer_Redux_Millis timer0;
};

struct SendingACK
{
  byte deviceID;
  byte aux_char;
};

class CANmannager
{ // Class CAN Mannager
private:
  // Private members for managing ACKs

  Simple_Linked_List<PendingACK> ackList;
  Simple_Linked_List<SendingACK> ackSendingList;

  unsigned canID;
  unsigned long ackTimeout; // Set an appropriate timeout value

  bool isMsgReady = false; // Flag to store if a MSG it's ready to send

  struct can_frame canMsg, canRcvMsg0, canRcvMsg1; // CanFrames to store the date 1 for send 2 for recieve
  MCP2515 mcp2515 = MCP2515::MCP2515(_DEFAULT_SPI_PIN, _DEFAULT_SPI_CLOCK); // Object of MCP2515 (can transiever) trought SPI

public:

  CANmannager(byte _id, CAN_SPEED _canSpeed, CanModes _mode, bool _isReset); // Constructor (INPUT: _ID, CANSPEED, MODE, IsResetFLAG)
  CANmannager(byte _id);

  // EVERY MSG HAS: Header (_IDreciever<<4) , DLC , _IDsender , AuxCommand , DataType , Data
  bool PrepareMsg(unsigned _canID, byte _ID, char _auxCommand, byte _data[], byte _dataLen = 1, bool isAckRequired = false); // Prepare a MSG of byte Array
  bool PrepareMsg(unsigned _canID, byte _ID, char _auxCommand, byte _data, bool isAckRequired = false);                      // Prepare a MSG of a byte
  bool PrepareMsg(unsigned _canID, byte _ID, char _auxCommand, int _data, bool isAckRequired = false);                       // Prepare a MSG of an INT
  bool PrepareMsg(unsigned _canID, byte _ID, char _auxCommand, long _data, bool isAckRequired = false);                      // Prepare a MSG of LONG
  bool PrepareMsg(unsigned _canID, byte _ID, char _auxCommand, float _data, bool isAckRequired = false);                     // Prepare a MSG of FLOAT
  bool PrepareMsg(unsigned _canID, byte _ID, char _auxCommand, char _data, bool isAckRequired = false);                      // Prepare a MSG of CHAR
  bool PrepareACKMsg(unsigned _canID, byte _ID, byte _answer);
  bool StoreValueInCanMsg(byte *_value, byte _size);
  bool StoreValueInCanMsg(byte *_value);
  bool SetHeaderMsg(byte _IDReciver, CanMsg_Type _msgCommand, DataTypes _typeData, bool _isAckReq, byte _parameter);

  // Method to extract an UNSIGNED value from a can_frame
  unsigned Msg2UNSIGNED(can_frame &_canMsg);

  // Method to extract a LONG value from a can_frame
  long Msg2LONG(can_frame &_canMsg);

  // Method to extract a BYTE value from a can_frame
  byte Msg2BYTE(can_frame &_canMsg);

  // Method to extract a BOOL value from a can_frame
  bool Msg2BOOL(can_frame &_canMsg);

  // Method to extract a FLOAT value from a can_frame
  float Msg2FLOAT(can_frame &_canMsg);

  MCP2515::ERROR SetFilters(byte _id);                 // Set the filters to accept: Own (ID<<4) + 0xXX0 ID + 0x7XX ID MSG's
  MCP2515::ERROR SendMsg();                            // Send a Prepared MSG If it's ready to send
  MCP2515::ERROR ReciveMsg(const MCP2515::RXBn _rxbn); // Recieve a MSG trought bufferX (INPUT: BufferX)
  MCP2515::ERROR ReciveMsg();                          // Recieve a MSG if it's available
  void ResetMsg(can_frame &_canMsg);                   // Reset a MSG frame to default (id FFFF, other = 0)
  void ReadLastMsg(bool _buff, can_frame &_canMsg);    // Read last MSG and copy to a CANFRAME (INPUT: BuferX, &CANFRAME)
  can_frame ReadLastMsg();                                  // Read Last MSG and print trought UART0 port
  unsigned CanGetID();                                 // Get the current ID of the CanMannager
  void CanSetID(byte _ID);
  void SetMode(CanModes _mode); // Set Can to a mode
  void SetMode();
  byte encodeCommand(CommandChain _commandChain);
  CommandChain decodeCommand(byte chain);
  void Data2Byte(int _value, can_frame &_dataFrame);   // Split INT data to byte[] and store in apropiate place ofa canFrame
  void Data2Byte(long _value, can_frame &_dataFrame);  // Split LONG data to byte[] and store in apropiate place ofa canFrame
  void Data2Byte(float _value, can_frame &_dataFrame); // Split FLOAT data to byte[] and store in apropiate place ofa canFrame

  int Byte2Int(can_frame &_dataFrame);     // Restore INT data from a canFrame apropiate MSG
  long Byte2Long(can_frame &_dataFrame);   // Restore LONG data from a canFrame apropiate MSG
  float Byte2Float(can_frame &_dataFrame); // Restore FLOAT data from a canFrame apropiate MSG
  char Byte2Char(can_frame &_dataFrame);   // Restore CHAR data from a canFrame apropiate MSG
  bool CheckISAckMsg(can_frame &_dataFrame);

private:
  bool __checkDLC(byte _n); // Check the DLC for a kind of MSG

  void __ResetMsg(unsigned _canID, byte _auxCommand); // Reset the Header of a MSG (DEPRECATED NOT USED!!!)

  void __splitData2Bytes(can_frame &_dataFrame, byte &_dlc, int _data); // Split data to byte[] (DEPRECATED NOT USED!!!)
  bool __CanSetID();                                                    // Method to Set and check an ID can (returns 1 if valid ID)
  ERROR_Type _AckMannage();
  bool _AckResponded(can_frame &_dataFrame);
  byte _bitShiftMask(byte _mask);
};


template <typename T>
bool SetNormalMsg(CANmannager *_can, byte _IDReciver, CanMsg_Type _msgCommand, DataTypes _typeData, bool _isAckReq, byte _parameter, T _value = 0)
{

  if (_can->SetHeaderMsg(_IDReciver, _msgCommand, _typeData, _isAckReq, _parameter))
  {
   // Serial.print("parameter:");
  //  Serial.println(_parameter);
  //    Serial.print("Value :");
 // Serial.print(_value,HEX);
    byte * valuePtr =(byte *) & _value;
    if (_can->StoreValueInCanMsg(valuePtr,sizeof(_value)))
    _can->SendMsg();
    return true;
  }
  return false;
}


template<typename T>
void SendMSG(CANmannager *_can, byte _IDReciver, CanMsg_Type _msgCommand, DataTypes _typeData, bool _isAckReq, byte _parameter, T _value = 0) {

  SetNormalMsg(_can, _IDReciver, _msgCommand, _typeData, _isAckReq, _parameter, _value);
  byte MSG_Status = _can->SendMsg();
  if (MSG_Status != MCP2515::ERROR_OK) {
    Serial.print("Failed Sending MSG");
    Serial.println(MSG_Status);
  }
}

void SendStepsMSG(CANmannager *_can, byte _IDReciver, CanMsg_Type _msgCommand, DataTypes _typeData, bool _isAckReq, byte _parameter, StepsUnits _steps = StepsUnits(0,0));

bool SetStepMsg(CANmannager *_can, byte _IDReciver, CanMsg_Type _msgCommand, DataTypes _typeData, bool _isAckReq, byte _parameter, StepsUnits _steps = StepsUnits(0,0));

#endif


