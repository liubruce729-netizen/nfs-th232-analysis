#include <cstdlib>

#include "TNWall.h"
#include "TNWallData.h"
// #include "NWallN2.h"

#include "fstream"

ClassImp(TNWall)
TNWall::TNWall(bool bspec)
{
  // Default constructor
  //  fNWallN2      = new NWallN2();
  fNWallData    = new TNWallData();
  BoolSpec=bspec;
  GoodCoincQT=GoodCoincQZCO=GoodCoincTZCO=BadCoincQT=BadCoincQZCO=BadCoincTZCO=0;
  LUTBool=false;
  PrevTS=prevBoard=prevChannel=prevQ=prevQ2=duplicatedEventC=PrevTStagged=PrevTSlocal=prevEvtNumber=0;
  tag=3;
}



TNWall::~TNWall()
{
  //  delete fNWallN2;
  delete fNWallData;
}



int TNWall::GetNbetween(int id1, int id2)
{
  return n_nw_dets_in_between[id1][id2];
}

 


bool TNWall::Init_n2()
{
  /* ------------------------------------------------------------ */
  /*  NW number of dets in between                                */
  /* ------------------------------------------------------------ */
   printf("\033[32mInfo::  Reading NEDA 2 neutrons discrimination \033[m \n");
  char line[80];
  FILE *fp;
  if ((fp = fopen("./CutNWall/how_many_nw_dets_in_between","r")) == NULL){
    cout << "Cannot open how_many_nw_dets_in_between file"  << endl;
  }
  else {
    int i_list = 0;
    int id1, id2, n_dets;
    printf("NW dets in between from how_many_nw_dets_in_between...");
    while (fgets(line,80,fp)) {
      if (sscanf(line, "%d %d %d\n", &id1, &id2, &n_dets)< 3) {
	printf(" Error reading in between dets positions\n");
	return(1);
      }
      else {
	  n_nw_dets_in_between[id1][id2] = n_dets;
	  i_list++;
      }	
    }	
    printf(" %d lines read \n", i_list); 
  }
  fclose(fp);

  if ((fp = fopen("./CutNWall/ns_dt.limits","r")) == NULL){
    printf("Error opening ns_dt.limits");
    return(1);
  }
  else {
    int i_list = 0;
    printf("NS-DT limits from ns_dt.limits...");
    while (fgets(line,80,fp)) {
      int ns;
      float  low_lim, high_lim, first_lim, last_lim;
      int nc = sscanf(line, "%d %e %e %e %e\n", &ns, &low_lim, &high_lim,
		    &first_lim, &last_lim);
      if ((nc != 3) && (nc !=5)) {
	printf(" Error reading ns-dt limits\n");
	return(1);
      }
      else if (nc == 3) {
// 	ns_dt_limits[ns].low = low_lim; 
// 	ns_dt_limits[ns].high = high_lim; 
// 	ns_dt_limits[ns].first = -9999.0;
// 	ns_dt_limits[ns].last = 9999.0;
 	ns_dt_limit_low[ns] = low_lim; 
 	ns_dt_limit_high[ns] = high_lim; 
 	ns_dt_limit_first[ns] = -9999.0;
 	ns_dt_limit_last[ns] = 9999.0;
 	  i_list++;
      }
      else if (nc == 5) {
	ns_dt_limit_low[ns] = low_lim;
	ns_dt_limit_high[ns] = high_lim;
	ns_dt_limit_first[ns] = first_lim;
	ns_dt_limit_last[ns] = last_lim;
	i_list++;
	}
    }	
    printf(" %d lines read \n", i_list); 
  }
  fclose(fp);
  
  printf("NWall N2 discrimination initialised \n");
  return true;
}

bool TNWall::IsSeparated(int id1, float z1, 
			      int id2,  float z2, float dt)
{
  int id_1, id_2, n_between, separated;
  //  float q1, q2;
  if (z1>z2) {
    id_1 = id1; id_2 = id2;
    //    q1=z1;  q2=z2; 
  }
  else {
    id_1 = id2; id_2 = id1;
    //    q1=z2; q2=z1; dt = -dt;
  }
  n_between =  GetNbetween(id_1,id_2);
  if ((dt <= ns_dt_limit_first[n_between]) ||
      ((dt >= ns_dt_limit_low[n_between])&&
       (dt <= ns_dt_limit_high[n_between])) ||
      (dt >= ns_dt_limit_last[n_between])) {
    separated = 1;
  }  
  else {
    separated = 0;
  }
  return separated;
}



bool TNWall::Clear()
{
  fNWallData->Clear();
  
  return true;
}

bool TNWall::InitCal()
{
  
  Int_t x; 	
  
  for(x=0;x<96;x++){
    TOFCoef[x][0]=0.;
    TOFCoef[x][1]=1.;
    TOFCoef[x][2]=0.;
  }
  
  for(x=0;x<96;x++){
    ZCOCoef[x][0]=0.;
    ZCOCoef[x][1]=1.;
    ZCOCoef[x][2]=0.;
  }	
  for(x=0;x<96;x++){
    QCoef[x][0]=0.;
    QCoef[x][1]=1.;
    QCoef[x][2]=0.;
  }
  TACCoef_FTCFD[0]=0;
  TACCoef_FTCFD[1]=1;
  TACCoef_FTCFD[2]=0;
  TACCoef_BAFHF[0]=0;
  TACCoef_BAFHF[1]=1;
  TACCoef_BAFHF[2]=0;
  TACCoef_FTHF[0]=0;
  TACCoef_FTHF[1]=1;
  TACCoef_FTHF[2]=0;
  TACCoef_CFDHF[0]=0;
  TACCoef_CFDHF[1]=1;
  TACCoef_CFDHF[2]=0;
  CalibDone=false;
  return true;
}


