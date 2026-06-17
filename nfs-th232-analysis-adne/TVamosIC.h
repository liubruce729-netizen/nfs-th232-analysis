
#ifndef __VamosIC__
#define __VamosIC__

#include <string>
#include <map>

#ifndef __DETECTOR__
#include "TDetector.h"
#endif

#ifndef __VamosICDATA__
#include "TVamosICData.h"
#endif

#include <MFMVamosICFrame.h>
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include <TObject.h>
#include "TRandom.h"
#include <TCutG.h>
#include "TObjString.h"




class TVamosIC : public TDetector {
  public:
   TVamosIC(bool bspec );
   int counter;
   virtual ~TVamosIC();
   bool  BoolSpec,LUTBool,CalibDone,debug, TrueIons, PrompIons,CutLoaded;
   char Cname[50];
   unsigned long long PrevTS,duplicatedEventC; 
   int prevBoard, prevChannel,tag;
   double prevQ,prevQ2;
   int Muliplicity;
   int NucleusId;
   TH1F * fMyHistoVamosICE[1000];
   TH2F * fMyHistoVamosICE2D;
   TH2F * fMyHistoVamosICECorrel;
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
   
   
   TObjArray HListVamosIC;
   
   // virtual methods from TDetector
   virtual bool Init(DataParameters*);
   virtual bool InitNumexo2(Char_t*);
   virtual bool Clear();
   virtual bool Is(UShort_t, Short_t);
   virtual bool IsMFMVamosIC(MFMVamosICFrame*);
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
   TVamosICData*	GetVamosICData() const {return fVamosICData;}


  private:
   // Data class for VamosIC
   TVamosICData  *fVamosICData;

   ClassDef(TVamosIC,1)  // VamosIC detector structure
};

#endif
