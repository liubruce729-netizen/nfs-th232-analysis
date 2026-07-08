#include <cstdlib>
#include <sstream>

#include "TExogam2.h"

#include "fstream"

ClassImp(TExogam2)
//if uncorrelated because of TDC then return false to save CPU time
TExogam2::TExogam2(bool bspec)
{
   // Default constructor
   fExogam2Data    = new TExogam2Data();
   BoolSpec=bspec;
   Beta=0.;
   GammaGateNFS=distanceTOF=GammaFlashOffset=0;
   fullCoinc=0;
   for(Int_t i=0;i<16;i++)CloverPresent[i]=false;
   Goccetrack=GOCCEActive=ESSActive=false;
   LUTBool=false;
   neutronNFS=false;
   NfsSpec=false;
   NfsCrystalTimeCorrection=false;
   PrevTS=0;
   Theta_P=Phi_P=0;
   TARGET_POSITION_X=0. ; // mm
   TARGET_POSITION_Y=0. ; // mm
   TARGET_POSITION_Z=0. ; // mm
   MaxClovMul=18;
   ESSThreshold=1000.;
   MINIBALL[0]=MINIBALL[1]=0;
   	 base= new TF1("base","[0]/(1.+exp((x-[1])/[2]))",-1000,1000); //wood-saxon like traces
	 trace = new TF1("trace","[0]/(1.+exp((x-[1])/[2]))",-1000,1000); //wood-saxon like traces
         traceExp = new TGraph(6);
   MGTTraces=fopen("Utils/MGTTraces.dat","w");
   MGTCounter=NoNetCharge=0;
   closed=false;
   fNfsCrystalDeltaTEnergy=NULL;
   fNfsAllCrystalTime=NULL;
   fNfsCrystalCrossTalk=NULL;
   fNfsCrystalBgoEfficiency=NULL;
   fNfsCrystalCsiEfficiency=NULL;
   fNfsAllGammaGammaMatrixNoCut=NULL;
   for(Int_t i=0;i<16*4;i++){
      fNfsCrystalDeltaT[i]=NULL;
      fNfsCrystalEnergy[i]=NULL;
      NfsCrystalTimeCorrectionValid[i]=false;
      NfsCrystalTimeCorrectionOffset[i]=0.;
      NfsCrystalTimeCorrectionGain[i]=1.;
      NfsCrystalTimeCorrectionGain2[i]=0.;
      NfsCrystalGammaFlashPeak[i]=0.;
      NfsCrystalGammaFlashFwhm[i]=0.;
   }
   for(Int_t i=0;i<16;i++){
      CloverDistanceMm[i]=0.;
      fNfsCloverAddbackGamma[i]=NULL;
      fNfsCloverAddbackGammaVeto[i]=NULL;
   }
   for(Int_t i=0;i<16;i++){
   	for(Int_t j=0;j<4;j++){
		BrokenSeg[i][j]=1000;
  		BrokenSegBool[i][j]=false;
   	}
   }
	
}



TExogam2::~TExogam2()
{
   delete fExogam2Data;
}



