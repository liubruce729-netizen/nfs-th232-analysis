// File : GEtalonnageMust.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GEtalonnageMust
//
// This class do all data treatment
//
////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                        *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/


#include "General.h"
#include "TSpectrum.h"
#include "TF1.h"
#include "math.h"
#include "TVirtualFitter.h"
#include "GEtalonnageMust.h"
#include "GAcq.h"
#include "TRandom.h"

//_________________________________global_variables______________________________


//______________________________________________________________________________

ClassImp( GEtalonnageMust);

GEtalonnageMust::GEtalonnageMust(GDevice* _fDevIn, GDevice* _fDevOut) {
	// Constructor/initialisator of Acquisition object
	//
	// entry:
	// - Input Device
	// - Output Device
	fDevIn = _fDevIn;
	fDevOut = _fDevOut;
	fCoups = 0;
	fcountevent = 0;
	gROOT->cd();
	fIncrement1 = 0;
	fIncrement2 = 0;
	fVerbose = 1;
	fModeCalib = 1;// 1 =test   2= control    3= rampe  4=rampe periodique
	fFirstIndex = 0;
	fStartedHisto =NULL;
	//fTelein2D =0;
      
	fnRun = 0;

	fMuviEnergy2D = "MUVI_ENERGY_2D";
	fMuviTime2D   = "MUVI_TIME_2D";
        SetPrintRC(true);
	fNameFirstParameter = NULL ; 
	for (int i = 0; i < NB_TELESCOPES*2; i++) {
		fGeneValues[i]=0;
	}
	for (int i = 0; i < NB_MATES*NB_TELESCOPES; i++) {
		fNoChannels[i]=0;
	}

	for (int i = 0; i < NB_TELESCOPES; i++) {
		fIndex2DE[i] = 0;
		fIndex2DT[i] = 0;
		fH2DE[i]=NULL;
		fH2DT[i] =NULL;
		fMM_X_E[i] = NULL;
		fMM_X_T[i] = NULL;
		fMM_Y_E[i] = NULL;
		fMM_Y_T[i] = NULL;
		fMM_Sili_E[i] = NULL;
		fMM_Sili_T[i] = NULL;
		fMM_Csi_E[i] = NULL;
		fMM_Csi_T[i] = NULL;
	}
}

//_____________________________________________________________________________

GEtalonnageMust::~GEtalonnageMust() {
	if (fVerbose > 9)
		fError.Infos( "Delete Etalonnage");
	gROOT->cd();
	
	
	if (fNameFirstParameter) {
	  delete [] fNameFirstParameter;
	  fNameFirstParameter= NULL;
	 }
	 
	  // ne pas delete les histo dans la database
	
	/*
	 if (fTrace2D) {
	 delete (fTrace2D);
	 fTrace2D=NULL;
	 }
	 for (int i =0; i< NB_CHANNEL; i++) {
	 if (fCurtrace[i]) {
	 delete fCurtrace[i];
	 fCurtrace[i]=NULL;
	 }
	 for (int j =0; j<5; j++) {
	 if (fTrace[j][i]) {
	 delete (fTrace[j][i]);
	 fTrace[j][i]=NULL;
	 }
	 }
	 if (fEtalonnage[i]) {
	 delete(fEtalonnage[i]);
	 fEtalonnage[i]=NULL;
	 }
	 if (fHistoVernier[i]) {
	 delete (fHistoVernier[i]);
	 fHistoVernier[i]=NULL;
	 }
	 }
	 */
	// Muvi
	
	
	if (fStartedHisto){
		delete (fStartedHisto);
		fStartedHisto=NULL;
	}
	
	for (int i = 0; i < NB_TELESCOPES; i++) {
		if (fH2DE[i]) {
			delete (fH2DE[i]);
			fH2DE[i]=NULL;
		}
		if (fH2DT[i]){
			delete(fH2DT[i]);
			fH2DT[i]=NULL;
		}
	
		if (fMM_X_E[i]) {
			delete[] (fMM_X_E[i]);
			fMM_X_E[i] = NULL;
		}
		if (fMM_X_T[i]) {
			delete[] (fMM_X_T[i]);
			fMM_X_T[i] = NULL;
		}
		if (fMM_Y_E[i]) {
			delete[] (fMM_Y_E[i]);
			fMM_Y_E[i] = NULL;
		}
		if (fMM_Y_T[i]) {
			delete[] (fMM_Y_T[i]);
			fMM_Y_T[i] = NULL;
		}
		if (fMM_Sili_E[i]) {
			delete[] (fMM_Sili_E[i]);
			fMM_Sili_E[i] = NULL;
		}
		if (fMM_Sili_T[i]) {
			delete[] (fMM_Sili_T[i]);
			fMM_Sili_T[i] = NULL;
		}
		if (fMM_Csi_E[i]) {
			delete[] (fMM_Csi_E[i]);
			fMM_Csi_E[i] = NULL;
		}
		if (fMM_Csi_T[i]) {
			delete[] (fMM_Csi_T[i]);
			fMM_Csi_T[i] = NULL;
		}
	}

}
//______________________________________________________________
void GEtalonnageMust::SetFirstMuviName(char * name){
  int len = strlen(name);
  
  fNameFirstParameter = new char[len+1];
  strcpy(fNameFirstParameter,name);
}
//______________________________________________________________

