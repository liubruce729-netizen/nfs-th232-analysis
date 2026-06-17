
#ifndef __EbyE__
#define __EbyE__

#include <string>
#include <map>

#ifndef __DETECTOR__
#include "TDetector.h"
#endif

#ifndef __EbyEDATA__
#include "TEbyEData.h"
#endif

#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include <TObject.h>
#include "TRandom.h"
#include <TCutG.h>
#include "TVector3.h"
#include "TObjString.h"


class TEbyE : public TDetector {
  public:
   TEbyE(bool bspec);
   int counter;
   virtual ~TEbyE();
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
   unsigned  long long TrefError;
   std::vector<int> matchFlags; // Tableau pour stocker les indicateurs de match
   
   TObjArray HListEbyE;
   TH2I * fMyHistoEbyE2d;
   TH1F * fMyHistoEbySum;
   TH1F * fMyHistoEbylabel;
   int GMT_Trigger, MMMul;
   
   // virtual methods from TDetector
   virtual bool Init(DataParameters*);
   virtual bool InitNumexo2(Char_t*);
   virtual bool Clear();
   virtual bool Is(UShort_t, Short_t);
   virtual bool IsMFMEbyE(UShort_t, UShort_t);
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
   TEbyEData*	GetEbyEData() const {return fEbyEData;}


  private:
   // Data class for EbyE
   TEbyEData  *fEbyEData;

   ClassDef(TEbyE,1)  // EbyE detector structure
};

#endif
