
#ifndef __Generic__
#define __Generic__

#include <string>
#include <map>

#ifndef __DETECTOR__
#include "TDetector.h"
#endif

#ifndef __GenericDATA__
#include "TGenericData.h"
#endif

#include <MFMReaGenericFrame.h>
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include <TObject.h>
#include "TRandom.h"
#include <TCutG.h>
#include "TObjString.h"




class TGeneric : public TDetector {
  public:
   TGeneric(bool bspec ,int Topology );
   int counter;
   virtual ~TGeneric();
   bool  BoolSpec,LUTBool,CalibDone,debug, TrueIons, PrompIons,CutLoaded;
   char Cname[50];
   unsigned long long PrevTS,duplicatedEventC; 
   int prevBoard, prevChannel,tag;
   double prevQ,prevQ2;
   int MuliplicitySec, MultiplicityChio, MultiplicityPlastic;
   int NucleusId;
   TH1F * fMyHistoGenericE[1000];
   TH1F * fMyHistoGenericT[1000];
   TH1F * fMyHistoGenericMul;
   TH1F * TimeStampDiff;
   TH2F * fMyHistoGenericPattE;
   TH2F * fMyHistoGenericPattT;
   TH2F * fMyHistoGenericDEE;
   TH1F * fMyHistoGenericDE;
   TH2F * fMyHistoGenericDEECond;
   TH2F * fMyHistoGenericDETOF;
   TH2F * fMyHistoGenericDETOF_LISE;
   TH2F * fMyHistoGenericDETOF_LISEcond;
   TH1F * TransverseMomentum;
   TH1F * fMyHistoGenericTT ;
   TH2F * fMyHistoGenericEE;
   
   
   TH1F *trackI[100];
   int Ic[100];
   unsigned long long G_TS,G_TS2;
   float FreeParam1, FreeParam2;
   
   /*
   Ions identification
   */
   TCutG *gcut_Array[100];
   int NombreIons, IonsCounter[100];
   unsigned long long  TS_IONS_Start[100], TS_IONS_Stop[100];
   
   
   
   Int_t Current_Numexo2_cfg_Board[255];
   Int_t Channel_in_Cluster[255];
   
   Float_t ECoef[1000][3];
   
   TObjArray HListGeneric;
   
   // virtual methods from TDetector
   virtual bool Init(DataParameters*);
   virtual bool InitNumexo2(Char_t*);
   virtual bool Clear();
   virtual bool Is(UShort_t, Short_t);
   virtual bool IsMFMGeneric(MFMReaGenericFrame*);
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
   TGenericData*	GetGenericData() const {return fGenericData;}


  private:
   // Data class for Generic
   TGenericData  *fGenericData;

   ClassDef(TGeneric,1)  // Generic detector structure
};

#endif
