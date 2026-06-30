// File : GAcqNumexo.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class  GAcqNumExo specific analyse for Numexo2 cards.
//
//
////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the Ldifficense, or *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************

#include <sstream>
using std::ostream;

#include <TObject.h>
#include "General.h"
#include "GAcq.h"
#include "GDevice.h"
#include "TH1.h"

//______________________________________________________________________________

ClassImp( GAcq);
// File : GAcqNumexo.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GAcqNumexo
//
// Class for User treatment
//
////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#include "GAcqNumexo.h"

#include "TROOT.h"
#include <TProfile.h>
#include <TRandom.h>
#include <cstdlib>
#include "GEventMFM.h"

//_________________________________global_variables______________________________


//______________________________________________________________________________

ClassImp ( GAcqNumexo);

GAcqNumexo::GAcqNumexo(GDevice* _fDevIn, GDevice* _fDevOut) {
	// Constructor/initialisator of Acquisition object
	//
	// entry:
	// - Input Device
	// - Output Device
	fDevIn = _fDevIn;
	fDevOut = _fDevOut;

	fOscilloFrame = NULL;
	countpoint = 0;
	fNoEvent = 0;
	fNoEventOld = 0;
	fOscilloFlag = true;
	fAcquiFlag= false;
	fTimeView=true;
	fSizehisto = 65536;
	fMaxhisto = 65536;
	fUnittime =5;
    fVerbose =0;
    for (int cristal =0; cristal<EXO_NUMBER_CRISTAL_ID;cristal++){
    	DeltaT[cristal]=NULL;
    	BGO[cristal]=NULL;
    	Csi[cristal]=NULL;
    	Outer[cristal]=NULL;
    	InnerM[cristal]=NULL;
    	InnerT[cristal]=NULL;
    	InnerT30vT90[cristal]=NULL;
    	InnerT30vT60[cristal]=NULL;
    }
}

//_____________________________________________________________________________

GAcqNumexo::~GAcqNumexo() {
	//Destructor of class GAcqNumexo
	gROOT->cd();

for (int cristal =0; cristal<EXO_NUMBER_CRISTAL_ID;cristal++){
	if  (DeltaT[cristal]!=NULL ){ delete DeltaT[cristal] ;DeltaT[cristal]=NULL;}
	if  (BGO[cristal]!=NULL )   { delete BGO[cristal] ;BGO[cristal]=NULL;}
	if  (Csi[cristal]!=NULL )   { delete Csi[cristal] ;Csi[cristal]=NULL;}
	if  (InnerT30vT90[cristal]!=NULL ){ delete InnerT30vT90[cristal] ;InnerT30vT90[cristal]=NULL;}
	if  (InnerT30vT60[cristal]!=NULL ){ delete InnerT30vT60[cristal] ;InnerT30vT60[cristal]=NULL;}

	for (int k = 0; k < EXO_NB_OUTER; k++) {
		if  ( Outer[cristal][k]!=NULL ) {delete  Outer[cristal][k];Outer[cristal][k]=NULL;}
	}
	for(int k = 0; k < EXO_NB_INNER_M; k++) {
		if  ( InnerM[cristal][k]!=NULL ) {delete  InnerM[cristal][k];InnerM[cristal][k]=NULL;}
		}
	for(int k = 0; k < EXO_NB_INNER_T; k++) {
		if  ( InnerT[cristal][k]!=NULL ) {delete  InnerT[cristal][k];InnerT[cristal][k]=NULL;}
		}
	if  (Outer[cristal]!=NULL )  { delete [] Outer[cristal]  ; Outer[cristal]=NULL;}
	if  (InnerM[cristal]!=NULL ) { delete [] InnerM[cristal]  ;InnerM[cristal]=NULL;}
	if  (InnerT[cristal]!=NULL ) { delete [] InnerT[cristal]  ;InnerT[cristal]=NULL;}
}

	if (fOscilloFrame){
		delete fOscilloFrame;
		fOscilloFrame=NULL;
	}
}

//______________________________________________________________

