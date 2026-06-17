#include <cstdlib>
#include "fstream"
#include "TGeneric.h"
#include "TGenericData.h"
#include <TCutG.h>
#include <iostream>
#include "stdio.h"
#include "string.h"
#include "TFile.h"
#include "TMath.h"
#include <MFMReaGenericFrame.h>

ClassImp(TGeneric)


/*

Generic Class for one numexo2 REA having 16 channels. For multiple REA, construct as many Generic type class as needed; offset in TGeneric::InitNumexo2()

*/
TGeneric::TGeneric(bool bspec, int Topology)
{
  // Default constructor
  fGenericData    = new TGenericData();
  BoolSpec=bspec;
  NombreIons=0;  
  duplicatedEventC=prevBoard=prevChannel=tag=prevQ=prevQ2=0;
  CutLoaded=false;
  LUTBool=false;
  debug=false;
  
  
  PrevTS=0;
  if(debug)printf("\033[35m********************Generic  Info :  debug level**********\033[m \n");
}



TGeneric::~TGeneric()
{
  delete fGenericData;
}



bool TGeneric::Clear()
{
  fGenericData->Clear();
  FreeParam1=FreeParam2=-1.;
  return true;
}
bool TGeneric::InitCal()
{
	for(Int_t x=0;x<1000;x++){
    		ECoef[x][0]=0.;
		ECoef[x][1]=1.;
        	ECoef[x][2]=0.;

    	}
	CalibDone=false;
	for(Int_t x=0;x<100;x++){
		gcut_Array[x]=NULL;
		TS_IONS_Start[x]=TS_IONS_Stop[x]=IonsCounter[x]=0;
	}
	
	
	
  return true;
}
bool TGeneric::ReadCut()
{
printf("\033[35mGeneric  Info :  Reading Cut\033[m \n");
char Cname[50];
	
	
	//sprintf(Cname, "./CutDiamant/putain.root");//RUN 638-679 CHIO  36Si setting
	//sprintf(Cname, "./CutDiamant/putain38Si.root");//RUN  CHIO  38Si setting
	//sprintf(Cname, "./CutDiamant/sorlinCHIO.root"); // RUN S36
	//sprintf(Cname, "./CutDiamant/sorlinCHIO34Si.root"); // RUN Si34 on Au
	sprintf(Cname, "./CutDiamant/sorlinCHIO34Si-C.root"); // RUN Si34 on C
	cout<<1<<endl;
	if (fopen(Cname, "r") != NULL) {
		TFile fileCut(Cname, "READ");
		/*gcut_Array[0]=(TCutG*) fileCut.Get("S40");
		gcut_Array[1]=(TCutG*) fileCut.Get("P38");
		gcut_Array[2]=(TCutG*) fileCut.Get("Si36");
		gcut_Array[3]=(TCutG*) fileCut.Get("Al34");
		gcut_Array[4]=(TCutG*) fileCut.Get("S39");
		gcut_Array[5]=(TCutG*) fileCut.Get("P37");
		gcut_Array[6]=(TCutG*) fileCut.Get("Si35");
		gcut_Array[7]=(TCutG*) fileCut.Get("Al33");
		*/
		
		/*gcut_Array[0]=(TCutG*) fileCut.Get("S42");
		gcut_Array[1]=(TCutG*) fileCut.Get("P40");
		gcut_Array[2]=(TCutG*) fileCut.Get("Si38");
		gcut_Array[3]=(TCutG*) fileCut.Get("Al36");
		gcut_Array[4]=(TCutG*) fileCut.Get("S43"); //41 i, fact
		gcut_Array[5]=(TCutG*) fileCut.Get("P39");
		gcut_Array[6]=(TCutG*) fileCut.Get("Si37");
		gcut_Array[7]=(TCutG*) fileCut.Get("Al35");
		*/
		//gcut_Array[0]=(TCutG*) fileCut.Get("S36");
		gcut_Array[0]=(TCutG*) fileCut.Get("Si34");
		/*gcut_Array[1]=(TCutG*) fileCut.Get("Si34");
		gcut_Array[2]=(TCutG*) fileCut.Get("Si34");
		gcut_Array[3]=(TCutG*) fileCut.Get("Si34");
		gcut_Array[4]=(TCutG*) fileCut.Get("Si34");
		gcut_Array[5]=(TCutG*) fileCut.Get("Si34");
		gcut_Array[6]=(TCutG*) fileCut.Get("Si34");
		gcut_Array[7]=(TCutG*) fileCut.Get("Si34");
		gcut_Array[8]=(TCutG*) fileCut.Get("Si34");*/

		
		//NombreIons=9;
		
		fileCut.Close();
		CutLoaded=true;
	}
	else {
		cout << "Error:: Could not read generic cut 1" << endl;
        }
	
	//sprintf(Cname, "./CutDiamant/putain2.root");//RUN 638-679 DSSD 36Si setting
	//sprintf(Cname, "./CutDiamant/putain2Si38.root");//RUN  DSSD 38Si setting
	//sprintf(Cname, "./CutDiamant/sorlinDSSD.root");	// RUN S36
	//sprintf(Cname, "./CutDiamant/sorlinDSSD34Si.root");	// RUN Si34 until run 1000

	//sprintf(Cname, "./CutDiamant/sorlinDSSD34SiMILL.root");	// RUN Si34 from run 1000
	sprintf(Cname, "./CutDiamant/commMUST2_2023_M.root");	// RUN Si34 on Carbon target

	if (fopen(Cname, "r") != NULL) {
		TFile fileCut2(Cname, "READ");
		gcut_Array[10]=(TCutG*) fileCut2.Get("Ni58");
		gcut_Array[11]=(TCutG*) fileCut2.Get("Ni57");
		fileCut2.Close();
		CutLoaded=true;
	}
	else {
		cout << "Error:: Could not read generic cut 2" << endl;
        }
	
	/* sprintf(Cname, "./CutDiamant/sorlinCHIO34Si-C-30Mg.root");	// RUN Si34 on Carbon target + DSSD = 30Mg

	if (fopen(Cname, "r") != NULL) {
		TFile fileCut3(Cname, "READ");
		gcut_Array[18]=(TCutG*) fileCut3.Get("Mg30");
		fileCut3.Close();
		CutLoaded=true;
	}
	else {
		cout << "Error:: Could not read generic cut" << endl;
        } */
	
	 sprintf(Cname, "./CutDiamant/commMUST2_2023.root");	// RUN Si34 on Carbon target + DSSD = 30Mg

	if (fopen(Cname, "r") != NULL) {
		TFile fileCut3(Cname, "READ");
		gcut_Array[20]=(TCutG*) fileCut3.Get("Ni");
		gcut_Array[21]=(TCutG*) fileCut3.Get("Co");
		fileCut3.Close();
		NombreIons=2;
		CutLoaded=true;
	}
	else {
		cout << "Error:: Could not read generic cut 3" << endl;
        } 
	
	return true;
	
	
	
}



