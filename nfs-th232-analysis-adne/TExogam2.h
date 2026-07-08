#ifndef __Exogam2__
#define __Exogam2__

#include <string>
#include <map>

#ifndef __DETECTOR__
#include "TDetector.h"
#endif

#ifndef __Exogam2DATA__
#include "TExogam2Data.h"
#endif

#include <MFMExogamFrame.h>
#include "TVector3.h"
#include "TVector2.h"
#include "TF1.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TMath.h"
#include "TGraph.h"
#include "TProfile.h"
#include <TObject.h>
#include "TRandom.h"
#include "TString.h"
//#include "GTTree.h"
#include "TObjString.h"

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

class TExogam2 : public TDetector {
  public:
   TExogam2(bool bspec);
   virtual ~TExogam2();
   
   bool  BoolSpec, LUTBool;
   //Merge check
   unsigned long long PrevTS;
   short fECCcopy_val;
   TH2F * fMyHistoECCcopy_val;
   // raw spectra
   

   TH2I * fMyHistoNCriERaw;
   TH2I * fMyHistoNCriTRaw;
   TH2I * fMyHistoNSegERaw;
   TH2I * fMyHistoSegSeg;
   TH1F * TimeStampDiff;
   // calibrated spectra
   TH1F * fMyHistoECCECal[16*100+4*10];
   // ECC vs Timestamp,
   TH1F * fMyHistoECCTCal[16*100+4*10];
   TH1F * fMyHistoECCTRaw[16*100+4*10];
   TH2F * ECCTcorrelCheck;
   TH1F * fMyHistoECCEVetoed[16*100+4*10];
   TH1F * fMyHistoECCEAccepted[16*100+4*10];
   TH1F * fMyHistoGOCCEE[16*100+4*10+4];
   TH1F * fMyHistoGOCCET[16*100+4*10+4];
   
   TH1F * fMyHistoECCET30[16*100+4*10];
   TH1F * fMyHistoECCET60[16*100+4*10];
   TH1F * fMyHistoECCET90[16*100+4*10];

   TH2F * fMyHistoECCET30_60[16*100+4*10];
   TH2F * fMyHistoECCET30_90[16*100+4*10];
   TH2F * fMyHistoECCET60_90[16*100+4*10];
   //TH1F * fMyHistoTT[16*4][16*4];
   TH1F * fMyHistoTTAll;
   
   TH1F *BoardIdHist ;
//--------------------------
   TH1F * fMyHistoGOCCENet ;
   TH1F * fMyHistoGOCCEMirror ;
   TH1F * fMyHistoPhiMirror ;
   TH2F * fMyHistoPhiMirrorPattern ;
   TH2F * fMyHistoRPattern ;
   TH1F * fMyHistoRMirror ;
   TH2F * fMyHistoPSASurface ;
   TH2F * fMyHistoPSASurfaceCarte ;
   TGraph *ShortTrace;
   TF1 *trace, *base;
   TGraph *traceExp;
   TH1F * fMyHistoPSACore ;
   TH1F * fMyHistoPSA_CFD ;
   TH2F * fMyHistoPSA_CFD_E ;
   TH2F * fMyHistoPSA_CFD_Pattern ;
   TH1F * fMyHistoPSAChi2 ;
   TH2F * fMyHistoPSAChi2_Pattern;
   TH1F * fMyHistoPSAChi2GRID ;
   TH2F * fMyHistoPSAChi2GRID_Pattern ;
   TH2F * fMyHistoPSAChi2_Radius ;
   TH2F * fMyHistoPSARadius_Radius ;
   TH2F * fMyHistoPSAWorld ;
   TH1F * fMyHistoPSARejectedTraces ;
   TH1F * fMyHistoPSAAcceptedTraces ;
   
   TH1F * fMyHistoPSAK ;
   TH2F * fMyHistoPSAK_Pattern ;
   TH2F * fMyHistoPSAL_Pattern ;
   TH2F * fMyHistoPSAKL;
   TH2F * fMyHistoPSAKL_postGrid;
   TH2F * fMyHistoPSAKEgamma;
 //--------------------------  
 
 
 