bool TExogam2::Clear()
{
   fExogam2Data->Clear();
   
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
bool TExogam2::ActivateGOCCE(bool act){
	GOCCEActive=act;
	return true;
}
bool TExogam2::ActivateESS(bool act, float Thres){
	ESSActive=act;
	ESSThreshold=Thres;
	return true;
}
bool TExogam2::IsActivateGOCCE(){
	return GOCCEActive;
}
bool TExogam2::IsActivateESS(){
	return ESSActive;
}
bool TExogam2::ActivateClover(int i)
{ 
	CloverPresent[i]=true;
	return true;
}
bool TExogam2::SetPromptGate(int a, int b){
	promptL=a;
	promptH=b;
		cerr<<"TExogam2 Info :: Prompt gate is "<<promptL<<"   "<<promptH<<endl;

	return true;
}
bool TExogam2::IsCloverActive(int i)
{ 
	return CloverPresent[i];
}
bool TExogam2::ActivateGOCCETrack(bool check){

	Goccetrack=check;
	return true;
}
bool TExogam2::SetBeta(float beta){
	Beta=beta;
	printf("\033[36mExogam2 Info ::  Beta = %f \033[m \n",Beta);
	return true;
}

bool TExogam2::SetTargetPosition(float X, float Y, float Z){
	TARGET_POSITION_X =X ;
	TARGET_POSITION_Y =Y ;
	TARGET_POSITION_Z =Z ;
	printf("\033[36mExogam2 Info ::  Target position set at (%f, %f, %f) mm \033[m \n",X,Y,Z);
	printf("\033[36mExogam2 Info ::  Wait for it, now calculating all clovers positions \033[m \n");
	sleep(2);
	return true;
}
bool TExogam2::SetCloverBrokenSeg(int clo, int cri, int seg){
  	BrokenSeg[clo][cri]=seg;
  	BrokenSegBool[clo][cri]=true;
  	return true;
}
bool TExogam2::SetMaxClovMul(int set){
	MaxClovMul=set;
	printf("\033[35mExogam2 Info ::  Set a max Clover Mult at %d \033[m \n",MaxClovMul);
	return true;
}

bool TExogam2::IsDiagonal(int cri1, int cri2){
   	bool result;
	if(abs(cri1-cri2)==2){result = true;}
	else {result = false;}
	return result;

}
bool TExogam2::LoadBase(bool check){
float TMAX, R, a,b,c,T0;
TGraph *slope = new TGraph(11);
TGraph *slopeWS = new TGraph(23);
TF1 *test = new TF1("test","[0]*x+[1]",0,200);
test->SetParameter(0,0.2);
test->SetParameter(1,-10.);
	if(check){
		printf("\033[35mExogam2 Info ::  Loading PSA Base from base_exogam.out \033[m \n");
		FILE *file = fopen("base_exogam.out","r");
		for(Int_t x=0;x<23;x++){
			fscanf(file,"%f %f %f %f %f %f\n",&TMAX,&T0,&R,&a,&b,&c);
			//printf("--> %f %f %f %f %f\n",R,a,b,c);
			if(x<11){
				slope->SetPoint(x,TMAX,R);
			}
			slopeWS->SetPoint(x,-1.*c,R);
			PSABASE[x][0]=a;
			PSABASE[x][1]=b;
			PSABASE[x][2]=c;
			PSABASER[x]=R;
			PSABASET0[x]=T0;
		}
		slope->Fit("test","Q");
		MINIBALL[0]=test->GetParameter(0);
		MINIBALL[1]=test->GetParameter(1);
		slopeWS->Fit("test","Q");
		LIVERPOOL[0]=test->GetParameter(0);
		LIVERPOOL[1]=test->GetParameter(1);
		fclose(file);
		printf("\033[35mExogam Info ::  Loading PSA Base DONE \033[m \n");
	}
	
	return true;
}
bool TExogam2::SetPSA(bool check){
	if(check)printf("\033[35mExogam Info :  PSA Analysis is active \033[m \n");
	else printf("\033[35mExogam Info :  PSA Analysis is NOT active \033[m \n");
	PSAActive=check;
	LoadBase(PSAActive);
	return true;
}
bool TExogam2::SetNFS_Parameter(float D, float offset, float GammaG, bool nfs){
	distanceTOF=D;
	GammaGateNFS=GammaG;
	neutronNFS=nfs;
	GammaFlashOffset=offset;
	
	return true;
}

bool TExogam2::SetNfsCrystalTimeCorrection(bool enable, TString correctionPath){
	NfsCrystalTimeCorrection=enable;
	for(Int_t i=0;i<16*4;i++){
		NfsCrystalTimeCorrectionValid[i]=false;
		NfsCrystalTimeCorrectionOffset[i]=0.;
		NfsCrystalTimeCorrectionGain[i]=1.;
		NfsCrystalTimeCorrectionGain2[i]=0.;
		NfsCrystalGammaFlashPeak[i]=0.;
		NfsCrystalGammaFlashFwhm[i]=0.;
	}

	if(!enable){
		printf("\033[36mNFS Exogam Info ::  Crystal time correction is disabled; using default gain 0.024 ns/channel and global gamma-flash offset %.3f ns \033[m \n",GammaFlashOffset);
		return true;
	}

	std::ifstream input(correctionPath.Data());
	if(!input.is_open()){
		printf("\033[31mNFS Exogam Alarm ::  Cannot open NFS time calibration file %s; using default gain 0.024 ns/channel and global gamma-flash offset %.3f ns \033[m \n", correctionPath.Data(), GammaFlashOffset);
		NfsCrystalTimeCorrection=false;
		return false;
	}

	// EN: Read ecc.cal-style coefficients. The first 64 triples are energy; the next 64 triples are NFS time.
	//     In the NFS time block: offset is relative to the global gamma-flash offset, gain is a ratio to 0.024 ns/channel, gain2 is reserved.
	// CN: Read ecc.cal format: the first 64 triples are energy calibration; the next 64 triples are NFS time calibration.
	//     In the NFS time block, offset is a difference relative to the global gamma-flash offset; gain is the ratio to 0.024 ns/channel; gain2 is reserved.
	std::string line;
	Int_t coefficientIndex=0;
	Int_t loaded=0;
	while(std::getline(input,line)){
		std::string trimmed=line;
		std::size_t first=trimmed.find_first_not_of(" \t\r\n");
		if(first==std::string::npos)continue;
		trimmed=trimmed.substr(first);
		if(trimmed.rfind("#",0)==0 || trimmed.rfind("//",0)==0)continue;

		std::istringstream iss(trimmed);
		Double_t offset=0., gain=0., gain2=0.;
		if(!(iss >> offset >> gain >> gain2))continue;

		if(coefficientIndex>=16*4 && coefficientIndex<2*16*4){
			Int_t detector=coefficientIndex-16*4;
			NfsCrystalTimeCorrectionOffset[detector]=offset;
			NfsCrystalTimeCorrectionGain[detector]=gain;
			NfsCrystalTimeCorrectionGain2[detector]=gain2;
			if(offset!=0. || gain!=0.){
				NfsCrystalTimeCorrectionValid[detector]=true;
				loaded++;
			}
		}
		coefficientIndex++;
		if(coefficientIndex>=2*16*4)break;
	}
	input.close();

	if(coefficientIndex<2*16*4){
		printf("\033[31mNFS Exogam Alarm ::  Time calibration file %s has only %d coefficient lines; expected at least 128 lines. Using default gain 0.024 ns/channel and global gamma-flash offset %.3f ns \033[m \n", correctionPath.Data(), coefficientIndex, GammaFlashOffset);
		NfsCrystalTimeCorrection=false;
		return false;
	}

	if(loaded==0){
		printf("\033[31mNFS Exogam Alarm ::  No valid NFS time coefficients loaded from %s; using default gain 0.024 ns/channel and global gamma-flash offset %.3f ns \033[m \n", correctionPath.Data(), GammaFlashOffset);
		NfsCrystalTimeCorrection=false;
		return false;
	}

	printf("\033[36mNFS Exogam Info ::  Loaded %d NFS crystal time calibrations from %s. Global gamma-flash offset is added; the offset column is relative. \033[m \n",loaded,correctionPath.Data());
	return true;
}

bool TExogam2::SetNFSAnaSpec(bool spec){
	NfsSpec=spec;
	return true;
}

bool TExogam2::NfsSpectraConstructor(){
	if(!NfsSpec)return false;
	if(HListNfsExogam2.GetLast()>=0)return true;

	char name[128];
	char title[256];
	// EN: In NFS quick analysis, Time means the final neutron TOF-like time in ns.
	// CN: 在 NFS 快速分析中，Time 表示最终使用的中子飞行时间式时间，单位 ns。
	fNfsCrystalDeltaTEnergy = new TH2F("nfs_all_crystal_time_vs_energy","NFS all crystal fires;Time (ns);Energy (keV)",1600,0,1600,4000,0,20000);
	HListNfsExogam2.Add(fNfsCrystalDeltaTEnergy);

	fNfsAllCrystalTime = new TH1F("nfs_all_crystal_time","NFS all crystal Time;Time (ns);Counts",1600,0,1600);
	HListNfsExogam2.Add(fNfsAllCrystalTime);

	// EN: Cross talk is event-level: if two or more crystals have positive gamma energy, fill all off-diagonal pairs.
	// CN: 串扰图是 event 级定义：同一 event 中两个以上 crystal 有正 gamma 能量时，填所有非对角两两组合。
	fNfsCrystalCrossTalk = new TH2F("nfs_crystal_cross_talk","NFS crystal cross talk;Crystal (clover-crystal);Crystal (clover-crystal)",64,-0.5,63.5,64,-0.5,63.5);
	HListNfsExogam2.Add(fNfsCrystalCrossTalk);

	// EN: TProfile bins store mean(0/1), so the bin content is the detector fire efficiency.
	// CN: TProfile 每个 bin 保存 0/1 平均值，因此 bin content 就是 detector fire 效率。
	fNfsCrystalBgoEfficiency = new TProfile("nfs_crystal_bgo_efficiency","NFS crystal BGO efficiency;Crystal (clover-crystal);BGO fire efficiency",64,-0.5,63.5,0.0,1.0);
	HListNfsExogam2.Add(fNfsCrystalBgoEfficiency);

	fNfsCrystalCsiEfficiency = new TProfile("nfs_crystal_csi_efficiency","NFS crystal CSI efficiency;Crystal (clover-crystal);CSI fire efficiency",64,-0.5,63.5,0.0,1.0);
	HListNfsExogam2.Add(fNfsCrystalCsiEfficiency);

	// EN: No prompt, time, timestamp, or BGO/CSI veto cut; only positive raw crystal energies are required.
	// CN: 不加 prompt、time、timestamp、BGO/CSI veto cut；仅要求 raw crystal 能量为正。
	fNfsAllGammaGammaMatrixNoCut = new TH2F("nfs_all_gamma_gamma_matrix_no_cut","NFS all crystal gamma-gamma no cut;Gamma energy (keV);Gamma energy (keV)",4096,0,4096,4096,0,4096);
	HListNfsExogam2.Add(fNfsAllGammaGammaMatrixNoCut);

	for(Int_t id=0; id<16*4; id++){
		TString label = TString::Format("%d-%d", id/4, id%4);
		fNfsCrystalCrossTalk->GetXaxis()->SetBinLabel(id+1,label.Data());
		fNfsCrystalCrossTalk->GetYaxis()->SetBinLabel(id+1,label.Data());
		fNfsCrystalBgoEfficiency->GetXaxis()->SetBinLabel(id+1,label.Data());
		fNfsCrystalCsiEfficiency->GetXaxis()->SetBinLabel(id+1,label.Data());
	}

	for(Int_t clo=0; clo<16; clo++){
		if(CloverPresent[clo]==false)continue;
		for(Int_t cri=0; cri<4; cri++){
			Int_t id=clo*4+cri;
			sprintf(name,"nfs_clover%d_crystal%d_time",clo,cri);
			sprintf(title,"Clover%d Crystal%d Time;Time (ns);Counts",clo,cri);
			fNfsCrystalDeltaT[id]=new TH1F(name,title,1600,0,1600);
			HListNfsExogam2.Add(fNfsCrystalDeltaT[id]);

			sprintf(name,"nfs_clover%d_crystal%d_energy",clo,cri);
			sprintf(title,"Clover%d Crystal%d Gamma Energy;Energy (keV);Counts",clo,cri);
			fNfsCrystalEnergy[id]=new TH1F(name,title,4000,0,20000);
			HListNfsExogam2.Add(fNfsCrystalEnergy[id]);
		}

		sprintf(name,"nfs_clover%d_addback_gamma",clo);
		sprintf(title,"Clover%d Addback Gamma;Energy (keV);Counts",clo);
		fNfsCloverAddbackGamma[clo]=new TH1F(name,title,4096,0,4096);
		HListNfsExogam2.Add(fNfsCloverAddbackGamma[clo]);

		sprintf(name,"nfs_clover%d_addback_gamma_bgo_csi_veto",clo);
		sprintf(title,"Clover%d Addback Gamma BGO CSI veto;Energy (keV);Counts",clo);
		fNfsCloverAddbackGammaVeto[clo]=new TH1F(name,title,4096,0,4096);
		HListNfsExogam2.Add(fNfsCloverAddbackGammaVeto[clo]);
	}

	return true;
}

void TExogam2::FillNfsSpectra(Int_t mapFinger, Int_t clo, Int_t cri, Float_t timeNs, Float_t energy){
	if(!NfsSpec)return;
	if(mapFinger<0 || mapFinger>=16*4)return;
	// EN: Crystal energy spectra use the same loose condition as E877 addback: raw gamma energy > 0.
	// CN: Crystal 能量谱采用和 E877 addback 一致的宽松条件：raw gamma 能量 > 0。
	if(energy<=0)return;
	if(fNfsCrystalEnergy[mapFinger])fNfsCrystalEnergy[mapFinger]->Fill(energy);
	// EN: Time spectra still require a valid positive corrected Time.
	// CN: Time 谱仍然要求修正后的 Time 为有效正值。
	if(timeNs<=0)return;
	if(fNfsCrystalDeltaTEnergy)fNfsCrystalDeltaTEnergy->Fill(timeNs,energy);
	if(fNfsAllCrystalTime)fNfsAllCrystalTime->Fill(timeNs);
	if(fNfsCrystalDeltaT[mapFinger])fNfsCrystalDeltaT[mapFinger]->Fill(timeNs);
}

void TExogam2::FillNfsCrystalEventSpectra(Bool_t *crystalFired, Bool_t *bgoFired, Bool_t *csiFired){
	// EN: Called once per event after raw positive-energy crystal fires have been collected.
	// CN: 每个 event 调用一次；输入已经是正能量 raw crystal fire 的 event 级标记。
	if(!NfsSpec)return;
	Int_t firedIds[16*4];
	Int_t firedMult=0;
	for(Int_t id=0; id<16*4; id++){
		if(!crystalFired[id])continue;
		firedIds[firedMult++]=id;
		// EN: TProfile stores the mean of 0/1 values, i.e. the BGO/CSI fire efficiency.
		// CN: TProfile 保存 0/1 的平均值，也就是 BGO/CSI fire 效率。
		// EN: Denominator is this crystal fired; numerator is BGO/CSI > 0 in the same stored event entry.
		// CN: 分母是该 crystal fire；分子是同一存储条目中 BGO/CSI > 0。
		if(fNfsCrystalBgoEfficiency)fNfsCrystalBgoEfficiency->Fill(id,bgoFired[id] ? 1.0 : 0.0);
		if(fNfsCrystalCsiEfficiency)fNfsCrystalCsiEfficiency->Fill(id,csiFired[id] ? 1.0 : 0.0);
	}
	if(firedMult<2 || !fNfsCrystalCrossTalk)return;
	for(Int_t i=0; i<firedMult; i++){
		for(Int_t j=i+1; j<firedMult; j++){
			fNfsCrystalCrossTalk->Fill(firedIds[i],firedIds[j]);
			fNfsCrystalCrossTalk->Fill(firedIds[j],firedIds[i]);
		}
	}
}

void TExogam2::FillNfsCloverAddbackSpectra(Int_t clo, Float_t energy, Float_t bgo, Float_t csi){
	if(!NfsSpec)return;
	if(clo<0 || clo>=16)return;
	if(energy<=0)return;
	if(fNfsCloverAddbackGamma[clo])fNfsCloverAddbackGamma[clo]->Fill(energy);
	if(bgo<=0 && csi<=0 && fNfsCloverAddbackGammaVeto[clo])fNfsCloverAddbackGammaVeto[clo]->Fill(energy);
}
float TExogam2::Doppler_Correction(float Theta_Gamma, float Phi_Gamma, float Theta_Part, float Phi_Part, float Beta_Part, float energie_Mes){  //rad, v/c
	float energievraie,cosinusPSI;
			  
		  cosinusPSI =TMath::Sin(Theta_Part)*TMath::Cos(Phi_Part)*TMath::Sin(Theta_Gamma)*TMath::Cos(Phi_Gamma)+
		  	      TMath::Sin(Theta_Part)*TMath::Sin(Phi_Part)*TMath::Sin(Theta_Gamma)*TMath::Sin(Phi_Gamma)+
			      TMath::Cos(Theta_Part)*TMath::Cos(Theta_Gamma);
			      
	energievraie = energie_Mes*(1.-Beta_Part*cosinusPSI)/sqrt(1.-Beta_Part*Beta_Part);

	return energievraie;
};

bool TExogam2::SpectraConstructor()
{
if(BoolSpec){

	char title[50];
	char cristal[5]={'A','B','C','D','\0'};  
	
	
	printf("\033[35mExogam Info :  Start Spectra constructors\033[m \n");
	fMyHistoNCriERaw = new TH2I("NCristal_ERaw","NCristal_ERaw",16*10,0,16*10,8000,0,16000);
	//HListExogam2.Add(fMyHistoNCriERaw);

	fMyHistoNCriTRaw = new TH2I("NCristal_TRaw","NCristal_TRaw",16*4,0,16*4,8000,0,1e5);
	//HListExogam2.Add(fMyHistoNCriTRaw);
	
	
	fMyHistoNCriECal = new TH2I("NCristal_Ecal","NCristal_Ecal",16*4,0,16*4,1000,0,3000);
	//HListExogam2.Add(fMyHistoNCriECal);
	
	fMyHistoNCriTCal = new TH2I("NCristal_Tcal","NCristal_Tcal",16*4,0,16*4,4000,0,1e5);
	//HListExogam2.Add(fMyHistoNCriTCal);

        TimeStampDiff= new TH1F("TimeStampDiffTExogam2","TimeStampDiffTExogam2",11000,-1000,10000);
	//HListExogam2.Add(TimeStampDiff);
	
	
	BoardIdHist = new TH1F("BoardIdHist","BoardIdHist",255*2,0,255);
	//HListExogam2.Add(BoardIdHist);
	
	fMyHistoSegSeg = new TH2I("fMyHistoSegSegExogam","fMyHistoSegSegExogam",300,0,300,300,0,300);
	//HListExogam2.Add(fMyHistoSegSeg);
	
	
	for(int k2=0 ; k2<16*4 ; k2++){
		for(int k=0 ; k<16*4 ; k++){
			//cout<<k2%4<<" "<<k%4<<endl;
			if(CloverPresent[k2/4]==false || CloverPresent[k/4]==false){continue;}
			else{
				sprintf(title,"fMyHistoTT%d_%d",k2,k);	
				//fMyHistoTT[k2][k]=new TH1F(title,title,40,-200,200);
				//HListExogam2.Add(fMyHistoTT[k2][k]);
			}
		}
	}
		
	
	
	for(int k=0 ; k<16 ; k++){
		if(CloverPresent[k]==false) continue;
			//cout << "Clover  :: "<<k<<endl;
			sprintf(title,"Clover%d_ECal",k);// No process
			fMyHistoCloverECal[k]= new TH1F(title,title,6000,0,3000); 
			HListExogam2.Add(fMyHistoCloverECal[k]);
			
			sprintf(title,"Clover%d_TCal",k);
			fMyHistoCloverTCal[k]= new TH1F(title,title,4000,0,1e5); 
			//HListExogam2.Add(fMyHistoCloverTCal[k]);
			
			sprintf(title,"Clover%d_ECalAdd",k); //Prompt AddBack
			fMyHistoCloverECalAdd[k]= new TH1F(title,title,6000,0,3000); 
			HListExogam2.Add(fMyHistoCloverECalAdd[k]);
			
			sprintf(title,"Clover%d_ECalACAddDC_DC",k);//AddBack + AC  /clover + DopplerCorrected
			fMyHistoCloverECalACAdd_DC[k]= new TH1F(title,title,6000,0,3000); 
			HListExogam2.Add(fMyHistoCloverECalACAdd_DC[k]);
			
			sprintf(title,"Clover%d_ECalACAdd_TC",k);//AddBack + AC + prompt /clover
			fMyHistoCloverECalACAdd_TC[k]= new TH1F(title,title,6000,0,3000); 
			HListExogam2.Add(fMyHistoCloverECalACAdd_TC[k]);
			
			sprintf(title,"Clover%d_ECalACAddRejectF",k);
			fMyHistoCloverECalACAddRejectF[k]= new TH1F(title,title,600,0,3000); 
			//HListExogam2.Add(fMyHistoCloverECalACAddRejectF[k]);
		for(int l=1;l<=4;l++){
			//cout << "          Crystal   :: "<<l<<endl;
			sprintf(title,"ECC%d_%c_ECal",k,cristal[l-1]);
			fMyHistoECCECal[k*100+10*(l-1)]= new TH1F(title,title,40000,0,20000); 
			HListExogam2.Add(fMyHistoECCECal[k*100+10*(l-1)]);
			
			sprintf(title,"ECC%d_%c_TCal",k,cristal[l-1]);
			fMyHistoECCTCal[k*100+10*(l-1)]= new TH1F(title,title,1000,0,1000); //kEV TKE
			//HListExogam2.Add(fMyHistoECCTCal[k*100+10*(l-1)]);
			
			sprintf(title,"ECC%d_%c_TRaw",k,cristal[l-1]);
			fMyHistoECCTRaw[k*100+10*(l-1)]= new TH1F(title,title,400,0,1e5); 
			//HListExogam2.Add(fMyHistoECCTRaw[k*100+10*(l-1)]);
			
			
			
			sprintf(title,"ECC%d_%c_T30",k,cristal[l-1]);
			fMyHistoECCET30[k*100+10*(l-1)]=new TH1F(title,title,100,0,100); 
			//HListExogam2.Add(fMyHistoECCET30[k*100+10*(l-1)]);
			sprintf(title,"ECC%d_%c_T60",k,cristal[l-1]);
			fMyHistoECCET60[k*100+10*(l-1)]=new TH1F(title,title,100,0,100); 
			//HListExogam2.Add(fMyHistoECCET60[k*100+10*(l-1)]);
			sprintf(title,"ECC%d_%c_T90",k,cristal[l-1]);
			fMyHistoECCET90[k*100+10*(l-1)]=new TH1F(title,title,100,0,100); 
			//HListExogam2.Add(fMyHistoECCET90[k*100+10*(l-1)]);
			
			sprintf(title,"ECC%d_%c_T30_60",k,cristal[l-1]);
			fMyHistoECCET30_60[k*100+10*(l-1)]=new TH2F(title,title,100,0,100,100,0,100); 
			//HListExogam2.Add(fMyHistoECCET30_60[k*100+10*(l-1)]);
			sprintf(title,"ECC%d_%c_T30_90",k,cristal[l-1]);
			fMyHistoECCET30_90[k*100+10*(l-1)]=new TH2F(title,title,100,0,100,100,0,100); 
			//HListExogam2.Add(fMyHistoECCET30_90[k*100+10*(l-1)]);
			sprintf(title,"ECC%d_%c_T60_90",k,cristal[l-1]);
			fMyHistoECCET60_90[k*100+10*(l-1)]=new TH2F(title,title,100,0,100,100,0,100); 
			//HListExogam2.Add(fMyHistoECCET60_90[k*100+10*(l-1)]);
			
			sprintf(title,"ESS%d_BGO%d",k,l);
			fMyHistoESS_BGO[k*100+10*(l-1)]=new TH1F(title,title,4000,0,16000); 
			//HListExogam2.Add(fMyHistoESS_BGO[k*100+10*(l-1)]);
			
			sprintf(title,"ESS%d_CSI%d",k,l);
			fMyHistoESS_CSI[k*100+10*(l-1)]=new TH1F(title,title,4000,0,16000); 
			//HListExogam2.Add(fMyHistoESS_CSI[k*100+10*(l-1)]);
			
			
			sprintf(title,"ESS%d_%c_CoreEAcE",k,cristal[l-1]);
			fMyHistoECCEESSE[k*100+10*(l-1)]=new TH2F(title,title,500,0,2000,500,0,1000);
			//HListExogam2.Add(fMyHistoECCEESSE[k*100+10*(l-1)]);
			
			sprintf(title,"ECC%d_%c_Vetoed",k,cristal[l-1]);
			fMyHistoECCEVetoed[k*100+10*(l-1)]= new TH1F(title,title,400,0,2000); 
			//HListExogam2.Add(fMyHistoECCEVetoed[k*100+10*(l-1)]);
			
			sprintf(title,"ECC%d_%c_Accepted",k,cristal[l-1]);
			fMyHistoECCEAccepted[k*100+10*(l-1)]= new TH1F(title,title,400,0,2000); 
			//HListExogam2.Add(fMyHistoECCEAccepted[k*100+10*(l-1)]);
			
			sprintf(title,"ECC%d_%c_ECalACRejectF",k,cristal[l-1]);
			fMyHistoCrysECalACRejectF[k*100+10*(l-1)]= new TH1F(title,title,400,0,2000); 
			//HListExogam2.Add(fMyHistoCrysECalACRejectF[k*100+10*(l-1)]);
			
			for(int m=1; m<=4; m++){
				sprintf(title,"GOCCE%d_%c_%d_E",k,cristal[l-1],m);
				fMyHistoGOCCEE[k*100+(l-1)*10+(m-1)]= new TH1F(title,title,10000,0,10000);
				HListExogam2.Add(fMyHistoGOCCEE[k*100+(l-1)*10+(m-1)]);
				
				sprintf(title,"GOCCE%d_%c_%d_T",k,cristal[l-1],m);
				fMyHistoGOCCET[k*100+(l-1)*10+(m-1)]= new TH1F(title,title,400,0,1e5);
				//HListExogam2.Add(fMyHistoGOCCET[k*100+(l-1)*10+(m-1)]);
			}

		}
	}
		
	fMyHistoSumECCE= new TH1F("fMyHistoSumECCE","fMyHistoSumECCE",6000,0,6000); 
	HListExogam2.Add(fMyHistoSumECCE);
	fMyHistoSumECCT= new TH1F("fMyHistoSumECCT","fMyHistoSumECCT",640,1,64000); 
	HListExogam2.Add(fMyHistoSumECCT);
	fMyHistoPatternECCT= new TH1F("PatternTimeCoreExogam2","PatternTimeCoreExogam2",2000,0,200); 
	HListExogam2.Add(fMyHistoPatternECCT);
	fMyHistoPatternECCE= new TH1F("PatternEnergyCoreExogam2","PatternEnergyCoreExogam2",200,0,200); 
	HListExogam2.Add(fMyHistoPatternECCE);
	fMyHistoTTAll = new TH1F("fMyHistoTTAll","fMyHistoTTAll EXOGAM [ns]",2000,-1000,1000);
	//HListExogam2.Add(fMyHistoTTAll);
	
	ECCTcorrelCheck = new TH2F("ECCTcorrelCheck","ECCTcorrelCheck",1000,00,100,400,0,1e5);
	
	fMyHistoSumECCE_ECCT = new TH2F("fMyHistoSumECCE_ECCT","fMyHistoSumECCE_ECCT",6000,0,6000,500,0000,100e3);
	HListExogam2.Add(fMyHistoSumECCE_ECCT);
	fMyHistoSumGOCCEE= new TH1F("fMyHistoSumGOCCEE","fMyHistoSumGOCCEE",70000,0,70000); 
	HListExogam2.Add(fMyHistoSumGOCCEE);
	fMyHistoSumGOCCET= new TH1F("fMyHistoSumGOCCET","fMyHistoSumGOCCET",4000,0,2000); 
	HListExogam2.Add(fMyHistoSumGOCCET);
	fMyHistoPatternGOCCEE= new TH1F("PatternEnergySegExogam2","PatternEnergySegExogam2",1700,0,1700); 
	HListExogam2.Add(fMyHistoPatternGOCCEE);
	fMyHistoPatternGOCCET= new TH1F("PatternTimeSegExogam2","PatternTimeSegExogam2",1700,0,1700); 
	HListExogam2.Add(fMyHistoPatternGOCCET);
	fMyHistoNSegERaw=new TH2I("PatternEnergySegExogam22D","PatternEnergySegExogam22D",1700,0,1700,3000,0,10000); 
	HListExogam2.Add(fMyHistoNSegERaw);
	
	fMyHistoPatternESSTQ = new TH2F("PatternEssQ","PatternEssQ",170,0,170,4000,0,16000); 
	HListExogam2.Add(fMyHistoPatternESSTQ);
	
		
	
	fMyHistoAngletheta= new TH1F("Theta_Distribution","Theta_Distribution",200,0,200);
	HListExogam2.Add(fMyHistoAngletheta);
        fMyHistoAnglephi= new TH1F("Phi_Distribution","Phi_Distribution",400,-200,200);
	HListExogam2.Add(fMyHistoAnglephi);
	fMyHistoMultiCrystal= new TH1F("TotalCrystalMultiplicity","TotalCrystalMultiplicity",16*4,0,16*4);
	HListExogam2.Add(fMyHistoMultiCrystal);
	fMyHistoMultiCrystalperClover= new TH1F("CrystalMultiplicityPerClover","CrystalMultiplicityPerClover",6,0,6);
	HListExogam2.Add(fMyHistoMultiCrystalperClover);
	fMyHistoMultiClover= new TH1F("TotalCloverMultiplicity","TotalCloverMultiplicity",20,0,20);
	HListExogam2.Add(fMyHistoMultiClover);
	fMyHistoMultiAntiCompt= new TH1F("TotalAntiComptonMultiplicity","TotalAntiComptonMultiplicity",20,0,20);
	HListExogam2.Add(fMyHistoMultiAntiCompt);
	
	for(int l=0;l<10;l++){
		sprintf(title,"Exogam2AddB_AC_DC_Nuc%d",l);
		fMyHistoSumExogam2DCNucId[l]= new TH1F(title,title,1000,0,6000);
		HListExogam2.Add(fMyHistoSumExogam2DCNucId[l]);
	}
	
	fMyHistoSumExogam2= new TH1F("Exogam2AddB_AC_noDC","Exogam2AddB_AC_noDC",10000,0,10000);//AddBack + AC rejected + Prompt
	HListExogam2.Add(fMyHistoSumExogam2);
	
	fMyHistoSumExogam2DC_VTS=new TH2F("Exogam2AddB_AC_DC_VTS","Exogam2AddB_AC_DC_VTS",500,0,10000,200,-100,100);
	HListExogam2.Add(fMyHistoSumExogam2DC_VTS);
	
	fMyHistoSumExogam2DC= new TH1F("Exogam2AddB_AC_DC","Exogam2AddB_AC_DC",10000,0,10000);
	HListExogam2.Add(fMyHistoSumExogam2DC);
	fMyHistoSumExogam22D= new TH2F("Exogam2AddB_AC_noDC2D","Exogam2AddB_AC_noDC2D",10,0,200,500,0,6000);
	HListExogam2.Add(fMyHistoSumExogam22D);
        fMyHistoSumExogam2DC2D= new TH2F("Exogam2AddB_AC_DC2D","Exogam2AddB_AC_DC2D",10,0,200,500,0,6000);
	HListExogam2.Add(fMyHistoSumExogam2DC2D);
	
	
	
	fMyHistoSumExogam2FoldCond= new TH1F("Exogam2AddB_AC_noDC_Fold1","Exogam2AddB_AC_noDC_Fold1",4000,0,8000);//AddBack + AC rejected + Prompt Fold1
	HListExogam2.Add(fMyHistoSumExogam2FoldCond);
	fMyHistoSumExogam2DCFoldCond= new TH1F("Exogam2AddB_AC_DC_Fold1","Exogam2AddB_AC_DC_Fold1",4000,0,8000);//AddBack + AC rejected + Prompt Fold 1 DC
	HListExogam2.Add(fMyHistoSumExogam2DCFoldCond);
	
	TransverseMomentumGamma = new TH2F("TransverseMomentumGamma","TransverseMomentumGamma",100,0,100,500,0,6000);
	HListExogam2.Add(TransverseMomentumGamma);
	
	
	fMyHistoGammaGamma= new TH2F("GammaGamma_AC_AD_DC","GammaGamma_AC_AD_DC",3000,0,3000,3000,0,3000);
	HListExogam2.Add(fMyHistoGammaGamma);
	fMyHistoTTinGammaGamma = new TH1F("fMyHistoTTinGammaGammaCloClo","fMyHistoTTinGammaGammaCloClo",201,-100,100); //Time Stamp Diff between 2 clovers after AddBack for fMyHistoGammaGamma construction
	HListExogam2.Add(fMyHistoTTinGammaGamma);
	
	fMyHistoGammaGammaCore= new TH2F("fMyHistoGammaGammaCore","fMyHistoGammaGammaCore",2000,0,2000,2000,0,2000);
	HListExogam2.Add(fMyHistoGammaGammaCore);
	fMyHistoGammaGammaCoreDiag= new TH2F("fMyHistoGammaGammaCoreDiag","fMyHistoGammaGammaCoreDiag",2000,0,2000,2000,0,2000);
	HListExogam2.Add(fMyHistoGammaGammaCoreDiag);
	
	
	fMyHistoSumExogam2BeforeAfterDC= new TH2F("Exogam2AddB_AC_BefAfterDC","Exogam2AddB_AC_BefAfterDC",3000,0,3000,3000,0,3000);
	HListExogam2.Add(fMyHistoSumExogam2BeforeAfterDC);
	
	fMyHistoSumExogam2Calorimeter=new TH1F("Exogam2CalorimeterAC","Exogam2CalorimeterAC",4000,0,20000);
	HListExogam2.Add(fMyHistoSumExogam2Calorimeter);
	fMyHistoCheckEss=new TH1F("CheckEss","CheckEss",100,0,100);


   	fMyHistoSumExogam2DCClover= new TH2F("fMyHistoSumExogam2DCClover","fMyHistoSumExogam2DCClover",16,0,16,500,0,6000);
       	fMyHistoSumExogam2Clover= new TH2F("fMyHistoSumExogam2Clover","fMyHistoSumExogam2Clover",16,0,16,500,0,6000);
	HListExogam2.Add(fMyHistoSumExogam2Clover);
	HListExogam2.Add(fMyHistoSumExogam2DCClover);

//--------------------------------------------PSA----------------------------------------------
	fMyHistoGOCCENet= new TH1F("fMyHistoGOCCENet","fMyHistoGOCCENet",70000,0,70000);  
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
	
	
	
	HListExogam2.Add(fMyHistoGOCCENet);
	HListExogam2.Add(fMyHistoGOCCEMirror);
	HListExogam2.Add(fMyHistoPhiMirror);
	HListExogam2.Add(fMyHistoRMirror);
	HListExogam2.Add(fMyHistoPSASurfaceCarte);
	HListExogam2.Add(fMyHistoPSAChi2);
	HListExogam2.Add(fMyHistoPSACore);
	HListExogam2.Add(fMyHistoPSARejectedTraces);
	HListExogam2.Add(fMyHistoPSAAcceptedTraces);
	HListExogam2.Add(fMyHistoPSA_CFD);
	HListExogam2.Add(fMyHistoPSA_CFD_E);
	HListExogam2.Add(fMyHistoPSA_CFD_Pattern);
	HListExogam2.Add(fMyHistoPSARadius_Radius);
	HListExogam2.Add(fMyHistoPhiMirrorPattern);
	HListExogam2.Add(fMyHistoRPattern);
	HListExogam2.Add(fMyHistoPSAWorld);
	HListExogam2.Add(fMyHistoPSAChi2_Pattern);
	HListExogam2.Add(fMyHistoPSAK);
	HListExogam2.Add(fMyHistoPSAK_Pattern);
	HListExogam2.Add(fMyHistoPSAL_Pattern);
	//--------------------------------------------PSA----------------------------------------------
	
	//--------------------------------------------NFS----------------------------------------------

	fMyHistoNFS_ToF= new TH1F("fMyHistoNFS_ToF","fMyHistoNFS_ToF [ns]",4000,-2000,2000); //ns
        HListExogam2.Add(fMyHistoNFS_ToF);
        fMyHistoNFS_En= new TH1F("fMyHistoNFS_En","fMyHistoNFS_En [MeV]",500,1,51); //MeV
        HListExogam2.Add(fMyHistoNFS_En);
	fMyHistoNFS_EnEstar= new TH2F("fMyHistoNFS_EnEstar","fMyHistoNFS_EnEstar",500,1,51,1000,0,10000); 
	fMyHistoNFS_EnGmul= new TH2F("fMyHistoNFS_EnGmul","fMyHistoNFS_EnGmul",500,1,51,20,0,20); 
	
	
	
	HListExogam2.Add(fMyHistoNFS_EnEstar);
	HListExogam2.Add(fMyHistoNFS_EnGmul);





	printf("\033[35m ----> Done \033[m \n");	     
	return true;
}
else {return false;}

}
bool TExogam2::InitCal()
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

bool TExogam2::DefaultCal()
{
	for(Int_t x=0;x<16*4;x++){
    		ECoef[x][0]=0.;
		ECoef[x][1]=0.18;
        	ECoef[x][2]=0.;
    	}
	for(Int_t x=0;x<16*16;x++){
    		ECoef_G[x][0]=0.;
		ECoef_G[x][1]=0.634;
        	ECoef_G[x][2]=0.;
    	}
	printf("\033[31mInfo Exogam2:: Using default nrj cal param Core (0.225) Seg (0.634)  \033[m \n");
	CalibDone=false;
return true;
}	


bool TExogam2::ReadCal()
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


bool TExogam2::TimeStampDiffQ(long long TS1, long long TS2 , int gate){

bool answer;

	if(TS1>=TS2&&(TS1-TS2)<gate) answer = true;
	else if(TS2>=TS1&&(TS2-TS1)<gate) answer = true;
	else answer = false;
	
return answer;

}

double TExogam2::Cal(UShort_t en, float offset, float gain, float gain2){
double enc;
	    enc = (double)en+gRandom->Uniform(1.0)-.5;
	    //enc = (double)enc;
	    enc = enc*enc*gain2+enc*gain+offset;
	    return enc;
}
bool TExogam2::InitNumexo2(Char_t *fileNumexo2){
TString schainetrack;
TObjArray* toks=0;
Int_t board,halfboard,clo,cri;
char stop[2];

	for(Int_t i= 0 ; i<255;i++){Current_Numexo2_cfg_clo[i][0]=Current_Numexo2_cfg_clo[i][1]=Current_Numexo2_cfg_cri[i][0]=Current_Numexo2_cfg_cri[i][1]=-1;}
	
	ifstream inf_f(fileNumexo2);
	if(inf_f.good()==false) {
		printf("\033[31m Error Exogam2:: Not a valid Look Up Table=> I complain here   \033[m \n"); 
		
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
				//cout<<"Exogam2:: board "<<board<<" halfboard "<<halfboard<<" is (clo, "<<clo<<", cri "<<cri<<")"<<endl;
				delete toks;
			}
		}
	
	}
	inf_f.close();
	printf("=>Look Up Table Numexo2 EXOGAM read \n");
	
