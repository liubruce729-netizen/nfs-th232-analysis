// File :  GUser.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GUser
//
// This class mange user methodes
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


#ifndef __GUser__
#define __GUser__

//#define UseAnv


#include <iostream>
#include <sstream>
#include <vector>
using std::ostream;
#include "Riostream.h"
#include <TRint.h>
#include "General.h"
#include "GAcq.h"
#include "GDevice.h"
#include "TSystem.h"
#include <TObject.h>
#include <TString.h>
#include <TH1.h>
#include <TH3.h>
#include <TH2.h>
#include "GParaCaliXml.h"
#include "GNetServerRoot.h"
//#include "GNetServerSoap.h"
//#include "GNetClientNarval.h"
#include "TObjString.h"
#include "TDatime.h"

#include <TGPicture.h>
#include <TGMenu.h>
#include <TGImageMap.h>
#include <TGMsgBox.h>
#include <yaml-cpp/yaml.h>

#include "./TDetector.h"
#include "./TExogam.h"
#include "./TExogam2.h"
#include "./TExogam2REA.h"
#include "./TExogam2Data.h"
#include "./TExogam2READata.h"
#include "./TTrigger.h"
#include "./TTriggerData.h"
#include "./TExogamData.h"
#include "./TMW.h"
#include "./TMWData.h"
#include "./TCsI.h"
#include "./TCsIData.h"
#include "./TParis.h"
#include "./TParisData.h"
#include "./TNWall.h"
#include "./TNWallData.h"
#include "./TGeneric.h"
#include "./TGenericData.h"
#include "./TEbyE.h"
#include "./TEbyEData.h"
#include "./TVamosIC.h"
#include "./TVamosICData.h"



#include <MFMMergeFrame.h>
#include <MFMExogamFrame.h>
#include <MFMDiamantFrame.h>
#include <MFMNedaFrame.h>
#include <MFMParisFrame.h>
#include <MFMNedaCompFrame.h>
#include <MFMCommonFrame.h>
#include <MFMEbyedatFrame.h>
#include <MFMReaGenericFrame.h>
#include <MFMVamosICFrame.h>

using namespace std;
//____________________

//_________________________________________________________________________________________

class GUser : public  GAcq{

 
 protected:

  int fMyLabel;
  TString fMyParameterName;
  
  MFMMergeFrame * pMergeFrame ;
  MFMMergeFrame * pInceptionMergeFrame[100] ;
  MFMExogamFrame * pExogamFrame ;
  MFMCommonFrame * pCommonFrame;
  MFMCommonFrame *   fInsideframe;
  MFMCommonFrame *   fInceptionInsideframe[100];
  MFMEbyedatFrame * pEbyedatFrame;
  MFMNedaFrame * pNedaFrame ;
  MFMNedaCompFrame * pNedaCompFrame ;
  MFMDiamantFrame * pDiamantFrame ;
  MFMParisFrame * pParisFrame ;
  MFMReaGenericFrame * pGenericFrame ;
  MFMVamosICFrame * pVamosICFrame ;
  
  
  
  GSpectra* MySpectraList  ;
  DataParameters *lparams;	
  //GNetServerRoot* MySpectraServer ;
  struct SysInfo_t infos;
  struct MemInfo_t meminfos;
  struct CpuInfo_t cpuinfos;	
  int PreviousTime;
  int InceptionLayer;
  unsigned  long long CMFM_MERGE_TS_FRAME_TYPE,CMFM_EXO2_FRAME_TYPE,CMFM_EXO2REA_FRAME_TYPE,CMFM_EBY_EN_FRAME_TYPE,CMFM_EBY_TS_FRAME_TYPE,CMFM_EBY_EN_TS_FRAME_TYPE,CMFM_NEDA_FRAME_TYPE,CMFM_DIAMANT_FRAME_TYPE,CMFM_NEDACOMPRESS_FRAME_TYPE, CMFM_PARIS_FRAME_TYPE, CMFM_GENERIC_FRAME_TYPE,CMFM_VAMOSIC_FRAME_TYPE ;
  int TSZero;
  int nbinsideframe, noevent;
  int Inceptionnbinsideframe[100], Inceptionnoevent[100];
  uint64_t PrevTS, TStart;
  uint64_t PrevTSInception[100];
  bool debug;
  bool fNfsExoAnaTree;
  bool fNfsExoAnaSpec;
  bool fNfsExoAnaRawTree;
  bool fNfsExoAnaCrystalTimeCorrection;
  TString fNfsExoAnaCorrectionPath;
  std::vector<TString> fNfsExoAnaDisabledCrystals;
  Double_t fStartTimeSeconds;
  Bool_t fUseMaxTimeWindow;
  Double_t fMaxTimeSeconds;
  UInt_t fMaxEventsToProcess;
  UInt_t fProcessedEventsInCurrentRun;
  ULong64_t fRunTimeZeroTS;
  Bool_t fRunTimeZeroValid;
  Bool_t fTimeWindowStarted;
  Bool_t fTimeWindowStopPrinted;
  TH1F *MergeFrameDeltaTS;
  TH1F *MergeDetectorFrameDeltaTS;
  TH1F *MFMKey;
  TH1F *DTS_test;
  TH1F *DTS_test2;
  TH1F *DTS_test3;
  TH2F *ExogamAlignement;
  TH1F *rate;
  
