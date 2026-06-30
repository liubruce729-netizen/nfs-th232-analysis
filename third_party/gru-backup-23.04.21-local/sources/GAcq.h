// File :  GAcq.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GAcq
//
// This class manger tape and file in Ganil format
// The associated methods can do copies,duplications, verifications, dumps...
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


#ifndef __GAcq__
#define __GAcq__

#include <sys/time.h>

#include "General.h"
#include "GBase.h"
#include "GDevice.h"
#include "GTape.h"
#include "GEvent.h"
#include "GBuffer.h"
#include "GTTree.h"
#ifndef GT
#include "GSpectra.h"
//#include "GNetServerRoot.h"
#endif
#include "GScaler.h"
#include "DataParameters.h"

#include "GStateMachine.h"
#include <TMutex.h>
#include <TCondition.h>

//_________________________________________________________________________________________


class GAcq : public GBase{

 private:



  int fSpectraMode;   // fSpectraMode = 1 , authorize the creation of standard histogram

  //bool fReinitTree;   // Reinitialization of run for each run
 // bool fInitTreeNeverDone; // true if init tree is not done
  //bool fTTreeWithRunNumber;   // TRee will contain run number in its file name
 // bool fSkipFillTtree; // Flag to significate that we do not fill the tree with current TTree
 // Int_t fTTreeCompressionLevel; // Compression level of TTree file 1 to 9 ( 1 low level but fast 9 high level but slow)

  GEventBase          *fEvent  ;          //! current internal event
  DataParameters *fDataParameters  ; //! Data parameters names class

  Bool_t           fInfiniteRead;     // allow to have a infinite read from a file device ( if endrun is reached a automatic rewind is done)
  UInt_t           fEventUSleep;      // allow to do a usleep on event analyze if necessary;
  UInt_t           fEventSleep;       // allow to do a sleep on event analyze if necessary;
  Bool_t           fInEvtTreatment;   // flag to know if we are in treatment (incrementation of spectra or user treatment)


  #ifndef GT
  GSpectra        *fSpectra;          //! acquisition spectra
 // GNetServerRoot  *fNet;  //!
#endif

  bool fBufferEmpty;    // flag to know if a buffer is empty
  Int_t          fCondition_of_stop;// =0 standard ( until number of events is reach) =1 until fStop is true
  bool           fStop; // User condition to stop a run;

  struct timeval fMt_begin, fMt_end; // time structure
  struct timezone fTz;

 protected:
  GDevice *fDevIn;    //! input device
  GDevice *fDevOut;   //! output device
  int      fStatus;   // Internal status ( to have a status without a returned status which noise the standard output of Cint )
  Float_t  fPauseTimeout;
  bool     fPause;
  bool     fInPause;
  bool     fReloadPause;
  Int_t          fRunNumber;     // current file run number
  Int_t          fEventCount;    // Our event counter
  Int_t          fBufferCount;    // Our event counter
  Float_t        fRateEvent;     // Our event counter rate
  Float_t        fRateBuffer;    // Our buffer counter rate
  Float_t        fRateBuffer_evt;    // Our event buffer counter rate
  Float_t        fSucces;        // Success rate rate in %
  Float_t        fSucces_mean;        // Success rate rate in %
  Float_t        fSucces_mean_total;        // Success rate rate in %
  Float_t        fSucces_nb_total;        // Success rate rate in %
  UInt_t         fnbEventAsked;    // nb event counter asked to analyze
  UInt_t         fnbBufferAsked;    // nb buffer counter asked to analyze
  int fCoups;// nb of events in one run.

  GScaler *fScaler;              //! Scaler object to manage events Scaler
  GTTree  *fTheTree;             //! manage TTree

  TThread**  fThreadPointer;//!Pointer other than producer
  TThread*  fThreadProducer;//!poinyer on Thread
  TThread*  fThreadvigru;//!pointer on Thread for vigru
  Int_t  fNumberMaxThread; // Maximun Number of Thread managed by this class
  Int_t  fCurrentNumberThread; // Current Number of Thread managed by this class

  Bool_t	fPrintRCbeforeStat;   // to print a RC before diplay of Statistique if need 
  Bool_t	fAcquisitionOn;  // = true when GRU acquisition is running
  Bool_t	fStoreOn;	    // = true when Gabil acquisition is in 'storage' mode
  Bool_t	fGanilAcqOn;     // = true when Ganil acquisition is running
  Bool_t	fConfGanilAcq ;  // Correlation with Ganil Acquisition in run number ( with creation of TTree with   good run number in file name and if fStoreOn = true
  Bool_t       fInfoCont;       // true to have continuous information on acquistion
  Bool_t       fOutAuthorized;   // true if a output is validate
  TMutex fMyMutex1;
  TCondition *fMyCondition1;
  uint16_t my_value;

 public:
// GAcq(GDevice** _fDevIn= NULL,int nbDevIn=0, GDevice** _fDevOut= NULL,int nbDevOut=0);
 GAcq(GDevice* _fDevIn= NULL, GDevice* _fDevOut= NULL) ;   // default constructor of GAcq object
  ~GAcq() ;

  void GAcqInit();
  void Infos ();
  TString  GetInfo (Int_t* nb_info);
  void SpectraList();
  int  GetStatus(){return fStatus;}
  void SetStatus(int _Status){fStatus =_Status;}
  void SetPrintRC(bool mybool) { fPrintRCbeforeStat =mybool ;}
  