   TH1F * fMyHistoESS_BGO[16*100+4*10];
   TH1F * fMyHistoESS_CSI[16*100+4*10];
   TH2F * fMyHistoECCEESSE[16*100+4*10];	
   TH1F * fMyHistoCloverECal[16];
   TH1F * fMyHistoCloverTCal[16];
   TH1F * fMyHistoCloverECalACAdd_DC[16];
   TH1F * fMyHistoCloverECalACAdd_TC[16];
   TH1F * fMyHistoCloverECalAdd[16];
   TH1F * fMyHistoCloverECalACAddDC[16];
   TH1F * fMyHistoCloverECalACAddRejectF[16];
   TH1F * fMyHistoCrysECalACRejectF[16*100+4*10];
   TH2F * fMyHistoGammaGamma;
   TH1F * fMyHistoTTinGammaGamma;
   TH2F * fMyHistoGammaGammaCore;
   TH2F * fMyHistoGammaGammaCoreDiag;
   TH1F * fMyHistoSumECCE;
   TH2F * fMyHistoSumECCE_ECCT;
   TH1F * fMyHistoSumECCT;
   TH1F * fMyHistoSumGOCCEE;
   TH1F * fMyHistoSumGOCCET;
   TH1F * fMyHistoSumExogam2;
   TH1F * fMyHistoSumExogam2DC;
   TH2F * fMyHistoSumExogam2DC_VTS;
   TH1F * fMyHistoSumExogam2DCNucId[10];
   TH1F * fMyHistoSumExogam2FoldCond;
   TH1F * fMyHistoSumExogam2DCFoldCond;
   TH1F * fMyHistoSumExogam2Calorimeter;
   TH2F * fMyHistoSumExogam22D;
   TH2F * fMyHistoSumExogam2DC2D;
   TH2F * fMyHistoSumExogam2BeforeAfterDC ;
   
   TH2F * fMyHistoSumExogam2DCClover;
   TH2F * fMyHistoSumExogam2Clover;
   TH2F * TransverseMomentumGamma;
   
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
   TH2F * fMyHistoPatternESSTQ;
   TH1F * fMyHistoCheckEss;
   TH2I * fMyHistoNCriECal;
   TH2I * fMyHistoNCriTCal;


//NFS extracted analysis spectra
   TH2F * fNfsCrystalDeltaTEnergy;
   TH1F * fNfsAllCrystalTime;
   TH1F * fNfsCrystalDeltaT[16*4];
   TH1F * fNfsCrystalEnergy[16*4];
   // EN/CN: Event-level crystal diagnostics: cross-talk matrix and BGO/CSI fire efficiencies.
   TH2F * fNfsCrystalCrossTalk;
   TProfile * fNfsCrystalBgoEfficiency;
   TProfile * fNfsCrystalCsiEfficiency;
   // EN: No-cut raw crystal gamma-gamma matrix. CN: 不加 cut 的 raw crystal gamma-gamma 符合矩阵。
   TH2F * fNfsAllGammaGammaMatrixNoCut;
   TH1F * fNfsCloverAddbackGamma[16];
   TH1F * fNfsCloverAddbackGammaVeto[16];

//NFS experiment spec
   TH1F * fMyHistoNFS_ToF;
   TH1F * fMyHistoNFS_En;
   TH2F * fMyHistoNFS_EnEstar;
   TH2F * fMyHistoNFS_EnGmul;

   FILE *MGTTraces;
   int MGTCounter;
   bool closed;
   Float_t PSAphiOff[16*16]; //GOCCEE-PSA-Phi
   
   Int_t Current_Numexo2_cfg_clo[255][2];
   Int_t Current_Numexo2_cfg_cri[255][2];

  
   Float_t ECoef[16*4][3]; //ECCE
   Float_t TCoef[16*4][3]; //ECCT
  
   Float_t TESSCoef[16*4][3]; //TESS core
   Float_t TcESSCoef[16][3];  //TESS clover