void GEtalonnageMust::InitCalim(char* exp_name, char* cardname, char*host_name,
		char* para_name, int nbpara, int calimode, char *eventinitmode) {
		// event init mode = MFM or GANIL 
		// calim mode = 1,2,3 

	
	TString tempos;
	GetSpectra()->RazDB();
	EventInit(exp_name, (char*) eventinitmode, false);

	if (strcmp(para_name, "") == 0)
		SetSpectraMode(1);
	else
		SetSpectraMode(1); //SetSpectraMode(1,para_name,nbpara);
		
        SetFirstMuviName(para_name);
	// Get  index of the first raw Muvi parameter ,
	if (para_name != NULL)
		fFirstIndex = GetEvent()->GetDataParameters()->GetIndex(
				fNameFirstParameter);
	else
		fFirstIndex = 0;
	tempos.Form("fFirstIndex = %d for parameter name %s ", fFirstIndex,fNameFirstParameter);
	if (fVerbose>0)fError.TreatError(0, 0, tempos);
	
	SetUp2DHisto();

	if (GetDeviceIn()->GetType() == GT_TYPE_FILE)
		GetDeviceIn()->Rewind();
	
	tempos.Form("Init Multi  Run on card %s", cardname);
	GetError()->Infos(tempos);
	SetModeCalib(calimode);
	InitUser();
}

//______________________________________________________________

void GEtalonnageMust::SetUp2DHisto() {
// set up 2D histograms 
   TString tempo,name;

   if (fVerbose>0)fError.Infos("Set up 2D Histo");

   // Definition of a 2D histogram for only 1 telescope 

   int nb_channel = NB_CHANNELS_PER_MATE * NB_MATES ; // limite a un seul tele l'histogramme.
 
   fTelein2D = 0;// numero wanted tele on fH2D (0->3)
   for (int i = 0; i < NB_TELESCOPES; i++) {
	if (fH2DE[i] == NULL) {
   		name.Form("%s_%d",fMuviEnergy2D.Data(),i);
		fH2DE[i] = new TH2S(name, name, nb_channel, 0, nb_channel, CHANNEL_SIZE, 0, CHANNEL_SIZE);
		fIndex2DE[i] = GetSpectra()->AddSpectrum(fH2DE[i]);
	} else {
		fIndex2DE [i]= GetSpectra()->GetDB()->GetIndex(fH2DE[i]);
	}
	tempo.Form("fIndex2DE inDB = %d", fIndex2DE[i]);
	if (fVerbose>0)fError.TreatError(0, 0, tempo);

	if (fH2DT[i] == NULL) {
		name.Form("%s_%d",fMuviTime2D.Data(),i);
		fH2DT[i] = new TH2S(name, name, nb_channel , 0,
				nb_channel, CHANNEL_SIZE, 0, CHANNEL_SIZE);
		fIndex2DT[i] = GetSpectra()->AddSpectrum(fH2DT[i]);
	} else {
		fIndex2DT[i] = GetSpectra()->GetDB()->GetIndex(fH2DT[i]);
	}
	tempo.Form("fIndex2DT inDB = %d", fIndex2DT[i]);
	if (fVerbose>0)fError.TreatError(0, 0, tempo);
}
	name.Form("StartedHisto");

	if (fStartedHisto == NULL) {
		fStartedHisto = new TH1S(name, name, GetEventArraySize(), 0,GetEventArraySize());
		fIndexStartedHisto = GetSpectra()->AddSpectrum(fStartedHisto);
	} else {
		fIndexStartedHisto = GetSpectra()->GetDB()->GetIndex(fStartedHisto);
	}
}
//______________________________________________________________