bool TNWall::SpectraConstructor()
{
  char title[50];
  if(BoolSpec){
    printf("\033[35mNeda  Info :  Start Spectra constructors\033[m \n");
    for(int k=0 ; k<96 ; k++){
      sprintf(title,"NWallQ%02d",k);
      fMyHistoNWallE[k]= new TH1F(title,title,16384,0,16384*4); 
      HListNWall.Add(fMyHistoNWallE[k]);
      
      sprintf(title,"NWallTOF%02d",k);
      fMyHistoNWallTOF[k]= new TH1F(title,title,16384,0,16384*4); 
      //fMyHistoNWallTOF[k]= new TH1F(title,title,4096,-200,200); 
      HListNWall.Add(fMyHistoNWallTOF[k]);
      
      sprintf(title,"NWallZCO%02d",k);
      fMyHistoNWallZCO[k]= new TH1F(title,title,16384,0,256*4); 
      HListNWall.Add(fMyHistoNWallZCO[k]);
      
      sprintf(title,"NWallTRF%02d",k);
      fMyHistoNWallTRF[k]= new TH1F(title,title,4096,-200,200); 
      HListNWall.Add(fMyHistoNWallTRF[k]);
      
      sprintf(title,"NWallQ_ZCO%02d",k);
      fMyHistoNWallZCO_E[k]= new TH2F(title,title,2000,0,2000,512,0,16384); 
      HListNWall.Add(fMyHistoNWallZCO_E[k]);
      
      sprintf(title,"NWallZCO_TOF%02d",k);
      fMyHistoNWallZCO_TOF[k]= new TH2F(title,title,2000,0,2000,2000,0,16384); 
      HListNWall.Add(fMyHistoNWallZCO_TOF[k]);

      sprintf(title,"NWallZCO_TRF%02d",k);
      fMyHistoNWallZCO_TRF[k]= new TH2F(title,title,512,300,3500,512,-100,50); 
      HListNWall.Add(fMyHistoNWallZCO_TRF[k]);

     sprintf(title,"NWallSlowIntegral%02d",k);
     fMyHistoNWallSlowIntegral[k]= new TH1F(title,title,16384,0,16384*2); 
     HListNWall.Add(fMyHistoNWallSlowIntegral[k]);
     
     sprintf(title,"NWallFastIntegral%02d",k);
     fMyHistoNWallFastIntegral[k]= new TH1F(title,title,16384,0,16384*2); 
     HListNWall.Add(fMyHistoNWallFastIntegral[k]);
     
     sprintf(title,"NWallBitfield%02d",k);
     fMyHistoNWallBitfield[k]= new TH1F(title,title,256,0,256);
     HListNWall.Add(fMyHistoNWallBitfield[k]);
     
     sprintf(title,"NWallAbsMax%02d",k);
     fMyHistoNWallAbsMax[k]= new TH1F(title,title,256,0,256);
     HListNWall.Add(fMyHistoNWallAbsMax[k]);





#ifdef TAC_CFD_HF_INDIVIDUAL_DETS
      // the following 3 lines are for CFD time alignment
      sprintf(title,"TACtmp%02d",k);
      fMyHistoTACtmp[k]= new TH1F(title,title,4096,-200,200); 
      HListNWall.Add(fMyHistoTACtmp[k]);
#endif      

#ifdef TAC_CFD_HF_VS_TRF
      sprintf(title,"TAC_CFD_HF_VS_TRF%02d",k);
      fMyHistoTAC_CFD_HF_VS_TRF[k] = new TH2F(title,title,512,-200,200,512,-200,200);
      HListNWall.Add(fMyHistoTAC_CFD_HF_VS_TRF[k]);
#endif



    }

    for (Int_t i_hist = 0; i_hist<9; i_hist++) {
      sprintf(title,"NWallNbetween_dt%01d",i_hist);
      fMyHistoNWallNbetween_dt[i_hist] = new TH1F(title,title,4096,-200,200); 
      HListNWall.Add(fMyHistoNWallNbetween_dt[i_hist]);
    }
    
    fMyHistoNWallPattE2d = new TH2F ("NWall_Pattern_E_2d","NWall_Pattern_E_2d",96,0,96,512,0,16384);
    fMyHistoNWallPattT2d = new TH2F ("NWall_Pattern_T_2d","NWall_Pattern_T_2d",96,0,96,512,0,8000);
    fMyHistoNWallPattZCO2d = new TH2F ("NWall_Pattern_ZCO_2d","NWall_Pattern_ZCO_2d",96,0,96,512,0,8000);
    TimeStampDiff= new TH1F("TimeStampDiffTNwall","TimeStampDiffTNwall",4000,-1000,3000);
    TimeStampDiffInterEvent= new TH1F("TimeStampDiffTNwallInterEvent","TimeStampDiffTNwallInterEvent",4000,-1000,3000);
    BoardDiff= new TH1F("BoardDiff","BoardDiff",100,0,100);
    ChannelDiff= new TH1F("ChannelDiff","ChannelDiff",100,0,100);
    ChannelDiff2D= new TH2F("ChannelDiff2D","ChannelDiff2D",100,0,100,100,0,100);
    CrossTalk2D= new TH2F("CrossTalk2D","CrossTalk2D",512,0,16384*4,512,0,16384*4);
    
    
    HListNWall.Add(fMyHistoNWallPattE2d); 
    HListNWall.Add(fMyHistoNWallPattT2d);
    HListNWall.Add(fMyHistoNWallPattZCO2d);
    HListNWall.Add(TimeStampDiff);
    HListNWall.Add(TimeStampDiffInterEvent);
    HListNWall.Add(BoardDiff);
    HListNWall.Add(ChannelDiff);
    HListNWall.Add(ChannelDiff2D);
    
    fMyHistoNWallPattE = new TH1F ("NWall_Pattern_E","NWall_Pattern_E",96,0,96);
    fMyHistoNWallPattT = new TH1F ("NWall_Pattern_T","NWall_Pattern_T",96,0,96);
    fMyHistoNWallPattZCO = new TH1F ("NWall_Pattern_ZCO","NWall_Pattern_ZCO",96,0,96);
    HListNWall.Add(fMyHistoNWallPattE); 
    HListNWall.Add(fMyHistoNWallPattT);
    HListNWall.Add(fMyHistoNWallPattZCO);
    
    fMyHistoTAC_BaF2_HF =new TH1F("TAC_BaF2_HF","TAC_BaF2_HF",16384,0,16384);
    fMyHistoTAC_FT_HF =new TH1F("TAC_FT_HF","TAC_FT_HF",16384,0,16384);
    fMyHistoTAC_FT_CFD =new TH1F("TAC_FT_CFD","TAC_FT_CFD",16384,0,16384);
    fMyHistoTAC_CFDft_HF =new TH1F("TAC_CFDft_HF","TAC_CFDft_HF",16384,0,16384);
    HListNWall.Add(fMyHistoTAC_BaF2_HF);
    HListNWall.Add(fMyHistoTAC_FT_HF);
    HListNWall.Add(fMyHistoTAC_FT_CFD);
    HListNWall.Add(fMyHistoTAC_CFDft_HF);
 
    
    fMyHistoTAC_BaF2_HF_cal =new TH1F("TAC_BaF2_HF_ns","TAC_BaF2_HF_ns",4096,-200,200);
    fMyHistoTAC_FT_HF_cal =new TH1F("TAC_FT_HF_ns","TAC_FT_HF_ns",4096,-200,200);
    fMyHistoTAC_FT_CFD_cal =new TH1F("TAC_FtT_CFD_ns","TAC_FT_CFD_ns",4096,-1000,1000);
    fMyHistoTAC_CFDft_HF_cal =new TH1F("TAC_CFDft_HF_ns","TAC_CFDft_HF_ns",4096,-200,200);
    fMyHistoTAC_BaF2_CFD_cal =new TH1F("TAC_BaF2_CFD_ns","TAC_BaF2_CFD_ns",4096,-200,200);
    fMyHistoTAC_BaF2_FT_cal =new TH1F("TAC_BaF2_FT_ns","TAC_BaF2_FT_ns",4096,-200,200);
    HListNWall.Add(fMyHistoTAC_BaF2_HF_cal);
    HListNWall.Add(fMyHistoTAC_FT_HF_cal);
    HListNWall.Add(fMyHistoTAC_FT_CFD_cal);
    HListNWall.Add(fMyHistoTAC_BaF2_CFD_cal);
    HListNWall.Add(fMyHistoTAC_CFDft_HF_cal);
    HListNWall.Add(fMyHistoTAC_BaF2_FT_cal);

    fMyHistoNWallEmult =new TH1F("NWall_Q_Mult_perEvent","NWall_Q_Mult_perEvent",96,0,96);
    fMyHistoNWallTmult =new TH1F("NWall_T_Mult_perEvent","NWall_T_Mult_perEvent",96,0,96);
    fMyHistoNWallZCOmult =new TH1F("NWall_ZCO_Mult_perEvent","NWall_ZCO_Mult_perEvent",96,0,96);
    
    HListNWall.Add(fMyHistoNWallEmult);
    HListNWall.Add(fMyHistoNWallTmult);
    HListNWall.Add(fMyHistoNWallZCOmult);
    
    fMyHistoNWallDataCheckQT=new TH1F("NWallDataCheckQT","NWallDataCheckQT",100,0,100);
    fMyHistoNWallDataCheckQZCO=new TH1F("NWallDataCheckQZCO","NWallDataCheckQZCO",100,0,100);
    fMyHistoNWallDataCheckTZCO=new TH1F("NWallDataCheckTZCO","NWallDataCheckTZCO",100,0,100);
    printf("\033[35m ----> Done \033[m \n");
    return true;
  }
  else {
    return false;
  }
}

