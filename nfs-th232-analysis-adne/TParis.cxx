#include <cstdlib>
#include "fstream"
#include "TParis.h"
#include "TParisData.h"
#include <TCutG.h>
#include <iostream>
#include "stdio.h"
#include "string.h"
#include "TFile.h"
#include "TMath.h"
#include <MFMParisFrame.h>

ClassImp(TParis)

TParis::TParis(bool bspec)
{
  // Default constructor
  fParisData    = new TParisData();
  BoolSpec=bspec;
  duplicatedEventC=prevBoard=prevChannel=tag=prevQ=prevQ2=0;
  EventNoTref=MergerCounter=EventWithTref=0;
  LUTBool=false;
  debug=false;
  PrevTS=0;
  Beta=0.; 
  TrefError=0;
  for(Int_t i=0;i<5;i++){
  	for(Int_t j = 0 ; j<16;j++){
		MapFinger[i][j]=-1;
	}
  }
  if(debug)printf("\033[35m********************Paris  Info :  debug level**********\033[m \n");
}



TParis::~TParis()
{
  delete fParisData;
}



bool TParis::Clear()
{
  fParisData->Clear();
  for(Int_t i=0;i<9;i++){ClusterAddB[i]=ClusterCalo[i]=0;}

  return true;
}
bool TParis::InitCal()
{
	for(Int_t x=0;x<80;x++){
    		ECoefQLong[x][0]=ECoefQShort[x][0]=ECoefQNaI[x][0]=0.;
		ECoefQLong[x][1]=ECoefQShort[x][1]=ECoefQNaI[x][1]=1.;
        	ECoefQLong[x][2]=ECoefQShort[x][2]=ECoefQNaI[x][2]=0.;
		TCfD[x]=0.;
		Paris_Angle[x][0]=90.*TMath::DegToRad();
		Paris_Angle[x][1]=0.*TMath::DegToRad();
		v1[x].SetX(0);
		v1[x].SetY(0);
		v1[x].SetZ(0);
    	}
	CalibDone=false;
	
	
	
	
	
	
  return true;
}

