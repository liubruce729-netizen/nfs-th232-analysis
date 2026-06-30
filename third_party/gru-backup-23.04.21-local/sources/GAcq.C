// File : GAcq.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GAcq
//
// This class do all data treatment
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

#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mtio.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <TObject.h>
#include <TThread.h>
#include <Riostream.h>
#include <TH1.h>
#include <TProfile.h>
#include "TROOT.h"
#include "TRint.h"
#include <stdlib.h>
#include <TFile.h>
#include <TString.h>
#include <TTree.h>
#include <TNtuple.h>
#include <TROOT.h>
#include <TFolder.h>
#include "General.h"

#include "DataParameters.h"
#include "GSpectra.h"
#include "GDevice.h"
#include "GAcq.h"
#include "Vigru.h"
#include "GEvent.h"
#include "GEventBase.h"
#include "GEventMFM.h"
#include "GInterrupt.h"
extern "C" {
#include "GEN_TYPE.H"
#include "STR_EVT.H"
#include "ERR_GAN.H"

#include "acq_codes_erreur.h"
}

//______________________________________________________________________________

ClassImp( GAcq);

GAcq::GAcq(GDevice* _fDevIn, GDevice* _fDevOut) {
	// Constructor/initializator of Acquisition object
	//
	// entry:
	// - Input Device
	// - Output Device

	// add interruption to catch Ctrlc
	 //Interrupt *IntHandle=new GInterrupt();
	  //IntHandle->Add();
	fOutAuthorized = false;
	SetDevIn(_fDevIn);
	SetDevOut(_fDevOut);
	fPrintRCbeforeStat= false;
	
	GAcqInit();
}

//_____________________________________________________________________________

GAcq::~GAcq() {

	gROOT->cd();
	// destructor of GAcq object

	if (fVerbose > 1)
		fError.TreatDebug(fVerbose, 0, "Delete Acquisition");
	if (fVerbose > 5)
		fError.TreatDebug(fVerbose, 0, "Delete Acquisition Thread");
	for (int i = 0; i < fCurrentNumberThread;i++) {
		if (fThreadPointer[i]) {
			TThread::Delete(fThreadPointer[i]);
			delete fThreadPointer[i];
			fThreadPointer[i] = NULL;
		}
	}
	delete[] fThreadPointer;
	if (fThreadProducer) {
		//procedure d'arret du thread
		TThread::Delete(fThreadProducer);
		delete fThreadProducer;

		//we reset to signal death
		fThreadProducer = NULL;
	}
	if (fMyCondition1){
		delete (fMyCondition1);
		fMyCondition1= NULL;
		}

 	if (fTheTree){
		delete fTheTree;
		fTheTree =NULL;
	}
	
	if (fVerbose > 5)
		fError.TreatDebug(fVerbose, 0, "Delete Acquisition Spectra  ");
	if (fSpectra) {
		delete (fSpectra);
		fSpectra = NULL;
	}

	if (fVerbose > 5)
		fError.TreatDebug(fVerbose, 0, "Delete Acquisition Scaler ");

	if (fScaler) {
		delete fScaler;
		fScaler = NULL;
	}
	if (fVerbose > 5)
		fError.TreatDebug(fVerbose, 0, "Delete Acquisition Event ");

	if (GetEvent()) {
		delete (GetEvent());
		SetEvent(NULL);
	}
	if (fVerbose > 5)
		fError.TreatDebug(fVerbose, 0, "Delete Acquisition DataParameter ");
	if (fDataParameters) {
		delete fDataParameters;
		fDataParameters = NULL;
	}

	if (fVerbose > 5)
		fError.TreatDebug(fVerbose, 0, "End of Delete Acquisition");

}
//_____________________________________________________________________________
void GAcq::GAcqInit() {
	// Called by every constructors.
	// Initialization of main attributes
	gROOT->cd();
	fVerbose = 0;
	fNumberMaxThread = 10;
	fThreadPointer = new TThread*[fNumberMaxThread];
	fCurrentNumberThread = 0;
	fThreadProducer = NULL;
	fEvent = NULL;
	fEventCount = 0;
	fBufferCount = 0;
	fSpectraMode = 0;
	fDataParameters = NULL;

	fCondition_of_stop = 0;
	fStop = false;
	fSpectra = NULL;
	fScaler = NULL;
	fBufferEmpty = true;

	fnbEventAsked = 0;
	fnbBufferAsked = 0;

	fPauseTimeout = 0;
	fReloadPause = false;
	fPause = false;
	fInPause = false;
	fInfiniteRead = false;
	fEventUSleep = 0;
	fEventSleep = 0;
	fInEvtTreatment = false;

	fMyCondition1 = new TCondition(&fMyMutex1);
  	fTheTree = new GTTree();
	
	fScaler  = new GScaler();
	fSpectra = new GSpectra(); // spectra data base.
        fSpectra->SetVerbose(fVerbose);
        fScaler->SetVerbose(fVerbose);
	if (fVerbose > 5)
		fError.TreatDebug(fVerbose, 0, "Creation de fSpectra ");

	SetTTreeMode(); // Standard setting (no TTree)
	SetScalerMode(); // standard setting (no Scaler TTree)
	SetConfGanilAcq(); // Set Configuration on Correlation with Ganil Acquisition

	fRateEvent = 0; // Our event  counter rate
	fRateBuffer = 0; // Our Buffer counter rate
	fRateBuffer_evt = 0; // Our Buffer counter rate

	fConfGanilAcq = false; // Correlation with Ganil Acquisition
	fAcquisitionOn = false;
	fStoreOn = false;
	fGanilAcqOn = false;
	fRunNumber = 0; // current run number
	fInfoCont = false; // give acquisition information in continuous
	fSucces = 0;
	fSucces_mean = 0;
	fSucces_mean_total = 0;
	fSucces_nb_total = 0;

	TFolder *grufolder = gROOT->GetRootFolder()->AddFolder("GRU", "GRU_data");
	gROOT->GetListOfBrowsables()->Add(grufolder, "GRU");
	grufolder->Add(fSpectra);

}

