#include <cstdlib>

#include "TExogam.h"

#include "fstream"

ClassImp(TExogam)

TExogam::TExogam(bool bspec)
{
   // Default constructor
   fExogamData    = new TExogamData();
   BoolSpec=bspec;
   Beta=0.;
   for(Int_t i=0;i<16;i++)CloverPresent[i]=false;
   Goccetrack=GOCCEActive=ESSActive=false;
}



TExogam::~TExogam()
{
   delete fExogamData;
}



bool TExogam::Clear()
{
   fExogamData->Clear();
   fECCcopy_val=0;
   
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
   
   return true;
}
bool TExogam::ActivateGOCCE(bool act){
	GOCCEActive=act;
	return true;
}
bool TExogam::ActivateESS(bool act){
	ESSActive=act;
	return true;
}
bool TExogam::IsActivateGOCCE(){
	return GOCCEActive;
}
bool TExogam::IsActivateESS(){
	return ESSActive;
}
bool TExogam::ActivateClover(int i)
{ 
	CloverPresent[i]=true;
	return true;
}
bool TExogam::SetPromptGate(int a, int b){
	promptL=a;
	promptH=b;
		cerr<<"TExogam Info :: Prompt gate is "<<promptL<<"   "<<promptH<<endl;

	return true;
}
bool TExogam::IsCloverActive(int i)
{ 
	return CloverPresent[i];
}
bool TExogam::ActivateGOCCETrack(bool check){

	Goccetrack=check;
	return true;
}
bool TExogam::SetBeta(float beta){
	Beta=beta;
	return true;
}
bool TExogam::SetVamosCorrelId(int clo,int cri){

	Vclo=clo;
	Vcore=cri;
return true;
}
float TExogam::Doppler_Correction(float Theta_Gamma, float Phi_Gamma, float Theta_Part, float Phi_Part, float Beta_Part, float energie_Mes){  //rad, v/c
	float energievraie,cosinusPSI;
			  
		  cosinusPSI =TMath::Sin(Theta_Part)*TMath::Cos(Phi_Part)*TMath::Sin(Theta_Gamma)*TMath::Cos(Phi_Gamma)+
		  	      TMath::Sin(Theta_Part)*TMath::Sin(Phi_Part)*TMath::Sin(Theta_Gamma)*TMath::Sin(Phi_Gamma)+
			      TMath::Cos(Theta_Part)*TMath::Cos(Theta_Gamma);
			      
	energievraie = energie_Mes*(1.-Beta_Part*cosinusPSI)/sqrt(1.-Beta_Part*Beta_Part);

	return energievraie;
};

