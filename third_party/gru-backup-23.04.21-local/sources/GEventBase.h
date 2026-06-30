// File : GEventBase.h
// Author: Luc Legeard  (spring 2004)
//////////////////////////////////////////////////////////////////////////////
//
// Class GEventBase
//
// Class to manage Events
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

#ifndef __GEventBase__
#define __GEventBase__

#include "General.h"
#include "GBuffer.h"
#include "GEN_TYPE.H"
#include "DataParameters.h"
#include <sstream>
#include <TString.h>
//______________________________________________________________________________________

class GEventBase : public GBase{


 public:
  Int_t          fEventNumber;   // Local event number in current buffer (should be renamed)
  long long      fTimeStamp; //
  Int_t          fEventCount;    // Our event counter
  Int_t          fEvenType; // type of event ; EBYEDAT, MFM,
  DataParameters* pParameter;//!
  int            fEvt_increment;
  int            fEventReadSize; // Size of read event in char size.
  bool           fIsStrFilePresent ;
  Int_t          fEvbsize;       // Size of the brut data buffer
 protected:
  int            fStatus;
  bool			 fMFM_Event,fInit_event_done;
  bool           fLipflop;
  int            fLaw;
  float        fProba;// Probability of parameter appearance

  uint16_t        *pDataArray;     //! Physical data array, the event is reconstitued with 0
  Int_t            fDataArraySize; // Data array size


 private:

 protected:
   char 		 *pEventBrut_char;//! Brut data event with header
  public :

  GEventBase(DataParameters* parameter);
  ~GEventBase();


  virtual char * GetEventDataChar(){return pEventBrut_char;}
  virtual int    GetEventDataSize(){return fEventReadSize;}

  virtual Int_t     GetEventNumber(){return fEventNumber;}
  virtual long long GetTimeStamp()   {return fTimeStamp;}

  virtual bool EventInitAlready();
  virtual void SetEventInitAlready(bool setinit) ;

  virtual int  NextEvent(GBuffer* _buffer)=0;
  virtual void EventInit(char* name= (char*)"local");
  virtual void SetRandomLaw (int law);
  virtual void SetRandomProba(Float_t proba);

  bool IsRaz();

  virtual void             DumpEventRaw(int dumpsize = 256,int increment=-1);
  virtual TString          GetDumpEventRaw(int dumpsize  = 256,int increment=-1);
  virtual void             DumpArray(char mode= 'd',bool nozero=true);
  virtual TString          GetDumpArray(char mode= 'd',bool nozero=true);
  virtual void             DumpEvent(char mode= 'd');
  virtual TString          GetDumpEvent(char mode= 'd')=0;
  virtual void             DumpHeader();
  virtual TString          GetDumpHeader()=0;
  private:
  virtual void ToDoInCaseOfInterrupt(){};
  public:
	// TODO
  virtual void             RazEvent()=0;
//  virtual uint16_t         *GetArray()=0;
//  virtual Int_t            GetArraySize()=0;
  virtual uint16_t *GetArray(){return pDataArray;}
  virtual Int_t     GetArraySize(){return fDataArraySize;}
  virtual uint16_t         *GetArrayLabelValue()=0;
  virtual Int_t            GetArrayLabelValueSize()=0;
  virtual uint16_t         GetArrayLabelValue_Label(uint16_t position)=0;
  virtual uint16_t         GetArrayLabelValue_Value(uint16_t position)=0;
  virtual Int_t            GetArrayLabelValueSizeMax()=0;
  virtual DataParameters*  GetDataParameters()const {return pParameter;};
  virtual void             EventInitWithFileNameBase(char* actionFilePAR, char* actionFileSTR);
  virtual void             EventInitWithFileName(char* actionFilePAR, char* actionFileSTR)=0;
  virtual int              EventUnravelling(void)=0;
  virtual void             ClearData(int value=-1);
  ClassDef (GEventBase ,1) // Data Event

};
#endif