bool TGeneric::SpectraConstructor()
{
  char title[100];
  if(BoolSpec){
    printf("\033[35mGeneric  Info :  Start Spectra constructors\033[m \n");
    for(int k=0 ; k<1000 ; k++){
      sprintf(title,"Generic_Energy%d",k);
      fMyHistoGenericE[k]= new TH1F(title,title,16384,0,16384); 
      HListGeneric.Add(fMyHistoGenericE[k]);
      
      sprintf(title,"Generic_TDC%d",k);
      fMyHistoGenericT[k]= new TH1F(title,title,16384,0,16384); 
      HListGeneric.Add(fMyHistoGenericT[k]);
		
     
    }
    
    for(Int_t i =0 ; i<100;i++){
  	Ic[i]=0;
	sprintf(Cname, "Rate_%d",i);
	trackI[i] = new TH1F(Cname,Cname,100,0,100);
	HListGeneric.Add(trackI[i]);

  }
    
    
    fMyHistoGenericDEE=new TH2F("fMyHistoGenericDEE","fMyHistoGenericDEE",4000,0,20000,1000,0,5000);
    fMyHistoGenericDEECond=new TH2F("fMyHistoGenericDEECond","fMyHistoGenericDEECond",4000,0,20000,1000,0,5000);
    fMyHistoGenericDE=new TH1F("fMyHistoGenericDE","fMyHistoGenericDE",5000,0,5000);
    fMyHistoGenericDETOF=new TH2F("fMyHistoGenericDETOF","fMyHistoGenericDETOF",5000,1,16384*4,5000,1,16384*2);
    fMyHistoGenericDETOF_LISE=new TH2F("fMyHistoGenericDETOF_LISE","fMyHistoGenericDETOF_LISE",1000,1,16384,1000,1,6000);
    fMyHistoGenericDETOF_LISEcond=new TH2F("fMyHistoGenericDETOF_LISEcond","fMyHistoGenericDETOF_LISEcond",1000,1,16384*4,1000,1,6000);
    
    fMyHistoGenericMul=new TH1F("fMyHistoGenericMul","fMyHistoGenericMul",100,0,100);
    fMyHistoGenericPattE=new TH2F("fMyHistoGenericPattE","fMyHistoGenericPattE",400,0,400,2048,0,16384*4);
    fMyHistoGenericPattT=new TH2F("fMyHistoGenericPattT","fMyHistoGenericPattT",400,0,400,2048,0,16384*4);
    
    TimeStampDiff= new TH1F("TimeStampDiff","TimeStampDiff",10000,-5000,5000); 
    
    TransverseMomentum= new TH1F("TransverseMomentum","TransverseMomentum",200,-100,100);
    
    fMyHistoGenericTT = new TH1F("fMyHistoGenericTT","fMyHistoGenericTT",1000,-500,500);
    
    fMyHistoGenericEE = new TH2F("fMyHistoGenericEE","fMyHistoGenericEE",5000,1,16384*4,5000,1,16384*2);
    
    
    
    
    HListGeneric.Add(fMyHistoGenericDEE);
    HListGeneric.Add(fMyHistoGenericDETOF);
    HListGeneric.Add(fMyHistoGenericDETOF_LISE);
    HListGeneric.Add(fMyHistoGenericDETOF_LISEcond);
    HListGeneric.Add(fMyHistoGenericMul);
    HListGeneric.Add(fMyHistoGenericPattE);
    HListGeneric.Add(fMyHistoGenericPattT);
    HListGeneric.Add(fMyHistoGenericDEECond);
    HListGeneric.Add(TransverseMomentum);
    HListGeneric.Add(fMyHistoGenericDE);
    HListGeneric.Add(fMyHistoGenericTT);
    HListGeneric.Add(fMyHistoGenericEE);
    
    printf("\033[35m ----> Done \033[m \n");

    
    

	
    return true;
  }
  else{return false;}
}
bool TGeneric::ReadCal()
{
	FILE *ecc_cal = fopen("CalFile/generic.cal","r");   // fichier de calibration 80 first are QShort and 80 next are QLong and 80 next are ThetaLab  and PhiLab in deg
	Int_t x;
	float a,b,c;

	
	for(x=0;x<200;x++){
		fscanf(ecc_cal,"%f %f %f\n",&a,&b,&c);
    		ECoef[x][0]=a;
		ECoef[x][1]=b;
        	ECoef[x][2]=c;
	//cout <<a << " "<<b << " "<<c <<"  "<<x<<endl;
    	}
	
	CalibDone=true;
	
	
	
  return true;
}

