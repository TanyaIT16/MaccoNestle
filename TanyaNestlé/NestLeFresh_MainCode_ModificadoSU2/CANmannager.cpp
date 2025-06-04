/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 CANmannager.cpp
                                                  0.1.1
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#include "HardwareSerial.h"
#ifndef _CANmannagger
#include "CANmannager.h"
#include "mcp2515.h"

//#ifdef __AVR_ATmega2560__
//MCP2515 CANmannager::mcp2515(53, _DEFAULT_SPI_CLOCK);
//#else
/// CAUTION !!!!         first parameter must be (10 for NANO128p) , (53 for MEGA2556)
//#endif

/*
// Can Id's -> Should be the _ID of the reciver << 4 places [0x10 -> 07F0] leaving 16 can Id's for broadcast msg HiPriority [0x00 -> 0x0F] and 15 LoPriority [0x7F1->0x7FF] Free of use
// _ID -> Should be the _ID of the sender
//CAN CUSTOM MSG -->>   [ID0] [ID1] [_ID] [isAck|auxCommand] [D2] ... [D7]
*/

// Constructor (INPUT: _ID, CANSPEED, MODE, IsResetFLAG)
CANmannager::CANmannager(byte _id, CAN_SPEED _canSpeed, CanModes _mode, bool _isReset) {
  // mcp2515 = _mcp2515;
  // CANmannager::mcp2515 = MCP2515::MCP2515(_DEFAULT_SPI_PIN, _DEFAULT_SPI_CLOCK);
  if (_isReset) {
    this->mcp2515.reset();
  }
  Serial.print(sizeof(*this));
  this->mcp2515.setBitrate(_canSpeed, MCP_8MHZ);
  this->canRcvMsg0.can_id = 0xFFFF;
  this->canRcvMsg1.can_id = 0xFFFF;
  CanSetID(_id);
  SetMode(_mode);
  ackList.clearList();
  ackSendingList.clearList();
  this->ackTimeout = 10000;
  CanSetID(_id);
}

CANmannager::CANmannager(byte _id) {
  // mcp2515 = _mcp2515;
  // this->mcp2515 = MCP2515::MCP2515(_DEFAULT_SPI_PIN, _DEFAULT_SPI_CLOCK);
  CANmannager(_id, CAN_250KBPS, NORMAL_MODE, true);
}

// Reset a MSG frame to default (id FFFF, other = 0)
void CANmannager::ResetMsg(can_frame &_canMsg) {
  _canMsg.can_id = 0xFFFF;
  _canMsg.can_dlc = 0x00;
  for (byte i = 0; i < 8; i++) {
    _canMsg.data[i] = 0x00;
  }
}

bool CANmannager::PrepareACKMsg(unsigned _canID, byte _ID, byte _answer) {
  ResetMsg(this->canMsg);
  byte n = 0;
  this->canMsg.can_id = _canID;
  this->canMsg.data[0] = _ID;
  this->canMsg.data[1] = 0;  //(AUXCOMMANDMASK & MSG_ACK);
  this->canMsg.data[2] = _answer;
  n = 3;
  this->isMsgReady = CANmannager::__checkDLC(n);
  return this->isMsgReady;
}

// Set the filters to accept: Own (ID<<4) + 0xXX0 ID + 0x7XX ID MSG's
MCP2515::ERROR CANmannager::SetFilters(byte _id) {
  MCP2515::ERROR _err;
  _err = this->mcp2515.setConfigMode();
  _err = max(_err, this->mcp2515.setFilterMask(MCP2515::MASK0, false, 0x7FF));
  _err = max(_err, this->mcp2515.setFilter(MCP2515::RXF0, false, (_id << 4)));
  _err = max(_err, this->mcp2515.setFilter(MCP2515::RXF1, false, 0x600));
  _err = max(_err, this->mcp2515.setFilterMask(MCP2515::MASK1, false, 0x7F0));
  _err = max(_err, this->mcp2515.setFilter(MCP2515::RXF2, false, 0x000));
  _err = max(_err, this->mcp2515.setFilter(MCP2515::RXF3, false, 0x7F0));
  _err = max(_err, this->mcp2515.setFilter(MCP2515::RXF4, false, 0x001));
  _err = max(_err, this->mcp2515.setFilter(MCP2515::RXF5, false, 0x000));
  this->mcp2515.setNormalMode();
  return _err;
}

