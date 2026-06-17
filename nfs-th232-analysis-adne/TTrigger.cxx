#include <cstdlib>

#include "TTrigger.h"


ClassImp(TTrigger)

TTrigger::TTrigger(bool bspec)
{
   // Default constructor
   fTriggerData    = new TTriggerData();
   BoolSpec=bspec;
   PrevEventNumbMK2=0;
   MK2EvtCounter=MK2EvtCounterTotal=0;
   ExoBufferAbscent=ExoBufferAbscentCompt=AGAVABufferAbscentCompt=0;
   MK2Present=AGAVAPresent=false;
   tgv=gmt=agava=PrevTS=0;
}



TTrigger::~TTrigger()
{
   delete fTriggerData;
}



bool TTrigger::Clear()
{
   fTriggerData->Clear();
   centrum0b=agava0b=false;
   return true;
}
bool TTrigger::InitCal()
{

   return true;
}
bool TTrigger::ReadCal()
{

   return true;
}
bool TTrigger::SetMK2(bool mk2b){
	MK2Present=mk2b;
	if(mk2b) cerr<<"TTrigger Info :: MK2 card present "<<endl;
	else cerr<<"TTrigger Info :: MK2 card is not present "<<endl;
	return true;
}
bool TTrigger::SetAGAVA(bool agab){
	AGAVAPresent=agab;
	if(agab) cerr<<"TTrigger Info :: AGAVA card present "<<endl;
	else cerr<<"TTrigger Info :: AGAVA card is not present "<<endl;
	return true;
}
bool TTrigger::IsAGAVA(){
	return AGAVAPresent ;
}
bool TTrigger::SpectraConstructor()
{
if(BoolSpec){
	centEXO = new TH1F("cent0","cent0",16000,0,16000);
	fMyHistoTriggerCorr= new TH2I("trigCorrVamos","trigCorrVamos",3000,0,3000,3000,0,3000);
	AGAVA_Status= new TH1F("AGAVA_Status_135","AGAVA_Status_135",301,0,300);
	TimeStampDiff= new TH1F("TimeStampDiffTTrigger","TimeStampDiffTTrigger",4000,-1000,3000);
	return true;
}
else{return false;}
}
double TTrigger::Cal(UShort_t en, float offset, float gain, float gain2)
{

   return 0.;
}
bool TTrigger::Init(DataParameters *params)
{
   bool status = false;
   Int_t nbParams = params->GetNbParameters();
   for (Int_t index = 0; index < nbParams; index++) {
      Int_t lbl    = params->GetLabel(index);
      string label = params->GetParNameFromIndex(index);
//      cout << index << "  " << lbl << "  " << label <<  endl;
       
      if (label.compare(0,11,"TGV_CONFDEC") == 0) {  //Trigger VAMOS crate1 TGV
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = TRIG_1;
	 status = true;
	 //cerr<<label<<endl;
//	    cout << fTypeMap[lbl] << endl;
      } 
      else if (label.compare(0,7,"GATCONF") == 0) {//Trigger VAMOS crate2 GMT
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = TRIG_2;
	 status = true;
	//cerr<<label<<endl;
      } 
      
      else if (label.compare(0,13,"Trig_EvNumLSB") == 0) {//Trigger EXOGAM VXI
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = TRIG_4;
	 status = true;
	 //cerr<<label<<endl;
      }
      else if (label.compare(0,9,"TGV_AGATA") == 0) { //Trigger VAMOS crate 3 TGV AGAVA
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = TRIG_3;
	 status = true;
	 //cerr<<label<<endl;
      } 
      
      
      else if (label.compare(0,13,"AGAVA_TRIG_UP") == 0) {
         fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]= AGAVA_TRIG_UP;
	 status = true;
	 //cerr<<label<<endl;
      }
       else if (label.compare(0,10,"AGAVA_TRIG") == 0) { //AGAVA trigger request
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	fParameterMap[lbl]= AGAVA_TRIG;
	 status = true;
	 //cerr<<label<<endl;
      }

    

      else if (label.compare(0,13,"AGAVA_STAT_UP") == 0) {
        fLabelMap[lbl] = label;
	 fTypeMap[lbl] =  AGAVA;
	 fParameterMap[lbl]=AGAVA_STAT_UP;
	 status = true;
	 //cerr<<label<<endl;
      }
      else if (label.compare(0,10,"AGAVA_STAT") == 0) { //AGAVA STATUS
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_STAT;
	 //cerr<<label<<endl;
	 status = true;
      } 
      
      
      else if (label.compare(0,12,"AGAVA_REJ_UP") == 0) {
         fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_REJ_UP;
	 //cerr<<label<<endl;
	 status = true;
      }
      else if (label.compare(0,9,"AGAVA_REJ") == 0) { //AGAVA rejected
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_REJ;
	 //cerr<<label<<endl;
	 status = true;
      } 
      
      
      else if (label.compare(0,12,"AGAVA_VAL_UP") == 0) {
         fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_VAL_UP;
	 //cerr<<label<<endl;
	 status = true;
      }
      else if (label.compare(0,9,"AGAVA_VAL") == 0) { //AGAVA Valid
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_VAL;
	 //cerr<<label<<endl;
	 status = true;
      }
      
     else if (label.compare(0,14,"AGAVA_LTAGL_UP") == 0) {
         fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_LTAGL_UP;
	 //cerr<<label<<endl;
	 status = true;
      }
     else if (label.compare(0,11,"AGAVA_LTAGH") == 0) { //AGAVA Valid
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_LTAGH;
	 //cerr<<label<<endl;
	 status = true;
      }
      else if (label.compare(0,11,"AGAVA_LTAGL") == 0) {
         fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_LTAGL;
	 //cerr<<label<<endl;
	 status = true;
      }
      
      
     else if (label.compare(0,11,"AGAVA_RTAGH") == 0) { //AGAVA Valid
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_RTAGH;
	 //cerr<<label<<endl;
	 status = true;
      }
      else if (label.compare(0,14,"AGAVA_RTAGL_UP") == 0) { //AGAVA Valid
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_RTAGL_UP;
	 //cerr<<label<<endl;
	 status = true;
      } 
      else if (label.compare(0,11,"AGAVA_RTAGL") == 0) { //AGAVA Valid
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] =  AGAVA;
	 fParameterMap[lbl]=AGAVA_RTAGL;
	 //cerr<<label<<endl;
	 status = true;
      } 
      
      else if (label.compare(0,11,"AGAVA_VTAGH") == 0) { //AGAVA Valid
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_VTAGH;
	// cerr<<label<<endl;
	 status = true;
      }
      else if (label.compare(0,14,"AGAVA_VTAGL_UP") == 0) { //AGAVA Valid
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_VTAGL_UP;
	 cerr<<label<<endl;
	 status = true;
      } 
      else if (label.compare(0,11,"AGAVA_VTAGL") == 0) { //AGAVA Valid
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_VTAGL;
	 //cerr<<label<<endl;
	 status = true;
      } 
      else if (label.compare(0,14,"AGAVA_EVENT_UP") == 0) { //AGAVA Valid
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_EVENT_UP;
	 //cerr<<label<<endl;
	 status = true;
      }
      else if (label.compare(0,11,"AGAVA_EVENT") == 0) { //AGAVA Valid
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_EVENT;
	 //cerr<<label<<endl;
	 status = true;
      }
      
      else if (label.compare(0,14,"AGAVA_LTRIG_UP") == 0) { //AGAVA Valid
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_LTRIG_UP;
	 //cerr<<label<<endl;
	 status = true;
      }
      else if (label.compare(0,11,"AGAVA_LTRIG") == 0) { //AGAVA Valid
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_LTRIG;
	 //cerr<<label<<endl;
	 status = true;
      }
      
      else if (label.compare(0,13,"AGAVA_TOUT_UP") == 0) { //AGAVA Valid
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_TOUT_UP;
	 //cerr<<label<<endl;
	 status = true;
      }
      else if (label.compare(0,10,"AGAVA_TOUT") == 0) { //AGAVA Valid
	 fLabelMap[lbl] = label;
	 fTypeMap[lbl] = AGAVA;
	 fParameterMap[lbl]=AGAVA_TOUT;
	// cerr<<label<<endl;
	 status = true;
      }
      
      
      else {
	 //cout << "TTrigger::Init() : problem reading Trigger's label" << endl;
	 status = false;
      }
   }
   
   
   return status;
}
bool TTrigger::InitNumexo2(Char_t *fileNumexo2){

return true;
}
/*bool TTrigger::IsMFMExo(MFMExogamFrame *frame)
{
	return false;
}
*/
bool TTrigger::Is(UShort_t lbl, Short_t val)
{
   bool status = false;

   switch (fTypeMap[lbl]) {
    
     case TRIG_1 :{  
       //cout<<  "- ---------< Trigger 1 >------------------!\n"; // VAMOS TRIGGER TGV
       fTriggerData->SetTRIG1(val);
       tgv++;
       status = true;
       break;
     }
  
     case TRIG_2 :{  
       //cout<<  "- ---------< Trigger 2 >------------------!\n"; //VAMOS TRIGGER GMT
       fTriggerData->SetTRIG2(val);
       gmt++;
       status = true;
       break;
     }

     case TRIG_3 :{  
      // cout<<  "- ---------< Trigger 3 >------------------!\n";
       fTriggerData->SetTRIG3(val);
       if(BoolSpec)centEXO->Fill(val);
       agava++;
       status = true;
       break;
     }
  
     case TRIG_4 :{  
       //cout<<  "- ---------< Trigger 4 >------------------!\n";
       fTriggerData->SetTRIG4(val);
      // cout<< " Trig_EvNumLSB  " << val <<endl;
       centrum0b=true;
       status = true;
       break;
     }
  
     case AGAVA :{  
       //cout<<  "- ---------< Trigger 5 >------------------!\n";
       fTriggerData->SetAGAVADATA(val,fParameterMap[lbl]);
       agava0b=true;
       status = true;
       break;
     }
    
    
     
     
     default:{
       //cout<<"TTrigger::Is --> not a good label"<<endl;
       status = false;
     }
   } // end of switch
  
   return status;
}