bool TNWall::ReadCal() {
  Float_t a, b, c;
  char  name[50];
  FILE *nwall_cal=fopen("CalFile/nwall.cal","r");// fichier de calibration NWall
  for(Int_t x=0;x<96;x++){
    if (fscanf(nwall_cal,"%f %f %f %s\n",&a,&b,&c,name)!=4) {
      cout << "Error reading NWall T calibration coefs" << endl;
      return false;
    }
    TOFCoef[x][0]=a;
    TOFCoef[x][1]=b;
    TOFCoef[x][2]=c;
	//	cout <<  a << "  " << b << "  " << c << endl; 
  }
  
  for(Int_t x=0;x<96;x++){
    if (fscanf(nwall_cal,"%f %f %f %s\n",&a,&b,&c,name)!=4) {
      cout << "Error reading NWall Z calibration coefs" << endl;
      return false;
    }
    ZCOCoef[x][0]=a;
    ZCOCoef[x][1]=b;
    ZCOCoef[x][2]=c;
  }	
  for(Int_t x=0;x<96;x++){
    if (fscanf(nwall_cal,"%f %f %f %s\n",&a,&b,&c,name)!=4) {
      cout << "Error reading NWall Q calibration coefs" << endl;
      return false;
    }
    QCoef[x][0]=a;
    QCoef[x][1]=b;
    QCoef[x][2]=c;
  }	
  fclose(nwall_cal);
 // cout << "NWall calib OK " << endl;
  
  FILE *tac_cal=fopen("CalFile/tac.cal","r");// fichier de calibration tac
  for(Int_t x=0;x<4;x++){
    if (fscanf(tac_cal,"%f %f %f %s\n",&a,&b,&c,name)!=4) {
      printf("%f %f %f %s\n",a, b,c,name);
      //   	  if (fscanf(tac_cal,"%f %f %f\n",&a,&b,&c)!=3) {
      cout << "Error reading TAC calibration coefs" << endl;
      return false;
    }
    if(x==0){
      TACCoef_FTCFD[0]=a;
      TACCoef_FTCFD[1]=b;
      TACCoef_FTCFD[2]=c;
    }
    else if(x==1){
      TACCoef_BAFHF[0]=a;
      TACCoef_BAFHF[1]=b;
      TACCoef_BAFHF[2]=c;
    }
    else if(x==2){
      TACCoef_FTHF[0]=a;
      TACCoef_FTHF[1]=b;
      TACCoef_FTHF[2]=c;
    }
    else{
      TACCoef_CFDHF[0]=a;
      TACCoef_CFDHF[1]=b;
      TACCoef_CFDHF[2]=c;
    }
  }	
  fclose(tac_cal);
 // cout << "TAC calib OK " << endl;
  return true;
}  


double TNWall::Cal(UShort_t en, float offset, float gain, float gain2) {
  double enc;
  enc = (double)en+gRandom->Uniform(1.0)-.5;
  enc = enc*enc*gain2+enc*gain+offset;
  //  enc = (double)en;
  return enc;
}

bool TNWall::Init(DataParameters *params) {
 bool status = false;
 Int_t channel;
 Int_t module;
 Int_t tmp;
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
     
     if (schaine.BeginsWith("ADC3214V")){
       
       fLabelMap[lbl] = labels;
       schaine.ReplaceAll("ADC3214V","");
       toks = schaine.Tokenize("_c");
       
       module = ((TObjString* )toks->At(0))->GetString().Atoi();
       channel = ((TObjString* )toks->At(1))->GetString().Atoi();
			
       if(module==0){tmp=channel; fTypeMap[lbl] = NWall_T;} //module 0
       else if(module==1&&channel<18){tmp=32+channel; fTypeMap[lbl] = NWall_T;}//TOF
       
       else if(module==4){tmp=channel;fTypeMap[lbl] = NWall_ZCO;}
       else if(module==5&&channel<18){tmp=32+channel;fTypeMap[lbl] = NWall_ZCO;} //ZCO
       
       else if(module==2){tmp=channel;fTypeMap[lbl] = NWall_E;}
       else if(module==3&&channel<18){tmp=32+channel;fTypeMap[lbl] = NWall_E;} //Q
       
       
       else{tmp=channel;fTypeMap[lbl] = ADCEMPTY;}
			
       fParameterMap[lbl] = tmp;
       //cout << fParameterMap[lbl]<<endl;
       delete toks;
       status = true;
     }
   }
   return status; 
}
bool TNWall::InitNumexo2(Char_t *fileNumexo2){
	LUTBool=false;
	
	
	
TString schainetrack;
TObjArray* toks=0;
Int_t board,PMToffset;
//char stop[2];

	for(Int_t i= 0 ; i<255;i++){Current_Numexo2_cfg_Board[i]=-1;}
	
	ifstream inf_f(fileNumexo2);
	if(inf_f.good()==false) {
		printf("\033[31m Error:: Not a valid Look Up Table=> I complain here   \033[m \n"); 
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
				PMToffset= ((TObjString* )toks->At(1))->GetString().Atoi();
				
				Current_Numexo2_cfg_Board[board]=PMToffset;
				delete toks;
			}
		}
	
	}
	inf_f.close();
	
	cout<<"=>Look Up Table Numexo2 NEDA read"<<endl;
	
	
	

