#ifndef __TRIGGER__
#define __TRIGGER__

#include <string>
#include <map>

#ifndef __DETECTOR__
#include "TDetector.h"
#endif

#ifndef __TRIGGERDATA__
#include "TTriggerData.h"
#endif

#include <MFMExogamFrame.h>
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TObjString.h"

#define	TRIG_1	1 // VAMOS TGV
#define	TRIG_2	2 // VAMOS GATCONF
#define	TRIG_3	3 //VAMOS AGAVA = TGV CONFDEC
#define	TRIG_4	4 //Trig_EvNumLSB
#define	AGAVA   200 //AGAVA DATA

#define	AGAVA_TRIG	5 //AGAVA_TRIG-------------------******
#define	AGAVA_STAT	6 //AGAVA_STAT-------------------***********
#define	AGAVA_REJ 	7 //AGAVA_REJ----------------******
#define	AGAVA_VAL 	8 //AGAVA_VAL----------------------*************
#define	AGAVA_TRIG_UP	9 //AGAVA_TRIG_UP------------------*********
#define	AGAVA_REJ_UP  	10 //AGAVA_REJ_UP---------------*********
#define	AGAVA_VAL_UP  	11 //AGAVA_VAL_UP-----------------********
#define	AGAVA_STAT_UP  	12//AGAVA_STAT_UP-----------********
#define	AGAVA_LTAGH 	13 //AGAVA_LTAGH-------------******	
#define	AGAVA_LTAGL_UP 	14 //AGAVA_LTAGL_UP-------------******  
#define	AGAVA_LTAGL 	15 //AGAVA_LTAGL----------********	
#define	AGAVA_RTAGH 	16 //AGAVA_RTAGH-----------------*******	
#define	AGAVA_RTAGL_UP 	17 //AGAVA_RTAGL_UP-----  **********
#define	AGAVA_RTAGL 	18 //AGAVA_RTAGL------------	********
#define	AGAVA_VTAGH 	19 //AGAVA_VTAGH-----	*****
#define	AGAVA_VTAGL_UP 	20 //AGAVA_VTAGL_UP------  ******
#define	AGAVA_VTAGL 	21 //AGAVA_VTAGL----------*******	
#define	AGAVA_EVENT_UP 	22 //AGAVA_EVENT_UP--------- ********** 
#define	AGAVA_EVENT 	23 //AGAVA_EVENT----------*********		
#define	AGAVA_LTRIG_UP 	24 //AGAVA_LTRIG_UP----------*****  
#define	AGAVA_LTRIG 	25 //AGAVA_LTRIG-----------*********	
#define	AGAVA_TOUT_UP 	26 //AGAVA_TOUT_UP----------*********	
#define	AGAVA_TOUT 	27 //AGAVA_TOUT	-----------------************

#define	Correl	100 //CorrelEvent


/*
The TTrigger class was written to deal with the VXI GMT Trigger data , i.e GATCONF, INCONF parameter.
In autonom mode with the Time Stamper, each "event" contains in the header an event number and a Time Stamp.
Per event the GetEvent()->GetTimeStamps() function return the Time Stamp value. As this parameter is independant
from the detector that fired (EXOGAM, Musett, Vamos), the value is stored in the TTrigger class.


 */


class TTrigger : public TDetector {
  public:
   TTrigger(bool bspec);
   virtual ~TTrigger();
   bool  BoolSpec;
   bool MK2Present,AGAVAPresent;
   UShort_t PrevEventNumbMK2, MK2EvtCounter,MK2EvtCounterTotal;
   unsigned long long PrevTS;
   int ExoBufferAbscent,ExoBufferAbscentCompt, tgv,gmt,agava,AGAVABufferAbscentCompt;
    TH1F * centEXO;
    TH1F * AGAVA_Status;
    TH2I * fMyHistoTriggerCorr;
    TH1F * TimeStampDiff;
    bool centrum0b,agava0b;
   // virtual methods from TDetector
   virtual bool Init(DataParameters*);
   virtual bool InitNumexo2(Char_t*);
   virtual bool Clear();
   virtual bool Is(UShort_t, Short_t);
   //virtual bool IsMFMExo(MFMExogamFrame*);
   virtual bool Treat();
   virtual bool InitCal();
   virtual bool ReadCal();
   virtual bool SetMK2(bool);
   virtual bool SetAGAVA(bool);
   virtual bool IsAGAVA();
   virtual double Cal(UShort_t, float , float , float);
   virtual bool  SpectraConstructor();
   virtual void InitBranch(TTree*);

   // getters and setters
   TTriggerData*	GetTriggerData() const {return fTriggerData;}

  private:
   // Data class for Trigger
   TTriggerData  *fTriggerData;

   ClassDef(TTrigger,1)  // Trigger structure
};

#endif
