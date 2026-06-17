
#ifndef __Paris__
#define __Paris__

#include <string>
#include <map>

#ifndef __DETECTOR__
#include "TDetector.h"
#endif

#ifndef __ParisDATA__
#include "TParisData.h"
#endif

#include <MFMParisFrame.h>
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include <TObject.h>
#include "TRandom.h"
#include <TCutG.h>
#include "TVector3.h"
#include "TObjString.h"


class TParis : public TDetector {
  public:
   TParis(bool bspec);
   int counter;;
   virtual ~TParis();
   bool  BoolSpec,LUTBool,CalibDone,debug;
   char Cname[50];
   unsigned long long PrevTS,duplicatedEventC, EventNoTref, MergerCounter,EventWithTref; 
   int prevBoard, prevChannel,tag;
   double prevQ,prevQ2;
   float ClusterAddB[9],ClusterCalo[9],Beta;
   int BoardCounter[6][16];
   int MapFinger[6][16];
   int Tref;
   bool TrefExist;
   TVector3 v1[80];
   unsigned  long long TrefError;
   TH1F * fMyHistoParisQShort[80];
   TH1F * fMyHistoParisQLong[80];
   TH1F * fMyHistoParisCfd[80];
   TH1F * fMyHistoParisQLaBr3[80];
   TH1F * fMyHistoParisQNaI[80];
   TH1F * fMyHistoParisQLongCompton[80];
   TH1F * fMyHistoParisQLaBr3Sum;
   TH1F * fMyHistoParisQLaBr3SumDC;
   TH2F * fMyHistoParisQShortGG;
   TH2F * fMyHistoParisQShortGGTimeGated;
   TH2F * fMyHistoParisCaloFold;
   TH1F * fMyHistoParisAddBack;
   TH1F * fMyHistoParisCalo;
   TH1F * fMyHistoParisComptonRec;
   TH2F * fMyHistoParisMerit;
   TH1F * fMyHistoParisMul;
   TH2F * NeutronPattern;
   TH1F * TimeStampDiffQLaBr3;
   TH2F * TimeStampDiffQLaBr3Eg;
   TH1F * TimeStampDiffQNaI;
   TH1F * TimeStampDiffAll;
   TH1F * TimeStampDiffTref;
   TH1F * IndvTimeStampDiffTref[80];
   TH2F * TimeStampDiffTrefPattern;
   TH2F * TimeStampDiffTrefEg;
   TH2F * fMyHistoParisQShortQLong[80];
   TH2F * fMyHistoParisQShortQLongCond[80];
   TH2F * fMyHistoParisCompton[80];

   TH2F * fMyHistoParisThetaPhi;
   
   TH2F * fMyHistoParisPattQShort2d;
   TH2F * fMyHistoParisPattQLong2d;
   TH2F * fMyHistoParisPattCfd2d;


   Int_t Current_Numexo2_cfg_Board[255];
   Int_t Channel_in_Cluster[255];
   
   Float_t Paris_Angle[80][2];
   Float_t ECoefQLong[80][3];
   Float_t ECoefQShort[80][3];
   Float_t ECoefQNaI[80][3];
   Float_t TCfD[80];
   
   TObjArray HListParis;
   
   // virtual methods from TDetector
   virtual bool Init(DataParameters*);
   virtual bool InitNumexo2(Char_t*);
   virtual bool Clear();
   virtual bool Is(UShort_t, Short_t);
   virtual bool IsMFMParis(MFMParisFrame*);
   virtual bool Treat(); 
   virtual bool InitCal();
   virtual bool ReadCal();
   virtual bool Counter();
   virtual bool ClearCounter();
   virtual double Cal(UShort_t, float , float , float);
   virtual double CalI(int, float , float , float);
   virtual float Doppler_Correction(float , float , float , float , float , float );
   virtual bool SetBeta(float);
   virtual bool SpectraConstructor();   
   virtual void InitBranch(TTree*);

   // getters and setters
   TParisData*	GetParisData() const {return fParisData;}


  private:
   // Data class for Paris
   TParisData  *fParisData;

   ClassDef(TParis,1)  // Paris detector structure
};

#endif