return true;
}
bool TNWall::IsMFMNeda(MFMNedaFrame *frame)
{
	
	bool result = false;
	int CristalId, MapFinger, Board ;
	float lNWall_E,lNWall_TOF, lNWall_ZCO;
	bool debug=false;
	bool DuplicatedEvent=false;
	
	
	CristalId=MapFinger=Board=-1;

	if(LUTBool==false){
   		printf("\033[31m Error in TNWall::IsMFMNeda => The Look Up Table is not known: I won't be able to unpack the event   \033[m \n"); 
	}
  	else{
   		CristalId=frame->GetChannelId(); //return cell ID
		Board=frame->GetBoardId();
   		MapFinger=CristalId+Current_Numexo2_cfg_Board[Board];
		if(debug)cerr<<"MapFinger : CristalId : Board = "<<MapFinger<<" : " <<CristalId<<" : "<<Board<<endl;
		if(MapFinger>MAX_NEUT)printf("\033[31m Error in TNwall::MapFinger is greater than Number of Detector  : %d  Board %d = offset %d CristalId %d \033[m \n",MapFinger,Board,Current_Numexo2_cfg_Board[Board],CristalId);
		
   		if(CristalId>=0&&Current_Numexo2_cfg_Board[Board]>=0){
			result=true;
		}
   		else {
			printf("\033[31m Error in TNwall::IsMFMNeda => This IP (%d) is not known from the Look Up Table   \033[m \n",Board); 
   			result=false;
   		}
   	}
	if(Board==prevBoard&&CristalId==prevChannel&&frame->GetTimeStamp()==PrevTSlocal&&prevQ==frame->GetSlowIntegral()&&prevEvtNumber==frame->GetEventNumber()){
		//cerr<<"Duplicated evt number :: Prev = "<<prevEvtNumber<<"  Current = "<<frame->GetEventNumber()<<endl;
		DuplicatedEvent=true;
		duplicatedEventC++;
	}
	if(frame->GetTimeStamp()==0)cerr<<"TNWall :: MapFinger : CristalId : Board = "<<MapFinger<<" : " <<CristalId<<" : "<<Board<<endl;

	if(result&&DuplicatedEvent==false&&(frame->GetSlowIntegral()>0)&&(frame->GetFastIntegral()>0)){
		if(debug)cerr<<"MapFinger : CristalId : Board = "<<MapFinger<<" : " <<CristalId<<" : "<<Board<<endl;
		fNWallData->SetNWallEDetNbr(MapFinger);
		fNWallData->SetNWallTOFDetNbr(MapFinger); 
		fNWallData->SetNWallZCODetNbr(MapFinger);
		
        	fNWallData->SetNWallEnergy(frame->GetSlowIntegral()+frame->GetFastIntegral());
		fNWallData->SetNWallTOF(frame->GetTdcValue());
		fNWallData->SetNWallZCO(frame->GetZcoInterval());
		
		
		fNWallData->SetNWallEnergyFrame(frame->GetLeInterval());
 		fNWallData->SetNWallTimeFrame(frame->GetInterpolCFD());
 		fNWallData->SetNWallSlowIntegral(frame->GetSlowIntegral());
 		fNWallData->SetNWallFastIntegral(frame->GetFastIntegral());
 		fNWallData->SetNWallIntRaiseTime(0);
 		fNWallData->SetNWallNeuralNetWork(0);
 		fNWallData->SetNWallNbZero(0);
 		fNWallData->SetNWallNeutronFlag(0);
		
		fNWallData->SetfTimeStamps(frame->GetTimeStamp(),MapFinger);
		
		
		
		
		
		lNWall_E=frame->GetSlowIntegral()+frame->GetFastIntegral();
		lNWall_TOF=frame->GetTdcValue();
		lNWall_ZCO=Cal(frame->GetSlowIntegral(),0,10,0)*256./Cal(frame->GetFastIntegral(),0,1,0);
		
		if(CristalId==tag){
			TimeStampDiffInterEvent->Fill(frame->GetTimeStamp()-PrevTStagged);
			PrevTStagged=frame->GetTimeStamp();
		}
		
		BoardDiff->Fill(Board-prevBoard);
		
		
		if((frame->GetTimeStamp()-PrevTSlocal)<4){
			if(debug)cout<<"Previous Channel "<<prevChannel  <<" Channel "<< CristalId << " Current TS "<<frame->GetTimeStamp()<<" prev TS "<<PrevTS<<" TSDiff " <<frame->GetTimeStamp()-PrevTS<<endl;
			if(debug)cout<< " ---- Q= " << frame->GetSlowIntegral()<<" Diff Q "<<prevQ- frame->GetSlowIntegral()<< " QF= " << frame->GetFastIntegral()<<" Diff Q "<<prevQ2- frame->GetFastIntegral()<<endl;
			ChannelDiff->Fill(CristalId-prevChannel);
			ChannelDiff2D->Fill(CristalId,prevChannel);
			CrossTalk2D->Fill(frame->GetSlowIntegral()+frame->GetFastIntegral(),prevQ+prevQ2);
		}
		
		if(debug)cout<<"GetSlowIntegral "<<frame->GetSlowIntegral()<<" GetFastIntegral "<<frame->GetFastIntegral()<<endl;
		if(debug)cout<<"lNWall_ZCO "  <<lNWall_ZCO<<" lNWall_TOF "<<lNWall_TOF<<" lNWall_E "<< lNWall_E<<endl;
		if(BoolSpec)fMyHistoNWallE[MapFinger]->Fill(lNWall_E);
    		if(BoolSpec)fMyHistoNWallPattE2d->Fill(MapFinger,lNWall_E);
    		if(BoolSpec)fMyHistoNWallPattE->Fill(MapFinger);
		
		if(BoolSpec)fMyHistoNWallPattT2d->Fill(MapFinger,lNWall_TOF);
    		if(BoolSpec)fMyHistoNWallPattT->Fill(MapFinger);
		if(BoolSpec)fMyHistoNWallTOF[MapFinger]->Fill(lNWall_TOF);
		
		if(BoolSpec)fMyHistoNWallPattZCO2d->Fill(MapFinger,lNWall_ZCO);
    		if(BoolSpec)fMyHistoNWallPattZCO->Fill(MapFinger);
		if(BoolSpec)fMyHistoNWallZCO[MapFinger]->Fill(lNWall_ZCO);
		
		
		if(BoolSpec)fMyHistoNWallSlowIntegral[MapFinger]->Fill(frame->GetSlowIntegral());
		if(BoolSpec)fMyHistoNWallFastIntegral[MapFinger]->Fill(frame->GetFastIntegral());
		if(BoolSpec)fMyHistoNWallBitfield[MapFinger]->Fill(frame->GetBitfield());
		if(BoolSpec)fMyHistoNWallAbsMax[MapFinger]->Fill(frame->GetAbsMax());
    		if(BoolSpec)fMyHistoNWallZCO_E[MapFinger]->Fill(lNWall_ZCO,lNWall_E);
	        if(BoolSpec)fMyHistoNWallZCO_TOF[MapFinger]->Fill(lNWall_ZCO,lNWall_TOF);
		prevBoard=Board;
		prevChannel=CristalId;
		prevQ=frame->GetSlowIntegral();
		prevQ2=frame->GetFastIntegral();
		PrevTSlocal=frame->GetTimeStamp();
		prevEvtNumber=frame->GetEventNumber();
      }
  
	
	return result;
 }
