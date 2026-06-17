#include <cstdlib>
#include "fstream"
#include "TCsI.h"
#include "TCsIData.h"
#include <TCutG.h>
#include <iostream>
#include "stdio.h"
#include "string.h"
#include "TFile.h"

ClassImp(TCsI)

TCsI::TCsI(bool bspec)
{
  // Default constructor
  fCsIData    = new TCsIData();
  BoolSpec=bspec;
  GoodCoincET=GoodCoincEPI=GoodCoincTPI=BadCoincET=BadCoincEPI=BadCoincTPI=0;
  PrevTSlocal=duplicatedEventC=prevBoard=prevChannel=tag=prevQ=prevQ2=0;
  LUTBool=false;
  CutLoaded=false;
  PrevTS=0;
}



TCsI::~TCsI()
{
  delete fCsIData;
}



bool TCsI::Clear()
{
  fCsIData->Clear();

  return true;
}
bool TCsI::InitCal()
{
	
  return true;
}

bool TCsI::SpectraConstructor()
{
  char title[50];
  if(BoolSpec){
    printf("\033[35mDiamant  Info :  Start Spectra constructors\033[m \n");
    for(int k=0 ; k<50 ; k++){
      sprintf(title,"Diamant_Energy%d",k);
      fMyHistoCsIE[k]= new TH1F(title,title,1024,0,16384*2); 
      HListCsI.Add(fMyHistoCsIE[k]);
		
      sprintf(title,"Diamant_ShortTop%d",k);
      fMyHistoCsIT[k]= new TH1F(title,title,1024,0,16384*2); 
      HListCsI.Add(fMyHistoCsIT[k]);
		
      sprintf(title,"Diamant_PID%d",k);
      fMyHistoCsIPI[k]= new TH1F(title,title,512,0,0.9); 
      HListCsI.Add(fMyHistoCsIPI[k]);
		
      sprintf(title,"Diamant_EvsPID_%d",k);
      fMyHistoCsIE_PI[k]= new TH2F(title,title,1024,0,16384*2,512,0,0.9); 
      HListCsI.Add(fMyHistoCsIE_PI[k]);		
		
    }
    TimeStampDiff= new TH1F("TimeStampDiffTCsI","TimeStampDiffTCsI",4000,-1000,3000);
    fMyHistoCsIPattE2d = new TH2F ("Diamant_Pattern_Energy_2d","Diamant Pattern for Energy",90,0,90,512,0,16384*2);
    fMyHistoCsIPattT2d = new TH2F ("Diamant_Pattern_ShortTop_2d","Diamant Pattern for ShortTop",90,0,90,512,0,16384*2);
    fMyHistoCsIPattPI2d = new TH2F ("Diamant_Pattern_PID_2d","Diamant Pattern for PID",90,0,90,512,0,16384*2);
    HListCsI.Add(TimeStampDiff);
    HListCsI.Add(fMyHistoCsIPattE2d); 
    HListCsI.Add(fMyHistoCsIPattT2d);
    HListCsI.Add(fMyHistoCsIPattPI2d);
	
    fMyHistoCsIPattE = new TH1F ("CsI_Pattern_E","CsI_Pattern_E",90,0,90);
    fMyHistoCsIPattT = new TH1F ("CsI_Pattern_T","CsI_Pattern_T",90,0,90);
    fMyHistoCsIPattPI = new TH1F ("CsI_Pattern_PI","CsI_Pattern_PI",90,0,90);
    HListCsI.Add(fMyHistoCsIPattE); 
    HListCsI.Add(fMyHistoCsIPattT);
    HListCsI.Add(fMyHistoCsIPattPI);
	
    fMyHistoCsIEmult=new TH1F("CsI_E_Mult_perEvent","CsI_E_Mult_perEvent",90,0,90); 
    fMyHistoCsITmult=new TH1F("CsI_T_Mult_perEvent","CsI_T_Mult_perEvent",90,0,90);
    fMyHistoCsIPImult=new TH1F("CsI_PI_Mult_perEvent","CsI_PI_Mult_perEvent",90,0,90);
	
	
    HListCsI.Add(fMyHistoCsIEmult);
    HListCsI.Add(fMyHistoCsITmult);
    HListCsI.Add(fMyHistoCsIPImult);
	
    fMyHistoCsIDataCheckET=new TH1F("CsIDataCheckET","CsIDataCheckET",100,0,100);
    fMyHistoCsIDataCheckEPI=new TH1F("CsIDataCheckEPI","CsIDataCheckEPI",100,0,100);
    fMyHistoCsIDataCheckTPI=new TH1F("CsIDataCheckTPI","CsIDataCheckTPI",100,0,100);
	
    HListCsI.Add(fMyHistoCsIDataCheckET);
    HListCsI.Add(fMyHistoCsIDataCheckEPI);
    HListCsI.Add(fMyHistoCsIDataCheckTPI);
    printf("\033[35m ----> Done \033[m \n");

	

	
    return true;
  }
  else{return false;}
}
bool TCsI::ReadCal()
{
	
  return true;
}

