#ifndef __EXOGAM__
#define __EXOGAM__

#include <string>
#include <map>

#ifndef __DETECTOR__
#include "TDetector.h"
#endif

#ifndef __EXOGAMDATA__
#include "TExogamData.h"
#endif
#include "TObjString.h"

#include <MFMExogamFrame.h>
#include "TVector3.h"
#include "TF1.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TMath.h"
#include <TObject.h>
#include "TRandom.h"
#define	ECCE	1
#define	ECCT	2
#define	GOCCEE	3
#define	GOCCET	4
#define GEFILL  5
#define ESSE    6
#define ESST    7 
#define ESSTQ   8 
#define ESSPat  9 
#define InteractionDepth	20.	// mm
#define D_CloveFlange_Targ12	(147.0+7.0)  //mm

class TExogam : public TDetector {
  public:
   TExogam(bool bspec);
   virtual ~TExogam();
   
   bool  BoolSpec;
   //Merge check
   
   short fECCcopy_val;
   TH2F * fMyHistoECCcopy_val;
   // raw spectra
   

   TH2I * fMyHistoNCriERaw;
   TH2I * fMyHistoNCriTRaw;
   TH2I * fMyHistoNSegERaw;

   // calibrated spectra
   TH1F * fMyHistoECCECal[16*100+4*10];
   // ECC vs Timestamp,
   TH1F * fMyHistoECCTCal[16*100+4*10];
   TH1F * fMyHistoECCEVetoed[16*100+4*10];
   TH1F * fMyHistoECCEAccepted[16*100+4*10];
   TH1F * fMyHistoGOCCEE[16*100+10*4+4];
   TH1F * fMyHistoGOCCET[16*100+10*4+4];
   
   TH1F * fMyHistoESS_Energy[16];
   TH1F * fMyHistoESS_Pat[16];
   TH1F * fMyHistoESS_Time[16];
   TH1F * fMyHistoESS_TimeQ[16*100+4*10];
   
   TH2F * fMyHistoECCEESSE[16*100+4*10];	
   
   TH1F * fMyHistoCloverECal[16];
   TH1F * fMyHistoCloverTCal[16];
   
   
   TH1F * fMyHistoCloverECalACAdd_TQ[16];
   TH1F * fMyHistoCloverECalACAdd_TC[16];
   TH1F * fMyHistoCloverECalACAdd[16];
   TH1F * fMyHistoCloverECalACAddDC[16];

   
   
   TH1F * fMyHistoCloverECalACAddRejectF[16];
   TH1F * fMyHistoCrysECalACRejectF[16*100+4*10];

   TH2F * fMyHistoGammaGamma;	

   
   TH1F * fMyHistoSumECCE;
   TH1F * fMyHistoSumECCT;
   TH1F * fMyHistoSumGOCCEE;
   TH1F * fMyHistoSumGOCCET;
   
   TH1F * fMyHistoSumExogam;
   TH1F * fMyHistoSumExogamDC;
   TH1F * fMyHistoSumExogamFoldCond;
   TH1F * fMyHistoSumExogamDCFoldCond;
   TH1F * fMyHistoSumExogamCalorimeter;

   TH2F * fMyHistoSumExogam2D;
   TH2F * fMyHistoSumExogamDC2D;
   
   TH1F * fMyHistoMultiCrystal;
   TH1F * fMyHistoMultiCrystalperClover;
   TH1F * fMyHistoMultiClover;
   TH1F * fMyHistoMultiAntiCompt;

   TH1F * fMyHistoAngletheta;
   TH1F * fMyHistoAnglephi;
   
   TH1F * fMyHistoPatternECCE;
   TH1F * fMyHistoPatternECCT;
   TH1F * fMyHistoPatternGOCCEE;
   TH1F * fMyHistoPatternGOCCET;
   TH1F * fMyHistoPatternESSTQ;
   
   TH1F * fMyHistoCheckEss;
  
   TH2I * fMyHistoNCriECal;
   TH2I * fMyHistoNCriTCal;



