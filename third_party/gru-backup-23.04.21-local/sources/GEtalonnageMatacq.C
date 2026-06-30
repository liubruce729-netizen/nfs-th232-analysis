// File : GEtalonnageMatacq.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GEtalonnageMatacq
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
#include "GEtalonnageMatacq.h"
#include "GAcq.h"
#include "TRandom.h"

//_________________________________global_variables______________________________


//______________________________________________________________________________

ClassImp( GEtalonnageMatacq);

GEtalonnageMatacq::GEtalonnageMatacq(GDevice* _fDevIn, GDevice* _fDevOut) {
	// Constructor/initialisator of Acquisition object
	//
	// entry:
	// - Input Device
	// - Output Device
	fDevIn = _fDevIn;
	fDevOut = _fDevOut;
	fCoups = 0;
	fNameFirstParameter = new char[MAX_CARACTERES];
	gROOT->cd();
	fIncrement1 = 0;
	fIncrement2 = 0;
	fVerbose = 1;
	fModeCalib = 1;// 1 =test   2= control    3= rampe  4=rampe periodique
	fFirstIndex = 0;

	fLabelMask = 0;
	fLabelNbCol = 0;
	fLabelTrigRec = 0;
	fLabelPostTrig = 0;
	fLabelTrace = 0;
	fLongTrace = 0;
	fCurrentNumberChannel = 0;
	fCurrentTrace = 0;
	fCurrent2D = 0;
	fHistory = 0;
	fnRun = 0;

	fNbOfWarning = 0;

	for (int i = 0; i < NB_CHANNEL; i++) {
		fVernierMax[i] = 0;
		fVernierMin[i] = 0;
	}

	fMatacqPostrig = "";
	fMatacqTrigRec = "";
	fMatacqNbCol = "";
	fMatacqMask = "";
	fMatacqVernierbase = "";

	/*
	 MATACQ_VERNIER1                        39                  15
	 MATACQ_VERNIER2                        40                  15
	 MATACQ_VERNIER3                        41                  15
	 MATACQ_VERNIER4                        42                  15
	 MATACQ_MASK                            43                  15
	 MATACQ_NCOL                            44                  15
	 MATACQ_POSTTRIG                        45                  15
	 MATACQ_TRIGREC
	 */
}

//_____________________________________________________________________________

GEtalonnageMatacq::~GEtalonnageMatacq() {
	if (fVerbose > 1)
		fError.TreatDebug(1, 0, "Delete Etalonnage");
	gROOT->cd();
	// ne pas delete sur les histo dans la database
	// Matacq
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

	if (fVerbose > 1)
		fError.TreatDebug(fVerbose, 0, "Delete Etalonnage");
}

//______________________________________________________________

void GEtalonnageMatacq::InitEtalonnage(char* name_para) {

	TString tempos;
	fError.TreatError(0, 0, " ---Etalonnage init for Matacq card---");
	int len = strlen(name_para);
	strncpy(fNameFirstParameter, name_para, len - 1);
	fNameFirstParameter[len - 1] = '\0';
	TString tempo;
	tempo.Form("NameFirstParameter = %s", name_para);
	;
	fError.TreatError(0, 0, tempo);
	tempos.Form("Base name for parameter name = %s", name_para);
	fError.TreatError(0, 0, tempo);

}

//_________________________________________________________________________________________

void GEtalonnageMatacq::InitCalim(char* exp_name, char* cardname, char*host_name,
		char* para_name, int nbpara, int calimode, char *eventinitmode) {
		// event init mode = MFM or GANIL 
		// calim mode = 1,2,3 

	
	TString tempos;
	GetSpectra()->RazDB();
	EventInit(exp_name, (char*) eventinitmode, false);

	if (strcmp(para_name, "") == 0)
		SetSpectraMode(1);
	else
		SetSpectraMode(1); // Etalon->SetSpectraMode(1,para_name,nbpara);

	InitEtalonnage(para_name);

	if (GetDeviceIn()->GetType() == GT_TYPE_FILE)
		GetDeviceIn()->Rewind();
	
	tempos.Form("Init Multi  Run on card %s", cardname);
	GetError()->Infos(tempos);
	SetModeCalib(calimode);
	InitUser();
}
//______________________________________________________________

