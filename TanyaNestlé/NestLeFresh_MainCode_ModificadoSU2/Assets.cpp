/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 Assets.cpp
                                                  0.0.01a
                                                                                             MaccoRobotics
                                                                                Mathias Lofeudo Clinckspoor
                                                                                                  25/09/22

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#ifndef _Assets
#include <Arduino.h>
#include "Assets.h"

ErrorID_Struct __ID::MainERROR;
void (*__ID::_CallErrorMannager_Ptr)(__ID & _ownID_ptr);


// __ID Constructor
__ID::__ID() {
  this->ERROR_ID.isInitialize = false;
  __ID::MainERROR.ID = ID_NODEF;
  __ID::MainERROR.type = T_NODEF;
  __ID::_CallErrorMannager_Ptr = NULL;
}

// Function that attach a new ID to an object, use to initialize an ID (INPUTS NªID & NºType) (Returns IsValidID)
bool __ID::AttachID(ID_Definition _ID, ObjectTYPE _type) {
  if (_ID != 0x00 && !(this->ERROR_ID.isInitialize)) {
    this->ERROR_ID.isInitialize = true;
    this->ERROR_ID.ID = _ID;
    this->ERROR_ID.type = _type;
    return 1;
  } else
    return 0;
}

// Function that (Returns the ID Nº)
ID_Definition __ID::GetID() {
  if (this->ERROR_ID.isInitialize)
    return (this->ERROR_ID.ID);
  else
    return ID_NODEF;
}

// Function that (Returns the type Nº)
ObjectTYPE __ID::GetType() {
  if (this->ERROR_ID.isInitialize)
    return (this->ERROR_ID.type);
  else
    return T_NODEF;
}

// Function to set an Error to an ID (INPUTS ERROR-Code)
void __ID::ThrowError(ERROR_Type _newERROR) {
  this->ERROR_ID.Error = _newERROR;
  __ID ownID = (*this);
  __ID::_CallErrorMannager_Ptr(ownID);
}

// Returns true If the ID it's initialize
bool __ID::GetIsInitialize() {
  return this->ERROR_ID.isInitialize;
}

// Returns the current ErrorStatus of the device (ownID)
ERROR_Type __ID::GetErrorCode() {
  return this->ERROR_ID.Error;
}

// Returns the ID Error data
ErrorID_Struct __ID::GetErrorID() {
  return this->ERROR_ID;
}

// Returns the global ErrorId data
ErrorID_Struct __ID::GetMainErrorID() {
  return __ID::MainERROR;
}

// Clear a error on OwnID
void __ID::ClearError() {
  this->ERROR_ID.Error = NO_ERROR;
}

// Clear Global Error
void __ID::ClearMainError() {
  __ID::MainERROR.Error = NO_ERROR;
  __ID::MainERROR.ID = ID_NODEF;
  __ID::MainERROR.type = T_NODEF;
  __ID::MainERROR.isInitialize = false;
}

Master__ID::Master__ID(): __ID() {

  void(*__CallBack)(__ID &_ownID_ptr) = (void(*)(__ID &ownID_ptr))&__ExampleCallback;
__ID::_CallErrorMannager_Ptr = __CallBack;

}
bool Master__ID::Attach_ErrorID_Callback(void (*_CallErrorFunction_Ptr)(__ID &_ownID_ptr)) {
  __ID::_CallErrorMannager_Ptr = _CallErrorFunction_Ptr;
}

void Master__ID::__ExampleCallback(__ID &_ownID_ptr) {
  // Use 1 Get Split Data
  Serial.println("Error:");
  Serial.print("\t ID: ");
  Serial.print((int)_ownID_ptr.GetID());
  Serial.print("\t Type: ");
  Serial.print((byte)_ownID_ptr.GetType());
  Serial.print("\t ErrorCode: ");
  Serial.println((byte)_ownID_ptr.GetErrorCode());

  //Use 2 Get all ID data packaged in ErrorID_Struct (__ID dataType)
  
    Serial.print("\t ID: ");
 //Serial.print(_ownID_ptr.ERROR_ID.ID);
  Serial.print("\t Type: ");
 // Serial.print(ERROR_ID.type);
  Serial.print("\t ErrorCode: ");
 // Serial.println(ERROR_ID.Error);
}

#endif