bool TParis::SpectraConstructor()
{
  char title[100];
  if(BoolSpec){
    printf("\033[35mParis  Info :  Start Spectra constructors\033[m \n");
    for(int k=0 ; k<80 ; k++){
      sprintf(title,"Paris_QShort%d",k);
      fMyHistoParisQShort[k]= new TH1F(title,title,2048,0,16384); 
      HListParis.Add(fMyHistoParisQShort[k]);
		
      sprintf(title,"Paris_QLong%d",k);
      fMyHistoParisQLong[k]= new TH1F(title,title,2048,0,16384); 
      HListParis.Add(fMyHistoParisQLong[k]);
	
      sprintf(title,"Paris_Cfd%d",k);
      fMyHistoParisCfd[k]= new TH1F(title,title,1000,0,10); 
      //HListParis.Add(fMyHistoParisCfd[k]);
	
      sprintf(title,"Paris_DeltaTS%d",k);	
      IndvTimeStampDiffTref[k]= new TH1F(title,title,10000,-1000,1000); 
      //HListParis.Add(IndvTimeStampDiffTref[k]);
      
      
      
      sprintf(title,"Paris_QShortQLong%d",k);
      fMyHistoParisQShortQLong[k]= new TH2F(title,title,2048,0,16384,2048,0,16384); 
      //HListParis.Add(fMyHistoParisQShortQLong[k]);
      
      sprintf(title,"Paris_QShortQLongCond%d",k);
      fMyHistoParisQShortQLongCond[k]= new TH2F(title,title,2048,0,8192,2048,0,8192);
      //HListParis.Add(fMyHistoParisQShortQLongCond[k]);
      
      sprintf(title,"Paris_Compton%d",k);
      fMyHistoParisCompton[k]= new TH2F(title,title,2048,0,8192,2048,0,8192);
      //HListParis.Add(fMyHistoParisCompton[k]);
      
      sprintf(title,"Paris_QLong_Compton%d",k);
      fMyHistoParisQLongCompton[k]= new TH1F(title,title,2048,0,16384); 
     // HListParis.Add(fMyHistoParisQLongCompton[k]);
      
      sprintf(title,"Paris_QShortCond%d",k);
      fMyHistoParisQLaBr3[k]= new TH1F(title,title,2048,0,8192); 
      HListParis.Add(fMyHistoParisQLaBr3[k]);
      
      sprintf(title,"Paris_QLongCond%d",k);
      fMyHistoParisQNaI[k]= new TH1F(title,title,2048,0,8192); 
      //HListParis.Add(fMyHistoParisQNaI[k]);
    }
    
    fMyHistoParisAddBack=new TH1F("Paris_AddBack","Paris_AddBack",2048,0,8192);
    HListParis.Add(fMyHistoParisAddBack);
    
    
    fMyHistoParisCalo=new TH1F("fMyHistoParisCalo","fMyHistoParisCalo",2048,0,8192);
    HListParis.Add(fMyHistoParisCalo);
    
    
    fMyHistoParisComptonRec=new TH1F("fMyHistoParisComptonRec","fMyHistoParisComptonRec",2048,0,8192);
    HListParis.Add(fMyHistoParisComptonRec);
    
    
    fMyHistoParisMerit=new TH2F("fMyHistoParisMerit","fMyHistoParisMerit",2048,0,8192,1000,-1.5,1.5);
    HListParis.Add(fMyHistoParisMerit);
    
    
    fMyHistoParisMul= new TH1F("fMyHistoParisMul","fMyHistoParisMul",100,0,100); 
    HListParis.Add(fMyHistoParisMul);
    
    fMyHistoParisPattQShort2d=new TH2F("fMyHistoParisPattQShort2d","fMyHistoParisPattQShort2d",100,0,100,2048,0,8192);
    fMyHistoParisPattQLong2d=new TH2F("fMyHistoParisPattQLong2d","fMyHistoParisPattQLong2d",100,0,100,2048,0,8192);
    fMyHistoParisPattCfd2d=new TH2F("fMyHistoParisPattCfd2d","fMyHistoParisPattCfd2d",100,0,100,1000,0,100);
    NeutronPattern=new TH2F("NeutronPattern","NeutronPattern",100,0,100,500,0,2000);
    
    
    HListParis.Add(fMyHistoParisPattQShort2d);
    HListParis.Add(fMyHistoParisPattQLong2d);
    HListParis.Add(fMyHistoParisPattCfd2d);
    HListParis.Add(NeutronPattern);
    
    
    
    TimeStampDiffQLaBr3= new TH1F("TimeStampDiffQLaBr3","TimeStampDiffQLaBr3",10000,-500,500); 
    HListParis.Add(TimeStampDiffQLaBr3);
    TimeStampDiffQLaBr3Eg= new TH2F("TimeStampDiffQLaBr3Eg","TimeStampDiffQLaBr3Eg",1000,1,4000,2000,-10,100); 
    HListParis.Add(TimeStampDiffQLaBr3Eg);
    TimeStampDiffQNaI= new TH1F("TimeStampDiffQNaI","TimeStampDiffQNaI",10000,-500,500); 
    HListParis.Add(TimeStampDiffQNaI);
    
    TimeStampDiffAll= new TH1F("TimeStampDiffAll","TimeStampDiffAll",10000,-500,500); 
    HListParis.Add(TimeStampDiffAll);
    
    TimeStampDiffTref= new TH1F("TimeStampDiffTref","TimeStampDiffTref",4000,-1000,1000); 
    HListParis.Add(TimeStampDiffTref);
    
    TimeStampDiffTrefPattern= new TH2F("TimeStampDiffTrefPattern","TimeStampDiffTrefPattern",80,0,80,5000,-1000,1000); 
    HListParis.Add(TimeStampDiffTrefPattern);
    
    TimeStampDiffTrefEg= new TH2F("TimeStampDiffTrefEg","TimeStampDiffTrefEg",1000,1,4000,2000,-1000,1000); 
    HListParis.Add(TimeStampDiffTrefEg);
    
    fMyHistoParisQLaBr3Sum= new TH1F("fMyHistoParisQLaBr3Sum","fMyHistoParisQLaBr3Sum",2048,0,8192); 
    HListParis.Add(fMyHistoParisQLaBr3Sum);
    fMyHistoParisQLaBr3SumDC= new TH1F("fMyHistoParisQLaBr3SumDC","fMyHistoParisQLaBr3SumDC",2048,0,8192);
    HListParis.Add(fMyHistoParisQLaBr3SumDC);
    
    fMyHistoParisQShortGG= new TH2F("fMyHistoParisQShortGG","fMyHistoParisQShortGG",2048,0,8192,2048,0,8192); 
    fMyHistoParisQShortGGTimeGated= new TH2F("fMyHistoParisQShortGGTimeGated","fMyHistoParisQShortGGTimeGated",2048,0,8192,2048,0,8192); 
    fMyHistoParisCaloFold= new TH2F("fMyHistoParisTASFold","fMyHistoParisTASFold",2048,0,16384,100,0,100); 
    HListParis.Add(fMyHistoParisQShortGG);
    HListParis.Add(fMyHistoParisQShortGGTimeGated);
    HListParis.Add(fMyHistoParisCaloFold);
    
    
    fMyHistoParisThetaPhi= new TH2F("fMyHistoParisThetaPhi","fMyHistoParisThetaPhi",100,60,120,100,-180,180);
    HListParis.Add(fMyHistoParisThetaPhi);
    
    printf("\033[35m ----> Done \033[m \n");

	

	
    return true;
  }
  else{return false;}
}
bool TParis::ReadCal()
{
	FILE *ecc_cal = fopen("CalFile/paris.cal","r");   // fichier de calibration 80 first are QShort and 80 next are QLong and 80 next are time alignement then 80 coef for NaI photopeak
	Int_t x;
	float a,b,c;

	
	for(x=0;x<80;x++){
		fscanf(ecc_cal,"%f %f %f\n",&a,&b,&c); 
    		ECoefQShort[x][0]=a;
		ECoefQShort[x][1]=b;
        	ECoefQShort[x][2]=c;
	//cout <<a << " "<<b << " "<<c <<"  "<<x<<endl;
    	}
	for(x=0;x<80;x++){
		fscanf(ecc_cal,"%f %f %f\n",&a,&b,&c);
    		ECoefQLong[x][0]=a;
		ECoefQLong[x][1]=b;
        	ECoefQLong[x][2]=c;
	//cout <<a << " "<<b << " "<<c <<"  "<<x<<endl;
    	}
	CalibDone=true;
	
	
	for(x=0;x<80;x++){
		fscanf(ecc_cal,"%f\n",&a);
		TCfD[x]=a;
		//cout<<"Paris CfD offset :: "<<a<< endl;
	}
	
	for(x=0;x<80;x++){
		fscanf(ecc_cal,"%f %f %f\n",&a,&b,&c);
    		ECoefQNaI[x][0]=a;
		ECoefQNaI[x][1]=b;
        	ECoefQNaI[x][2]=c;
	//cout <<a << " "<<b << " "<<c <<"  "<<x<<endl;
    	}
	
	
  return true;
}

