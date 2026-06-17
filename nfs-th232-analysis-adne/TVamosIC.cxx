#include <cstdlib>
#include "fstream"
#include "TVamosIC.h"
#include "TVamosICData.h"
#include <TCutG.h>
#include <iostream>
#include "stdio.h"
#include "string.h"
#include "TFile.h"
#include "TMath.h"
#include <MFMVamosICFrame.h>

ClassImp(TVamosIC)


/*

VamosIC Class for one numexo2 REA having 16 channels in IC firmware. For multiple REA, construct as many VamosIC type class as needed; offset in TVamosIC::InitNumexo2()

*/
TVamosIC::TVamosIC(bool bspec)
{
  // Default constructor
  fVamosICData    = new TVamosICData();
  BoolSpec=bspec;
  NombreIons=0;  
  duplicatedEventC=prevBoard=prevChannel=tag=prevQ=prevQ2=0;
  CutLoaded=false;
  LUTBool=false;
  debug=false;
  
  
  PrevTS=0;
  if(debug)printf("\033[35m********************VamosIC  Info :  debug level**********\033[m \n");
}



TVamosIC::~TVamosIC()
{
  delete fVamosICData;
}



bool TVamosIC::Clear()
{
  fVamosICData->Clear();
  FreeParam1=FreeParam2=-1.;
  return true;
}
bool TVamosIC::InitCal()
{
	
	
  return true;
}
bool TVamosIC::ReadCut()
{
printf("\033[35mVamosIC  Info :  Reading Cut\033[m \n");
	
	
	
	return true;
	
	
	
}
bool TVamosIC::SpectraConstructor()
{
  char title[100];
  if(BoolSpec){
    printf("\033[35mVamosIC  Info :  Start Spectra constructors\033[m \n");
    for(int k=0 ; k<1000 ; k++){
      sprintf(title,"VamosIC_Energy%d",k);
      fMyHistoVamosICE[k]= new TH1F(title,title,16384,0,16384); 
      HListVamosIC.Add(fMyHistoVamosICE[k]);
		
     
    }
    fMyHistoVamosICE2D= new TH2F("VamosICE2D","VamosICE2D",1000,0,1000,16384,0,16384); 
    HListVamosIC.Add(fMyHistoVamosICE2D);
    
    fMyHistoVamosICECorrel = new TH2F("fMyHistoVamosICECorrel","fMyHistoVamosICECorrel",200,0,200,200,0,200); 
    HListVamosIC.Add(fMyHistoVamosICECorrel);

    printf("\033[35m ----> Done \033[m \n");

   
	
    return true;
  }
  else{return false;}
}
bool TVamosIC::ReadCal()
{
	/*FILE *ecc_cal = fopen("CalFile/VamosIC.cal","r");   // fichier de calibration 80 first are QShort and 80 next are QLong and 80 next are ThetaLab  and PhiLab in deg
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
	*/
	
	
  return true;
}

double TVamosIC::Cal(UShort_t en, float offset, float gain, float gain2){
  double enc; 
  enc = (double)en+gRandom->Uniform(1.0)-.5;
  enc = enc*enc*gain2+enc*gain+offset;
	   
  return enc;
}
double TVamosIC::CalI(int en, float offset, float gain, float gain2){
  double enc; 
  enc = (double)en+gRandom->Uniform(1.0)-.5;
  enc = enc*enc*gain2+enc*gain+offset;
	   
  return enc;
}


bool TVamosIC::Init(DataParameters *params)
{
  bool status = false;
  
		
  return status; 
   
}

bool TVamosIC::InitNumexo2(Char_t *fileNumexo2){


  LUTBool=false;
	
  TString schainetrack;
  TObjArray* toks=0;
  Int_t board,Celloffset, ClusterId;

  for(Int_t i= 0 ; i<255;i++){Current_Numexo2_cfg_Board[i]=Channel_in_Cluster[i]=-1;}
	
  ifstream inf_f(fileNumexo2);
  if(inf_f.good()==false) {
    printf("\033[31m Error VamosIC:: Not a valid Look Up Table=> I complain here   \033[m \n"); 
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
	//cout<<board<<endl;
	Channel_in_Cluster[board]=ClusterId;
	Current_Numexo2_cfg_Board[board]=Celloffset;
	delete toks;
      }
    }
	
  }
  inf_f.close();

   
	
  cout<<"=>Look Up Table Numexo2 VamosIC read"<<endl;
	
  return true;
}



