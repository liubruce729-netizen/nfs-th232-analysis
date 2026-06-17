#include <cstdlib>

#include "TMW.h"
#include "TObjString.h"

ClassImp(TMW)

TMW::TMW(bool bspec)
{
   // Default constructor
   fMWData    = new TMWData();
   BoolSpec=bspec;
}



TMW::~TMW()
{
   delete fMWData;
}



bool TMW::Clear()
{
   fMWData->Clear();

   return true;
}

bool TMW::InitCal()
{
	for(int i = 0;i<20;i++){
		TCoef[i][0]=0.;
		TCoef[i][1]=1.;	
	}
return true;
}
bool TMW::SpectraConstructor()
{
if(BoolSpec){
	fMyHistoMW=new TH2F("MWPC2D","MPWPC2D",20,0,20,2000,0,1000);
	fMyHistoVelocity=new TH1F("V_cm_ns","V_cm_ns",1000,0,10);
	fMyHistoBeta=new TH1F("beta","beta",1000,0,1);
	fMyHistoMWMul=new TH1F("MWmul","MWmul",20,0,20);
	fMyHistoTacCorr = new TH2F("tacCor","tacCor",2048,0,16384,2048,0,16384);
	return true;
}
else {return false;}
}
bool TMW::ReadCal()
{
	FILE *cal = fopen("CalFile/mw.cal","r"); 
	Int_t x;
	float a,b;
	
	for(x=0;x<20;x++){
		fscanf(cal,"%f %f \n",&a,&b);
    		TCoef[x][0]=a;
		TCoef[x][1]=b;
	//cout <<a << " "<<b << " "<<x<<endl;
    	}
	fclose(cal);
	Path= 750.; //cm
return true;
}

double TMW::Cal(UShort_t en, float offset, float gain, float gain2){
double enc;
	    enc = (double)en+gRandom->Uniform(1.0)-.5;
	    enc = enc*enc*gain2+enc*gain+offset;
	    return enc;
}
bool TMW::InitNumexo2(Char_t *fileNumexo2){

return true;
}

bool TMW::Init(DataParameters *params)
{
   bool status = false;
   Int_t channel;
   TString schaine;
   TObjArray* toks=0;
   Char_t label[30];
//cerr<<"*****************************ENTER in loop:: there are "<< params->GetNbParameters() <<endl;
   Int_t nbParams = params->GetNbParameters();
   
   for (Int_t index = 0; index < nbParams; index++) {
       Int_t lbl    = params->GetLabel(index);
       strcpy(label,params->GetParNameFromIndex(index));
       string labels=params->GetParNameFromIndex(index);
       schaine.Form("%s",label);
            // cout << index << "  " << lbl << "  " << label << endl;
       
       if (schaine.BeginsWith("MW")) {  
	  fLabelMap[lbl] = labels;
          fTypeMap[lbl] = MW_E;
	  toks = schaine.Tokenize("_");
	  channel = ((TObjString* )toks->At(1))->GetString().Atoi();
	  fParameterMap[lbl] = channel;
	  delete toks;
 	  status = true;
	 // cerr<<schaine<<endl;
	 }
	else if(
		schaine.BeginsWith("TIA_HYB_W")  
		){
		fLabelMap[lbl] = labels;
		fTypeMap[lbl] = TAC;
		status = true;
	}
 	else if(schaine.BeginsWith("T_Barrel_EXO")){
		fLabelMap[lbl] = labels;
		fTypeMap[lbl] = BARREL;
		status = true;
	}
	else if(schaine.BeginsWith("T_Hyball_EXO")){
		fLabelMap[lbl] = labels;
		fTypeMap[lbl] = HYBALL;
		status = true;
	}
        else if(schaine.BeginsWith("T_MM_EXO")){
		fLabelMap[lbl] = labels;
		fTypeMap[lbl] = MUST2;
		status = true;
	}
	else if(schaine.BeginsWith("MM")&&schaine.Contains("E")&&schaine.Contains("STRX")){
		fLabelMap[lbl] = labels;
		fTypeMap[lbl] = MUST2NRJX;
		status = true;
	}
	else if(schaine.BeginsWith("MM")&&schaine.Contains("E")&&schaine.Contains("STRY")){
		fLabelMap[lbl] = labels;
		fTypeMap[lbl] = MUST2NRJY;
		status = true;
	}
	
   }
   
   return status;
}
bool TMW::SetBeta(float b){

	Velocity=b;
	return true;
}


double TMW::GetBeta(){

	return Velocity;
}
/*bool TMW::IsMFMExo(MFMExogamFrame *frame)
{
	return false;
 }*/
bool TMW::Is(UShort_t lbl, Short_t val)
{
   bool status = false;
   int  MapFinger;
   float valf;
   
   switch (fTypeMap[lbl]) {
     case MW_E :{  
       //     cout<<  "- ---------< MW E >------------------!\n";
       MapFinger=fParameterMap[lbl];
       valf=Cal(val,TCoef[MapFinger][0],TCoef[MapFinger][1],0.); 
       fMWData->SetEnergy(valf);
       fMWData->SetMWDetectorNbr(MapFinger);
       if(BoolSpec){
       		fMyHistoMW->Fill(MapFinger,valf); //2D for wire vs Charge
		fMyHistoVelocity->Fill(Path/valf); //cm per ns
	}
	SetBeta((Path/valf)/29.9792458);
	fMyHistoBeta->Fill(GetBeta());
	
       status = true;
       break;
     }
 
   case TAC :{
       if(val>3000)fMWData->SetTac(val);
       status = true;
       break;
   }
   case BARREL :{
       fMWData->SetBARREL(val);
       status = true;
       break;
   }
   case HYBALL :{
       fMWData->SetHYBALL(val);
       status = true;
       break;
   }
   case MUST2 :{
       fMWData->SetMUST2(val);
       status = true;
       break;
   }
   case MUST2NRJX :{
       if(val>8300&&val<15000)fMWData->SetMUST2NRJX(val);
       status = true;
       break;
   }
   case MUST2NRJY :{
       if(val>100&&val<8100)fMWData->SetMUST2NRJY(val);
       status = true;
       break;
   }
   
     default:{
       //cout<< "TMW::Is --> not a good label"<<endl; 
       status = false;
       break;
     }
  } // end of switch

  return status;
}



bool TMW::Treat()
{
	fMyHistoMWMul->Fill(fMWData->GetMultE());
	fMyHistoTacCorr->Fill(fMWData->GetTac(),fMWData->GetBARREL());
	
	
   return true;
}



void TMW::InitBranch(TTree *tree)
{
   tree->Branch("MW", "TMWData", &fMWData,32000,99);
}