double TParis::Cal(UShort_t en, float offset, float gain, float gain2){
  double enc; 
  enc = (double)en+gRandom->Uniform(1.0)-.5;
  enc = enc*enc*gain2+enc*gain+offset;
	   
  return enc;
}
double TParis::CalI(int en, float offset, float gain, float gain2){
  double enc; 
  enc = (double)en+gRandom->Uniform(1.0)-.5;
  enc = enc*enc*gain2+enc*gain+offset;
	   
  return enc;
}
float TParis::Doppler_Correction(float Theta_Gamma, float Phi_Gamma, float Theta_Part, float Phi_Part, float Beta_Part, float energie_Mes){  //rad, v/c
	float energievraie,cosinusPSI;
			  
		  cosinusPSI =TMath::Sin(Theta_Part)*TMath::Cos(Phi_Part)*TMath::Sin(Theta_Gamma)*TMath::Cos(Phi_Gamma)+
		  	      TMath::Sin(Theta_Part)*TMath::Sin(Phi_Part)*TMath::Sin(Theta_Gamma)*TMath::Sin(Phi_Gamma)+
			      TMath::Cos(Theta_Part)*TMath::Cos(Theta_Gamma);
			      
	energievraie = energie_Mes*(1.-Beta_Part*cosinusPSI)/sqrt(1.-Beta_Part*Beta_Part);

	return energievraie;
};
bool TParis::SetBeta(float beta){
	Beta=beta;
	return true;
}
bool TParis::Init(DataParameters *params)
{
  bool status = false;
  
		
  return status; 
   
}

bool TParis::InitNumexo2(Char_t *fileNumexo2){


  LUTBool=false;
  	
  TString schainetrack;
  TObjArray* toks=0;
  Int_t board,channel, ClusterId, DetId;
  //char stop[2];

  for(Int_t i= 0 ; i<255;i++){Current_Numexo2_cfg_Board[i]=Channel_in_Cluster[i]=-1;}
	
  ifstream inf_f(fileNumexo2);
  if(inf_f.good()==false) {
    printf("\033[31m Error:: Not a valid Look Up Table=> I complain here here   \033[m \n"); 
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
	ClusterId = ((TObjString* )toks->At(0))->GetString().Atoi();
	DetId	  =((TObjString* )toks->At(1))->GetString().Atoi();
	board = ((TObjString* )toks->At(2))->GetString().Atoi();
	channel = ((TObjString* )toks->At(3))->GetString().Atoi();
	
	if(ClusterId==99&&DetId==99){//T REF detected
		MapFinger[board][channel]=99; //99 value
	}
	else if (ClusterId==101&&DetId==101){//T REF detected but not used
		MapFinger[board][channel]=101; //101 value
	}
	else{
		MapFinger[board][channel]=ClusterId*9+DetId;
		Channel_in_Cluster[MapFinger[board][channel]]=ClusterId;
		v1[MapFinger[board][channel]].SetX(1.*((TObjString* )toks->At(5))->GetString().Atoi());
		v1[MapFinger[board][channel]].SetY(1.*((TObjString* )toks->At(6))->GetString().Atoi());
		v1[MapFinger[board][channel]].SetZ(1.*((TObjString* )toks->At(7))->GetString().Atoi());
		
		Paris_Angle[MapFinger[board][channel]][0]=v1[MapFinger[board][channel]].Theta();
		Paris_Angle[MapFinger[board][channel]][1]=v1[MapFinger[board][channel]].Phi();
		//cout<<"Paris Angles :: "<<v1[MapFinger[board][channel]].X()<< "  "<< v1[MapFinger[board][channel]].Y() <<"  "<<v1[MapFinger[board][channel]].Z()<<" "<<endl;
	}
       }
	
	
	//cerr<<"ClusterId " <<ClusterId<<" DetId "<<DetId<<" board "<<board<<" channel "<<channel<<" Z " <<((TObjString* )toks->At(7))->GetString().Atof()<<endl;

	delete toks;
      }
    }
	
  //gets(stop);
  inf_f.close();   
	
  cout<<"=>Look Up Table Numexo2 Paris read"<<endl;
	
  return true;
}