bool TVamosIC::IsMFMVamosIC(MFMVamosICFrame *frame)
{ 
  bool result = false;
  int CristalId, MapFinger, Board ;
  double valf;
  bool DuplicatedEvent=false;	
  Board=CristalId=MapFinger=-10;
  int localDTS;
  int Q;
  Q=localDTS=0;
  
  if(debug)cerr<<"Enter in VamosIC"<<endl;
  if(LUTBool==false){
    printf("\033[31m Error in TVamosIC::IsMFMVamosIC => The Look Up Table is not known: I won't be able to unpack the event   \033[m \n"); 
  }
  else{
     if(debug)cerr<<"--> Get Inside VamosIC Frame :: "<<endl;
     if(debug)cerr<<" (*) GetChannelId :: "<<frame->GetChannelId()<<endl;
     if(debug)cerr<<" (*) GetCristal   :: "<<frame->GetTGCristalId()<<endl;
     if(debug)cerr<<" (*) GetBoard     :: "<<frame->GetBoardId()<<endl;
     if(debug)cerr<<" (*) FrameSize    :: "<<frame->GetFrameSize()<<endl;
     if(debug)cerr<<" (*) Energy is    :: "<<frame->GetEnergy()<<endl;
     if(debug)cerr<<" (*) Time Stamp is :: "<<frame->GetTimeStamp()<<endl;
    
    CristalId=frame->GetChannelId(); //return cell ID
    Board=frame->GetBoardId();

    MapFinger=CristalId+Current_Numexo2_cfg_Board[Board];
    if(debug)cerr<<" (*) MapFinger::  "<<MapFinger<<endl;
    
    
    if(CristalId>=0&&Current_Numexo2_cfg_Board[Board]>=0){
      result=true;
    }
    else {
      //printf("\033[31m Error in TVamosIC::IsMFMVamosIC => This IP (%d) is not known from the Look Up Table   \033[m \n",Board); 
      result=false;
    }
  }
  
  
  
  
  if(Board==prevBoard&&CristalId==prevChannel&&frame->GetTimeStamp()==PrevTS&&prevQ==Q){
		//cerr<<"Duplicated evt number :: Prev = "<<prevEvtNumber<<"  Current = "<<frame->GetEventNumber()<<endl;
		DuplicatedEvent=true;
		duplicatedEventC++;
  }	
  if(frame->GetTimeStamp()==0)cerr<<"TVamosIC :: MapFinger : CristalId : Board = "<<MapFinger<<" : " <<CristalId<<" : "<<Board<<endl;

  if(result&&DuplicatedEvent==false){
  
    if(debug)cerr<<" (*) Push Data "<<endl; 
    //Push Raw Data
    fVamosICData->SetVamosICType(Channel_in_Cluster[Board]);
    //Calibrate
    valf = frame->GetEnergy()*0.1;
    fVamosICData->SetVamosICDetectorNbr(MapFinger);
    fVamosICData->SetVamosICEnergy(valf);
    fVamosICData->SetfTimeStamps(frame->GetTimeStamp(),MapFinger);
    if(debug)cerr<<" (*) Push Data completed "<<endl;
   
    if(debug&&BoolSpec)cerr<<" (*) Fill spectra "<<endl;
    if(BoolSpec)fMyHistoVamosICE[MapFinger]->Fill(valf);
    if(BoolSpec)fMyHistoVamosICE2D->Fill(MapFinger,valf);
    localDTS=(int)frame->GetTimeStamp()-(int)PrevTS;
    if(debug&&BoolSpec)cerr<<" (*) Fill spectra done"<<endl;
    
  
  }

  return result;
}

bool TVamosIC::Is(UShort_t lbl, Short_t val)
{

  bool status = false;
 
  return status;
}



bool TVamosIC::Treat()
{
 G_TS=0;
 Muliplicity=0;;
 	for(Int_t i =0 ; i<fVamosICData->GetVamosICMult() ;i++){
		//if(fVamosICData->GetVamosICDet(i)<40 && fVamosICData->GetVamosICDet(i)!=15){
		if(fVamosICData->GetVamosICDet(i)<200){
			if(abs(fVamosICData->GetVamosICEnergy(i)-3000)<300){
				Muliplicity++;
				G_TS = fVamosICData->GetfTimeStamps(fVamosICData->GetVamosICDet(i));
			}
		}
 	}
	
	if(fVamosICData->GetVamosICMult()>1){
		for(Int_t i =0 ; i<fVamosICData->GetVamosICMult() ;i++){
			for(Int_t j =0 ; j<fVamosICData->GetVamosICMult() ;j++){
				if(i!=j&&BoolSpec)fMyHistoVamosICECorrel->Fill(fVamosICData->GetVamosICDet(i),fVamosICData->GetVamosICDet(j));
			}
		}
	
	
	}
 
  return true;
}

bool TVamosIC::Counter()
{

 
  return true;

}

bool TVamosIC::ClearCounter()
{
	
  return true;

}

void TVamosIC::InitBranch(TTree *tree)
{
  tree->Branch("VamosIC", "TVamosICData", &fVamosICData,32000,99);
}