void GEtalonnageMust::InitUser() {
	// Global Initialisation for specific user treatement
	fnRun = 0;
	fcountevent = 0;
   
	if (fVerbose >0) {
		fError.TreatError(0, 0, " -----------------GEtalonnageMust:InitUser Muvi----------------");
		}
	
	GetSpectra()->SetStopped((char*) "all");
	GetSpectra()->RazDB(); 

	// init memorisation of value of gene
	for (int i = 0; i < NB_TELESCOPES * ENERGIE_AND_TIME; i++) {
		for (int j = 0; j < NB_MAX_OF_PICS; j++) {
			fMemoGeneValues[i][j] = 0;
		}
	}
}

//______________________________________________________________
void GEtalonnageMust::InitUserRun() {
	// Initialization only before each run
	// For specific user treatment

	TString tempo;
	fIncrement1 = 0;
	fIncrement2 = 0;
	fnRun++;

	//------------- test labo2 --------------------------
	tempo.Form("---------------GEtalonnageMust:InitUserRun no = %d --------------", fnRun);
	if (fVerbose >0) fError.TreatError(0, 0, tempo);
	usleep(5000); // wait during InitUserRun for a while for be sur we have events
	if (GetDeviceIn()->GetType() == GA_NET_CLIENT) {
		if (GetDeviceIn()->GetIsOpen())
			GetDeviceIn()->ReadBuffer(); //just to empty current buffer
		else {
			GetDeviceIn()->Open();
			GetDeviceIn()->ReadBuffer();
			GetDeviceIn()->Close();
		}
	}
	GetSpectra()->SetStopped((char*) "all");
	// start 2D spectra
	GetSpectra()->SetStarted(  fIndexStartedHisto);
	GetSpectra()->SetStarted(0);
	GetSpectra()->GetHisto(  fIndexStartedHisto)->Fill(0); 
	GetSpectra()->GetHisto(  fIndexStartedHisto)->Fill(  fIndexStartedHisto); 
	for (int i = 0; i < NB_TELESCOPES; i++) {
		GetSpectra()->SetStarted(fIndex2DE[i]);
		GetSpectra()->SetStarted(fIndex2DT[i]);
		GetSpectra()->GetHisto(  fIndexStartedHisto)->Fill(fIndex2DE[i]);
		GetSpectra()->GetHisto(  fIndexStartedHisto)->Fill(fIndex2DT[i]);
	}
	if(fVerbose>3) fError.Infos(tempo);
	int x_or_y, index_E, index_T;
	// start 2D  spectra of selected channels
	for (int i = 0; i < NB_MATES * NB_TELESCOPES; i++) {
		index_E   = ConvertInIndex(0, i);
		index_T   = ConvertInIndex(1, i);

		if (index_E >= 0) {
			GetSpectra()->SetStarted(index_E);
			GetSpectra()->SetStarted(index_T);
			GetSpectra()->GetHisto(  fIndexStartedHisto)->Fill(index_E);
			GetSpectra()->GetHisto(  fIndexStartedHisto)->Fill(index_T);
			tempo.Form("Start Index :%d  & %d  Name : %s & %s  Started %d",
					index_E,index_T,
					GetEvent()->GetDataParameters()->GetParNameFromIndex(index_E),
					GetEvent()->GetDataParameters()->GetParNameFromIndex(index_T),
					GetSpectra()->IsStarted(index_E));
			if(fVerbose>3)fError.Infos( tempo);
		}
	}
	if(fVerbose>3)fError.TreatError(0, 0, "Value of gene used [telescope][runnumber] around the piedestal Yvalue<8192<Xvalue");
	// memorisation of gene value 
	for (int i = 0; i < NB_TELESCOPES * 2; i++) {
		x_or_y = i % 2;
		char tempochar[16];
		if (x_or_y) strcpy (tempochar,"X");
		else strcpy (tempochar,"Y");
		// we memorise  only values differents of the already memorized
		int calcul_value = fGeneValues[i] / 2 - (CHANNEL_SIZE/2) * (x_or_y - 1);
		fMemoGeneValues[i][fnRun - 1] = calcul_value;
		tempo.Form("  Value %s gene [%d][%d]= %d ",tempochar, i, fnRun - 1, calcul_value);
		if(fVerbose>3)fError.Infos(tempo);
	}
}