double TGeneric::Cal(UShort_t en, float offset, float gain, float gain2){
  double enc; 
  enc = (double)en+gRandom->Uniform(1.0)-.5;
  enc = enc*enc*gain2+enc*gain+offset;
	   
  return enc;
}
double TGeneric::CalI(int en, float offset, float gain, float gain2){
  double enc; 
  enc = (double)en+gRandom->Uniform(1.0)-.5;
  enc = enc*enc*gain2+enc*gain+offset;
	   
  return enc;
}


bool TGeneric::Init(DataParameters *params)
{
  bool status = false;
  
		
  return status; 
   
}

bool TGeneric::InitNumexo2(Char_t *fileNumexo2){


  LUTBool=false;
	
  TString schainetrack;
  TObjArray* toks=0;
  Int_t board,Celloffset, ClusterId;

  for(Int_t i= 0 ; i<255;i++){Current_Numexo2_cfg_Board[i]=Channel_in_Cluster[i]=-1;}
	
  ifstream inf_f(fileNumexo2);
  if(inf_f.good()==false) {
    printf("\033[31m Error Exogam2:: Not a valid Look Up Table=> I complain here   \033[m \n"); 
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
	board = ((TObjString* )toks->At(0))->GetString().Atoi();
	Celloffset= ((TObjString* )toks->At(1))->GetString().Atoi();
	ClusterId= ((TObjString* )toks->At(2))->GetString().Atoi();
	
	Channel_in_Cluster[board]=ClusterId;
	Current_Numexo2_cfg_Board[board]=Celloffset;
	delete toks;
      }
    }
	
  }
  inf_f.close();

   
	
  cout<<"=>Look Up Table Numexo2 Generic read"<<endl;
	
  return true;
}