  virtual  void SetTTreeMode   (TTreeMode mode = TREE_NO, const char* filename = "MyTTree",       bool reinit = false, bool withrun= false);
  virtual  void InitTTree();
  virtual  void SetScalerMode  (TTreeMode mode = TREE_NO, const char* filename = "MyScalerTTree", bool reinit = false, bool withrun= false);
  virtual  void SetSpectraMode (int mode=0,char* para_name=NULL, int nb_spec =0,TString family="Raw",Int_t auto_level=1);
  virtual  void SetUserMode    (int mode = 0);
  virtual  void SetCompressionLevel(int level =1){fTheTree->SetCompressionLevel (level);};

  virtual  void SetConfGanilAcq(int mode = 0);
  virtual  void SetInfoCont    (int mode=0);
  virtual  void SetConditionStop(int stop){fCondition_of_stop=stop;};
  virtual  void SetStop(bool stop);
  virtual  void SetInfiniteRead(Bool_t infinite =true){fInfiniteRead =infinite;}
  virtual  void SetOut(Bool_t  out);

  virtual  void SetEventUSleep(int evtusleep);
  virtual  void SetEventSleep(int evtsleep);

  virtual void Pause(int timeout) ;
  virtual void ResumePause() ;
  virtual void IsOnPause();
  virtual void IsOnSleep();

  virtual  void SetNetMode(Int_t mode=0,Int_t port=9090);
  virtual  void SetDevIn  (GDevice* _fDevIn);
  virtual  void SetDevOut (GDevice* _fDevOut);
  virtual  void SetCoups(int coups){fCoups= coups;}
  virtual  void DumpBuffer(char c = 'b');
  virtual  void DumpBuffer(char* c);
  virtual  void DumpEvent(char c = 'b', char mode ='d', bool nozero =true,bool ctrl =true);
  virtual  void DumpEvent(char* c ,char* mode ,bool nozero =true,bool ctrl =false);
  virtual  void DumpCurrentEvent(char mode,  bool nozero,bool ctrl);
  virtual  void DumpScaler();
  virtual  void SpeSave  (const char* filename );

  // bool Next(void);

  virtual  void Stat(bool verb = false);
  virtual  void EventInit(char* Exp_Name =(char*)"local",char * type=(char*)"ganil", bool creation =true,bool noactionfile=false);
  virtual  void DumpParameterName(void) ;

  virtual void DoRunT(UInt_t nEvents=0,UInt_t nBuffers=0); // it is a DoRun with a theaded treatment
  virtual void DoRun(UInt_t  nEvents=0,UInt_t nBuffers=0);
  virtual void DoRunWait(UInt_t nEvents =1, UInt_t nBuffers =0);
  virtual void SkipFillTree(){ fTheTree->SetSkipFillTree(false); };

  virtual void StopAcq  ();  // stoppe les threads s'ils sont en cours d'execution

  virtual DataParameters* GetDataParameters() { return fDataParameters; };
  virtual GEventBase*     GetEvent()          { return fEvent; };
  virtual void            SetEvent(GEventBase* event) {fEvent =event; };

  virtual UShort_t *GetEventArray(){return GetEvent()->GetArray();}
  virtual UShort_t  GetEventArray_Value(UShort_t position){return GetEventArray()[position];}
  virtual Int_t     GetEventArraySize(){return GetEvent()->GetArraySize();}
  virtual UShort_t  *GetEventArrayLabelValue(){return GetEvent()->GetArrayLabelValue();}
  virtual Int_t     GetEventArrayLabelValueSize(){return GetEvent()->GetArrayLabelValueSize();}
  virtual UShort_t  GetEventArrayLabelValue_Label(UShort_t position){return GetEvent()->GetArrayLabelValue_Label(position);};
  virtual UShort_t  GetEventArrayLabelValue_Value(UShort_t position){return GetEvent()->GetArrayLabelValue_Value(position);};
  virtual UShort_t  GetEventArrayLabelValueSizeMax(){return GetEvent()->GetArrayLabelValueSizeMax();};
  virtual bool      GetStop(){return fStop ;};
  virtual TTree*    GetTree(){return fTheTree->GetTree();}
   void      SetTree(GTTree* t ){fTheTree =t; cout << "Set GAcq :: TTree " << fTheTree <<"\n";}
  virtual TFile*    GetTreeFile(){return  fTheTree->GetTreeFile();}
  virtual Float_t   GetSucces();
  virtual Int_t     GetNumEvent();
  virtual Int_t     GetNumBuffer(){return fBufferCount;};
  virtual Float_t   GetRateEvent();
  virtual Float_t   GetRateBuffer();
  virtual Bool_t    GetInEvtTreatment() {return fInEvtTreatment;}
  virtual TCondition * GetCondition1(){ return fMyCondition1;}
  

  virtual void EndUser(){};

 // virtual GNetServerRoot* GetSpeServ(){return fNet;};
  virtual GScaler*        GetScaler(){return fScaler;};


#ifndef GT
  virtual GSpectra*        GetSpectra()             { return fSpectra; };
#endif
  virtual GDevice*         GetDeviceIn()            { return fDevIn; };
  virtual GDevice*         GetDeviceOut()           { return fDevOut;};
  virtual Int_t            GetRunNumber() const     { return fRunNumber; };
  virtual void             SetRunNumber(Int_t _run) { fRunNumber = _run; };
  virtual Bool_t           GetNextEvent();

  virtual void InitUser(){};
  virtual void InitUserRun(){};
  virtual void User(){};
  virtual void EndUserRun(){}; 
  virtual void InitTTreeUser(){};
  //virtual  void VigruLaunch();

  virtual  void TestLaunch();

 protected:

private:
static void TestNew(void* arg);
static void FillTree(void* arg);
//static void VigruNew(void* arg);

protected:
virtual void StopTree();

 bool GetRunStatus(bool verb=false);

private:
 virtual void ToDoInCaseOfInterrupt();

 public:

 ClassDef (GAcq ,1); // Controle of Data Flow
};

#endif