//_____________________________________________________________________________
void GAcq::FillTree(void * arg) {
	// Fill  TTree and treatment of events
	//treeMode parameter:
	//0 : no TTree
	//1 : Standard initalisation is a sheet by parameter
	//  this conversion can be  slow .
	//2 : TTree is initialised  with only on sheet with "my branch" as name. In this sheet we store
	// a fix size vector which contain a values of event (all values of parameter, even a parameter is null)
	// b_spectra = true , authorize the creation of standard histogram
	//  if   iGAcq->fnbEventAsked= 0 (default ), all events will be treated, else only iGAcq->fnbEventAsked events will be treated,
	//  if   iGAcq->fnbBufferAsked= 0 (default ), all buffer will be treated, else only iGAcq->fnbBufferAsked buffers will be treated,

	//TThread::
	// we can can the thread only un this part
	//TThread::SetCancelDeferred();

	GAcq* iGAcq = (GAcq*) arg; // instance of GAcq given in parameter

	TString tempos;

	struct timeval mt_reference;
	struct timeval mt_info;// to compute time out
	struct timeval mt_autosp;// to compute cycle information
	struct timeval mt_Event, mt_Buffer; // time structure to compute rate
	//  struct timeval mt_Timeout;  // time to validate a timeout (if is validated)
	struct timezone tz;

	gettimeofday(&mt_info, &tz);
	gettimeofday(&mt_autosp, &tz);
	gettimeofday(&mt_Event, &tz);
	gettimeofday(&mt_Buffer, &tz);

	UInt_t nbEvents = 0; // use to count events and do a save when a level is reached
	Int_t diff_nbEvents = 0;
	//Int_t diff_time_save = 60; // time between 2 tempory save
	Int_t diff_time = 3, current_diff_time = 0;
	Int_t time_out = 1800; // go out when time_out without events is reached ( in second)

	UInt_t nBuffers = 0; // nb of buffers
	Int_t nBuffers_ev = 0; // nb of event buffers
	Int_t diff_nBuffers = 0; //
	Int_t diff_nBuffers_ev = 0; // differcence betx event buffers
	Int_t nBuffers_send = 0;

	Int_t diff_nBuffers_send = 0;
	Int_t diff_nBuffers_send_ev = 0;

	iGAcq->fStop = false;
	iGAcq->fSucces = 0;

	tempos.Form(
			"Begin of acquisition data----with-%d-asked-events-or-%d-asked-buffers-asked- (0=full)--",
			iGAcq->fnbEventAsked, iGAcq->fnbBufferAsked);
	if (iGAcq->fPrintRCbeforeStat)cout<<endl;
	(iGAcq->fError).TreatError(0, 0, tempos);

	while (true) {// on input buffer
		//TThread::CancelPoint();
		//-----------------------------------------------------
		//   if (iGAcq->GetRunStatus()){
		//    if (fBufferEmpty)

		iGAcq->GetDeviceIn()->ReadBuffer();

		if (iGAcq->fInfiniteRead) {// a remplacer par qq chose avec fStatus = ACQ_ENDOFFILE;
			if ((iGAcq->GetDeviceIn()->GetCurrentBuffer()->GetType()
					== ENDRUN_Idn) || (iGAcq->GetDeviceIn()->GetStatus()
					== ACQ_ENDOFFILE)) {
				iGAcq->GetDeviceIn()->Rewind();
				while (true) {
					iGAcq->GetDeviceIn()->ReadBuffer();
					if (iGAcq->GetDeviceIn()->GetBuffer()->IsAEventBuffer())
						break;
				}
			}
		};

		if (iGAcq->GetEvent()==NULL) iGAcq->fError.TreatError(3, 0, "GetEvent() NULL\n");
		iGAcq->GetEvent()->RazEvent();

		// time out in case of no event during "time_out"
		gettimeofday(&mt_reference, &tz);
		if (((mt_reference.tv_sec - mt_info.tv_sec) > time_out) && (nbEvents
				== 0)) {
			iGAcq->fError.TreatError(1, 0, "Time out with no event\n");
			break;
		};

		if (iGAcq->GetDeviceIn()->GetStatus() != ACQ_OK) {
			if (iGAcq->GetDeviceIn()->GetStatus() != ACQ_UNKBUF)
				break;
		}
		//cout <<"debug iGAcq->GetDeviceIn() = "<<iGAcq->GetDeviceIn()<<"\n";
		//cout <<"debug iGAcq->GetDeviceIn() = ->GetCurrentBuffer()"<<iGAcq->GetDeviceIn()->GetCurrentBuffer()<<"\n";;
		//cout <<"debug iGAcq->GetDeviceIn()->GetCurrentBuffer()->IsAEventBuffer() = "<<iGAcq->GetDeviceIn()->GetCurrentBuffer()->IsAEventBuffer()<<"\n";;
		//iGAcq->GetDeviceIn()->GetCurrentBuffer()->DumpBuffer(32,0);
		if (iGAcq->GetDeviceIn()->GetCurrentBuffer()->IsAEventBuffer()) {
			nBuffers_ev++;
		} // count only event buffers
		nBuffers++;// count all valid buffers
		iGAcq->fBufferCount = nBuffers;

		// case of In2p3 Scaler Buffer
		if (iGAcq->GetDeviceIn()->GetCurrentBuffer()->fGBuf_type == SCALER_Idn) {
			iGAcq->fScaler->ScalerTreatement(
				((GBufferIn2p3*) (iGAcq->GetDeviceIn()->GetCurrentBuffer())),nbEvents);
		}

		// case of Events Buffer
		//----------------------------

		if (iGAcq->GetDeviceIn()->GetCurrentBuffer()->fIsAEventType) {

			gettimeofday(&mt_reference, &tz);
			if (iGAcq->GetEvent()->EventInitAlready()) {

				while (true) {// event treatment

					iGAcq->fStatus = iGAcq->GetEvent()->NextEvent(
							(iGAcq->GetDeviceIn()->GetCurrentBuffer()));
					if (iGAcq->fStatus != ACQ_OK) {
						iGAcq->fBufferEmpty = true;
						break;
					}
					//} else { if(GetDeviceIn()->fType == GT_TYPE_FILE )fBufferEmpty = false; }
					//cout << "Debug GAcq Event Treat\n";
					nbEvents++;
					iGAcq->fEventCount = nbEvents;
					iGAcq->IsOnPause();
					iGAcq->fInEvtTreatment = true;
					iGAcq->IsOnSleep();

					iGAcq-> User();
					// fill raw spectra
					if (iGAcq->fSpectraMode > 0) {
						iGAcq->fSpectra->FillRawSpectra(iGAcq->GetEvent()); // increment of all Raw spectra ;
					}

					// Fill TTree
					iGAcq->fTheTree->FillTTreeOnce();
					iGAcq->fInEvtTreatment = false;

					if ((iGAcq->fnbEventAsked != 0) && (nbEvents
							>= iGAcq->fnbEventAsked)) {
						iGAcq->fStop = true;
					}
					if (iGAcq->fStop) {//go out when nb of wanted events is reached
						break;
					}

				} // end of while ( buffer is empty)
				iGAcq->fTheTree->AutoSave();
			} // end of if Init done

			//all  diff_time secondes  rate computing
			current_diff_time = (mt_reference.tv_sec - mt_autosp.tv_sec);

			if ((current_diff_time) >= diff_time) {
				nBuffers_send = iGAcq-> GetDeviceIn()->GetBuffer()->GetNumBuf();
				diff_nBuffers_send = nBuffers_send - diff_nBuffers_send;
				diff_nBuffers_send_ev = nBuffers_send - diff_nBuffers_send_ev;
				diff_nBuffers = nBuffers - diff_nBuffers;
				diff_nBuffers_ev = nBuffers_ev - diff_nBuffers_ev;
				diff_nbEvents = nbEvents - diff_nbEvents;
				iGAcq->fRateEvent = ((Float_t)(diff_nbEvents) / (Float_t)(
						current_diff_time));
				iGAcq->fRateBuffer = ((Float_t)(diff_nBuffers) / (Float_t)(
						current_diff_time));
				iGAcq->fRateBuffer_evt = ((Float_t)(diff_nBuffers_ev)
						/ (Float_t)(current_diff_time));

				//ut<<"diff_nBuffers_ev = "<<diff_nBuffers_ev <<"  diff_nBuffers_send_ev = "<<diff_nBuffers_send_ev<<"\n";
				if (diff_nBuffers_send_ev > 0) {
					iGAcq->fSucces = diff_nBuffers_ev * 100
							/ diff_nBuffers_send_ev;
				}

				iGAcq->fSucces_mean_total += iGAcq->fSucces;
				if (iGAcq->fSucces_nb_total != 0)
					iGAcq->fSucces_mean = iGAcq->fSucces_mean_total
							/ iGAcq->fSucces_nb_total;
				iGAcq->fSucces_nb_total++;
				gettimeofday(&mt_autosp, &tz);

				if (((iGAcq->fSucces > 100) && (iGAcq->GetDeviceIn()->GetType()
						== GT_TYPE_TAPE)) or (iGAcq->GetDeviceIn()->GetType()
						== GT_TYPE_FILE))
					iGAcq->fSucces = 100;
				if (iGAcq->fPrintRCbeforeStat)cout<<endl<<flush;
				tempos.Form(
						"Event_Rate = %.2f Buffer_Rate = %.2f (%.2f) Succes_Rate = %.2f (Mean %.2f)",
						iGAcq->fRateEvent, iGAcq->fRateBuffer,
						iGAcq->fRateBuffer_evt, iGAcq->fSucces,
						iGAcq->fSucces_mean);
				(iGAcq->fError).TreatError(0, 0, tempos);
				diff_nbEvents = nbEvents;
				diff_nBuffers_ev = nBuffers_ev;
				diff_nBuffers = nBuffers;
				diff_nBuffers_send = nBuffers_send;
				diff_nBuffers_send_ev = nBuffers_send;
			}
		} // end of if eventBuffer

		if ((iGAcq->fnbBufferAsked != 0) && (nBuffers >= iGAcq->fnbBufferAsked)) {
			iGAcq->fStop = true;
		}

		if (iGAcq->fOutAuthorized) {
			if (iGAcq->GetDeviceOut()->GetIsOpen()) {
				iGAcq->GetDeviceOut()->WriteBuffer(
						iGAcq->GetDeviceIn()->GetBuffer());
	
			} else {
				iGAcq->fError.TreatError(3, 0, "Output Device is not open");
			}
		}

		if (iGAcq->fStop) {
			break;
		}
		
	}// end of while (true ) on inputbuffer

	(iGAcq->fError).TreatError(0, 0, "---End of acquisition data----");
	if (iGAcq->fPrintRCbeforeStat)cout<<endl<<flush;
}