double TCsI::Cal(UShort_t en, float offset, float gain, float gain2){
  double enc; 
  enc = (double)en+gRandom->Uniform(1.0)-.5;
  enc = enc*enc*gain2+enc*gain+offset;
	   
  return enc;
}
double TCsI::CalI(int en, float offset, float gain, float gain2){
  double enc; 
  enc = (double)en+gRandom->Uniform(1.0)-.5;
  enc = enc*enc*gain2+enc*gain+offset;
	   
  return enc;
}

bool TCsI::Init(DataParameters *params)
{
  bool status = false;
  Int_t channel;
  TString schaine;
  TObjArray* toks=0;
  Char_t label[30];
  Int_t nbParams = params->GetNbParameters();
  for (Int_t index = 0; index < nbParams; index++) {
    Int_t lbl    = params->GetLabel(index);
    strcpy(label,params->GetParNameFromIndex(index));
    string labels=params->GetParNameFromIndex(index);
    schaine.Form("%s",label);
    //      cout << index << "  " << lbl << "  " << label << endl;
       
    if (schaine.BeginsWith("CsI")){
      if(schaine.Contains("Energy")||schaine.Contains("ENERGY")) {  
	fLabelMap[lbl] = labels;
	fTypeMap[lbl] = CsI_E;
	schaine.ReplaceAll("CsI","");
	toks = schaine.Tokenize("_");
	channel = ((TObjString* )toks->At(0))->GetString().Atoi();
	fParameterMap[lbl] = channel;
	delete toks;
	status = true;
      }
		
      else if(schaine.Contains("Time")||schaine.Contains("TIME")){
	fLabelMap[lbl] = labels;
	fTypeMap[lbl] = CsI_T;
	schaine.ReplaceAll("CsI","");
	toks = schaine.Tokenize("_");
	channel = ((TObjString* )toks->At(0))->GetString().Atoi();
	fParameterMap[lbl] = channel;
	delete toks;
	status = true;
		
      }
		
      else if(schaine.Contains("PI")){
	fLabelMap[lbl] = labels;
	fTypeMap[lbl] = CsI_PI;
	schaine.ReplaceAll("CsI","");
	toks = schaine.Tokenize("_");
	channel = ((TObjString* )toks->At(0))->GetString().Atoi();
	fParameterMap[lbl] = channel;
	delete toks;
	status = true;
      }
      else {printf("\033[31m Problem with Diamant Label \033[m \n");}
		
		
    }//End if CsI
  }
  return status; 
   
}