return true;
}
bool TExogam2::Init(DataParameters *params)
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
	 if (Xtalnum < 0) cout << "TExogam2::Init() : problem with ECC/Xtalnum" << endl;

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
	   cout << "TExogam2::Init() : problem reading Exogam2/ECC label" << sigtype << endl;
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
	 if (Xtalnum < 0) cout << "TExogam2::Init() : problem with GOCCE/Xtalnum" << endl;

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
	    cout << "TExogam2::Init() : problem reading Exogam2/GOCCE label" << endl;
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

bool TExogam2::Is(UShort_t lbl, Short_t val)
{
	bool result = false;
	return result;
}
bool TExogam2::IsMFMExo(MFMExogamFrame *frame)
{
   
   
   Int_t clo, cri;
   float Thresh;
   double valf,valf3;
   float valf2;
   Float_t deltaTNs, neutronTOF;
   UShort_t rawDeltaT;
   bool result = false;
   bool debug = false;
   float CaloSegForBrokenSeg, missingEnergy;
   //---------------PSA control
   bool PSA_debug = false;
   int  EXO2BoardId, EXO2CrysId, MapFinger,MulNetCharge ;
   int statusSegment, binstatus;
   float s[4]={0.,0.,0.,0.};
   int   psa[4]={0,0,0,0};
   int PSANetcharge;
   MulNetCharge=0;
   int q1,q2;
   float PSA_phi;
   float T30,T60,T90,PSA_r;
   TVector2 PSAHit(0,0);
   TVector3 PSAHit3D(0,0,0);
   float psa_T[6] = {0,0,0,0,0,0};
   float psa_Q[6] = {0,0,0,0,0,0};
   /*float psa_TD1[5] = {0,0,0,0,0};
   float psa_QD1[5] = {0,0,0,0,0};
   float psa_TD2[4] = {0,0,0,0};
   float psa_QD2[4] = {0,0,0,0};*/
   float QMAX, TMAX,chi2,Xp,Yp,Xpp,Ypp, Rp,ThetaP,psachi2, bestchi2, Zp;
   float RforBestChi2, WS1, WS2;
   float PSACutOFF=20.;
   float CFD_PSA_Value;
   bool CFDOK;
   bool validNfsTime;
   //---------------PSA control
   
   //cout<<frame->GetTimeStamp()<<endl;
   clo=cri=MapFinger=-100;
   T30=T60=T90=PSA_r=PSA_phi=0;
   QMAX= TMAX = Rp=0;
   neutronNRJ=0.;
   deltaTNs=neutronTOF=0.;
   rawDeltaT=0;
   validNfsTime=false;
   
   if(LUTBool==false){
   	printf("\033[31m Error in TExogam2::IsMFMExo => The Look Up Table is not known: I won't be able to unpack the event   \033[m \n"); 
	}
   else{
   	if(debug)cerr<<"-------------------------"<<endl;
   	if(debug)cerr<<"ExoGetTGCristalId() "<<frame->ExoGetTGCristalId()<<endl;
	if(debug)cerr<<"ExoGetBoardId()     "<<frame->ExoGetBoardId() <<endl;
	if(debug)cerr<<"ExoGetInnerM(0)     "<<frame->ExoGetInnerM(0)<<endl;
	if(debug)cerr<<"ExoGetDeltaT()      "<<frame->ExoGetDeltaT() <<endl;
	if(debug)cerr<<"ExoGetBGO()  	    "<<frame->ExoGetBGO() <<endl;
	if(debug)cerr<<"ExoGetCsi()         "<<frame->ExoGetCsi() <<endl;
	if(debug)cerr<<"ExoGetStatus()      "<<(((UShort_t)frame->ExoGetStatus(2))) <<endl;
	if(debug)cerr<<"Outers NRJ : 1 - "<<frame->ExoGetOuter(0)<<"  2- "<<frame->ExoGetOuter(1)<<"  3- "<<frame->ExoGetOuter(2)<<"  4- "<<frame->ExoGetOuter(3)<<endl;
	
   	EXO2CrysId=frame->ExoGetTGCristalId(); //return 0 = upper part of NUMEXO = 1st crystal, return 8 = bottom part = 2nd crystal
   	EXO2BoardId=frame->ExoGetBoardId(); //return NUMEXO Board ID
   	if (EXO2CrysId !=0)  EXO2CrysId=1; //if EXO2CrysId =8, ie second half of the board, then value set to 1
		
	
	
   	clo=Current_Numexo2_cfg_clo[EXO2BoardId][EXO2CrysId];
   	cri=Current_Numexo2_cfg_cri[EXO2BoardId][EXO2CrysId];
   	MapFinger=clo*4+cri;
   	if(clo>=0){
		result=true;
		}
   	else {
		printf("\033[31m Error in TExogam2::IsMFMExo => This IP (%d,%d) is not known from the Look Up Table   \033[m \n",EXO2BoardId,EXO2CrysId); 
   		result=false;
   	}
   }
   
   //if uncorrelated because of TDC then return false to save CPU time
  // if(frame->ExoGetDeltaT()>60000)result=false; //if uncorrelated because of TDC then return false to save CPU time
   
   if(IsCloverActive(clo)&&result&&frame->ExoGetInnerM(0)>10){ 

   	valf=Cal(frame->ExoGetInnerM(0),ECoef[MapFinger][0],ECoef[MapFinger][1],ECoef[MapFinger][2]); 
        
	rawDeltaT=frame->ExoGetDeltaT();
	valf2=Cal(rawDeltaT,TCoef[MapFinger][0],TCoef[MapFinger][1],TCoef[MapFinger][2]);
	Double_t reversedDeltaT=65536.0-static_cast<Double_t>(rawDeltaT);
	Bool_t useNfsTimeCal=NfsCrystalTimeCorrection && MapFinger>=0 && MapFinger<16*4 && NfsCrystalTimeCorrectionValid[MapFinger];
	if(useNfsTimeCal){
		// EN: NFS ecc.cal time coefficients are relative corrections:
		//     offset is added to the global gamma-flash offset, gain scales the default 0.024 ns/channel, gain2 is reserved.
		// CN: NFS ecc.cal time coefficients are relative corrections:
		//     offset is added to the global gamma-flash offset; gain scales the default 0.024 ns/channel; gain2 is reserved.
		deltaTNs=reversedDeltaT*0.024*NfsCrystalTimeCorrectionGain[MapFinger]
		        + GammaFlashOffset
		        + NfsCrystalTimeCorrectionOffset[MapFinger];
	}
	else{
		deltaTNs=reversedDeltaT*0.024;
	}
	
	//For NFS Only - Time becomes neutron energy - EXOGAM at 5.00 Meters from convertor - time Cal is in ns - neutron mass is 939.5654 MeV/c^2
	if(neutronNFS){
		if(useNfsTimeCal){
			neutronTOF=deltaTNs; // EN/CN: relative ecc time correction already includes the global gamma-flash offset.
		}
		else{
			neutronTOF=deltaTNs+GammaFlashOffset; // global correction: reversed DeltaT plus gamma-flash offset, in ns
		}
	
		if(BoolSpec)fMyHistoNFS_ToF->Fill(neutronTOF);
		
		if(neutronTOF>0)neutronNRJ= 0.5*(939.5654)*pow(distanceTOF/(neutronTOF*0.299),2.); //in MeV
		else neutronNRJ=0;
		
		if(BoolSpec)fMyHistoNFS_En->Fill(neutronNRJ);
		
		if(rawDeltaT<=60000&&neutronTOF>0){
			validNfsTime=true;
			fExogam2Data->SetDeltaT(deltaTNs);
			fExogam2Data->SetNeutronTOF(neutronTOF);
		}
		fExogam2Data->SetNeutronNRJ(neutronNRJ);
	}

	if(neutronNFS)fExogam2Data->SetTime(validNfsTime ? neutronTOF : 0.);
	else fExogam2Data->SetTime(0.);
	
       //incremente root Tree setter
       fExogam2Data->SetECCEClover(clo);
       fExogam2Data->SetECCECristal(cri);
       fExogam2Data->SetECCEDetNbr(MapFinger);
       fExogam2Data->SetECCEEnergy(valf);
       fExogam2Data->SetECCTClover(clo);
       fExogam2Data->SetECCTCristal(cri);
       fExogam2Data->SetECCTTime(valf2); // Delta is the TAC value between the CFD and the external ref
      // fExogam2Data->SetECCTTSRequest(frame->ExoGetTGCristalId());// is the TS request of the core to the GTS
       fExogam2Data->SetECCET30(frame->ExoGetInnerT(0));
       fExogam2Data->SetECCET60(frame->ExoGetInnerT(1));
       fExogam2Data->SetECCET90(frame->ExoGetInnerT(2));
       fExogam2Data->SetESSTQClover(clo);
       fExogam2Data->SetESSTQCristal(cri);
       fExogam2Data->SetESSTQBGO(frame->ExoGetBGO());
       fExogam2Data->SetESSTQCSI(frame->ExoGetCsi());
       fExogam2Data->SetfTimeStamps(frame->GetTimeStamp(),MapFinger);
       
       T30=frame->ExoGetInnerT(0); //in ticks
       T60=frame->ExoGetInnerT(1); //
       T90=frame->ExoGetInnerT(2);
       
       
   	if(BoolSpec&&IsCloverActive(clo)){ //Core Specta filling
		fMyHistoNCriERaw->Fill(clo*10+cri,frame->ExoGetInnerM(0)); //6MeV Core energy
		fMyHistoPatternECCE->Fill(clo*10+cri);
		fMyHistoECCECal[clo*100+10*cri]->Fill(valf);
		BoardIdHist->Fill(EXO2BoardId);
		
		fMyHistoSumECCE->Fill(valf);
		fMyHistoSumECCE_ECCT->Fill(valf,valf2);
		
		fMyHistoCloverECal[clo]->Fill(valf);
		fMyHistoNCriECal->Fill(MapFinger,valf);
		fMyHistoNCriTRaw->Fill(MapFinger,frame->ExoGetDeltaT());
		if(valf2>0)fMyHistoPatternECCT->Fill(clo*10+cri);
		fMyHistoECCTRaw[clo*100+10*cri]->Fill(frame->ExoGetDeltaT());
		
		fMyHistoECCTCal[clo*100+10*cri]->Fill(valf2);
		
		fMyHistoSumECCT->Fill(valf2);
		fMyHistoCloverTCal[clo]->Fill(valf2);
		fMyHistoNCriTCal->Fill(MapFinger,valf2);
		fMyHistoECCET30[clo*100+10*cri]->Fill(frame->ExoGetInnerT(0));
		fMyHistoECCET60[clo*100+10*cri]->Fill(frame->ExoGetInnerT(1));
		fMyHistoECCET90[clo*100+10*cri]->Fill(frame->ExoGetInnerT(2));
		fMyHistoECCET30_60[clo*100+10*cri]->Fill(frame->ExoGetInnerT(0),frame->ExoGetInnerT(1)); 
		fMyHistoECCET30_90[clo*100+10*cri]->Fill(frame->ExoGetInnerT(0),frame->ExoGetInnerT(2));
		fMyHistoECCET60_90[clo*100+10*cri]->Fill(frame->ExoGetInnerT(1),frame->ExoGetInnerT(2));
	}
	
	//search for standard Doppler correction
	if(valf>MaxCrisEval[clo]){
       		MaxCrisEval[clo]=valf;
        	MaxCris[clo]=cri;
       }
        
       
	//--------Now Segment
	
	//prepare the mirror energy status
       statusSegment = frame->ExoGetStatus(2);
       
       
       for(Int_t seg=0;seg<4;seg++){ //first loop to unpack what is in the data
       		fExogam2Data->SetGOCCEEClover(clo);
		fExogam2Data->SetGOCCEECristal(cri);
		fExogam2Data->SetGOCCEESegment(seg);
		
		//Treating the Status first
		//15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0  --> Structure of STATUS3
		//0  0  X  X  X  X  0 0 0 0 X X X X 0 0
		//          B part            A Part
		binstatus= (((statusSegment>>(8*EXO2CrysId))>>seg+2) &1);
		// binstatus ==1 --> Mirror
		// binstatus ==0 --> Net Charge
		fExogam2Data->SetGOCCEEStatus(binstatus); //Define if energy is Net or Mirror
		psa[seg]=binstatus;
		
		if(BoolSpec)fMyHistoPatternGOCCEE->Fill(clo*100+10*cri+seg);
		mulG[clo][cri]=mulG[clo][cri]+1;
		GOCCE_Pat[clo][cri][seg]=true;

		if(binstatus ==0){// binstatus ==0 --> Net Charge
			s[seg]=valf3;
			MulNetCharge++;
			valf3=Cal(frame->ExoGetOuter(seg),ECoef_G[seg+4*(MapFinger)][0],ECoef_G[seg+4*(MapFinger)][1],ECoef_G[seg+4*(MapFinger)][2]);
			fExogam2Data->SetGOCCEEEnergy(valf3); //is net charge
			if(CalibDone){Thresh=30;}
       			else{Thresh=50;}
			
			if(valf3>Thresh){
       				mulGnrj[clo][cri]=mulGnrj[clo][cri]+1;
        			GOCCE_Ener[clo][cri][seg]=true;
       			}
			//fill histograms
			if(BoolSpec){
				fMyHistoGOCCENet->Fill(valf3) ;
				fMyHistoNSegERaw->Fill(clo*100+10*cri+seg,valf3);
				fMyHistoGOCCEE[clo*100+10*cri+seg]->Fill(valf3);
				fMyHistoSumGOCCEE->Fill(valf3);
			}
			
		}
		else if(binstatus ==1){// binstatus ==1 --> Mirror //calibration means nothing so get the raw value there; we collect the Charge integration and not MWD
			if(frame->ExoGetOuter(seg)<32768)s[seg]=frame->ExoGetOuter(seg);
			else s[seg]=frame->ExoGetOuter(seg)-65536;
			fExogam2Data->SetGOCCEEEnergy(s[seg]);//is mirror charge
			if(BoolSpec)fMyHistoGOCCEMirror->Fill(s[seg]) ;
		} 
       }//end first Loop Segment
       
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
        //After all process ->search for standard Doppler at segment level
       for(Int_t seg=0;seg<4;seg++){
		if(psa[seg]==0&&s[seg]>MaxSegEval[clo][cri]){ //only on Net Charge
       			MaxSegEval[clo][cri]=s[seg];
        		MaxSeg[clo][cri]=seg;
       		}
       }		
 	//Do a check
	if(MaxSeg[clo][cri]==1000&&debug){
		printf("\033[31m TExogam2 Alarm:: No segment found for DC \033[m \n");
		if(BrokenSegBool[clo][cri]){
			cout<<"With Broken Segment "<< BrokenSeg[clo][cri]<<endl;
		}
		else{
			cout<<"Without Broken Segment "<< endl;
		}
 		for(Int_t seg=0;seg<4;seg++){
			cout<<"psa[seg] "<<psa[seg]<<" s[seg] "<<s[seg]<<endl;
		}
	}
          if(PSAActive){
	  	if(MulNetCharge==0){
       			if(PSA_debug)printf("\033[31m TExogam2 Alarm:: No Net charge segment for the core %d signal at %f keV  \033[m \n",MapFinger,valf);
       			NoNetCharge++;
       		}
       
		
		
       		if(MulNetCharge==1&&valf>30&&BrokenSegBool[clo][cri]==false){ //PSA Studies
       			if(PSA_debug)cout<<""<<endl;
			if(PSA_debug)cout<<"======================================="<<endl;
      			if(PSA_debug)cout<<"PSA Analysis || Single interaction only"<<endl;
       			if(PSA_debug)printf("(%d)%f --- (%d)%f \n",psa[0],s[0],psa[1],s[1]);
                	if(PSA_debug)printf("---     %f     ---\n",valf);
       			if(PSA_debug)printf("(%d)%f --- (%d)%f \n",psa[3],s[3],psa[2],s[2]);
       			if(PSA_debug)cout<<"----------------------"<<endl;
			
			
			
			//-----------------first calculate Phi angle
			for(Int_t search = 0 ; search<4 ; search++){
			   	if(psa[search]==0){
			   		PSANetcharge=search; // search for the next charge of the event
					break;
				}
			}
			if(PSANetcharge==0){
				q1=1;
				q2=3;
			}
			else if(PSANetcharge==3){
				q1=0;
				q2=2;
			}
			else if(PSANetcharge==2){
				q1=PSANetcharge+1;
				q2=PSANetcharge-1;
			}
			
			else {
				q1=PSANetcharge+1;
				q2=PSANetcharge-1;
			}
			
			PSA_phi=TMath::Log10(abs(s[q2])/abs(s[q1]));
			PSA_phi=(1.0*PSA_phi*TMath::RadToDeg()-PSAphiOff[PSANetcharge+4*(MapFinger)]); // in Deg 
			//if(abs(PSA_phi)>45)PSA_phi=0; //phi cannot be larger than the segment side; if so, centers the value
			if(BoolSpec)fMyHistoPhiMirror->Fill(PSA_phi);// in Deg
			if(BoolSpec)fMyHistoPhiMirrorPattern->Fill(clo*100+10*cri+PSANetcharge,PSA_phi);// in Deg
			
			
			//place phi at the center of the segment
			PSA_phi=PSA_phi+45;
			if(PSA_debug)cout<<"Phi " <<PSA_phi<<endl;
			//-----------------END calculate Phi angle
						
			//------------------------------------Secondly calculate Radius
			psa_T[0]=Cal(T30,0.,-5.0,0);//NUMEXO2 LSB for these quantities is 2.5 ns, ie each ticks is 2.5 ns
			psa_T[1]=Cal(T30,0.,-2.5,0);
			psa_T[2]=Cal(T30,0.,2.5,0);
			psa_T[3]=Cal(T60,0.,2.5,0);
			psa_T[4]=Cal(T90,0.,2.5,0);
			psa_T[5]=psa_T[4]+500.;
			
			psa_Q[0]=0;
			psa_Q[1]=0.;
			psa_Q[2]=30.;
			psa_Q[3]=60.;
			psa_Q[4]=90.;
			psa_Q[5]=100.;
			if(PSA_debug)cout<<"Input Trace"<<endl;
		
		        for(Int_t psaStep = 0 ; psaStep<6; psaStep++){
				if(PSA_debug)cout<<psa_T[psaStep]<<"  "<<psa_Q[psaStep]<<endl;
			  	traceExp->SetPoint(psaStep,psa_T[psaStep],psa_Q[psaStep]);
				if(MapFinger>0)ShortTrace->SetPoint(psaStep,psa_T[psaStep],psa_Q[psaStep]);
			}
			
	
			trace->SetParameter(0,100);
			trace->SetParameter(1,100) ;
			trace->SetParameter(2,-300);
			for(Int_t trial = 0 ; trial < 2; trial++){
				traceExp->Fit("trace","Q");
			}
			if(PSA_debug)cout<<"Chi2 is "<<trace->GetChisquare()<<endl;
			if(BoolSpec)fMyHistoPSAK->Fill(Cal(-1*trace->GetParameter(2),LIVERPOOL[1],LIVERPOOL[0],0));
			if(BoolSpec)fMyHistoPSAK_Pattern->Fill(MapFinger,-1*trace->GetParameter(2));
			if(BoolSpec)fMyHistoPSAL_Pattern->Fill(MapFinger,-1*trace->GetParameter(1));
			WS1=-1*trace->GetParameter(1);
			WS2=-1*trace->GetParameter(2);
			if(BoolSpec)fMyHistoPSAKL->Fill(WS2,WS1);  //K(2) is diffusness L(1) is the 0 level
			chi2=trace->GetChisquare();
			if(BoolSpec)fMyHistoPSAChi2->Fill(chi2);
			if(BoolSpec)fMyHistoPSAChi2_Pattern->Fill(MapFinger,chi2);
			if(chi2<PSACutOFF){	// if chi� is good then we proceed
				
				//Traces LIverpool bases algo for R
				bestchi2=1e10;
   				RforBestChi2=100;
				for(Int_t x=0;x<23;x++){ //from 0 to 11 it is a scan of A1 segment and from 12 to 23 it is a scan of A4
					for(Int_t u = 0 ; u<3 ;u++){base->SetParameter(u,PSABASE[x][u]);} //get base
					for(Int_t u = -10 ; u<=10 ;u=u+10){	//authorizes small TS scan (3 thick step)
						psachi2=0;
						for(Int_t TS = psa_T[1] ; TS<psa_T[4];TS=TS+2){ //calculate the chi� every 2 ns
							psachi2=psachi2+pow(trace->Eval(TS+u)-base->Eval(TS),2)/base->Eval(TS);
						}
						psachi2=psachi2*2./(psa_T[4]-psa_T[1]);
						if(psachi2<bestchi2){
							bestchi2=psachi2;
							RforBestChi2=PSABASER[x];
						}	
					}
				}
				if(BoolSpec)fMyHistoPSAChi2GRID->Fill(bestchi2);
				if(BoolSpec)fMyHistoPSAChi2GRID_Pattern->Fill(MapFinger,bestchi2);
				
				
				
				//Full PSA Grid Search method
				PSA_r=RforBestChi2;
				PSA_r=gRandom->Gaus(RforBestChi2,2);
				//MINIBALL Method
				//PSA_r=Cal(TMAX,MINIBALL[1],MINIBALL[0],0)); 
				
				//LIVERPOOL Based WS fit
				PSA_r=Cal(-1*trace->GetParameter(2),LIVERPOOL[1],LIVERPOOL[0],0); 
				
				
				//mix of possible methods
				PSA_r=(2*Cal(-1*trace->GetParameter(2),LIVERPOOL[1],LIVERPOOL[0],0)+gRandom->Gaus(RforBestChi2,2))/3.;
				
				if(BoolSpec)fMyHistoPSAChi2_Radius->Fill(bestchi2,PSA_r);

				//CFD calculation
				CFDOK=false;
				for(Int_t u=0;u<500;u++){A[u]=B[u]=C[u]=0;} //prepare the numerical CFD
				
				
				for(Int_t trial =0 ; trial <=400 ; trial++){
					A[trial]=0.25*trace->Eval(trial); //25% constrant Fraction
					B[trial]=-1.0*trace->Eval(trial-80); // 80 ns delays
				}
				for(Int_t trial =0 ; trial <200 ; trial++){
					C[trial]=A[trial]+B[trial];
					if(CFDOK==false&&C[trial]<0.02){ //ZCO time 
						CFD_PSA_Value=Cal(trial,0.,1.,0); // in ns
						CFDOK=true;
					}
				}
				if(BoolSpec)fMyHistoPSA_CFD->Fill(CFD_PSA_Value);
				if(BoolSpec)fMyHistoPSA_CFD_Pattern->Fill(MapFinger,CFD_PSA_Value);

				//From trace, get MINIBALL Style Radius
				for(Int_t trial = psa_T[1] ; trial <psa_T[4] ; trial=trial+2){ //MINIBALL algo for R
					if(trace->Derivative2(trial)<QMAX){
						QMAX=trace->Derivative2(trial);
						TMAX=1.*trial;
					}
				}
				if(PSA_debug)cout<<"Max is "<<QMAX<<" at "<<TMAX<<endl;
				if(BoolSpec)fMyHistoPSARadius_Radius->Fill(RforBestChi2,Cal(TMAX,MINIBALL[1],MINIBALL[0],0));

			}
			
			//------------------------------------END calculate Radius
			
				//For R&D purpose, print the first 100 good traces
				if(MGTCounter<100){//print out the first 100 good traces
					fprintf(MGTTraces,"%d %f %d\n",-1000,psa_T[4]+100-(psa_T[1]-100),MapFinger); //new evt marker  & number of steps & Core ID
					for(Int_t TS = psa_T[1]-100 ; TS<psa_T[4]+100;TS++){
						fprintf(MGTTraces,"%d %f \n",TS,trace->Eval(TS));
					}
				}
				else{
					if(closed==false){
						fprintf(MGTTraces,"%d %f\n",-2000,-12000.); //end of file
						fclose(MGTTraces);
						closed=true;
					}
  				}
				MGTCounter++;
			
			if(BoolSpec){fMyHistoPSAKEgamma->Fill(bestchi2*100,valf);}
			//chi2 is from WS fit bestchi2 is base comparison with LIVERPOOL scan
			if(chi2<PSACutOFF&&bestchi2<0.5&&abs(CFD_PSA_Value-105)<10&&WS2>15&&WS2<60&&WS1>-150&&WS1<-20){ //these are good traces and good GridSearch 
			
				if(BoolSpec){
					fMyHistoRMirror->Fill(PSA_r);
					fMyHistoPSASurface->Fill(PSA_r,PSAHit.Phi()*TMath::RadToDeg());
					fMyHistoPSAChi2_Radius->Fill(bestchi2,PSA_r);
					fMyHistoPSA_CFD_E->Fill(CFD_PSA_Value,valf); //nice cut-off for < 200 ns
					fMyHistoRPattern->Fill(MapFinger,PSA_r);
					fMyHistoPSAAcceptedTraces->Fill(valf);
					fMyHistoPSACore->Fill(valf);
					fMyHistoPSAKL_postGrid->Fill(WS2,WS1);
				}
			
				//set the  vector
				PSAHit.SetMagPhi(PSA_r,PSA_phi*TMath::DegToRad());

				//Now starts the Segment-to-crystal-to-clover rotations
				Xp=PSAHit.X();
				Yp=PSAHit.Y();
				Rp=sqrt(Xp*Xp+Yp*Yp);
				//cout<<Rp<<" - 1 - "<<PSA_r<<endl;
				
				//placed in the crystal reference frame
				Xp=PSAHit.X()*TMath::Cos(PSANetcharge*TMath::Pi()/2.)-PSAHit.Y()*TMath::Sin(PSANetcharge*TMath::Pi()/2.);
				Yp=PSAHit.X()*TMath::Sin(PSANetcharge*TMath::Pi()/2.)+PSAHit.Y()*TMath::Cos(PSANetcharge*TMath::Pi()/2.);
				//placed in the clover reference frame
				Xpp=Xp*TMath::Cos(cri*TMath::Pi()/2.)-Yp*TMath::Sin(cri*TMath::Pi()/2.);
				Ypp=Xp*TMath::Sin(cri*TMath::Pi()/2.)+Yp*TMath::Cos(cri*TMath::Pi()/2.);
				Xp=Xpp;
				Yp=Ypp;
				Rp=sqrt(Xp*Xp+Yp*Yp);
				//cout<<Rp<<" - 2 - "<<PSA_r<<endl;
				
				if(Xp>0&&Yp>=0)ThetaP=TMath::ATan(Yp/Xp);
				if(Xp>0&&Yp<0)ThetaP=TMath::ATan(Yp/Xp)+2*TMath::Pi();
				if(Xp<0)ThetaP=TMath::ATan(Yp/Xp)+TMath::Pi();
				if(Xp==0&&Yp>0)ThetaP=TMath::Pi()/2.;
				if(Xp==0&&Yp<0)ThetaP=3*TMath::Pi()/2.;
				
				//set the  vector
				PSAHit.SetMagPhi(PSA_r,ThetaP);
				
				
				Xp=PSAHit.X()+GammaPosCoreX[clo][cri];
				Yp=PSAHit.Y()+GammaPosCoreY[clo][cri];
				Zp=GammaPosCoreZ[clo][cri];
				
				PSAHit3D.SetXYZ(Xp,Yp,Zp);
				if(BoolSpec)fMyHistoPSAWorld->Fill(-1*PSAHit3D.Y(),PSAHit3D.X());
				//placed in the clover reference frame
				if(cri==0){PSAHit.Set(PSAHit.X()+30,PSAHit.Y()-30);}
				else if (cri==1){PSAHit.Set(PSAHit.X()-30,PSAHit.Y()-30);}
				else if (cri==2){PSAHit.Set(PSAHit.X()-30,PSAHit.Y()+30);}
				else {PSAHit.Set(PSAHit.X()+30,PSAHit.Y()+30);}
				
				if(BoolSpec)fMyHistoPSASurfaceCarte->Fill(PSAHit.X(),PSAHit.Y());
				
				
				
			
			}
			else{
				if(BoolSpec)fMyHistoPSARejectedTraces->Fill(valf);
				if(BoolSpec)fMyHistoPSACore->Fill(valf);

			
			}
       		}
       
       }
  //------------------Now Anti Comption     
       UShort_t sum = frame->ExoGetBGO()+frame->ExoGetCsi(); //do a sum of CsI+BGO energies
       if(sum>ESSThreshold&&IsActivateESS()){
       		NoComptonCore[clo][cri]=false;
       		NoCompton[clo]=false;
       }
       if(BoolSpec&&IsCloverActive(clo)&&IsActivateESS()){
       		fMyHistoESS_BGO[clo*100+10*cri]->Fill(frame->ExoGetBGO());
       		fMyHistoESS_CSI[clo*100+10*cri]->Fill(frame->ExoGetCsi());
		fMyHistoPatternESSTQ->Fill(clo*10+cri,sum);
       }
       

   }//if core Id>=0
   
       

   return result;
}



