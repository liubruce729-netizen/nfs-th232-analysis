// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GTTree
//
// This class manage TTree of events for GRU
//
/////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <dirent.h>
#include "math.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <TROOT.h>
#include <TSystem.h>
#include <unistd.h>
#include "General.h"
#include "GTTree.h"


ClassImp( GTTree);
//_______________________________________________________________________________
GTTree::GTTree(){
   fTTreeMode = TREE_NO;  

   fTree = new TTree("AutoTree", "Automatic filled Tree");;
   fTreeFile = NULL;
   fSkipFillTtree = true;
   fTTreeCompressionLevel = 5;
   fReinitTree = false; 
   fTTreeWithRunNumber = false; // TRee will not contains run number in its file name
   fInitTreeNeverDone = true;
   fTTreeWithRunNumber =false;
   fDiff_time_save = 60;
   fIsUserInit=false;
}
//_______________________________________________________________________________
GTTree::~GTTree(){


if (fTreeFile) {
	if (fVerbose > 5) fError.TreatDebug(fVerbose, 0, "Close TTree file");
	fTreeFile->cd();
	fTree->Write();
	fTreeFile->Close();
	//  if (fTree)  {   fTree->Delete() ;fTree=NULL;}
	// I think that fTree is deleted with close of file fTreeFile
	}
}
//_______________________________________________________________________________
void GTTree::InitTTree(int runnumber,GEventBase* event) {

	// creat and initialize the branch of TTree


	// fTTreeWithRunNumber :
	//                   true  => get run number and treefile whill contain run number in file name
	//                   false => (default)  treefile will not contain run number in file name
	// fReinitTree         : true  => reinit tree branch for each run (not default)
	//                      false => no re initialization of tree branch
	// filename is the  name of TTree file
	// To create the TTree we have few ways selected by "fTTreeMode" parameter:
	// TREE_NO : no TTree
	// TREE_STANDARD : Standard initialization is a sheet by parameter
	//  this conversion can be  slow .
	// TREE_ONE_VECTOR : TTree is initialized  with only on sheet with "my branch" as name. In this sheet we store
	//     a fix size vector which contain a values of event (all values of parameter, even a parameter is null)
	//     b_spectra = true , authorize the creation of standard histogram
	// TREE_USER : User TTree (defined in GUser.C)
	// TREE_CIRCULAR :  it like a TREE_USER but TTree is circular, so with a limited size

	char tempo[64];
	TString format;
	char* newname1;
	char newname2[MAX_CARACTERES];
	TString parName;
	TString parType;
	TString tempos;
     
	strcpy(newname2, fNameTreeFile);
	gettimeofday(&fMt_autosave, &fTz);
	fIsUserInit =false;
       
	if (fTTreeMode > 0) {
		gROOT->cd();

		if (fTTreeWithRunNumber) {
			newname1 = RemoveChar(fNameTreeFile, (char*) ".root", false);
			sprintf(newname2, "%s_%4d.root", newname1, runnumber);
			delete [] newname1;
			ReplaceChar(newname2, ' ', '0');
			fError.TreatError(0, 0, "New name of Tree is ", newname2);
		}
		if (fReinitTree) {
			StopTree();
		}

		if ((fInitTreeNeverDone) || (fReinitTree)) {
			fTreeFile = new TFile(newname2, "recreate");
		} 

		fTreeFile->cd();
		fTreeFile->SetCompressionLevel(fTTreeCompressionLevel);

		if ((fInitTreeNeverDone) || (fReinitTree)) {
			fInitTreeNeverDone = false;
			        if (fTree == NULL){
					fTree = new TTree("AutoTree", "Automatic filled Tree");
					fError.TreatError(0, 0, "GTTree : New TTree");
				}
			
			if ((fTTreeMode == 1) ) {
				fError.TreatError(0, 0, "Initialization of TTree ");
				for (Int_t i = 0; i < event->GetArraySize(); i++) {
					parName = event->GetDataParameters()->GetParNameFromIndex(i);

					// il est necessaire de remplacer les - par des _
					ReplaceChar(&parName, '-', '_');
					parType = parName + "/s";
					fTree->Branch(parName, &(event->GetArray()[i]), parType);
				}
			}
			if (fTTreeMode == 2) {
				sprintf(tempo, "EventArray[%d]/s", event->GetArraySize());
				format = (TString) tempo;
				fError.TreatError(0, 0,
						"Initialization of TTree with format :", tempo);
				fTree->Branch("mybranche", (event->GetArray()), format);
			}

			if (fTTreeMode == 3) {
				fIsUserInit =true;
			}
		}// if ((fInitTreeNeverDone)|| (fReinitTree...
		else {
			// fTree= (TTree*) fTreeFile->Get("AutoTree");
		}
	} // if fTTreeMode >...
}

//______________________________________________________________________________
void GTTree::StopTree() { //Stop tree and save it
	if ((fTTreeMode > 0) && (fTreeFile)) {
		fError.TreatError(0, 0, "Closing TTree");
		fTreeFile->cd();
		fTree->Write();
		if (fReinitTree) {
			fTreeFile ->Close();
			fTreeFile = NULL;
		}
	}
}
//________________________________________________________________________________

void GTTree::AutoSave(){
gettimeofday(&fMt_reference, &fTz);
if (fTTreeMode > 0) {
	//toutes les diff_time seconde autosave du TTree
	if ((fMt_reference.tv_sec - fMt_autosave.tv_sec)> fDiff_time_save) {
			gettimeofday(&fMt_autosave, &fTz);
			fTreeFile-> cd();
			fTree->FlushBaskets();
			fTree->AutoSave();
		}
	}
}

//_________________________________________________________________________________

void GTTree::FillTTreeOnce() {
	// Fill Tree Once	
	if (fSkipFillTtree) {
		if (fTTreeMode > 0) {
			fTree->Fill();
		}
	}
	fSkipFillTtree = true;
}
//_________________________________________________________________________________
void GTTree::SetTTreeMode(TTreeMode mode, const char* filename, bool reinit,
		bool withrun) {
	// Set  mode of creation and configuration of TTree
	// mode= TREE_NO  : no TTree
	// mode = TREE_STANDARD   : Standart initialization is a sheet by parameter
	//  this conversion can be  slow .
	// mode = TREE_ONE_VECTOR  : TTree is initialised  with only on sheet with "my branch" as name. In this sheet we store
	//  a fix size vector whitch contain a values of event (all values of parameter, even a parameter is null)
	// mode= TREE_USER : user TTree but need to use GUser class
	// Name of outpout rootfile
	if ((mode != TREE_STANDARD) && (mode != TREE_NO) && (mode
			!= TREE_ONE_VECTOR) && (mode != TREE_USER) && (mode
			!= TREE_CIRCULAR)) {
		fError.TreatError(1, 0,
				"TTree mode is out of range, so Standard mode is selected");

		mode = TREE_STANDARD;
	}
	if ((withrun) && (!reinit)) {
		fError.TreatError(
				1,
				0,
				"no reinitialization is incompatible with taking account run number , so reinitialization is forced!\n");
		reinit = true;
	}
	fTTreeMode = mode;
	fReinitTree = reinit;
	fTTreeWithRunNumber = withrun;
	strcpy(fNameTreeFile, filename);
}
//_________________________________________________________________________________
////////////////////////////////////////fin /////////////////////////////////////
