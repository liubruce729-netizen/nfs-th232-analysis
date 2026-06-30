// File : GEvent.h
// Author: Luc Legeard  (spring 2004)
//////////////////////////////////////////////////////////////////////////////
//
// Class GEvent
//
// This class manger Ganil Event
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

#ifndef __GEvent__
#define __GEvent__

#include "General.h"
#include "GBufferIn2p3.h"
#include "DataParameters.h"
#include "GSubEvent.h"
#include "GEventBase.h"
#include "GEN_TYPE.H"

#include <sstream>

#define ExogamEventToken 0xff60
#define EndDataBlockToken 0xff00

#define TPC_NbChannels 384
#define TPC_Separateur 0xefff
#define CPUFAMI 1
#define LENLINE 132
#define CAM_COUPL CPUFAMI<<8
//______________________________________________________________________________________


class GEvent : public GEventBase{

 public:
  int            fStatus;



 private:

  UNSINT16 		 *pNextEvent ; //! Pointer on start of next event
  EBYEDAT_EVENT_HD *fEventHd ;//! stucture on ebyedat header event.
  Int_t            fTailleMatrice; // total size of Multiplicity matrice.
 protected:
  char            *pStructEvent;   //!
  Int_t            fCtrlForm;      // Fix(EVCT_FIX)/variable length events(EVCT_VAR) or brut(-1)
  uint16_t        *pEventBrut;     //!  Brut data event
  uint16_t        *pEventBrutData; //! recreated Brut data event (not pointer on data coming from flow of file!)

  uint16_t        *pEventCtrl;     //! recreated Control data event (not pointer on data coming from flow of file!)
  uint16_t        *pEventCtrl_0;     //! recreated Control data event (not pointer on data coming from flow of file!)
  bool             fInit_event_done; // Flag to know if initialisation of event have been realised

  Int_t            fEventCtrlCurrentSize;// Size of the current ctr event
  Int_t            fEvcsize;       // Size of the ctrl data buffer
  int     		   fNbSubEvt;      // number of subevents inside of Event
  long long *      pTimeStampSubEvts; //! point of sub evt time stamps


 public :

  GEvent(DataParameters* parameter);
  ~GEvent();


  virtual long long * GetTimeStampSubEvt(){return pTimeStampSubEvts;}

//  virtual uint16_t*  GetArray(){return pDataArray;}
//  virtual Int_t      GetArraySize(){return fDataArraySize;}
  virtual uint16_t*  GetEventCtrl(){return pEventCtrl;}
  virtual Int_t      GetEvcsize(){return fEvcsize;}
  virtual uint16_t*  GetArrayLabelValue(){return  pEventBrutData;}
  virtual Int_t      GetArrayLabelValueSize(){return fEventCtrlCurrentSize;}
  virtual Int_t      GetArrayLabelValueSizeMax(){return fEvcsize;}
  virtual uint16_t   GetArrayLabelValue_Label(uint16_t position);
  virtual uint16_t   GetArrayLabelValue_Value(uint16_t position);
  virtual Int_t      GetCtrlForm(){return fCtrlForm;}
  virtual void       RazEvent();

  virtual  int       NextEvent(GBuffer* _buffer);
  virtual  int       ReadNextEvent_EVENTCT(GBuffer* _buffer);
  virtual  int       ReadNextEvent_EVENTDB(GBuffer* _buffer);
  virtual  int       ReadNextEvent_EBEYEDAT(GBufferIn2p3* _buffer);
  virtual  int       EventUnravelling(void);

  virtual TString   GetDumpHeader();
  virtual TString   GetDumpEvent(char mode= 'd');
//  virtual TString   GetDumpArray(char mode= 'd',bool nozero=true);
  virtual void      GetDumpRawEvent();

 // virtual  void   ClearData(int value =-1);
  //virtual  void   EventInit(char* Exp_Name= (char*)"local");

  virtual  void   EventInitWithFileName(char* actionFilePAR,char* actionFileSTR);

  virtual void     SetCtrlForm(Int_t entier){fCtrlForm = entier;}
  virtual void     SetRandomLaw (int law);
  virtual void     SetRandomProba(Float_t proba);
  virtual int      GetNbofSubEvt();


//Giovanni
	GSubEvent *fSubEvent; // Sub event to generate events
	Char_t fHeader_size; // Size of event's header in 16bits words
	Short_t fEvent_size; // Size of the event in 16 bits words

	bool IsRaz();
	int  GetUsedSize();
	virtual void FillEvent(Char_t systemId, Char_t statusWords, Char_t numberWords, Char_t clockWords, Char_t format, Long64_t status, Long64_t number, Int_t** subTabsOfLabls, Int_t nbOfSUbEvents);
	virtual void MakeEventHeader(Char_t, Char_t,Char_t, Char_t, Long64_t, Long64_t, Short_t);
	virtual void FillBufferWithEvent(GBuffer* buffer);
	virtual void FillBufferWithRawEvent(GBuffer* buffer) ;
	virtual void DumpMadeEvent(); // DO dump of a generated event
	virtual void DumpSubEvent(int);

public :
//////////////// AJOUTS JOHN ////////////////////////////////////////////////
  virtual void Connect(const Int_t index,uint16_t **p) const;
  virtual Bool_t Connect(const TString parName,uint16_t **p) const;

//////////////// AJOUTS JOHN ////////////////////////////////////////////////of

  ClassDef (GEvent ,1) // Data Event

};
#endif
