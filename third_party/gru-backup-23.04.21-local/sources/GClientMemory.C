// Author: $Author: legeard $
// Part of GRU
//
// GClientMemory : Client of Memory It can ask for a spectrum Data base and
// histogram.
//
//
//
//////////////////////////////////////////////////////////////////////////

// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************

#include <sys/time.h>
#include "GClientMemory.h"
#include "TCanvas.h"
#include "TProfile.h"

ClassImp( GClientMemory);
//_________________________________________________________________________________________
GClientMemory::GClientMemory(char* host) {
	fType = GA_MEMORY;
	fVerbose = 0;
	SetDevice(host);
	fTempo = 60;
	pSpectraDBServer = NULL;
}
//_________________________________________________________________________________________
GClientMemory::~GClientMemory() {
	Close();
}

//_________________________________________________________________________________________
void GClientMemory::InitClient(GSpectraDB *pspectra) {
	pSpectraDBServer = pspectra;

}

//_________________________________________________________________________________________
TString *GClientMemory::GetListSpectra() {
	//return in a TString the liste of histogram

	TString *liste = new TString();
	*liste = pSpectraDBServer-> GetSpectraList();
	return (liste);

}

//_________________________________________________________________________________________
TNamed* GClientMemory::GetSpectrum(const char *spectrumname, TNamed *old_sp) {
	//return  the histogram with "histoname"
	// old_histo: in case of refresh we use the odl histogram
	TNamed* sp = pSpectraDBServer->GetSpectrumByName(spectrumname);
	return (sp);

}
//_________________________________________________________________________________________
TNamed* GClientMemory::GetSpectrum(const char *spectrumname,
		const char *family, TNamed *old_sp) {
	//return  the histogram with "histoname"
	// old_histo: in case of refresh we use the odl histogram
	TNamed *sp;

	sp = pSpectraDBServer->GetSpectrumByName(spectrumname, family);

	return (sp);
}

//_________________________________________________________________________________________
void GClientMemory::GetSpectrum(GSpectrumIdentity* id) {
	//Get spectrum for local DBServe   and  put it in indentity

	TNamed * sp = NULL;
	TNamed* clonesp = NULL;
	TString spectrumname = id->GetSpectrumName();

	TString family = id->GetFamily();

	sp = pSpectraDBServer->GetSpectrumByName(spectrumname.Data(),family.Data());

	if ((sp)) {
		clonesp = (TNamed *) (sp->Clone());
		if (clonesp == NULL) {
			fError.TreatError(2, 0,
					"GClientMemory::GetSpectrum : Spectrum not cloned ");
		}
		id->DeleteSpectrumInstance();
		id->SetSpectrum(clonesp);
	}

	return;
}
//_________________________________________________________________________________________

GSpectraDB* GClientMemory::GetSpectraDB() {
	//return  a Data base of spectra name

	return (pSpectraDBServer);
}
//_________________________________________________________________________________________
void GClientMemory::SpeSave(char* filename) {
	//to save histograms of root server

	fError.TreatError(2, fStatus, " Not yet implemented");
}
//_________________________________________________________________________________________
void GClientMemory::Open(char* mod) {
	//set and open network root client
	Open();
}

//_________________________________________________________________________________________
void GClientMemory::Open(char mod) {
	//set and open network root client
	fIsOpen = true;
}
//______________________________________________________________________________

void GClientMemory::Close() {
	MyClose();
}
//______________________________________________________________________________

void GClientMemory::Close(Int_t tempo) {
	MyClose(tempo);
}
//_________________________________________________________________________________________
void GClientMemory::MyClose(Int_t tempo) {
	//close  client
	fIsOpen = false;
}
//_________________________________________________________________________________________
bool GClientMemory::TestServer() {
	// test server

	return (true);
}
//_________________________________________________________________________________________
void GClientMemory::ResetSpectrumOnServer(GSpectrumIdentity* id) {
	id->Reset();
}
//_________________________________________________________________________________________

//_________________________________________________________________________________________
