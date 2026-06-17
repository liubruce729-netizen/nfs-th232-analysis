#include <cstdlib>

#include "TExogam2REA.h"

#include "fstream"

ClassImp(TExogam2REA)
//if uncorrelated because of TDC then return false to save CPU time
TExogam2REA::TExogam2REA(bool bspec)
{
   // Default constructor
   fExogam2READata    = new TExogam2READata();
   BoolSpec=bspec;
   Beta=0.;
   fullCoinc=0;
   for(Int_t i=0;i<16;i++)CloverPresent[i]=false;
   Goccetrack=GOCCEActive=ESSActive=false;
   LUTBool=false;
   PrevTS=0;
   MaxClovMul=18;
   ESSThreshold=1000.;
  
   
   closed=false;
   for(Int_t i=0;i<16;i++){
   	for(Int_t j=0;j<4;j++){
		BrokenSeg[i][j]=1000;
  		BrokenSegBool[i][j]=false;
   	}
   }
	
}



TExogam2REA::~TExogam2REA()
{
   delete fExogam2READata;
}



bool TExogam2REA::Clear()
{
   fExogam2READata->Clear();
   
   for(int k=0 ; k<16 ; k++){
	NoCompton[k]=true;
	MaxCris[k]=1000;
	MaxCrisEval[k]=-1000.;
	for(int l=0;l<4;l++){
		NoComptonCore[k][l]=true;
		mulG[k][l]=0;
                mulGnrj[k][l]=0;
		MaxSeg[k][l]=1000;
		MaxSegEval[k][l]=-1000.;
       		for(int m =0; m<4;m++){
		  GOCCE_Pat[k][l][m]=GOCCE_Ener[k][l][m]=false;
		}
	}
   }
   

   AncillaryCheck=false;
   noyau=-1;
   FreeParam1=FreeParam2=-1.;
   return true;
}
bool TExogam2REA::ActivateGOCCE(bool act){
	GOCCEActive=act;
	return true;
}
bool TExogam2REA::ActivateESS(bool act, float Thres){
	ESSActive=act;
	ESSThreshold=Thres;
	return true;
}
bool TExogam2REA::IsActivateGOCCE(){
	return GOCCEActive;
}
bool TExogam2REA::IsActivateESS(){
	return ESSActive;
}
bool TExogam2REA::ActivateClover(int i)
{ 
	CloverPresent[i]=true;
	return true;
}
bool TExogam2REA::SetPromptGate(int a, int b){
	promptL=a;
	promptH=b;
		cerr<<"TExogam2 REA Info :: Prompt gate is "<<promptL<<"   "<<promptH<<endl;

	return true;
}
bool TExogam2REA::IsCloverActive(int i)
{ 
	return CloverPresent[i];
}
bool TExogam2REA::ActivateGOCCETrack(bool check){

	Goccetrack=check;
	return true;
}
bool TExogam2REA::SetBeta(float beta){
	Beta=beta;
	return true;
}
bool TExogam2REA::SetCloverBrokenSeg(int clo, int cri, int seg){
  	BrokenSeg[clo][cri]=seg;
  	BrokenSegBool[clo][cri]=true;
  	return true;
}
bool TExogam2REA::SetMaxClovMul(int set){
	MaxClovMul=set;
	printf("\033[35mExogam REA Info ::  Set a max Clover Mult at %d \033[m \n",MaxClovMul);
	return true;
}

bool TExogam2REA::IsDiagonal(int cri1, int cri2){
   	bool result;
	if(abs(cri1-cri2)==2){result = true;}
	else {result = false;}
	return result;

}

float TExogam2REA::Doppler_Correction(float Theta_Gamma, float Phi_Gamma, float Theta_Part, float Phi_Part, float Beta_Part, float energie_Mes){  //rad, v/c
	float energievraie,cosinusPSI;
			  
		  cosinusPSI =TMath::Sin(Theta_Part)*TMath::Cos(Phi_Part)*TMath::Sin(Theta_Gamma)*TMath::Cos(Phi_Gamma)+
		  	      TMath::Sin(Theta_Part)*TMath::Sin(Phi_Part)*TMath::Sin(Theta_Gamma)*TMath::Sin(Phi_Gamma)+
			      TMath::Cos(Theta_Part)*TMath::Cos(Theta_Gamma);
			      
	energievraie = energie_Mes*(1.-Beta_Part*cosinusPSI)/sqrt(1.-Beta_Part*Beta_Part);

	return energievraie;
};