bool TExogam2::Treat()
{
int clo,cri,cloESS,cloverMul,id1,id2,EnergyAddTQDCMAPFINGER[16],clo1,cri1,clo2,cri2;
char cristal[5]={'A','B','C','D','\0'};
float EnergyAddTQ[16],EnergyAddTC[16],EnergyAdd[16],mulCrysPerClover[16],EnergyAddTQDC[16];
float E877CloverE[16],E877CloverT[16],E877MaxCrystalE[16],E877CloverBGO[16],E877CloverCSI[16];
bool E877CloverFired[16];
Bool_t NfsCrystalFired[16*4],NfsCrystalBgoFired[16*4],NfsCrystalCsiFired[16*4];
float Theta_Gamma, Phi_Gamma, X_Gamma, Y_Gamma, Z_Gamma;
float X_Gamma_Core, Y_Gamma_Core, Z_Gamma_Core;
bool IsPrompt[16][4];
float SumCalorimeter;
Long64_t TS1, TS2;
bool GateisValid;

GateisValid=false;
SumCalorimeter=0;
	//------------------Time Treat
	for(Int_t c=0;c<16;c++){
		for(Int_t d=0;d<4;d++){
			IsPrompt[c][d]=false;
		}
	}

//Using the TDC 
	for (UShort_t i = 0; i < fExogam2Data->GetECCTMult(); i++) {
		clo=fExogam2Data->GetECCTClover(i);
		cri=fExogam2Data->GetECCTCristal(i);
		if(fExogam2Data->GetECCTTime(i)>=promptL&&fExogam2Data->GetECCTTime(i)<=promptH)IsPrompt[clo][cri]=true;
		
	}
	
//Using the Time stamp analysis if no TDC available	
	/*for(Int_t a = 0; a<16;a++){
	 	for(Int_t b = 0; b<16;b++){IsPrompt[clo][cri]=IsPromptPublic[a][b];}
	 }
	*/
	for (UShort_t i = 0; i < fExogam2Data->GetGOCCEEMult(); i++) {
		for (UShort_t j = 0; j < fExogam2Data->GetGOCCEEMult(); j++) {
				id1=fExogam2Data->GetGOCCEEClover(i)*16+fExogam2Data->GetGOCCEECristal(i)*10+fExogam2Data->GetGOCCEESegment(i);
				id2=fExogam2Data->GetGOCCEEClover(j)*16+fExogam2Data->GetGOCCEECristal(j)*10+fExogam2Data->GetGOCCEESegment(j);
				if(id1!=id2&&BoolSpec&&fExogam2Data->GetGOCCEEEnergy(i)>100&&fExogam2Data->GetGOCCEEEnergy(j)>100&&BoolSpec){fMyHistoSegSeg->Fill(id1,id2);}
		}
	}
	//------------------Energy Treat
	// EN: NFS no-cut gamma-gamma matrix uses all positive raw crystal energies before prompt/TS/veto cuts.
	// CN: NFS 无 cut gamma-gamma 矩阵使用所有正 raw crystal 能量，位置在 prompt/TS/veto 判断之前。
	if(NfsSpec && fNfsAllGammaGammaMatrixNoCut && fExogam2Data->GetECCEMult()>1){
		for (UShort_t i = 0; i < fExogam2Data->GetECCEMult(); i++) {
			if(fExogam2Data->GetECCEEnergy(i)<=0)continue;
			for (UShort_t j = 0; j < fExogam2Data->GetECCEMult(); j++) {
				if(j==i)continue;
				if(fExogam2Data->GetECCEEnergy(j)<=0)continue;
				fNfsAllGammaGammaMatrixNoCut->Fill(fExogam2Data->GetECCEEnergy(i),fExogam2Data->GetECCEEnergy(j));
			}
		}
	}

	if(fExogam2Data->GetECCEMult()>1){
		for (UShort_t i = 0; i < fExogam2Data->GetECCEMult(); i++) {
			for (UShort_t j = 0; j < fExogam2Data->GetECCEMult(); j++) {
				if(j!=i){
					clo1=fExogam2Data->GetECCEClover(i);
					cri1=fExogam2Data->GetECCECristal(i);
					clo2=fExogam2Data->GetECCEClover(j);
					cri2=fExogam2Data->GetECCECristal(j);
					if(BoolSpec&&fExogam2Data->GetECCEEnergy(i)>0&&fExogam2Data->GetECCEEnergy(j)>0&&IsPrompt[clo1][cri1]&&IsPrompt[clo2][cri2]){
						id1=fExogam2Data->GetECCEDetNbr(i);
						id2=fExogam2Data->GetECCEDetNbr(j);
						TS1=Cal(fExogam2Data->GetfTimeStamps(id1),0.,1.,0);
						TS2=Cal(fExogam2Data->GetfTimeStamps(id2),0.,1.,0);
						//cout<<id1<<"  " <<id2<< "  "<<TS1-TS2<<endl;
						//if(BoolSpec&&TMath::Abs(TS1-TS2)<5){
						if(BoolSpec&&TimeStampDiffQ(fExogam2Data->GetfTimeStamps(id1),fExogam2Data->GetfTimeStamps(id2),5)){
							fMyHistoGammaGammaCore->Fill(fExogam2Data->GetECCEEnergy(i),fExogam2Data->GetECCEEnergy(j)); //all core gg
							if(fExogam2Data->GetECCEClover(i)==fExogam2Data->GetECCEClover(j)){ //if part of the same clover; then gg only on diagonal cristal
								if(IsDiagonal(fExogam2Data->GetECCECristal(i),fExogam2Data->GetECCECristal(j)))fMyHistoGammaGammaCoreDiag->Fill(fExogam2Data->GetECCEEnergy(i),fExogam2Data->GetECCEEnergy(j));
							}
							else {
								fMyHistoGammaGammaCoreDiag->Fill(fExogam2Data->GetECCEEnergy(i),fExogam2Data->GetECCEEnergy(j));
							}
						}
						//if(BoolSpec)fMyHistoTT[id1][id2]->Fill(Cal(fExogam2Data->GetfTimeStamps(id1),0.,10.,0)-Cal(fExogam2Data->GetfTimeStamps(id2),0.,10.,0));
						if(BoolSpec)fMyHistoTTAll->Fill(Cal(fExogam2Data->GetfTimeStamps(id1),0.,10.,0)-Cal(fExogam2Data->GetfTimeStamps(id2),0.,10.,0));
						//if(TMath::Abs(Cal(fExogam2Data->GetfTimeStamps(id1),0.,10.,0)-Cal(fExogam2Data->GetfTimeStamps(id2),0.,10.,0)-289)<30)cout<<id1<<"  "<<id2<<endl;
					}
				}
			}
		}
	}
		
	for(Int_t c=0;c<16;c++){
		EnergyAddTQDCMAPFINGER[c]=EnergyAddTQ[c]=EnergyAddTC[c]=EnergyAdd[c]=mulCrysPerClover[c]=EnergyAddTQDC[c]=0.;
		E877CloverE[c]=E877CloverT[c]=E877MaxCrystalE[c]=E877CloverBGO[c]=E877CloverCSI[c]=0.;
		E877CloverFired[c]=false;
	}
	for(Int_t id=0; id<16*4; id++){
		NfsCrystalFired[id]=false;
		NfsCrystalBgoFired[id]=false;
		NfsCrystalCsiFired[id]=false;
	}

	cloverMul=0;
	for (UShort_t i = 0; i < fExogam2Data->GetECCEMult(); i++) {
		clo=fExogam2Data->GetECCEClover(i);
		cri=fExogam2Data->GetECCECristal(i);
		//cerr<<clo<<" IN "<<cri<<endl;
	        
		if(IsCloverActive(clo)==false){continue;}
		
		//cerr<<clo<<" OUT "<<cri<<endl;

		mulCrysPerClover[clo]++;
		mulECC[clo][cri]++;
		if(mulG[clo][cri]==4)mulECCG[clo][cri]++;
		if(mulGnrj[clo][cri]>0)mulECCGnrj[clo][cri]++;
		// EN: NFS E877 addback uses raw crystal fires grouped by clover, before prompt/AC cuts.
		//     The threshold here is only energy > 0, to reject empty/zero gamma deposits.
		// CN: NFS E877 合并直接把 raw crystal fire 按 clover 分组，不套 prompt/AC 判断。
		//     这里唯一能量阈值是 energy > 0，用来排除空/零能量 gamma 沉积。
		if(fExogam2Data->GetECCEEnergy(i)>0){
			Float_t e877Energy=fExogam2Data->GetECCEEnergy(i);
			Float_t e877Time=0.;
			if(i<fExogam2Data->GetTimeMult())e877Time=fExogam2Data->GetTime(i);

			E877CloverFired[clo]=true;
			E877CloverE[clo]+=e877Energy;
			if(e877Energy>E877MaxCrystalE[clo]){
				E877MaxCrystalE[clo]=e877Energy;
				E877CloverT[clo]=e877Time;
			}
			if(i<fExogam2Data->GetESSTQMult()){
				// EN: BGO/CSI values are kept as stored; later veto/efficiency uses >0 as fire condition.
				// CN: BGO/CSI 保留原始存储值；后续 veto/效率统计用 >0 作为 fire 条件。
				E877CloverBGO[clo]+=fExogam2Data->GetESSTQBGO(i);
				E877CloverCSI[clo]+=fExogam2Data->GetESSTQCSI(i);
			}

			Int_t nfsCrystalId=clo*4+cri;
			if(nfsCrystalId>=0 && nfsCrystalId<16*4){
				FillNfsSpectra(nfsCrystalId,clo,cri,e877Time,e877Energy);
				// EN: Event-level crystal fire for cross-talk and BGO/CSI efficiency, with zero-energy fires skipped.
				// CN: 用于串扰和 BGO/CSI 效率的 event 级 crystal fire，跳过零能量 fire。
				NfsCrystalFired[nfsCrystalId]=true;
				if(i<fExogam2Data->GetESSTQMult()){
					if(fExogam2Data->GetESSTQBGO(i)>0)NfsCrystalBgoFired[nfsCrystalId]=true;
					if(fExogam2Data->GetESSTQCSI(i)>0)NfsCrystalCsiFired[nfsCrystalId]=true;
				}
			}
		}
		//cerr<<mulGnrj[clo][cri]<<endl;
//------------------------------------------------------------------------------------------------------------------HERE the prompt cond!!!!!!!!!!!!!!!!!!
		if(IsPrompt[clo][cri]==false){
			//cout<<"Not Prompt"<<endl;
			continue;
		}
		if(NoComptonCore[clo][cri]==false){
			NoComptonCoreCounter[clo][cri]++;
			if(BoolSpec)fMyHistoECCEVetoed[clo*100+10*cri]->Fill(fExogam2Data->GetECCEEnergy(i));
		}
		else{
			if(BoolSpec)fMyHistoECCEAccepted[clo*100+10*cri]->Fill(fExogam2Data->GetECCEEnergy(i));
			//EnergyAddTQ[clo]=EnergyAddTQ[clo]+fExogam2Data->GetECCEEnergy(i);
		}
		
		if(NoCompton[clo]&&IsPrompt[clo][cri]){
		
			EnergyAddTC[clo]=EnergyAddTC[clo]+fExogam2Data->GetECCEEnergy(i);
			Theta_Gamma=GammaAngleSegTheta[clo][MaxCris[clo]][MaxSeg[clo][MaxCris[clo]]];
			Phi_Gamma=GammaAngleSegPhi[clo][MaxCris[clo]][MaxSeg[clo][MaxCris[clo]]];
			X_Gamma=GammaAngleSegX[clo][MaxCris[clo]][MaxSeg[clo][MaxCris[clo]]];
			Y_Gamma=GammaAngleSegY[clo][MaxCris[clo]][MaxSeg[clo][MaxCris[clo]]];
			Z_Gamma=GammaAngleSegZ[clo][MaxCris[clo]][MaxSeg[clo][MaxCris[clo]]];
			X_Gamma_Core=GammaPosCoreX[clo][MaxCris[clo]];
			Y_Gamma_Core=GammaPosCoreY[clo][MaxCris[clo]];
			Z_Gamma_Core=GammaPosCoreZ[clo][MaxCris[clo]];
			
			//analysed Tree at core level, no Addback
			fExogam2Data->SingleSetGammaEnergy(fExogam2Data->GetECCEEnergy(i));
			fExogam2Data->SingleSetGammaTheta(Theta_Gamma);
			fExogam2Data->SingleSetGammaPhi(Phi_Gamma);
			fExogam2Data->SingleSetGammaCoreId(fExogam2Data->GetECCEDetNbr(i));
			fExogam2Data->SingleSetGammaX(X_Gamma);
			fExogam2Data->SingleSetGammaY(Y_Gamma);
			fExogam2Data->SingleSetGammaZ(Z_Gamma);
			fExogam2Data->SingleSetGammaX_Core(X_Gamma_Core);
			fExogam2Data->SingleSetGammaY_Core(Y_Gamma_Core);
			fExogam2Data->SingleSetGammaZ_Core(Z_Gamma_Core);
			
			
			
			
		}
		
		//here is the AddBack without selection
		if(fExogam2Data->GetECCEEnergy(i)>10)EnergyAdd[clo]=EnergyAdd[clo]+fExogam2Data->GetECCEEnergy(i); 
		
		
		//---------------------------------------------------------
		for(Int_t k =0 ; k<4;k++){
			if(Goccetrack&&GOCCE_Pat[clo][cri][k]==false)printf("\033[31m GOCCE %d Crys %c Seg %d No Data   \033[m \n",clo,cristal[cri],k);
			if(Goccetrack&&GOCCE_Ener[clo][cri][k]==false)printf("\033[31m GOCCE %d Crys %c Seg %d Only Pedestal   \033[m \n",clo,cristal[cri],k);
		}
		for(UShort_t j = 0; j < fExogam2Data->GetESSTQMult(); j++){
			cloESS=fExogam2Data->GetESSTQClover(j);
			if(cloESS==clo&&BoolSpec)fMyHistoECCEESSE[clo*100+10*cri]->Fill(fExogam2Data->GetECCEEnergy(i),fExogam2Data->GetESSTQBGO(j)+fExogam2Data->GetESSTQCSI(j));
		}
		
		//---------------------------------------------------------

   	}

	FillNfsCrystalEventSpectra(NfsCrystalFired,NfsCrystalBgoFired,NfsCrystalCsiFired);

	// EN: Store and histogram the raw clover addback requested for the NFS E877-style tree.
	// CN: 保存并绘制 NFS E877 格式的 raw clover 合并结果。
	for(Int_t c=0;c<16;c++){
		if(CloverPresent[c]==false)continue;
		if(E877CloverFired[c]==false)continue;
		fExogam2Data->SetE877Clover(c);
		fExogam2Data->SetE877CloverEnergy(E877CloverE[c]);
		fExogam2Data->SetE877CloverTime(E877CloverT[c]);
		fExogam2Data->SetE877CloverBGO(E877CloverBGO[c]);
		fExogam2Data->SetE877CloverCSI(E877CloverCSI[c]);
		FillNfsCloverAddbackSpectra(c,E877CloverE[c],E877CloverBGO[c],E877CloverCSI[c]);
	}

	if(BoolSpec)fMyHistoMultiCrystal->Fill(fExogam2Data->GetECCEMult());
	if(BoolSpec)fMyHistoMultiAntiCompt->Fill(fExogam2Data->GetESSTQMult());

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
			EnergyAddTQDCMAPFINGER[c]=c*4+MaxCris[c]; //save the Cristal Map Finger for the highest cristal energy into the "c" clover after AddBack
			
			if(MaxSeg[c][MaxCris[c]]<1000){ //if Max Seg is different than the Init Value
				Theta_Gamma=GammaAngleSegTheta[c][MaxCris[c]][MaxSeg[c][MaxCris[c]]];
				Phi_Gamma=GammaAngleSegPhi[c][MaxCris[c]][MaxSeg[c][MaxCris[c]]];
				X_Gamma=GammaAngleSegX[c][MaxCris[c]][MaxSeg[c][MaxCris[c]]];
				Y_Gamma=GammaAngleSegY[c][MaxCris[c]][MaxSeg[c][MaxCris[c]]];
				Z_Gamma=GammaAngleSegZ[c][MaxCris[c]][MaxSeg[c][MaxCris[c]]];
				X_Gamma_Core=GammaPosCoreX[c][MaxCris[c]];
				Y_Gamma_Core=GammaPosCoreY[c][MaxCris[c]];
				Z_Gamma_Core=GammaPosCoreZ[c][MaxCris[c]];
				//cout<<c <<"I am using the Seg for DC"<<endl;
			}
			else{
				Theta_Gamma=GammaAngleCoreTheta[c][MaxCris[c]];
				Phi_Gamma=GammaAngleCorePhi[c][MaxCris[c]];
				X_Gamma_Core=GammaPosCoreX[c][MaxCris[c]];
				Y_Gamma_Core=GammaPosCoreY[c][MaxCris[c]];
				Z_Gamma_Core=GammaPosCoreZ[c][MaxCris[c]];
				
				//if(IsActivateGOCCE())printf("\033[31m I am using the core for DC for this clover ********* \033[m \n");
			}
			//cerr<<"Do DC "<<Theta_Gamma*180./3.14159<<" ::  "<<Beta<<endl;
			if(BoolSpec)fMyHistoSumExogam2DC->Fill(Doppler_Correction(Theta_Gamma,0.0,Theta_P, Phi_P, Beta, EnergyAddTC[c]));
			if(BoolSpec)fMyHistoSumExogam2BeforeAfterDC->Fill(EnergyAddTC[c],Doppler_Correction(Theta_Gamma,0.0,Theta_P, Phi_P, Beta, EnergyAddTC[c]));
			if(BoolSpec)fMyHistoSumExogam2DC_VTS->Fill(Doppler_Correction(Theta_Gamma,0.0,Theta_P, Phi_P, Beta, EnergyAddTC[c]),DTSAlign);
			
			if(noyau>=0&&BoolSpec){
				fMyHistoSumExogam2DCNucId[noyau]->Fill(Doppler_Correction(Theta_Gamma,0.0,Theta_P, Phi_P, Beta, EnergyAddTC[c]));
				TransverseMomentumGamma->Fill(FreeParam1,Doppler_Correction(Theta_Gamma,0.0,Theta_P, Phi_P, Beta, EnergyAddTC[c]));
			}
			//if(noyau==2)SumCalorimeter=SumCalorimeter+Doppler_Correction(Theta_Gamma,0.0,0., 0., Beta, EnergyAddTC[c]); //gated on Sulfur
			SumCalorimeter=SumCalorimeter+Doppler_Correction(Theta_Gamma,0.0,Theta_P, Phi_P, Beta, EnergyAddTC[c]); 
			
			if(BoolSpec)fMyHistoSumExogam2DCClover->Fill(c,Doppler_Correction(Theta_Gamma,0.0,Theta_P, Phi_P, Beta, EnergyAddTC[c]));
			if(BoolSpec)fMyHistoSumExogam2Clover->Fill(c,Doppler_Correction(Theta_Gamma,0.0,Theta_P, Phi_P, 0, EnergyAddTC[c]));

			
			//Analyzed Branches
			fExogam2Data->SetGammaEnergy(EnergyAddTC[c]);
			fExogam2Data->SetGammaTheta(Theta_Gamma);
			fExogam2Data->SetGammaPhi(Phi_Gamma);
			fExogam2Data->SetGammaCoreId(EnergyAddTQDCMAPFINGER[c]);
			fExogam2Data->SetGammaX(X_Gamma);
			fExogam2Data->SetGammaY(Y_Gamma);
			fExogam2Data->SetGammaZ(Z_Gamma);
			fExogam2Data->SetGammaX_Core(X_Gamma_Core);
			fExogam2Data->SetGammaY_Core(Y_Gamma_Core);
			fExogam2Data->SetGammaZ_Core(Z_Gamma_Core);
			

			//Fold condition
			if(cloverMul==1&&BoolSpec)fMyHistoSumExogam2DCFoldCond->Fill(Doppler_Correction(Theta_Gamma,0.0,Theta_P, Phi_P, Beta, EnergyAddTC[c]));
			//----------------
			
			
			if(BoolSpec)fMyHistoAngletheta->Fill(Theta_Gamma*180./3.14159);
			if(BoolSpec)fMyHistoAnglephi->Fill(Phi_Gamma*180./3.14159);
			if(BoolSpec)fMyHistoCloverECalACAdd_DC[c]->Fill(Doppler_Correction(Theta_Gamma,Phi_Gamma,Theta_P, Phi_P, Beta, EnergyAddTC[c]));
			
			EnergyAddTQDC[c]=Doppler_Correction(Theta_Gamma,Phi_Gamma,0., 0., Beta, EnergyAddTC[c]);
			
			if(BoolSpec)fMyHistoSumExogam22D->Fill(Theta_Gamma*180./3.14159,EnergyAddTC[c]);
			if(BoolSpec)fMyHistoSumExogam2DC2D->Fill(Theta_Gamma*180./3.14159,Doppler_Correction(Theta_Gamma,Phi_Gamma,Theta_P, Phi_P, Beta, EnergyAddTC[c]));
			
			
		}
		if(EnergyAddTC[c]>0&&BoolSpec){fMyHistoCloverECalACAdd_TC[c]->Fill(EnergyAddTC[c]);} //with selection
		if(EnergyAdd[c]>0&&BoolSpec)  {fMyHistoCloverECalAdd[c]->Fill(EnergyAdd[c]);} //without selection
		
	}
     }
	if(SumCalorimeter>0&&BoolSpec)fMyHistoSumExogam2Calorimeter->Fill(SumCalorimeter);
	
	
	//NFS Cases
	if(neutronNRJ>0&&neutronNFS){
		for(Int_t c=0;c<16;c++){
			if(abs(EnergyAddTC[c]-GammaGateNFS)<5)GateisValid=true;
		}
		if(SumCalorimeter>0&&BoolSpec&&GateisValid)fMyHistoNFS_EnEstar->Fill(neutronNRJ,SumCalorimeter); //matrix gated by GammaGateNFS nrj
		if(BoolSpec&&GateisValid)fMyHistoNFS_EnGmul->Fill(neutronNRJ,cloverMul); //matrix gated by GammaGateNFS nrj
	}
	
	
	//create gamma-gamma matrix
	if(cloverMul>1){
		for(Int_t c=0;c<16;c++){
			if(CloverPresent[c]==false) continue;
			for(Int_t d=0;d<16;d++){
				if(CloverPresent[d]==false) continue;
				TS1=Cal(fExogam2Data->GetfTimeStamps(EnergyAddTQDCMAPFINGER[c]),0.,1.,0);//get the TS of the Cristal Map Finger for the highest cristal energy into the "c" clover after AddBack
				TS2=Cal(fExogam2Data->GetfTimeStamps(EnergyAddTQDCMAPFINGER[d]),0.,1.,0);//get the TS of the Cristal Map Finger for the highest cristal energy into the "d" clover after AddBack
				if(BoolSpec&&c!=d&&EnergyAddTQDC[c]>0&&EnergyAddTQDC[d]>0)fMyHistoTTinGammaGamma->Fill(TS1-TS2);
				
				if(c!=d&&EnergyAddTQDC[c]>0&&EnergyAddTQDC[d]>0&&
					//TMath::Abs(TS1-TS2)<5
					TimeStampDiffQ(fExogam2Data->GetfTimeStamps(EnergyAddTQDCMAPFINGER[c]),fExogam2Data->GetfTimeStamps(EnergyAddTQDCMAPFINGER[d]),5)
					&&BoolSpec)fMyHistoGammaGamma->Fill(EnergyAddTQDC[c],EnergyAddTQDC[d]); // with DC
			}
		}
	}
	
	if(BoolSpec)fMyHistoMultiClover->Fill(cloverMul);

   return true;
}
bool TExogam2::CounterReset()
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
bool TExogam2::Counter()
{	

bool GOCCEfailure1,GOCCEfailure2;
float div,div3;
int div2,tvb;
char cristal[5]={'A','B','C','D','\0'}; 
GOCCEfailure1=GOCCEfailure2=false;
tvb=0; 

	/*FILE *LogFile= fopen("LogFileDataCheck.log","w");
        TDatime date;
        fprintf(LogFile," Log File at Date :%d Time : %d \n",date.GetDate(),date.GetTime());
*/
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
						//printf("\033[31m ECC%dCrys%c Missing GOCCE Ratio (Gated Word/Raw) = %2.3f percent \033[m \n",i, cristal[j],div);
						printf("\033[32m ECC%dCrys%c Missing GOCCE Ratio (Gated Nrj/Raw) = %2.3f percent \033[m \n",i, cristal[j],(1-1.*mulECCGnrj[i][j]/mulECC[i][j])*100.);
			        		GOCCEfailure1=true;
					}
			 		else if(div3>10.){
						
						//printf("\033[32m ECC%dCrys%c Missing GOCCE Ratio (Gated Word/Raw) = %2.3f percent \033[m \n",i, cristal[j],div);
						printf("\033[31m ECC%dCrys%c Missing GOCCE Ratio (Gated Nrj/Raw) = %2.3f percent \033[m \n",i, cristal[j],(1-1.*mulECCGnrj[i][j]/mulECC[i][j])*100.);
						GOCCEfailure2=true;
					}
					else{
						//printf("\033[32m ECC%dCrys%c Missing GOCCE Ratio (Gated Word/Raw) = %2.3f percent \033[m \n",i, cristal[j],div);
						printf("\033[32m ECC%dCrys%c Missing GOCCE Ratio (Gated Nrj/Raw) = %2.3f percent \033[m \n",i, cristal[j],(1-1.*mulECCGnrj[i][j]/mulECC[i][j])*100.);
					}
				//fprintf(LogFile,"ECC%dCrys%c Missing GOCCE Ratio (Gated Word/Raw) = %2.3f \n",i, cristal[j],div);
				//fprintf(LogFile,"ECC%dCrys%c Missing GOCCE Ratio (Gated Nrj/Raw) = %2.3f \n",i, cristal[j],(1-1.*mulECCGnrj[i][j]/mulECC[i][j])*100.);

				}
				
				//ESS checking
				if(IsActivateESS()){
					div2=(int)((1.*NoComptonCoreCounter[i][j]/mulECC[i][j])*100);
					if(div2==0){printf("\033[31m ESS%dCryst%c without Data   \033[m \n",i, cristal[j]);}
					else{
						printf("\033[34m ECC%dCrys%c AC suppression ratio = %2.1f per cent \033[m \n",i, cristal[j],(1.*NoComptonCoreCounter[i][j]/mulECC[i][j])*100.);
						if(BoolSpec)fMyHistoCheckEss->Fill((1.*NoComptonCoreCounter[i][j]/mulECC[i][j])*100.);
						//fprintf(LogFile,"ECC%dCrys%c AC suppression ratio = %2.1f \n",i, cristal[j],(1.*NoComptonCoreCounter[i][j]/mulECC[i][j])*100.);
					}
				}
		        }
		       else {printf("\033[31m ECC%dCrys%c without Data   \033[m \n",i,cristal[j]);tvb++;}

		     }
		}
	}
	cout<< "   "<<endl;
	if(tvb==0){	printf("\033[36m *********  TVB all cores are running ********* \033[m \n");
		        //fprintf(LogFile,"TVB all cores are running\n");
		}
	else{ printf("\033[31m *********  At least one core is not running  (see above) ********* \033[m \n");
		//fprintf(LogFile,"At least one core is not running\n");
		}
	if(GOCCEfailure1&&IsActivateGOCCE())printf("\033[31m *********  At least one GOCCE is not running properly (see above) : No Enough Readout of the card ********* \033[m \n");
	if(GOCCEfailure2&&IsActivateGOCCE())printf("\033[31m *********  At least one GOCCE is not running properly (see above) : Only Pedestal ********* \033[m \n");
	
	//fclose(LogFile);
