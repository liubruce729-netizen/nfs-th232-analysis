// File : GGeneBuffer.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GGeneBuffer
//
// This class is a  device generating Filled  buffer
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

#include <sys/time.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mtio.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <TObject.h>
#include <TH1.h>

#include "General.h"
#include "GTape.h"

#include "DataParameters.h"
#include "GGeneBuffer.h"
#include "GEvent.h"
#include "GEventMFM.h"
extern "C" {
#include "GEN_TYPE.H"
#include "STR_EVT.H"
#include "ERR_GAN.H"

#include "acq_codes_erreur.h"
}

#define DEFAULT_ACTIONFILE "./ACTION_local.CHC_PAR"

ClassImp( GGeneBuffer);
//______________________________________________________________________________
GGeneBuffer::GGeneBuffer() {
	// Default constructor of object;
	GGeneInit("./");
}
//______________________________________________________________________________
GGeneBuffer::GGeneBuffer(const char* _name, int type) {
	//  constructor of object;
	GGeneInit(_name, type);
}
//______________________________________________________________________________
void GGeneBuffer::GGeneInit(const char* _name, int type,int insidetype) {
	fBufferType = type;
	fBufferInsideType = insidetype;
	fEvent = NULL;
	fParameter = NULL;
	fSubTabsOfIndex = NULL;
	fType = GA_GENERATOR;
	fVerbose = 0;
	fEventCounter = 0;
	fSizeOfSubTab = 0;
	fBufferCounter = 0;
	fNbOfSubEvents = 5;
	fBufferSize = 8;
	fInfiniteRead = false;
	fLaw = 0;
	fProba = 0.6;
	TString tempos;
	SetDevice(_name);
	tempos.Form("New device  Name : %s fType  Generator", fName);
	fError.Infos(tempos);
	fTimeStamp = new TTimeStamp(0, 0);
	if ((fBufferType >= IN2P3_MIN_Idn)and(fBufferType <= IN2P3_MAX_Idn)) {
		fBufferSize = BUFSIZE;
		fBuffer = new GBufferIn2p3(fBufferSize);
	}
	if ((fBufferType >= MFM_MIN_TYPE) and (fBufferType <= MFM_MAX_TYPE)) {
		fBuffer = new GBufferMFM(fBufferSize);
	}
}

//______________________________________________________________________________

GGeneBuffer::~GGeneBuffer(void) {
	// Default destructor of object;
	fError.Infos("Destructor of GGeneBuffer");
	if (fEvent) {
		delete fEvent;
		fEvent = NULL;
	}
	if (fBuffer) {
		delete fBuffer;
		fBuffer = NULL;
	}
	if (fParameter) {
		delete fParameter;
		fParameter = NULL;
	}
}

//______________________________________________________________________________
void GGeneBuffer::WriteBuffer(GBuffer* _Buffer) {
	cout << "GGeneBuffer::WriteBuffer has no action \n";
}

