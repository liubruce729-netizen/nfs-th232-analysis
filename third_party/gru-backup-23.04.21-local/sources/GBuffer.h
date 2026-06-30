// File : GBuffer.h
// Author: Luc Legeard  (winter 2004)
//////////////////////////////////////////////////////////////////////////////
//
// Class GBuffer
//
// This class manger Ganil Buffer
//
/////////////////////////////////////////////////////////////////////////////


// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#ifndef __GBuffer__
#define __GBuffer__

// Size of different element lot of buffer
#define COUNT_SIZE      4
#define ASCII_INFO_SIZE 80

#include "GEN_TYPE.H"
#include "TObject.h"
#include "General.h"
#include "GBase.h"

#include "MFMTypes.h"

#define UNKNOWN_Idn       0

#define IN2P3_MIN_Idn     100001
#define FILEH_Idn         100001
#define EVENTH_Idn        100002
#define PARAM_Idn         100003
#define COMMENT_Idn       100004
#define EVENTDB_Idn       100005
#define EVENTDB_SWAP_Idn  100006
#define EVENTCT_Idn       100007
#define EVENTCT_SWAP_Idn  100008
#define JBUS_Idn          100009
#define JBUS_SWAP_Idn     100010
#define SCALER_Idn        100011
#define SCALER_SWAP_Idn   100012
#define STATUS_Idn        100013
#define STATUS_SWAP_Idn   100014
#define EBYEDAT_Idn       100015
#define RAWDT32_Idn       100016
#define CONFIG_Idn        100017
#define INFODAT_Idn       100018
#define ENDRUN_Idn        100019
#define IN2P3_MAX_Idn     100019

// for MFM Identifier the type are declared in
// #include "MFMTypes.h"


extern "C"
{
#include "gan_acq_buf.h"
}

//______________________________________________________________________________________



class GBuffer : public GBase{

 public :

 // char   fGBuf_header[HEADER_SIZE+1];// header of buffer copy of 8 first char of buffer
  unsigned int    fGBuf_index; // index of buffer (counter of buffer) ( of buffnumber)
  int    fGBuf_type ; // type of buffer ( EVENTDB_Idn,SCALER_Idn,.....)
  char*  fGBuf_data ; //! pointeur on begin buffer
  int    fGBuf_increment;//
  bool   fIsAEventType ; // Boolean to indicate if buffer contents events
  bool   fIsAIn2p3Type;
  bool   fIsAMFMType;
  bool   fIsAHeaderType;
  bool   fIsALocalAlloc;// allow to know if space buffer (fGBuf_data) is allocated in GBuffer or outside
  int    fRunNumber ;// Run number
  char   fDate[MAX_CARACTERES]; // Date
  unsigned int fUsedEventsSize; //Used size by Event in bytes of the buffer during filling

protected:
  int    fGBuf_size ; // size of buffer
  Int_t	 fReadSize;         // if read buffer is FILEH  then fReadSize contains written buffer size


 public :

  GBuffer(int _bufsize = 0);
  ~GBuffer();

virtual  char*     GetBufDataChar(){return fGBuf_data;}
virtual  TString   GetDumpBuffer(int dumpsize  = 256,int increment=-1);
virtual  int   DumpBuffer(int dumpsize = 256,int increment=-1);
virtual  void  SetBufSize(int size,bool forcetoalloc=true);
virtual  void  SetExternalDataZone(void* pt, int size);
virtual  int   GetBufSize(){return fGBuf_size;};
virtual  char* GetDate(){return fDate;};
virtual  int   GetNumBuf(){return fGBuf_index;}
virtual  void  SetNumBuf( unsigned int num){fGBuf_index =num;}
virtual  int   GetNumRun(){return fRunNumber;};
virtual  int   GetReadSize(){return fReadSize;};
virtual  Int_t SetReadSize(Int_t size){return fReadSize = size;};
virtual  void  DumpBufferHeader() ; // Print a dump of header of a  Buffer
virtual void   SetType(int type)  {fGBuf_type = type; };
virtual  char* GetDate_()=0;
virtual  int   GetNumRun_()=0;
virtual  void  Equal(GBuffer& buf1)=0;
virtual  void  Equal(GBuffer* buf1)=0;
virtual  void  MyDelete()=0;
virtual  int   GetBufSizeFromBuffer()=0;
virtual  void  SetAttributs(bool quiet =false) =0;
virtual  int   GetHeaderSize()=0;
virtual TString GetDumpBufferHeader()=0 ; // Get a dump of header of a  Buffer

virtual unsigned int GetUsedEventsSize(){return fUsedEventsSize;};
virtual void SetUsedEventsSize(unsigned int usedEventSize);
virtual void RazBuffer(); // Do a reset of the buffer's data

virtual  bool  IsAHeaderBuffer();
virtual  bool  IsAEventBuffer();
virtual  bool  IsAMFMBuffer();
virtual  Int_t TestType(char * pt)=0;

/*************AJOUTS JOHN*****************************/
virtual  Bool_t IsEventBuffer() const { return fIsAEventType; };
virtual  Int_t  GetType()   const { return fGBuf_type;        };
virtual  Bool_t IsFILEH()   const { return GetType()==FILEH_Idn;  };
virtual  Bool_t IsEVENTH()  const { return GetType()==EVENTH_Idn; };
virtual  Bool_t IsPARAM()   const { return GetType()==PARAM_Idn;  };
virtual  Bool_t IsCOMMENT() const { return GetType()==COMMENT_Idn;};
virtual  Bool_t IsEVENTDB() const { return GetType()==EVENTDB_Idn;};
virtual  Bool_t IsEVENTCT() const { return GetType()==EVENTCT_Idn;};
virtual  Bool_t IsSCALER()  const { return GetType()==SCALER_Idn; };
virtual  Bool_t IsENDRUN()  const { return GetType()==ENDRUN_Idn; };
/*****************************************************/
private:
virtual void ToDoInCaseOfInterrupt(){};
  ClassDef (GBuffer ,1) // In2P3 Buffer

};
#endif