//______________________________________________________________________________

void GAcq::SetDevIn(GDevice* _fDevIn) {
	// Set  input device
	fDevIn = _fDevIn;
}

//______________________________________________________________________________

void GAcq::SetDevOut(GDevice* _fDevOut) {
	// Set  input device
	fDevOut = _fDevOut;
	SetOut(true);

}

//______________________________________________________________________________

void GAcq::SetOut(Bool_t out) {
	// Set  input device

	if (GetDeviceOut())
		fOutAuthorized = out;
	else
		fOutAuthorized = false;

}

//______________________________________________________________________________

void GAcq::SetConfGanilAcq(int mode) {
	// Set  flag to be have a correlation with Ganil Acquisition
	// Default is mode = 0
	if (mode > 0)
		fConfGanilAcq = true;
	else
		fConfGanilAcq = false;
}

//______________________________________________________________________________

void GAcq::SetInfoCont(int mode) {
	// Set  flag to be have continuous information about  Acquisition
	// Default is mode = 0
	if (mode > 0)
		fInfoCont = true;
	else
		fInfoCont = false;
}
//______________________________________________________________________________
void GAcq::SetEventUSleep(int evtusleep) {
	// make a pause of evtusleep microseconds for each event
	fEventUSleep = evtusleep;
}
//______________________________________________________________________________
void GAcq::SetEventSleep(int evtsleep) {
	// make a pause of evtusleep  seconds for each event
	fEventSleep = evtsleep;
}

//______________________________________________________________________________
void GAcq::SetUserMode(int mode) {
	// Set User treatment mode
	TString tempos;
	tempos.Form("The Method " "SetUserMode" " is obsolete , Use InitUser() !");
	fError.TreatError(1, 0, tempos);
	InitUser();
}
//______________________________________________________________________________

void GAcq::SetScalerMode(TTreeMode mode, const char* filename, bool reinit,
		bool withrun) {
	fScaler->SetScalerMode(mode, filename, reinit, withrun);
}

//______________________________________________________________________________

void GAcq::SetSpectraMode(int mode, char* para_name, int nb_spec,
		TString family, Int_t auto_level) { // Set  mode of creation of raw histograms
	// 0 : No Spectra predefined, A empty Spectra Data Base is validate,
	//     user  histograms can be defined and incremented if user treatment is validated (see SetUserMode)
	// 1 : All Raw Parameters increment histograms, user histograms can be defined also.
	// if =1 and para_name defined only nb_spec parameter will be incremented and first is "para_name"
	// by default family = "Raw"
	// auto_level allow to auto select raw parameters in sub family if names of parameters contain "_"
	// auto_level = 1-> max of sub_family =1; auto level = 2-> max of sub_family =2; etc...
	// by default auto_level =0

	if (GetEvent()) {
		if (!GetEvent()->EventInitAlready()) {
			fError.TreatError(1, 0,
					"Event is not initialized, so  SetSpectraMode have no effect!");
			return;
		}
	} else {
		fError.TreatError(1, 0,
				"Event is not instanced, so  SetSpectraMode have no effect!");
		return;
	}

	GetSpectra()->AddAllParameterInDB(GetDataParameters(), family, auto_level);

	fStatus = ACQ_OK;
	if ((mode > 3) || (mode < 0)) {
		fError.TreatError(1, 0,
				" Spectra mode is out of range, so Standard mode is selected.");
		mode = 1;
	}
	fSpectraMode = mode;
	fError.TreatError(0, 0, "Initialization of Spectra");

	if (fSpectraMode > 0) {
		if (!GetEvent()->EventInitAlready()) {
			fError.TreatError(1, 0,
					"Impossible to init spectra if init event not done!");
			return;
		}
		fSpectra->StartAllRawParameters(); //creation of all histograms of all raw parameters
	}
	

}

//______________________________________________________________________________

void GAcq::SetNetMode(Int_t mode, Int_t port) {

	// start(more=1) or stop (mode=0) spectra server
	// port number can be given
	TString tempos;
	tempos = "\n";
	tempos += "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
	tempos += "+  SetNetMode() method is obsolete                         +\n";
	tempos += "+  You have to declare spectra server separately and       +\n";
	tempos += "+  Analysis objects : example :                             +\n";
	tempos += "+     GUser a = new GUser(input_device);                   +\n";
	tempos += "+ or  GAcq  a = new GAcq (input_device);                   +\n";
	tempos += "+     int port = 9090;                                     +\n";
	tempos += "+     GNetServerRoot  serv = new GNetServerRoot (port,a);  +\n";
	tempos += "+     serv->StartServer();                                 +\n";
	tempos += "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
	fError.Infos(tempos);

}