bool TNWall::IsMFMNedaComp(MFMNedaCompFrame *frame)
{
	bool result = false;
	bool debug=false;
	bool DuplicatedEvent=false;
	
	int CristalId, MapFinger, Board ;
	float lNWall_E,lNWall_TOF, lNWall_ZCO;
	
	CristalId=MapFinger=Board=-1;

	
	
	if(debug)cerr<<"In IsMFMNedaComp"<<endl;
	if(LUTBool==false){
   		printf("\033[31m Error in TNWall::IsMFMNeda => The Look Up Table is not known: I won't be able to unpack the event   \033[m \n"); 
	}
  	else{
   		CristalId=frame->GetChannelId(); //return cell ID
		Board=frame->GetBoardId();
   		MapFinger=CristalId+Current_Numexo2_cfg_Board[Board];
		if(debug)cerr<<"MapFinger : CristalId : Board = "<<MapFinger<<" : " <<CristalId<<" : "<<Board<<endl;

		if(MapFinger>MAX_NEUT)printf("\033[31m Error in TNwall::MapFinger is greater than Number of Detector  : %d  Board %d = offset %d CristalId %d \033[m \n",MapFinger,Board,Current_Numexo2_cfg_Board[Board],CristalId);
   		
		if(CristalId>=0&&Current_Numexo2_cfg_Board[Board]>=0){
			result=true;
		}
   		else {
			printf("\033[31m Error in TNwall::IsMFMNeda => This IP (%d) is not known from the Look Up Table   \033[m \n",Board); 
   			result=false;
   		}
   	}
	
	if(Board==prevBoard&&CristalId==prevChannel&&frame->GetTimeStamp()==PrevTSlocal&&prevQ==frame->GetSlowIntegral()&&prevEvtNumber==frame->GetEventNumber()){
		//cerr<<"Duplicated evt number :: Prev = "<<prevEvtNumber<<"  Current = "<<frame->GetEventNumber()<<endl;
		DuplicatedEvent=true;
		duplicatedEventC++;
		
	}
	if(debug)cerr<<" Step 0 "<<endl;
	if(debug)cerr<<" Step TS "<<frame->GetTimeStamp()<<endl;
	if(frame->GetTimeStamp()==0)cerr<<"TNwall :: MapFinger : CristalId : Board = "<<MapFinger<<" : " <<CristalId<<" : "<<Board<<endl;
	
	if(debug)cerr<<" Step FT "<<frame->GetFastIntegral()<<endl;
	
	if(result&&DuplicatedEvent==false&&(frame->GetFastIntegral()>0)){
		if(debug)cerr<<" Step 1 "<<endl;
		fNWallData->SetNWallEDetNbr(MapFinger);
		fNWallData->SetNWallTOFDetNbr(MapFinger); 
		fNWallData->SetNWallZCODetNbr(MapFinger);
		
		
		//Calculated Output
        	fNWallData->SetNWallEnergy(frame->GetSlowIntegral()+frame->GetFastIntegral());
		fNWallData->SetNWallTOF(frame->GetTdcCorValue());
		fNWallData->SetNWallZCO(Cal(frame->GetSlowIntegral(),0,1,0)*256./Cal(frame->GetFastIntegral(),0,1,0));
		//frame Output
		fNWallData->SetNWallEnergyFrame(frame->GetEnergy());
		fNWallData->SetNWallTimeFrame(frame->GetTime());
		fNWallData->SetNWallSlowIntegral(frame->GetSlowIntegral());
		fNWallData->SetNWallFastIntegral(frame->GetFastIntegral());
		fNWallData->SetNWallIntRaiseTime(frame->GetIntRaiseTime());
		fNWallData->SetNWallNeuralNetWork(frame->GetNeuralNetWork());
		fNWallData->SetNWallNbZero(frame->GetNbZero());
		fNWallData->SetNWallNeutronFlag(frame->GetNeutronFlag());
		if(debug)cerr<<" Step 2 "<<endl;
		
		lNWall_E=frame->GetSlowIntegral()+frame->GetFastIntegral();
		lNWall_TOF=frame->GetTdcCorValue();
		lNWall_ZCO=Cal(frame->GetSlowIntegral(),0,1,0)*256/Cal(frame->GetFastIntegral(),0,1,0);
		
		if(debug)cerr<<" Step 2.0 "<<endl;
		if(CristalId==tag){
			if(debug)cerr<<" Step 2.1 "<<endl;
			if(BoolSpec)TimeStampDiffInterEvent->Fill(frame->GetTimeStamp()-PrevTStagged);
			PrevTStagged=frame->GetTimeStamp();
		}
		if(debug)cerr<<" Step 2.2 "<< Board<<"  "<< prevBoard<<endl;
		if(BoolSpec)BoardDiff->Fill(Board-prevBoard);
		
		if(debug)cerr<<" Step 3 "<<endl;
		if((frame->GetTimeStamp()-PrevTSlocal)<4){
			if(debug)cout<<"Previous Channel "<<prevChannel  <<" Channel "<< CristalId << " Current TS "<<frame->GetTimeStamp()<<" prev TS "<<PrevTS<<" TSDiff " <<frame->GetTimeStamp()-PrevTS<<endl;
			if(debug)cout<< " ---- Q= " << frame->GetSlowIntegral()<<" Diff Q "<<prevQ- frame->GetSlowIntegral()<< " QF= " << frame->GetFastIntegral()<<" Diff Q "<<prevQ2- frame->GetFastIntegral()<<endl;
			if(BoolSpec)ChannelDiff->Fill(CristalId-prevChannel);
			if(BoolSpec)ChannelDiff2D->Fill(CristalId,prevChannel);
			if(BoolSpec)CrossTalk2D->Fill(frame->GetSlowIntegral()+frame->GetFastIntegral(),prevQ+prevQ2);
		}
		
		if(debug)cout<<"GetSlowIntegral "<<frame->GetSlowIntegral()<<" GetFastIntegral "<<frame->GetFastIntegral()<<endl;
		if(debug)cout<<"lNWall_ZCO "  <<lNWall_ZCO<<" lNWall_TOF "<<lNWall_TOF<<" lNWall_E "<< lNWall_E<<endl;
		
		
		if(BoolSpec)fMyHistoNWallE[MapFinger]->Fill(lNWall_E);
    		if(BoolSpec)fMyHistoNWallPattE2d->Fill(MapFinger,lNWall_E);
    		if(BoolSpec)fMyHistoNWallPattE->Fill(MapFinger);
		
		if(BoolSpec)fMyHistoNWallPattT2d->Fill(MapFinger,lNWall_TOF);
    		if(BoolSpec)fMyHistoNWallPattT->Fill(MapFinger);
		if(BoolSpec)fMyHistoNWallTOF[MapFinger]->Fill(lNWall_TOF);
		
		if(BoolSpec)fMyHistoNWallPattZCO2d->Fill(MapFinger,lNWall_ZCO);
    		if(BoolSpec)fMyHistoNWallPattZCO->Fill(MapFinger);
		if(BoolSpec)fMyHistoNWallZCO[MapFinger]->Fill(lNWall_ZCO);
		
		
		if(BoolSpec)fMyHistoNWallSlowIntegral[MapFinger]->Fill(frame->GetSlowIntegral());
		if(BoolSpec)fMyHistoNWallFastIntegral[MapFinger]->Fill(frame->GetFastIntegral());
		if(BoolSpec)fMyHistoNWallZCO_E[MapFinger]->Fill(lNWall_ZCO,lNWall_E);
	        if(BoolSpec)fMyHistoNWallZCO_TOF[MapFinger]->Fill(lNWall_ZCO,lNWall_TOF);
		if(debug)cout<<"All Hist Filled"<<endl;
		
		prevBoard=Board;
		prevChannel=CristalId;
		prevQ=frame->GetSlowIntegral();
		prevQ2=frame->GetFastIntegral();
		PrevTSlocal=frame->GetTimeStamp();
		prevEvtNumber=frame->GetEventNumber();
	}
	
	
	if(debug)cerr<<" Step Exit "<<endl;
	
	return result;

}
bool TNWall::Is(UShort_t lbl, Short_t ival)
{
  
  bool status = false;
  int  MapFinger;
  switch (fTypeMap[lbl]) {
    
  case NWall_E :{  
    //cout<<  "- ---------< NWall E >------------------!\n";
    //cout<<lbl<<"NWall E "<<ival <<" | "<<fParameterMap[lbl]<<endl;
    MapFinger=fParameterMap[lbl];
    fNWallData->SetNWallEDetNbr(MapFinger);
    fNWallData->SetNWallEnergy(ival);
    if(BoolSpec)fMyHistoNWallE[MapFinger]->Fill(ival);
    if(BoolSpec)fMyHistoNWallPattE2d->Fill(MapFinger,ival);
    if(BoolSpec)fMyHistoNWallPattE->Fill(MapFinger);
    status = true;
    break;
  }
    
  case NWall_T :{  
    //cout<<  "- ---------< NWall T>------------------!\n";
    //cout<<lbl<<"NWall T "<<ival <<" | "<<fParameterMap[lbl]<<endl;
    MapFinger=fParameterMap[lbl];
    fNWallData->SetNWallTOFDetNbr(MapFinger);
    fNWallData->SetNWallTOF(ival);
    //    if(BoolSpec)fMyHistoNWallTOF[MapFinger]->Fill(ival);
    if(BoolSpec)fMyHistoNWallPattT2d->Fill(MapFinger,ival);
    if(BoolSpec)fMyHistoNWallPattT->Fill(MapFinger);
    status = true;
    break;
  }
    
  case NWall_ZCO :{  
    //cout<<  "- ---------< NWall ZCO>------------------!\n";
    //cout<<lbl<<"NWall ZCO "<<ival <<" | "<<fParameterMap[lbl]<<endl;
    MapFinger=fParameterMap[lbl];
    fNWallData->SetNWallZCODetNbr(MapFinger);
    fNWallData->SetNWallZCO(ival);
    //    if(BoolSpec)fMyHistoNWallZCO[MapFinger]->Fill(ival);
    if(BoolSpec)fMyHistoNWallPattZCO2d->Fill(MapFinger,ival);
    if(BoolSpec)fMyHistoNWallPattZCO->Fill(MapFinger);
    status = true;
    break;
  }
    
  case TAC_BaF2_HF :{  
    fNWallData->SetTAC_BaF2_HF(ival);
    if(BoolSpec){
      fMyHistoTAC_BaF2_HF->Fill(ival);
    }
    status = true;
    break;
  }
    
  case TAC_FT_HF :{  
    fNWallData->SetTAC_FT_HF(ival);
    if(BoolSpec){
      fMyHistoTAC_FT_HF->Fill(ival);
    }
    status = true;
    break;
  }
    
  case TAC_FT_CFD :{  
    fNWallData->SetTAC_FT_CFD(ival);
    if(BoolSpec){
      fMyHistoTAC_FT_CFD->Fill(ival);
    }
    status = true;
    break;
  }
  case TAC_CFDft_HF :{  
    fNWallData->SetTAC_CFD_HF(ival);
    if(BoolSpec){
      fMyHistoTAC_CFDft_HF->Fill(ival);
    }
    status = true;
    break;
  }
  case ADCEMPTY :{  
    status = true;
    break;
  }
    
  default:{
    //cout << " Should not Happen : Problem in  TNWall::Is() "<< fTypeMap[lbl]<<endl;
    status = false;
    break;
  }
    
  } // end of switch
  
  return status;
}