bool TExogam2REA::SpectraConstructor()
{
if(BoolSpec){

	char title[50];
	char cristal[5]={'A','B','C','D','\0'};  
	
	
	printf("\033[35mExogam REA Info :  Start Spectra constructors\033[m \n");
	fMyHistoNCriERaw = new TH2I("NCristal_ERaw_REA","NCristal_ERaw_REA",16*10,0,16*10,8000,0,16000);
	//HLisTExogam2REA.Add(fMyHistoNCriERaw);

	fMyHistoNCriTRaw = new TH2I("NCristal_TRaw_REA","NCristal_TRaw_REA",16*4,0,16*4,8000,0,1e5);
	//HLisTExogam2REA.Add(fMyHistoNCriTRaw);
	
	
	fMyHistoNCriECal = new TH2I("NCristal_Ecal_REA","NCristal_Ecal_REA",16*4,0,16*4,1000,0,3000);
	//HLisTExogam2REA.Add(fMyHistoNCriECal);
	
	fMyHistoNCriTCal = new TH2I("NCristal_Tcal_REA","NCristal_Tcal_REA",16*4,0,16*4,4000,0,1e5);
	//HLisTExogam2REA.Add(fMyHistoNCriTCal);

        TimeStampDiff= new TH1F("TimeStampDiffTExogam2REA_REA","TimeStampDiffTExogam2REA_REA",11000,-1000,10000);
	//HLisTExogam2REA.Add(TimeStampDiff);
	
	
	BoardIdHist = new TH1F("BoardIdHist_REA","BoardIdHist_REA",255*2,0,255);
	//HLisTExogam2REA.Add(BoardIdHist);
	
	fMyHistoSegSeg = new TH2I("fMyHistoSegSegExogam_REA","fMyHistoSegSegExogam_REA",300,0,300,300,0,300);
	//HLisTExogam2REA.Add(fMyHistoSegSeg);
	
	
	for(int k2=0 ; k2<16*4 ; k2++){
		for(int k=0 ; k<16*4 ; k++){
			//cout<<k2%4<<" "<<k%4<<endl;
			if(CloverPresent[k2/4]==false || CloverPresent[k/4]==false){continue;}
			else{
				sprintf(title,"fMyHistoTT_REA%d_%d",k2,k);	
				//fMyHistoTT[k2][k]=new TH1F(title,title,40,-200,200);
				//HLisTExogam2REA.Add(fMyHistoTT[k2][k]);
			}
		}
	}
		
	
	
	for(int k=0 ; k<16 ; k++){
		if(CloverPresent[k]==false) continue;
			//cout << "Clover  :: "<<k<<endl;
			sprintf(title,"Clover%d_ECal_REA",k);// No process
			fMyHistoCloverECal[k]= new TH1F(title,title,6000,0,3000); 
			//HLisTExogam2REA.Add(fMyHistoCloverECal[k]);
			
			sprintf(title,"Clover%d_TCal_REA",k);
			fMyHistoCloverTCal[k]= new TH1F(title,title,4000,0,1e5); 
			//HLisTExogam2REA.Add(fMyHistoCloverTCal[k]);
			
			sprintf(title,"Clover%d_ECalAdd_REA",k); //Prompt AddBack
			fMyHistoCloverECalAdd[k]= new TH1F(title,title,6000,0,3000); 
			//HLisTExogam2REA.Add(fMyHistoCloverECalAdd[k]);
			
			sprintf(title,"Clover%d_ECalACAddDC_DC_REA",k);//AddBack + AC  /clover + DopplerCorrected
			fMyHistoCloverECalACAdd_DC[k]= new TH1F(title,title,6000,0,3000); 
			//HLisTExogam2REA.Add(fMyHistoCloverECalACAdd_DC[k]);
			
			sprintf(title,"Clover%d_ECalACAdd_TC_REA",k);//AddBack + AC + prompt /clover
			fMyHistoCloverECalACAdd_TC[k]= new TH1F(title,title,6000,0,3000); 
			//HLisTExogam2REA.Add(fMyHistoCloverECalACAdd_TC[k]);
			
			sprintf(title,"Clover%d_ECalACAddRejectF_REA",k);
			fMyHistoCloverECalACAddRejectF[k]= new TH1F(title,title,600,0,3000); 
			//HLisTExogam2REA.Add(fMyHistoCloverECalACAddRejectF[k]);
		for(int l=1;l<=4;l++){
			//cout << "          Crystal   :: "<<l<<endl;
			sprintf(title,"ECC%d_%c_ECal_REA",k,cristal[l-1]);
			fMyHistoECCECal[k*100+10*(l-1)]= new TH1F(title,title,40000,0,20000); 
			HLisTExogam2REA.Add(fMyHistoECCECal[k*100+10*(l-1)]);
			
			sprintf(title,"ECC%d_%c_TCal_REA",k,cristal[l-1]);
			fMyHistoECCTCal[k*100+10*(l-1)]= new TH1F(title,title,10000,0,10000); //kEV TKE
			//HLisTExogam2REA.Add(fMyHistoECCTCal[k*100+10*(l-1)]);
			
			sprintf(title,"ECC%d_%c_TRaw_REA",k,cristal[l-1]);
			fMyHistoECCTRaw[k*100+10*(l-1)]= new TH1F(title,title,400,0,1e5); 
			//HLisTExogam2REA.Add(fMyHistoECCTRaw[k*100+10*(l-1)]);
			
			
			
			sprintf(title,"ECC%d_%c_T30_REA",k,cristal[l-1]);
			fMyHistoECCET30[k*100+10*(l-1)]=new TH1F(title,title,100,0,100); 
			//HLisTExogam2REA.Add(fMyHistoECCET30[k*100+10*(l-1)]);
			sprintf(title,"ECC%d_%c_T60_REA",k,cristal[l-1]);
			fMyHistoECCET60[k*100+10*(l-1)]=new TH1F(title,title,100,0,100); 
			//HLisTExogam2REA.Add(fMyHistoECCET60[k*100+10*(l-1)]);
			sprintf(title,"ECC%d_%c_T90_REA",k,cristal[l-1]);
			fMyHistoECCET90[k*100+10*(l-1)]=new TH1F(title,title,100,0,100); 
			//HLisTExogam2REA.Add(fMyHistoECCET90[k*100+10*(l-1)]);
			
			sprintf(title,"ECC%d_%c_T30_60_REA",k,cristal[l-1]);
			fMyHistoECCET30_60[k*100+10*(l-1)]=new TH2F(title,title,100,0,100,100,0,100); 
			//HLisTExogam2REA.Add(fMyHistoECCET30_60[k*100+10*(l-1)]);
			sprintf(title,"ECC%d_%c_T30_90_REA",k,cristal[l-1]);
			fMyHistoECCET30_90[k*100+10*(l-1)]=new TH2F(title,title,100,0,100,100,0,100); 
			//HLisTExogam2REA.Add(fMyHistoECCET30_90[k*100+10*(l-1)]);
			sprintf(title,"ECC%d_%c_T60_90_REA",k,cristal[l-1]);
			fMyHistoECCET60_90[k*100+10*(l-1)]=new TH2F(title,title,100,0,100,100,0,100); 
			//HLisTExogam2REA.Add(fMyHistoECCET60_90[k*100+10*(l-1)]);
			
			sprintf(title,"ESS%d_BGO%d_REA",k,l);
			fMyHistoESS_BGO[k*100+10*(l-1)]=new TH1F(title,title,4000,0,16000); 
			//HLisTExogam2REA.Add(fMyHistoESS_BGO[k*100+10*(l-1)]);
			
			sprintf(title,"ESS%d_CSI%d_REA",k,l);
			fMyHistoESS_CSI[k*100+10*(l-1)]=new TH1F(title,title,4000,0,16000); 
			//HLisTExogam2REA.Add(fMyHistoESS_CSI[k*100+10*(l-1)]);
			
			sprintf(title,"ESS%d_BGO%d_T_REA",k,l);
			fMyHistoESS_BGOT[k*100+10*(l-1)]=new TH1F(title,title,4000,0,16000); 
			//HLisTExogam2REA.Add(fMyHistoESS_BGO[k*100+10*(l-1)]);
			
			sprintf(title,"ESS%d_CSI%d_T_REA",k,l);
			fMyHistoESS_CSIT[k*100+10*(l-1)]=new TH1F(title,title,4000,0,16000); 
			//HLisTExogam2REA.Add(fMyHistoESS_CSI[k*100+10*(l-1)]);
			
			sprintf(title,"ESS%d_%c_CoreEAcE_REA",k,cristal[l-1]);
			fMyHistoECCEESSE[k*100+10*(l-1)]=new TH2F(title,title,500,0,2000,500,0,1000);
			//HLisTExogam2REA.Add(fMyHistoECCEESSE[k*100+10*(l-1)]);
			
			sprintf(title,"ECC%d_%c_Vetoed_REA",k,cristal[l-1]);
			fMyHistoECCEVetoed[k*100+10*(l-1)]= new TH1F(title,title,400,0,2000); 
			//HLisTExogam2REA.Add(fMyHistoECCEVetoed[k*100+10*(l-1)]);
			
			sprintf(title,"ECC%d_%c_Accepted_REA",k,cristal[l-1]);
			fMyHistoECCEAccepted[k*100+10*(l-1)]= new TH1F(title,title,400,0,2000); 
			//HLisTExogam2REA.Add(fMyHistoECCEAccepted[k*100+10*(l-1)]);
			
			sprintf(title,"ECC%d_%c_ECalACRejectF_REA",k,cristal[l-1]);
			fMyHistoCrysECalACRejectF[k*100+10*(l-1)]= new TH1F(title,title,400,0,2000); 
			//HLisTExogam2REA.Add(fMyHistoCrysECalACRejectF[k*100+10*(l-1)]);
			
			for(int m=1; m<=4; m++){
				sprintf(title,"GOCCE%d_%c_%d_E_REA",k,cristal[l-1],m);
				fMyHistoGOCCEE[k*100+(l-1)*10+(m-1)]= new TH1F(title,title,20000,0,20000);
				HLisTExogam2REA.Add(fMyHistoGOCCEE[k*100+(l-1)*10+(m-1)]);
				
				sprintf(title,"GOCCE%d_%c_%d_T_REA",k,cristal[l-1],m);
				fMyHistoGOCCET[k*100+(l-1)*10+(m-1)]= new TH1F(title,title,16384,0,5*16384);
				//HLisTExogam2REA.Add(fMyHistoGOCCET[k*100+(l-1)*10+(m-1)]);
			}

		}
	}
		
	fMyHistoSumECCE= new TH1F("fMyHistoSumECCE_REA","fMyHistoSumECCE_REA",6000,0,6000); 
	HLisTExogam2REA.Add(fMyHistoSumECCE);
	fMyHistoSumECCT= new TH1F("fMyHistoSumECCT_REA","fMyHistoSumECCT_REA",640,1,64000); 
	HLisTExogam2REA.Add(fMyHistoSumECCT);
	fMyHistoPatternECCT= new TH1F("PatternTimeCoreExogam2_REA","PatternTimeCoreExogam2_REA",2000,0,200); 
	HLisTExogam2REA.Add(fMyHistoPatternECCT);
	fMyHistoPatternECCE= new TH1F("PatternEnergyCoreExogam2_REA","PatternEnergyCoreExogam2_REA",200,0,200); 
	HLisTExogam2REA.Add(fMyHistoPatternECCE);
	fMyHistoTTAll = new TH1F("fMyHistoTTAll_REA","fMyHistoTTAll_REA",2000,-1000,1000);
	//HLisTExogam2REA.Add(fMyHistoTTAll);
	
	ECCTcorrelCheck = new TH2F("ECCTcorrelCheck_REA","ECCTcorrelCheck_REA",1000,00,100,400,0,1e5);
	
	fMyHistoSumECCE_ECCT = new TH2F("fMyHistoSumECCE_ECCT_REA","fMyHistoSumECCE_ECCT_REA",3000,0,3000,500,0000,10000);
	HLisTExogam2REA.Add(fMyHistoSumECCE_ECCT);
	fMyHistoSumGOCCEE= new TH1F("fMyHistoSumGOCCEE_REA","fMyHistoSumGOCCEE_REA",70000,0,70000); 
	HLisTExogam2REA.Add(fMyHistoSumGOCCEE);
	fMyHistoSumGOCCET= new TH1F("fMyHistoSumGOCCET_REA","fMyHistoSumGOCCET_REA",4000,0,20000); 
	HLisTExogam2REA.Add(fMyHistoSumGOCCET);
	fMyHistoPatternGOCCEE= new TH1F("PatternEnergySegExogam2_REA","PatternEnergySegExogam2_REA",1700,0,1700); 
	HLisTExogam2REA.Add(fMyHistoPatternGOCCEE);
	fMyHistoPatternGOCCET= new TH1F("PatternTimeSegExogam2_REA","PatternTimeSegExogam2_REA",1700,0,1700); 
	HLisTExogam2REA.Add(fMyHistoPatternGOCCET);
	fMyHistoNSegERaw=new TH2I("PatternEnergySegExogam22D_REA","PatternEnergySegExogam22D_REA",1700,0,1700,3000,0,10000); 
	HLisTExogam2REA.Add(fMyHistoNSegERaw);
	fMyHistoSumSeg_Core= new TH2F("fMyHistoSumSeg_Core","fMyHistoSumSeg_Core",3000,0,3000,3000,0,3000);
	HLisTExogam2REA.Add(fMyHistoSumSeg_Core);
	
	fMyHistoPatternESSTQ = new TH2F("PatternEssQ_REA","PatternEssQ_REA",170,0,170,4000,0,16000); 
	HLisTExogam2REA.Add(fMyHistoPatternESSTQ);
	
		
	
	fMyHistoAngletheta= new TH1F("Theta_Distribution_REA","Theta_Distribution_REA",200,0,200);
	HLisTExogam2REA.Add(fMyHistoAngletheta);
        fMyHistoAnglephi= new TH1F("Phi_Distribution_REA","Phi_Distribution_REA",400,-200,200);
	HLisTExogam2REA.Add(fMyHistoAnglephi);
	fMyHistoMultiCrystal= new TH1F("TotalCrystalMultiplicity_REA","TotalCrystalMultiplicity_REA",16*4,0,16*4);
	HLisTExogam2REA.Add(fMyHistoMultiCrystal);
	fMyHistoMultiCrystalperClover= new TH1F("CrystalMultiplicityPerClover_REA","CrystalMultiplicityPerClover_REA",6,0,6);
	HLisTExogam2REA.Add(fMyHistoMultiCrystalperClover);
	fMyHistoMultiClover= new TH1F("TotalCloverMultiplicity_REA","TotalCloverMultiplicity_REA",20,0,20);
	HLisTExogam2REA.Add(fMyHistoMultiClover);
	fMyHistoMultiAntiCompt= new TH1F("TotalAntiComptonMultiplicity_REA","TotalAntiComptonMultiplicity_REA",20,0,20);
	HLisTExogam2REA.Add(fMyHistoMultiAntiCompt);
	
	for(int l=0;l<10;l++){
		sprintf(title,"Exogam2AddB_AC_DC_Nuc%d_REA",l);
		fMyHistoSumExogam2DCNucId[l]= new TH1F(title,title,1000,0,6000);
		HLisTExogam2REA.Add(fMyHistoSumExogam2DCNucId[l]);
	}
	
	fMyHistoSumExogam2= new TH1F("Exogam2AddB_AC_noDC_REA","Exogam2AddB_AC_noDC_REA",10000,0,10000);//AddBack + AC rejected + Prompt
	HLisTExogam2REA.Add(fMyHistoSumExogam2);
	fMyHistoSumExogam2DC= new TH1F("Exogam2AddB_AC_DC_REA","Exogam2AddB_AC_DC_REA",10000,0,10000);
	HLisTExogam2REA.Add(fMyHistoSumExogam2DC);
	fMyHistoSumExogam22D= new TH2F("Exogam2AddB_AC_noDC2D_REA","Exogam2AddB_AC_noDC2D_REA",10,0,200,500,0,2000);
	HLisTExogam2REA.Add(fMyHistoSumExogam22D);
        fMyHistoSumExogam2DC2D= new TH2F("Exogam2AddB_AC_DC2D_REA","Exogam2AddB_AC_DC2D_REA",10,0,200,500,0,2000);
	HLisTExogam2REA.Add(fMyHistoSumExogam2DC2D);
	
	
	
	fMyHistoSumExogam2FoldCond= new TH1F("Exogam2AddB_AC_noDC_Fold1_REA","Exogam2AddB_AC_noDC_Fold1_REA",4000,0,4000);//AddBack + AC rejected + Prompt Fold1
	HLisTExogam2REA.Add(fMyHistoSumExogam2FoldCond);
	fMyHistoSumExogam2DCFoldCond= new TH1F("Exogam2AddB_AC_DC_Fold1_REA","Exogam2AddB_AC_DC_Fold1_REA",4000,0,4000);//AddBack + AC rejected + Prompt Fold 1 DC
	HLisTExogam2REA.Add(fMyHistoSumExogam2DCFoldCond);
	
	TransverseMomentumGamma = new TH2F("TransverseMomentumGamma_REA","TransverseMomentumGamma_REA",100,0,100,500,0,6000);
	HLisTExogam2REA.Add(TransverseMomentumGamma);
	
	
	fMyHistoGammaGamma= new TH2F("GammaGamma_AC_AD_DC_REA","GammaGamma_AC_AD_DC_REA",2000,0,2000,2000,0,2000);
	HLisTExogam2REA.Add(fMyHistoGammaGamma);
	fMyHistoGammaGammaCore= new TH2F("fMyHistoGammaGammaCore_REA","fMyHistoGammaGammaCore_REA",2000,0,2000,2000,0,2000);
	HLisTExogam2REA.Add(fMyHistoGammaGammaCore);
	fMyHistoGammaGammaCoreDiag= new TH2F("fMyHistoGammaGammaCoreDiag_REA","fMyHistoGammaGammaCoreDiag_REA",2000,0,2000,2000,0,2000);
	HLisTExogam2REA.Add(fMyHistoGammaGammaCoreDiag);
	
	
	
	fMyHistoSumExogam2Calorimeter=new TH1F("Exogam2CalorimeterAC_REA","Exogam2CalorimeterAC_REA",4000,0,20000);
	HLisTExogam2REA.Add(fMyHistoSumExogam2Calorimeter);
	fMyHistoCheckEss=new TH1F("CheckEss_REA","CheckEss_REA",100,0,100);


   	fMyHistoSumExogam2DCClover= new TH2F("fMyHistoSumExogam2DCClover_REA","fMyHistoSumExogam2DCClover_REA",16,0,16,500,0,6000);
       	fMyHistoSumExogam2Clover= new TH2F("fMyHistoSumExogam2Clover_REA","fMyHistoSumExogam2Clover_REA",16,0,16,500,0,6000);
	HLisTExogam2REA.Add(fMyHistoSumExogam2Clover);
	HLisTExogam2REA.Add(fMyHistoSumExogam2DCClover);

//--------------------------------------------PSA----------------------------------------------
/*	fMyHistoGOCCENet= new TH1F("fMyHistoGOCCENet","fMyHistoGOCCENet",70000,0,70000);  
	fMyHistoGOCCEMirror = new TH1F("fMyHistoGOCCEMirror","fMyHistoGOCCEMirror",70000,-35000,35000);
	fMyHistoPhiMirror = new TH1F("fMyHistoPhiMirror","fMyHistoPhiMirror",400,-400,400); 
	fMyHistoRMirror= new TH1F("fMyHistoRMirror","fMyHistoRMirror",1000,0,100); 
	fMyHistoPSASurface= new TH2F("fMyHistoPSASurface","fMyHistoPSASurface",200,0,50,1000,-360,360);
	fMyHistoPSASurfaceCarte= new TH2F("fMyHistoPSASurfaceCarte","fMyHistoPSASurfaceCarte",400,-100,100,400,-100,100);
	ShortTrace= new TGraph(6);
	fMyHistoPSAChi2 = new TH1F("fMyHistoPSAChi2","fMyHistoPSAChi2",7000,0,700);
	fMyHistoPSAChi2_Pattern= new TH2F("fMyHistoPSAChi2_Pattern","fMyHistoPSAChi2_Pattern",16*4,0,16*4,7000,0,700);
	fMyHistoPSAChi2GRID = new TH1F("fMyHistoPSAChi2GRID","fMyHistoPSAChi2GRID",700,0,70);
	fMyHistoPSAChi2GRID_Pattern = new TH2F("fMyHistoPSAChi2GRID_Pattern","fMyHistoPSAChi2GRID_Pattern",16*4,0,16*4,700,0,70);
	fMyHistoPSAChi2_Radius= new TH2F("fMyHistoPSAChi2_Radius","fMyHistoPSAChi2_Radius",100,0,100,50,0,50);
	fMyHistoPSACore= new TH1F("fMyHistoPSACore","fMyHistoPSACore",7000,0,7000);
	fMyHistoPSA_CFD= new TH1F("fMyHistoPSA_CFD","fMyHistoPSA_CFD",4000,0,2000);
	fMyHistoPSA_CFD_E= new TH2F("fMyHistoPSA_CFD_E","fMyHistoPSA_CFD_E",300,-100,200,1000,0,2000);
	fMyHistoPSA_CFD_Pattern= new TH2F("fMyHistoPSA_CFD_Pattern","fMyHistoPSA_CFD_Pattern",16*4,0,16*4,4000,0,200);
	fMyHistoPhiMirrorPattern=new TH2F("fMyHistoPhiMirrorPattern","fMyHistoPhiMirrorPattern",1700,0,1700,200,-100,100);
	fMyHistoPSARejectedTraces= new TH1F("fMyHistoPSARejectedTraces","fMyHistoPSARejectedTraces",7000,0,7000);
	fMyHistoPSAAcceptedTraces  = new TH1F("fMyHistoPSAAcceptedTraces","fMyHistoPSAAcceptedTraces",7000,0,7000);
	fMyHistoRPattern =new TH2F("fMyHistoRPattern","fMyHistoRPattern",16*4,0,16*4,50,0,50);
	fMyHistoPSARadius_Radius = new TH2F("fMyHistoPSARadius_Radius","fMyHistoPSARadius_Radius",400,0,100,400,-100,100);
	fMyHistoPSAWorld= new TH2F("fMyHistoPSAWorld","fMyHistoPSAWorld",200,-200,200,200,-200,200);
	fMyHistoPSAK = new TH1F("fMyHistoPSAK","fMyHistoPSAK",300,0,300);
        fMyHistoPSAK_Pattern  = new TH2F("fMyHistoPSAK_Pattern","fMyHistoPSAK_Pattern",16*4,0,16*4,300,0,300);;
        fMyHistoPSAL_Pattern= new TH2F("fMyHistoPSAL_Pattern","fMyHistoPSAL_Pattern",16*4,0,16*4,300,-300,300);
	fMyHistoPSAKL = new TH2F("fMyHistoPSAKL","fMyHistoPSAKL",600,0,300,600,-300,0);
	fMyHistoPSAKL_postGrid= new TH2F("fMyHistoPSAKL_postGrid","fMyHistoPSAKL_postGrid",600,0,300,600,-300,0);
	fMyHistoPSAKEgamma = new TH2F("fMyHistoPSAKEgamma","fMyHistoPSAKEgamma",300,-300,300,3000,0,3000);
	
	
	
	HLisTExogam2REA.Add(fMyHistoGOCCENet);
	HLisTExogam2REA.Add(fMyHistoGOCCEMirror);
	HLisTExogam2REA.Add(fMyHistoPhiMirror);
	HLisTExogam2REA.Add(fMyHistoRMirror);
	HLisTExogam2REA.Add(fMyHistoPSASurfaceCarte);
	HLisTExogam2REA.Add(fMyHistoPSAChi2);
	HLisTExogam2REA.Add(fMyHistoPSACore);
	HLisTExogam2REA.Add(fMyHistoPSARejectedTraces);
	HLisTExogam2REA.Add(fMyHistoPSAAcceptedTraces);
	HLisTExogam2REA.Add(fMyHistoPSA_CFD);
	HLisTExogam2REA.Add(fMyHistoPSA_CFD_E);
	HLisTExogam2REA.Add(fMyHistoPSA_CFD_Pattern);
	HLisTExogam2REA.Add(fMyHistoPSARadius_Radius);
	HLisTExogam2REA.Add(fMyHistoPhiMirrorPattern);
	HLisTExogam2REA.Add(fMyHistoRPattern);
	HLisTExogam2REA.Add(fMyHistoPSAWorld);
	HLisTExogam2REA.Add(fMyHistoPSAChi2_Pattern);
	HLisTExogam2REA.Add(fMyHistoPSAK);
	HLisTExogam2REA.Add(fMyHistoPSAK_Pattern);
	HLisTExogam2REA.Add(fMyHistoPSAL_Pattern);
	//--------------------------------------------PSA----------------------------------------------
	

*/










	printf("\033[35m ----> Done \033[m \n");	     
	return true;
}
else {return false;}

}
bool TExogam2REA::InitCal()
{
	for(Int_t x=0;x<16*4;x++){
    		ECoef[x][0]=0.;
		ECoef[x][1]=1.;
        	ECoef[x][2]=0.;
    	}
	for(Int_t x=0;x<16*4;x++){
    		TCoef[x][0]=0.;
		TCoef[x][1]=1.;
        	TCoef[x][2]=0.;
		TESSCoef[x][0]=0.;
		TESSCoef[x][1]=1.;
		TESSCoef[x][2]=0.;
    	}
	for(Int_t x=0;x<16;x++){
		TcESSCoef[x][0]=0.;
		TcESSCoef[x][1]=1.;
		TcESSCoef[x][2]=0.;
	}
 	for(Int_t x=0;x<16*16;x++){
    		ECoef_G[x][0]=0.;
		ECoef_G[x][1]=1.;
        	ECoef_G[x][2]=0.;
		TCoef_G[x][0]=0.;
		TCoef_G[x][1]=1.;
        	TCoef_G[x][2]=0.;
		PSAphiOff[x]=0.;
		
    	}
	
	CalibDone=false;
return true;
}	

