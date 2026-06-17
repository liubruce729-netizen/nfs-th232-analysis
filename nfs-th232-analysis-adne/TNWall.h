#ifndef __NWall__
#define __NWall__

#include <string>
#include <map>

#ifndef __DETECTOR__
#include "TDetector.h"
#endif

#ifndef __RINGDATA__
#include "TNWallData.h"
#endif
#include <MFMNedaFrame.h>
#include <MFMNedaCompFrame.h>
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TCutG.h"
#include "TFile.h"
#include <TObject.h>
#include "TRandom.h"
#include "TObjString.h"

#define	NWall_E		1
#define	NWall_T		2
#define	NWall_ZCO	3
#define	TAC_BaF2_HF	4
#define	TAC_FT_HF	5
#define	TAC_FT_CFD	6
#define	TAC_CFDft_HF	7
#define	ADCEMPTY	8


#define MAX_NEUT  96


class TNWall : public TDetector {
  public:
   TNWall(bool bspec);
   virtual ~TNWall();
   bool  BoolSpec,LUTBool;
   TH1F * fMyHistoNWallE[96];
   TH1F * fMyHistoNWallTOF[96];
   TH1F * fMyHistoNWallZCO[96];
   TH1F * TimeStampDiff;
   TH1F * TimeStampDiffInterEvent;
   TH1F * BoardDiff;
   TH1F * ChannelDiff;
   TH2F * ChannelDiff2D;
   TH2F * CrossTalk2D;
   TH1F *fMyHistoNWallSlowIntegral[96];
   TH1F *fMyHistoNWallFastIntegral[96];
   TH1F *fMyHistoNWallBitfield[96];
   TH1F *fMyHistoNWallAbsMax[96];
   
   unsigned long long PrevTS, PrevTStagged,PrevTSlocal,duplicatedEventC;
   int prevBoard, prevChannel,tag;
   double prevQ,prevQ2;
   unsigned int prevEvtNumber;
   TH1F * fMyHistoNWallTRF[96];

   TH2F * fMyHistoNWallZCO_E[96];
   TH2F * fMyHistoNWallZCO_TOF[96];
   TH2F * fMyHistoNWallZCO_TRF[96];

#define TAC_CFD_HF_INDIVIDUAL_DETS
#ifdef TAC_CFD_HF_INDIVIDUAL_DETS
   TH1F * fMyHistoTACtmp[96];
#endif

   // #define   TAC_CFD_HF_VS_TRF
#ifdef TAC_CFD_HF_VS_TRF
   TH2F * fMyHistoTAC_CFD_HF_VS_TRF[96];
#endif


   TH1F * fMyHistoNWallNbetween_dt[9];



   
   TH2F * fMyHistoNWallPattE2d;
   TH2F * fMyHistoNWallPattT2d;
   TH2F * fMyHistoNWallPattZCO2d;


   TH1F * fMyHistoNWallPattE;
   TH1F * fMyHistoNWallPattT;
   TH1F * fMyHistoNWallPattZCO;


   TH1F * fMyHistoTAC_BaF2_HF;
   TH1F * fMyHistoTAC_FT_HF;
   TH1F * fMyHistoTAC_FT_CFD;
   TH1F * fMyHistoTAC_CFDft_HF;
   
   
   TH1F * fMyHistoTAC_BaF2_HF_cal;
   TH1F * fMyHistoTAC_FT_HF_cal;
   TH1F * fMyHistoTAC_FT_CFD_cal;
   TH1F * fMyHistoTAC_CFDft_HF_cal;
   TH1F * fMyHistoTAC_BaF2_CFD_cal; 
   TH1F * fMyHistoTAC_BaF2_FT_cal;
   
   TH1F * fMyHistoNWallEmult;
   TH1F * fMyHistoNWallTmult;
   TH1F * fMyHistoNWallZCOmult;
   
   
   TH1F * fMyHistoNWallDataCheckQT;
   TH1F * fMyHistoNWallDataCheckQZCO;
   TH1F * fMyHistoNWallDataCheckTZCO;
   
   Int_t Current_Numexo2_cfg_Board[255];

   Int_t GoodCoincQT, GoodCoincQZCO, GoodCoincTZCO;
   Int_t BadCoincQT,   BadCoincQZCO, BadCoincTZCO;
   //   Double_t  calib_baf2_hf, calib_ft_hf, calib_ft_cfd,calib_cfdFT_hf;
   TObjArray HListNWall;
   
                      // k: tof-qvc, l: zco-qvc, m: zco-tof
   Int_t n_neutrons_zco;


   // virtual methods from TDetector
   virtual bool Init(DataParameters*);
   virtual bool InitNumexo2(Char_t*);
   virtual bool Clear();
   virtual bool Is(UShort_t, Short_t);
   virtual bool IsMFMNeda(MFMNedaFrame*); 
   virtual bool IsMFMNedaComp(MFMNedaCompFrame*);
   virtual bool Treat(); 
   virtual bool InitCal();
   virtual bool ReadCal();
   virtual bool ReadCut();
   virtual bool Init_n2();
   virtual bool Counter();
   virtual bool ClearCounter();
   virtual double Cal(UShort_t, float , float , float);
   virtual bool SpectraConstructor();
   virtual void InitBranch(TTree*);

   Bool_t IsSeparated(Int_t id1, Float_t z1, Int_t id2, Float_t z2, Float_t dt);
 
   // getters and setters
   TNWallData*	GetNWallData() const {return fNWallData;}
   //  NWallN2*	GetNWallN2() const {return fNWallN2;}
   int getNumberOfNeutrons();

  private:
   // Data class for NWall
   TNWallData  *fNWallData;
   // NWallN2    *fNWallN2;

   Int_t GetNbetween(int id1, int id2);

   float ns_dt_limit_first[9];
   float ns_dt_limit_low[9];
   float ns_dt_limit_high[9];
   float ns_dt_limit_last[9];
   int n_nw_dets_in_between[MAX_NEUT][MAX_NEUT];
   TCutG *NWall_zco_tof_cut[96];


   Float_t TOFCoef[96][3];
   Float_t QCoef[96][3];
   Float_t ZCOCoef[96][3];

   Float_t TACCoef_FTCFD[3];
   Float_t TACCoef_BAFHF[3];
   Float_t TACCoef_FTHF[3];
   Float_t TACCoef_CFDHF[3];
   bool CalibDone;

  Int_t NNeutrons_zco();
  Int_t NNeutrons_n2();
  Int_t NNeutrons();

   ClassDef(TNWall,1)  // NWall detector structure
};

#endif