//______________________________________________________________________________
void GAcq::SpeSave(const char* filename) {

	//Save the spectra
	if (fSpectra)
		fSpectra->SpeSave(filename);

}

//______________________________________________________________________________

void GAcq::Infos() {
	// Give few informations about device : name , type ...;
	// No entry.
	TString tempos;
	fError.TreatError(0, 0, "\n Acquisition informations\n");
	cout << " Input  Device Name               : " << fDevIn-> GetName()
			<< "\n";
	cout << " Input  Device Point              : " << fDevIn << "\n";
	if (fDevOut)
		cout << " Output Device Name               : " << fDevOut->GetName()
				<< "\n";
	cout << " Buffer Size                      : ";
	cout << fDevIn->GetCurrentBuffer()->GetBufSize();
	cout << "\n";
	cout << " Spectra Mode                     : " << fSpectraMode << "\n";
	cout << " TTree Mode                       : " << fTheTree->GetTTreeMode();
	if (fTheTree->GetTTreeMode()>0)
		cout << "->" << (char*)(fTheTree->GetNameTreeFile());
	cout << "\n";
	cout << " Scaler Mode                      : " << fScaler->GetScalerMode();
	if (fScaler->GetScalerMode())
		cout << "->" << fScaler->GetNameScalerTreeFile();
	cout << "\n";
	cout << " Status                           : " << fStatus << "\n";
	cout << " Acquisition State                : " << fAcquisitionOn << "\n";
	if (fAcquisitionOn) {
		cout << " Acquisition Event Rate            : " << fRateEvent << "\n";
		cout << " Acquisition Buffer Rate           : " << fRateBuffer << "\n";
		cout << " Acquisition Succes Rate in        : " << fSucces << "%\n";
	}
	if (fConfGanilAcq) {
		GetRunStatus(true);
		cout << " Ganil Acquisition State          : " << fGanilAcqOn << "\n";
		cout << " Ganil Acquisition Store          : " << fStoreOn << "\n";
		cout << " fRunNumber                       : " << fRunNumber << "\n";
	}
}
//______________________________________________________________________________
TString GAcq::GetInfo(Int_t* nb_info) {
	// Give few informations about device : name , type ...;
	// No entry.
	TString info;
	TString tempos;

	info.Form(
			"Input_Device_Name %s  Buffer_Size %d Acquisition_Event_Rate %f Acquisition_Buffer_Rate %f Acquisition_Succes_Rate %f ",
			fDevIn-> GetName(), fDevIn->GetCurrentBuffer()->GetBufSize(),
			fRateEvent, fRateBuffer, fSucces);
	*nb_info = 5;

	return info;
}
//______________________________________________________________________________

void GAcq::SpectraList() {
	//  Print all name of defined spectra ( histogram)

	if (fSpectra)
		fSpectra->GetDB()->SpectraList();
	else
		fError.TreatError(1, 0, "No Available Spectrum ");

}
//___________________________________________________________________________________

void GAcq::DumpBuffer(char* c) {
	// idem DumpBuffer(char c)
	// allows to take in account this kind of write : DumpBuffer("b") instead DumpBuffer('b');
	char b;
	b = c[0];
	DumpBuffer(b);
}

//_____________________________________________________________________________

void GAcq::DumpBuffer(char c) {
	// Dump raw data  of a current input device buffer
	// char =  'n' to dump next data of current dumped block
	//      =  'b' to dump next block (buffer) (in this case a new read of a buffer is done on device)

	fDevIn->DumpBuffer(c);
}

//_____________________________________________________________________________

void GAcq::DumpEvent(char* c, char* m, bool nozero, bool ctrl) {
	// idem DumpEvent(char c)
	// allows to take in account this kind of write : DumpEvent("b") instead DumpEvent('b');
	char b;
	char f; // = b,d,o or h for binary ,decimal, octal or hexadecimal
	f = m[0];
	b = c[0];
	DumpEvent(b, f, nozero, ctrl);
}

//_____________________________________________________________________________

void GAcq::DumpEvent(char c, char mode, bool nozero, bool ctrl) {
	// Dump event with automatic pointer  incrementation in buffer
	// Dump to console events data of a buffer
	// c    =  'n' to dump next event data of current dumped block
	//      =  'b' to dump next block (buffer)
	//         (in this case a new read of a buffer is done on device)
	// mode = b,d,o or h for binary ,decimal, octal or hexadecimal
	// nozero =true, all zero parameters are not displayed (default)
	// Ctrl   =true, Ctrl event is displayed (default)
	TString tempos;
	bool contextopen = fDevIn->GetIsOpen();
	fStatus = ACQ_OK;
	if (!GetEvent()->EventInitAlready()) {
		fError.TreatError(1, 0, "Event dump not possible, init event not done!");
		return;
	}

	if (contextopen == false)
		fDevIn->Open('r');

	if (fDevIn->GetIsOpen() == false) {
		fError.TreatError(1, 0, "Device not opened ");
		return;
	}

	if (fDevIn->GetType() == GT_TYPE_DIR) {
		tempos.Form("No dump on a directory device :%s", fDevIn->GetName());
		fError.TreatError(1, 0, tempos);
		return;
	}

	switch (c) {
	case 'n':
		tempos.Form("---------------In buffer %d nb :%d------",
				fDevIn->GetCurrentBuffer()->fGBuf_type,
				fDevIn->GetCurrentBuffer()->fGBuf_index);
		if (fDevIn->GetCurrentBuffer()->GetType() == SCALER_Idn
				|| fDevIn->GetCurrentBuffer()->GetType() == SCALER_SWAP_Idn) {
			fError.TreatError(0, 0, tempos);
			DumpScaler();
		} else {
			fStatus = GetEvent()->NextEvent(fDevIn->GetCurrentBuffer());
			if (fStatus != ACQ_OK) {
				fError.TreatError(1, fStatus,
						"Next Event : Go to a other buffer");
				break;
			}
			if (ctrl) {
				GetEvent()->DumpEvent(mode);
			} else {
				GetEvent()->DumpArray(mode, nozero);
			}
		}
		break;
	case 'b':
		fDevIn->ReadBuffer();
		tempos.Form("---------------In buffer %d nb :%d------",
				fDevIn->GetCurrentBuffer()->fGBuf_type,
				fDevIn->GetCurrentBuffer()->fGBuf_index);
		if (fDevIn->GetCurrentBuffer()->GetType() == SCALER_Idn
				|| fDevIn->GetCurrentBuffer()->GetType() == SCALER_SWAP_Idn) {
			fError.TreatError(0, 0, tempos);
			DumpScaler();
		} else {

			GetEvent()->RazEvent();
			if (fDevIn->GetStatus() != ACQ_OK) {
				fError.TreatError(3, fStatus, "  read error ");
			} else {

				fError.TreatError(0, 0, tempos);
				if (fDevIn->GetCurrentBuffer()->fIsAEventType) {
					fStatus = GetEvent()->NextEvent(fDevIn->GetCurrentBuffer());
					if (fStatus != ACQ_OK) {
						fError.TreatError(1, fStatus,
								" No Event to dump , probably a empty buffer ");
						break;
					}
					if (ctrl) {
						GetEvent()->DumpEvent(mode);
					} else {
						GetEvent()->DumpArray(mode, nozero);
					}
				} else
					fError.TreatError(1, 0, "No event in this buffer");
			}
		}
		break;
	default:
		fError.TreatError(1, 0, "Invalid argument , must be 'n' or ' b' !");
		break;
	}

	if (contextopen == false)
		fDevIn->Close();
}
//_____________________________________________________________________________
void GAcq::DumpCurrentEvent(char mode, bool nozero, bool ctrl) { //Dump current event with no automatic incrementation pointer in buffer
	// (with no getnextevent included in method)

	// mode = b,d,o or h for binary ,decimal, octal or hexadecimal
	// nozero =true, all zero parameters are not displayed (default)
	// Ctrl   =true, Ctrl event is displayed ( not default)
	//

	if (ctrl) {
		GetEvent()->DumpEvent(mode);
	} else {
		GetEvent()->DumpArray(mode, nozero);
	}

}
//_____________________________________________________________________________