bool TCsI::InitNumexo2(Char_t *fileNumexo2){


  LUTBool=false;
	
  TString schainetrack;
  TObjArray* toks=0;
  Int_t board,Celloffset;
 // char stop[2];

  for(Int_t i= 0 ; i<255;i++){Current_Numexo2_cfg_Board[i]=-1;}
	
  ifstream inf_f(fileNumexo2);
  if(inf_f.good()==false) {
    printf("\033[31m Error Diamant:: Not a valid Look Up Table=> I complain here   \033[m \n"); 
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
				
	Current_Numexo2_cfg_Board[board]=Celloffset;
	delete toks;
      }
    }
	
  }
  inf_f.close();
	
  cout<<"=>Look Up Table Numexo2 DIAMANT read"<<endl;
	
  return true;
}



bool TCsI::IsMFMDiamant(MFMDiamantFrame *frame)
{ 
  bool result = false;
  int CristalId, MapFinger, Board ;
  double lCsI_ZCO;
  bool DuplicatedEvent=false;
  bool debug = false;	
  Board=CristalId=MapFinger=-10;
  if(LUTBool==false){
    printf("\033[31m Error in TCsI::IsMFMDiamant => The Look Up Table is not known: I won't be able to unpack the event   \033[m \n"); 
  }
  else{
    CristalId=frame->GetTGCristalId(); //return cell ID
    Board=frame->GetBoardId();
    if(debug)cerr<<"GetTGCristalId()  "<< frame->GetTGCristalId() <<endl;
    if(debug)cerr<<"GetBoardId()      "<<frame->GetBoardId() <<endl;
    //Board=(frame->DiaGetCristalId()>>5) &0x07ff;
    MapFinger=CristalId+Current_Numexo2_cfg_Board[Board];
    //cerr<<MapFinger<<"  "<<frame->GetTimeStamp()<< " "<< frame->GetEnergy()/512.<< endl;
    if(CristalId>=0&&Current_Numexo2_cfg_Board[Board]>=0){
      result=true;
    }
    else {
      printf("\033[31m Error in TCsI::IsMFMDiamant => This IP (%d) is not known from the Look Up Table   \033[m \n",Board); 
      result=false;
    }
  }
  if(Board==prevBoard&&CristalId==prevChannel&&frame->GetTimeStamp()==PrevTSlocal&&prevQ==frame->GetEnergy()){
		//cerr<<"Duplicated evt number :: Prev = "<<prevEvtNumber<<"  Current = "<<frame->GetEventNumber()<<endl;
		DuplicatedEvent=true;
		duplicatedEventC++;
  }	
  if(frame->GetTimeStamp()==0)cerr<<"TCsI :: MapFinger : CristalId : Board = "<<MapFinger<<" : " <<CristalId<<" : "<<Board<<endl;

  if(result&&DuplicatedEvent==false&&frame->GetEnergy()>0){
  
    lCsI_ZCO=1-frame->GetTop()*1./frame->GetEnergy();
  
    fCsIData->SetCsIEDetectorNbr(MapFinger);
    fCsIData->SetCsITDetectorNbr(MapFinger); 
    fCsIData->SetCsIPIDetectorNbr(MapFinger);
		
    fCsIData->SetCsIEnergy(frame->GetEnergy());
    fCsIData->SetCsIPI(lCsI_ZCO);
    fCsIData->SetCsITime(frame->GetTop());
    fCsIData->SetfTimeStamps(frame->GetTimeStamp(),MapFinger);
    
    
    if(BoolSpec)fMyHistoCsIE[MapFinger]->Fill(frame->GetEnergy()/512.);
    if(BoolSpec)fMyHistoCsIT[MapFinger]->Fill(frame->GetTop());
    if(BoolSpec)fMyHistoCsIPI[MapFinger]->Fill(lCsI_ZCO);
    if(BoolSpec)fMyHistoCsIE_PI[MapFinger]->Fill(frame->GetEnergy()/512.,lCsI_ZCO);
    if(BoolSpec)fMyHistoCsIPattE2d->Fill(MapFinger,frame->GetEnergy()/512.);
    if(BoolSpec)fMyHistoCsIPattE->Fill(MapFinger);
    if(BoolSpec)fMyHistoCsIPattPI2d->Fill(MapFinger,lCsI_ZCO);
    if(BoolSpec)fMyHistoCsIPattPI->Fill(MapFinger);
    
    prevBoard=Board;
    prevChannel=CristalId;
    prevQ=frame->GetEnergy();
    PrevTSlocal=frame->GetTimeStamp();
	
		
  }
   if(debug) cerr<<"Exit CsI IsMFM "<<endl;
  return result;
}