void GEtalonnageMatacq::InitUser() {
	// Global Initialisation for specific user treatement
	fnRun = 0;

	//Matacq
	fError.TreatError(0, 0, " -----------------InitUser Mataq----------------");
	fHistory = 20;
	fLongTrace = 128 * 20;
	fCurrent2D = 0;
	fCurrentTrace = 0;
	fHistoryTrace = 5;
	cout << fNameFirstParameter << "\n";
	TString tempos;
	tempos.Form("%s%d", fNameFirstParameter, 0);
	Int_t IndexBase = GetEvent()->GetDataParameters()->GetIndex(tempos.Data());
	cout << tempos.Data() << "  Index = " << IndexBase << "\n";
	;
	TString name, nameVernier;
	int nbits;
	for (int i = 0; i < NB_CHANNEL; i++) {
		name = "";
		name.Form("CurrentTrace%d", i);
		fCurtrace[i] = new TH1I(name.Data(), name.Data(), fLongTrace, 0,
				fLongTrace);
		GetSpectra()->AddSpectrum(fCurtrace[i], (char*) "User");
		name.Form("Etalon%d", i);
		fEtalonnage[i] = new TH1F(name.Data(), name.Data(), fLongTrace, 0,
				fLongTrace);
		GetSpectra()->AddSpectrum(fEtalonnage[i], (char*) "User");

		name.Form("Vernier_%d", i);

		nameVernier = GetEvent()->GetDataParameters()->GetParNameFromIndex(IndexBase + 4
				+ i);

		nbits = (int) pow(2., GetEvent()->GetDataParameters()->GetNbits(
				nameVernier.Data()));

		fHistoVernier[i] = new TH1F(name.Data(), name.Data(), nbits, 0, nbits);
		GetSpectra()->AddSpectrum(fHistoVernier[i], (char*) "User");
		fVernierMax[i] = 0;
		fVernierMin[i] = (Float_t) nbits;
		fDeltaT[i] = 0;
		fLabelVernier[i] = GetEvent()->GetDataParameters()->GetLabel(IndexBase
				+ 4 + i);

		for (int j = 0; j < fHistoryTrace; j++) {
			name.Form("Trace%d_%d", j, i);
			fTrace[j][i] = new TH1I(name.Data(), name.Data(), fLongTrace, 0,
					fLongTrace);
			GetSpectra()->AddSpectrum(fTrace[j][i], (char*) "User");
		}
	}
	fMatacqPostrig = "";
	fMatacqTrigRec = "";
	fMatacqNbCol = "";
	fMatacqMask = "";
	fMatacqVernierbase = "";
	fTrace2D = new TH2I("Trace2D", "Trace2D", fLongTrace, 0, fLongTrace - 1,
			fHistory, 0, fHistory - 1);
	fLabelMask = GetEvent()->GetDataParameters()->GetLabel(IndexBase + 8);
	fLabelNbCol = GetEvent()->GetDataParameters()->GetLabel(IndexBase + 9);
	fLabelPostTrig = GetEvent()->GetDataParameters()->GetLabel(IndexBase + 10);
	fLabelTrigRec = GetEvent()->GetDataParameters()->GetLabel(IndexBase + 11);

	fMatacqMask = GetEvent()->GetDataParameters()->GetParNameFromIndex(IndexBase + 8);
	fMatacqNbCol = GetEvent()->GetDataParameters()->GetParNameFromIndex(IndexBase + 9);
	fMatacqPostrig
			= GetEvent()->GetDataParameters()->GetParNameFromIndex(IndexBase + 10);
	fMatacqTrigRec
			= GetEvent()->GetDataParameters()->GetParNameFromIndex(IndexBase + 11);

	fCurrent2D = 0;

	GetSpectra()->AddSpectrum(fTrace2D);

	SetConditionStop(1);

	fError.TreatError(0, 0,
			" -----------------Init matacq done-----------------");

}

//______________________________________________________________
void GEtalonnageMatacq::InitUserRun() {
	// Initialization only before each run
	// For specific user treatment

	TString tempo;
	fIncrement1 = 0;
	fIncrement2 = 0;
	fnRun++;
	fNbOfWarning = 0;

	tempo.Form("---------------InitUser Run %d --------------", fnRun);
	fError.TreatError(0, 0, tempo);
	SetStop(false);
	// construction nom courant MATACQ_VOIE_X
	TString name;
	name = "";
	fCurrentNumberChannel = fNoChannels[0];
	name.Form("%s%d", fNameFirstParameter, fCurrentNumberChannel);
	fEtalonnage[fCurrentNumberChannel] ->Reset();
	fIncrement[fCurrentNumberChannel] = 0;

	if (name.Data() != NULL)
		fLabelTrace = GetEvent()->GetDataParameters()->GetLabel(name.Data());
	else
		fLabelTrace = 0;

	tempo.Form(" --------------%s - Label = %d --no channel:%d", name.Data(),
			fLabelTrace, fCurrentNumberChannel);
	fError.TreatError(0, 0, tempo);

}

