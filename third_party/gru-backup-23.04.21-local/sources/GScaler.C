// File: GScaler.C   $
// Author: $Author: patois/legeard $
//***************************************************************************
//
//                             -------------------
//    begin                : Thu Jun 14 2001
//    copyright            : (C) 2001 by Garp
//    email                :  legeard@ganil.fr
//////////////////////////////////////////////////////////////////////////
//
// GScaler
//
// Scaler class for scaler events.
//
//////////////////////////////////////////////////////////////////////////

// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#include "GBuffer.h"

#include <iostream>
#include <TMath.h>
#include "GScaler.h"

#include "GScalerEvent.h"

#include "TROOT.h"
#include "TRint.h"
extern "C" {
#include <GEN_TYPE.H>
#include <gan_acq_buf.h>
}

ClassImp( GScaler);

//______________________________________________________________________________
GScaler::GScaler(void) {
	// Constructor/initialisator
	fVerbose = 0;
	fNbChannel = -1; // Initialisation Marker
	fScalerMode = 0;
	fScalerArray = NULL;
	theScalerTree = NULL;
	theScalerTreeFile = NULL;
	fReinitTree = true; // Reinitialisation of run for each run
	fInitTreeNeverDone = true; // true if init tree is not done
	fTTreeWithRunNumber = false; // TRee will contain run number in its file name
}

//_______________________ _______________________________________________________
GScaler::~GScaler(void) {
	// Delete

	if (theScalerTreeFile) {
		theScalerTreeFile->cd();
		if (theScalerTreeFile->IsOpen()) {
			theScalerTree->Write();
			theScalerTreeFile->Close();
			}
	}
	if (fScalerArray){
	delete[] fScalerArray;
	}
	//	if (theScalerTreeFile)
	//	delete theScalerTreeFile;

	//if (theScalerTree)
	//delete theScalerTree;// I think that theTree is deleted with close of file theScalerTreeFile

}

//_______________________ _______________________________________________________
void GScaler::GScalerInit(GBufferIn2p3* _buffer, int runnum) {
	char* newname1;
	char newname2[MAX_CARACTERES];
	TString parName;
	TString parType;

	fNbChannel = _buffer->GetBuffer_map_in2p3()->les_donnees.cas.scale.Nb_channel;

	if (fNbChannel == 0) {
		fError.TreatError(1,0," Scaler Nb is 0, so treatment of scaler is invalidated");
		fScalerMode = 0;
		return;
	} else {
		gROOT->cd();

		if (fInitTreeNeverDone || fReinitTree) {
			fInitTreeNeverDone = false;

			if (fTTreeWithRunNumber) {
				newname1 = RemoveChar(fNameScalerTreeFile, (char*) ".root",
						false);
				sprintf(newname2, "%s_%4d.root", newname1, runnum);
				delete [] newname1;
				ReplaceChar(newname2, ' ', '0');
				fError.Infos("New name of Scaler Tree is ",newname2 );
			}

			fScalerArray = new GScalerEvent[fNbChannel];

			GetScalerEvents( _buffer,  0,0);
			if (fScalerMode == 0)
				return;

			theScalerTreeFile = new TFile(fNameScalerTreeFile, "recreate");
			theScalerTreeFile ->cd();
			theScalerTree = new TTree("AutoScalerTree",
					"Automatic filled ScalerTree");

			if (fNbChannel == 0) {
				fError.TreatError(1,0," Scaler Nb is 0, so treatment of scaler is invalidated");
				fScalerMode = 0;
				return;
			}

			fError.Infos("Initialization of ScalerTTree");

			for (Int_t i = 0; i < fNbChannel; i++) {
				char parName2[20];
				char parName3[40];
				sprintf(parName2, "SCALE%d", i);
				parName = parName2;
				if (fVerbose > 5)
					cout << parName.Data() << "\n";
				parType = parName + "/i";
				sprintf(parName3, "%sLabel", parName2);
				theScalerTree->Branch(parName3,
						(fScalerArray[i].GetLabelPoint()), parType);
				sprintf(parName3, "%sStatus", parName2);
				parType = parName + "/I";
				theScalerTree->Branch(parName3,
						(fScalerArray[i].GetStatusPoint()), parType);
				parType = parName + "/i";
				sprintf(parName3, "%sCount", parName2);
				theScalerTree->Branch(parName3,
						(fScalerArray[i].GetCountPoint()), parType);
				sprintf(parName3, "%sFreq", parName2);
				theScalerTree->Branch(parName3,
						(fScalerArray[i].GetFreqPoint()), parType);
				sprintf(parName3, "%sTics", parName2);
				theScalerTree->Branch(parName3,
						(fScalerArray[i].GetTicsPoint()), parType);
				sprintf(parName3, "%sNE", parName2);
				theScalerTree->Branch(parName3,
						(fScalerArray[i].GetEventCountPoint()), parType);

			}
		}
	}// if (fNbChannel ==0) {
}