  TH1F *TailleEvt; 
  TH2F *TailleEvt2D; 
  TH2F *TailleEvtNRJ; 
  TH2F *TailleEvtNRJEss; 
  TH2F *TailleEvtNRJGOCCE; 
  TH2F *TreatD2VBSpec;
  TH2F *TreatD2VBSpec2;
  TH2F *TreatD2VBSpec3;

   TH1F * fMyHistoAnglephiD2VB_1;
   TH1F * fMyHistoAnglephiD2VB_2;
   TH1F * fMyHistoAnglephiD2VB_3;




  
   Float_t ECoef[16*4][3]; //ECCE
   Float_t TCoef[16*4][3]; //ECCT
  
   Float_t TESSCoef[16*4][3]; //TESS core
   Float_t TcESSCoef[16][3];  //TESS clover

   Float_t ECoef_G[16*16][3]; //GOCCEE
   Float_t TCoef_G[16*16][3]; //GOCCET
  
   bool NoCompton[16],NoComptonCore[16][4],CloverPresent[16];
   bool GOCCE_Pat[16][4][4], GOCCE_Ener[16][4][4];
   bool CalibDone,Goccetrack,GOCCEActive,ESSActive;
   int NoComptonCounter[16],NoComptonCoreCounter[16][4] ;
   int mulG[16][4], mulGnrj[16][4];
   int mulECC[16][4],mulECCG[16][4], mulECCGnrj[16][4];
   float GammaAngleSegTheta[16][4][4],GammaAngleCoreTheta[16][4];
   float GammaAngleSegPhi[16][4][4],GammaAngleCorePhi[16][4],Beta;
   float TARGET_POSITION_X  ; 
   float TARGET_POSITION_Y  ;
   float TARGET_POSITION_Z  ; 
   int MaxCris[16],MaxSeg[16][4];
   float MaxCrisEval[16],MaxSegEval[16][4];
   int Vclo,Vcore, promptH,promptL;
   struct Clover_struc {
	
	float Theta_Clover;  	// clover center scattering angle by respect to beam axe
	float Phi_Clover;	// clover center phi angle of the PROJECTION on the (0,x,y) plan of the gamma vector 
	float X_Clover,Y_Clover,Z_Clover;
	float Theta_Crystal[4]; //crystalA=0 crystaB=1 etc ....
	float Phi_Crystal[4];
	float X_Crystal[4],Y_Crystal[4],Z_Crystal[4];
	float Theta_Crystal_Seg[4][4]; //CrystalA Segment1 =0,0 CrystalA Segment2=0,1 etc ...
	float Phi_Crystal_Seg[4][4];
	float X_Crystal_Seg[4][4],Y_Crystal_Seg[4][4],Z_Crystal_Seg[4][4];
   };

   
   TObjArray HListExogam;
   
   // virtual methods from TDetector
   virtual bool Init(DataParameters*);
   virtual bool Clear();
   virtual bool Is(UShort_t, Short_t);
   //virtual bool IsMFMExo(MFMExogamFrame*); 
   virtual bool Treat();
   virtual bool InitCal();
   virtual bool ReadCal();
   virtual bool Counter();
   virtual bool CounterReset();
   virtual bool ActivateClover(int);
   virtual bool IsCloverActive(int);
   virtual bool ActivateGOCCE(bool);
   virtual bool ActivateESS(bool);
   virtual bool IsActivateGOCCE();
   virtual bool IsActivateESS();
   virtual bool ACReject();
   virtual bool ActivateGOCCETrack(bool);
   virtual bool SetCloverPosition(int,int,float);
   virtual bool SetBeta(float);
   virtual bool SetVamosCorrelId(int, int);
   virtual bool SetPromptGate(int, int);
   virtual float Doppler_Correction(float , float , float , float , float , float );
   virtual double Cal(UShort_t, float , float , float);
   virtual bool  SpectraConstructor();
   virtual bool  CheckCoreResolution(float);
   virtual bool  D2VBTreat(int);

   virtual void InitBranch(TTree*);

   // getters and setters
   TExogamData*	GetExogamData() const {return fExogamData;}

  private:
   // Data class for Exogam
   TExogamData  *fExogamData;

   ClassDef(TExogam,1)  // Exogam detector structure
};

#endif