bool TNWall::Treat()
{
  Float_t val;
  UShort_t ival;
  Float_t tac_baf2_hf_cal,  tac_ft_hf_cal, tac_cfd_hf_cal,  tac_ft_cfd_cal;
  Float_t tac_baf2_cfd_cal = -1E09;
  Float_t tac_baf2_ft_cal = -1E09;
  Int_t id;

  ival = fNWallData->GetTAC_BaF2_HF();
  if (ival > 300) {
    tac_baf2_hf_cal = Cal(ival, TACCoef_BAFHF[0], TACCoef_BAFHF[1], TACCoef_BAFHF[2]);
  }
  else {
    tac_baf2_hf_cal  = -1E08;
  }
  fNWallData->SetTAC_BaF2_HF_C(tac_baf2_hf_cal);
  fMyHistoTAC_BaF2_HF_cal->Fill(tac_baf2_hf_cal);


  ival =   fNWallData->GetTAC_FT_HF();
  if (ival > 300) {
      tac_ft_hf_cal = Cal(ival,TACCoef_FTHF[0],TACCoef_FTHF[1],TACCoef_FTHF[2]);
  }
  else {
     tac_ft_hf_cal   = -1E08;
  }
  fNWallData->SetTAC_FT_HF_C(tac_ft_hf_cal);
  //-----
  
  fMyHistoTAC_FT_HF_cal->Fill(tac_ft_hf_cal);

  ival =  fNWallData->GetTAC_CFD_HF();
  if (ival > 300) {
    tac_cfd_hf_cal = Cal(ival,TACCoef_CFDHF[0],TACCoef_CFDHF[1],TACCoef_CFDHF[2]);
  }
  else {
     tac_cfd_hf_cal =  -1E08;
  }
  fNWallData->SetTAC_CFD_HF_C(tac_cfd_hf_cal);
  fMyHistoTAC_CFDft_HF_cal->Fill(tac_cfd_hf_cal);

  ival =  fNWallData->GetTAC_FT_CFD();
  if (ival > 300) {
    tac_ft_cfd_cal = Cal(ival,TACCoef_FTCFD[0],TACCoef_FTCFD[1],TACCoef_FTCFD[2]);
  }
  else {
     tac_ft_cfd_cal =  -1E08;
  }
  fNWallData->SetTAC_FT_CFD_C(tac_ft_cfd_cal);
  fMyHistoTAC_FT_CFD_cal->Fill(tac_ft_cfd_cal);

//------
#ifdef TAC_CFD_HF_INDIVIDUAL_DETS
  // the following 4 lines are for NWall CFD time alignment
  if (fNWallData->GetNWallTOFMult() == 1) {
    id = fNWallData->GetNWallTOFDetNbr(0);
    fMyHistoTACtmp[id]->Fill(tac_cfd_hf_cal);
  }
#endif

 //-------  

  if ((tac_baf2_hf_cal>-1E06) && (tac_cfd_hf_cal>-1E06)) {
    tac_baf2_cfd_cal =  tac_baf2_hf_cal -  tac_cfd_hf_cal;
  }
  if (( tac_baf2_hf_cal>-1E06) && (tac_ft_hf_cal>-1E06)) {
    tac_baf2_ft_cal  =  tac_baf2_hf_cal -  tac_ft_hf_cal;
  }
  fMyHistoTAC_BaF2_CFD_cal->Fill(tac_baf2_cfd_cal);
  fMyHistoTAC_BaF2_FT_cal->Fill(tac_baf2_ft_cal);





  for(Int_t j= 0 ; j<fNWallData->GetNWallTOFMult();j++){
    ival = fNWallData->GetNWallTOF(j);
    id = fNWallData->GetNWallTOFDetNbr(j);
    //cout<<id<<endl;
    val = Cal(ival, TOFCoef[id][0], TOFCoef[id][1], TOFCoef[id][2]);
    //cout<<"done"<<endl;
    val = val + fNWallData->GetTAC_CFD_HF_C();
	//cout<<val<<endl;
   /* if(fNWallData->GetTAC_CFD_HF_C()>0){
    	fNWallData->SetNWallTRFDetNbr(id);
    	fNWallData->SetNWallTRF(val);
    }*/
    if(BoolSpec)fMyHistoNWallTRF[id]->Fill(val);
   // cout<<"spec"<<endl;
  }
  
#ifdef TAC_CFD_HF_VS_TRF
  for(Int_t j= 0 ; j<fNWallData->GetNWallTOFMult();j++){
    id = fNWallData->GetNWallTOFDetNbr(j);
    val =  fNWallData->GetNWallTRF(j);
    if(BoolSpec)fMyHistoTAC_CFD_HF_VS_TRF[id]->Fill(val,tac_cfd_hf_cal);
  }
#endif


  for(Int_t i = 0 ; i<fNWallData->GetNWallTOFMult();i++){
    ival = fNWallData->GetNWallTOF(i);
    id =  fNWallData->GetNWallTOFDetNbr(i);
    val = Cal(ival,  TOFCoef[id][0], TOFCoef[id][1], TOFCoef[id][2]);
    fNWallData->SetNWallTOFCalDetNbr(id);
    fNWallData->SetNWallTOFCal(val);

  }

#ifdef ZCOMULT1 
  if (fNWallData->GetNWallZCOMult()==1) {
#endif
    for(Int_t i = 0 ; i<fNWallData->GetNWallZCOMult();i++){
      ival = fNWallData->GetNWallZCO(i);
      id =  fNWallData->GetNWallZCODetNbr(i);
      val = Cal(ival,  ZCOCoef[id][0], ZCOCoef[id][1], ZCOCoef[id][2]);
      fNWallData->SetNWallZCOCalDetNbr(id);
      fNWallData->SetNWallZCOCal(val);
      
    }
#ifdef ZCOMULT1 
  }
#endif

 
  
  fMyHistoNWallEmult->Fill(fNWallData->GetNWallEMult());
  fMyHistoNWallTmult->Fill(fNWallData->GetNWallTOFMult());
  fMyHistoNWallZCOmult->Fill(fNWallData->GetNWallZCOMult());
  
  
  if(fNWallData->GetNWallZCOMult()==fNWallData->GetNWallTOFMult()){GoodCoincTZCO++;}
  else{BadCoincTZCO++;}
  
  if(fNWallData->GetNWallZCOMult()==fNWallData->GetNWallEMult()){GoodCoincQZCO++;}
  else{BadCoincQZCO++;}
  
  if(fNWallData->GetNWallEMult()==fNWallData->GetNWallTOFMult()){GoodCoincQT++;}
  else{BadCoincQT++;}
  
  return true;
}