bool TExogam::SpectraConstructor()
{
if(BoolSpec){
	fMyHistoNCriERaw = new TH2I("NCristal_ERaw","NCristal_ERaw",16*4,0,16*4,8000,0,16000);
	HListExogam.Add(fMyHistoNCriERaw);

	fMyHistoNCriTRaw = new TH2I("NCristal_TRaw","NCristal_TRaw",16*4,0,16*4,8000,0,32000);
	HListExogam.Add(fMyHistoNCriTRaw);
	
	
	fMyHistoNCriECal = new TH2I("NCristal_Ecal","NCristal_Ecal",16*4,0,16*4,1000,0,3000);
	HListExogam.Add(fMyHistoNCriECal);
	
	fMyHistoNCriTCal = new TH2I("NCristal_Tcal","NCristal_Tcal",16*4,0,16*4,4000,0,16000);
	HListExogam.Add(fMyHistoNCriTCal);

	char title[50];
	char cristal[5]={'A','B','C','D','\0'};  

	
	for(int k=0 ; k<16 ; k++){
		if(CloverPresent[k]==false) continue;
		
			sprintf(title,"Clover%d_ECal",k);
			fMyHistoCloverECal[k]= new TH1F(title,title,3000,0,3000); 
			HListExogam.Add(fMyHistoCloverECal[k]);
			
			sprintf(title,"Clover%d_TCal",k);
			fMyHistoCloverTCal[k]= new TH1F(title,title,4000,0,16000); 
			HListExogam.Add(fMyHistoCloverTCal[k]);
			
			sprintf(title,"Clover%d_ECalACAdd",k);
			fMyHistoCloverECalACAdd[k]= new TH1F(title,title,3000,0,3000); 
			HListExogam.Add(fMyHistoCloverECalACAdd[k]);
			
			sprintf(title,"Clover%d_ECalACAddDC_TQ",k);
			fMyHistoCloverECalACAdd_TQ[k]= new TH1F(title,title,3000,0,3000); 
			HListExogam.Add(fMyHistoCloverECalACAdd_TQ[k]);
			
			sprintf(title,"Clover%d_ECalACAdd_TC",k);
			fMyHistoCloverECalACAdd_TC[k]= new TH1F(title,title,3000,0,3000); 
			HListExogam.Add(fMyHistoCloverECalACAdd_TC[k]);
			
			sprintf(title,"Clover%d_ECalACAddRejectF",k);
			fMyHistoCloverECalACAddRejectF[k]= new TH1F(title,title,3000,0,3000); 
			HListExogam.Add(fMyHistoCloverECalACAddRejectF[k]);
			
			sprintf(title,"ESS%d_E",k);
			fMyHistoESS_Energy[k]=new TH1F(title,title,6000,0,6000); 
			HListExogam.Add(fMyHistoESS_Energy[k]);
			
			sprintf(title,"ESS%d_T",k);
			fMyHistoESS_Time[k]=new TH1F(title,title,200,0,200); 
			HListExogam.Add(fMyHistoESS_Time[k]);
			
			sprintf(title,"ESS%d_Pat",k);
			fMyHistoESS_Pat[k]=new TH1F(title,title,200,0,200); 
			HListExogam.Add(fMyHistoESS_Pat[k]);
			
		for(int l=1;l<=4;l++){
		
			sprintf(title,"ECC%d_%c_ECal",k,cristal[l-1]);
			fMyHistoECCECal[k*100+10*(l-1)]= new TH1F(title,title,16000,0,16000); 
			HListExogam.Add(fMyHistoECCECal[k*100+10*(l-1)]);
			
			sprintf(title,"ECC%d_%c_TCal",k,cristal[l-1]);
			fMyHistoECCTCal[k*100+10*(l-1)]= new TH1F(title,title,8000,0,16000); 
			HListExogam.Add(fMyHistoECCTCal[k*100+10*(l-1)]);
			
			sprintf(title,"ESS%d_TQ%d",k,l);
			fMyHistoESS_TimeQ[k*100+10*(l-1)]=new TH1F(title,title,400,0,400); 
			HListExogam.Add(fMyHistoESS_TimeQ[k*100+10*(l-1)]);
			
			sprintf(title,"ESS%d_%c_CoreEAcE",k,cristal[l-1]);
			fMyHistoECCEESSE[k*100+10*(l-1)]=new TH2F(title,title,500,0,2000,500,0,1000);
			HListExogam.Add(fMyHistoECCEESSE[k*100+10*(l-1)]);
			
			sprintf(title,"ECC%d_%c_Vetoed",k,cristal[l-1]);
			fMyHistoECCEVetoed[k*100+10*(l-1)]= new TH1F(title,title,3000,0,3000); 
			HListExogam.Add(fMyHistoECCEVetoed[k*100+10*(l-1)]);
			
			sprintf(title,"ECC%d_%c_Accepted",k,cristal[l-1]);
			fMyHistoECCEAccepted[k*100+10*(l-1)]= new TH1F(title,title,3000,0,3000); 
			HListExogam.Add(fMyHistoECCEAccepted[k*100+10*(l-1)]);
			
			sprintf(title,"ECC%d_%c_ECalACRejectF",k,cristal[l-1]);
			fMyHistoCrysECalACRejectF[k*100+10*(l-1)]= new TH1F(title,title,3000,0,3000); 
			HListExogam.Add(fMyHistoCrysECalACRejectF[k*100+10*(l-1)]);
			
			for(int m=1; m<=4; m++){
				sprintf(title,"GOCCE%d_%c_%d_E",k,cristal[l-1],m);
				fMyHistoGOCCEE[k*100+10*(l-1)+(m-1)]= new TH1F(title,title,8000,0,16000);
				HListExogam.Add(fMyHistoGOCCEE[k*100+10*(l-1)+(m-1)]);
				
				sprintf(title,"GOCCE%d_%c_%d_T",k,cristal[l-1],m);
				fMyHistoGOCCET[k*100+10*(l-1)+(m-1)]= new TH1F(title,title,8000,0,16000);
				HListExogam.Add(fMyHistoGOCCET[k*100+10*(l-1)+(m-1)]);
			}

		}
	}
		
	fMyHistoSumECCE= new TH1F("SumEnergyCoreExogam","SumEnergyCoreExogam",16000,0,16000); 
	HListExogam.Add(fMyHistoSumECCE);
	fMyHistoSumECCT= new TH1F("SumTimeCoreExogam","SumTimeCoreExogam",8000,0,32000); 
	HListExogam.Add(fMyHistoSumECCT);

	fMyHistoSumGOCCEE= new TH1F("SumEnergySegExogam","SumEnergySegExogam",8000,0,16000); 
	HListExogam.Add(fMyHistoSumGOCCEE);
	fMyHistoSumGOCCET= new TH1F("SumTimeSegExogam","SumTimeSegExogam",8000,0,16000); 
	HListExogam.Add(fMyHistoSumGOCCET);
	

	fMyHistoPatternECCE= new TH1F("PatternEnergyCoreExogam","PatternEnergyCoreExogam",200,0,200); 
	HListExogam.Add(fMyHistoPatternECCE);
	fMyHistoPatternECCT= new TH1F("PatternTimeCoreExogam","PatternTimeCoreExogam",2000,0,200); 
	HListExogam.Add(fMyHistoPatternECCT);
	
	
	fMyHistoAngletheta= new TH1F("Theta_Distribution","Theta_Distribution",1500,50,200);
	HListExogam.Add(fMyHistoAngletheta);
        fMyHistoAnglephi= new TH1F("Phi_Distribution","Phi_Distribution",400,-200,200);
	HListExogam.Add(fMyHistoAnglephi);
	
	
	
	fMyHistoSumExogam= new TH1F("ExogamAddB_AC_noDC","ExogamAddB_AC_noDC",5000,0,5000);
	HListExogam.Add(fMyHistoSumExogam);
	fMyHistoSumExogamFoldCond= new TH1F("ExogamAddB_AC_noDC_Fold1","ExogamAddB_AC_noDC_Fold1",5000,0,5000);
	HListExogam.Add(fMyHistoSumExogamFoldCond);
	fMyHistoSumExogamDCFoldCond= new TH1F("ExogamAddB_AC_DC_Fold1","ExogamAddB_AC_DC_Fold1",5000,0,5000);
	HListExogam.Add(fMyHistoSumExogamDCFoldCond);
	
	
	
        fMyHistoSumExogamDC= new TH1F("ExogamAddB_AC_DC","ExogamAddB_AC_DC",5000,0,5000);
	HListExogam.Add(fMyHistoSumExogamDC);
	
	fMyHistoSumExogam2D= new TH2F("ExogamAddB_AC_noDC2D","ExogamAddB_AC_noDC2D",1500,50,200,2000,0,2000);
	HListExogam.Add(fMyHistoSumExogam2D);
        fMyHistoSumExogamDC2D= new TH2F("ExogamAddB_AC_DC2D","ExogamAddB_AC_DC2D",1500,50,200,2000,0,2000);
	HListExogam.Add(fMyHistoSumExogamDC2D);
	
	
	
	
	fMyHistoPatternGOCCEE= new TH1F("PatternEnergySegExogam","PatternEnergySegExogam",1700,0,1700); 
	HListExogam.Add(fMyHistoPatternGOCCEE);
	fMyHistoPatternGOCCET= new TH1F("PatternTimeSegExogam","PatternTimeSegExogam",1700,0,1700); 
	HListExogam.Add(fMyHistoPatternGOCCET);
	
	fMyHistoPatternESSTQ = new TH1F("PatternEssQ","PatternEssQ",170,0,170); 
	HListExogam.Add(fMyHistoPatternESSTQ);
	
	fMyHistoNSegERaw=new TH2I("PatternEnergySegExogam2D","PatternEnergySegExogam2D",1700,0,1700,3000,0,10000); 
	fMyHistoECCcopy_val= new TH2F("CorrelVamosExo","CorrelVamosExo",1000,0,16000,1000,0,16000);


	fMyHistoMultiCrystal= new TH1F("TotalCrystalMultiplicity","TotalCrystalMultiplicity",16*4,0,16*4);
	HListExogam.Add(fMyHistoMultiCrystal);
	fMyHistoMultiCrystalperClover= new TH1F("CrystalMultiplicityPerClover","CrystalMultiplicityPerClover",6,0,6);
	HListExogam.Add(fMyHistoMultiCrystalperClover);
	fMyHistoMultiClover= new TH1F("TotalCloverMultiplicity","TotalCloverMultiplicity",20,0,20);
	HListExogam.Add(fMyHistoMultiClover);
	fMyHistoMultiAntiCompt= new TH1F("TotalAntiComptonMultiplicity","TotalAntiComptonMultiplicity",20,0,20);
	HListExogam.Add(fMyHistoMultiAntiCompt);
	
	fMyHistoGammaGamma= new TH2F("GammaGamma_AC_AD_DC","GammaGamma_AC_AD_DC",2000,0,2000,2000,0,2000);
	HListExogam.Add(fMyHistoGammaGamma);
	fMyHistoSumExogamCalorimeter=new TH1F("ExogamCalorimeterAC","ExogamCalorimeterAC",3000,0,3000);
	HListExogam.Add(fMyHistoSumExogamCalorimeter);
	fMyHistoCheckEss=new TH1F("CheckEss","CheckEss",100,0,100);
	
	
	TailleEvt= new TH1F("NombreDeMot16Bit","NombreDeMot16Bit",1000,0,1000);
	TailleEvt2D= new TH2F("NombreDeMot16Bit2D","NombreDeMot16Bit2D",1000,0,1000,60,0,60);
	TailleEvtNRJ= new TH2F("NombreDeMot16BitNRJ","NombreDeMot16BitNRJ",1000,0,1000,5000,0,20000);
	TailleEvtNRJEss= new TH2F("NombreDeMot16BitNRJEss","NombreDeMot16BitNRJEss",1000,0,1000,5000,0,20000);
	TailleEvtNRJGOCCE= new TH2F("NombreDeMot16BitNRJGOCCE","NombreDeMot16BitNRJGOCCE",1000,0,1000,5000,0,20000);
	HListExogam.Add(TailleEvt);
	HListExogam.Add(TailleEvt2D);
	HListExogam.Add(TailleEvtNRJ);
	HListExogam.Add(TailleEvtNRJEss);
	HListExogam.Add(TailleEvtNRJGOCCE);
	
	TreatD2VBSpec= new TH2F("TreatD2VBSpec","TreatD2VBSpec",50,0,50,3000,0,3000);
	HListExogam.Add(TreatD2VBSpec);
	TreatD2VBSpec2= new TH2F("TreatD2VBSpec2","TreatD2VBSpec2",50,0,50,3000,0,3000);
	HListExogam.Add(TreatD2VBSpec2);
	TreatD2VBSpec3= new TH2F("TreatD2VBSpec3","TreatD2VBSpec3",50,0,50,3000,0,3000);
	HListExogam.Add(TreatD2VBSpec3);
	
	fMyHistoAnglephiD2VB_1=new TH1F("fMyHistoAnglephiD2VB_1","fMyHistoAnglephiD2VB_1",360,-180,180);
	fMyHistoAnglephiD2VB_2=new TH1F("fMyHistoAnglephiD2VB_2","fMyHistoAnglephiD2VB_2",360,-180,180);
	fMyHistoAnglephiD2VB_3=new TH1F("fMyHistoAnglephiD2VB_3","fMyHistoAnglephiD2VB_3",360,-180,180);
	HListExogam.Add(fMyHistoAnglephiD2VB_1);
	HListExogam.Add(fMyHistoAnglephiD2VB_2);	     
	HListExogam.Add(fMyHistoAnglephiD2VB_3);	     
	return true;
}
else {return false;}

}
bool TExogam::InitCal()
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
		
		
    	}
	
	CalibDone=false;