//______________________________________________________________________________
void GGeneBuffer::SetNbOfSubEvents(int myint) {
	//Ajouter gestion des erreurs
	fNbOfSubEvents = myint;
}
//______________________________________________________________________________
void GGeneBuffer::Open(char* mod) {
	Open(mod[0]);
}
//______________________________________________________________________________
void GGeneBuffer::Open(char mod) {
	fBufferCounter = 0;
	fParameter = new DataParameters();
	fParameter->FillFromActionFile(fName);
	int nbOfLabels = fParameter->GetNbParameters();

	if (fBufferType == EBYEDAT_Idn) {
		fError.TreatDebug(0, 0, "GGeneBuffer::Open in generator mode  EBYEDAT");
		if (fNbOfSubEvents >= nbOfLabels/2) fNbOfSubEvents = nbOfLabels/2;
		if (fNbOfSubEvents ==0) fNbOfSubEvents =1;
		fSizeOfSubTab = (int) ((nbOfLabels / fNbOfSubEvents) + 1);
		fError.TreatDebug(0, 0, "GGeneBuffer::Open in generator mode2 ");
		//Repartition of labels in the sub-event groups (simple version)
		//		int listOfIndex[nbOfLabels];
		//		for (int i = 0; i < nbOfLabels; i++)
		//			listOfIndex[i] = i;

		//Initialisation of the two dimensions tab fSubTabsOfIndex
		fSubTabsOfIndex = new int*[fNbOfSubEvents];
		for (int i = 0; i < fNbOfSubEvents; i++) {
			fSubTabsOfIndex[i] = new int[fSizeOfSubTab];
		}
		//Filling of fSubTabsOfIndex
		int i, j;
		for (i = 0; i < fNbOfSubEvents; i++) {
			for (j = 0; j < fSizeOfSubTab; j++) {
				if (((i * fSizeOfSubTab) + j) < nbOfLabels)
					fSubTabsOfIndex[i][j] = (i * fSizeOfSubTab) + j;
				else
					fSubTabsOfIndex[i][j] = -1;

				if (fVerbose == 10)
					cout << fSubTabsOfIndex[i][j] << endl;
			}
			if (fVerbose == 10) {
				cout << endl;
				cout << "end of SubTab" << endl;
				cout << endl;
			}
		}
		fEvent = new GEvent(fParameter);
		fIsOpen = true;
	}

	if ((fBufferType >= MFM_MIN_TYPE)&&(fBufferType <= MFM_MAX_TYPE)){

		if (fEvent!= NULL)  fError.TreatDebug(4, 0,
				"Pointer on Event is not null");
		fEvent = new GEventMFM(fParameter);


		if (fEvent== NULL) fError.TreatDebug(4, 0,
			"Pointer on Event is null");

		fEvent -> fError.TreatDebug(0, 0,
			"GGeneBuffer::Open in generator mode MFM");

		((GEventMFM*) fEvent)->MakeEventHeader(fBufferType,nbOfLabels);
		fIsOpen = true;

	}

	SetRandomLaw(fLaw);
	SetRandomProba(fProba);
	if (!fIsOpen)
		fError.TreatError(1, 0, "GGeneBuffer not open");

}

//______________________________________________________________________________
void GGeneBuffer::SetRandomLaw(int law) {
	fLaw = law;
	if (fEvent) {
		fEvent->SetRandomLaw(fLaw);
		TString tempos;
		switch (fLaw) {
		case 1:
			tempos.Form("Random Law white noise");
			break;
		case 2:
			tempos.Form("Random Law is binomial");
			break;
		default:
			tempos.Form("Random Law is gaussian");
			break;
		}
		fError.Infos(tempos.Data());
	} else {
		fError.TreatError(1, 0,
				"SetRandom Law has no effect because Event doesn't exist yet");
	}
}
//______________________________________________________________________________
void GGeneBuffer::SetRandomProba(Float_t proba) {
	fProba = proba;
	if (fEvent) {
		fEvent->SetRandomProba(fProba);
	} else {
		fError.TreatError(1, 0,
				"SetRandom Proba has no effect because Event doesn't exist yet");
	}
}
//______________________________________________________________________________
void GGeneBuffer::Close() {
	if (fEvent) {
		delete fEvent;
		fEvent = NULL;
	}
	if (fParameter) {
		delete fParameter;
		fParameter = NULL;
	}
	fIsOpen = false;
}

//______________________________________________________________________________

void GGeneBuffer::ReadBuffer() {
	TString tempos;
	if (fBufferType == EBYEDAT_Idn){
		ReadBufferEBYEDAT();
		return;
	}
	if (fBufferType == MFM_COBO_FRAME_TYPE){
		ReadBufferMFMCOBO();
		return;
	}

	if (fBufferType == MFM_EXO2_FRAME_TYPE){
		ReadBufferMFMEXOGAM2();
		return;
	}

	if ((fBufferType == MFM_EBY_EN_FRAME_TYPE)or(fBufferType == MFM_EBY_TS_FRAME_TYPE)or(fBufferType == MFM_EBY_EN_TS_FRAME_TYPE)) {
		ReadBufferMFMEEBYEDAT(fBufferType);
		return;
	}
	if (fBufferType == MFM_OSCI_FRAME_TYPE){
		ReadBufferMFMOSCI();
		return;
	}
	if ((fBufferType ==MFM_VAMOSIC_FRAME_TYPE) or (fBufferType ==MFM_VAMOSPD_FRAME_TYPE)){
		ReadBufferMFM(fBufferType);
		return;
	}
	tempos.Form("No GGeneBuffer::ReadBuffer avalaible for %d type ",fBufferType);
	fError.TreatError(4,0,tempos);
	return;
}