// Send a Prepared MSG If it's ready to send
MCP2515::ERROR CANmannager::SendMsg() {
  if (this->isMsgReady) {
    if (CheckISAckMsg(this->canMsg)) {
      PendingACK newAckQueue;
      newAckQueue.deviceID = (byte)(this->canMsg.can_id >> 4);
      newAckQueue.aux_char = (this->canMsg.data[1] & AUXCOMMANDMASK);
      newAckQueue.timer0.ResetTimer();
      ackList.insert(newAckQueue);
      Serial.println("NewACKpendingMSG");
    }
    MCP2515::ERROR _err = this->mcp2515.sendMessage(&this->canMsg);
    if (_err == MCP2515::ERROR_OK) {
      ResetMsg(this->canMsg);
    }
    return _err;
  } else
    return MCP2515::ERROR_FAIL;
}

// Recieve a MSG if it's available
MCP2515::ERROR CANmannager::ReciveMsg() {
  MCP2515::ERROR err = this->mcp2515.readMessage(&canRcvMsg0);
  _AckMannage();
  if (err == MCP2515::ERROR_OK) {

    if (CheckISAckMsg(this->canRcvMsg0)) {
      SendingACK newAckSendingQueue;
      newAckSendingQueue.deviceID = this->canRcvMsg0.data[0];
      newAckSendingQueue.aux_char = (canRcvMsg0.data[1] & AUXCOMMANDMASK);
      ackSendingList.insert(newAckSendingQueue);
      Serial.println("NewACKsendingMSG");
    }
    //Serial.println("Message recieved");
    _AckResponded(canRcvMsg0);
  }
  return err;
}

// Recieve a MSG trought bufferX (INPUT: BufferX)
MCP2515::ERROR CANmannager::ReciveMsg(const MCP2515::RXBn _rxbn) {

  if (_rxbn == MCP2515::RXB0)
    return this->mcp2515.readMessage(_rxbn, &canRcvMsg0);
  if (_rxbn == MCP2515::RXB1)
    return this->mcp2515.readMessage(_rxbn, &canRcvMsg1);
  else
    return MCP2515::ERROR_FAIL;
}

// Read last MSG and copy to a CANFRAME (INPUT: BuferX, &CANFRAME)
void CANmannager::ReadLastMsg(bool _buff, can_frame &_canMsg) {
  if (!_buff)
    _canMsg = this->canRcvMsg0;
  else
    _canMsg = this->canRcvMsg1;
}

// Read Last MSG and print trought UART0 port
can_frame CANmannager::ReadLastMsg() {
  if (false) {
    if (this->canRcvMsg0.can_id <= 0x7FF) {
      Serial.println("Can Msg:");
      Serial.print(" __ID Rcv : ");
      Serial.print(this->canRcvMsg0.can_id, HEX);
      Serial.print("\t __ID snd ");
      Serial.print(this->canRcvMsg0.data[IDSENDERORDER], HEX);
      Serial.print("\t CommandType : ");
      Serial.print(this->canRcvMsg0.data[COMMANDORDER], HEX);
      Serial.print("\t Parameter ");
      Serial.print(this->canRcvMsg0.data[PARAMORDER], HEX);
      Serial.print("\t Data: ");

      for (int i = PARAMORDER + 1; i < this->canRcvMsg0.can_dlc; i++) {  // print the data
        Serial.print(this->canRcvMsg0.data[i], HEX);
        Serial.print(" ");
      }
      Serial.println("");
      Serial.print("Msg Recived, with data type, ");
      CommandChain chainCommand;
      chainCommand = decodeCommand(this->canRcvMsg0.data[COMMANDORDER]);
      DataTypes _dataType = (DataTypes)chainCommand.dataType;
      Serial.print(_dataType);
      switch (_dataType) {
        case BYTE:
          Serial.println(" Byte \t");
          Serial.println(this->canRcvMsg0.data[3]);
          break;
        case LONG:
          Serial.println(" Long \t");
          Serial.println(Msg2LONG(this->canRcvMsg0));
          break;
        case INT:
          Serial.println(" INTEGER \t");
          Serial.println(Byte2Int(this->canRcvMsg0));
          break;
        case FLOAT:
          Serial.println(" Float16 \t");
          Serial.println(Byte2Float(this->canRcvMsg0));
          break;
        default:
          Serial.println(" NotDefined Type\t");

          break;
      }
    }
  }
  return this->canRcvMsg0;
}