bool TExogam2REA::DefaultCal()
{
	for(Int_t x=0;x<16*4;x++){
    		ECoef[x][0]=0.;
		ECoef[x][1]=0.225;
        	ECoef[x][2]=0.;
    	}
	for(Int_t x=0;x<16*16;x++){
    		ECoef_G[x][0]=0.;
		ECoef_G[x][1]=0.634;
        	ECoef_G[x][2]=0.;
    	}
	printf("\033[31mInfo Exogam2  REA:: Using default nrj cal param Core (0.225) Seg (0.634)  \033[m \n");
	CalibDone=false;
return true;
}	


bool TExogam2REA::ReadCal()
{
  FILE *ecc_cal = fopen("CalFile/ecc.cal","r");   // fichier de calibration ecc= inner=core=cristal
  FILE *gocce_cal = fopen("CalFile/gocce.cal","r");// fichier de calibration gocce= outer=segment
  FILE *file2 = fopen("CalFile/gocce_phi_offset.cal","r");// fichier de calibration gocce=EN PHI PSA

// fichier de calibration gocce= outer=segment
	Int_t x;
	float a,b,c;

	
	for(x=0;x<16*4;x++){
	fscanf(ecc_cal,"%f %f %f\n",&a,&b,&c);
    	ECoef[x][0]=a;
	ECoef[x][1]=b;
        ECoef[x][2]=c;
	//cout <<a << " "<<b << " "<<c <<"  "<<x<<endl;
    	}
	for(x=0;x<16*4;x++){
	fscanf(ecc_cal,"%f %f %f \n",&a,&b,&c);
    	TCoef[x][0]=a;
	TCoef[x][1]=b;
        TCoef[x][2]=c;
    	}
 
 	for(x=0;x<16*16;x++){
	fscanf(gocce_cal,"%f %f %f\n",&a,&b,&c);
    	ECoef_G[x][0]=a;
	ECoef_G[x][1]=b;
        ECoef_G[x][2]=c;
    	}
	for(x=0;x<16*16;x++){
	fscanf(gocce_cal,"%f %f %f\n",&a,&b,&c);
    	TCoef_G[x][0]=a;
	TCoef_G[x][1]=b;
        TCoef_G[x][2]=c;
    	}
	for(Int_t x=0;x<16*16;x++){
		fscanf(file2,"%f\n",&a);
        	PSAphiOff[x]=a;
		//printf("Phi Offset PSA --> (%d , %f)\n",x,a);
    	}
	fclose(file2);
	
	CalibDone=true;
return true;	
}

double TExogam2REA::Cal(UShort_t en, float offset, float gain, float gain2){
double enc;
	    enc = (double)en+gRandom->Uniform(1.0)-.5;
	    //enc = (double)enc;
	    enc = enc*enc*gain2+enc*gain+offset;
	    return enc;
}
bool TExogam2REA::InitNumexo2(Char_t *fileNumexo2){
TString schainetrack;
TObjArray* toks=0;
Int_t board,halfboard,clo,cri;

	for(Int_t i= 0 ; i<255;i++){Current_Numexo2_cfg_clo[i][0]=Current_Numexo2_cfg_clo[i][1]=Current_Numexo2_cfg_cri[i][0]=Current_Numexo2_cfg_cri[i][1]=-1;}
	
	ifstream inf_f(fileNumexo2);
	if(inf_f.good()==false) {
		printf("\033[31m Error Exogam2 REA:: Not a valid Look Up Table=> I complain here   \033[m \n"); 
		//gets(stop);
	}
	else{
		LUTBool=true;
		while (inf_f.good()){
			schainetrack.ReadLine(inf_f);
			if(schainetrack.BeginsWith("//")){continue;}
			else if(schainetrack.BeginsWith("END")){break;}
			else {
				toks = schainetrack.Tokenize(" ");
				board = ((TObjString* )toks->At(2))->GetString().Atoi();
				halfboard = ((TObjString* )toks->At(3))->GetString().Atoi();
				clo = ((TObjString* )toks->At(0))->GetString().Atoi();
				cri = ((TObjString* )toks->At(1))->GetString().Atoi();
				Current_Numexo2_cfg_clo[board][halfboard]=clo;
				Current_Numexo2_cfg_cri[board][halfboard]=cri;
				//cout<<"Exogam2 REA:: board "<<board<<" halfboard "<<halfboard<<" is (clo, "<<clo<<", cri "<<cri<<")"<<endl;
				delete toks;
			}
		}
	
	}
	inf_f.close();
	printf("=>Look Up Table Numexo2 EXOGAM REA read \n");
	printf("\n");
	
return true;
}
bool TExogam2REA::Init(DataParameters *params)
{
   Int_t channum, segnum;
   Int_t Xtalnum = -1;
   string crystal, sigtype;
   bool status = false;
   TString schaine;
   TObjArray* toks=0;
   Char_t labels[30];
   
   Int_t nbParams = params->GetNbParameters();
   for (Int_t index = 0; index < nbParams; index++) {
      Int_t lbl      = params->GetLabel(index);
      string label   = params->GetParNameFromIndex(index);
      // Inner contacts
      if (label.compare(0,3,"ECC") == 0) {
         fLabelMap[lbl] = label;
	 status = true;

         // clover number
         channum = atoi(label.substr(3,label.find_first_of("_")).c_str());
	 // crystal name
	 crystal = label.substr(label.find_first_of("_")+1,1);

         // crystal number
         if (crystal == "A")      Xtalnum = 0;
	 else if (crystal == "B") Xtalnum = 1;
	 else if (crystal == "C") Xtalnum = 2;
	 else if (crystal == "D" || crystal == "d") Xtalnum = 3;
	 if (Xtalnum < 0) cout << "TExogam2REA::Init() : problem with ECC/Xtalnum" << endl;

         // signal type (Energy or Time)
         if (label.find_last_of("_") != string::npos) 
	    sigtype = label.substr(label.find_last_of("_")+1);
	 else sigtype = "";

         if ((sigtype == "6MeV")||(sigtype == "6MEV")) {
	    fTypeMap[lbl] = ECCE;
	    fParameterMap[lbl] = channum*4 + Xtalnum;
	 }
	 else if (sigtype == "TAC" || sigtype == "tac") {
	    fTypeMap[lbl] = ECCT;
	    fParameterMap[lbl] = channum*4 + Xtalnum;
 //           cout << label << " " << fTypeMap[lbl] << "  " << fParameterMap[lbl] << endl;
	 }
	 else if ((sigtype == "20MeV")||(sigtype == "20MEV")){} // we don't look at 20 MeV range here
	 else {
	   cout << "TExogam2REA::Init() : problem reading Exogam2/ECC label" << sigtype << endl;
	    status = false;
	    cout<<" I read this lable " <<label<<endl;
	 }
      }

      // Segments
      if (label.compare(0,5,"GOCCE") == 0) {  
	 fLabelMap[lbl] = label;
	 status =true;

         // clover number
         channum = atoi(label.substr(5,label.find_first_of("_")).c_str());
	 // crystal name
	 crystal = label.substr(label.find_first_of("_")+1,1);

         // crystal number
         if (crystal == "A")      Xtalnum = 0;
	 else if (crystal == "B") Xtalnum = 1;
	 else if (crystal == "C") Xtalnum = 2;
	 else if (crystal == "D") Xtalnum = 3;
	 if (Xtalnum < 0) cout << "TExogam2REA::Init() : problem with GOCCE/Xtalnum" << endl;

         // segment number
         segnum = atoi(label.substr(label.find_first_of("_")+3,1).c_str());

//         cout << label << endl;
//         cout << "clo " << channum << " cris " << Xtalnum << " seg " << segnum << endl;

         // signal type (Energy or Time)
         if (label.find_last_of("_") != string::npos) 
	    sigtype = label.substr(label.find_last_of("_")+1);
	 else sigtype = "";

         if (sigtype == "E") {
	    fTypeMap[lbl] = GOCCEE;
	    fParameterMap[lbl] = channum*16 + Xtalnum*4 + segnum;
	 }
	 else if (sigtype == "T") {	// never used in E530 exp.
	    fTypeMap[lbl] = GOCCET;
	    fParameterMap[lbl] = channum*16 + Xtalnum*4 + segnum;
	 }
	 else {
	    cout << "TExogam2REA::Init() : problem reading Exogam2/GOCCE label" << endl;
	    status = false;
	 }
      }
      
      //ESS
      if (label.compare(0,3,"ESS") == 0) {
	fLabelMap[lbl] = label;
	strcpy(labels,params->GetParNameFromIndex(index));
	status =true;
	schaine.Form("%s",labels);
	schaine.ReplaceAll("ESS","");
	toks= schaine.Tokenize("_");
	channum=((TObjString* )toks->At(0))->GetString().Atoi();
	if(schaine.Contains("Energy")){
		fTypeMap[lbl] =ESSE;
		fParameterMap[lbl] = channum;
	}
	else if(schaine.Contains("Q")){
		fTypeMap[lbl] =ESSTQ;
		if (schaine.Contains("QA"))      Xtalnum = 0;
	 	else if (schaine.Contains("QB")) Xtalnum = 1;
	 	else if (schaine.Contains("QC")) Xtalnum = 2;
	 	else if (schaine.Contains("QD")) Xtalnum = 3;
		fParameterMap[lbl] = channum*16 + Xtalnum;
	}
        else if(schaine.Contains("TDC")){
		fTypeMap[lbl] =ESST;
		fParameterMap[lbl] = channum;
	}
	else if(schaine.Contains("Pattern")){
		fTypeMap[lbl] =ESSPat;
		fParameterMap[lbl] = channum;
	}
      
      }
	
     
   }
   return status;
}