void GAcq::DumpScaler() {
	// Dump current Scalers

	fScaler->GetScalerEvents((GBufferIn2p3*) (fDevIn->GetCurrentBuffer()));
	fScaler->DumpScaler();
}

//______________________________________________________________________________

void GAcq::SetTTreeMode(TTreeMode mode, const char* filename, bool reinit,
		bool withrun) {
  if (fTheTree!=NULL) 
	fTheTree->SetTTreeMode( mode,  filename,  reinit, withrun) ;
  else 
	fError.TreatError(3, 0, "GAcq::SetTTreeMode TTree is not constructed ");
}

//______________________________________________________________________________
void GAcq::InitTTree() {
fTheTree->InitTTree(fRunNumber,GetEvent());
  if (fTheTree->IsUserInit() ) {
	InitTTreeUser();
  }
}
//______________________________________________________________________________
void GAcq::StopTree() {
	 //Stop tree and save it
	fTheTree->StopTree();
	fScaler -> Stop();
}

//______________________________________________________________________________
void GAcq::Pause(int timeout) {
	// execute a pause  with timeout in second
	// if timeout is negative timeout is no limit
	// if timeout = 0 , pause is resumed

	fPauseTimeout = (Float_t) timeout;
	if (timeout == 0) {
		ResumePause();
	} else {

		if (fPause == true) {
			fReloadPause = true;
			fMyCondition1->Signal();
		}
		fPause = true;
	}
	//fError.Infos("Pause in GAcq::Pause");

}
//______________________________________________________________________________
void GAcq::ResumePause() {
	//fError.Infos("Resume Pause in GAcq::ResumePause");

	fPause = false;
	fPauseTimeout = 0;
	fReloadPause = false;
	fMyCondition1->Signal();

}
//______________________________________________________________________________
void GAcq::IsOnSleep() {
	if (fEventUSleep > 0)
		usleep(fEventUSleep);
	if (fEventSleep > 0)
		sleep(fEventSleep);
}
//______________________________________________________________________________
void GAcq::IsOnPause() {
	// execute au pause  with timeout
	// if fPauseTimeout is negative timeout is no limit
	if (fPause) {
		fReloadPause = false;
		bool first = true;
		fInPause = true;
		while ((fReloadPause == true) || (first == true)) {
			first = false;
			fReloadPause = false;
			if (fPauseTimeout > 0) {
				fMyCondition1->TimedWaitRelative(
						(ULong_t)(fPauseTimeout * 1000));
			}
			if (fPauseTimeout < 0) {
				fMyCondition1->Wait();
			}
		}
		fInPause = false;
		fPause = false;
	}
}

//______________________________________________________________________________
void GAcq::DoRunWait(UInt_t nEvents, UInt_t nBuffers) {
	//same as DoRun but wait for keyboard return for each event or set of event.
	// useful to debug
	bool conti = true;
	char c;
	if (fAcquisitionOn) {
		fError.TreatError(1, 0, "Acquisition is already started\n");
		return;
	}

	TString tempos;

	gettimeofday(&fMt_begin, &fTz);
	//fAcquisitionOn = true;
	SetStop(false);
	//InitUserRun();
	GetRunStatus();
	InitTTree();
	InitUserRun();
	fnbEventAsked = nEvents;
	fnbBufferAsked = nBuffers;

	while (conti) {
		FillTree((void*) this);
		printf("'Enter' to step  -  'q' to quit  - 'c' to continue\n");
		c = getchar();
		if (c == 'q')
			conti = false;
		if (c == 'c') {
			fnbEventAsked = 0;
			fnbBufferAsked = 0;
		}
		if (fStatus != ACQ_OK)
			conti = false;
	}

	EndUserRun();

	StopTree();

	gettimeofday(&fMt_end, &fTz);
	tempos.Form("Working time : %lld seconds for %d events, %d buffers ",
			(Long64_t)(fMt_end.tv_sec) - (Long64_t)(fMt_begin.tv_sec),
			fEventCount, fBufferCount);
	fError.TreatError(0, 0, tempos);
}
//______________________________________________________________________________
void GAcq::DoRun(UInt_t nEvents, UInt_t nBuffers) {
	// Automatically create  fill a tree created from ganil data.
	// Can be use to convert a ganil tape or file to a ROOT file.
	// The number of actual converted events is set with the nEvents parameter.
	//

	if (fAcquisitionOn) {
		fError.TreatError(1, 0, "Acquisition is already started\n");
		return;
	}
       
	TString tempos;
	gettimeofday(&fMt_begin, &fTz);
	SetStop(false);
	//InitUserRun();
	GetRunStatus();
	InitTTree();
	InitUserRun();	
	fnbEventAsked = nEvents;
	fnbBufferAsked = nBuffers;
	fAcquisitionOn = true;
	FillTree((void*) this);
	EndUserRun();
	StopTree();
	fAcquisitionOn = false;
	gettimeofday(&fMt_end, &fTz);
	tempos.Form("Working time : %lld seconds for %d events, %d buffers ",
			(Long64_t)(fMt_end.tv_sec) - (Long64_t)(fMt_begin.tv_sec),
			fEventCount, fBufferCount);
	fError.TreatError(0, 0, tempos);
}