// Get the current ID of the CanMannager

// Set Can to a mode
void CANmannager::SetMode(CanModes _mode) {
  switch (_mode) {
    case CONFIGMODE:
      this->mcp2515.setConfigMode();
      break;
    case LISTENONLYMODE:
      this->mcp2515.setListenOnlyMode();
      break;
    case LOOPBACKMODE:
      this->mcp2515.setLoopbackMode();
      break;
    default:
      this->mcp2515.setNormalMode();
      break;
  };
}

void CANmannager::SetMode() {
  SetMode(NORMAL_MODE);
}

// Method 1: Encode parameters into a byte
byte CANmannager::encodeCommand(CommandChain _commandChain) {
  // Create a byte with the following bit structure:
  // MSB (Bit 7): _isACKReq (bool)
  // Bits 6-4: _type (3 bits)
  // Bits 3-0: _auxCommand (4 bits)
  //Serial.print("*Command:\t");
  //Serial.print(_commandChain.codeMSG);
  //Serial.print("\t \t *type:\t");
  //Serial.print(_commandChain.dataType);
  //Serial.print("\t \t *IsAckReq:\t");
  //Serial.print(_commandChain.isACKreq);
  //return ((_commandChain.isACKreq ? 1 : 0) << _bitShiftMask(ACKREQUIREDMASK)) | ((_commandChain.dataType & DATATYPEMASK) << _bitShiftMask(DATATYPEMASK)) | ((_commandChain.codeMSG & AUXCOMMANDMASK) << _bitShiftMask(AUXCOMMANDMASK));
  byte result = (((_commandChain.isACKreq) << 7) | ((_commandChain.dataType & DATATYPEMASK) << 6) | ((_commandChain.codeMSG & AUXCOMMANDMASK)));
  result = (((_commandChain.isACKreq) << _bitShiftMask(ACKREQUIREDMASK)) | ((_commandChain.dataType) << _bitShiftMask(DATATYPEMASK)) & DATATYPEMASK | ((_commandChain.codeMSG & AUXCOMMANDMASK) << _bitShiftMask(AUXCOMMANDMASK)) & AUXCOMMANDMASK);
  //Serial.print("\t \t *Command Byte : \t");
 // Serial.println(result);

  return result;
}

// Method 2: Decode a byte into parameters
CommandChain CANmannager::decodeCommand(byte _chain) {
  // Extract values from the bit structure of the input byte
  CommandChain _commandChain;
  _commandChain.isACKreq = ((_chain & ACKREQUIREDMASK) != 0);  // MSB

  //_commandChain.dataType = (_chain );
  _commandChain.codeMSG = ((_chain & AUXCOMMANDMASK));
  _commandChain.dataType = (DataTypes)((_chain & DATATYPEMASK) >> _bitShiftMask(DATATYPEMASK)) < DATATYPE_NODEF ? (DataTypes)((_chain & DATATYPEMASK) >> _bitShiftMask(DATATYPEMASK)) : DATATYPE_NODEF;
  //_commandChain.codeMSG = (CanMsg_Type)((_chain & AUXCOMMANDMASK) >> _bitShiftMask(AUXCOMMANDMASK)) < MSG_NODEF ? (CanMsg_Type)((_chain & AUXCOMMANDMASK) >> _bitShiftMask(AUXCOMMANDMASK)) : MSG_NODEF;
  return _commandChain;
}

// Split INT data to byte[] and store in apropiate place ofa canFrame
void CANmannager::Data2Byte(int _value, can_frame &_dataFrame) {
  _dataFrame.data[4] = (byte)_value & 0xFF;
  _dataFrame.data[3] = (byte)(_value >> 8) & 0xFF;
  _dataFrame.data[2] = INT;
}