bool TExogam2REA::Is(UShort_t lbl, Short_t val)
{
	bool result = false;
	return result;
}
bool TExogam2REA::IsMFMExo(MFMReaGenericFrame *frame)
{
   
   
   Int_t clo, cri;
   float Thresh;
   double valf,valf3;
   float valf2;
   bool result = false;
   bool debug = false;
   //float CaloSegForBrokenSeg, missingEnergy;
   //---------------PSA control
   int  EXO2BoardId, EXO2CrysId, MapFinger,MulNetCharge ;
   int   EXO2CrysIdLegacy;
   int Q,T,seg;
   float s[4]={0.,0.,0.,0.};

   
   //cout<<frame->GetTimeStamp()<<endl;
   seg=clo=cri=MapFinger=-100;
   EXO2CrysIdLegacy=10000;
   Q=T=EXO2BoardId=0;
   EXO2CrysId=1e3;
   if(LUTBool==false){
   	printf("\033[31m Error in TExogam2REA::IsMFMExo => The Look Up Table is not known: I won't be able to unpack the event   \033[m \n"); 
	}
   else{
   	/*if(debug)cerr<<"-------------------------"<<endl;
   	if(debug)cerr<<"ExoGetTGCristalId() "<<frame->ExoGetTGCristalId()<<endl;
	if(debug)cerr<<"ExoGetBoardId()     "<<frame->ExoGetBoardId() <<endl;
	if(debug)cerr<<"ExoGetInnerM(0)     "<<frame->ExoGetInnerM(0)<<endl;
	if(debug)cerr<<"ExoGetDeltaT()      "<<frame->ExoGetDeltaT() <<endl;
	if(debug)cerr<<"ExoGetBGO()  	    "<<frame->ExoGetBGO() <<endl;
	if(debug)cerr<<"ExoGetCsi()         "<<frame->ExoGetCsi() <<endl;
	if(debug)cerr<<"ExoGetStatus()      "<<(((UShort_t)frame->ExoGetStatus(2))) <<endl;
	if(debug)cerr<<"Outers NRJ : 1 - "<<frame->ExoGetOuter(0)<<"  2- "<<frame->ExoGetOuter(1)<<"  3- "<<frame->ExoGetOuter(2)<<"  4- "<<frame->ExoGetOuter(3)<<endl;
	*/
	
	if(frame->GetTypeTns() != REA_GENERIC_ENERGY_TIME_TYPE){
		printf("\033[31m ALARM in TExogam2REA::IsMFMExo => expected REA_GENERIC_ENERGY_TIME_TYPE  for board %d \033[m \n",frame->GetBoardId());
		printf("\033[31m ALARM in TExogam2REA::IsMFMExo Detected %d instead\033[m \n",frame->GetTypeTns());
		return false;
	}
	
	if(debug)cerr<<"--> Get Inside REA for EXOGAM2 Frame :: "<<endl;
        if(debug)cerr<<" (*) GetChannelId :: "<<frame->GetChannelId()<<endl;
        if(debug)cerr<<" (*) GetCristal   :: "<<frame->GetTGCristalId()<<endl;
        if(debug)cerr<<" (*) GetBoard     :: "<<frame->GetBoardId()<<endl;
        if(debug)cerr<<" (*) FrameSize    :: "<<frame->GetFrameSize()<<endl;
        if(debug)cerr<<" (*) Energy is    :: "<<frame->GetEnergy()<<endl;
        if(debug)cerr<<" (*) Time is      :: "<<frame->GetTime()<<endl;
	if(debug)cerr<<" (*) TS is        :: "<<frame->GetTimeStamp()<<endl;
	if(frame->GetTimeStamp()==0){printf("\033[31m ALARM in TExogam2REA::IsMFMExo => Board %d has TS=0 \033[m \n",frame->GetBoardId());}
	EXO2CrysId=frame->GetChannelId(); //return cell ID
        EXO2BoardId=frame->GetBoardId();
	T=frame->GetTime();
	Q=frame->GetEnergy();
	if(BoolSpec)TimeStampDiff->Fill(frame->GetTimeStamp()-PrevTS);
	//cout<<frame->GetTimeStamp()-PrevTS<<endl;
	
	//Now Reorganizing the data set
	
	if(EXO2CrysId>=0&&EXO2CrysId<8){ //re-create the half board logic of the EXOGAM2 firmware
	
		EXO2CrysIdLegacy=0;
	}
	else if(EXO2CrysId>=8&&EXO2CrysId<16){
	
		EXO2CrysId=EXO2CrysId-8;
		EXO2CrysIdLegacy=1;
	}
	
	else {
		printf("\033[31m Error in TExogam2REA::IsMFMExo => Impossible  EXO2CrysId \033[m \n"); 
	}
			
	
	
   	clo=Current_Numexo2_cfg_clo[EXO2BoardId][EXO2CrysIdLegacy];
   	cri=Current_Numexo2_cfg_cri[EXO2BoardId][EXO2CrysIdLegacy];
   	MapFinger=clo*4+cri;
   	if(clo>=0){
		result=true;
		}
   	else {
		printf("\033[31m Error in TExogam2REA::IsMFMExo => This IP (%d) is not known from the Look Up Table   \033[m \n",EXO2BoardId); 
   		result=false;
   	}
   }
   
   //if uncorrelated because of TDC then return false to save CPU time
  // if(frame->ExoGetDeltaT()>60000)result=false; //if uncorrelated because of TDC then return false to save CPU time
   
   if(IsCloverActive(clo)&&result&&Q>0){ 
   	if(debug)cerr<<" TExogam2::REA::: start packing  "<< EXO2CrysId<<endl; 
        
   	if(EXO2CrysId==0){//core 6 MeV
	
		valf=Cal(Q,ECoef[MapFinger][0],ECoef[MapFinger][1],ECoef[MapFinger][2]); 
		valf2=Cal(T,TCoef[MapFinger][0],TCoef[MapFinger][1],TCoef[MapFinger][2]);
		
		fExogam2READata->SetECCEClover(clo);
       		fExogam2READata->SetECCECristal(cri);
       		fExogam2READata->SetECCEDetNbr(MapFinger);
       		fExogam2READata->SetECCEEnergy(valf);
       
       		fExogam2READata->SetECCTClover(clo);
       		fExogam2READata->SetECCTCristal(cri);
       		fExogam2READata->SetECCTTime(valf2); // Delta is the TAC value between the CFD and the external ref
		fExogam2READata->SetfTimeStamps(frame->GetTimeStamp(),MapFinger);
		
		if(BoolSpec){ //Core Specta filling
			fMyHistoNCriERaw->Fill(clo*10+cri,Q); 
			fMyHistoPatternECCE->Fill(clo*10+cri);
			fMyHistoECCECal[clo*100+10*cri]->Fill(valf);
			BoardIdHist->Fill(EXO2BoardId);
		
			fMyHistoSumECCE->Fill(valf);
			fMyHistoSumECCE_ECCT->Fill(valf,valf2);
		
			fMyHistoCloverECal[clo]->Fill(valf);
			fMyHistoNCriECal->Fill(MapFinger,valf);
			fMyHistoNCriTRaw->Fill(MapFinger,T);
			if(valf2>0)fMyHistoPatternECCT->Fill(clo*10+cri);
			fMyHistoECCTRaw[clo*100+10*cri]->Fill(T);
		
			fMyHistoECCTCal[clo*100+10*cri]->Fill(valf2);
		
			fMyHistoSumECCT->Fill(valf2);
			fMyHistoCloverTCal[clo]->Fill(valf2);
			fMyHistoNCriTCal->Fill(MapFinger,valf2);
		}
		
	
	
	//search for standard Doppler correction
		if(valf>MaxCrisEval[clo]){
       			MaxCrisEval[clo]=valf;
        		MaxCris[clo]=cri;
       		}
  	}
	else if(EXO2CrysId==1){ //20 Mev Core
	
	}
	else if (EXO2CrysId>1&&EXO2CrysId<6){ //segments
		seg=EXO2CrysId-2;
		fExogam2READata->SetGOCCEEClover(clo);
		fExogam2READata->SetGOCCEECristal(cri);
		fExogam2READata->SetGOCCEESegment(seg);
		
		fExogam2READata->SetGOCCETClover(clo);
		fExogam2READata->SetGOCCETCristal(cri);
		fExogam2READata->SetGOCCETSegment(seg);
		
		
		fExogam2READata->SetGOCCETTime(T);
		fExogam2READata->SetfTimeStampsSegment(frame->GetTimeStamp());
		valf3=Cal(Q,ECoef_G[seg+4*(MapFinger)][0],ECoef_G[seg+4*(MapFinger)][1],ECoef_G[seg+4*(MapFinger)][2]);
		fExogam2READata->SetGOCCEEEnergy(valf3);
				
		if(BoolSpec){
			fMyHistoPatternGOCCEE->Fill(clo*100+10*cri+seg);
			fMyHistoNSegERaw->Fill(clo*100+10*cri+seg,valf3);
			fMyHistoGOCCEE[clo*100+10*cri+seg]->Fill(valf3);
			fMyHistoGOCCET[clo*100+10*cri+seg]->Fill(T);
			fMyHistoSumGOCCEE->Fill(valf3);
			fMyHistoSumGOCCET->Fill(T);
		}
		
		
		s[seg]=valf3;
		MulNetCharge++;
		mulG[clo][cri]=mulG[clo][cri]+1;
		GOCCE_Pat[clo][cri][seg]=true;
		if(CalibDone){Thresh=30;}
       		
		else{Thresh=50;}
			
		if(valf3>Thresh){
       			mulGnrj[clo][cri]=mulGnrj[clo][cri]+1;
        		GOCCE_Ener[clo][cri][seg]=true;
       		}
		
		if(s[seg]>MaxSegEval[clo][cri]){ //only on Net Charge
       			MaxSegEval[clo][cri]=s[seg];
        		MaxSeg[clo][cri]=seg;
       		}
	
	}
	
	else if (EXO2CrysId==6){ //bgo
		fExogam2READata->SetESSBGOClover(clo);
       		fExogam2READata->SetESSBGOCristal(cri);
       		fExogam2READata->SetESSBGOE(Q);
       		fExogam2READata->SetESSBGOT(T);
		fExogam2READata->SetfTimeStampsBGO(frame->GetTimeStamp());
		if(Q>ESSThreshold){ 
			NoComptonCore[clo][cri]=false;
       			NoCompton[clo]=false;
			if(BoolSpec&&IsCloverActive(clo)){
       				fMyHistoESS_BGO[clo*100+10*cri]->Fill(Q);
				fMyHistoESS_BGOT[clo*100+10*cri]->Fill(T);
				fMyHistoPatternESSTQ->Fill(clo*10+cri,Q);
       			}
		}
       
	}
	else if (EXO2CrysId==7){ //csi
		fExogam2READata->SetESSCSIClover(clo);
       		fExogam2READata->SetESSCSICristal(cri);
       		fExogam2READata->SetESSCSIE(Q);
       		fExogam2READata->SetESSCSIT(T);		
		fExogam2READata->SetfTimeStampsCSI(frame->GetTimeStamp());

		if(Q>ESSThreshold){
			NoComptonCore[clo][cri]=false;
       			NoCompton[clo]=false;
			if(BoolSpec&&IsCloverActive(clo)){
       				fMyHistoESS_CSI[clo*100+10*cri]->Fill(Q);
				fMyHistoESS_CSIT[clo*100+10*cri]->Fill(T);
				fMyHistoPatternESSTQ->Fill(clo*10+cri,Q);
       			}
			
		}
	}
	
	else{
		printf("\033[31m Error in TExogam2REA::IsMFMExo tag 2 => Impossible   EXO2CrysId \033[m \n"); 
	}
	
	
       /*

       //Now treat the case of broken Segment if declared :: calib must be done
       if(BrokenSegBool[clo][cri]&&CalibDone){
       		CaloSegForBrokenSeg=missingEnergy=0;	
       		for(Int_t seg=0;seg<4;seg++){
			if(psa[seg]==0&&s[seg]>30){CaloSegForBrokenSeg=CaloSegForBrokenSeg+s[seg];}
		}
		missingEnergy=valf-CaloSegForBrokenSeg; //valf is core energy; calculate missing energy
       		s[BrokenSeg[clo][cri]]=missingEnergy;
		psa[BrokenSeg[clo][cri]]=0;
		if(BoolSpec)fMyHistoGOCCEE[clo*100+10*cri+BrokenSeg[clo][cri]]->Fill(missingEnergy);
       }
       
*/
 

   }//if core Id>=0
   
     if(debug)cerr<<" TExogam2::REA::Is() completed  "<<endl; 

   return result;
}