return true;
}	
bool TExogam::ReadCal()
{
  FILE *ecc_cal = fopen("CalFile/ecc.cal","r");   // fichier de calibration ecc= inner=core=cristal
  FILE *gocce_cal = fopen("CalFile/gocce.cal","r");// fichier de calibration gocce= outer=segment

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
	
	
	CalibDone=true;
return true;	
}

double TExogam::Cal(UShort_t en, float offset, float gain, float gain2){
double enc;
	    enc = (double)en+gRandom->Uniform(1.0)-.5;
	    enc = enc*enc*gain2+enc*gain+offset;
	    return enc;
}

bool TExogam::Init(DataParameters *params)
{
   Int_t channum, segnum;
   Int_t Xtalnum = -1;
   string crystal, sigtype;
   bool status = false;
   TString schaine;
   TObjArray* toks=0;
   Char_t labels[30];
   cerr<<"Start"<<endl;
   Int_t nbParams = params->GetNbParameters();
   cerr<<nbParams<<endl;
   for (Int_t index = 0; index < nbParams; index++) {
      Int_t lbl      = params->GetLabel(index);
     // string label   = params->GetParName(index);
      
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
	 if (Xtalnum < 0) cout << "TExogam::Init() : problem with ECC/Xtalnum" << endl;

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
	   cout << "TExogam::Init() : problem reading EXOGAM/ECC label" << sigtype << endl;
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
	 if (Xtalnum < 0) cout << "TExogam::Init() : problem with GOCCE/Xtalnum" << endl;

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
	    cout << "TExogam::Init() : problem reading EXOGAM/GOCCE label" << endl;
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



bool TExogam::Is(UShort_t lbl, Short_t val)
{
   Int_t clo, cri, seg;
   float Thresh;
   double valf;
   bool result = false;
   int  MapFinger;
   switch (fTypeMap[lbl]) {
     case ECCE : {  
       //cout<<  "- ---------< EXOGAM/ECC E >------------------!\n";
       MapFinger=fParameterMap[lbl];
       clo = (Int_t) MapFinger / 4;
       cri = (Int_t) (MapFinger- clo*4) % 4;
       
       if(BoolSpec&&IsCloverActive(clo))fMyHistoNCriERaw->Fill(MapFinger,val);
       if(BoolSpec&&IsCloverActive(clo))fMyHistoPatternECCE->Fill(clo*10+cri);
       
       if(clo==Vclo&&cri==Vcore){fECCcopy_val=val;}
	
       //calibration for online spectra
       valf=Cal(val,ECoef[MapFinger][0],ECoef[MapFinger][1],ECoef[MapFinger][2]); 
       val=Cal(val,ECoef[MapFinger][0],ECoef[MapFinger][1],ECoef[MapFinger][2]); 
       
       if(valf>MaxCrisEval[clo]){
       	MaxCrisEval[clo]=valf;
        MaxCris[clo]=cri;
       }
       if(BoolSpec&&IsCloverActive(clo))fMyHistoECCECal[clo*100+10*cri]->Fill(valf);
       if(BoolSpec&&IsCloverActive(clo))fMyHistoSumECCE->Fill(valf);
       if(BoolSpec&&IsCloverActive(clo))fMyHistoCloverECal[clo]->Fill(valf);
       if(BoolSpec&&IsCloverActive(clo))fMyHistoNCriECal->Fill(MapFinger,valf);
        
       fExogamData->SetECCEClover(clo);
       fExogamData->SetECCECristal(cri);
       fExogamData->SetECCEEnergy(valf);

       result = true;
       break;
     }
    
     case ECCT : {
       //cout<<  " ----------< EXOGAM/ECC T >------------------!\n";
       
       MapFinger=fParameterMap[lbl]; 
       clo = (Int_t) MapFinger / 4;
       cri = (Int_t) (MapFinger- clo*4) % 4;
       //cout << clo << "  " << cri << endl;
       //cout << val << endl;
       
       if(BoolSpec&&IsCloverActive(clo))fMyHistoNCriTRaw->Fill(MapFinger,val);
       if(BoolSpec&&IsCloverActive(clo))fMyHistoPatternECCT->Fill(clo*10+cri);

       val=Cal(val,TCoef[MapFinger][0],TCoef[MapFinger][1],TCoef[MapFinger][2]);
       //cout <<TCoef[ fParameterMap[lbl]][0] << " "<< TCoef[fParameterMap[lbl]][1]<< " "<< TCoef[fParameterMap[lbl]][2]<< endl;
       if(BoolSpec&&IsCloverActive(clo))fMyHistoECCTCal[clo*100+10*cri]->Fill(val);
       if(BoolSpec&&IsCloverActive(clo))fMyHistoSumECCT->Fill(val);
       if(BoolSpec&&IsCloverActive(clo))fMyHistoCloverTCal[clo]->Fill(val);
       if(BoolSpec&&IsCloverActive(clo))fMyHistoNCriTCal->Fill(MapFinger,val);
       fExogamData->SetECCTClover(clo);
       fExogamData->SetECCTCristal(cri);
       fExogamData->SetECCTTime(val);
       result = true;
       break;
       //cout<<  " ----------< EXOGAM/ECC T OUT  >------------------!\n";
     }


     case GOCCEE : {  
       //cout<<  "- ---------< EXOGAM/GOCCE E >------------------!\n";
	
       clo = (Int_t) fParameterMap[lbl]/16;
       cri = (Int_t) (fParameterMap[lbl]- clo*16) / 4;
       seg = (Int_t) (fParameterMap[lbl]- clo*16) % 4;
       fExogamData->SetGOCCEEClover(clo);
       fExogamData->SetGOCCEECristal(cri);
       fExogamData->SetGOCCEESegment(seg);
       fExogamData->SetGOCCEEEnergy(val);
       if(BoolSpec&&IsCloverActive(clo))fMyHistoPatternGOCCEE->Fill(clo*100+10*cri+seg);
       if(BoolSpec&&IsCloverActive(clo))fMyHistoNSegERaw->Fill(clo*100+10*cri+seg,val);
       GOCCE_Pat[clo][cri][seg]=true;
       mulG[clo][cri]=mulG[clo][cri]+1;
       
       if(CalibDone){Thresh=100;}
       else{Thresh=100;}
       if(val>Thresh){
       		mulGnrj[clo][cri]=mulGnrj[clo][cri]+1;
        	GOCCE_Ener[clo][cri][seg]=true;
       }

       val=Cal(val,ECoef_G[fParameterMap[lbl]][0],ECoef_G[fParameterMap[lbl]][1],ECoef_G[fParameterMap[lbl]][2]);

      
       if(val>MaxSegEval[clo][cri]){
       	MaxSegEval[clo][cri]=val;
        MaxSeg[clo][cri]=seg;
       }
       if(BoolSpec&&IsCloverActive(clo))fMyHistoGOCCEE[clo*100+10*cri+seg]->Fill(val);

       if(BoolSpec&&IsCloverActive(clo))fMyHistoSumGOCCEE->Fill(val);
	
       result = true;
       break;
     }
    
     case GOCCET : {	
      // cout<<  " ----------< EXOGAM/GOCCE T >------------------!\n"; 

       clo = (Int_t) fParameterMap[lbl] / 16;
       cri = (Int_t) (fParameterMap[lbl]- clo*16) / 4;
       seg = (Int_t) (fParameterMap[lbl]- clo*16) % 4;
       fExogamData->SetGOCCETClover(clo);
       fExogamData->SetGOCCETCristal(cri);
       fExogamData->SetGOCCETSegment(seg);
       fExogamData->SetGOCCETTime(val);
       if(BoolSpec&&IsCloverActive(clo))fMyHistoGOCCET[clo*100+10*cri+seg]->Fill(val);
       if(BoolSpec&&IsCloverActive(clo))fMyHistoSumGOCCET->Fill(val);
       if(BoolSpec&&IsCloverActive(clo))fMyHistoPatternGOCCET->Fill(clo*100+10*cri+seg);

       result = true;;
       break;
     }
     
     
     case ESSE : {
     	clo = (Int_t) fParameterMap[lbl];
	fExogamData->SetESSEClover(clo);
	fExogamData->SetESSEEnergy(val);
	if(BoolSpec&&IsCloverActive(clo))fMyHistoESS_Energy[clo]->Fill(val);
	result = true;
	//NoCompton[clo]=false;
        break;
     }
     case ESST : {
     	clo = (Int_t) fParameterMap[lbl];
	fExogamData->SetESSTClover(clo);
	fExogamData->SetESSTTime(val);
	if(BoolSpec&&IsCloverActive(clo))fMyHistoESS_Time[clo]->Fill(val);
	NoCompton[clo]=false;
	result = true;
        break;
     }
     case ESSTQ : {
     	clo = (Int_t) fParameterMap[lbl] /16;
	cri = (Int_t) (fParameterMap[lbl]- clo*16);
	fExogamData->SetESSTQClover(clo);
	fExogamData->SetESSTQCristal(cri);
	fExogamData->SetESSTQTime(val);
	if(BoolSpec&&IsCloverActive(clo))fMyHistoESS_TimeQ[clo*100+10*cri]->Fill(val);
	if(BoolSpec&&IsCloverActive(clo))fMyHistoPatternESSTQ->Fill(clo*10+cri);
	//NoCompton[clo]=false;
	if(val>4)NoComptonCore[clo][cri]=false;
	result = true;
        break;
     }
     case ESSPat : {
     	clo = (Int_t) fParameterMap[lbl];
	if(BoolSpec&&IsCloverActive(clo))fMyHistoESS_Pat[clo]->Fill((val&0x003F)+10);
	//NoCompton[clo]=false;
	result = true;
        break;
     }
     
    
     default : {
       result = false;
       break;
     }
   }

   return result;
}

/*bool TExogam::IsMFMExo(MFMExogamFrame *frame)
{
	return false;
}*/
bool TExogam::Treat()
{
int clo,cri,cloESS,cloverMul;
char cristal[5]={'A','B','C','D','\0'};
float EnergyAddTQ[16],EnergyAddTC[16],EnergyAdd[16],mulCrysPerClover[16],EnergyAddTQDC[16];
float Theta_Gamma, Phi_Gamma;
bool IsPrompt[16][4];
float SumCalorimeter;

SumCalorimeter=0;
	//------------------Time Treat
	for(Int_t c=0;c<16;c++){
		for(Int_t d=0;d<4;d++){
			IsPrompt[c][d]=false;
		}
	}


	for (UShort_t i = 0; i < fExogamData->GetECCTMult(); i++) {
		clo=fExogamData->GetECCTClover(i);
		cri=fExogamData->GetECCTCristal(i);
		if(fExogamData->GetECCTTime(i)>=promptL&&fExogamData->GetECCTTime(i)<=promptH)IsPrompt[clo][cri]=true;
	}
	
	
	//------------------Energy Treat
	
	for(Int_t c=0;c<16;c++){EnergyAddTQ[c]=EnergyAddTC[c]=EnergyAdd[c]=mulCrysPerClover[c]=EnergyAddTQDC[c]=0.;}

	cloverMul=0;
	for (UShort_t i = 0; i < fExogamData->GetECCEMult(); i++) {
		clo=fExogamData->GetECCEClover(i);
		cri=fExogamData->GetECCECristal(i);
		//cerr<<clo<<" IN "<<cri<<endl;
	        
		if(IsCloverActive(clo)==false){continue;}
		
		mulCrysPerClover[clo]++;
		mulECC[clo][cri]++;
		if(mulG[clo][cri]==4)mulECCG[clo][cri]++;
		if(mulGnrj[clo][cri]>0)mulECCGnrj[clo][cri]++;
		//cerr<<mulGnrj[clo][cri]<<endl;
//------------------------------------------------------------------------------------------------------------------HERE the prompt cond!!!!!!!!!!!!!!!!!!
		if(IsPrompt[clo][cri]==false)continue;
		
		if(NoComptonCore[clo][cri]==false){
			NoComptonCoreCounter[clo][cri]++;
			fMyHistoECCEVetoed[clo*100+10*cri]->Fill(fExogamData->GetECCEEnergy(i));
		}
		else{
			fMyHistoECCEAccepted[clo*100+10*cri]->Fill(fExogamData->GetECCEEnergy(i));
			EnergyAddTQ[clo]=EnergyAddTQ[clo]+fExogamData->GetECCEEnergy(i);
		}
		
		if(NoCompton[clo]&&IsPrompt[clo][cri]){EnergyAddTC[clo]=EnergyAddTC[clo]+fExogamData->GetECCEEnergy(i);}
		
		//here is the AddBack
		if(fExogamData->GetECCEEnergy(i)>10)EnergyAdd[clo]=EnergyAdd[clo]+fExogamData->GetECCEEnergy(i); 
		
		
		//---------------------------------------------------------
		for(Int_t k =0 ; k<4;k++){
			if(Goccetrack&&GOCCE_Pat[clo][cri][k]==false)printf("\033[31m GOCCE %d Crys %c Seg %d No Data   \033[m \n",clo,cristal[cri],k);
			if(Goccetrack&&GOCCE_Ener[clo][cri][k]==false)printf("\033[31m GOCCE %d Crys %c Seg %d Only Pedestal   \033[m \n",clo,cristal[cri],k);
		}
		for(UShort_t j = 0; j < fExogamData->GetESSEMult(); j++){
			cloESS=fExogamData->GetESSEClover(j);
			if(cloESS==clo)fMyHistoECCEESSE[clo*100+10*cri]->Fill(fExogamData->GetECCEEnergy(i),fExogamData->GetESSEEnergy(j));
		}
		//---------------------------------------------------------

   	}
	fMyHistoMultiCrystal->Fill(fExogamData->GetECCEMult());
	fMyHistoMultiAntiCompt->Fill(fExogamData->GetESSEMult());

	//------------------Final Treat
	for(Int_t c=0;c<16;c++){ //determine the true Fold
		if(CloverPresent[c]==false) continue;
		
		if(mulCrysPerClover[c]>0){
			fMyHistoMultiCrystalperClover->Fill(mulCrysPerClover[c]);
			cloverMul++;
		}

	}
	
	
	for(Int_t c=0;c<16;c++){
		
		if(CloverPresent[c]==false) continue;
		
		
		if(EnergyAddTQ[c]>0){
			fMyHistoSumExogam->Fill(EnergyAddTQ[c]);
			SumCalorimeter=SumCalorimeter+EnergyAddTQ[c];
			//Fold condition
			if(cloverMul==1)fMyHistoSumExogamFoldCond->Fill(EnergyAddTQ[c]);
			//----------------

			if(MaxSeg[c][MaxCris[c]]<1000){ //if Max Seg is different than the Init Value
				Theta_Gamma=GammaAngleSegTheta[c][MaxCris[c]][MaxSeg[c][MaxCris[c]]];
				Phi_Gamma=GammaAngleSegPhi[c][MaxCris[c]][MaxSeg[c][MaxCris[c]]];
				//cout<<c <<"I am using the Seg for DC"<<endl;
			}
			else{
				Theta_Gamma=GammaAngleCoreTheta[c][MaxCris[c]];
				Phi_Gamma=GammaAngleCorePhi[c][MaxCris[c]];
				if(IsActivateGOCCE())printf("\033[31m I am using the core for DC for this clover ********* \033[m \n");
				continue;
			}
			//cerr<<"Do DC "<<Theta_Gamma*180./3.14159<<" ::  "<<Beta<<endl;
			fMyHistoSumExogamDC->Fill(Doppler_Correction(Theta_Gamma,0.0,0., 0., Beta, EnergyAddTQ[c]));

			//Fold condition
			if(cloverMul==1)fMyHistoSumExogamDCFoldCond->Fill(Doppler_Correction(Theta_Gamma,0.0,0., 0., Beta, EnergyAddTQ[c]));
			//----------------
			
			
			fMyHistoAngletheta->Fill(Theta_Gamma*180./3.14159);
			fMyHistoAnglephi->Fill(Phi_Gamma*180./3.14159);
			fMyHistoCloverECalACAdd_TQ[c]->Fill(Doppler_Correction(Theta_Gamma,Phi_Gamma,0., 0., Beta, EnergyAddTQ[c]));
			
			EnergyAddTQDC[c]=Doppler_Correction(Theta_Gamma,Phi_Gamma,0., 0., Beta, EnergyAddTQ[c]);
			
			fMyHistoSumExogam2D->Fill(Theta_Gamma*180./3.14159,EnergyAddTQ[c]);
			fMyHistoSumExogamDC2D->Fill(Theta_Gamma*180./3.14159,Doppler_Correction(Theta_Gamma,Phi_Gamma,0., 0., Beta, EnergyAddTQ[c]));
			
			
		}
		if(EnergyAddTC[c]>0){fMyHistoCloverECalACAdd_TC[c]->Fill(EnergyAddTC[c]);}
		if(EnergyAdd[c]>0)  {fMyHistoCloverECalACAdd[c]->Fill(EnergyAdd[c]);}
		
	}
	
	fMyHistoSumExogamCalorimeter->Fill(SumCalorimeter);
	
	//create gamma-gamma matrix
	if(cloverMul>1){
		for(Int_t c=0;c<16;c++){
			if(CloverPresent[c]==false) continue;
			for(Int_t d=0;d<16;d++){
				if(CloverPresent[d]==false) continue;
				if(c!=d)fMyHistoGammaGamma->Fill(EnergyAddTQDC[c],EnergyAddTQDC[d]); // with DC
			}
		}
	}
	
	fMyHistoMultiClover->Fill(cloverMul);

   return true;
}
bool TExogam::CounterReset()
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
bool TExogam::Counter()
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
						printf("\033[31m ECC%dCrys%c Missing GOCCE Ratio (Gated Word/Raw) = %2.3f percent \033[m \n",i, cristal[j],div);
						printf("\033[32m ECC%dCrys%c Missing GOCCE Ratio (Gated Nrj/Raw) = %2.3f percent \033[m \n",i, cristal[j],(1-1.*mulECCGnrj[i][j]/mulECC[i][j])*100.);
			        		GOCCEfailure1=true;
					}
			 		else if(div3>10.){
						
						printf("\033[32m ECC%dCrys%c Missing GOCCE Ratio (Gated Word/Raw) = %2.3f percent \033[m \n",i, cristal[j],div);
						printf("\033[31m ECC%dCrys%c Missing GOCCE Ratio (Gated Nrj/Raw) = %2.3f percent \033[m \n",i, cristal[j],(1-1.*mulECCGnrj[i][j]/mulECC[i][j])*100.);
						GOCCEfailure2=true;
					}
					else{
						printf("\033[32m ECC%dCrys%c Missing GOCCE Ratio (Gated Word/Raw) = %2.3f percent \033[m \n",i, cristal[j],div);
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
						fMyHistoCheckEss->Fill((1.*NoComptonCoreCounter[i][j]/mulECC[i][j])*100.);
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
	
	
return true;
}

bool TExogam::ACReject()
{
Int_t bin1,bin2,c1,c2;
Float_t div;

	for(Int_t x=0;x<16;x++){
	  if(CloverPresent[x]==false) continue;
		for(Int_t a =1 ; a <3000;a++){
			bin1=fMyHistoCloverECalACAdd[x]->GetBin(a);
			bin2=fMyHistoCloverECalACAdd_TQ[x]->GetBin(a);
			c1=fMyHistoCloverECalACAdd[x]->GetBinContent(bin1);
			c2=fMyHistoCloverECalACAdd_TQ[x]->GetBinContent(bin2);
			if(c2>0){div=10.*c1/c2;}
			else {div=0.;}
			fMyHistoCloverECalACAddRejectF[x]->Fill(a,(int)(div));
		}
		
		for(Int_t g = 0 ; g <4 ; g++){
			for(Int_t a =1 ; a <3000;a++){
				bin1=fMyHistoECCECal[100*x+10*g]->GetBin(a);
				bin2=fMyHistoECCEAccepted[100*x+10*g]->GetBin(a);
				c1=fMyHistoECCECal[100*x+10*g]->GetBinContent(bin1);
				c2=fMyHistoECCEAccepted[100*x+10*g]->GetBinContent(bin2);
				if(c2>0){div=10.*c1/c2;}
				else {div=0.;}
				fMyHistoCrysECalACRejectF[100*x+10*g]->Fill(a,(int)(div));
			}
		}		
	}
	
	
	
	
return true;
}
bool TExogam::SetCloverPosition(int ECC_VXInb,int flange, float distance)
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

float EXOGAM_Crystal_Center; 
float EXOGAM_Segment_Pos1,EXOGAM_Segment_Pos2 ;

	if(InteractionDepth>=30){EXOGAM_Crystal_Center= 24.5;EXOGAM_Segment_Pos1=39.; EXOGAM_Segment_Pos2=12.25; }
	else{EXOGAM_Crystal_Center=ShapeC->Eval(InteractionDepth);
	     EXOGAM_Segment_Pos1=ShapeS1->Eval(InteractionDepth); //gd
	     EXOGAM_Segment_Pos2=ShapeS2->Eval(InteractionDepth); //pt
	}
	


	//Crystal A
	flange12Crist[0].SetY(flange12.Y()+(EXOGAM_Crystal_Center));
	flange12Crist[0].SetZ(flange12.Z()-(EXOGAM_Crystal_Center));
	flange12Crist[0].SetX(Real_position);
		//segment1
	flange12CristSeg[0][0].SetY(flange12.Y()+(EXOGAM_Segment_Pos1)); //gd
	flange12CristSeg[0][0].SetZ(flange12.Z()-(EXOGAM_Segment_Pos1)); //gd
	flange12CristSeg[0][0].SetX(Real_position);
		//segment2
	flange12CristSeg[0][1].SetY(flange12.Y()+(EXOGAM_Segment_Pos2));
	flange12CristSeg[0][1].SetZ(flange12.Z()-(EXOGAM_Segment_Pos1));
	flange12CristSeg[0][1].SetX(Real_position);
		//segment3
	flange12CristSeg[0][2].SetY(flange12.Y()+(EXOGAM_Segment_Pos2));
	flange12CristSeg[0][2].SetZ(flange12.Z()-(EXOGAM_Segment_Pos2));
	flange12CristSeg[0][2].SetX(Real_position);
		//segment4
	flange12CristSeg[0][3].SetY(flange12.Y()+(EXOGAM_Segment_Pos1));
	flange12CristSeg[0][3].SetZ(flange12.Z()-(EXOGAM_Segment_Pos2));
	flange12CristSeg[0][3].SetX(Real_position);

	//Crystal B
	flange12Crist[1].SetY(flange12.Y()-(EXOGAM_Crystal_Center));
	flange12Crist[1].SetZ(flange12.Z()-(EXOGAM_Crystal_Center));
	flange12Crist[1].SetX(Real_position);
		//segment1
	flange12CristSeg[1][0].SetY(flange12.Y()-(EXOGAM_Segment_Pos1));
	flange12CristSeg[1][0].SetZ(flange12.Z()-(EXOGAM_Segment_Pos1));
	flange12CristSeg[1][0].SetX(Real_position);
		//segment2
	flange12CristSeg[1][1].SetY(flange12.Y()-(EXOGAM_Segment_Pos1));
	flange12CristSeg[1][1].SetZ(flange12.Z()-(EXOGAM_Segment_Pos2));
	flange12CristSeg[1][1].SetX(Real_position);
		//segment3
	flange12CristSeg[1][2].SetY(flange12.Y()-(EXOGAM_Segment_Pos2));
	flange12CristSeg[1][2].SetZ(flange12.Z()-(EXOGAM_Segment_Pos2));
	flange12CristSeg[1][2].SetX(Real_position);
		//segment4
	flange12CristSeg[1][3].SetY(flange12.Y()-(EXOGAM_Segment_Pos2));
	flange12CristSeg[1][3].SetZ(flange12.Z()-(EXOGAM_Segment_Pos1));
	flange12CristSeg[1][3].SetX(Real_position);
	
	//Crystal C
	flange12Crist[2].SetY(flange12.Y()-(EXOGAM_Crystal_Center));
	flange12Crist[2].SetZ(flange12.Z()+(EXOGAM_Crystal_Center));
	flange12Crist[2].SetX(Real_position);
		//segment1
	flange12CristSeg[2][0].SetY(flange12.Y()-(EXOGAM_Segment_Pos1));
	flange12CristSeg[2][0].SetZ(flange12.Z()+(EXOGAM_Segment_Pos1));
	flange12CristSeg[2][0].SetX(Real_position);
		//segment2
	flange12CristSeg[2][1].SetY(flange12.Y()-(EXOGAM_Segment_Pos2));
	flange12CristSeg[2][1].SetZ(flange12.Z()+(EXOGAM_Segment_Pos1));
	flange12CristSeg[2][1].SetX(Real_position);
		//segment3
	flange12CristSeg[2][2].SetY(flange12.Y()-(EXOGAM_Segment_Pos2));
	flange12CristSeg[2][2].SetZ(flange12.Z()+(EXOGAM_Segment_Pos2));
	flange12CristSeg[2][2].SetX(Real_position);
		//segment4
	flange12CristSeg[2][3].SetY(flange12.Y()-(EXOGAM_Segment_Pos1));
	flange12CristSeg[2][3].SetZ(flange12.Z()+(EXOGAM_Segment_Pos2));
	flange12CristSeg[2][3].SetX(Real_position);
	
	//Crystal D
	flange12Crist[3].SetY(flange12.Y()+(EXOGAM_Crystal_Center));
	flange12Crist[3].SetZ(flange12.Z()+(EXOGAM_Crystal_Center));
	flange12Crist[3].SetX(Real_position);
		//segment1
	flange12CristSeg[3][0].SetY(flange12.Y()+(EXOGAM_Segment_Pos1));
	flange12CristSeg[3][0].SetZ(flange12.Z()+(EXOGAM_Segment_Pos1));
	flange12CristSeg[3][0].SetX(Real_position);
		//segment2
	flange12CristSeg[3][1].SetY(flange12.Y()+(EXOGAM_Segment_Pos1));
	flange12CristSeg[3][1].SetZ(flange12.Z()+(EXOGAM_Segment_Pos2));
	flange12CristSeg[3][1].SetX(Real_position);
		//segment3
	flange12CristSeg[3][2].SetY(flange12.Y()+(EXOGAM_Segment_Pos2));
	flange12CristSeg[3][2].SetZ(flange12.Z()+(EXOGAM_Segment_Pos2));
	flange12CristSeg[3][2].SetX(Real_position);
		//segment4
	flange12CristSeg[3][3].SetY(flange12.Y()+(EXOGAM_Segment_Pos2));
	flange12CristSeg[3][3].SetZ(flange12.Z()+(EXOGAM_Segment_Pos1));
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
		
		// Take into account the case of a target which is not at the center of the EXOGAM array
                // Target position
                TVector3 targetPos(TARGET_POSITION_X, TARGET_POSITION_Y, TARGET_POSITION_Z);

		// case of the selected EXOGAM detector
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
			Result.X_Crystal[i]=flange12Crist[i].X();
			Result.Y_Crystal[i]=flange12Crist[i].Y();
			Result.Z_Crystal[i]=flange12Crist[i].Z();
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
	
	else {printf("Bad flange number, flange %d  doesn't exist in your EXOGAM config : ECC%d!!!!!! \n",flange,ECC_VXInb);}


return true;
}
bool TExogam::CheckCoreResolution(float EnergyC)
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
bool TExogam::D2VBTreat(int sizeEvnt)
{
float Limit, sum1, Limit2;

	sum1=0.;
	Limit = 200.;
	Limit2 = 100.;

    TailleEvt->Fill(sizeEvnt);
    TailleEvt2D->Fill(sizeEvnt,fExogamData->GetECCEMult());
    
    for (UShort_t i = 0; i < fExogamData->GetECCEMult(); i++) {
    	TailleEvtNRJ->Fill(sizeEvnt,fExogamData->GetECCEEnergy(i));
	sum1=sum1+fExogamData->GetECCEEnergy(i);
    }
    
    for (UShort_t i = 0; i < fExogamData->GetESSEMult(); i++) {
    	TailleEvtNRJEss->Fill(sizeEvnt,fExogamData->GetESSEEnergy(i));
    }

    for (UShort_t i = 0; i < fExogamData->GetGOCCEEMult(); i++) {
    	TailleEvtNRJGOCCE->Fill(sizeEvnt,fExogamData->GetGOCCEEEnergy(i));
    }

//create gamma-gamma matrix
/*	if(fExogamData->GetECCEMult()>1&&sizeEvnt<Limit2){
		for(Int_t c=0;c<fExogamData->GetECCEMult();c++){
			for(Int_t d=0;d<fExogamData->GetECCEMult();d++){
				if(fExogamData->GetECCECristal(c)!=fExogamData->GetECCECristal(d))fMyHistoGammaGamma->Fill(fExogamData->GetECCEEnergy(c),fExogamData->GetECCEEnergy(d)); // with DC
			}
		}
	}
	
*/


    for (UShort_t i = 0; i < fExogamData->GetECCEMult(); i++) {
	if(sizeEvnt>Limit){
		TreatD2VBSpec->Fill(fExogamData->GetECCEMult(),fExogamData->GetECCEEnergy(i));
		if(fExogamData->GetECCEEnergy(i)>=84&&fExogamData->GetECCEEnergy(i)<=90)fMyHistoAnglephiD2VB_1->Fill(GammaAngleCorePhi[fExogamData->GetECCEClover(i)][fExogamData->GetECCECristal(i)]*180./3.14159);
	}
	
	else if(sizeEvnt>Limit2&&sizeEvnt<Limit){
		TreatD2VBSpec2->Fill(fExogamData->GetECCEMult(),fExogamData->GetECCEEnergy(i));
		if(fExogamData->GetECCEEnergy(i)>=84&&fExogamData->GetECCEEnergy(i)<=90)fMyHistoAnglephiD2VB_2->Fill(GammaAngleCorePhi[fExogamData->GetECCEClover(i)][fExogamData->GetECCECristal(i)]*180./3.14159);
	}
	else if(sizeEvnt<Limit2){
		TreatD2VBSpec3->Fill(fExogamData->GetECCEMult(),fExogamData->GetECCEEnergy(i));
		if(fExogamData->GetECCEEnergy(i)>=84&&fExogamData->GetECCEEnergy(i)<=90)fMyHistoAnglephiD2VB_3->Fill(GammaAngleCorePhi[fExogamData->GetECCEClover(i)][fExogamData->GetECCECristal(i)]*180./3.14159);
	}
	
    }
	




return true;
}

void TExogam::InitBranch(TTree *tree)
{
   tree->Branch("EXOGAM", "TExogamData", &fExogamData);
}