bool TCsI::Is(UShort_t lbl, Short_t val)
{

  bool status = false;
  int  MapFinger;
  switch (fTypeMap[lbl]) {
    
  case CsI_E :{  
    //cout<<  "- ---------< CsI E >------------------!\n";
    //cout<<lbl<<"CsI E "<<val <<" | "<<fParameterMap[lbl]<<endl;
    MapFinger=fParameterMap[lbl];
    fCsIData->SetCsIEDetectorNbr(MapFinger);
    fCsIData->SetCsIEnergy(val);
    if(BoolSpec)fMyHistoCsIE[MapFinger]->Fill(val);
    if(BoolSpec)fMyHistoCsIPattE2d->Fill(MapFinger,val);
    if(BoolSpec)fMyHistoCsIPattE->Fill(MapFinger);
    status = true;
    break;
  }
     
  case CsI_T :{  
    //cout<<  "- ---------< CsI T>------------------!\n";
    //cout<<lbl<<"CsI T "<<val <<" | "<<fParameterMap[lbl]<<endl;
    MapFinger=fParameterMap[lbl];
    fCsIData->SetCsITDetectorNbr(MapFinger);
    fCsIData->SetCsITime(val);

    if(BoolSpec)fMyHistoCsIPattT2d->Fill(MapFinger,val);
    if(BoolSpec)fMyHistoCsIPattT->Fill(MapFinger);
    status = true;
    break;
  }
     
  case CsI_PI :{  
    //cout<<  "- ---------< CsI PI>------------------!\n";
    //cout<<lbl<<"CsI PI "<<val <<" | "<<fParameterMap[lbl]<<endl;
    MapFinger=fParameterMap[lbl];
    fCsIData->SetCsIPIDetectorNbr(MapFinger);
    fCsIData->SetCsIPI(val);
    if(BoolSpec)fMyHistoCsIPI[MapFinger]->Fill(val);
    if(BoolSpec)fMyHistoCsIPattPI2d->Fill(MapFinger,val);
    if(BoolSpec)fMyHistoCsIPattPI->Fill(MapFinger);
    status = true;
    break;
  }
  default:{
    //cout<< "TCsI::Is --> not a good label"<<endl; 
    status = false;
    break;
  }
    
  } // end of switch

  return status;
}



