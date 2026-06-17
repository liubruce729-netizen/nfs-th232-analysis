
#ifndef __CsI__
#define __CsI__

#include <string>
#include <map>

#ifndef __DETECTOR__
#include "TDetector.h"
#endif

#ifndef __RINGDATA__
#include "TCsIData.h"
#endif
#include <MFMDiamantFrame.h>
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include <TObject.h>
#include "TRandom.h"
#include <TCutG.h>
#include "TObjString.h"
#include "TCsIData.h"
#include "DataParameters.h"
#define	CsI_E	1
#define	CsI_T	2
#define	CsI_PI	3


class TCsI : public TDetector {
  public:
   TCsI(bool bspec);
   int counter, alpham, protonm, twoprotonm, chargedm, totalalpha, totalproton, totaltwoproton, totalcharged, PID_Index;
   virtual ~TCsI();
   bool  BoolSpec,LUTBool,CutLoaded;
   char Cname[50];
   unsigned long long PrevTS,PrevTSlocal,duplicatedEventC; 
   int prevBoard, prevChannel,tag;
   double prevQ,prevQ2;
   TH1F * fMyHistoCsIE[50];
   TH1F * fMyHistoCsIT[50];
   TH1F * fMyHistoCsIPI[50];
   TH1F * TimeStampDiff;
   TH2F * fMyHistoCsIE_PI[50];
   TH2F * fMyHistoCsIE_T[50];
   TH2F * fMyHistoCsIT_PI[50];

   
   TH2F * fMyHistoCsIPattE2d;
   TH2F * fMyHistoCsIPattT2d;
   TH2F * fMyHistoCsIPattPI2d;

   TH1F * fMyHistoCsIPattE;
   TH1F * fMyHistoCsIPattT;
   TH1F * fMyHistoCsIPattPI;
   
   TH1F * fMyHistoCsIEmult;
   TH1F * fMyHistoCsITmult;
   TH1F * fMyHistoCsIPImult;
   
   TH1F * fMyHistoCsIDataCheckET;
   TH1F * fMyHistoCsIDataCheckEPI;
   TH1F * fMyHistoCsIDataCheckTPI;
   

   //Stores the CUTS on the Energy vs PID Cuts
   TCutG *gcut_AlphaArray[84];
   TCutG *gcut_ProtonArray[84];
   TCutG *gcut_TwoProtonArray[84];
   //Used for a veto cut
   TCutG *gcut_ChargedArray[84];
   Int_t Current_Numexo2_cfg_Board[255];
   Int_t GoodCoincET, GoodCoincEPI, GoodCoincTPI;
   Int_t BadCoincET,   BadCoincEPI, BadCoincTPI;

   TObjArray HListCsI;
   
   // virtual methods from TDetector
   virtual bool Init(DataParameters*);
   virtual bool InitNumexo2(Char_t*);
   virtual bool Clear();
   virtual bool Is(UShort_t, Short_t);
   virtual bool IsMFMDiamant(MFMDiamantFrame*);
   virtual bool Treat(); 
   virtual bool InitCal();
   virtual bool ReadCal();
   virtual bool ReadCut();
   virtual bool Counter();
   virtual bool ClearCounter();
   virtual double Cal(UShort_t, float , float , float);
   virtual double CalI(int, float , float , float);
   virtual bool SpectraConstructor();
   virtual void InitBranch(TTree*);

   // getters and setters
   TCsIData*	GetCsIData() const {return fCsIData;}

   int getPID_Index();
   int isParticleVeto();

  private:
   // Data class for CsI
   TCsIData  *fCsIData;
   bool chargeParticleDetected;

   ClassDef(TCsI,1)  // CsI detector structure
};

#endif