//______________________________________________________________________________
void GAcq::DoRunT(UInt_t nEvents, UInt_t nBuffers) {
	// Automatically create  fill a tree created from ganil data.
	// Can be use to convert a ganil tape or file to a ROOT file.
	// The number of actual converted events is set with the nEvents parameter.
	//


	if (fInPause) {
		ResumePause();
		return;
	}

	if (fAcquisitionOn) {
		fError.TreatError(1, 0, "Acquisition is already started\n");
		return;
	}

	TString tempo;
	gettimeofday(&fMt_begin, &fTz);

	fnbEventAsked = nEvents;
	fnbBufferAsked = nBuffers;

	SetStop(false);
	GetRunStatus();
	InitTTree();
	InitUserRun();
	fAcquisitionOn = true;
	if (!(fDevIn->GetIsOpen())) {

		fDevIn->Open();
		if (!(fDevIn->GetIsOpen())) {
			fError.TreatError(1, 0, "No open connection or no open file");
			fStatus = ACQ_DISCONNECT;
		}
	}
	if (fDevIn->GetIsOpen()) {

		if (!fDevIn->GetBusy()) {
			// si le thread n'existe pas, on le cree
			if (fThreadProducer == NULL) {
				fThreadProducer = new TThread("GRUThread",
						(void(*)(void*)) &FillTree, (void*) this);

				// ... et on lance l'execution du thread
				fThreadProducer ->Run();
				fDevIn->SetBusy(true);
			}
		} else {
			fError.TreatError(1, 0, "Input Device is busy");
		}
	}
	if (fStatus == ACQ_OK)
		fAcquisitionOn = true;

}
//_________________________________________________________________________________
void GAcq::SetStop(bool stop) {

	if ((fAcquisitionOn == false) && (stop == true)) {
		fError.TreatError(0, 0, "Acquisition already stopped");
		fStop = stop;

	} else {
		fStop = stop;
		usleep(1000);
		if (stop)
			StopAcq();

	}
}
;
//_________________________________________________________________________________
void GAcq::StopAcq() {
	// stop an acquisition running onto a monoprocessor architecture :
	// stop the running thread, calculate the performance of the acquisition and open a new connexion :
	// because of thread, the previous connection seems to be lost suddenly
	TString tempos;

	fAcquisitionOn = false;

	if (fThreadProducer) {
		//procedure d'arret du thread
		fThreadProducer->Kill();
		fError.TreatError(0, 0, "Acquisition stopping");
	}
	if (fThreadProducer) {
		while (fThreadProducer->GetState() == TThread::kRunningState) {
			tempos.Form("waiting   %lld %lld", (Long64_t) fThreadProducer,
					(Long64_t) fThreadProducer->GetState());
			fError.TreatError(0, 0, tempos);
			usleep(1000);
		}
	}
	usleep(500);
	EndUserRun();
	StopTree();
	fDevIn->SetBusy(false);
	fAcquisitionOn = false;

	if (fThreadProducer) {
		TThread::Delete(fThreadProducer);
		fThreadProducer = NULL;
	}

	fError.TreatError(0, 0, "Acq Completely stopped");
	gettimeofday(&fMt_end, &fTz);

	tempos.Form("Working time :%lld seconds for %d events %d buffers ",
			(Long64_t)(fMt_end.tv_sec - fMt_begin.tv_sec), fEventCount,
			fBufferCount);
	fError.TreatError(0, 0, tempos);
}
//______________________________________________________________________________
void GAcq::DumpParameterName(void) {
	// Dump parameter index and name
	GetDataParameters()->DumpListOfNames();
}
//______________________________________________________________________________
void GAcq::TestLaunch() {
	// Launch a test in a Thread

	if (fCurrentNumberThread > fNumberMaxThread) {
		fError.TreatError(1, 0, "Max Number of Thread Reached");
		return;
	}
	TString name ;
	//name.Form("TTest_%d",fCurrentNumberThread);
	name.Form("TTest_%d",fCurrentNumberThread);
	fThreadPointer[fCurrentNumberThread] = new TThread(name,
			(void(*)(void*)) &TestNew, (void*) ((long) fCurrentNumberThread));

	// ... et on lance l'execution du thread
	fThreadPointer[fCurrentNumberThread] ->Run();
	fCurrentNumberThread++;
	cout << " Test of launching just a compter , do nothing else , nothing with data"<<endl;
	cout <<"Test "<<(fCurrentNumberThread-1)<< " Launched\n";
	
	return;
}

//______________________________________________________________________________
void GAcq::TestNew(void * arg) {
// methode to test a thread. Do do nothing with data
	int i = 0;
	long j = (long) (arg);

	for (i = 0; i < 20; i++) {
		sleep(1);
		cout << "Test" << j << "  " << i << "\n";
	}

	cout << "Test" << j << "  ended\n";
	return;
}