bool TExogam2REA::Treat()
{
int clo,cri,seg,cloverMul,id1,id2,EnergyAddTQDCMAPFINGER[16],clo1,cri1,clo2,cri2;
char cristal[5]={'A','B','C','D','\0'};
float EnergyAddTQ[16],EnergyAddTC[16],EnergyAdd[16],mulCrysPerClover[16],EnergyAddTQDC[16];
float Theta_Gamma, Phi_Gamma;
bool IsPrompt[16][4];
bool IsSegPrompt[16][4][4];
float SumCalorimeter;
float CoreLocal[16][4], SumSeg[16][4];
Long64_t TS1, TS2;
TS1=TS2=0;

SumCalorimeter=0;
	//------------------Time Treat
	for(Int_t c=0;c<16;c++){ //clo
		for(Int_t d=0;d<4;d++){ //cri
			IsPrompt[c][d]=false;
			CoreLocal[c][d]=SumSeg[c][d]=0;
			for(Int_t s=0;s<4;s++){ //seg
				IsSegPrompt[c][d][s]=false;
			}
		}
	}


	for (UShort_t i = 0; i < fExogam2READata->GetECCTMult(); i++) {
		clo=fExogam2READata->GetECCTClover(i);
		cri=fExogam2READata->GetECCTCristal(i);
		if(fExogam2READata->GetECCTTime(i)>=promptL&&fExogam2READata->GetECCTTime(i)<=promptH){
				IsPrompt[clo][cri]=true;
		}
		
	}
	
	for (UShort_t i = 0; i < fExogam2READata->GetGOCCETMult(); i++) {
		clo=fExogam2READata->GetGOCCETClover(i);
		cri=fExogam2READata->GetGOCCETCristal(i);
		seg=fExogam2READata->GetGOCCETSegment(i);
		//cout<<i<<"  "<<promptL<<" - " <<promptH<<endl;
		//cout<<fExogam2READata->GetGOCCETTime(i)<<endl;
		if(fExogam2READata->GetGOCCETTime(i)>=promptL&&fExogam2READata->GetGOCCETTime(i)<=promptH){
				IsSegPrompt[clo][cri][seg]=true;
		}
	
	}
	
	
	for (UShort_t i = 0; i < fExogam2READata->GetGOCCEEMult(); i++) {
		for (UShort_t j = 0; j < fExogam2READata->GetGOCCEEMult(); j++) {
				id1=fExogam2READata->GetGOCCEEClover(i)*16+fExogam2READata->GetGOCCEECristal(i)*10+fExogam2READata->GetGOCCEESegment(i);
				id2=fExogam2READata->GetGOCCEEClover(j)*16+fExogam2READata->GetGOCCEECristal(j)*10+fExogam2READata->GetGOCCEESegment(j);
				if(id1!=id2&&BoolSpec&&fExogam2READata->GetGOCCEEEnergy(i)>100&&fExogam2READata->GetGOCCEEEnergy(j)>100){fMyHistoSegSeg->Fill(id1,id2);}
				
		}
	}
	
	
	//--------------Specific REA consistency check
	//cout<<" Self check REA  "<<endl;
	for (UShort_t i = 0; i < fExogam2READata->GetECCEMult(); i++) {
		clo=fExogam2READata->GetECCEClover(i);
		cri=fExogam2READata->GetECCECristal(i);
		if(fExogam2READata->GetECCEEnergy(i)>80&&IsPrompt[clo][cri])CoreLocal[clo][cri]=fExogam2READata->GetECCEEnergy(i);
		//cout<<i<<" Clo "<<clo<<" Core "<<cri<<" nrj "<<CoreLocal[clo][cri]<<endl;
	}
	//cout<<" Segments Section "<<endl;
	for (UShort_t i = 0; i < fExogam2READata->GetGOCCEEMult(); i++) {
		clo=fExogam2READata->GetGOCCEEClover(i);
		cri=fExogam2READata->GetGOCCEECristal(i);
		seg=fExogam2READata->GetGOCCEESegment(i);
		if(fExogam2READata->GetGOCCEEEnergy(i)>80&&IsSegPrompt[clo][cri][seg])SumSeg[clo][cri]=SumSeg[clo][cri]+fExogam2READata->GetGOCCEEEnergy(i);
		//cout<<i<<" Clo "<<clo<<" Core "<<cri<<" nrj "<<SumSeg[clo][cri]<<endl;
		
	}
	for(Int_t c=0;c<16;c++){ //clo
		for(Int_t d=0;d<4;d++){ //cri
			if(CoreLocal[c][d]>0&&SumSeg[c][d]==0)NoNetCharge++;
			if(CoreLocal[c][d]==0&&SumSeg[c][d]>0)NoCoreCharge++;
		}
	}
	
	for(UShort_t i = 0; i < 16; i++) {
		for(UShort_t j = 0; j < 4; j++) {
			if(BoolSpec)fMyHistoSumSeg_Core->Fill(CoreLocal[i][j],SumSeg[i][j]);
		}
	}
	
	
	//fMyHistoSumSeg_Core->Fill(CoreLocal[3][0],SumSeg[3][0]);
	//------------------Energy Treat
	if(fExogam2READata->GetECCEMult()>1){
		for (UShort_t i = 0; i < fExogam2READata->GetECCEMult(); i++) {
			for (UShort_t j = 0; j < fExogam2READata->GetECCEMult(); j++) {
				if(j!=i){
					clo1=fExogam2READata->GetECCEClover(i);
					cri1=fExogam2READata->GetECCECristal(i);
					clo2=fExogam2READata->GetECCEClover(j);
					cri2=fExogam2READata->GetECCECristal(j);
					if(BoolSpec&&fExogam2READata->GetECCEEnergy(i)>0&&fExogam2READata->GetECCEEnergy(j)>0&&IsPrompt[clo1][cri1]&&IsPrompt[clo2][cri2]){
						id1=fExogam2READata->GetECCEDetNbr(i);
						id2=fExogam2READata->GetECCEDetNbr(j);
						
						TS1=fExogam2READata->GetfTimeStamps(id1);
						TS1=fExogam2READata->GetfTimeStamps(id2);
						if(BoolSpec&&TMath::Abs(TS1-TS2)<5){
							fMyHistoGammaGammaCore->Fill(fExogam2READata->GetECCEEnergy(i),fExogam2READata->GetECCEEnergy(j)); //all core gg
							if(fExogam2READata->GetECCEClover(i)==fExogam2READata->GetECCEClover(j)){ //if part of the same clover; then gg only on diagonal cristal
								if(IsDiagonal(fExogam2READata->GetECCECristal(i),fExogam2READata->GetECCECristal(j)))fMyHistoGammaGammaCoreDiag->Fill(fExogam2READata->GetECCEEnergy(i),fExogam2READata->GetECCEEnergy(j));
							}
							else {
								fMyHistoGammaGammaCoreDiag->Fill(fExogam2READata->GetECCEEnergy(i),fExogam2READata->GetECCEEnergy(j));
							}
						}
						//if(BoolSpec)fMyHistoTT[id1][id2]->Fill(Cal(fExogam2READata->GetfTimeStamps(id1),0.,10.,0)-Cal(fExogam2READata->GetfTimeStamps(id2),0.,10.,0));
						if(BoolSpec)fMyHistoTTAll->Fill(Cal(fExogam2READata->GetfTimeStamps(id1),0.,10.,0)-Cal(fExogam2READata->GetfTimeStamps(id2),0.,10.,0));
					}
				}
			}
		}
	}
		
	for(Int_t c=0;c<16;c++){EnergyAddTQDCMAPFINGER[c]=EnergyAddTQ[c]=EnergyAddTC[c]=EnergyAdd[c]=mulCrysPerClover[c]=EnergyAddTQDC[c]=0.;}

	cloverMul=0;
	for (UShort_t i = 0; i < fExogam2READata->GetECCEMult(); i++) {
		clo=fExogam2READata->GetECCEClover(i);
		cri=fExogam2READata->GetECCECristal(i);
		//cerr<<clo<<" IN "<<cri<<endl;
	        
		if(IsCloverActive(clo)==false){continue;}
		
		//cerr<<clo<<" OUT "<<cri<<endl;

		mulCrysPerClover[clo]++;
		mulECC[clo][cri]++;
		if(mulG[clo][cri]==4)mulECCG[clo][cri]++;
		if(mulGnrj[clo][cri]>0)mulECCGnrj[clo][cri]++;
		//cerr<<mulGnrj[clo][cri]<<endl;
//------------------------------------------------------------------------------------------------------------------HERE the prompt cond!!!!!!!!!!!!!!!!!!
		if(IsPrompt[clo][cri]==false)continue;
		
		if(NoComptonCore[clo][cri]==false){
			NoComptonCoreCounter[clo][cri]++;
			if(BoolSpec)fMyHistoECCEVetoed[clo*100+10*cri]->Fill(fExogam2READata->GetECCEEnergy(i));
		}
		else{
			if(BoolSpec)fMyHistoECCEAccepted[clo*100+10*cri]->Fill(fExogam2READata->GetECCEEnergy(i));
			//EnergyAddTQ[clo]=EnergyAddTQ[clo]+fExogam2READata->GetECCEEnergy(i);
		}
		
		if(NoCompton[clo]&&IsPrompt[clo][cri]){EnergyAddTC[clo]=EnergyAddTC[clo]+fExogam2READata->GetECCEEnergy(i);}
		
		//here is the AddBack
		if(fExogam2READata->GetECCEEnergy(i)>10)EnergyAdd[clo]=EnergyAdd[clo]+fExogam2READata->GetECCEEnergy(i); 
		
		
		//---------------------------------------------------------
		for(Int_t k =0 ; k<4;k++){
			if(Goccetrack&&GOCCE_Pat[clo][cri][k]==false)printf("\033[31m GOCCE %d Crys %c Seg %d No Data   \033[m \n",clo,cristal[cri],k);
			if(Goccetrack&&GOCCE_Ener[clo][cri][k]==false)printf("\033[31m GOCCE %d Crys %c Seg %d Only Pedestal   \033[m \n",clo,cristal[cri],k);
		}
		/*for(UShort_t j = 0; j < fExogam2READata->GetESSTQMult(); j++){
			cloESS=fExogam2READata->GetESSTQClover(j);
			if(cloESS==clo&&BoolSpec)fMyHistoECCEESSE[clo*100+10*cri]->Fill(fExogam2READata->GetECCEEnergy(i),fExogam2READata->GetESSTQBGO(j)+fExogam2READata->GetESSTQCSI(j));
		}*/
		
		//---------------------------------------------------------

   	}
	if(BoolSpec)fMyHistoMultiCrystal->Fill(fExogam2READata->GetECCEMult());
	if(BoolSpec)fMyHistoMultiAntiCompt->Fill(fExogam2READata->GetESSBGOMult()+fExogam2READata->GetESSCSIMult());


			


	//------------------Final Treat
	for(Int_t c=0;c<16;c++){ //determine the true Fold
		if(CloverPresent[c]==false) continue;
		
		if(mulCrysPerClover[c]>0){
			if(BoolSpec)fMyHistoMultiCrystalperClover->Fill(mulCrysPerClover[c]);
			cloverMul++;
		}

	}
	
     if(cloverMul<=MaxClovMul&&cloverMul>0){ //by default 16 but to be set by SetMaxClovMul();
	for(Int_t c=0;c<16;c++){
		
		if(CloverPresent[c]==false) continue;
		
		
		if(EnergyAddTC[c]>0){
			if(BoolSpec)fMyHistoSumExogam2->Fill(EnergyAddTC[c]);
			//Fold condition
			if(cloverMul==1&&BoolSpec)fMyHistoSumExogam2FoldCond->Fill(EnergyAddTC[c]);
			//----------------
			EnergyAddTQDCMAPFINGER[c]=c*4+MaxCris[c]; //save the Cristal Map Finger
			
			if(MaxSeg[c][MaxCris[c]]<1000){ //if Max Seg is different than the Init Value
				Theta_Gamma=GammaAngleSegTheta[c][MaxCris[c]][MaxSeg[c][MaxCris[c]]];
				Phi_Gamma=GammaAngleSegPhi[c][MaxCris[c]][MaxSeg[c][MaxCris[c]]];
				//cout<<c <<"I am using the Seg for DC"<<endl;
			}
			else{
				Theta_Gamma=GammaAngleCoreTheta[c][MaxCris[c]];
				Phi_Gamma=GammaAngleCorePhi[c][MaxCris[c]];
				//if(IsActivateGOCCE())printf("\033[31m I am using the core for DC for this clover ********* \033[m \n");
			}
			//cerr<<"Do DC "<<Theta_Gamma*180./3.14159<<" ::  "<<Beta<<endl;
			if(BoolSpec)fMyHistoSumExogam2DC->Fill(Doppler_Correction(Theta_Gamma,0.0,0., 0., Beta, EnergyAddTC[c]));
			
			if(noyau>=0){
				if(BoolSpec)fMyHistoSumExogam2DCNucId[noyau]->Fill(Doppler_Correction(Theta_Gamma,0.0,0., 0., Beta, EnergyAddTC[c]));
				if(BoolSpec)TransverseMomentumGamma->Fill(FreeParam1,Doppler_Correction(Theta_Gamma,0.0,0., 0., Beta, EnergyAddTC[c]));
			}
			//if(noyau==2)SumCalorimeter=SumCalorimeter+Doppler_Correction(Theta_Gamma,0.0,0., 0., Beta, EnergyAddTC[c]); //gated on Sulfur
			SumCalorimeter=SumCalorimeter+Doppler_Correction(Theta_Gamma,0.0,0., 0., Beta, EnergyAddTC[c]); 
			
			if(BoolSpec)fMyHistoSumExogam2DCClover->Fill(c,Doppler_Correction(Theta_Gamma,0.0,0., 0., Beta, EnergyAddTC[c]));
			if(BoolSpec)fMyHistoSumExogam2Clover->Fill(c,Doppler_Correction(Theta_Gamma,0.0,0., 0., 0, EnergyAddTC[c]));


			//Fold condition
			if(cloverMul==1&&BoolSpec)fMyHistoSumExogam2DCFoldCond->Fill(Doppler_Correction(Theta_Gamma,0.0,0., 0., Beta, EnergyAddTC[c]));
			//----------------
			EnergyAddTQDC[c]=Doppler_Correction(Theta_Gamma,Phi_Gamma,0., 0., Beta, EnergyAddTC[c]);

			if(BoolSpec){
				fMyHistoAngletheta->Fill(Theta_Gamma*180./3.14159);
				fMyHistoAnglephi->Fill(Phi_Gamma*180./3.14159);
				fMyHistoCloverECalACAdd_DC[c]->Fill(Doppler_Correction(Theta_Gamma,Phi_Gamma,0., 0., Beta, EnergyAddTC[c]));
				fMyHistoSumExogam22D->Fill(Theta_Gamma*180./3.14159,EnergyAddTC[c]);
				fMyHistoSumExogam2DC2D->Fill(Theta_Gamma*180./3.14159,Doppler_Correction(Theta_Gamma,Phi_Gamma,0., 0., Beta, EnergyAddTC[c]));
			}
			
		}
		
		
		//Analyzed Branches
			fExogam2READata->SetGammaEnergy(EnergyAddTC[c]);
			fExogam2READata->SetGammaTheta(Theta_Gamma);
			fExogam2READata->SetGammaPhi(Phi_Gamma);
			fExogam2READata->SetGammaCoreId(EnergyAddTQDCMAPFINGER[c]);
		

		
		if(EnergyAddTC[c]>0&&BoolSpec){fMyHistoCloverECalACAdd_TC[c]->Fill(EnergyAddTC[c]);}
		if(EnergyAdd[c]>0&&BoolSpec)  {fMyHistoCloverECalAdd[c]->Fill(EnergyAdd[c]);}
		
	}
     }
	if(SumCalorimeter>0&&BoolSpec)fMyHistoSumExogam2Calorimeter->Fill(SumCalorimeter);
	
	//create gamma-gamma matrix
	if(cloverMul>1){
		for(Int_t c=0;c<16;c++){
			if(CloverPresent[c]==false) continue;
			
			for(Int_t d=0;d<16;d++){
				if(CloverPresent[d]==false) continue;
				TS1=fExogam2READata->GetfTimeStamps(EnergyAddTQDCMAPFINGER[c]);
				TS2=fExogam2READata->GetfTimeStamps(EnergyAddTQDCMAPFINGER[d]);
				if(c!=d&&EnergyAddTQDC[c]>0&&EnergyAddTQDC[d]>0&&
					TMath::Abs(TS1-TS2)<5
					&&BoolSpec)fMyHistoGammaGamma->Fill(EnergyAddTQDC[c],EnergyAddTQDC[d]); // with DC
			}
		}
	}
	
	if(BoolSpec)fMyHistoMultiClover->Fill(cloverMul);

   return true;
}
bool TExogam2REA::CounterReset()
{	
	for(int k=0 ; k<16 ; k++){
		for(int l=0;l<4;l++){
			NoComptonCoreCounter[k][l]=0;
			mulECC[k][l]=0;
			mulECCG[k][l]=0;
			mulECCGnrj[k][l]=0;
		}
	}
	
	
return true;
}
bool TExogam2REA::Counter()
{	

bool GOCCEfailure1,GOCCEfailure2;
float div,div3;
int div2,tvb;
char cristal[5]={'A','B','C','D','\0'}; 
GOCCEfailure1=GOCCEfailure2=false;
tvb=0; 
	for(Int_t i=0; i<16;i++){
		if(IsCloverActive(i)){
		     cout<< "   "<<endl;
		     for(Int_t j=0;j<4;j++){
		       if(mulECC[i][j]>0){
		       
		        	div=(1.-1.*mulECCG[i][j]/mulECC[i][j])*100.;
				div3=(1-1.*mulECCGnrj[i][j]/mulECC[i][j])*100.;
				
				//GOCCE checking
				if(IsActivateGOCCE()){
					if(div>10.){
						//printf("\033[31m (REA) ECC%dCrys%c Missing GOCCE Ratio (Gated Word/Raw) = %2.3f percent \033[m \n",i, cristal[j],div);
						printf("\033[32m (REA) ECC%dCrys%c Missing GOCCE Ratio (Gated Nrj/Raw) = %2.3f percent \033[m \n",i, cristal[j],(1-1.*mulECCGnrj[i][j]/mulECC[i][j])*100.);
			        		GOCCEfailure1=true;
					}
			 		else if(div3>10.){
						
						//printf("\033[32m (REA) ECC%dCrys%c Missing GOCCE Ratio (Gated Word/Raw) = %2.3f percent \033[m \n",i, cristal[j],div);
						printf("\033[31m (REA) ECC%dCrys%c Missing GOCCE Ratio (Gated Nrj/Raw) = %2.3f percent \033[m \n",i, cristal[j],(1-1.*mulECCGnrj[i][j]/mulECC[i][j])*100.);
						GOCCEfailure2=true;
					}
					else{
						printf("\033[32m (REA) ECC%dCrys%c Missing GOCCE Ratio (Gated Word/Raw) = %2.3f percent \033[m \n",i, cristal[j],div);
						printf("\033[32m (REA) ECC%dCrys%c Missing GOCCE Ratio (Gated Nrj/Raw) = %2.3f percent \033[m \n",i, cristal[j],(1-1.*mulECCGnrj[i][j]/mulECC[i][j])*100.);
					}
				//fprintf(LogFile,"ECC%dCrys%c Missing GOCCE Ratio (Gated Word/Raw) = %2.3f \n",i, cristal[j],div);
				//fprintf(LogFile,"ECC%dCrys%c Missing GOCCE Ratio (Gated Nrj/Raw) = %2.3f \n",i, cristal[j],(1-1.*mulECCGnrj[i][j]/mulECC[i][j])*100.);

				}
				
				//ESS checking
				if(IsActivateESS()){
					div2=(int)((1.*NoComptonCoreCounter[i][j]/mulECC[i][j])*100);
					if(div2==0){printf("\033[31m (REA) ESS%dCryst%c without Data   \033[m \n",i, cristal[j]);}
					else{
						printf("\033[34m (REA) ECC%dCrys%c AC suppression ratio = %2.1f per cent \033[m \n",i, cristal[j],(1.*NoComptonCoreCounter[i][j]/mulECC[i][j])*100.);
						if(BoolSpec)fMyHistoCheckEss->Fill((1.*NoComptonCoreCounter[i][j]/mulECC[i][j])*100.);
						//fprintf(LogFile,"ECC%dCrys%c AC suppression ratio = %2.1f \n",i, cristal[j],(1.*NoComptonCoreCounter[i][j]/mulECC[i][j])*100.);
					}
				}
		        }
		       else {printf("\033[31m (REA) ECC%dCrys%c without Data   \033[m \n",i,cristal[j]);tvb++;}

		     }
		}
	}
	cout<< "   "<<endl;
	if(tvb==0){	printf("\033[36m *********  TVB all cores(REA)  are running ********* \033[m \n");
		        //fprintf(LogFile,"TVB all cores are running\n");
		}
	else{ printf("\033[31m *********  At least one core(REA)  is not running  (see above) ********* \033[m \n");
		//fprintf(LogFile,"At least one core is not running\n");
		}
	if(GOCCEfailure1&&IsActivateGOCCE())printf("\033[31m *********  At least one GOCCE (REA) is not running properly (see above) : No Enough Readout of the card ********* \033[m \n");
	if(GOCCEfailure2&&IsActivateGOCCE())printf("\033[31m *********  At least one GOCCE (REA) is not running properly (see above) : Only Pedestal ********* \033[m \n");
	
	
return true;
}