bool TParis::IsMFMParis(MFMParisFrame *frame)
{ 
  bool result = false;
  int CristalId, localMapFinger, Board ;
  double valfShort, valfLong, valf;
  bool DuplicatedEvent=false;
  bool LaBr3, NaI, Compton;
  float k, thetaY, thetaX, q1, q2;
  LaBr3=false;
  NaI=false;
  Compton=false;
  TVector3 v1Local;
  float NaISize = 150. ; //mm  - to be confirmed
  float LaBrSize = 40.; //mm   - to be confirmed
  bool IsNotTref = true;
  
  Board=CristalId=-10;
  localMapFinger=1e4;
  int localDTS;
  if(debug)cerr<<"Enter in PARIS"<<endl;
  if(LUTBool==false){
    printf("\033[31m Error in TParis::IsMFMParis => The Look Up Table is not known: I won't be able to unpack the event   \033[m \n"); 
  }
  else{
     if(debug)cerr<<"--> Get Inside Paris Frame :: "<<endl;
     if(debug)cerr<<" (*) GetChannelId :: "<<frame->GetChannelId()<<endl;
     if(debug)cerr<<" (*) GetCristal   :: "<<frame->GetTGCristalId()<<endl;
     if(debug)cerr<<" (*) GetBoard     :: "<<frame->GetBoardId()<<endl;
     if(debug)cerr<<" (*) FrameSize    :: "<<frame->GetFrameSize()<<endl;
     if(debug)cerr<<" (*) Q Long is    :: "<<frame->GetQLong()<<endl;
     if(debug)cerr<<" (*) Cfd is       :: "<<frame->GetCfd()<<endl;
     if(debug)cerr<<" (*) GetTimeStamp+CFD   :: "<<frame->GetTimeStamp()*10.+frame->GetCfd()<<endl;

    CristalId=frame->GetChannelId(); //return cell ID
    Board=frame->GetBoardId();

    localMapFinger=MapFinger[Board][CristalId];
    
    if(localMapFinger==99||localMapFinger==101)IsNotTref=false;
    
    if(debug)cerr<<" (*) MapFinger::  "<<localMapFinger<<endl;
    if(debug&&localMapFinger==99)cerr<<" (*) MapFinger::  is TRef"<<endl;
    
    if(CristalId>=0&&localMapFinger>=0){
      result=true;
    }
    else {
      printf("\033[31m Error in TParis::IsMFMParis => This IP (%d) Channel (%d) is not known from the Look Up Table   \033[m\n",Board,CristalId); 
      result=false;
    }
  }
  if(Board==prevBoard&&CristalId==prevChannel&&frame->GetTimeStamp()==PrevTS&&prevQ==frame->GetQLong()){
		//cerr<<"Duplicated evt number :: Prev = "<<prevEvtNumber<<"  Current = "<<frame->GetEventNumber()<<endl;
		DuplicatedEvent=true;
		duplicatedEventC++;
  }	
  if(frame->GetTimeStamp()==0)cerr<<"TParis :: MapFinger : CristalId : Board = "<<localMapFinger<<" : " <<CristalId<<" : "<<Board<<endl;

  if(result&&DuplicatedEvent==false&&frame->GetQLong()>0&&IsNotTref){
  
    if(debug)cerr<<" (*) Push Data "<<endl; 
    
    //Calibrate
    valfShort = Cal(frame->GetQShort(),ECoefQShort[localMapFinger][0],ECoefQShort[localMapFinger][1],ECoefQShort[localMapFinger][2]);
    valfLong  = Cal(frame->GetQLong(), ECoefQLong[localMapFinger][0],ECoefQLong[localMapFinger][1],ECoefQLong[localMapFinger][2]);
    //Push  Data
    fParisData->SetParisDetectorNbr(localMapFinger);
    fParisData->SetParisQShort(valfShort);
    fParisData->SetParisQLong(valfLong);
    fParisData->SetParisCfd(frame->GetCfd()+TCfD[localMapFinger]);
    //fParisData->SetParisTheta(Paris_Angle[localMapFinger][0]);
    //fParisData->SetParisPhi(Paris_Angle[localMapFinger][1]);
    fParisData->SetfTimeStamps(frame->GetTimeStamp(),localMapFinger);

    if(CalibDone&&
    abs(valfShort-valfLong)<30 &&valfShort>10){ //LaBr3 fired
    	fParisData->SetParisQLaBr3Cond(valfShort);
	ClusterAddB[Channel_in_Cluster[localMapFinger]]=ClusterAddB[Channel_in_Cluster[localMapFinger]]+valfShort;
	ClusterCalo[Channel_in_Cluster[localMapFinger]]=ClusterCalo[Channel_in_Cluster[localMapFinger]]+valfShort;
	if(BoolSpec)fMyHistoParisQLaBr3[localMapFinger]->Fill(valfShort);
	if(BoolSpec)fMyHistoParisQShortQLongCond[localMapFinger]->Fill(valfShort,valfLong);
	if(BoolSpec)fMyHistoParisQLaBr3Sum->Fill(valfShort);
	if(abs(valfShort-1172)<50){//checking Card counters for Readout Debug
		BoardCounter[Board][CristalId]=BoardCounter[Board][CristalId]+1;
	}
	LaBr3=true;
	
	
    }
    else {
    	fParisData->SetParisQLaBr3Cond(0.);
    }
    
     
    if(CalibDone&&
    abs(valfShort*1.95-valfLong)<70){ //NaI fired ; was 2.23
    	fParisData->SetParisQNaICond(Cal(valfLong,ECoefQNaI[localMapFinger][0],ECoefQNaI[localMapFinger][1],ECoefQNaI[localMapFinger][2])); //*1.30135-49.9581
	ClusterCalo[Channel_in_Cluster[localMapFinger]]=ClusterCalo[Channel_in_Cluster[localMapFinger]]+valfLong*1.30135-49.9581;
	if(BoolSpec)fMyHistoParisQNaI[localMapFinger]->Fill(Cal(valfLong,ECoefQNaI[localMapFinger][0],ECoefQNaI[localMapFinger][1],ECoefQNaI[localMapFinger][2]));
	NaI=true;
    }
    else {
    	fParisData->SetParisQNaICond(0.);
    }
    
    if(NaI==false&&LaBr3==false && (valfLong>valfShort &&valfLong< valfShort*1.95)){ //Compton between the two
        if(BoolSpec)fMyHistoParisQLongCompton[localMapFinger]->Fill(valfLong);
	Compton=true;
    
    	// C. Ghosh et al arXiv:1605.00811v2 (2016)
	thetaX=24*TMath::DegToRad();
	thetaY=45*TMath::DegToRad();
	k=1./(TMath::Cos(thetaX+thetaY));
	q1=k*(valfShort*TMath::Cos(thetaX)-valfLong*TMath::Sin(thetaX));
	q2=k*(-1.0*valfShort*TMath::Sin(thetaY)+valfLong*TMath::Cos(thetaY));
	q1=q1*0.706458+7.002252;
	q2=q2*1.087571+50.766300;
	
	if(BoolSpec)fMyHistoParisCompton[localMapFinger]->Fill(q1,q2);
        if(localMapFinger==0&&BoolSpec)fMyHistoParisComptonRec->Fill(q1+q2);
	fParisData->SetParisQCompton(q1+q2);
   
    	ClusterCalo[Channel_in_Cluster[localMapFinger]]=ClusterCalo[Channel_in_Cluster[localMapFinger]]+q1+q2;
   }
   
   else {
   	fParisData->SetParisQCompton(0.);
   }
    
    //Define now the angle per Paris detector for the DC
        v1Local.SetX(v1[localMapFinger].X());
	v1Local.SetY(v1[localMapFinger].Y());
	v1Local.SetZ(v1[localMapFinger].Z());
	//cout<<localMapFinger<<" "<<v1[localMapFinger].X()<<"  "<<v1[localMapFinger].Y()<<endl;
	if(LaBr3||Compton){
		switch(Channel_in_Cluster[localMapFinger]){
			case 0:{
				v1Local.SetX(v1[localMapFinger].X()+(LaBrSize)/2.);
				v1Local.SetY(v1[localMapFinger].Y());
				v1Local.SetZ(v1[localMapFinger].Z());
				break;
			}
			case 1:{
				v1Local.SetX(v1[localMapFinger].X()+1./TMath::Sqrt(2)*(LaBrSize)/2.);
				v1Local.SetY(v1[localMapFinger].Y()+1./TMath::Sqrt(2)*(LaBrSize)/2.);
				v1Local.SetZ(v1[localMapFinger].Z());
				break;
			}
			case 2:{
				v1Local.SetX(v1[localMapFinger].X());
				v1Local.SetY(v1[localMapFinger].Y()+(LaBrSize)/2.);
				v1Local.SetZ(v1[localMapFinger].Z());
				break;
			}
			case 3:{
				v1Local.SetX(v1[localMapFinger].X()-1./TMath::Sqrt(2)*(LaBrSize)/2.);
				v1Local.SetY(v1[localMapFinger].Y()-1./TMath::Sqrt(2)*(LaBrSize)/2.);
				v1Local.SetZ(v1[localMapFinger].Z());
				break;
			}
			case 4:{
				v1Local.SetX(v1[localMapFinger].X()-(LaBrSize)/2.);
				v1Local.SetY(v1[localMapFinger].Y());
				v1Local.SetZ(v1[localMapFinger].Z());
				break;
			}
			case 5:{
				v1Local.SetX(v1[localMapFinger].X()-1./TMath::Sqrt(2)*(LaBrSize)/2.);
				v1Local.SetY(v1[localMapFinger].Y()-1./TMath::Sqrt(2)*(LaBrSize)/2.);
				v1Local.SetZ(v1[localMapFinger].Z());
				break;
			}
			case 6:{
				v1Local.SetX(v1[localMapFinger].X());
				v1Local.SetY(v1[localMapFinger].Y()-(LaBrSize)/2.);
				v1Local.SetZ(v1[localMapFinger].Z());
				break;
			}
			case 7:{
				v1Local.SetX(v1[localMapFinger].X()+1./TMath::Sqrt(2)*(LaBrSize)/2.);
				v1Local.SetY(v1[localMapFinger].Y()-1./TMath::Sqrt(2)*(LaBrSize)/2.);
				v1Local.SetZ(v1[localMapFinger].Z());
				break;
			}
			default:{
				cerr<<"TParis:: Warning--> Impossible Cluster Id"<<endl;
			}
		}
	
	}
	
	else if(NaI){
		switch(Channel_in_Cluster[localMapFinger]){
			case 0:{
				v1Local.SetX(v1[localMapFinger].X()+(LaBrSize+NaISize/2.));
				v1Local.SetY(v1[localMapFinger].Y());
				v1Local.SetZ(v1[localMapFinger].Z());
				break;
			}
			case 1:{
				v1Local.SetX(v1[localMapFinger].X()+1./TMath::Sqrt(2)*(LaBrSize+NaISize/2.));
				v1Local.SetY(v1[localMapFinger].Y()+1./TMath::Sqrt(2)*(LaBrSize+NaISize/2.));
				v1Local.SetZ(v1[localMapFinger].Z());
				break;
			}
			case 2:{
				v1Local.SetX(v1[localMapFinger].X());
				v1Local.SetY(v1[localMapFinger].Y()+(LaBrSize+NaISize/2.));
				v1Local.SetZ(v1[localMapFinger].Z());
				break;
			}
			case 3:{
				v1Local.SetX(v1[localMapFinger].X()-1./TMath::Sqrt(2)*(LaBrSize+NaISize/2.));
				v1Local.SetY(v1[localMapFinger].Y()-1./TMath::Sqrt(2)*(LaBrSize+NaISize/2.));
				v1Local.SetZ(v1[localMapFinger].Z());
				break;
			}
			case 4:{
				v1Local.SetX(v1[localMapFinger].X()-(LaBrSize+NaISize/2.));
				v1Local.SetY(v1[localMapFinger].Y());
				v1Local.SetZ(v1[localMapFinger].Z());
				break;
			}
			case 5:{
				v1Local.SetX(v1[localMapFinger].X()-1./TMath::Sqrt(2)*(LaBrSize+NaISize/2.));
				v1Local.SetY(v1[localMapFinger].Y()-1./TMath::Sqrt(2)*(LaBrSize+NaISize/2.));
				v1Local.SetZ(v1[localMapFinger].Z());
				break;
			}
			case 6:{
				v1Local.SetX(v1[localMapFinger].X());
				v1Local.SetY(v1[localMapFinger].Y()-(LaBrSize+NaISize/2.));
				v1Local.SetZ(v1[localMapFinger].Z());
				break;
			}
			case 7:{
				v1Local.SetX(v1[localMapFinger].X()+1./TMath::Sqrt(2)*(LaBrSize+NaISize/2.));
				v1Local.SetY(v1[localMapFinger].Y()-1./TMath::Sqrt(2)*(LaBrSize+NaISize/2.));
				v1Local.SetZ(v1[localMapFinger].Z());
				break;
			}
			default:{
				cerr<<"TParis:: Warning--> Impossible Cluster Id"<<endl;
			}
		}
	}
	
	else{
		v1Local.SetX(v1[localMapFinger].X());
		v1Local.SetY(v1[localMapFinger].Y());
		v1Local.SetZ(v1[localMapFinger].Z());
	}
	
	
	
	fParisData->SetParisTheta(v1Local.Theta());
    	fParisData->SetParisPhi(v1Local.Phi());
	
    
    
    
    if(debug)cerr<<" (*) Push Data completed "<<endl;
      
   
    if(debug&&BoolSpec)cerr<<" (*) Fill spectra "<<endl;
    if(BoolSpec)fMyHistoParisQShort[localMapFinger]->Fill(valfShort);
    if(BoolSpec)fMyHistoParisQLong[localMapFinger]->Fill(valfLong);
    if(BoolSpec)fMyHistoParisCfd[localMapFinger]->Fill(frame->GetCfd());
    if(BoolSpec)fMyHistoParisQShortQLong[localMapFinger]->Fill(valfShort,valfLong);

 
    if(BoolSpec&&CalibDone){
    	valf=(valfLong-valfShort)/valfShort;
	if(BoolSpec)fMyHistoParisMerit->Fill(valfShort,valf);
    }
    
    if(BoolSpec)fMyHistoParisPattQShort2d->Fill(localMapFinger,valfShort);
    if(BoolSpec)fMyHistoParisPattQLong2d->Fill(localMapFinger,valfLong);
    if(BoolSpec)fMyHistoParisPattCfd2d->Fill(localMapFinger,frame->GetCfd());
    localDTS=(int)frame->GetTimeStamp()-(int)PrevTS;
    //if(BoolSpec)TimeStampDiff->Fill(localDTS);
    //if(debug)cerr<<" (*)   --DT "<<PrevTS<<" -  "<< frame->GetTimeStamp()<< " = " <<localDTS<<endl;
    if(debug&&BoolSpec)cerr<<" (*) Fill spectra done"<<endl;
    
    
    if(debug)cerr<<" (*) Setting Prev event"<<endl;
   
    if(debug)cerr<<" (*) Is equal :  "<<frame->GetTimeStamp()<<"  //  " <<fParisData->GetfTimeStamps(localMapFinger)<<endl;
    if(debug)cerr<<" (*) Prev event is Board "<< prevBoard  << "  |  channel "<<  prevChannel <<" prevTS  " <<  PrevTS  <<"  | for energy "<< prevQ <<   "  at TS = "<< fParisData->GetfTimeStamps(localMapFinger)<< " DTS = "<< localDTS<< endl;
    //if(localDTS<0)printf("\033[31m*********  Time Stamp Error :: TParis back in future !!! ********* \033[m \n");
    PrevTS= fParisData->GetfTimeStamps(localMapFinger);
    prevBoard=Board;
    prevChannel=CristalId;
    prevQ=frame->GetQLong();
    
  }
  else if(IsNotTref==false){ //So it is TRef
  	if(debug)cerr<<" (*) Tref Push "<<endl;
  	fParisData->SetParisDetectorNbr(localMapFinger); //should be 99 
	fParisData->SetParisCfd(frame->GetCfd());
    	fParisData->SetfTimeStamps(frame->GetTimeStamp(),localMapFinger);
    	fParisData->SetParisQShort(0);
    	fParisData->SetParisQLong(0);
    	fParisData->SetParisTheta(0);
    	fParisData->SetParisPhi(0);
	fParisData->SetParisQLaBr3Cond(0) ;
	fParisData->SetParisQNaICond(0) ;
	fParisData->SetParisQCompton(0) ;
  	if(localMapFinger==99)EventWithTref++;
  }
   if(debug)cerr<<"(*) Paris Exit "<<endl;
  return result;
}