//______________________________________________________________
void GEtalonnageMust::User() {
  // User method for specific treatement
  //
  // Treatement of event

  TString tempos;  
  bool filtrage = true; 
  
  int nb_coup_par_point =10;
  bool flagcr= false;
  static bool info_point = true;
  fIncrement1++;
  if ((info_point) &&(fVerbose >0)){
 	tempos.Form("----------Nb of event for each spot %d --------",nb_coup_par_point );
	fError.TreatError(0, 0, tempos);
	info_point = false;
  }
  // can be not use  , default is =1
  //if (fcountevent>100)
  // display point on screen
  if (fVerbose>0){
	if ((fIncrement1 % nb_coup_par_point) == 0) { //
		cout << ".";
		cout.flush();
		fIncrement2++;
		flagcr=false;
	}
	if (fIncrement2 >= 80) {  // 80 points par ligne
		cout << "\n";
		fIncrement1 = 0;
		fIncrement2 = 0;
		flagcr=true;
	}
  }
	
  Int_t i=0, index_E=0,index2D=0,index_T=0,  mate=0, tele=0, channel=0, energy_or_time=0;
  UShort_t pied = CHANNEL_SIZE/2;
  UShort_t filtremin = 0;
  UShort_t filtremax = 0;
  UShort_t value_E   = 0;// current value of energie
  UShort_t value_T   = 0;// current value of time
  bool X = false; //  is a X value (true) or  Y value (false)
  bool Y = false; //  is a Y value (true) or  X value (false)
  filtremin = 30;
  filtremax = 30;

  // on scan tous les mates des 4 telescope
  for (i = 0; i < NB_MATES * NB_TELESCOPES; i++) {
	index_E   = ConvertInIndex(0, i);
	index_T   = ConvertInIndex(1, i);
	// if index<0 is mean that channel is not selected;
		
	if (index_E>=0){ // if channel si selected
		value_E = GetEventArray()[index_E];
		value_T = GetEventArray()[index_T];
		ConvertInverse(index_E, &tele, &mate, &channel, &energy_or_time);
		index2D   = mate*NB_CHANNELS_PER_MATE + channel;
		if ((mate < 8) or ( mate == 16)) 
			{X = true;Y = false;}
		else 
			{Y = true;X=false;}
		if (filtrage){
			// filtrage du pidestal pour les spectres brutes
			if ((value_T > pied - filtremin) && (value_T < pied + filtremax)) {GetEventArray()[index_T] = 0;value_T=0;}
			if ((value_E > pied - filtremin) && (value_E < pied + filtremax)) {GetEventArray()[index_E] = 0;value_E=0;}
			// filtrage des parasites en "negatif" en energy
			if ((X == true) && (value_E < pied)) {GetEventArray()[index_E] = 0;value_E=0;}
			if ((Y == true) && (value_E > pied)) {GetEventArray()[index_E] = 0;value_E=0;}
			// filtrage des parasites en "negatif" en temp
			if ( (X==true)  && (value_T < pied)) {GetEventArray()[index_T]= 0 ;value_T=0;}
			if ( (Y==true)  && (value_T > pied)) {GetEventArray()[index_T]= 0 ;value_T=0;}
		}
		if (GetSpectra()->IsStarted(fIndex2DE[tele])) {
			if ((value_E != 0) && (fH2DE[tele]))
					fH2DE[tele]->Fill(index2D, value_E);
			} 
		if (GetSpectra()->IsStarted(fIndex2DT[tele])) {
				if ((value_T!= 0) && (fH2DT[tele]))
					fH2DT[tele]->Fill(index2D, value_T);
			} //time
		}// if (index_E>=0){
	}
	if (flagcr) cout <<"\n"<<flush;
}