void GAcqNumexo::InitUser() {
	// Initialisation for global  user treatement
	// choice: allow  to switch on different treatements (1 to 6)
	// called with command SetUserMode( choice )
	// In this method , we can make histograms (prevously declared in GAcqNumexo.h)
	// Ex : in  GAcqNumexo.h we have declared           TH1I *fMyHisto ;
	//      in this methode we make it             fHisto = new TH1I ("MyHisto","MyHisto",1024,0,1024);
	//      we can include it in database          GetSpectra()->AddSpectrum(fMyHisto,"MyFamily");

	TString name;
	TString famname;

	//-----------------------------for analyse data -----------------------------------------------

	if (fAcquiFlag){
		cout << "----------------Init GAcqNumexo----------------------------\n";
	char idname[2];

	NbEventH = new TH1I("NoEventH", "NoEventH", fSizehisto, 0, fMaxhisto);
	GetSpectra()->AddSpectrum(NbEventH);
	NbEventL = new TH1I("NoEventL", "NoEventL", fSizehisto, 0, fMaxhisto);
	GetSpectra()->AddSpectrum(NbEventL);
	NbEvent = new TH1F("NoEvent", "NoEvent", fSizehisto, 0,
			(fMaxhisto * fMaxhisto));
	GetSpectra()->AddSpectrum(NbEvent);

	for (int cristal = 0; cristal < EXO_NUMBER_CRISTAL_ID; cristal++) {

		if (cristal == 0)
			strcpy(idname, "A");
		else
			strcpy(idname, "B");
		famname.Form("Cristal_%s", idname);

		name.Form("InnerT30vT90_%s", idname);
		InnerT30vT90[cristal] = new TH2I(name.Data(), name.Data(),
				(int) (fSizehisto / 128), 0, (int) (fSizehisto / 128),
				(int) (fSizehisto / 128), 0, (int) (fSizehisto / 128));
		GetSpectra()->AddSpectrum(InnerT30vT90[cristal], famname);
		name.Form("InnerT30vT60_%s", idname);
		InnerT30vT60[cristal] = new TH2I(name.Data(), name.Data(),
				(int) (fSizehisto / 128), 0, (int) (fSizehisto / 128),
				(int) (fSizehisto / 128), 0, (int) (fSizehisto / 128));
		GetSpectra()->AddSpectrum(InnerT30vT60[cristal], famname);

		name.Form("DeltaT_%s", idname);
		DeltaT[cristal] = new TH1I(name.Data(), name.Data(), fSizehisto, 0,
				fMaxhisto);
		//cout <<"debug  test\n"<<famname.Data()<<"\n";
		GetSpectra()->AddSpectrum(DeltaT[cristal], famname);

		name.Form("Inner6M_%s", idname);

		InnerM[cristal]=  new TH1I*[EXO_NB_INNER_M];

		InnerM[cristal][0] = new TH1I(name.Data(), name.Data(), fSizehisto, 0,
				fMaxhisto);
		name.Form("Inner20M_%s", idname);
		InnerM[cristal][1] = new TH1I(name.Data(), name.Data(),fSizehisto, 0,
				fMaxhisto);
		GetSpectra()->AddSpectrum(InnerM[cristal][0], famname);
		GetSpectra()->AddSpectrum(InnerM[cristal][1], famname);
		Outer[cristal] = new TH1I*[EXO_NB_OUTER];
		for (int i = 0; i < EXO_NB_OUTER; i++) {
			name.Form("Outer_%d_%s", (1 + i), idname);
			cout << " Outer name " << name.Data() << "\n";
			Outer[cristal][i] = new TH1I(name, name, fSizehisto, 0, fMaxhisto);
			GetSpectra()->AddSpectrum(Outer[cristal][i], famname);
		}
		name.Form("BGO_%s", idname);
		BGO[cristal] = new TH1I(name.Data(), name.Data(), fSizehisto, 0,
				fMaxhisto);
		GetSpectra()->AddSpectrum(BGO[cristal], famname);
		name.Form("CsI_%s", idname);
		Csi[cristal] = new TH1I(name.Data(), name.Data(), fSizehisto, 0,
				fMaxhisto);
		GetSpectra()->AddSpectrum(Csi[cristal], famname);
		InnerT[cristal]= new TH1I*[EXO_NB_INNER_T];
		for (int i = 0; i < EXO_NB_INNER_T; i++) {
			name.Form("Inner%dT_%s", 30 + i * 30, idname);
			cout << " Inner name " << name.Data() << "\n";
			InnerT[cristal][i] = new TH1I(name, name, fSizehisto, 0, fMaxhisto);
			GetSpectra()->AddSpectrum(InnerT[cristal][i], famname);
		}
	}
	}
	//-----------------------for oscillo-----------------------------------------------------
if (fOscilloFlag){
	for (int k = 0; k < NBTRACE; k++) {
		name.Form("Voie_%d", k + 1);
		mytrace[k]
				= new TH1I(name.Data(), name.Data(), SIZETRACE, 0, SIZETRACE);
		GetSpectra()->AddSpectrum(mytrace[k]);
		if (fTimeView)
		{
			mytrace[k]->GetXaxis()->SetTitle("us");
			mytrace[k]->GetXaxis()->Set(SIZETRACE, 0.0,(Double_t)(SIZETRACE*fUnittime/1000));
		}
	}
	mytraceSum=new TH1I("VoieSum", "VoieSum", SIZETRACE*NBTRACE, 0, SIZETRACE*NBTRACE);
	GetSpectra()->AddSpectrum(mytraceSum);
	fOscilloFrame = new MFMOscilloFrame();

}
}
//______________________________________________________________