bool TGeneric::IsMFMGeneric(MFMReaGenericFrame *frame)
{ 
  bool result = false;
  int CristalId, MapFinger, Board ;
  double valf;
  bool DuplicatedEvent=false;	
  Board=CristalId=MapFinger=-10;
  int localDTS;
  int Q,T;
  Q=T=0;
  
  
  
  if(debug)cerr<<"Enter in Generic"<<endl;
  if(LUTBool==false){
    printf("\033[31m Error in TGeneric::IsMFMGeneric => The Look Up Table is not known: I won't be able to unpack the event   \033[m \n"); 
  }
  else{
     if(debug)cerr<<"--> Get Inside Generic Frame :: "<<endl;
     if(debug)cerr<<" (*) GetChannelId :: "<<frame->GetChannelId()<<endl;
     if(debug)cerr<<" (*) GetCristal   :: "<<frame->GetTGCristalId()<<endl;
     if(debug)cerr<<" (*) GetBoard     :: "<<frame->GetBoardId()<<endl;
     if(debug)cerr<<" (*) FrameSize    :: "<<frame->GetFrameSize()<<endl;
     if(debug)cerr<<" (*) Energy is    :: "<<frame->GetEnergy()<<endl;
     if(debug)cerr<<" (*) Time Stamp is :: "<<frame->GetTimeStamp()<<endl;
     if(debug)cerr<<" (*) TypeTns is :: "<<frame->GetTypeTns()<<endl;
    
    CristalId=frame->GetChannelId(); //return cell ID
    Board=frame->GetBoardId();

    MapFinger=CristalId+Current_Numexo2_cfg_Board[Board];
    if(debug)cerr<<" (*) MapFinger::  "<<MapFinger<<endl;
    
    
    if(CristalId>=0&&Current_Numexo2_cfg_Board[Board]>=0){
      result=true;
    }
    else {
      //printf("\033[31m Error in TGeneric::IsMFMGeneric => This IP (%d) is not known from the Look Up Table   \033[m \n",Board); 
      result=false;
    }
  }
  
  /*if(frame->GetTypeTns() == REA_GENERIC_TIME_TYPE){
  	Q=frame->GetTime();
  }
  else{
  	Q=frame->GetEnergy();
  }
  */
  
  switch(frame->GetTypeTns()){
  	case REA_GENERIC_ENERGY_TYPE:
		Q=frame->GetEnergy();
		T=0;
		break;
	
	case REA_GENERIC_CHARGE_TYPE:
		Q=frame->GetEnergy();
		T=0;
		break;
		
	case REA_GENERIC_ENERGY_TIME_TYPE:
		Q=frame->GetEnergy();
		T=frame->GetTime();
		break;
		
	case REA_GENERIC_TIME_TYPE:
		Q=0;
		T=frame->GetTime();
		break;
		
  	default : 
		cerr<<"TGeneric ::IsMFMGeneric unknown tns type #"<<frame->GetTypeTns()<<" Board:"<<Board<<endl;
		break;
  }
  if(Board==prevBoard&&CristalId==prevChannel&&frame->GetTimeStamp()==PrevTS&&prevQ==Q){
		//cerr<<"Duplicated evt number :: Prev = "<<prevEvtNumber<<"  Current = "<<frame->GetEventNumber()<<endl;
		DuplicatedEvent=true;
		duplicatedEventC++;
  }	
  if(frame->GetTimeStamp()==0)cerr<<"TGeneric :: MapFinger : CristalId : Board = "<<MapFinger<<" : " <<CristalId<<" : "<<Board<<endl;

  if(result&&DuplicatedEvent==false&&Q>0){
  
    if(debug)cerr<<" (*) Push Data "<<endl; 
    
    
    //Push Raw Data
    fGenericData->SetGenericType(Channel_in_Cluster[Board]);
    
    //Calibrate
    valf = Cal(Q,ECoef[MapFinger][0],ECoef[MapFinger][1],ECoef[MapFinger][2]);
    if(Channel_in_Cluster[Board]==1)valf=valf*1.0;
    fGenericData->SetGenericDetectorNbr(MapFinger);
    fGenericData->SetGenericEnergy(valf);
    fGenericData->SetfTimeStamps(frame->GetTimeStamp(),MapFinger);
    fGenericData->SetGenericTime(T);

    
    if(debug)cerr<<" (*) Push Data completed "<<endl;
      
   
    if(debug&&BoolSpec)cerr<<" (*) Fill spectra "<<endl;
    if(BoolSpec)fMyHistoGenericE[MapFinger]->Fill(valf);
    if(BoolSpec)fMyHistoGenericT[MapFinger]->Fill(T);
    if(BoolSpec)fMyHistoGenericPattE->Fill(MapFinger,valf);
    if(BoolSpec)fMyHistoGenericPattT->Fill(MapFinger,T);
    localDTS=(int)frame->GetTimeStamp()-(int)PrevTS;
    if(BoolSpec)TimeStampDiff->Fill(localDTS);
    if(debug)cerr<<" (*)   --DT "<<PrevTS<<" -  "<< frame->GetTimeStamp()<< " = " <<localDTS<<endl;
    if(debug&&BoolSpec)cerr<<" (*) Fill spectra done"<<endl;
    
    
    /*if(debug)cerr<<" (*) Setting Prev event"<<endl;
    prevBoard=Board;
    prevChannel=CristalId;
    prevQ=frame->GetEnergy();
    if(debug)cerr<<" (*) Is equal :  "<<frame->GetTimeStamp()<<"  //  " <<fGenericData->GetfTimeStamps(MapFinger)<<endl;
    if(debug)cerr<<" (*) Prev event is Board "<< prevBoard  << "  |  channel "<<  prevChannel <<" prevTS  " <<  PrevTS  <<"  | for energy "<< prevQ <<   "  at TS = "<< fGenericData->GetfTimeStamps(MapFinger)<< " DTS = "<< localDTS<< endl;
   // if(localDTS<0)printf("\033[31m*********  Time Stamp Error :: TGeneric back in future !!! ********* \033[m \n");
    PrevTS= fGenericData->GetfTimeStamps(MapFinger);
  */
  
  
  }

  return result;
}