//______________________________________________________________
void GEtalonnageMust::EndUserRun() {
  // excuted a end of run

  // ajouter un status de retour vers soap.
  TString tempos;
  int i, j = 0;
  tempos.Form("----------GEtalonnageMust::EndUserRun------- Events Counted Done: %d", fCoups);
  if(fVerbose>0)fError.TreatError(0, 0, tempos);
  
  GetSpectra()->SetStopped((char*) "all");
  
  // 
  if(fVerbose>7){
     for (i = 0; i < NB_TELESCOPES; i++) {
  	cout << "Tele no : " << i << "\n";
  	cout << "   Val gene X: " << fGeneValues[i * 2]  ;
  	cout << "   Mate de " << 1 << " a " << (NB_MATES / 2 -1)<< "  Voies respectives :"<<endl;
 	for (j = 0; j < (NB_MATES / 2 - 1); j++){
  		cout << "      No de voie " << fNoChannels[i * NB_MATES + j] - 1 << "=>" << ConvertInIndex(0, i * NB_MATES + j) << " "
  			<< GetEvent()->GetDataParameters()->GetParNameFromIndex(ConvertInIndex(0, i * NB_MATES + j))<<endl;
  	}
  	cout << "   Val gene Y: " << fGeneValues[i * 2 + 1] << "\n";
  	cout << "   Mate de " << j << " a " << NB_MATES - 2 << "  Voies respectives :  "<<endl;
  	for (j = NB_MATES / 2 - 1; j < (NB_MATES - 2); j++)
  		cout <<  "      No de voie " << fNoChannels[i * NB_MATES + j] - 1 << "=>"
  				<< ConvertInIndex(0, i * NB_MATES + j) << " "
  				<< GetEvent()->GetDataParameters()->GetParNameFromIndex(
  						ConvertInIndex(0, i * NB_MATES + j))<<endl;;

  	cout << "   Mate Sili  :" << NB_MATES - 1 << " Val gene : " << fGeneValues[i * 2] << " Voie : "
  			<< fNoChannels[i * NB_MATES + NB_MATES - 2] - 1 << "=>"
  			<< ConvertInIndex(0, i * NB_MATES + NB_MATES - 2) << " "
  			<< GetEvent()->GetDataParameters()->GetParNameFromIndex(
  					ConvertInIndex(0, i * NB_MATES + NB_MATES - 2))<< endl;
  	cout << "   Mate Csi   :" << NB_MATES << " Val gene : " << fGeneValues[i * 2 + 1]<< " Voie : " << fNoChannels[i
  			* NB_MATES + NB_MATES - 1] - 1 << "=>" << ConvertInIndex(0, i
  		* NB_MATES + NB_MATES - 1) << " "
  			<< GetEvent()->GetDataParameters()->GetParNameFromIndex(
  		ConvertInIndex(0, i * NB_MATES + NB_MATES - 1)) << endl;
     }
  }//end of if(fVerbose>7)
  
}
//______________________________________________________________
void GEtalonnageMust::EndUser() {
// find pics and compute calibration coefficients on a polynome of 2nd degree
// the result is stored in a xml file
      TString tempos;

//Creating the gui canvases 


TCanvas myCanvas("test","test",0,0,1000,700);
myCanvas.cd();
myCanvas.Update();
myCanvas.Draw();


      if(fVerbose>0)tempos.Form("----------GEtalonnageMust::EndUser----------- with %d events and %d runs",
                  fCoups, fnRun);
      fError.TreatError(0, 0, tempos);
      int i;
     // TH1S* h;
      TH1D* hE;
  //    TH1D* hT;
      char tempochar[200];
      Float_t parapoly[4];
      Int_t nb_cof_poly;
      Float_t tempof;
      char typepoly[10];
      int nb_of_runsfor_each_channel;
      Int_t Nx, j, k,l;
      Nx = 0;
      strcpy (typepoly,"");
       /* for (i=0 ; i<10;i++){
              typepoly[i] =0;
              parapoly[i] =0;
        } */ 
      fXmlFile.InitXml(true, "Etalonnage_Must2");

      gROOT->cd();
      //Calcul des coefficients

      if(fVerbose>7){
      fError.Infos("--------------Memorized Gene Values-------------------");
      tempos = "";
      for (i = 0; i < NB_TELESCOPES * 2; i++) {
            for (int j = 0; j < fnRun; j++)
                  tempos.Form("%s %f", tempos.Data(), fMemoGeneValues[i][j]);
            fError.TreatError(0, 0, tempos);
            tempos = "";
      }

      fError.TreatError(0, 0, "---------------Sorted Memorized Gene Values--------");
      }// end of if(fVerbose>7)
      // we put in order values of gene.
      for (i = 0; i < NB_TELESCOPES * 2; i++) {
            for (j = 0; j < fnRun; j++) {
                  for (k = j; k < fnRun; k++) {
                        if (fMemoGeneValues[i][j] > fMemoGeneValues[i][k]) {
                              tempof = fMemoGeneValues[i][j];
                              fMemoGeneValues[i][k] = tempof;
                        }
                  }
            }
      }
      tempos = "";
      if(fVerbose>7){
      // and remove double values
      for (i = 0; i < NB_TELESCOPES * 2; i++) {
            for (int j = 0; j < fnRun; j++)
                  tempos.Form("%s %f", tempos.Data(), fMemoGeneValues[i][j]);
            fError.TreatError(0, 0, tempos);
            tempos = "";
      }
      if(fVerbose>7)fError.Barre();
      }// end of if(fVerbose>7)      
      int indice = 0;
      for (i = 0; i < NB_TELESCOPES * 2; i++) {
            indice = 0;
            for (j = 1; j < fnRun; j++) {
                  tempof = fMemoGeneValues[i][j];
                  if (fMemoGeneValues[i][indice] != tempof) {
                        indice++;
                        fMemoGeneValues[i][indice] = tempof;
                  }
                  if (indice != j)
                        fMemoGeneValues[i][j] = 0;
            }
      }
      tempos = "";
      for (i = 0; i < NB_TELESCOPES * 2; i++) {
            for (int j = 0; j < fnRun; j++)
                  tempos.Form("Generator Values =  %f", fMemoGeneValues[i][j]);
                  if(fVerbose>7)fError.TreatError(0, 0, tempos);
            tempos = "";
      }
      if(fVerbose>7) fError.Barre();

      // recherche des pics
      // principe , nous cherchons autour des valeurs donnée  par DAS ou GECO les caracteristiques au 2 eme ordre des pics sur le spectre
      // ensuite nous eccrivons cela dans un fichier XML 
      int tele, mate, channel, energy_or_time,index2D;
      for (i = fFirstIndex; i < NB_TELESCOPES * NB_MATES * NB_CHANNELS_PER_MATE
                  * ENERGIE_AND_TIME + fFirstIndex; i++) {
            nb_of_runsfor_each_channel = (int) fStartedHisto->GetBinContent(i + 1); // +1 because of convention  of TH1
            ConvertInverse(i, &tele, &mate, &channel, &energy_or_time);
            index2D   = mate*NB_CHANNELS_PER_MATE + channel;
            Float_t values_gene[nb_of_runsfor_each_channel];
            for (l=0;l<nb_of_runsfor_each_channel;l++)values_gene[l]=0;
            if ((nb_of_runsfor_each_channel > 0) && (energy_or_time == 0)) {

                  Float_t value_gene;
                  if (fVerbose>7){
                        cout << "---------- Calcul sur voie-" << i << "--Energie or Time-"
                              << energy_or_time << "----------  \n";
                        cout << " Name : "
                              << GetEvent()->GetDataParameters()->GetParNameFromIndex(i)
                              << "\n";
                              
                        cout << " Tele    : " << tele << "  Mate   : " << mate << "\n";
                        cout << " Channel : " << channel << "  E or T : " << energy_or_time
                              << "\n";
                  }
                  nb_cof_poly = nb_of_runsfor_each_channel;
                  if (nb_cof_poly > 3)
                        nb_cof_poly = 3;
                  strcpy(typepoly, "");
                  sprintf(typepoly, "pol%d", nb_cof_poly);
                  // search in raw spectra or in 2D spectra where we do a Projection in Y 
                  // h = (TH1S*) GetSpectra()->GetHisto(i);   to do it in standart raw database
                  // dans un premier temps le travail ne se fait que sur l'energie et non le temps
                  hE = fH2DE[tele]->ProjectionY("myproject",index2D+1,index2D+1);// to do it un  2D spectra (+1 car nous sommes en bin de root) 
                  //hT
                  if (hE) {
                        Nx = (hE->GetNbinsX());
                  }
                  if (Nx ==0) Nx =1;
                  TH1F hf("Courbe", "Courbe", Nx, 0, Nx);
		  TF1 f2("MyFunct", typepoly, (Float_t) 100, (Float_t) 12810);
                  for (int j = 0; j <= 3; j++)
                        parapoly[j] = 0;
                  value_gene = 0;

                  TSpectrum equation(nb_of_runsfor_each_channel * 2 + 1);

                  Int_t nfound = 0;
                  Float_t threshold = 0.2;
                  Float_t sigma = 1;
                  nfound = equation.Search(hE, sigma, "goff", threshold);

                  if (fVerbose>7)cout<< " nombres de pics = " << nfound<<" nombre de runs = "<<nb_of_runsfor_each_channel<<  "\n";
                  float pics_in_order[nfound];

                  if (nfound != nb_of_runsfor_each_channel) {
                        if (fVerbose>7)cout << " nombre de runs differents du nombre de pics : "
                                    << nb_of_runsfor_each_channel << " !=" << nfound
                                    << "\n";
                        for (Int_t entier = 0; entier < nfound; entier++) {
                              pics_in_order[entier] = equation.GetPositionX()[entier];
                        }

                        for (Int_t entier = 0; entier < nfound; entier++) {
                              for (Int_t entier1 = entier; entier1 < nfound; entier1++) {
                                    if (pics_in_order[entier] > pics_in_order[entier1]) {
                                          tempof = pics_in_order[entier];
                                          pics_in_order[entier] = pics_in_order[entier1];
                                          pics_in_order[entier1] = tempof;
                                    }
                              }
                        }

                  } else {

                        for (Int_t entier = 0; entier < nb_of_runsfor_each_channel; entier++) {
                              pics_in_order[entier] = equation.GetPositionX()[entier];
                        }
                        if (fVerbose>7)cout << " Calcul sur voie " << i << "\n";
                        for (Int_t entier = 0; entier < nb_of_runsfor_each_channel; entier++) {
                              for (Int_t entier1 = entier; entier1 < nfound; entier1++) {
                                    if (pics_in_order[entier] > pics_in_order[entier1]) {
                                          tempof = pics_in_order[entier];
                                          pics_in_order[entier] = pics_in_order[entier1];
                                          pics_in_order[entier1] = tempof;
                                    }
                              }
                        }
                        hf.Reset();

                        for (Int_t entier = 0; entier < nb_of_runsfor_each_channel; entier++) {
                              if (fVerbose>7)cout << "peak no :" << entier << "="
                                          << pics_in_order[entier] << " for mate " << mate
                                          << " for  tele " << tele;
                              if (((mate >= 0) && (mate < NB_MATES / 2 - 1)) || (mate
                                          < NB_MATES - 2))
                                    value_gene = fMemoGeneValues[tele * 2 + 0][entier];
                              else
                                    value_gene = fMemoGeneValues[tele * 2 + 1][entier];
                              values_gene[entier] = value_gene;
                              if (fVerbose>7)cout << "  value gene " << value_gene << endl;
                              hf.Fill(value_gene, pics_in_order[entier]);
                        }
   
                        hf.Fit(&f2, "N");

                        for (int j = 0; j <= nb_cof_poly; j++) {
                              parapoly[j] = f2.GetParameter(j);
           
                        }
                           if (fVerbose>7) {cout << " Poly : "; for (int j = 0; j <= nb_cof_poly; j++) {  cout << " a[" << j << "] = " << parapoly[j] ;} cout<< endl;}
                       //fin de calcul
                        
 	
                        gROOT->cd();

                  } // end  if (nfound!= nb_of_runsfor_each_channel)
                 		if (fVerbose>=10){
                  	// display current histogram to verify   // comment when no usat
		  	sprintf(tempochar ,"Parameter %d tele %d mate %d channel %d index2D %d",i,tele,mate,channel, index2D); 
		  	myCanvas.cd(); 
		 	 myCanvas.SetTitle(tempochar);                
		  	 hE->Draw();
		  	 f2.Draw("same");
		  	 //hf.Draw();
		 	 myCanvas.Update();
 		 	 myCanvas.Modified();
 		 	 printf("Enter 'enter' to continue\n"); getchar();
  		 	 myCanvas.Update();
  		  	}
                  fXmlFile.SaveParameter(i,(GetEvent()->GetDataParameters()->GetParNameFromIndex(i)),
                              nb_of_runsfor_each_channel, values_gene, parapoly[0],
                              parapoly[1], parapoly[2], parapoly[3], mate, channel, tele,
                              pics_in_order, nfound);
                  gROOT->cd();
            }//end  if (fStartedHisto->GetBinContent(i)>0)
            gROOT->cd();
      }// end for (i=
      fXmlFile.CloseXml();
}