bool TCsI::Treat()
{
  bool debug = false;
  PID_Index = 0;
  alpham = 0;
  protonm = 0;
  twoprotonm = 0;
  chargedm = 0;
  chargeParticleDetected = false;
  if(CutLoaded){
    if(debug)cerr<<"In CsI TCut treat"<<endl;
  for(Int_t i = 0 ; i<fCsIData->GetCsIEMult();i++){
    for(Int_t j= 0 ; j<fCsIData->GetCsIPIMult();j++){
      if((fCsIData->GetCsIEDetectorNbr(i))==(fCsIData->GetCsIPIDetectorNbr(j))){
				
	//-------------------------------------------------------------------------------------------------
	//-------------------------------------------testing output----------------------------------------
	//-------------------------------------------------------------------------------------------------
	/*			cout << "detector no: " << fCsIData->GetCsIEDetectorNbr(i) << endl;
				cout << "multiplicities :" << i << j << endl;
				cout << "energy: " << fCsIData->GetCsIEnergy(i) << endl;
				cout << "PID: " << fCsIData->GetCsIPI(j) << endl;
				cout << " " << endl;
				counter = counter + 1;
				cout << counter << endl;*/
	//-------------------------------------------------------------------------------------------------
	//-------------------------------------------Gating------------------------------------------------
	//-------------------------------------------------------------------------------------------------

	//---------------------------------------alpha---------------------------------------------------
	
	if ((gcut_AlphaArray[fCsIData->GetCsIEDetectorNbr(i)]->IsInside(fCsIData->GetCsIEnergy(i),fCsIData->GetCsIPI(j))==1))
	  {
	    //cout << "KAPUT " << << endl;
	    alpham = alpham+1;
	    totalalpha= totalalpha+1;
	    //	cout << "Total alphas: " << totalalpha << endl;
	  }
	//---------------------------------------proton---------------------------------------------------
	if ((gcut_ProtonArray[fCsIData->GetCsIEDetectorNbr(i)]->IsInside(fCsIData->GetCsIEnergy(i),fCsIData->GetCsIPI(j))==1))
	  {
	    protonm = protonm+1;
	    totalproton= totalproton+1;
	    //cout << "Total protons: " << totalproton << endl;
	  }
	//---------------------------------------two proton---------------------------------------------------
	if ((gcut_TwoProtonArray[fCsIData->GetCsIEDetectorNbr(i)]->IsInside(fCsIData->GetCsIEnergy(i),fCsIData->GetCsIPI(j))==1))
	  {
	    protonm = protonm+2;
	    totalproton= totalproton+2;
	    //cout << "Total protons: " << totalproton << endl;
	  }
	//---------------------------------------charged particle---------------------------------------------------
	/*                   if ((gcut_ChargedArray[fCsIData->GetCsIEDetectorNbr(i)]->IsInside(fCsIData->GetCsIEnergy(i),fCsIData->GetCsIPI(j))==1))
			     {
			     chargedm = chargedm+1;
			     totalcharged= totalcharged+1;
			     //				cout << "Total charged: " << totalcharged << endl;
			     chargeParticleDetected = true;
			     }*/



      }
    }
    /*	for(Int_t j= 0 ; j<fCsIData->GetCsITMult();j++){
	if((fCsIData->GetCsIEDetectorNbr(i))==(fCsIData->GetCsITDetectorNbr(j))){
	//if(BoolSpec)fMyHistoCsIE_T[fCsIData->GetCsIEDetectorNbr(i)]->Fill(fCsIData->GetCsIEnergy(i),fCsIData->GetCsITime(j));
	}
	}
    */
  }
  }
  /*
    for(Int_t i = 0 ; i<fCsIData->GetCsITMult();i++){
    for(Int_t j= 0 ; j<fCsIData->GetCsIPIMult();j++){
    if((fCsIData->GetCsITDetectorNbr(i))==(fCsIData->GetCsIPIDetectorNbr(j))){
    if(BoolSpec)fMyHistoCsIT_PI[fCsIData->GetCsITDetectorNbr(i)]->Fill(fCsIData->GetCsITime(i),fCsIData->GetCsIPI(j));
    }
    }
    }
  */
	
  if(debug)cout << "In TCsI.cxx Global spectra treat"<< endl;
  if(BoolSpec)fMyHistoCsIEmult->Fill(fCsIData->GetCsIEMult());
  if(BoolSpec)fMyHistoCsITmult->Fill(fCsIData->GetCsITMult());
  if(BoolSpec)fMyHistoCsIPImult->Fill(fCsIData->GetCsIPIMult());
  
  if(fCsIData->GetCsIEMult()==fCsIData->GetCsIPIMult()){GoodCoincEPI++;}
  else{BadCoincEPI++;}
  
  if(fCsIData->GetCsIEMult()==fCsIData->GetCsITMult()){GoodCoincET++;}
  else{BadCoincET++;}
    
  if(fCsIData->GetCsIPIMult()==fCsIData->GetCsITMult()){GoodCoincTPI++;}
  else{BadCoincET++;}
	

  //Limit the number, so that there is no overflow
  if(protonm>7)
    protonm = 7;
  if(alpham>3)
    alpham = 3;
  PID_Index = protonm*16+alpham*4;
  if(PID_Index>0)
    chargeParticleDetected = true;
  if(debug)cout << "In TCsI.cxx PID_Index = " << PID_Index << endl;
  if(debug)cout << "TCsI.cxx Treat Exit = " << endl;
  return true;
}