//______________________________________________________________________________
void GAcq::Stat(bool verb) {
	// Do statistics and tests on buffers and events that come from the device
	// if device is a tape, statistics will done on a  whole current run
	// if test_multi test if a label is present more than once
	TString tempos;
	TString namepar;
	Int_t nbEvents = 0; // use to count events and do a save when a level is reached
	Int_t nbEventsByBuf = 0;
	Int_t NbBufENDRUN = 0;
	Int_t NbBufEVENTDB = 0;
	Int_t NbBufEBYEDAT = 0;
	Int_t NbBufJBUS = 0;
	Int_t NbBufCOMMENT = 0;
	Int_t NbBufSCALER = 0;
	Int_t NbBufFILEH = 0;
	Int_t NbBufEVENTH = 0;
	Int_t NbBufEVENTCT = 0;
	Int_t NbBufPARAM = 0;
	Int_t NbBufSTATUS = 0;
	Int_t NbBufUNKNOWN = 0;
	Int_t NbBuf = 0;
	Int_t first_index = 0;
	//Int_t last_index = 0;
	//Int_t para_max = 0;
	//Int_t value = 0;
	Int_t nb_error_lab_ctrl = 0;
	Int_t nb_multi_label = 0;
	Int_t nb_good_lab_ctrl = 0;
	Int_t noevent = 0;
	int labelc;
	Int_t i;
	bool test_multi = true;

	Int_t* LabelToExist = NULL; // vector to count nb of appearance of a label in a run ( use in stat)
	Int_t* LabelMulti = NULL;// vector to count nb of appearance of a label in a run ( use in stat)
	Int_t* LabelMultiTmp = NULL;// vector to count nb of appearance of a label in a run ( use in stat)
	Int_t max_label;
	Int_t max_label2;

	struct timeval mt_begin, mt_end; // time structure
	struct timezone tz;
	gettimeofday(&mt_begin, &tz);
	max_label = 0;
	max_label2 = 0;
	fStatus = ACQ_OK;

	if (fDevIn->GetIsOpen() != true) {
		fDevIn->Open();
		if (fStatus != ACQ_OK) {
			fError.TreatError(1, -1, "Device not opened");
			return;
		}
	} else {
		fDevIn->Rewind();
	}

	if (GetEvent()->EventInitAlready()) {
		LabelToExist = GetDataParameters()->creatLabelToExist(
				&max_label);
		LabelMulti = GetDataParameters()->creatLabelToExist(
				&max_label2);
		LabelMultiTmp = GetDataParameters()->creatLabelToExist(
				&max_label2);

		for (i = 0; i <= max_label2; i++) {
			LabelMulti[i] = 0;
			LabelMultiTmp[i] = 0;
		}

	}

	while (true) {// While on Buffer

		fDevIn-> ReadBuffer();
		GetEvent()->RazEvent();
		if (fDevIn->GetStatus() != ACQ_OK) {
			break;
		}
		NbBuf++;

		if (verb) {
			tempos.Form("Buffer : index = \t%d   type = %d\n",
					fDevIn->GetCurrentBuffer()->fGBuf_index,
					fDevIn->GetCurrentBuffer()->fGBuf_type);
			fError.TreatError(0, 0, tempos);
		}
		if (first_index == 0)
			first_index = fDevIn-> GetCurrentBuffer()->fGBuf_index;
		//if (fDevIn->GetCurrentBuffer()->fGBuf_index != 0)
		//last_index = fDevIn->GetCurrentBuffer()->fGBuf_index;

		switch (fDevIn->GetCurrentBuffer()->fGBuf_type) {
		case EVENTDB_Idn:
			NbBufEVENTDB++;
			break;
			//   case EVENTDB_SWAP_Idn: NbBufEVENTDB++;  break;
		case EVENTCT_Idn:
			NbBufEVENTCT++;
			break;
			// case EVENTCT_SWAP_Idn: NbBufEVENTCT ++; break;
		case EBYEDAT_Idn:
			NbBufEBYEDAT++;
			break;
		case SCALER_Idn:
			NbBufSCALER++;
			break;
			// case SCALER_SWAP_Idn : NbBufSCALER++;   break;
		case ENDRUN_Idn:
			NbBufENDRUN++;
			break;
		case COMMENT_Idn:
			NbBufCOMMENT++;
			break;
		case FILEH_Idn:
			NbBufFILEH++;
			break;
		case PARAM_Idn:
			NbBufPARAM++;
			break;
		case EVENTH_Idn:
			NbBufEVENTH++;
			break;
		case JBUS_Idn:
			NbBufJBUS++;
			break;
			//case JBUS_SWAP_Idn :   NbBufSTATUS++;   break;
		case STATUS_Idn:
			NbBufSTATUS++;
			break;
			//case STATUS_SWAP_Idn : NbBufSTATUS++;   break;
		default:
			NbBufUNKNOWN++;
		}

		if ((GetEvent()->EventInitAlready())
				&& (fDevIn->GetCurrentBuffer()->IsEventBuffer())) {
			//para_max = GetEvent()->GetDataParameters()->GetNbParameters();
			while (true) {

				fStatus = GetEvent()->NextEvent(fDevIn->GetCurrentBuffer());
				if (fStatus != ACQ_OK) {
					break;
				}

				nbEvents++;
				nbEventsByBuf++;
				if (verb) {
					tempos.Form("%s\t%d\t%s\t%d", "--no buff :  ",
							fDevIn-> GetCurrentBuffer()->fGBuf_index,
							"  -- no  event : ", nbEvents);
					fError.TreatError(0, 0, tempos);
				}
				for (i = 0; i <= max_label2; i++) {
					LabelMultiTmp[i] = 0;
				}

				for (i = 0; i < GetEvent()->GetArrayLabelValueSize() / 2; i++) {
					labelc = (int) GetEventArrayLabelValue_Label(i);
					//value = (int) GetEventArrayLabelValue_Value(i);
					LabelMultiTmp[labelc]++;
					if ((LabelToExist[labelc]) <= 0) {
						nb_error_lab_ctrl++;
						LabelToExist[labelc]--;
					} else {
						nb_good_lab_ctrl++;
						LabelToExist[labelc]++;
					}
				}
				for (i = 0; i <= max_label2; i++) {
					if (LabelMultiTmp[i] > 1) {
						LabelMulti[i] += LabelMultiTmp[i] - 1;
						noevent = GetEvent()->GetEventNumber();
						tempos.Form(
								" Multi Label : %d (0x%x) for Event : %d of Buffer :%d Count %d ",
								i, i, noevent,
								fDevIn->GetCurrentBuffer()->fGBuf_index,
								LabelMulti[i]);
						fError.Infos(tempos);
						nb_multi_label += LabelMultiTmp[i] - 1;
					}
				}
			}//end of while
			if (nbEvents < 0)
				break; // ERROR
		}// end if ((GetEvent()
		nbEventsByBuf = 0;
	}

	gettimeofday(&mt_end, &tz);
	tempos.Form("%s%lld%s", "--------------------STATISTICS-done in ",
			(Long64_t)(mt_end.tv_sec - mt_begin.tv_sec),
			" seconds----------------------");
	fError.TreatError(0, 0, tempos);
	fError.TreatError(0, 0, "Nb of Buffers in function of there nature :\n");
	tempos.Form("%s\t%d\t%s\t%d", " FILEH     : ", NbBufFILEH,
			"      EVENTH   : ", NbBufEVENTH);
	fError.TreatError(0, 0, tempos);
	tempos.Form("%s\t%d\t%s\t%d", " COMMENT   : ", NbBufCOMMENT,
			"      PARAM    : ", NbBufPARAM);
	fError.TreatError(0, 0, tempos);
	tempos.Form("%s\t%d\t%s\t%d", " EBYEDAT   : ", NbBufEBYEDAT,
			"      EVENTDB  : ", NbBufEVENTDB);
	fError.TreatError(0, 0, tempos);
	tempos.Form("%s\t%d\t%s\t%d", " EVENTCT   : ", NbBufEVENTCT,
			"      SCALER   : ", NbBufSCALER);
	fError.TreatError(0, 0, tempos);
	tempos.Form("%s\t%d\t%s\t%d", " JBUS      : ", NbBufJBUS,
			"      STATUS   : ", NbBufSTATUS);
	fError.TreatError(0, 0, tempos);
	tempos.Form("%s\t%d\t%s\t%d", " ENDRUN    : ", NbBufENDRUN,
			"      UNKNOWN  : ", NbBufUNKNOWN);
	fError.TreatError(0, 0, tempos);
	tempos.Form("%s\t%d\n", " TOTAL     : ", NbBuf);
	fError.TreatError(0, 0, tempos);
	tempos.Form("%s\t%d\t%s\t%d", " First Buffer Index : ", first_index,
			"   Last : ", fDevIn->GetCurrentBuffer()->fGBuf_index);
	fError.TreatError(0, 0, tempos);

	if (GetEvent()->EventInitAlready()) {
		tempos.Form("%s\t%d\n", " NB  total events : ", nbEvents);
		fError.TreatError(0, 0, tempos);
		fError.TreatError(0, 0, "-----------Stat for each parameter----------");
		for (i = 0; i <= max_label; i++) {
			if (LabelToExist[i] >= 1) {
				tempos.Form("stat for : %s (label=%d(0x%x)) = %d ",
						GetEvent()->GetDataParameters()->GetParNameFromIndex(
								GetEvent()->GetDataParameters()->GetIndex(i)),
						i, i, LabelToExist[i] - 1);
				fError.TreatError(0, 0, tempos);
			}
		}
		fError.Infos("----------------Good-Label-------------------");
		tempos.Form(" NB good  Control labels : \t%d", nb_good_lab_ctrl);
		fError.TreatError(0, 0, tempos);
		fError.Infos("----------------Errors-of-no-know-labels----------------");
		tempos.Form(" NB error Control labels : \t%d", nb_error_lab_ctrl);
		fError.Infos(tempos);
		for (i = 0; i <= max_label; i++) {
			if (LabelToExist[i] < 0) {
				tempos.Form(
						"Error for label =%d (0x%x) . Nb of appearance  :%d",
						i, i, (-LabelToExist[i]));
				fError.Infos(tempos);

			}
		}
		if (test_multi) {
			fError.Infos("-----------------Multi-Label----------------");
			tempos.Form(" NB Multi-Label  : \t%d", nb_multi_label);
			fError.Infos(tempos);

			for (i = 0; i <= max_label; i++) {
				//ut<<"toto"<<LabelMulti[i]<<"\n";

				if (LabelMulti[i] > 0) {
					namepar
							= GetEvent()->GetDataParameters()->GetParNameFromIndex(
									GetEvent()->GetDataParameters()->GetIndex(i));
					tempos.Form(
							"Multi label = 0x%x (%d) . Nb of appearance  :%d : %s",
							i, i, LabelMulti[i], namepar.Data());
					fError.Infos(tempos);
				}
			}
		}
	}

	if (LabelMulti) {
		delete[] LabelMulti;
		LabelMulti = NULL;
	}
	if (LabelMultiTmp) {
		delete[] LabelMultiTmp;
		LabelMultiTmp = NULL;
	}
	if (LabelToExist) {
		delete[] LabelToExist;
		LabelToExist = NULL;
	}
	fError.Infos("-------------------------------------------");
}

