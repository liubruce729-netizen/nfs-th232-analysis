#ifndef __DETECTOR__
#define __DETECTOR__

#include <string>
#include <map>

#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TCutG.h"
#include "TFile.h"
#include <TObject.h>
#include "TRandom.h"
#include "DataParameters.h"
#include <MFMExogamFrame.h>
#include "TObjString.h"

class TDetector : public TObject {
  public:
   TDetector();
   virtual ~TDetector();

   virtual bool Init(DataParameters*)        = 0;
   virtual bool InitNumexo2(Char_t*)	      = 0;
   virtual bool Clear()                       = 0;
   virtual bool Is(UShort_t, Short_t)         = 0; 
   virtual bool Treat()                       = 0;
   virtual bool InitCal() 		      = 0;
   virtual bool ReadCal() 		      = 0;
   virtual double Cal(UShort_t, float , float , float ) 		      = 0;
   virtual bool  SpectraConstructor()		=0;
   virtual void InitBranch(TTree*)            = 0;
   

  public:
   string GetLabelMap(int i)	{return fLabelMap[i];}
	
  protected:
    map<int, string>	fLabelMap;
    map<int, int>	fTypeMap;
    map<int, int>	fParameterMap;
    static long long fTimestamp;   //Static memeber to make histograms that traces channels as a function of time
    static long long fTimestampT0; //To subtract time "zero"
   ClassDef(TDetector,1)  // Detector abstract class
};

#endif