//______________________________________________________________
void GEtalonnageMatacq::User() {
	// User method for specific treatement
	//
	// Treatement of event
	fIncrement1++;
	fIncrement2++;

	// can be not use  , default is =1
	//if (fcountevent>100)

	if ((fIncrement1 % 10) == 0) {
		cout << ".";
		cout.flush();
	}
	if (fIncrement1 >= 500) {
		cout << "\n";
		fIncrement1 = 0;
	}

	/*	if ((fIncrement1 % 10) == 0) {
	 cout << ".";
	 cout.flush();
	 }
	 if (fIncrement1 >= 500) {
	 cout << "\n";
	 fIncrement1 = 0;
	 }*/
	Int_t ancienne_pos = 0;
	UShort_t  i, k, val_event, val_label;
	//UShort_t val_TrigRec =0, ncol ,val_PostTrig =0;
	Float_t mean =0.0;
	UShort_t val_mask, no_channel_calc_mask; //value of mask and no of channel computer with mask
	bool no_channel_mask[NB_CHANNEL];
	k = 0;

	val_mask = 1024;
	no_channel_calc_mask = 555; //valeur bidon

	for (i = 0; i < GetEventArrayLabelValueSize() / 2; i++) {

		//if (GetEventArrayLabelValue_Label(i) == fLabelTrigRec) {
		//	val_TrigRec = GetEventArrayLabelValue_Value(i);
		//}

		//if (GetEventArrayLabelValue_Label(i) == fLabelPostTrig) {
		//	val_PostTrig = GetEventArrayLabelValue_Value(i);
		//}
		//if (GetEventArrayLabelValue_Label(i) == fLabelNbCol) {
		//	ncol = GetEventArrayLabelValue_Value(i);
		//}
		if (GetEventArrayLabelValue_Label(i) == fLabelMask) {
			val_mask = GetEventArrayLabelValue_Value(i);
		}

	}
	no_channel_calc_mask = 555;
	if (val_mask == 0)
		no_channel_calc_mask = 1024;
	if (val_mask == 1)
		no_channel_calc_mask = 0;
	if (val_mask == 2)
		no_channel_calc_mask = 1;
	if (val_mask == 4)
		no_channel_calc_mask = 2;
	if (val_mask == 8)
		no_channel_calc_mask = 3;
	if (val_mask == 16)
		no_channel_calc_mask = 4;
	if (val_mask == 32)
		no_channel_calc_mask = 5;
	if (val_mask == 64)
		no_channel_calc_mask = 6;
	if (val_mask == 128)
		no_channel_calc_mask = 7;

	if (no_channel_calc_mask == 555) {
		fNbOfWarning++;
		if (fNbOfWarning < 20)
			fError.TreatError(1, val_mask,
					"Validation of mask not only one channel!");
	}

	for (i = 0; i < NB_CHANNEL; i++) {
		no_channel_mask[i] = (bool) ((val_mask >> i) & 1);
	}

	//if ((no_channel_calc_mask==fCurrentNumberChannel) {
	if (no_channel_mask[fCurrentNumberChannel]) {

		fIncrement[fCurrentNumberChannel]++;
		val_event = GetEventArrayLabelValue_Value(i);
		for (i = 0; i < GetEventArrayLabelValueSize() / 2; i++) {
			val_event = GetEventArrayLabelValue_Value(i);
			val_label = GetEventArrayLabelValue_Label(i);

			if (val_label == fLabelVernier[fCurrentNumberChannel]) {
				fHistoVernier[fCurrentNumberChannel]->Fill(val_event);
				if (fVernierMax[fCurrentNumberChannel] < val_event)
					fVernierMax[fCurrentNumberChannel] = val_event;
				if (fVernierMin[fCurrentNumberChannel] > val_event)
					fVernierMin[fCurrentNumberChannel] = val_event;
			}

			if (val_label == fLabelTrace) {

				ancienne_pos = k;
				k++;

				mean += val_event;
				//nouvelle_pos = (ancienne_pos + fLongTrace + (20
					//	* ((val_PostTrig + val_TrigRec) % 128))) % fLongTrace;

				fCurtrace[fCurrentNumberChannel]->SetBinContent(ancienne_pos
						+ 1, val_event);
				fEtalonnage[fCurrentNumberChannel]->Fill(ancienne_pos,
						val_event);
				fTrace[fCurrentTrace][fCurrentNumberChannel]->SetBinContent(
						ancienne_pos + 1, val_event);

				fTrace2D->SetBinContent(ancienne_pos + 1, fCurrent2D, val_event);
			}
		}

		fCurrent2D++;
		fCurrent2D = fCurrent2D % fHistory;
		fCurrentTrace++;
		fCurrentTrace = fCurrentTrace % fHistoryTrace;

	}

	if (GetDeviceIn()->GetType() == GT_TYPE_FILE) {
		if (fIncrement2 >= fCoups)
			SetStop(true);
		//cout << "marque stop  = : "<< GetStop()<<"  "<<fIncrement2<< " fAcquisitionOn = "<<fAcquisitionOn <<"\n";
	}

	if (fIncrement[fCurrentNumberChannel] >= fCoups) {
		SetStop(true);
		//cout << "stop2 = : "<< GetStop()<<"  fIncrement[fCurrentNumberChannel]="<<fIncrement[fCurrentNumberChannel] << " coup = " << fCoups<<"\n";
	}

}