bool TGeneric::Is(UShort_t lbl, Short_t val)
{

  bool status = false;
 
  return status;
}



bool TGeneric::Treat()
{
  int GenericMulitiplicity=0, DTS;
  float DE, E, TOF, DELISE;
  float LocalMap;
  unsigned long long TS1,TS2;
  MuliplicitySec=MultiplicityChio=MultiplicityPlastic=DE=E=TOF=0;
  TrueIons=PrompIons=false;
  G_TS=G_TS2=0;
  DE=E=TOF=DELISE=0;
  NucleusId=-1.;
   //fGenericData->Dump();
  LocalMap=-1;
 	//cout<<"Next Event"<<endl;
  for(Int_t i =0 ; i<fGenericData->GetGenericMult(); i++){
  
  	//if(fGenericData->GetGenericEnergy(i)>0){
	if(fGenericData->GetGenericType(i)<4){
		GenericMulitiplicity++;
		
		//cout<<"In Treat Generic "<<fGenericData->GetGenericType(i)<<endl;
		//cout<<fGenericData->GetfTimeStamps(fGenericData->GetGenericDet(i))<<endl;
		if(fGenericData->GetGenericType(i)==0){//OPSA
			MuliplicitySec++;
			//DE=fGenericData->GetGenericEnergy(i);
			G_TS=fGenericData->GetfTimeStamps(fGenericData->GetGenericDet(i));
		}
		else if(fGenericData->GetGenericType(i)==1){ //ICZDD offset is 32
			
			
			if(fGenericData->GetGenericDet(i)-32==0){ //Only IC0
				DE=fGenericData->GetGenericEnergy(i)+DE;
				MultiplicityChio++;
			}
			//cout<<DE/10<<endl;
			G_TS2=fGenericData->GetfTimeStamps(fGenericData->GetGenericDet(i));
			//Transverse Momentum
			LocalMap=fGenericData->GetGenericDet(i);
			
			
			
		}
		else if(fGenericData->GetGenericType(i)==3){ //ZDD plastic central is 2 and 7; offset is 0
			
			if(fGenericData->GetGenericDet(i)==2 || fGenericData->GetGenericDet(i)==7){ //exc central plastique 
				E=fGenericData->GetGenericEnergy(i);
				if(abs(E-30e3)<4e3)MultiplicityPlastic++;
				G_TS=fGenericData->GetfTimeStamps(fGenericData->GetGenericDet(i));
			}
			
		}
		
		//Studies for SMART
		 for(Int_t j =0 ; j<fGenericData->GetGenericMult(); j++){
		 	if(j!=i){
			
				TS1=fGenericData->GetfTimeStamps(fGenericData->GetGenericDet(i));
				TS2=fGenericData->GetfTimeStamps(fGenericData->GetGenericDet(j));
				DTS=TS1-TS2;
				//cout<<fGenericData->GetfTimeStamps(fGenericData->GetGenericDet(i))<<"  "<<fGenericData->GetfTimeStamps(fGenericData->GetGenericDet(j))<<"   "<<DTS<<endl;
				//cout<<DTS<<endl;
				if(BoolSpec)fMyHistoGenericTT->Fill(DTS);
				if(BoolSpec)fMyHistoGenericEE->Fill(fGenericData->GetGenericEnergy(i),fGenericData->GetGenericEnergy(j));
			}
		 
		 }
		
		
	}
	if(fGenericData->GetGenericType(i)==4&&fGenericData->GetGenericDet(i)-48==2){//TAC_CATS_Pl  offset is 48
		if(abs(fGenericData->GetGenericEnergy(i)-9945)<25)PrompIons=true;
		TOF=fGenericData->GetGenericEnergy(i);
	} 

	if(fGenericData->GetGenericType(i)==4&&fGenericData->GetGenericDet(i)-48==8){//TAC CATS1 D4
		//TOF=fGenericData->GetGenericEnergy(i);
	} 
	 
	
	
	
  }
  //cout<<G_TS2<<"  "<<G_TS<<endl;
  FreeParam1=LocalMap;
  
  if(BoolSpec)TransverseMomentum->Fill(FreeParam1);
 // if(DE>10000&&DE<12000&&E>15000&&E<16200){ //scattered beam
  if(BoolSpec&&MultiplicityChio>0)fMyHistoGenericDEE->Fill(E,DE);
  if(BoolSpec&&MultiplicityChio>0)fMyHistoGenericDE->Fill(DE);
  //if(BoolSpec)fMyHistoGenericDETOF->Fill(TOF,DE);
  if(BoolSpec)fMyHistoGenericMul->Fill(GenericMulitiplicity);
  if(BoolSpec)fMyHistoGenericDETOF_LISE->Fill(TOF,E/1.0e1);
  		/*gcut_Array[20]=(TCutG*) fileCut3.Get("Ni"); 
		gcut_Array[21]=(TCutG*) fileCut3.Get("Co");
		gcut_Array[10]=(TCutG*) fileCut2.Get("Ni58");
		gcut_Array[11]=(TCutG*) fileCut2.Get("Ni57");  
		*/
  if(CutLoaded&&DE>0&&E>0&&MultiplicityChio>0){
	for(int cc =0 ; cc<NombreIons;cc++){
		if(gcut_Array[cc+20]->IsInside(E,DE)){ //putain
				/*NucleusId=cc;
				if(BoolSpec)fMyHistoGenericDETOF->Fill(TOF,DE);
				if(BoolSpec)fMyHistoGenericDEECond->Fill(E,DE);*/
				if(cc==0&&gcut_Array[10]->IsInside(TOF,DE)){//Ni58
					if(BoolSpec)fMyHistoGenericDETOF->Fill(TOF,DE);
					if(BoolSpec)fMyHistoGenericDEECond->Fill(E,DE);
					NucleusId=0;
				}
				
				else if(cc==0&&gcut_Array[11]->IsInside(TOF,DE)){//Ni57
					if(BoolSpec)fMyHistoGenericDETOF->Fill(TOF,DE);
					if(BoolSpec)fMyHistoGenericDEECond->Fill(E,DE);
					NucleusId=1;
				}
				else if(cc==1){ //Co
					//if(BoolSpec)fMyHistoGenericDEECond->Fill(E,DE);
					if(BoolSpec)fMyHistoGenericDETOF->Fill(TOF,DE);
					NucleusId=2;
				} 	
				TrueIons=true;
		}
	}
  }
 
  return true;
}

bool TGeneric::Counter()
{
float rate;

  for(int cc =0 ; cc<NombreIons;cc++){
  	if(IonsCounter[cc]>0){
		rate = 1.*IonsCounter[cc]/((float)(TS_IONS_Stop[cc]-TS_IONS_Start[cc])/1e8);
  	        trackI[cc]->Fill(Ic[cc],rate);
		Ic[cc]++;
		if(Ic[cc]>99)Ic[cc]=0;

		cout<<"Ions Id :: "<<cc<<"   "<<IonsCounter[cc]<<" Delta TS "<<TS_IONS_Stop[cc]-TS_IONS_Start[cc] <<"  Rate  "<< rate <<" pps "<<endl;
	}
  }
  return true;

}

bool TGeneric::ClearCounter()
{
	for(int cc =0 ; cc<NombreIons;cc++){
  		TS_IONS_Start[cc]=TS_IONS_Stop[cc]=IonsCounter[cc]=0;
	}

  return true;

}

void TGeneric::InitBranch(TTree *tree)
{
  tree->Branch("Generic", "TGenericData", &fGenericData,32000,99);
}