//______________________________________________________________________________
void GGeneBuffer::ReadBufferEBYEDAT() {
	// fill buffer with random event
	if (!fIsOpen) {
		fError.TreatError(3, 0, "Device GGeneBuffer not open");
		return;
	}
	fBufferCounter++;
	UInt_t numOfFirstEventInBuffer = 0;
	numOfFirstEventInBuffer = fEventCounter;

	fBuffer->RazBuffer();

	GBufferIn2p3 * localbuf = (GBufferIn2p3 *) fBuffer;
	GEvent * localevent = (GEvent *) fEvent;

	while ((int) (GetBufferSize()) - (int) (localbuf->GetUsedEventsSize()
			+ GANIL_BUF_HD_SIZE) > (int) (localevent->GetUsedSize()) + 4)
	//test if free memory is greater than event size plus 4 bytes for the End-data block
	{

		if (localevent->IsRaz()) {
			localevent->FillEvent(0, 1, 2, 2, 0, 0, fEventCounter++,
					fSubTabsOfIndex, fNbOfSubEvents);
		}
		// use of MakeEvent
		//MakeEvent(Char_t systemId, Char_t statusWords, Char_t numberWords,
		//Char_t clockWords, Char_t format, Long64_t status, Long64_t number,
		//Int_t** subTabsOfIndex, Int_t nbOfSubEvents)

		localevent->FillBufferWithEvent(fBuffer);// copy Event in Buffer
		localevent->RazEvent();
		localevent->FillEvent(0, 1, 2, 2, 0, 0, fEventCounter++,
				fSubTabsOfIndex, fNbOfSubEvents);
	     cout << "debug GGeneBuffer::ReadBufferEBYEDAT() No event "<<fEventCounter<<" In2p3BufferSize "<< GetBufferSize()<< " fill in In2p3Buffer "<<localbuf->GetUsedEventsSize()<< " event size " << localevent->GetUsedSize()<<flush<<"\n";

	}
 cout << "debug GGeneBuffer::ReadBufferEBYEDAT() "<<fBufferCounter<<flush<<"\n";
	((GBufferIn2p3*) fBuffer)->MakeEBYEDATHeader(fBufferCounter, 1523, 1541,
			1234, (fEventCounter - numOfFirstEventInBuffer));

}

//______________________________________________________________________________
void GGeneBuffer::ReadBufferMFMCOBO() {
	// fill buffer with random event
	if (!fIsOpen) {
		fError.TreatError(3, 0, "Device GGeneBuffer not open");
		return;
	}
	fBufferCounter++;

	fBuffer->RazBuffer();
	long long timestamp = 0;
	int sec = 0;
	GEventMFM * localevent = (GEventMFM *) fEvent;

	//if (localevent->IsRaz()) {
	fTimeStamp->Set();
	sec = (Int_t) (fTimeStamp->GetSec());
	timestamp = fTimeStamp->GetNanoSec();
	timestamp += sec * 1000000000;
	localevent->FillEvent(MFM_COBO_FRAME_TYPE, timestamp, fBufferCounter);
	localevent->FillBufferWithEvent(fBuffer);// copy Event in Buffer

	((GBufferMFM*) fBuffer)->SetAttributs();
	((GBufferMFM*) fBuffer)->SetType(MFM_COBO_FRAME_TYPE);
	fBuffer->fIsAEventType = true;
	localevent->RazEvent();

	//}
}