bool TExogam2REA::ACReject()
{
Int_t bin1,bin2,c1,c2;
Float_t div;
   if(BoolSpec){
	for(Int_t x=0;x<16;x++){
	  if(CloverPresent[x]==false) continue;
		for(Int_t a =1 ; a <3000;a++){
			bin1=fMyHistoCloverECalAdd[x]->GetBin(a);
			bin2=fMyHistoCloverECalACAdd_DC[x]->GetBin(a);
			c1=fMyHistoCloverECalAdd[x]->GetBinContent(bin1);
			c2=fMyHistoCloverECalACAdd_DC[x]->GetBinContent(bin2);
			if(c2>0){div=10.*c1/c2;}
			else {div=0.;}
			fMyHistoCloverECalACAddRejectF[x]->Fill(a,(int)(div));
		}
		
		for(Int_t g = 0 ; g <4 ; g++){
			for(Int_t a =1 ; a <3000;a++){
				bin1=fMyHistoECCECal[x*100+10*g]->GetBin(a);
				bin2=fMyHistoECCEAccepted[x*100+10*g]->GetBin(a);
				c1=fMyHistoECCECal[x*100+10*g]->GetBinContent(bin1);
				c2=fMyHistoECCEAccepted[x*100+10*g]->GetBinContent(bin2);
				if(c2>0){div=10.*c1/c2;}
				else {div=0.;}
				fMyHistoCrysECalACRejectF[x*100+10*g]->Fill(a,(int)(div));
			}
		}		
	}
	
}
	
	
return true;
}
bool TExogam2REA::SetCloverPosition(int ECC_VXInb,int flange, float distance)
{


struct Clover_struc Result;
int i,j;
float Real_position=distance+7.0; //mm + distance capot-crystal (np18-22-03)
//Elements definition
TVector3 flange12(D_CloveFlange_Targ12,0,0);  	//clover vector
TVector3 flange12Crist[4];			//crystals vectors
TVector3 flange12CristSeg[4][4];		//segments vectors
//Initialisation by default
TVector3 v1(1,0,0);
TVector3 v2(0,1,0);
TVector3 v3(0,0,1);

	for(i=0;i<4;i++){
		flange12Crist[i].SetX(1.);	
		flange12Crist[i].SetY(1.);	
		flange12Crist[i].SetZ(1.);
		flange12Crist[i].SetTheta(1.);	
		flange12Crist[i].SetPhi(1.);	
		flange12Crist[i].SetMag(1.);	
		for(j=0;j<4;j++){
			flange12CristSeg[i][j].SetX(1.);
			flange12CristSeg[i][j].SetY(1.);
			flange12CristSeg[i][j].SetZ(1.);
			flange12CristSeg[i][j].SetTheta(1.);
			flange12CristSeg[i][j].SetPhi(1.);
			flange12CristSeg[i][j].SetMag(1.);
		}
	}
/*Initialisation of all the vectors to the flange 12 BUT with the correct distance for the asked clover
 between target and germanium crystal*/
	
	//Clover position
	flange12.SetTheta(90.0*TMath::Pi()/180.0);
	flange12.SetPhi(0.0*TMath::Pi()/180.0);
	flange12.SetMag(Real_position); 

//segment mean angle depends on the interaction depth due to the crystal shape; this creates a linear function with InteractionDepth as input
// see plan np18-22-05 && np18-22-04
TF1 *ShapeC = new TF1("ShapeC","0.132*x+20.54");
TF1 *ShapeS1 = new TF1("ShapeS1","0.273*x+30.81");  //gd
TF1 *ShapeS2 = new TF1("ShapeS2","0.066*x+10.27"); //pt

float Exogam2_Crystal_Center; 
float Exogam2_Segment_Pos1,Exogam2_Segment_Pos2 ;

	if(InteractionDepth>=30){Exogam2_Crystal_Center= 24.5;Exogam2_Segment_Pos1=39.; Exogam2_Segment_Pos2=12.25; }
	else{Exogam2_Crystal_Center=ShapeC->Eval(InteractionDepth);
	     Exogam2_Segment_Pos1=ShapeS1->Eval(InteractionDepth); //gd
	     Exogam2_Segment_Pos2=ShapeS2->Eval(InteractionDepth); //pt
	}
	


	//Crystal A
	flange12Crist[0].SetY(flange12.Y()+(Exogam2_Crystal_Center));
	flange12Crist[0].SetZ(flange12.Z()-(Exogam2_Crystal_Center));
	flange12Crist[0].SetX(Real_position);
		//segment1
	flange12CristSeg[0][0].SetY(flange12.Y()+(Exogam2_Segment_Pos1)); //gd
	flange12CristSeg[0][0].SetZ(flange12.Z()-(Exogam2_Segment_Pos1)); //gd
	flange12CristSeg[0][0].SetX(Real_position);
		//segment2
	flange12CristSeg[0][1].SetY(flange12.Y()+(Exogam2_Segment_Pos2));
	flange12CristSeg[0][1].SetZ(flange12.Z()-(Exogam2_Segment_Pos1));
	flange12CristSeg[0][1].SetX(Real_position);
		//segment3
	flange12CristSeg[0][2].SetY(flange12.Y()+(Exogam2_Segment_Pos2));
	flange12CristSeg[0][2].SetZ(flange12.Z()-(Exogam2_Segment_Pos2));
	flange12CristSeg[0][2].SetX(Real_position);
		//segment4
	flange12CristSeg[0][3].SetY(flange12.Y()+(Exogam2_Segment_Pos1));
	flange12CristSeg[0][3].SetZ(flange12.Z()-(Exogam2_Segment_Pos2));
	flange12CristSeg[0][3].SetX(Real_position);

	//Crystal B
	flange12Crist[1].SetY(flange12.Y()-(Exogam2_Crystal_Center));
	flange12Crist[1].SetZ(flange12.Z()-(Exogam2_Crystal_Center));
	flange12Crist[1].SetX(Real_position);
		//segment1
	flange12CristSeg[1][0].SetY(flange12.Y()-(Exogam2_Segment_Pos1));
	flange12CristSeg[1][0].SetZ(flange12.Z()-(Exogam2_Segment_Pos1));
	flange12CristSeg[1][0].SetX(Real_position);
		//segment2
	flange12CristSeg[1][1].SetY(flange12.Y()-(Exogam2_Segment_Pos1));
	flange12CristSeg[1][1].SetZ(flange12.Z()-(Exogam2_Segment_Pos2));
	flange12CristSeg[1][1].SetX(Real_position);
		//segment3
	flange12CristSeg[1][2].SetY(flange12.Y()-(Exogam2_Segment_Pos2));
	flange12CristSeg[1][2].SetZ(flange12.Z()-(Exogam2_Segment_Pos2));
	flange12CristSeg[1][2].SetX(Real_position);
		//segment4
	flange12CristSeg[1][3].SetY(flange12.Y()-(Exogam2_Segment_Pos2));
	flange12CristSeg[1][3].SetZ(flange12.Z()-(Exogam2_Segment_Pos1));
	flange12CristSeg[1][3].SetX(Real_position);
	
	//Crystal C
	flange12Crist[2].SetY(flange12.Y()-(Exogam2_Crystal_Center));
	flange12Crist[2].SetZ(flange12.Z()+(Exogam2_Crystal_Center));
	flange12Crist[2].SetX(Real_position);
		//segment1
	flange12CristSeg[2][0].SetY(flange12.Y()-(Exogam2_Segment_Pos1));
	flange12CristSeg[2][0].SetZ(flange12.Z()+(Exogam2_Segment_Pos1));
	flange12CristSeg[2][0].SetX(Real_position);
		//segment2
	flange12CristSeg[2][1].SetY(flange12.Y()-(Exogam2_Segment_Pos2));
	flange12CristSeg[2][1].SetZ(flange12.Z()+(Exogam2_Segment_Pos1));
	flange12CristSeg[2][1].SetX(Real_position);
		//segment3
	flange12CristSeg[2][2].SetY(flange12.Y()-(Exogam2_Segment_Pos2));
	flange12CristSeg[2][2].SetZ(flange12.Z()+(Exogam2_Segment_Pos2));
	flange12CristSeg[2][2].SetX(Real_position);
		//segment4
	flange12CristSeg[2][3].SetY(flange12.Y()-(Exogam2_Segment_Pos1));
	flange12CristSeg[2][3].SetZ(flange12.Z()+(Exogam2_Segment_Pos2));
	flange12CristSeg[2][3].SetX(Real_position);
	
	//Crystal D
	flange12Crist[3].SetY(flange12.Y()+(Exogam2_Crystal_Center));
	flange12Crist[3].SetZ(flange12.Z()+(Exogam2_Crystal_Center));
	flange12Crist[3].SetX(Real_position);
		//segment1
	flange12CristSeg[3][0].SetY(flange12.Y()+(Exogam2_Segment_Pos1));
	flange12CristSeg[3][0].SetZ(flange12.Z()+(Exogam2_Segment_Pos1));
	flange12CristSeg[3][0].SetX(Real_position);
		//segment2
	flange12CristSeg[3][1].SetY(flange12.Y()+(Exogam2_Segment_Pos1));
	flange12CristSeg[3][1].SetZ(flange12.Z()+(Exogam2_Segment_Pos2));
	flange12CristSeg[3][1].SetX(Real_position);
		//segment3
	flange12CristSeg[3][2].SetY(flange12.Y()+(Exogam2_Segment_Pos2));
	flange12CristSeg[3][2].SetZ(flange12.Z()+(Exogam2_Segment_Pos2));
	flange12CristSeg[3][2].SetX(Real_position);
		//segment4
	flange12CristSeg[3][3].SetY(flange12.Y()+(Exogam2_Segment_Pos2));
	flange12CristSeg[3][3].SetZ(flange12.Z()+(Exogam2_Segment_Pos1));
	flange12CristSeg[3][3].SetX(Real_position);
	
	if(IsCloverActive(ECC_VXInb)==false){}
	
	else if (flange >=1 && flange <=17){
	
			switch(flange){ //which flange ??
			
			case 1: 
				flange12.RotateY(-45.0*TMath::Pi()/180.0);
				for(i=0;i<4;i++){
					flange12Crist[i].RotateY(-45.0*TMath::Pi()/180.0);
					for(j=0;j<4;j++){
						flange12CristSeg[i][j].RotateY(-45.0*TMath::Pi()/180.0);
					}
				}
				break;
				
				
				
			case 2: 
				flange12.RotateY(90.0*TMath::Pi()/180.0);
				flange12.RotateX(-45.0*TMath::Pi()/180.0);
				for(i=0;i<4;i++){
					flange12Crist[i].RotateY(90.0*TMath::Pi()/180.0);
					flange12Crist[i].RotateX(-45.0*TMath::Pi()/180.0);
					for(j=0;j<4;j++){
						flange12CristSeg[i][j].RotateY(90.0*TMath::Pi()/180.0);
						flange12CristSeg[i][j].RotateX(-45.0*TMath::Pi()/180.0);
					}
				}
				break;
				
				
				
			case 3: 
				flange12.RotateY(90.0*TMath::Pi()/180.0);
				flange12.RotateX(45.0*TMath::Pi()/180.0);
				for(i=0;i<4;i++){
					flange12Crist[i].RotateY(90.0*TMath::Pi()/180.0);
					flange12Crist[i].RotateX(45.0*TMath::Pi()/180.0);
					for(j=0;j<4;j++){
						flange12CristSeg[i][j].RotateY(90.0*TMath::Pi()/180.0);
						flange12CristSeg[i][j].RotateX(45.0*TMath::Pi()/180.0);
					}
				}
				break;
				
				
				
			case 4:
				flange12.RotateY(45.0*TMath::Pi()/180.0);
				for(i=0;i<4;i++){
					flange12Crist[i].RotateY(45.0*TMath::Pi()/180.0);
					for(j=0;j<4;j++){
						flange12CristSeg[i][j].RotateY(45.0*TMath::Pi()/180.0);
					}
				}
				break;
				
				
				
			case 5: 
				flange12.RotateY(135.0*TMath::Pi()/180.0);
				for(i=0;i<4;i++){
					flange12Crist[i].RotateY(135.0*TMath::Pi()/180.0);
					for(j=0;j<4;j++){
						flange12CristSeg[i][j].RotateY(135.0*TMath::Pi()/180.0);
					}
				}
				break;
				
				
			case 6: 
				flange12.RotateY(90.0*TMath::Pi()/180.0);
				flange12.RotateX(90.0*TMath::Pi()/180.0);
				for(i=0;i<4;i++){
					flange12Crist[i].RotateY(90.0*TMath::Pi()/180.0);
					flange12Crist[i].RotateX(90.0*TMath::Pi()/180.0);
					for(j=0;j<4;j++){
						flange12CristSeg[i][j].RotateY(90.0*TMath::Pi()/180.0);
						flange12CristSeg[i][j].RotateX(90.0*TMath::Pi()/180.0);
					}
				}
				break;
				
			case 7:  
				flange12.RotateY(135.0*TMath::Pi()/180.0);
				flange12.RotateX(90.0*TMath::Pi()/180.0);
				for(i=0;i<4;i++){
					flange12Crist[i].RotateY(135.0*TMath::Pi()/180.0);
					flange12Crist[i].RotateX(90.0*TMath::Pi()/180.0);
					for(j=0;j<4;j++){
						flange12CristSeg[i][j].RotateY(135.0*TMath::Pi()/180.0);
						flange12CristSeg[i][j].RotateX(90.0*TMath::Pi()/180.0);
					}
				}
				break;
				
				
			case 8:   
				flange12.RotateY(180.0*TMath::Pi()/180.0);
				for(i=0;i<4;i++){
					flange12Crist[i].RotateY(180.0*TMath::Pi()/180.0);
					for(j=0;j<4;j++){
						flange12CristSeg[i][j].RotateY(180.0*TMath::Pi()/180.0);
					}
				}
				break;
				
				
			case 9:   
				flange12.RotateY(135.0*TMath::Pi()/180.0);
				flange12.RotateX(-90.0*TMath::Pi()/180.0);
				for(i=0;i<4;i++){
					flange12Crist[i].RotateY(135.0*TMath::Pi()/180.0);
					flange12Crist[i].RotateX(-90.0*TMath::Pi()/180.0);
					for(j=0;j<4;j++){
						flange12CristSeg[i][j].RotateY(135.0*TMath::Pi()/180.0);
						flange12CristSeg[i][j].RotateX(-90.0*TMath::Pi()/180.0);
					}
				}
				break;
				
				
			case 10:   
				flange12.RotateY(90.0*TMath::Pi()/180.0);
				flange12.RotateX(-90.0*TMath::Pi()/180.0);
				for(i=0;i<4;i++){
					flange12Crist[i].RotateY(90.0*TMath::Pi()/180.0);
					flange12Crist[i].RotateX(-90.0*TMath::Pi()/180.0);
					for(j=0;j<4;j++){
						flange12CristSeg[i][j].RotateY(90.0*TMath::Pi()/180.0);
						flange12CristSeg[i][j].RotateX(-90.0*TMath::Pi()/180.0);
					}
				}
				break;
				
				
			case 11:    
				flange12.RotateY(45.0*TMath::Pi()/180.0);
				flange12.RotateX(-90.0*TMath::Pi()/180.0);
				for(i=0;i<4;i++){
					flange12Crist[i].RotateY(45.0*TMath::Pi()/180.0);
					flange12Crist[i].RotateX(-90.0*TMath::Pi()/180.0);
					for(j=0;j<4;j++){
						flange12CristSeg[i][j].RotateY(45.0*TMath::Pi()/180.0);
						flange12CristSeg[i][j].RotateX(-90.0*TMath::Pi()/180.0);
					}
				}
				break;
				
				
			case 12: //Clover of initialisation --> not move
				break;
				
				
			case 13:    
				flange12.RotateY(45.0*TMath::Pi()/180.0);
				flange12.RotateX(90.0*TMath::Pi()/180.0);
				for(i=0;i<4;i++){
					flange12Crist[i].RotateY(45.0*TMath::Pi()/180.0);
					flange12Crist[i].RotateX(90.0*TMath::Pi()/180.0);
					for(j=0;j<4;j++){
						flange12CristSeg[i][j].RotateY(45.0*TMath::Pi()/180.0);
						flange12CristSeg[i][j].RotateX(90.0*TMath::Pi()/180.0);
					}
				}
				break;
				
				
			case 14:    
				flange12.RotateY(-135.0*TMath::Pi()/180.0);
				for(i=0;i<4;i++){
					flange12Crist[i].RotateY(-135.0*TMath::Pi()/180.0);
					for(j=0;j<4;j++){
						flange12CristSeg[i][j].RotateY(-135.0*TMath::Pi()/180.0);
					}
				}
				break;
				
				
			case 15:     
				flange12.RotateY(90.0*TMath::Pi()/180.0);
				flange12.RotateX(-135.0*TMath::Pi()/180.0);
				for(i=0;i<4;i++){
					flange12Crist[i].RotateY(90.0*TMath::Pi()/180.0);
					flange12Crist[i].RotateX(-135.0*TMath::Pi()/180.0);
					for(j=0;j<4;j++){
						flange12CristSeg[i][j].RotateY(90.0*TMath::Pi()/180.0);
						flange12CristSeg[i][j].RotateX(-135.0*TMath::Pi()/180.0);
					}
				}
				break;	
				
			case 16:    
				flange12.RotateY(90.0*TMath::Pi()/180.0);
				flange12.RotateX(135.0*TMath::Pi()/180.0);
				for(i=0;i<4;i++){
					flange12Crist[i].RotateY(90.0*TMath::Pi()/180.0);
					flange12Crist[i].RotateX(135.0*TMath::Pi()/180.0);
					for(j=0;j<4;j++){
						flange12CristSeg[i][j].RotateY(90.0*TMath::Pi()/180.0);
						flange12CristSeg[i][j].RotateX(135.0*TMath::Pi()/180.0);
					}
				}
			
				break;
				
			case 17:
				flange12.RotateY(90.0*TMath::Pi()/180.0);
				for(i=0;i<4;i++){
					flange12Crist[i].RotateY(90.0*TMath::Pi()/180.0);
					for(j=0;j<4;j++){
						flange12CristSeg[i][j].RotateY(90.0*TMath::Pi()/180.0);
					}
				}
				break;	
				
			default : 
				break;
			}
		
		// Take into account the case of a target which is not at the center of the Exogam2 array
                // Target position
                TVector3 targetPos(TARGET_POSITION_X, TARGET_POSITION_Y, TARGET_POSITION_Z);

		// case of the selected Exogam2 detector
/*		for (Int_t k = 0; k < 3; k++) {
		   cout << "flange12 avant : " << flange12(k) << endl;
		   cout << "targetPos      : " << targetPos(k) << endl;
		}*/
		flange12 -= targetPos;
/*		for (Int_t k = 0; k < 3; k++) {
		   cout << "flange12 apres : " << flange12(k) << endl;
		}*/
		// loop on cristals
		for (Int_t ii = 0; ii < 4; ii++) {
		   flange12Crist[ii] -= targetPos;
		   // loop on segments
		   for (Int_t jj = 0; jj < 4; jj++) {
		      flange12CristSeg[ii][jj] -= targetPos;
		   }
		}
			
		/*
		printf(" flange %d theta %f   phi Proj %f \n",flange,flange12.Theta()*180.0/(TMath::Pi()),flange12.Phi()*180.0/(TMath::Pi()));
		for(i=0;i<4;i++){
			printf(" flange %d  cristal %d theta %f   phi Proj %f \n",
			flange,i+1,flange12Crist[i].Theta()*180.0/(TMath::Pi()),flange12Crist[i].Phi()*180.0/(TMath::Pi()));
			for(j=0;j<4;j++){
				printf(" flange %d  cristal %d  seg %d theta %f   phi Proj %f \n",
				flange,i+1,j+1,flange12CristSeg[i][j].Theta()*180.0/(TMath::Pi()),flange12CristSeg[i][j].Phi()*180.0/(TMath::Pi()));		
			}
		}
		 printf("#########################\n"); 
		*/
		
		
		//Output result in rad in the "Clover_struc" structur
		Result.Theta_Clover=flange12.Theta();
		Result.Phi_Clover=flange12.Phi();
		Result.X_Clover=flange12.X();
		Result.Y_Clover=flange12.Y();
		Result.Z_Clover=flange12.Z();
			
		for(i=0;i<4;i++){
			Result.Theta_Crystal[i]=GammaAngleCoreTheta[ECC_VXInb][i] = flange12Crist[i].Theta();
			Result.Phi_Crystal[i]  =GammaAngleCorePhi[ECC_VXInb][i]   = flange12Crist[i].Phi();
			Result.X_Crystal[i]=GammaPosCoreX[ECC_VXInb][i]=flange12Crist[i].X();
			Result.Y_Crystal[i]=GammaPosCoreY[ECC_VXInb][i]=flange12Crist[i].Y();
			Result.Z_Crystal[i]=GammaPosCoreZ[ECC_VXInb][i]=flange12Crist[i].Z();
			//cout <<" ECC " << ECC_VXInb<< " " << cristal[i] << "  "<<flange12Crist[i].Theta() *180./TMath::Pi()<<endl;		

			for(j=0;j<4;j++){
				Result.Theta_Crystal_Seg[i][j]= GammaAngleSegTheta[ECC_VXInb][i][j] =flange12CristSeg[i][j].Theta();
				Result.Phi_Crystal_Seg[i][j]  = GammaAngleSegPhi[ECC_VXInb][i][j]   =flange12CristSeg[i][j].Phi();
				Result.X_Crystal_Seg[i][j]=flange12CristSeg[i][j].X();
				Result.Y_Crystal_Seg[i][j]=flange12CristSeg[i][j].Y();
				Result.Z_Crystal_Seg[i][j]=flange12CristSeg[i][j].Z();	
			}
		}
		  
		 

	}
	
	else {printf("Bad flange number, flange %d  doesn't exist in your Exogam2 config : ECC%d!!!!!! \n",flange,ECC_VXInb);}


return true;
}
bool TExogam2REA::CheckCoreResolution(float EnergyC)
{
	TF1 *expectedReso = new TF1("expectedReso","1.41251+x*0.000760227",0,3000);

	TF1 *mfit = new TF1("mfit","gaus(0)+pol0(3)",EnergyC-100,EnergyC+100);
	mfit->SetParameter(0,10);
	mfit->SetParameter(1,EnergyC);
	mfit->SetParameter(3,60.);
	char cristal[5]={'A','B','C','D','\0'}; 
cout << "Now checking their resolution " << endl;

	for(Int_t clo=0;clo<16;clo++){
		for(Int_t cri=0;cri<4;cri++){
			if(BoolSpec&&IsCloverActive(clo)&&fMyHistoECCECal[clo*100+10*cri]->Integral(100,2000,"")>0){
				 
				 mfit->SetParameter(0,fMyHistoECCECal[clo*100+10*cri]->GetBinContent(EnergyC));
				 mfit->SetParameter(3,fMyHistoECCECal[clo*100+10*cri]->GetBinContent(EnergyC+30));
				 mfit->SetParameter(2,expectedReso->Eval(EnergyC));

				fMyHistoECCECal[clo*100+10*cri]->Fit(mfit,"QN","",EnergyC-10,EnergyC+10);
				if(mfit->GetParameter(2)*2.35>0&&mfit->GetParameter(2)*2.35<1.1*expectedReso->Eval(EnergyC)){
					printf("\033[32m ECC%dCrys%c Resolution is currently %2.2f keV at %2.1f keV (< %2.1f keV)\033[m\n",clo,cristal[cri],mfit->GetParameter(2)*2.35,EnergyC,expectedReso->Eval(EnergyC));
				}
				else if(mfit->GetParameter(2)*2.35>1.5*expectedReso->Eval(EnergyC)){
					printf("\033[31m ECC%dCrys%c Resolution is currently %2.2f keV at %2.1f keV (expecting < %2.1f keV) \033[m\n",clo,cristal[cri],mfit->GetParameter(2)*2.35,EnergyC,expectedReso->Eval(EnergyC));
				}
			}
			
		}
	}

return true;
}

void TExogam2REA::InitBranch(TTree *tree)
{
   tree->Branch("Exogam2_REA", "TExogam2READata", &fExogam2READata,32000,99);
}