//_______________________ _______________________________________________________
void GScaler::Stop() {

  if (GetScalerMode() > 0) {
	if (theScalerTree) {
	   fError.Infos("Closing ScalerTTree");
		theScalerTreeFile ->cd();
		theScalerTree ->Write();
		if (fVerbose > 4)
			theScalerTreeFile->ls();
		if (fReinitTree)
			theScalerTreeFile ->Close("dtor");
	}
  }
}
//_______________________ _______________________________________________________
void GScaler::SetScalerMode(int mode, const char* filename, bool reinit, bool withrun) {
	// Set User Scaler treatment mode and configuration of Scaler TTree
	// = 0 no treatment(default)
	// = 1 treatment
	//  filename = name of TTree file

	if ((withrun) && (!reinit)) {
		fError.TreatError(
				1,
				0,
				"no reinitialization is incompatible with taking account run number , so reinitialization is forced!\n");

		reinit = true;
	}

	if (mode > 1 || mode < 0) {
		fError.TreatError(1, 0,
				"  Scaler mode is out of range, so Standard mode is selected \n");

		fScalerMode = 0;
	}
	fScalerMode = mode;
	fReinitTree = reinit;
	fTTreeWithRunNumber = withrun; // TRee will contain run number in its file name
	strcpy(fNameScalerTreeFile, filename);
}

//______________________________________________________________________________
void GScaler::ScalerTreatement(GBufferIn2p3 *_buffer, UInt_t nbevents, int runnum) {

        if (GetScalerMode() >0){
		GetScalerEvents(_buffer, nbevents, runnum);
		if (fScalerMode == 0)
			return;
		theScalerTree->Fill();
		theScalerTree->AutoSave();
	}
}
//______________________________________________________________________________
void GScaler::GetScalerEvents(GBufferIn2p3 *_buffer, UInt_t nbevents, int runnum) {

	if (fVerbose > 2) {
		cout << "GOT A SCALER EVENT !!!\n";
		cout << _buffer->GetBuffer_map_in2p3()->les_donnees.cas.scale.Length
				<< "  \n";
		cout << _buffer->GetBuffer_map_in2p3()->les_donnees.cas.scale.Length
				<< "  \n";
		cout << _buffer->GetBuffer_map_in2p3()->les_donnees.cas.scale.Nb_channel
				<< "  \n"; // This is number of actual scalers
		cout << _buffer->GetBuffer_map_in2p3()->les_donnees.cas.scale.Acq_status
				<< "  \n";
		cout << (int) nbevents << "  \n";
	}
	if (fNbChannel == -1) {
		GScalerInit(_buffer, runnum);// First use
		if (fScalerMode == 0)
			return;
	} else if (fNbChannel
			!= (Int_t) _buffer->GetBuffer_map_in2p3()->les_donnees.cas.scale.Nb_channel) {
		cout << "Dont know how to deal with variable number of scalers!\n"
				<< endl;
		cout << fNbChannel << " versus "
				<< _buffer->GetBuffer_map_in2p3()->les_donnees.cas.scale.Nb_channel
				<< endl;
		return;
	}
	for (Int_t i = 0; i < fNbChannel; i++) {
		fScalerArray[i].Set(_buffer, i, nbevents);
		if (fVerbose > 2) {
			cout << "count =" << fScalerArray[i].GetCount() << "\n";
		}
	}

	if (fScalerMode == 0)
		return;
	if (theScalerTreeFile)
		theScalerTreeFile ->cd();

}

//______________________________________________________________________________
void GScaler::DumpScalerSpe(void) {
	// Dump scalers value on cout
	if (fNbChannel == -1) {
		cout << "No scaler to dump\n";
	} else {
		cout << "Dumping " << fNbChannel << " scalers\n";
		for (Int_t i = 0; i < fNbChannel; i++) {
			cout << "--------- " << i << " -------------\n";
			fScalerArray[i].Dump();
		}
	}
}
//______________________________________________________________________________
void GScaler::DumpScaler(void) {
	// Dump scalers value on cout
	int col;
	col = 2;
	int cont_col;
	cont_col = 0;
	char tempo[256];
	cout << "-- Dumping Scaler, nb :" << fNbChannel
			<< "-----(|no count Freq)|-----\n";
	if (fNbChannel == -1) {
		cout << "No scaler to dump\n";
	} else {
		for (Int_t i = 0; i < fNbChannel; i++) {
			sprintf(tempo, "|%3d : %10d  %10d ", i,
					(int) (fScalerArray[i].GetCount()),
					(int) (fScalerArray[i].GetFreq()));
			cout << tempo;
			if ((cont_col++) == col) {
				cout << "\n";
				cont_col = 0;
			}
		}
	}
	cout << "\n";
}
//______________________________________________________________________________

const GScalerEvent* GScaler::GetScalerPtr(Int_t index) const {
	// Return a constant pointer to the Scaler Index
	if (index < 0 || index >= fNbChannel) {
		if (fVerbose > 0)
			cout << "Scaler index out of range. Got: " << index
					<< " Limits are: [" << 0 << "," << fNbChannel - 1 << "]\n";
		return (NULL);
	}
	return (&(fScalerArray[index]));
}
//______________________________________________________________________________

//______________________________________________________________________________