// Split LONG data to byte[] and store in apropiate place ofa canFrame
void CANmannager::Data2Byte(long _value, can_frame &_dataFrame) {
  this->canMsg.data[3] = (byte)_value & 0xFF;
  this->canMsg.data[4] = (byte)(_value >> 8) & 0xFF;
  this->canMsg.data[5] = (byte)(_value >> 16) & 0xFF;
  this->canMsg.data[6] = (byte)(_value >> 24) & 0xFF;
  this->canMsg.data[2] = LONG;
  this->isMsgReady = true;
}

// Split FLOAT data to byte[] and store in apropiate place ofa canFrame
void CANmannager::Data2Byte(float _value, can_frame &_dataFrame) {
  byte _data[4];
  float *_dataPtr = (float *)&_data[0];
  *_dataPtr = _value;
  Serial.print("Value of the addres pointed by (float*)");
  Serial.println(*_dataPtr);

  _dataFrame.data[6] = _data[0];
  _dataFrame.data[5] = _data[1];
  _dataFrame.data[4] = _data[2];
  _dataFrame.data[3] = _data[3];
  _dataFrame.data[2] = FLOAT;
}

// Restore INT data from a canFrame apropiate MSG
int CANmannager::Byte2Int(can_frame &_dataFrame) {
  int _data = 0;
  _data = ((int)_dataFrame.data[3] << 8);
  _data |= ((int)_dataFrame.data[4]);
  return _data;
}

// Restore LONG data from a canFrame apropiate MSG
long CANmannager::Byte2Long(can_frame &_dataFrame) {
  long _data = 0;
  _data |= ((long)_dataFrame.data[3] << 24);
  _data |= ((long)_dataFrame.data[4] << 16);
  _data |= ((long)_dataFrame.data[5] << 8);
  _data |= ((long)_dataFrame.data[6]);
  return _data;
}

// Restore FLOAT data from a canFrame apropiate MSG
float CANmannager::Byte2Float(can_frame &_dataFrame) {
  byte _data[4];
  _data[0] = _dataFrame.data[6];
  _data[1] = _dataFrame.data[5];
  _data[2] = _dataFrame.data[4];
  _data[3] = _dataFrame.data[3];

  float *_dataPtr = (float *)&_data[0];
  return (float)*_dataPtr;
}



// Restore CHAR data from a canFrame apropiate MSG
char CANmannager::Byte2Char(can_frame &_dataFrame) {
  char _data = 0;
  _data = _dataFrame.data[3];
  return _data;
}

// Return True if the bitFlag from ACK it's == 1
bool CANmannager::CheckISAckMsg(can_frame &_dataFrame) {
  byte _auxDataByte = _dataFrame.data[COMMANDORDER];
  return (_dataFrame.data[COMMANDORDER] >> 7);
}

// Check the DLC for a kind of MSG
bool CANmannager::__checkDLC(byte _n) {
  //_n += this->canMsg.can_dlc;
  if (_n > 0 && _n < CAN_MAX_DLEN) {
    this->canMsg.can_dlc = _n;
    return true;
  } else
    return false;
}

unsigned CANmannager::CanGetID() {
  return this->canID;
}

void CANmannager::CanSetID(byte _ID) {
  this->canID = ((unsigned)_ID << 4);
  SetFilters(_ID);
}

bool CANmannager::StoreValueInCanMsg(byte *_value, byte _size) {
  if (_size <= 0 || _size > 5) {
    // Invalid size, return false
    return false;
  }

  // Get a pointer to the value's memory


  //reinterpret_cast<const byte *>(_value);
  // Store the value bit by bit starting from this->canMsg.data[2]
  for (byte i = 0; i < _size; i++) {
    // Serial.print("Store Value :");
    // this->canMsg.data[(DATAHEADERBITS + _size - 1) - i] = (const byte)(_value[i]);
    // Serial.println(_value[i],HEX);
  }

  this->canMsg.data[(3)] = (const byte)(_value[0]);
  this->canMsg.data[(4)] = (const byte)(_value[1]);
  this->canMsg.data[(5)] = (const byte)(_value[2]);
  this->canMsg.data[(6)] = (const byte)(_value[3]);
  //Serial.print("\t ");
  //Serial.print(_value[3], HEX);
 // Serial.print(" ");
  //Serial.print(_value[2], HEX);
 // Serial.print(" ");
 // Serial.print(_value[1], HEX);
 // Serial.print(" ");
 // Serial.println(_value[0], HEX);
  for (byte i = DATAHEADERBITS; i < DATAHEADERBITS + _size; i++) {
    // Serial.print("Store Value :");
    // this->canMsg.data[i] = (const byte)(_value[i - DATAHEADERBITS]);
    // Serial.println(_value[i - DATAHEADERBITS],HEX);
  }
  this->isMsgReady = true;
  return true;  // Successful storage
}