void GAcqNumexo::InitUserRun() {
	// Initialisation for user treatemeant for each  run
	// For specific user treatement



}
//______________________________________________________________
void GAcqNumexo::User() {
	// User method for user treatement for each events
	// the event is presented on two ways ( user can use one or other )
	// 1 - event is a vector of a serie of couples  UShor_t/Short_t Label/Value ( GetEventArrayLabelValue())
	//      of variable size   GetEventArrayLabelValueSize() and  with a max size of GetEventArrayLabelValueSizeMax()
	//      The numebers of couple  Label/Value is GetEventArrayLabelValueSize()/2
	//      GetEventArrayLabelValue_Label(i)  return  Label number i in  vector GetEventArrayLabelValue()
	//      GetEventArrayLabelValue_Value(i)  return  Value number  i in  vector GetEventArrayLabelValue()
	//
	//      Exemple of use of manage index,label,name
	//      GetEvent()->GetDataParameters()->GetLabel("NAME")  return label of parameter with name "NAME"
	//      GetEvent()->GetDataParameters()->GetLabel(index)   return label of parameter with index i (in GetEventArray() vector)
	//      GetEvent()->GetDataParameters()->GetParName(label) return name of parameter with label 'label'
	//      GetEvent()->GetDataParameters()->GetIndex(label)   return index(in GetEventArray() vector) of parameter with label 'label
	//      GetEvent()->GetDataParameters()->GetIndex("NAME")  return index(in GetEventArray() vector) of parameter with name "NAME"
	// 2  - event is a vector of UShor_t* GetEventArray() of fixed size  (GetEventArraySize())
	//      '0's have been included in vector where parameter were not present
	//      GetEventArray_Value(i) return value index i in vector GetEventArray()
	//      In case of multi value for same label, this way must not be used


	GEventMFM * pMFMevent;
	int nb_item = 0, framesize, maxdump = 0, dumpsize, i_item;
	int type = 0;
	pMFMevent = (GEventMFM*) GetEvent();
	uint16_t value;
	fCommonframe = pMFMevent->GetFrame();
	TString name;
	type = pMFMevent->GetFrameType();
    int correctif =1;// valeur a soustraire car les nb of channel vont de 1 à 4 et non de 0 à 3

	if ((type == MFM_OSCI_FRAME_TYPE)and ( fOscilloFlag)) {
		fOscilloFrame->SetAttributs(fCommonframe->GetPointHeader());
		nb_item = (fOscilloFrame->GetNbItems());
		nb_item = 16384;// a enlever un fois le bug corrigé
		framesize = fOscilloFrame->GetFrameSize();
		uint16_t card_channel = fOscilloFrame->GetChannelIdxBoard();
		uint16_t osci_channel = fOscilloFrame->GetChannelIdxNumber();
		int basetime = fOscilloFrame->GetConfigTimeBase();
		int onoff    = fOscilloFrame->GetConfigOnOff();
		int trig     = fOscilloFrame->GetConfigTrig();
		int confsig  = fOscilloFrame->GetConfigSignal();
		int confchan = fOscilloFrame->GetConfigChannelIdx();
		basetime = pow(2,basetime);

		if (GetVerbose()>1)
		{
		cout << " debug info channel = " <<  fOscilloFrame->GetChannelIdx() << "\n";
			cout << " debug channel = " << osci_channel << "\n";
			cout << " debug nb_item = " << nb_item << "\n";
			cout << " debug card_channel = " << card_channel << "\n";
			cout << " debug basetime = " << basetime << "\n";
			cout << " debug onoff = " << onoff << "\n";
			cout << " debug trigger = " << trig << "\n";
			cout << " debug signal = " << confsig << "\n";
			cout << " debug conf channel = " <<  confchan << "\n";
		}
		if ((0 > osci_channel) or (NBTRACE + correctif < osci_channel)) {

			fError.TreatError(2, osci_channel,
					" Nb osci_Channel not compatible ");
			return;
		}

		name.Form("Voiecarte_%d", card_channel);

		//fOscilloFrame->HeaderDisplay();
		if (GetVerbose()>5) {
			fOscilloFrame->HeaderDisplay();
			if (framesize < maxdump)
				dumpsize = framesize;
			else
				dumpsize = maxdump;
			fOscilloFrame->DumpRaw(dumpsize, 0);
		}

		for (i_item = 0; i_item < nb_item; i_item++) {
			if (GetVerbose()>8)
				cout << "osci_channel, i_item, value = " << osci_channel
						<< " - " << i_item << " - " << value << " correctif = "<<correctif<< "\n";
			fOscilloFrame->OscilloGetParameters(i_item, &value);
			mytrace[osci_channel - correctif] ->SetBinContent(i_item + 1, value);
			mytraceSum->SetBinContent(i_item*NBTRACE +osci_channel - correctif+ 1, value);

		}
		if (fTimeView) mytrace[osci_channel-correctif]->GetXaxis()->Set(SIZETRACE, 0.0,(Double_t)(SIZETRACE*fUnittime*basetime)/1000);
		mytrace[osci_channel- correctif]->SetEntries(SIZETRACE);

	}



	if ((type == MFM_EXO2_FRAME_TYPE)and ( fAcquiFlag)) {
		countpoint++;
		if (countpoint > 10) {
			cout << "10 events" << flush << "\n";
			countpoint = 0;
		}

		MFMExogamFrame * pFrame;
		pFrame = (MFMExogamFrame*) (pMFMevent->GetFrame());
		long long ts = pFrame->GetTimeStamp();
		fNoEvent = pFrame->GetEventNumber();
		if (GetVerbose()>2) {
			cout << " Time stamp = " << ts << " envent num = " << fNoEvent
					<< "\n";
		}
		framesize = pFrame->GetFrameSize();
		// pFrame->DumpRaw(framesize,0);
		UInt_t diff = 0;
		int cristal;
		if (fNoEventOld != 0) {
			diff = fNoEvent - fNoEventOld;
			if (diff > 1)
				cout << " Warning ----- important difference of No Event "
						<< fNoEventOld << "  >>  " << fNoEvent << " = " << diff
						<< " ----\n";
		}
		fNoEventOld = fNoEvent;
		//pFrame->HeaderDisplay();
		UInt_t event = fNoEvent;
		NbEventH->Fill(event >> 16);
		event = fNoEvent;
		NbEventL->Fill(event & 0x0000ffff);

		NbEvent->Fill(fNoEvent);

		int value_InnerT[EXO_NB_INNER_T];

		cristal = pFrame->ExoGetTGCristalId();
		if (cristal != 0)
			cristal = 1;
		DeltaT[cristal]->Fill(pFrame->ExoGetDeltaT());

		InnerM[cristal][0]->Fill(pFrame->ExoGetInnerM(0));
		InnerM[cristal][1]->Fill(pFrame->ExoGetInnerM(1));

		for (int i = 0; i < EXO_NB_OUTER; i++) {
			Outer[cristal][i]->Fill(pFrame->ExoGetOuter(i));

		}

		BGO[cristal] ->Fill(pFrame->ExoGetBGO());
		Csi[cristal] ->Fill(pFrame->ExoGetCsi());

		for (int i = 0; i < EXO_NB_INNER_T; i++) {
			value_InnerT[i] = pFrame->ExoGetInnerT(i);
			InnerT[cristal][i] ->Fill(value_InnerT[i]);
		}
		InnerT30vT90[cristal] ->Fill(value_InnerT[0], value_InnerT[2]);
		InnerT30vT60[cristal] ->Fill(value_InnerT[0], value_InnerT[1]);
	}

}
//______________________________________________________________
void GAcqNumexo::EndUserRun() {

	//  end of run ,  executed a end of each run

}

//______________________________________________________________
void GAcqNumexo::EndUser() {
	// globlal final end executed a end of runs
	// must be explicitly called!

}

//////fin/////////////////////////////////////