bool TTrigger::Treat()
{
  if((fTriggerData->GetTRIG4()-PrevEventNumbMK2)!=1){MK2EvtCounter++;}
  PrevEventNumbMK2=fTriggerData->GetTRIG4();
  unsigned long long Trig,Rej,Val,LocalTag,RejectTag,ValidTag,EventCounter,LocalTrig,TimeOut;
  Trig=Rej=Val=LocalTag=RejectTag=ValidTag=EventCounter=LocalTrig=TimeOut=0;
  
  int tempo=1E5;
  if(centrum0b==false){ExoBufferAbscentCompt++;} 
  if(agava0b==false){AGAVABufferAbscentCompt++;} 
  ExoBufferAbscent++;
   
   
  AGAVA_Status->Fill(fTriggerData->GetAGAVADATA(AGAVA_STAT));

   if(AGAVAPresent){
   
        Trig=(unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_TRIG)+((unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_TRIG_UP)<<16);
	Rej=(unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_REJ)+((unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_REJ_UP)<<16);
	Val=(unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_VAL)+((unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_VAL_UP)<<16);
	LocalTag=10*((unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_LTAGL)+((unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_LTAGL_UP)<<16)+((unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_LTAGH)<<32)); //in ns
	RejectTag=10*((unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_RTAGL)+((unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_RTAGL_UP)<<16)+((unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_RTAGH)<<32));//in ns
	ValidTag=10*((unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_VTAGL)+((unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_VTAGL_UP)<<16)+((unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_VTAGH)<<32));//in ns
	EventCounter=(unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_EVENT)+((unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_EVENT_UP)<<16);
	LocalTrig=(unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_LTRIG)+((unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_LTRIG_UP)<<16);
	TimeOut=(unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_TOUT)+((unsigned long long)fTriggerData->GetAGAVADATA(AGAVA_TOUT_UP)<<16);
	
	fTriggerData->SetfTimeStamps(LocalTag,0);
	fTriggerData->SetfTimeStamps(RejectTag,1);
	fTriggerData->SetfTimeStamps(ValidTag,2);
	
   }
   
  if(ExoBufferAbscent%tempo==0&& AGAVAPresent){
	printf("\033[32m ---------------- TTrigger Infos --------------\033[m \n");
	printf("\033[32m Merged Branches %d --> GMT:: %d  TGV:: %d  TGV_AGAVA:: %d \033[m \n",tempo,gmt,tgv,agava);
	printf("\033[32m AGAVA (Req,Rej,Val):: (%lld, %lld, %lld :: %2.2f)\033[m \n",Trig,Rej,Val,100.*Val/Trig);
	printf("\033[32m AGAVA (EvtCounter,LocalTrig,TimeOut):: (%lld, %lld, %lld)\033[m \n",EventCounter,LocalTrig,TimeOut);
	printf("\033[32m AGAVA Latency:: %2.2f usec \033[m \n",(ValidTag*1.- LocalTag*1.)/1000.);
	printf("\033[33m AGAVA Info :  %2.2f percent of events without AGAVA Patt \033[m \n",100.*AGAVABufferAbscentCompt/ExoBufferAbscent);
        printf("\033[32m ----------------------------------------------\033[m \n");
	gmt=tgv=agava=0;
	ExoBufferAbscent=AGAVABufferAbscentCompt=0;	
	
  }
	
	
  if((fTriggerData->GetTRIG2()==0 ||fTriggerData->GetTRIG1()==0 ||fTriggerData->GetTRIG3()==0) && AGAVAPresent){
		printf("\033[31m *********  Branches are missing !!! ********* \033[m \n");
		if(fTriggerData->GetTRIG2()==0)printf("\033[31m *********  GMT Branch is missing !!! ********* \033[m \n");
		if(fTriggerData->GetTRIG1()==0)printf("\033[31m *********  TGV Branch is missing !!! ********* \033[m \n");
		if(fTriggerData->GetTRIG3()==0)printf("\033[31m *********  TGV_AGAVA Branch is missing !!! ********* \033[m \n");
  }
  else {fMyHistoTriggerCorr->Fill(fTriggerData->GetTRIG3(),fTriggerData->GetTRIG1());}
 
 
 if(ExoBufferAbscent>1E5&&MK2Present){
   	printf("\033[33m MK2 Info :  %2.2f percent of events without MK2 EXOGAM Patt \033[m \n",100.*ExoBufferAbscentCompt/ExoBufferAbscent);
	printf("\033[37m MK2 Info :  %2.2f percent of events with MK2 inconsistent Evt number \033[m \n",100.*MK2EvtCounter/ExoBufferAbscent);
   	MK2EvtCounter=MK2EvtCounterTotal=0;
	ExoBufferAbscent=ExoBufferAbscentCompt=0;	
   }
  
  
   return true;
   
   
   
   
}



void TTrigger::InitBranch(TTree *tree)
{
   tree->Branch("Trigger", "TTriggerData", &fTriggerData);
}