//______________________________________________________________________________
void GGeneBuffer::ReadBufferMFMEXOGAM2() {
	// fill buffer with random event
	if (!fIsOpen) {
		fError.TreatError(3, 0, "Device GGeneBuffer not open");
		return;
	}
	fBufferCounter++;


	fBuffer->RazBuffer();
	long long timestamp = 0;
	int sec = 0;
	GEventMFM * localevent = (GEventMFM *) fEvent;

	//if (localevent->IsRaz()) {
	fTimeStamp->Set();
	sec = (Int_t) (fTimeStamp->GetSec());
	timestamp = (long long)fTimeStamp->GetNanoSec();
	timestamp += sec * 1000000000;
	localevent->FillEvent(MFM_EXO2_FRAME_TYPE, timestamp, fBufferCounter);
	localevent->FillBufferWithEvent(fBuffer);// copy Event in Buffer

	((GBufferMFM*) fBuffer)->SetAttributs();
	((GBufferMFM*) fBuffer)->SetType(MFM_EXO2_FRAME_TYPE);
	fBuffer->fIsAEventType = true;
	localevent->RazEvent();
	//}
}
//______________________________________________________________________________
void GGeneBuffer::ReadBufferMFMOSCI() {
	// fill buffer with random event
	if (!fIsOpen) {
		fError.TreatError(3, 0, "Device GGeneBuffer not open");
		return;
	}
	fBufferCounter++;


	fBuffer->RazBuffer();
	long long timestamp = 0;
	GEventMFM * localevent = (GEventMFM *) fEvent;
	localevent->FillEvent(MFM_OSCI_FRAME_TYPE, timestamp, (fBufferCounter%4));
	localevent->FillBufferWithEvent(fBuffer);// copy Event in Buffer

	((GBufferMFM*) fBuffer)->SetAttributs();
	((GBufferMFM*) fBuffer)->SetType(MFM_OSCI_FRAME_TYPE);
	fBuffer->fIsAEventType = true;
	localevent->RazEvent();
}

//______________________________________________________________________________
void GGeneBuffer::ReadBufferMFMEEBYEDAT(uint32_t type) {
	// fill buffer with random event
	if (!fIsOpen) {
		fError.TreatError(3, 0, "Device GGeneBuffer not open");
		return;
	}
	fBufferCounter++;


	fBuffer->RazBuffer();
	long long timestamp = 0;
	int sec = 0;
	GEventMFM * localevent = (GEventMFM *) fEvent;

	//if (localevent->IsRaz()) {
	fTimeStamp->Set();
	sec = (Int_t) (fTimeStamp->GetSec());
	timestamp = (long long)(fTimeStamp->GetNanoSec());
	timestamp += sec * 1000000000;
	localevent->FillEvent(type, timestamp, fBufferCounter);
	localevent->FillBufferWithEvent(fBuffer);// copy Event in Buffer
	((GBufferMFM*) fBuffer)->SetAttributs();
	((GBufferMFM*) fBuffer)->SetType(type);
	fBuffer->fIsAEventType = true;
	localevent->RazEvent();

}

//______________________________________________________________________________
void GGeneBuffer::ReadBufferMFM(uint32_t type){
	if (!fIsOpen) {
		fError.TreatError(3, 0, "Device GGeneBuffer not open");
		return;
	}
	fBufferCounter++;


	fBuffer->RazBuffer();
	long long timestamp = 0;
	int sec = 0;
	GEventMFM * localevent = (GEventMFM *) fEvent;

	fTimeStamp->Set();
	sec = (Int_t) (fTimeStamp->GetSec());
	timestamp = (long long)(fTimeStamp->GetNanoSec());
	timestamp += sec * 1000000000;
	localevent->FillEvent(type, timestamp, fBufferCounter);
	localevent->FillBufferWithEvent(fBuffer);// copy Event in Buffer
	((GBufferMFM*) fBuffer)->SetAttributs();
	((GBufferMFM*) fBuffer)->SetType(type);
	fBuffer->fIsAEventType = true;
	localevent->RazEvent();
}

//______________________________________________________________________________
//______________________________________________________________________________