bool TParis::Is(UShort_t lbl, Short_t val)
{

  bool status = false;
 
  return status;
}



bool TParis::Treat()
{
  int ParisMulitiplicity=0;
  float ParisCalo=0;
  int detect1, detect2, TrueM;
  //char stop[1];
  Tref = -1;
  TrefExist = false;
  TrueM=0;  
  MergerCounter++;
  
  if(debug)cerr<<"(*) Paris Treat 0"<<endl;
  for(Int_t i =0 ; i<fParisData->GetParisMult(); i++){  //search for Tref
  	if(fParisData->GetParisDet(i)==99) { // 101 will not be treated
		if(TrefExist==true){
			//cout<<"Impossible, duplicated  TRef "<<TrefError <<endl;
			TrefError++;
		}
		Tref = i;
		TrefExist=true;
	}
	
	
  }
  
 if(TrefExist==false){
 	EventNoTref++;
	//cout<<"Not TRef  with PARIS Mul  = "<< fParisData->GetParisMult() <<endl;
	//gets(stop);
  }
  else {
  	//cout<<"TRef  with PARIS Mul  = "<<fParisData->GetParisMult() <<endl;
	
  	//gets(stop);
  
  }
 for(Int_t i =0 ; i<fParisData->GetParisMult(); i++){ 
 	if(fParisData->GetParisQShort(i)>0){//will exclude Tref
		ParisMulitiplicity++;
		detect1=fParisData->GetParisDet(i);
		if(BoolSpec&&TrefExist)TimeStampDiffTref->Fill((fParisData->GetfTimeStamps(detect1)*10.+fParisData->GetParisCfd(i)) - (fParisData->GetfTimeStamps(99)*10.+fParisData->GetParisCfd(Tref)));
		if(BoolSpec&&TrefExist)TimeStampDiffTrefPattern->Fill(detect1,(fParisData->GetfTimeStamps(detect1)*10.+fParisData->GetParisCfd(i)) - (fParisData->GetfTimeStamps(99)*10.+fParisData->GetParisCfd(Tref)));
		if(BoolSpec&&TrefExist)IndvTimeStampDiffTref[detect1]->Fill((fParisData->GetfTimeStamps(detect1)*10.+fParisData->GetParisCfd(i)) - (fParisData->GetfTimeStamps(99)*10.+fParisData->GetParisCfd(Tref)));
		
	}
 }
  
  if(debug)cerr<<"(*) Paris Treat 1"<<endl;
   //fParisData->Dump();
  for(Int_t i =0 ; i<fParisData->GetParisMult(); i++){
  	if(fParisData->GetParisQLaBr3Cond(i)>50){ //will exclude Tref
		
		detect1=fParisData->GetParisDet(i);
		fMyHistoParisThetaPhi->Fill(fParisData->GetParisTheta(i)*TMath::RadToDeg(),fParisData->GetParisPhi(i)*TMath::RadToDeg());
		//cout<<"theta "<< fParisData->GetParisTheta(i)*TMath::RadToDeg()<< "  "<<fParisData->GetParisPhi(i)*TMath::RadToDeg()<<endl;
		///cout<<"theta "<< fParisData->GetParisTheta(i)<< " Beta "<<Beta <<" "<<fParisData->GetParisQLaBr3Cond(i)<< "  "<<Doppler_Correction(fParisData->GetParisTheta(i),0,0., 0., Beta,fParisData->GetParisQLaBr3Cond(i))<<endl;
		if(BoolSpec)fMyHistoParisQLaBr3SumDC->Fill(Doppler_Correction(fParisData->GetParisTheta(i),0,0., 0., Beta,fParisData->GetParisQLaBr3Cond(i)));
		if(TrefExist&&((fParisData->GetfTimeStamps(detect1)*10.+fParisData->GetParisCfd(i)) - (fParisData->GetfTimeStamps(99)*10.+fParisData->GetParisCfd(Tref)))<680&&
			      ((fParisData->GetfTimeStamps(detect1)*10.+fParisData->GetParisCfd(i)) - (fParisData->GetfTimeStamps(99)*10.+fParisData->GetParisCfd(Tref)))>630){
			
			if(BoolSpec)NeutronPattern->Fill(detect1,fParisData->GetParisQLaBr3Cond(i));
			ParisCalo=ParisCalo+fParisData->GetParisQLaBr3Cond(i);
			TrueM++;
		}
		if(TrefExist&&BoolSpec&&TrueM>0)TimeStampDiffTrefEg->Fill(Doppler_Correction(fParisData->GetParisTheta(i),0,0., 0., Beta,fParisData->GetParisQLaBr3Cond(i)),((fParisData->GetfTimeStamps(detect1)*10.+fParisData->GetParisCfd(i)) - (fParisData->GetfTimeStamps(99)*10.+fParisData->GetParisCfd(Tref))));
		if(TrefExist&&BoolSpec&&((fParisData->GetfTimeStamps(detect1)*10.+fParisData->GetParisCfd(i)) - (fParisData->GetfTimeStamps(99)*10.+fParisData->GetParisCfd(Tref)))>10){
			//NeutronPattern->Fill(detect1,fParisData->GetParisQLaBr3Cond(i));
		}
	}
  }
  
  if(debug)cerr<<"(*) Paris Treat 2"<<endl;
  if(BoolSpec)fMyHistoParisMul->Fill(ParisMulitiplicity);
  if(BoolSpec&&ParisCalo>10&&TrueM>0)fMyHistoParisCaloFold->Fill(ParisCalo,TrueM);
  
  if(fParisData->GetParisMult()>1){
  	for(Int_t i =0 ; i<fParisData->GetParisMult(); i++){
		for(Int_t j =0 ; j<fParisData->GetParisMult(); j++){
			if(i!=j&&fParisData->GetParisQLaBr3Cond(i)>0&&fParisData->GetParisQLaBr3Cond(j)>0){
				fMyHistoParisQShortGG->Fill(fParisData->GetParisQLaBr3Cond(i),fParisData->GetParisQLaBr3Cond(j)); // with DC
				detect1=fParisData->GetParisDet(i);
				detect2=fParisData->GetParisDet(j);
				if(BoolSpec&&fParisData->GetParisQLaBr3Cond(i)>50&&fParisData->GetParisQLaBr3Cond(j)>50){
					TimeStampDiffQLaBr3->Fill((fParisData->GetfTimeStamps(detect1)*10.+fParisData->GetParisCfd(i)) - (fParisData->GetfTimeStamps(detect2)*10.+fParisData->GetParisCfd(j)));
					TimeStampDiffQLaBr3Eg->Fill(fParisData->GetParisQLaBr3Cond(i), abs((fParisData->GetfTimeStamps(detect1)*10.+fParisData->GetParisCfd(i)) - (fParisData->GetfTimeStamps(detect2)*10.+fParisData->GetParisCfd(j))));
					TimeStampDiffQLaBr3Eg->Fill(fParisData->GetParisQLaBr3Cond(j), abs((fParisData->GetfTimeStamps(detect1)*10.+fParisData->GetParisCfd(i)) - (fParisData->GetfTimeStamps(detect2)*10.+fParisData->GetParisCfd(j))));
					
				}
				//if(BoolSpec)TimeStampDiff->Fill((fParisData->GetfTimeStamps(detect1)*10.) - (fParisData->GetfTimeStamps(detect2)*10.));
				//if(BoolSpec)TimeStampDiff->Fill(fParisData->GetParisCfd(i) - fParisData->GetParisCfd(j));
				if(abs((fParisData->GetfTimeStamps(detect1)*10.+fParisData->GetParisCfd(i)) - (fParisData->GetfTimeStamps(detect2)*10.+fParisData->GetParisCfd(j)))<1)fMyHistoParisQShortGGTimeGated->Fill(fParisData->GetParisQLaBr3Cond(i),fParisData->GetParisQLaBr3Cond(j)); 
			}
		
			if(i!=j&&fParisData->GetParisQShort(i)>0&&fParisData->GetParisQShort(j)>0){
				detect1=fParisData->GetParisDet(i);
				detect2=fParisData->GetParisDet(j);
				if(BoolSpec)TimeStampDiffAll->Fill((fParisData->GetfTimeStamps(detect1)*10.+fParisData->GetParisCfd(i)) - (fParisData->GetfTimeStamps(detect2)*10.+fParisData->GetParisCfd(j)));
			}
			if(i!=j&&fParisData->GetParisQNaICond(i)>1000&&fParisData->GetParisQNaICond(j)>1000){
				detect1=fParisData->GetParisDet(i);
				detect2=fParisData->GetParisDet(j);
			   	if(BoolSpec)TimeStampDiffQNaI->Fill((fParisData->GetfTimeStamps(detect1)*10.+fParisData->GetParisCfd(i)) - (fParisData->GetfTimeStamps(detect2)*10.+fParisData->GetParisCfd(j)));
			}
			
		}
		
		
		
		
		
	}
  
  }
  
  
  if(debug)cerr<<"(*) Paris Treat 3"<<endl;

  for(Int_t i=0;i<9;i++){
	if(ClusterAddB[i]>0&&BoolSpec)fMyHistoParisAddBack->Fill(ClusterAddB[i]);
	if(ClusterCalo[i]>0&&BoolSpec)fMyHistoParisCalo->Fill(ClusterCalo[i]);
	
  }
  if(debug)cerr<<"(*) Paris Treat Exit"<<endl;
  return true;
}

bool TParis::Counter()
{
	for(int i =0 ; i <5;i++){
  		for(int j =0 ; j <16;j++){
			if(BoardCounter[i][j]>0)printf("PARIS :: Card %d Cristal %d 1.1 MeV Gated %d ",i,j,BoardCounter[i][j]);
		}
		//cout<<"------ Next Board"<<endl;
  	}
	printf(" \n");
  return true;

}

bool TParis::ClearCounter()
{


	for(int i =0 ; i <5;i++){
  		for(int j =0 ; j <16;j++){
			BoardCounter[i][j]=0;
		}
  	}

  return true;

}

void TParis::InitBranch(TTree *tree)
{
  tree->Branch("Paris", "TParisData", &fParisData,32000,99);
}