return true;
}

bool TExogam2::ACReject()
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
			if(BoolSpec)fMyHistoCloverECalACAddRejectF[x]->Fill(a,(int)(div));
		}
		
		for(Int_t g = 0 ; g <4 ; g++){
			for(Int_t a =1 ; a <3000;a++){
				bin1=fMyHistoECCECal[x*100+10*g]->GetBin(a);
				bin2=fMyHistoECCEAccepted[x*100+10*g]->GetBin(a);
				c1=fMyHistoECCECal[x*100+10*g]->GetBinContent(bin1);
				c2=fMyHistoECCEAccepted[x*100+10*g]->GetBinContent(bin2);
				if(c2>0){div=10.*c1/c2;}
				else {div=0.;}
				if(BoolSpec)fMyHistoCrysECalACRejectF[x*100+10*g]->Fill(a,(int)(div));
			}
		}		
	}
	
}
	
	
return true;
}
bool TExogam2::SetCloverPosition(int ECC_VXInb,int flange, float distance)
{


struct Clover_struc Result;
int i,j;
float Real_position=distance+7.0; //mm + distance capot-crystal (np18-22-03)
if(ECC_VXInb>=0 && ECC_VXInb<16)CloverDistanceMm[ECC_VXInb]=distance;
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
		//cout<<TARGET_POSITION_X<<"  "<<TARGET_POSITION_Y<<"  "<<TARGET_POSITION_Z<<endl;
                TVector3 targetPos(TARGET_POSITION_X, TARGET_POSITION_Y, TARGET_POSITION_Z);

		// case of the selected Exogam2 detector
		/*for (Int_t k = 0; k < 3; k++) {
		   cout << "flange12 avant : " << flange12(k) << endl;
		   cout << "targetPos      : " << targetPos(k) << endl;
		}*/
		flange12 = flange12+targetPos;
		/*for (Int_t k = 0; k < 3; k++) {
		   cout << "flange12 apres : " << flange12(k) << endl;
		}*/
		// loop on cristals
		for (Int_t ii = 0; ii < 4; ii++) {
		   flange12Crist[ii] = flange12Crist[ii]+targetPos;
		   // loop on segments
		   for (Int_t jj = 0; jj < 4; jj++) {
		      flange12CristSeg[ii][jj] =flange12CristSeg[ii][jj] + targetPos;
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
				Result.X_Crystal_Seg[i][j]=GammaAngleSegX[ECC_VXInb][i][j]=flange12CristSeg[i][j].X();
				Result.Y_Crystal_Seg[i][j]=GammaAngleSegY[ECC_VXInb][i][j]=flange12CristSeg[i][j].Y();
				Result.Z_Crystal_Seg[i][j]=GammaAngleSegZ[ECC_VXInb][i][j]=flange12CristSeg[i][j].Z();	
			}
		}
		  
		 

	}
	
	else {printf("Bad flange number, flange %d  doesn't exist in your Exogam2 config : ECC%d!!!!!! \n",flange,ECC_VXInb);}


return true;
}
bool TExogam2::CheckCoreResolution(float EnergyC)
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

void TExogam2::InitBranch(TTree *tree)
{
   tree->Branch("Exogam2", "TExogam2Data", &fExogam2Data,32000,99);
}