bool CANmannager::StoreValueInCanMsg(byte *_value) {
  byte _size = this->canMsg.can_dlc - DATAHEADERBITS;

  if (StoreValueInCanMsg(_value, _size)) {
    this->isMsgReady = true;
    return true;
  } else return false;
}

bool CANmannager::SetHeaderMsg(byte _IDReciver, CanMsg_Type _msgCommand, DataTypes _typeData, bool _isAckReq, byte _parameter) {
  this->canMsg.can_id = (_IDReciver << _bitShiftMask(CANIDMASK));

  CommandChain _chainCommandSend;
  _chainCommandSend.isACKreq = _isAckReq;
  _chainCommandSend.dataType = _typeData;
  _chainCommandSend.codeMSG = _msgCommand;
  this->canMsg.data[IDSENDERORDER] = (CanGetID() >> _bitShiftMask(CANIDMASK));
  this->canMsg.data[COMMANDORDER] = (encodeCommand(_chainCommandSend));
  this->canMsg.data[PARAMORDER] = _parameter;
  byte n = DATAHEADERBITS;

  if (_typeData == BYTE || _typeData == CHAR)
    n += 1;
  else if (_typeData == INT || _typeData == UNSIGNED)
    n += 2;
  else if (_typeData == LONG || _typeData == FLOAT || _typeData == UNSIGNEDLONG)
    n += 4;
  else
    return false;  // Not valid data type

  if (__checkDLC(n))
    return true;
  else
    return false;
}

// Method to extract an UNSIGNED value from a can_frame
unsigned CANmannager::Msg2UNSIGNED(can_frame &_canMsg) {
  byte _data[sizeof(unsigned)];
  for (int i = 0; i < sizeof(unsigned); i++) {
    _data[i] = _canMsg.data[3 + i];
  }

  unsigned *_dataPtr = (unsigned *)&_data[0];
  return (unsigned)*_dataPtr;
}

// Method to extract a LONG value from a can_frame
long CANmannager::Msg2LONG(can_frame &_canMsg) {
  byte _data[sizeof(long)];
  for (int i = 0; i < sizeof(long); i++) {
    //_data[(sizeof(long))-(i+1)] = _canMsg.data[3 + i];
    _data[i] = _canMsg.data[3 + i];
  }

  long *_dataPtr = (long *)&_data[0];
  return (long)*_dataPtr;
}

// Method to extract a BYTE value from a can_frame
byte CANmannager::Msg2BYTE(can_frame &_canMsg) {
  return _canMsg.data[3];
}

// Method to extract a BOOL value from a can_frame
bool CANmannager::Msg2BOOL(can_frame &_canMsg) {
  return _canMsg.data[3] != 0;
}

// Method to extract a FLOAT value from a can_frame
float CANmannager::Msg2FLOAT(can_frame &_canMsg) {
  byte _data[sizeof(float)];
  for (int i = 0; i < sizeof(float); i++) {
    _data[i] = _canMsg.data[3 + i];
  }

  float *_dataPtr = (float *)&_data[0];
  return (float)*_dataPtr;
}