//______________________________________________________________
void GEtalonnageMust::SetVectors(int* gene_value_of_mat, int* selected_channel) {
      // set value gene  to our vector from extern request
      // set -1,0, or 1 on each mat the no channel selected (1->16)
      // gene_value_of_mat is vecor of 8 values ( Nb telescope * 2) containing the value of generateur in X and ( negative ) in Y
      // selected_channel is a vector of 72 values representing le selected channel (1->16) else -1 or 0
      
      int i;
      for (i = 0; i < NB_TELESCOPES * 2; i++) {
            fGeneValues[i] = gene_value_of_mat[i];
            
      }
      for (i = 0; i < NB_MATES * NB_TELESCOPES; i++) {
            fNoChannels[i] = selected_channel[i];
      }
}
//______________________________________________________________
void GEtalonnageMust::SetVectorsStandart() { // 
      // fill vector by  random value do to test 
      // see :SetVectors(int* gene_value_of_mat, int* p_channel) for explanation of rest
      int i;
      for (i = 0; i < NB_TELESCOPES * 2; i++) {
            fGeneValues[i] = 8000+i;
      }
      for (i = 0; i < NB_MATES * NB_TELESCOPES; i++) {
            fNoChannels[i] = 1; // nous les validons toutes 1 ere voie de chaque mat
            //cout <<"debug fNoChannels["<<i<<"] = "<<fNoChannels[i]<<"\n";
            //if (i% NB_TELESCOPES == NB_TELESCOPES-1 )cout<<"\n";
      }
}
//______________________________________________________________
int GEtalonnageMust::ConvertInIndex(int EnergieOrTime, int NoMat) {
      // return  index of parameters table(ACTIONS_XXXX.CHC_PAR)   in fonction (Engergie or Time (0-1) and no of mat on 1 card (0-71)
      //  the 8 firsts mats are X silicium strips ( 128 channels => 256 parameters ( Energy/time))
      //  the mat 17 is Sili detector (16 ways=>32 parameters ( Energy/time ways))
      // the 9to 16 mats are Y silicium strips ( 128=> 256 parameters ( Energy/time ways)
      //  the mat 18 is Csi detector (16 ways => 32 parameters ( Energy/time ways))
      // we suppose that all muvi parameters ares in the same index range in  ACTIONS_XXXX.CHC_PAR file
      //cout <<"debug ConvertInIndex mate = "<<mate <<" fNoChannels["<<NoMat<<"] =  "<<fNoChannels[NoMat]<<"\n";
      // fNoChannels[NoMat] is given [1-16]
      // if fNoChannels[NoMat] is -1 or 0 means that channel is not selected ( we can selec only 1/16 )
      
      if ((NoMat >= 0) && (EnergieOrTime >= 0) && (fNoChannels[NoMat] > 0))
            return ((((NoMat) * NB_CHANNELS_PER_MATE)
                        + (fNoChannels[NoMat] - 1)) * 2 + EnergieOrTime % 2 + (fFirstIndex));
      else
      //cout <<"debug ConvertInIndex mate = "<<mate <<" fNoChannels["<<NoMat<<"] =  "<<fNoChannels[NoMat]<<"\n";
      return (-1);
}
//______________________________________________________________
int GEtalonnageMust::ConvertInIndexinDB(int EnergieOrTime, int NoMat) {
      // return index of spectra table
      return ConvertInIndex(EnergieOrTime, NoMat) - fFirstIndex;
}
//______________________________________________________________
void GEtalonnageMust::ConvertInverse(int index, int* tele, int* mate,
            int *channel, int *energy_or_time) {
      // convertion inverse of ConvertInIndex
      // tele = no telescope
      // mate = no mate
      // index = index of parameter in parameter list
      
      Int_t u = index - fFirstIndex;

      *energy_or_time = u % 2; // 0= energie,1 = temps

      u = u / 2;

      *tele = u / (NB_CHANNELS_PER_MATE * NB_MATES);
      u = u - ((*tele) * (NB_CHANNELS_PER_MATE * NB_MATES));

      *mate = u / NB_CHANNELS_PER_MATE;
      u = u - ((*mate) * (NB_CHANNELS_PER_MATE));
      *channel = u;
      
      //cout<<" debug  GEtalonnageMust::ConvertInverse index =" <<index<< " tele = "<< *tele << " mate = " << *mate << " channel = " << *channel << " energy_or_time = " << *energy_or_time <<endl; 

}
//______________________________________________________________
void GEtalonnageMust::ConvertInverseFromDB(int indexDB, int* tele, int* mate,
            int *channel, int *energy_or_time) {
      // convertion inverse of ConvertInIndex

      Int_t u = indexDB;

      *energy_or_time = u % 2; // 0= energie,1 = temps

      u = u / 2;

      *tele = u / (NB_CHANNELS_PER_MATE * NB_MATES);
      u = u - ((*tele) * (NB_CHANNELS_PER_MATE * NB_MATES));

      *mate = u / NB_CHANNELS_PER_MATE;
      u = u - ((*mate) * (NB_CHANNELS_PER_MATE));
      *channel = u;

}
//______________________________________________________________________________

void GEtalonnageMust::InitTTreeUser() {
      // User method for specfic initialisation of TTree
      // It can be usefull for example multi-hit detections
      // or to have a TTree with only few parameters ( for low compute)
      // to run this method , you have to slect mode 3 in  SetTTreeMode
      // ex : a->SetTTreeMode(3,"/space/MyTTree.root");
}
