/************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                 Assets.h
                                                  0.0.01a

                                                                                             MaccoRobotics

                                                                                Mathias Lofeudo Clinckspoor

                                                                                                  25/09/22
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ************************************************************************************************************************
Assets: contains useful objects (ID, Error)

************************************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#pragma once

#ifndef _Assets
#include "ID_Def.h"
#define _Assets

struct ErrorID_Struct // Struct that holds an Error message
{                             
  ID_Definition ID = ID_NODEF; // __ID of the Error
  ObjectTYPE type = T_NODEF;   // Object type of the Error
  ERROR_Type Error = NO_ERROR; // Error Code
  bool isInitialize = false;   // Initialize Flag
};

class __ID  // __ID Class, Mannage ID data of an object
{
public:
  static ErrorID_Struct MainERROR; // Main Error Holder 1 for all nodes
  static void (*_CallErrorMannager_Ptr)(__ID & _ownID_ptr);
static void (*_Dummy_Ptr)();

  __ID();
 
  bool AttachID(ID_Definition _ID, ObjectTYPE _type); // Function that attach a new ID to an object, use to initialize an ID (INPUTS NªID & NºType) (Returns IsValidID)
  ID_Definition GetID();                              // Function that (Returns the ID Nº)
  ObjectTYPE GetType();                               // Function that (Returns the type Nº)
  void ThrowError(ERROR_Type _newERROR);              // Function to set an Error to an ID (INPUTS ERROR-Code)
  bool GetIsInitialize();                             // Returns true If the ID it's initialize
  ERROR_Type GetErrorCode();                          // Returns the current ErrorStatus of the device (ownID)
  ErrorID_Struct GetErrorID();                        // Returns the current ErrorStatus of the device (ownID)
  ErrorID_Struct GetMainErrorID();                    // Returns the global ErrorStatus
  void ClearError();                                  // Clear a error on OwnID
  void ClearMainError();                              // Clear Global Error

private:
  ErrorID_Struct ERROR_ID; // Error Holder
};


class Master__ID :public __ID 
{
public:
Master__ID();
bool Attach_ErrorID_Callback (void (*_CallErrorMannager_Ptr)(__ID & _ownID_ptr));

private:
void __ExampleCallback (__ID & _ownID_ptr);

};



#endif