  TDatime date;
  bool Exogam2b,Exogam2REAb,Exogamb,Triggerb,MWb,Tacb,eventOK,nowcheck,CsIb,NWallb,Parisb, Genericb, EbyEb, VamosICb ;
  private:
   TExogam	*fExogam;
   TExogam2	*fExogam2;
   TExogam2REA	*fExogam2REA;
   TTrigger	*fTrigger;
   TMW		*fMW;
   TCsI		*fCsI;
   TNWall	*fNeda;
   TParis	*fParis;
   TGeneric     *fGeneric;
   TEbyE        *fEbyE;
   TVamosIC     *fVamosIC;
   
   
   //TGeneric     *fGeneric[10];
 public:
   
  GUser(GDevice* _fDevIn= NULL, GDevice* _fDevOut= NULL) ;   // default constructor of GUser object 
  ~GUser() ; 
  bool IsNFSExoAnaTreeEnabled() const { return fNfsExoAnaTree; }
  bool IsNFSExoAnaSpecEnabled() const { return fNfsExoAnaSpec; }
  bool IsNFSExoAnaRawTreeEnabled() const { return fNfsExoAnaRawTree; }
  bool IsNFSExoAnaCrystalTimeCorrectionEnabled() const { return fNfsExoAnaCrystalTimeCorrection; }
  TString GetNFSExoAnaCorrectionPath() const { return fNfsExoAnaCorrectionPath; }
  void SetRunTimeWindow(Double_t startTimeSeconds, UInt_t maxEvents, Bool_t useMaxTimeWindow, Double_t maxTimeSeconds);
  