//______________________________________________________________
void GEtalonnageMatacq::EndUserRun() {
	// excuted a end of run

	// ajouter un status de retour vers soap.

	TString tempos;

	tempos.Form("----------EndUserRun-Mataq---------- Event Count : %d", fCoups);
	fError.TreatError(0, 0, tempos);

}
//______________________________________________________________
void GEtalonnageMatacq::EndUser() {
	TString tempos;

	TString tempo;
	fError.TreatError(0, 0, "Closing all");
	tempo.Form("----------EndUserRun----------- with %d events and %d runs\n",
			fCoups, fnRun);
	fError.TreatError(0, 0, tempo);

	Double_t sum;
	Float_t mean;

	Float_t tempof;

	fXmlFile.InitXml(true, "Etalonnage_Matacq");

	for (int i = 0; i < NB_CHANNEL; i++) {
		sum = 0;
		for (int j = 0; j < fLongTrace; j++) {
			sum += (Float_t) fEtalonnage[i]->GetBinContent(j + 1);
		}

		mean = 0;
		if (fIncrement[i] != 0) {

			mean = (Float_t)(sum / (Float_t)(fLongTrace * fIncrement[i]));

			/*for (int j =0; j<fHistoVernier[i]->GetNbinsX(); j++) {
			 if (fHistoVernier[i]->GetBinContent(j+1)!=0) {
			 fVernierMin[i]
			 = fHistoVernier[i]->GetXaxis()->GetBinCenter(j);
			 break;
			 }
			 }
			 for (int j =fHistoVernier[i]->GetNbinsX(); j>0; j--) {
			 if (fHistoVernier[i]->GetBinContent(j+1)!=0) {
			 fVernierMax[i]=fHistoVernier[i]->GetXaxis()->GetBinCenter(j);
			 break;
			 }
			 }*/

		}
		tempo.Form(" +-+-+-+-+-+-+-+mean %f ----Increment :%d ", mean,
				fIncrement[i]);
		fError.TreatError(0, 0, tempo);

		for (int j = 0; j < fLongTrace; j++) {
			tempof = (fEtalonnage[i]->GetBinContent(j + 1));
			if (fIncrement[i] != 0) {
				tempof = (tempof / (Float_t) fIncrement[i]) - mean;
			} else {
				tempof = -mean;
			}
			fEtalonnage[i]->SetBinContent(j + 1, tempof);

		}

	}
	fXmlFile.SaveCoefMatacq(fEtalonnage, fDeltaT, fVernierMin, fVernierMax,
			NB_CHANNEL, fLongTrace);
	fXmlFile.CloseXml();

}


//______________________________________________________________________________

void GEtalonnageMatacq::InitTTreeUser() {
	// User method for specfic initialisation of TTree
	// It can be usefull for example multi-hit detections
	// or to have a TTree with only few parameters ( for low compute)
	// to run this method , you have to slect mode 3 in  SetTTreeMode
	// ex : a->SetTTreeMode(3,"/space/MyTTree.root");


}
