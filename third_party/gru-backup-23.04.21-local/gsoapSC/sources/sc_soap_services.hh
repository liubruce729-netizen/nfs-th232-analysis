/* gsoap definitions for SC_Embedded package
   Author : B. Raine GANIL

   Modifications :
    21 dec 2011 - BR - Add CreateRegister
    june 2012   - FS - style: document, encoding: literal instead of rpc,encoded
    august 2012 - BR - Register string 

   Utilisation :
     Valid type values :
       enum RegType {
       REG_UINT8 = 1,
       REG_UINT16,
       REG_UINT32,
       REG_STRING,
       REG_NONE
       };

       enum RegAccess {
       REG_READ_ONLY = 1,
       REG_WRITE_ONLY,
       REG_READ_WRITE
       };

     Valid string values for RegType :
       "INT8", REG_UINT8,
       "INT16", REG_UINT16,
       "INT32", REG_UINT32,
       "STRING", REG_STRING

     Valid string values for RegAccess :
       "READ_ONLY", REG_READ_ONLY,
       "WRITE_ONLY", REG_WRITE_ONLY,
       "READ_WRITE", REG_READ_WRITE

     Valid transition values :
       enum ECC_transition {NONE=0, DESCRIBE, PREPARE, CONFIGURE, START, STOP, BREAKUP, UNDO};

     Valid state values
       enum ECC_state {OFF=0,IDLE, DESCRIBED, PREPARED, READY, RUNNING};

     Valid error values in state response
       enum ECC_error {ECC_NO_ERROR=0, ECC_ERROR_DURING_TRANSITION, ECC_ERROR_GENERIC};

*/

//gsoap sc service name: sc
//gsoap sc service style: document
//gsoap sc service encoding: literal
//gsoap sc service location: http://localhost:8061
//gsoap sc schema namespace: urn:sc
#ifndef __libSCSoap_ServiceH__
#define __libSCSoap_ServiceH__
#include <stdlib.h>
#include <string>


class sc__Response
{
  int Error;
  std::string ErrorMessage;
};
class sc__ResponseInt
{
  int Error;
  std::string ErrorMessage;
  int value;
};

class sc__ResponseFloat
{
  int Error;
  std::string ErrorMessage;
  float value;
};

class sc__ResponseBool
{
  int Error;
  std::string ErrorMessage;
  bool value;
};

class sc__ResponseString
{
  int Error;
  std::string ErrorMessage;
  std::string value;
};

class sc__ResponseRegister
{
  int Error; 
  std::string ErrorMessage;
  int Type;
  std::string val_string;
};

class sc__ResponseState
{
  int Error;
  std::string ErrorMessage;
  int State;
  int Transition;
  int StateError;
  std::string State_ascii;
  std::string Transition_ascii;
  std::string StateError_ascii;
};

class xsd__base64Binary_test
{
 public:
   unsigned char* __ptr;
   int __size;
};

int sc__ServerExit(sc__Response & response);

int sc__CreateDevice(std::string DeviceName, std::string Type, std::string LinuxDeviceName, sc__Response & response);

int sc__CreateMemoryDevice(std::string DeviceName, std::string LinuxDeviceName, 
			   unsigned int LowAddress, unsigned int HighAddress,
			   sc__Response & response);

int sc__CreateRegister(std::string DeviceName, std::string RegisterName,
			  unsigned int OffsetAddress, std::string RegType, std::string RegAccess,
			  sc__Response & response);

int sc__CreateRegister_DefVal(std::string DeviceName, std::string RegisterName,
				 unsigned int OffsetAddress, std::string RegType, std::string RegAccess,
				 std::string DefValue, sc__Response & response);


int sc__WriteRegister(std::string DeviceName, std::string RegisterName, std::string Data, sc__ResponseString & response);


int sc__ReadRegister(std::string DeviceName, std::string RegisterName, sc__ResponseRegister & response);



int sc__CreateRegisterInt(std::string DeviceName, std::string RegisterName,
			  unsigned int OffsetAddress, std::string RegType, std::string RegAccess,
			  sc__Response & response);

int sc__CreateRegisterInt_DefVal(std::string DeviceName, std::string RegisterName,
				 unsigned int OffsetAddress, std::string RegType, std::string RegAccess,
				 int DefValue, sc__Response & response);

int sc__WriteRegisterInt(std::string DeviceName, std::string RegisterName, int Data, sc__ResponseInt & response);

int sc__ReadRegisterInt(std::string DeviceName, std::string RegisterName, sc__ResponseInt & response);

int sc__Reset(std::string DeviceName, sc__Response &Response);

int sc__GetInfo(sc__ResponseString &Response);

int sc__GetInfoDevice(std::string DeviceName, sc__ResponseString &Response);

int sc__SetDebugLevel(std::string DeviceName, int DebugLevel, sc__ResponseInt & response);

int sc__GetDebugLevel(sc__ResponseInt & response);

// In the following methods, set DeviceName = "" to access the Module services

int sc__Describe(std::string DeviceName, std::string, std::string, sc__ResponseString & response);

int sc__Prepare(std::string DeviceName, std::string, std::string, sc__ResponseString & response);

int sc__Configure(std::string DeviceName, std::string, std::string, sc__ResponseString & response);

int sc__Start(std::string DeviceName, sc__Response & response);

int sc__Stop(std::string DeviceName, sc__Response & response);

int sc__Breakup(std::string DeviceName, sc__Response & response);

int sc__Undo(std::string DeviceName, sc__Response & response);

int sc__GetState(std::string DeviceName, sc__ResponseState & response);

int sc__ReadScope(xsd__base64Binary_test & response);
#endif