Int_t TNWall::NNeutrons_zco() {
  Int_t n_neutrons=0;
  for(Int_t j=0; j<fNWallData->GetNWallZCOMult(); j++){
    Int_t i = fNWallData->GetNWallZCODetNbr(j);
    if (fNWallData->GetNWallZCO(i) < 2000) {
      n_neutrons++;
    }
  }	

  if (n_neutrons>3) n_neutrons = 3;
  return n_neutrons;
}


Int_t TNWall::NNeutrons() {
  Float_t x, y;
  Int_t id; 
  //   cout <<" In NNeutrons" << endl;
  Int_t n_neutrons=0;
  for(Int_t i = 0 ; i<fNWallData->GetNWallZCOMult();i++){
    for(Int_t j= 0 ; j<fNWallData->GetNWallTRFMult();j++){
      if((fNWallData->GetNWallZCODetNbr(i))==(fNWallData->GetNWallTRFDetNbr(j))){
	x = fNWallData->GetNWallZCOCal(i); 
	y =  fNWallData->GetNWallTRF(j);
	id = fNWallData->GetNWallZCODetNbr(i);
	//	cout << id << "   " <<  x << "   " << y << endl;
	if(NWall_zco_tof_cut[id]->IsInside(x, y) == 1) {
	  n_neutrons++;
	}
      }
    }
  }
  
  if (n_neutrons > 3) n_neutrons = 3;
  return n_neutrons;
}

