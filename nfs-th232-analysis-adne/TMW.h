#ifndef __MW__
#define __MW__

#include <string>
#include <map>

#ifndef __DETECTOR__
#include "TDetector.h"
#endif

#ifndef __MWDATA__
#include "TMWData.h"
#endif

#include <MFMExogamFrame.h>
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include <TObject.h>
#include "TRandom.h"
#include "TObjString.h"

#define	MW_E	1
#define	TAC	2
#define	BARREL	3
#define	HYBALL	4
#define	MUST2	5
#define	MUST2NRJX 6
#define	MUST2NRJY 7


class TMW : public TDetector {
  public:
   TMW(bool bspec);
   virtual ~TMW();
   bool  BoolSpec;
   TH2F * fMyHistoMW;
   TH1F * fMyHistoVelocity;
   TH1F * fMyHistoBeta;
   TH1F * fMyHistoMWMul;
   TH2F * fMyHistoTacCorr;
   float Path,Velocity; 
   TObjArray HListMW;
   
    Float_t TCoef[20][2]; //Time Calib
   
   // virtual methods from TDetector
   virtual bool Init(DataParameters*);
   virtual bool InitNumexo2(Char_t*);
   virtual bool Clear();
   virtual bool Is(UShort_t, Short_t);
  // virtual bool IsMFMExo(MFMExogamFrame*); 
   virtual bool Treat();
   virtual bool InitCal();
   virtual bool ReadCal();
   virtual bool SetBeta(float);
   virtual double GetBeta();
   virtual double Cal(UShort_t, float , float , float);
   virtual bool SpectraConstructor();
   virtual void InitBranch(TTree*);

   // getters and setters
   TMWData*	GetMWData() const {return fMWData;}

  private:
   // Data class for MW
   TMWData  *fMWData;

   ClassDef(TMW,1)  
};

#endif