ERROR_Type CANmannager::_AckMannage() {
  int counter = 0;
  // ACK pending
  if (ackList.countElements() == 0) {
    // Serial.println("Waiting recieve ACK from :\t");
    // Serial.println(ackList.countElements());
  }


  while (ackList.countElements() > 0 && counter < ackList.countElements()) {
    PendingACK pendAck;
    Serial.print(counter);
    if (ackList.get(counter, pendAck)) {
      if (pendAck.timer0.GetDeltaTime() > 10000) {
        // ERROR NODE NOT RESPOND
        Serial.print(pendAck.timer0.GetDeltaTime());
        Serial.println("\tERROR ACK TIMEOUT");
        ackList.remove(counter);
        return NOT_CONECTED_ERROR;
      } else if (pendAck.timer0.GetDeltaTime() > 5000) {
        PrepareACKMsg((pendAck.deviceID << 4), (CanGetID()), (MSG_ACK & ACKREQUIREDMASK));
        if (this->mcp2515.sendMessage(&this->canMsg) == MCP2515::ERROR_OK) {
          Serial.println("ACKfriendlyReminder SEND");
        } else
          Serial.println("ACKfriendlyReminder NOT SEND");
        ResetMsg(this->canMsg);
      } else {
        // Serial.println("Waiting recieve ACK from :\t");
        //  Serial.println(ackList.countElements());
      }
    }
    counter++;
  }

  // ACK Sender
  counter = 0;
  while (ackSendingList.countElements() > 0 && counter < ackSendingList.countElements()) {
    SendingACK sendACK;
    if (ackSendingList.get(counter, sendACK)) {
      PrepareACKMsg((sendACK.deviceID << 4), (this->CanGetID() >> 4), sendACK.aux_char);
      if (SendMsg() == MCP2515::ERROR_OK) {
        ackSendingList.remove(counter);
        Serial.println("ACK Sent");
      }
      ResetMsg(this->canMsg);
      counter++;
    }
  }
}

bool CANmannager::_AckResponded(can_frame &_dataFrame) {


  if ((_dataFrame.data[COMMANDORDER] & AUXCOMMANDMASK) == MSG_ACK)  // When a device respond and ack;
  {
    Serial.print("AckResponded \t to Command:");
    Serial.println(_dataFrame.data[COMMANDORDER] & AUXCOMMANDMASK);
    int counter = 0;
    while (ackList.countElements() > 0 && counter < ackList.countElements()) {
      PendingACK pendAck;
      if (ackList.get(counter, pendAck)) {
        if (pendAck.deviceID == _dataFrame.data[0]) {  //&& _dataFrame.data[2]&AUXCOMMANDMASK == pendAck.aux_char) {
          Serial.print("ACKRecieved From \t");
          Serial.println(pendAck.deviceID);
          ackList.remove(counter);
        }
      }
      counter++;
    }
    return true;
  } else
    return false;
}

byte CANmannager::_bitShiftMask(byte _mask) {
  if (_mask == 0) {
    return 0;  // No shifts needed for 0x00
  }
  byte count = 0;
  while ((_mask & 0x01) == 0) {  // Continue shifting until the LSB is set
    _mask >>= 1;
    count++;
  }
  return count;
}


void SendStepsMSG(CANmannager *_can, byte _IDReciver, CanMsg_Type _msgCommand, DataTypes _typeData, bool _isAckReq, byte _parameter, StepsUnits _steps) {

  SetStepMsg(_can, _IDReciver, _msgCommand, _typeData, _isAckReq, _parameter, _steps);
  byte MSG_Status = _can->SendMsg();
  if (MSG_Status != MCP2515::ERROR_OK) {
    Serial.print("Failed Sending MSG");
    Serial.println(MSG_Status);
  }
}

bool SetStepMsg(CANmannager *_can, byte _IDReciver, CanMsg_Type _msgCommand, DataTypes _typeData, bool _isAckReq, byte _parameter, StepsUnits _steps) {
  if (_can->SetHeaderMsg(_IDReciver, _msgCommand, _typeData, _isAckReq, _parameter)) {
    //Serial.print("SetSteps:");
    //Serial.println(_parameter);
    Serial.print("Laps :");
    Serial.print(_steps.getLaps(), HEX);
    Serial.print("\t Steps :");
    Serial.println(_steps.getTicks(), HEX);
    byte *stepsPtr = (byte *)&_steps;
    if (_can->StoreValueInCanMsg(stepsPtr, sizeof(_steps)))
      _can->SendMsg();
    return true;
  }
  return false;
}


#endif