  int eventcounter,tvb;
  int MFMtype;
  bool TreeFillBool,MakeTreeOnly, SpyOnly;
  TFile *fileTree2;
  TTree *LocalTree;
  bool NfsTreeFillBool;
  TFile *fileNfsTree;
  TTree *NfsTree;
  bool NfsMult3TreeFillBool;
  TFile *fileNfsMult3Tree;
  TTree *NfsMult3Tree;
  bool RawTreeFillBool;
  TTree *RawTree;
  ULong64_t fRawEventCounter;
  ULong64_t fRawEventIndex;
  Int_t fRawTopFrameType;
  Int_t fRawTopDataSource;
  Int_t fRawTopFrameSize;
  Int_t fRawTopNbItems;
  UInt_t fRawTopEventNumber;
  ULong64_t fRawTopTimeStamp;
  std::vector<UChar_t> fRawEventBytes;
  std::vector<Int_t> fRawFrameDepth;
  std::vector<Int_t> fRawFrameIndexInParent;
  std::vector<Int_t> fRawFrameType;
  std::vector<Int_t> fRawFrameDataSource;
  std::vector<Int_t> fRawFrameRevision;
  std::vector<Int_t> fRawFrameSize;
  std::vector<Int_t> fRawFrameHeaderSize;
  std::vector<Int_t> fRawFrameUnitBlockSize;
  std::vector<Int_t> fRawFrameNbItems;
  std::vector<Int_t> fRawFrameBoardId;
  std::vector<Int_t> fRawFrameChannelId;
  std::vector<UInt_t> fRawFrameEventNumber;
  std::vector<ULong64_t> fRawFrameTimeStamp;
  std::vector<Int_t> fRawExoCristalId;
  std::vector<Int_t> fRawExoTGCristalId;
  std::vector<Int_t> fRawExoBoardId;
  std::vector<Int_t> fRawExoStatus1;
  std::vector<Int_t> fRawExoStatus2;
  std::vector<Int_t> fRawExoStatus3;
  std::vector<Int_t> fRawExoDeltaT;
  std::vector<Int_t> fRawExoInnerM6;
  std::vector<Int_t> fRawExoInnerM20;
  std::vector<Int_t> fRawExoOuter1;
  std::vector<Int_t> fRawExoOuter2;
  std::vector<Int_t> fRawExoOuter3;
  std::vector<Int_t> fRawExoOuter4;
  std::vector<Int_t> fRawExoBGO;
  std::vector<Int_t> fRawExoCSI;
  std::vector<Int_t> fRawExoInnerT30;
  std::vector<Int_t> fRawExoInnerT60;
  std::vector<Int_t> fRawExoInnerT90;
  std::vector<Int_t> fRawExoPadding;
  std::vector<Int_t> fRawDiamantCristalId;
  std::vector<Int_t> fRawDiamantBoardId;
  std::vector<Int_t> fRawDiamantChannelId;
  std::vector<Int_t> fRawDiamantStatus1;
  std::vector<Int_t> fRawDiamantStatus2;
  std::vector<Int_t> fRawDiamantEnergy;
  std::vector<Int_t> fRawDiamantTop;
  std::vector<Int_t> fRawNedaBoardId;
  std::vector<Int_t> fRawNedaChannelId;
  std::vector<Int_t> fRawNedaLocationId;
  std::vector<Int_t> fRawNedaLeInterval;
  std::vector<Int_t> fRawNedaZcoInterval;
  std::vector<Int_t> fRawNedaTdcValue;
  std::vector<Int_t> fRawNedaSlowIntegral;
  std::vector<Int_t> fRawNedaFastIntegral;
  std::vector<Int_t> fRawNedaBitfield;
  std::vector<Int_t> fRawNedaAbsMax;
  std::vector<Int_t> fRawNedaInterpolCFD;
  std::vector<Int_t> fRawNedaCompEnergy;
  std::vector<Int_t> fRawNedaCompTime;
  std::vector<Int_t> fRawNedaCompTdcCorValue;
  std::vector<Int_t> fRawNedaCompSlowIntegral;
  std::vector<Int_t> fRawNedaCompFastIntegral;
  std::vector<Int_t> fRawNedaCompIntRaiseTime;
  std::vector<Int_t> fRawNedaCompNeuralNetWork;
  std::vector<Int_t> fRawNedaCompNbZero;
  std::vector<Int_t> fRawNedaCompNeutronFlag;
  std::vector<Int_t> fRawParisCristalId;
  std::vector<Int_t> fRawParisBoardId;
  std::vector<Int_t> fRawParisChannelId;
  std::vector<Int_t> fRawParisQShort;
  std::vector<Int_t> fRawParisQLong;
  std::vector<Float_t> fRawParisCfd;
  std::vector<Int_t> fRawParisPLL;
  std::vector<Int_t> fRawParisPUR;
  std::vector<Int_t> fRawParisOVR;
  std::vector<Int_t> fRawGenericCristalId;
  std::vector<Int_t> fRawGenericBoardId;
  std::vector<Int_t> fRawGenericChannelId;
  std::vector<Int_t> fRawGenericStatus1;
  std::vector<Int_t> fRawGenericStatus2;
  std::vector<Int_t> fRawGenericTypeTns;
  std::vector<Int_t> fRawGenericEnergy;
  std::vector<Int_t> fRawGenericTime;
  std::vector<Int_t> fRawVamosCristalId;
  std::vector<Int_t> fRawVamosBoardId;
  std::vector<Int_t> fRawVamosChannelId;
  std::vector<Int_t> fRawVamosStatus1;
  std::vector<Int_t> fRawVamosStatus2;
  std::vector<Int_t> fRawVamosEnergy;
  std::vector<Int_t> fRawEbyEFrameIndex;
  std::vector<Int_t> fRawEbyELabel;
  std::vector<Int_t> fRawEbyEValue;
  
  virtual void InitUser();
  virtual void InitUserRun();
  virtual void User();
  virtual void EndUserRun();
  virtual void Unpack(MFMCommonFrame*, bool);
  virtual void Analysis();
  void EndUser();
  virtual void OnlyTreeConversion(bool);
  virtual void LoadCut(bool);
  virtual void InitTTreeUser(Char_t*, bool, bool, bool, bool, bool, bool, bool, bool, bool, bool, bool);  //exogam2, trigger, MW, CsI; Neda, Paris, Generic, EbyEdat, exogam2_rea, VamosIC, raw_tree;
  virtual TString BuildNfsTreeFileName(Char_t*);
  virtual TString BuildNfsMult3TreeFileName(Char_t*);
  virtual void WhichMFM(uint16_t);
  virtual void InitRawTreeBranches();
  virtual void ClearRawTreeEvent();
  virtual void CaptureRawEvent(MFMCommonFrame*);
  virtual void CaptureRawFrame(MFMCommonFrame*, Int_t, Int_t);
  virtual void PushRawDetectorDefaults();
  ClassDef (GUser ,1); // User Treatment of Data
  
};

#endif
 
