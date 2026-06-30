// File : GEventMFM.h
// Author: Luc Legeard  (spring 2004)
//////////////////////////////////////////////////////////////////////////////
//
// Class GEventMFM
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

#ifndef __GEventMFM__
#define __GEventMFM__

#include "General.h"
#include "GBuffer.h"
#include "GEventBase.h"
#include "GEN_TYPE.H"
#include "DataParameters.h"
#include "MFMAllFrames.h"
//#include "math.h"

#include <sstream>
//______________________________________________________________________________________


class GEventMFM : public GEventBase{

 protected:

  uint16_t        *pEventBrutData; //! Brut data event
  bool            fLipflop;
  bool            fInit_event_done; // Flag to know if initialisation of event have been realised

  MFMBasicFrame   *pBasicFrame;
  MFMCoboFrame    *pCoboFrame;
  MFMCommonFrame  *pCommonFrame;
  MFMExogamFrame  *pExogamFrame;
  MFMCommonFrame  *pCurrentFrame;
  MFMEbyedatFrame *pEbyedatFrame;
  MFMRibfFrame    *pRibfFrame;
  MFMOscilloFrame *pOscilloFrame;
  MFMMergeFrame   *pMergeFrame;
  MFMScalerDataFrame   *pScalerFrame;
  MFMMutantFrame   *pMutantFrame;
  MFMHelloFrame    *pHelloFrame;
  MFMChimeraFrame  *pChimeraFrame;
  MFMBoxDiagFrame  *pBoxDiagFrame;
  MFMVamosICFrame  *pVamosICFrame;
  MFMVamosPDFrame  *pVamosPDFrame;
  MFMVamosTACFrame  *pVamosTACFrame;
  MFMNedaFrame     *pNedaFrame;
  MFMNedaCompFrame *pNedaCompFrame;
  MFMDiamantFrame  *pDiamantFrame;
  MFMS3BaF2Frame * pS3BaF2Frame;
  MFMS3AlphaFrame * pS3AlphaFrame;
  MFMS3RuthFrame * pS3RuthFrame;
  MFMS3eGUNFrame * pS3eGUNFrame;
  MFMS3SynchroFrame * pS3SynchroFrame;
  MFMReaGenericFrame * pReaGenericFrame;
  MFMReaTraceFrame * pReaTraceFrame;
  MFMSiriusFrame * pSiriusFrame;
  MFMS3DeflectorFrame * pDeflectorFrame;
  MFMParisFrame * pParisFrame;
 private:
  int             fFrameType;

 public :

  GEventMFM(DataParameters* parameter);
  ~GEventMFM();

  virtual  int  NextEvent(GBuffer* _buffer);
  virtual  int  ReadNextEvent(GBuffer* _buffer);

  virtual  void ClearData(int value =-1);

  //virtual  void EventInit(char*name);

  bool IsRaz();
  virtual void FillEvent(int enventtype,long long timestamp, int number);
  virtual void MakeEventHeader(int type,int nbpara =0);
  virtual void FillBufferWithEvent(GBuffer* buffer);
  virtual void DumpMadeEvent(); // DO dump of a generated event
  virtual MFMBasicFrame  * GetFrame(){return  (MFMBasicFrame  *)pCurrentFrame;};
  virtual Int_t GetEventNumber();
  virtual Int_t GetFrameType(){return fFrameType;};
//TODO
// methods just to keep compatibility with GEvent
  virtual void      RazEvent();
  virtual  void     EventInitWithFileName(char* actionFilePAR, char* actionFileSTR);

  virtual Int_t     GetArrayLabelValueSize();
  virtual uint16_t  GetArrayLabelValue_Label(uint16_t position);
  virtual uint16_t  GetArrayLabelValue_Value(uint16_t position);
  virtual Int_t     GetArrayLabelValueSizeMax(){return 0;};
  virtual uint16_t  *GetArrayLabelValue(){return NULL;};
  virtual TString   GetDumpArray(char mode= 'd',bool nozero=true){TString toto = "bidon";return toto;};
  virtual TString   GetDumpEvent(char mode= 'd');
  virtual TString   GetDumpHeader();


  virtual long long   GetTimeStamp();
  virtual int       GetNbItems();
  virtual void      GetItem(int i);
//  virtual void      CoboGetParameters(int i,unsigned int * sample,unsigned int *buckidx,unsigned int *chanidx,unsigned int *agetidx);
  virtual int       EventUnravelling(void) ;

  ClassDef (GEventMFM ,1) // Data Event

};
#endif