//_________________________________________________________________________________

void GAcq::EventInit(char* _Name, char * type, bool creation,bool noactionfile) {

	// get Structure of event
	// ACTION files ( list of parameters and structure event files )
	// are created  by the Inquire() method
	// in case of a GTape device
	// if parameter "creation" is true(default), ACTION_Name.CHC_XXX ares created
	// from  parameter buffers of current run
	// family : name of family in where associated histogram will be found
	// by default _Name is set to "local"
	// type is envent type =Ganil, MFM , by defautl = Ganil,
    // noactionfile ( default =false) we init event whithout action_file.
	fStatus = ACQ_OK;
	TString ExpName;
	TString info ;
       
	TString ExpNameFromHost = gSystem->Getenv("ACQ_VME_EXPNAME");
	fDataParameters = new DataParameters();

	if (_Name == NULL) {
		fError.TreatError(2, fStatus,
				"Event  initialization not done, no experiment name\n");
		return;
	}

	if (CompareWordsIgnoreCase(type, "ganil")) {
		SetEvent(((GEventBase*) (new GEvent(GetDataParameters()))));
		
	}
	if (CompareWordsIgnoreCase(type, "mfm")) {
		SetEvent(((GEventBase*) (new GEventMFM(GetDataParameters()))));
	}

    if (noactionfile ){
      GetEvent()->SetEventInitAlready(true);
    return;
    }
	if (!fDevIn) {
		fError.TreatError(2, fStatus,
				"Event  initialization not done, Input device null\n");
		return;
	}

	if (((fDevIn->GetType() == GT_TYPE_TAPE) || (fDevIn->GetType()
			== GT_TYPE_FILE)) && (creation)) {
		ExpName = _Name;
		fDevIn->Rewind();
		fDevIn->Inquire(_Name);
		fDevIn->Rewind();
	} else {
		if ((ExpNameFromHost.CompareTo("") == 0) && (strcmp(_Name, "local")
				== 0)) {
			ExpName = ExpNameFromHost;
			ExpName = "local";
		} else {
			ExpName = _Name;
		}
	}

	if (fStatus == ACQ_OK){
		info.Form ("Event initialization type : %s",type);
		fError.TreatError( 0,0,info);
		GetEvent()->EventInit((char*) (ExpName.Data()));
		

	}
	else
		fError.TreatError(2, fStatus, "Event  initialization not done\n");
	return;
}

//_________________________________________________________________________________

bool GAcq::GetRunStatus(bool verb) { // Verify if all acquisition parameters are OK
// example in a client ganil acquisition  store process, Ganil Acqusition  on
// on  a tape is could be only the verification of a open file.
// the method have to bee virtual in future
// return true or false
// if input verb = true , then  informations are displayed

#ifdef NET_LIB
	if (fDevIn->GetType() == GA_NET_CLIENT) {
		fError.TreatError(1,0," GNetClientGanil is obsolete");
		return ((fAcquisitionOn) && (fStoreOn) );
	} else
#endif
	if ((fDevIn->GetType() == GT_TYPE_TAPE) || (fDevIn->GetType()
			== GT_TYPE_FILE)) {
		GTape* filelocal = (GTape*) fDevIn;
		fRunNumber = filelocal->GetRunNumber();
	}
	return (true);
}
//_________________________________________________________________________________

//*****************AJOUT JOHN************************************//
Bool_t GAcq::GetNextEvent() {

	//Use this method to read sequentially events from a device.
	//You need to call EventInit() before beginning to read,
	//after that GetDeviceIn()->GetRunNumber() gives run number.
	//We assume that the ACTIONS_xxxx.xxxx files are present.
	//
	//Common use (with a run file):
	//GTape dlt("run6030.raw");
	//dlt.Open();
	//GAcq a(&dlt);
	//a.EventInit();
	//cout << "Run number = " << dlt.GetRunNumber() << endl;
	//cout << "Run number = " << a.GetDeviceIn()->GetRunNumber() << endl;
	//a.GetNextEvent();
	//a.GetEvent()->DumpEvent();
	//a.GetNextEvent();
	//etc. etc.
	//
	//Returns kTRUE if event correctly read
	//Returns kFALSE if not (e.g. end of file reached)

	//si le buffer courant n'est pas un buffer a evenements, on en lit
	//jusqu'a ce qu'on en trouve un

	while ((!GetDeviceIn()->GetCurrentBuffer()->IsEventBuffer())
			&& (GetDeviceIn()->GetStatus() == ACQ_OK)) {
		GetDeviceIn()->ReadBuffer();
		GetEvent()->RazEvent();
	}
	//si c'est toujours pas bon, on sort
	if ((!GetDeviceIn()->GetCurrentBuffer()->IsEventBuffer())
			|| (GetDeviceIn()->GetStatus() != ACQ_OK))
		return kFALSE;

	// read next event in buffer
	fStatus = GetEvent()->NextEvent(GetDeviceIn()->GetCurrentBuffer());
	if (fStatus != ACQ_OK)
		return kFALSE;
	return kTRUE;
}
//***********************FIN AJOUT JOHN***************************************//

//_________________________________________________________________________________
Float_t GAcq::GetSucces() {
	return fSucces;
}
//_________________________________________________________________________________
Int_t GAcq::GetNumEvent() {
	return fEventCount;
}
//_________________________________________________________________________________
Float_t GAcq::GetRateEvent() {
	return fRateEvent;
}
//_________________________________________________________________________________
Float_t GAcq::GetRateBuffer() {
	return fRateBuffer;
}
//_________________________________________________________________________________
 void  GAcq::ToDoInCaseOfInterrupt(){
	 SetStop(true);
	 EndUser();
	 SpeSave("histoCtrlC.root");
};
//_________________________________________________________________________________
////////////////////////////////////////fin/////////////////////////////////////