Int_t TNWall::NNeutrons_n2() {
  Float_t x, y;
  Int_t id; 
  Int_t neutron_id[50];
  Float_t neutron_trf[50];
  Int_t neutron_q[50];
  //   cout <<" In NNeutrons" << endl;
  Int_t n_neutrons=0;
  memset(neutron_id, 0, sizeof(neutron_id));
  memset(neutron_trf, 0, sizeof(neutron_trf));
  memset(neutron_q, 0, sizeof(neutron_q));

  for(Int_t i = 0 ; i<fNWallData->GetNWallZCOMult();i++){
    for(Int_t j= 0 ; j<fNWallData->GetNWallTRFMult();j++){
      if((fNWallData->GetNWallZCODetNbr(i))==(fNWallData->GetNWallTRFDetNbr(j))){
	x = fNWallData->GetNWallZCOCal(i); 
	y =  fNWallData->GetNWallTRF(j);
	id = fNWallData->GetNWallZCODetNbr(i);
	//	cout << id << "   " <<  x << "   " << y << endl;
	if(NWall_zco_tof_cut[id]->IsInside(x, y) == 1) {
	  //	  cout << " A neutron:" << id << "  " << x << y <<  " n_neutrons: " << n_neutrons << endl;
	  neutron_id[n_neutrons] = id; 
	  neutron_trf[n_neutrons] = y;
	  n_neutrons++;
	}
      }
    }
  }
  if (n_neutrons>1) {
    for (int i_list = 0; i_list<n_neutrons; i_list++) {
      for (Int_t i=0; i<fNWallData->GetNWallEMult(); i++) {
	if (fNWallData->GetNWallEDetNbr(i) ==  neutron_id[i_list]) {
	  neutron_q[i_list] = fNWallData->GetNWallEnergy(i);
	}
      }
    }
    
    Int_t  n_separated, n_scatters, n_clean_neutrons;
    n_separated = n_scatters = n_clean_neutrons = 0;
    Float_t dt;
    for (int i_list=0; i_list<n_neutrons; i_list++) {
      Int_t neutron_id1 = neutron_id[i_list];
      for (int j_list=i_list+1; j_list<n_neutrons; j_list++) {
	Int_t neutron_id2 =  neutron_id[j_list];
	Int_t n_between = GetNbetween(neutron_id1, neutron_id2);
	dt =  neutron_trf[i_list] - neutron_trf[j_list];
	//       cout << "id1: " << neutron_id1  << " id2: " << neutron_id2 << " Nbetween: " << n_between << " dt: " << dt << endl;
	if (IsSeparated( neutron_id1, neutron_q[i_list], 
			 neutron_id2, neutron_q[j_list], dt)) {
	  n_separated++;
   	} 
	else {
	  n_scatters++;
	}
	if (neutron_q[i_list] < neutron_q[j_list])  dt = -dt;
	fMyHistoNWallNbetween_dt[n_between]->Fill(dt);
      }
    }
    //  if (n_separated==1) {
    //  n_clean_neutrons = 2;
    // }
    // else if (n_separated>1) {
    //    n_clean_neutrons = 3;
    n_clean_neutrons = n_neutrons - n_scatters;
    // }
    if ((n_neutrons>0) & (n_clean_neutrons<1)) {
      n_clean_neutrons = 1;
    }
    n_neutrons = n_clean_neutrons;
  }
  if (n_neutrons >3) {
    n_neutrons = 3;
  }
  return n_neutrons;
}



int TNWall::getNumberOfNeutrons() {
  return NNeutrons_n2();
}

bool TNWall::Counter()
{
  
  printf("\033[34m ********* NWall Info :  Missing Coinc ZCO_TOF  = %2.3f percent  (Average 2009 value [1-3])\033[m \n",BadCoincTZCO*100./(BadCoincTZCO+GoodCoincTZCO));
  printf("\033[34m ********* NWall Info :  Missing Coinc ZCO_Q    = %2.3f percent  (Average 2009 value [1-3])\033[m \n",BadCoincQZCO*100./(BadCoincQZCO+GoodCoincQZCO));
  printf("\033[34m ********* NWall Info :  Missing Coinc Q_TOF    = %2.3f percent  (Average 2009 value [1-3])\033[m \n",BadCoincQT*100./(BadCoincQT+GoodCoincQT));
 
  

  if(BoolSpec)fMyHistoNWallDataCheckTZCO->Fill(BadCoincTZCO*100./(BadCoincTZCO+GoodCoincTZCO));
  if(BoolSpec)fMyHistoNWallDataCheckQZCO->Fill(BadCoincQZCO*100./(BadCoincQZCO+GoodCoincQZCO));
  if(BoolSpec)fMyHistoNWallDataCheckQT->Fill(BadCoincQT*100./(BadCoincQT+GoodCoincQT));
  
  return true;
  
}

bool TNWall::ClearCounter()
{
  GoodCoincQT=GoodCoincQZCO=GoodCoincTZCO=BadCoincQT=BadCoincQZCO=BadCoincTZCO=0;
  return true;
  
}

void TNWall::InitBranch(TTree *tree)
{
  tree->Branch("NWall", "TNWallData", &fNWallData,32000,99);
}

bool TNWall::ReadCut() {
  
  //cuts NWall tof vs zco
  cerr<<""<<endl;
  printf("\033[32mInfo::  Reading NEDA TOF vs ZCO cuts (i for individual, c for common): \033[m \n");
  bool zco_tof_cuts;
  bool status=false;
  char generic_name[50];  
  sprintf(generic_name,"./CutNWall/NWall_zco_tof_cut.root");
  zco_tof_cuts=false;
  
  for(Int_t nn=0 ; nn<50; nn++){
    char Cname[50];
    sprintf(Cname,"./CutNWall/NWall_zco_tof_cut%02i",nn);
    if(fopen(Cname,"r")!=NULL) {
      TFile fileCut(Cname,"READ");
      NWall_zco_tof_cut[nn]=(TCutG*)fileCut.Get("neutron");
      fileCut.Close();
      printf("i:%0d\n ",nn);
      zco_tof_cuts = true;
      status=true;
    }
    else if (fopen(generic_name,"r")!=NULL) {
      TFile fileCut(generic_name,"READ");
      NWall_zco_tof_cut[nn]=(TCutG*)fileCut.Get("neutron");
      fileCut.Close();
      printf("c:%0d ",nn);
      zco_tof_cuts = true;
      status=true;
    }
    else {
      cout << "Error reading NWall tof vs zco cuts" << endl;
      zco_tof_cuts = false;
    }
  }
 
 
  return status;
}
 
  