   Float_t ECoef_G[16*16][3]; //GOCCEE
   Float_t TCoef_G[16*16][3]; //GOCCET
  
  
   bool NoCompton[16],NoComptonCore[16][4],CloverPresent[16];
   bool GOCCE_Pat[16][4][4], GOCCE_Ener[16][4][4];
   bool CalibDone,Goccetrack,GOCCEActive,ESSActive;
   bool PSAActive, neutronNFS, NfsSpec, NfsCrystalTimeCorrection;
   bool NfsCrystalTimeCorrectionValid[16*4];
   float NfsCrystalTimeCorrectionOffset[16*4];
   float NfsCrystalTimeCorrectionGain[16*4];
   float NfsCrystalTimeCorrectionGain2[16*4];
   float NfsCrystalGammaFlashPeak[16*4];
   float NfsCrystalGammaFlashFwhm[16*4];
   float CloverDistanceMm[16];
   float PSABASE[23][3];
   float PSABASER[23];
   float PSABASET0[23];
   float MINIBALL[2],LIVERPOOL[2];
   int DTSAlign;
   float A[500], B[500],C[500];
   int NoComptonCounter[16],NoComptonCoreCounter[16][4], NoNetCharge, noyau ;
   int mulG[16][4], mulGnrj[16][4];
   int mulECC[16][4],mulECCG[16][4], mulECCGnrj[16][4];
   float GammaAngleSegTheta[16][4][4],GammaAngleCoreTheta[16][4];
   float GammaPosCoreX[16][4],GammaPosCoreY[16][4],GammaPosCoreZ[16][4];
   float GammaAngleSegX[16][4][4],GammaAngleSegY[16][4][4],GammaAngleSegZ[16][4][4];
   float GammaAngleSegPhi[16][4][4],GammaAngleCorePhi[16][4],Beta;
   float TARGET_POSITION_X  ; 
   float TARGET_POSITION_Y  ;
   float TARGET_POSITION_Z  ; 
   int MaxCris[16],MaxSeg[16][4];
   float MaxCrisEval[16],MaxSegEval[16][4];
   int Vclo,Vcore, promptH,promptL,MaxClovMul,fullCoinc;
   int BrokenSeg[16][4];
   float ESSThreshold;
   float FreeParam1, FreeParam2, distanceTOF,GammaGateNFS,neutronNRJ,GammaFlashOffset;
   bool IsPromptPublic[16][4];
   bool BrokenSegBool[16][4];
   bool PromptAncillary[16][4], AncillaryCheck;
   float Theta_P, Phi_P;
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

   
   TObjArray HListExogam2;
   TObjArray HListNfsExogam2;
   
   // virtual methods from TDetector
   virtual bool Init(DataParameters*);
   virtual bool InitNumexo2(Char_t*);
   virtual bool Clear();
   virtual bool IsMFMExo(MFMExogamFrame*); 
   virtual bool Is(UShort_t, Short_t);
   virtual bool Treat();
   virtual bool InitCal();
   virtual bool ReadCal();
   virtual bool DefaultCal();
   virtual bool Counter();
   virtual bool CounterReset();
   virtual bool ActivateClover(int);
   virtual bool IsCloverActive(int);
   virtual bool ActivateGOCCE(bool);
   virtual bool ActivateESS(bool, float);
   virtual bool IsActivateGOCCE();
   virtual bool IsActivateESS();
   virtual bool ACReject();
   virtual bool ActivateGOCCETrack(bool);
   virtual bool SetCloverPosition(int,int,float);
   virtual bool SetCloverBrokenSeg(int,int,int);
   virtual bool SetBeta(float);
   virtual bool SetPSA(bool);
   virtual bool SetMaxClovMul(int);
   virtual bool LoadBase(bool);
   virtual bool SetPromptGate(int, int);
   virtual bool IsDiagonal(int, int);   
   virtual bool SetTargetPosition(float,float,float);
   virtual float Doppler_Correction(float , float , float , float , float , float );
   virtual double Cal(UShort_t, float , float , float);
   virtual bool  TimeStampDiffQ(long long, long long  , int);
   virtual bool  SpectraConstructor();
   virtual bool  CheckCoreResolution(float);
   virtual bool  SetNFS_Parameter(float, float, float, bool);
   virtual bool  SetNfsCrystalTimeCorrection(bool, TString);
   virtual bool  SetNFSAnaSpec(bool);
   virtual bool  NfsSpectraConstructor();
   virtual void  FillNfsSpectra(Int_t, Int_t, Int_t, Float_t, Float_t);
   virtual void  FillNfsCrystalEventSpectra(Bool_t*, Bool_t*, Bool_t*);
   virtual void  FillNfsCloverAddbackSpectra(Int_t, Float_t, Float_t, Float_t);

   virtual void InitBranch(TTree*);

   // getters and setters
   TExogam2Data*	GetExogam2Data() const {return fExogam2Data;}

  private:
   // Data class for Exogam2
   TExogam2Data  *fExogam2Data;

   ClassDef(TExogam2,1)  // Exogam2 detector structure
};

#endif