bool TCsI::Counter()
{

  printf("\033[35m ********* Diamant Info :  Missing Coinc E_Time   = %2.3f percent (Average 2009 value [20-50])\033[m \n",BadCoincET*100./(BadCoincET+GoodCoincET));
  printf("\033[35m ********* Diamant Info :  Missing Coinc E_PID    = %2.3f percent (Average 2009 value [10-40])\033[m \n",BadCoincEPI*100./(BadCoincEPI+GoodCoincEPI));
  printf("\033[35m ********* Diamant Info :  Missing Coinc Time_PID = %2.3f percent (Average 2009 value 0)\033[m \n",BadCoincTPI*100./(BadCoincTPI+GoodCoincTPI));

  if(BoolSpec)fMyHistoCsIDataCheckET->Fill(BadCoincET*100./(BadCoincET+GoodCoincET));
  if(BoolSpec)fMyHistoCsIDataCheckEPI->Fill(BadCoincEPI*100./(BadCoincEPI+GoodCoincEPI));
  if(BoolSpec)fMyHistoCsIDataCheckTPI->Fill(BadCoincTPI*100./(BadCoincTPI+GoodCoincTPI));

  return true;

}

bool TCsI::ClearCounter()
{
  GoodCoincET=GoodCoincEPI=GoodCoincTPI=BadCoincET=BadCoincEPI=BadCoincTPI=0;

  return true;

}

void TCsI::InitBranch(TTree *tree)
{
  tree->Branch("CsI", "TCsIData", &fCsIData,32000,99);
}

int TCsI::getPID_Index() {
  return PID_Index;
}

int TCsI::isParticleVeto() {
  return chargeParticleDetected;
}
bool TCsI::ReadCut() {

  cerr<<""<<endl;
  printf("\033[32mInfo::  Reading DIAMANT cuts \033[m \n");

  for(int i =0; i<84; i++) {
    gcut_ProtonArray[i] = NULL;
    gcut_AlphaArray[i] =  NULL;
    gcut_TwoProtonArray[i] =  NULL;
    gcut_ChargedArray[i]= NULL;
  }
  // Reads the cuts for Energy vs PID
  for (Int_t nn = 0; nn < 84; nn++) {
    sprintf(Cname, "./CutDiamant/diamantCut%d.root", nn);
    if (fopen(Cname, "r") != NULL) {
      TFile fileCut(Cname, "READ");
      gcut_ProtonArray[nn] = (TCutG*) fileCut.Get("proton");
      gcut_AlphaArray[nn] = (TCutG*) fileCut.Get("alpha");
      gcut_TwoProtonArray[nn] = (TCutG*) fileCut.Get("twoproton");
      gcut_ChargedArray[nn]=(TCutG*)fileCut.Get("charged");
      fileCut.Close();
      printf("%d cut loaded, ", nn);
    } else {
      cout << "Warning:: could not read cuts for diamant detector number " << nn << ". Use generic cut." << endl;
      //Read an alternative Cut for this diamant detector (generic fallback solution)
      TFile fileCut("./CutDiamant/diamantCut.root", "READ");
      if(fileCut.IsOpen()) {
	gcut_ProtonArray[nn] = (TCutG*) fileCut.Get("proton");
	gcut_AlphaArray[nn] = (TCutG*) fileCut.Get("alpha");
	gcut_TwoProtonArray[nn] = (TCutG*) fileCut.Get("twoproton");
	gcut_ChargedArray[nn]=(TCutG*)fileCut.Get("charged");
	fileCut.Close();
      } else {
	cout << "Warning:: Could not read generic cut, Fall back to empty cuts for " << nn << endl;
      }
    }
  }
  CutLoaded=true;
  return true;

}
