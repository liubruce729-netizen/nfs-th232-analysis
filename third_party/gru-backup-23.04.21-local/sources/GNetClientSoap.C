// Author: $Author: legeard $
// Part of GRU
//
// GNetClientSoap : Client of a GNetServerSoap.
//  Liste of available commands:
//  GRU TEST   : test if server is running well
//  GRU GET SPECTRA LIST : give liste names of histogram with family names
//  GRU INFO   : give liste of informations (name_info1 info1 name_info2 info2....)
//  GRU RESET_SPECTRUM spectrumname familyname : ask of server to reset the spectra with name "spectrumname" and family "familyname"
//  GRU SPECTRUM spectrumname familyname : ask a spectrum from server with name "spectrumname" and "familyname"
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
#include "GNetClientSoap.h"
#include "TCanvas.h"
#include "TProfile.h"
#include "TString.h"
#ifndef NO_GSOAP
#include "GruInterfaceSoap.h"
#include "SCInterfaceSoap.h"
#endif
ClassImp( GNetClientSoap);
//_________________________________________________________________________________________
GNetClientSoap::GNetClientSoap(char* host) {
	fType = GA_NET_SOAP;
	fVerbose = 0;
	SetDevice(host);
	fPortDef = 6603; //default  Port
	SetPort(fPortDef); //Root Port
	fTempo = 60;
	InitClient();

}

//_________________________________________________________________________________________
GNetClientSoap::~GNetClientSoap() {
	Close();
	if (fZoneData_char) {
		delete[] fZoneData_char;
		fZoneData_char = NULL;
	}
}

//_________________________________________________________________________________________
void GNetClientSoap::InitClient() {
	fSizeZoneData = 0;
	fZoneData_char = NULL;
	fComptNotAvailable = 0;
	fBufferMFM = new GBufferMFM();
	fBuffer = fBufferMFM;
	fPremier_Buffer = 0;
	fComptbuffOK = 0;
	fSCBufferEndReached = true;
	fZoneData_cur_char = NULL;
	fZoneData_cur_int = NULL;
	fTimestampsbuffersize = 0;
	fSize_SC_buffer = 0;
	fNb_Buffers_Local = 0;
	fPremier_Buffer = 0;
	fNb_Buffers_Recus = 0;
	fNBuffers_Envoyes = 0;
}

//_________________________________________________________________________________
/*bool GNetClientSoap::EndOfPage() {
	// no EndOfPage use in case of soap protocol.
	fError.TreatError(2, fStatus, "no EndOfPage use in case of soap protocol");
	return false;
}*/

//_________________________________________________________________________________
void GNetClientSoap::GetSpectrum(GSpectrumIdentity* id) {
	//Get Histogram

	Bool_t SameServer;
	TNamed* sp;
	TNamed* old_sp = id->GetSpectrum();
	TString spectrumname = id->GetSpectrumName();
	TString family = id->GetFamily();
	TString tempo;
	TString SourceName = id->GetSourceName();
	TString SourceType = id->GetSourceType();
	TString Source = id->GetSource();//net or file
	Int_t port = id->GetPort();
	SameServer = true;

	if (Source.CompareTo("FILE") == 0) {
		fError.TreatError(
				1,
				0,
				"GNetCLientSoap : Get Spectrum, not implemented for FILE type in GNetClientSoap");
		return;
	}
	if (SourceType != "SOAP") {
		fError.TreatError(
				1,
				0,
				"GNetCLientSoap : Get Spectrum, not implemented for no SOAP type in GNetClientSoap");
		return;
	}

	if (strcmp(spectrumname.Data(), "") == 0) {
		tempo.Form("GNetCLientSoap : No name for spectrum");
		fError.TreatError(0, 0, tempo);
		return;
	}
	if ((SourceName == fName) && (GetPort() == port))
		SameServer = true;
	else
		SameServer = false;
	SetPort(port);
	if (fIsOpen) {
		if (!SameServer) {
			Close();
			SetPort(port);
			SetDevice(SourceName.Data());
		}
	}

	if (!fIsOpen) {
		Open();
		if (!fIsOpen) {
			fError.TreatError(2, 0, "GNetCLientSoap : Any connected server !");
			fStatus = ACQ_DISCONNECT;
			return;
		}
	}

	sp = GetSpectrum(spectrumname.Data(), family.Data(), old_sp);

	if ((sp) && (sp != old_sp)) {
		id->DeleteSpectrumInstance();
		id->SetSpectrum(sp);
	}

}

//_________________________________________________________________________________
TNamed* GNetClientSoap::GetSpectrum(const char *spectrumname, TNamed* old_spectrum) {

	TNamed* th;
	TNamed* old_histo = (TNamed*) old_spectrum;
	th = GetSpectrum(spectrumname, 0, old_histo);
	TNamed* sp = (TNamed*) th;
	return sp;
}

//_________________________________________________________________________________________
TNamed* GNetClientSoap::GetSpectrum(const char *spectrumname,
		const char *familyname, TNamed *old_sp) {
	//return  the histogram with "histoname"
	// old_histo: in case of refresh we use the odl histogram
	TNamed *histo = NULL;
	TString tempos;
#ifndef NO_GSOAP	
	int i;

	Int_t port = GetPort();
	TString httpmessage;
	httpmessage.Form("http://%s:%d", GetDeviceName(), port);
	Int_t entry;
	int value;
	char command[255];
	int sizeofhisto;
    int error;

    GruInterfaceSoap InterfaceSoap;
    sprintf(command, "GRU GET SPECTRUM %s %s ", spectrumname, familyname);

    uint32_t *myvectinpt =NULL;
    uint32_t myvectinsize=0;
    

	error = InterfaceSoap.AwordsRvectorInt((char*)(httpmessage.Data()) , (char*)command, &myvectinpt,&myvectinsize );

	if (error != 0) {
		//vecteuru = NULL;
		sizeofhisto = 0;
		//soap_print_fault(&v_soap, stderr);
		fError.TreatError(1, 0, "GNetClientSoap::GetSpectrum no returned spectrum ");
		return old_sp;
	}

	//sizeofhisto = vector.__size / sizeofelement;
	sizeofhisto = myvectinsize;

	tempos.Form(
			"GNetCLientSoap : Get Spectrumname = %s Content of vector in ushort: sizeofhisto = %d  size = %d \n",
			spectrumname, sizeofhisto, myvectinsize);

	fError.Infos(tempos);

	if (old_sp) {
		Int_t nbinx;
		nbinx = ((TH1*) old_sp)->GetNbinsX();
		if (nbinx != sizeofhisto) {
			delete (old_sp);
			old_sp = NULL;
		}
	}

	if (old_sp) {
		histo = old_sp;
	} else {

		histo = (TH1*) (new TH1F(spectrumname, spectrumname, sizeofhisto, 0,
				sizeofhisto));
	}
	entry = 0;
	((TH1*) histo)->Reset();
	for (i = 0; i < sizeofhisto; i++) {

		value =myvectinpt[i];

		entry += value;
		((TH1*) histo)->Fill(i, value);
	}
	((TH1*) histo)->SetEntries(entry);

   if(myvectinpt)  delete[] myvectinpt;
#else
	tempos.Form(
			"GNetCLientSoap : Compile with NO_GSOAP , so GNetClientSoap:GetSpectrum return NULL");

	fError.Infos(tempos);
#endif

	return (histo);
}

//_________________________________________________________________________________________
void GNetClientSoap::SpeSave(char* filename) {
	//to save histograms of root server

	fError.TreatError(2, fStatus,
			"GNetCLientSoap::SpeSave : Not yet implemented");
}
//_________________________________________________________________________________________
void GNetClientSoap::Open(char* mod) {
	//set and open network root client
	Open();
}
//_________________________________________________________________________________________
Int_t GNetClientSoap::TestPortFree(Int_t port, char* name) {
	//test a port by a open/close
	// return port if port is free
	// return O if port is not free
	// return -1 if pb

	TString tempo;
	TSocket *sock;
	Int_t ret;
	ret = port;
	if (fVerbose) {
		tempo.Form("GNetCLientSoap : TestPort to : %s on port : %d ", name,
				port);
		fError.TreatError(0, 0, tempo);
	}

	if (strcmp(name, "") == 0) {
		fError.TreatError(2, fStatus,
				"GNetClientSoap : device name not valid in TestPort:", name);
		ret = -1;
		return ret;
	}
	sock = NULL;
	sock = new TSocket(name, port);

	if (sock !=NULL) {
		if (sock->IsValid()) {
			ret = 0;
			if (fVerbose) {
				tempo.Form(
						"GNetCLientSoap : TestPort to : %s on port : %d done so no free",
						name, port);
				fError.TreatError(0, 0, tempo);
			}
			sock->Close();
		}
	}
	return ret;
}
//_________________________________________________________________________________________
void GNetClientSoap::Open(char mod) {
	//set and open   client
	fIsOpen = true;
}
//______________________________________________________________________________

void GNetClientSoap::Close() {
	// close
	fIsOpen = false;
}
//_________________________________________________________________________________________
bool GNetClientSoap::SendCommand0(char* command) {
	// send simple command with a bool in return
	bool retour = true ;
#ifndef NO_GSOAP	
	Int_t port = GetPort();
	char httpmessage[256];
	
	GruInterfaceSoap InterfaceSoap;
	sprintf(httpmessage, "http://%s:%d", GetDeviceName(), port);
	int error = InterfaceSoap.Awords  ( httpmessage, command  );
	if( error == 1) retour= false;
	else retour = true;
#else	
	TString tempos;
	retour = true ;
	tempos.Form("GNetCLientSoap : Compile with NO_GSOAP , so GNetClientSoap:SendCommand0 inoperative ");
	fError.Infos(tempos);
#endif

	return (retour);
}

//_________________________________________________________________________________
TString* GNetClientSoap::SendCommand1(char* command) {
	// send a command with a "Get" and wait a string in return


	TString* listwords;
	listwords = new TString("");
	
#ifndef NO_GSOAP
	string listword2s;
	GruInterfaceSoap InterfaceSoap;

	int error = 0;
	Int_t port = GetPort();
	char httpmessage[256];
	sprintf(httpmessage, "http://%s:%d", GetDeviceName(), port);

	error = InterfaceSoap.AwordsRwords(httpmessage,(char*) command,&listword2s);
	listwords->Form("%s", listword2s.data());
	//cout <<"Final "<<listwords->Data()<<"\n";
	if (error ==0)
		fError.TreatError(0, 0, "GNetCLientSoap : SendCommand OK :", command);
	else
		fError.TreatError(0, 0, "GNetCLientSoap : SendCommand Not OK :",
				command);
#else
	TString tempos;
	tempos.Form("GNetCLientSoap : Compile with NO_GSOAP , so GNetClientSoap:SendCommand1 inoperative" );
	fError.Infos(tempos);
#endif

	return (listwords);
}

//_________________________________________________________________________________________
bool GNetClientSoap::StartSpectraServer(int port) {
	char command[MAX_CARACTERES];
	sprintf(command, "GRU SPECTRASERVER START %d", port);
	bool retour = SendCommand(command);
	if (retour)
		fError.TreatError(0, 0, "GNetClientSoap : SPECTRASERVER-START OK");
	else
		fError.TreatError(1, 0, "GNetClientSoap : SPECTRASERVER-START Not OK");
	return (retour);
}
//_________________________________________________________________________________________
GSpectraDB* GNetClientSoap::GetSpectraDB() {

	TString *liste;
	GSpectraDB* db = new GSpectraDB();

	liste = SendCommand1((char*) "GRU GET SPECTRA LIST");
	int nb_words = 0;
	char **listsliced = NULL;
	int * bindon = NULL;
	const char *tempo = NULL;
	tempo = liste->Data();
	int test = 0;
	nb_words = Slicer((char*) tempo, &listsliced, &bindon);

	TString n;
	TString sName;
	TString sType = "SOAP";
	TString sSource = "NET";
	TString fam;
	// 			               TString source, Int_t port, TString fam ,
	for (int i = 0; i < (nb_words / 2); i++) {
		n = listsliced[i * 2];
		sName = GetDeviceName();
		fam = listsliced[i * 2 + 1];
//cout << " debug "<<n.Data()<<" "<< sName.Data()<<"'\n";
		test = db->AddIdentity(NULL, n, sName, sType, sSource, GetPort(),
				listsliced[i * 2 + 1]);

		if (test < 0)
			fError.TreatError(1, 0,
					"GNetClientSoap::GetSpectraDB() no spectrum include in DB");
	}

	if (listsliced) {
		for (int i = 0; i < nb_words; i++) {
			delete[] listsliced[i];
			listsliced[i] = NULL;
		}
		delete[] listsliced;
		listsliced = NULL;
	}
	if (bindon) {
		delete[] bindon;
		bindon = NULL;
	}
	delete listsliced;


	return db;
}

//_________________________________________________________________________________________

bool GNetClientSoap::GetBufferSC(bool afficher) {
	// Get Sc buffer
	// return true if data ok
	TString tempos;
	fStatus = ACQ_OK;
#ifndef NO_GSOAP
    int error =0;
    int  sizevector=0;

	Int_t port = GetPort();

	GBase::RazZone(fZoneData_char,fSizeZoneData);
    SCInterfaceSoap InterfaceSoap;

	TString httpmessage;
	fTaille_Retour = 0;
	httpmessage.Form("http://%s:%d", GetDeviceName(), port);

	error = InterfaceSoap.ReadScope( (char*) httpmessage.Data(),&fZoneData_char, &fSizeZoneData,&sizevector);

	fTaille_Retour =fSizeZoneData;

	fSize_SC_buffer=sizevector;

	if (error!=0) {
		fError.TreatError(1, 0, "GNetClientSoap::GetBufferSC no returned data ");
		fComptNotAvailable++;
		if ((fComptNotAvailable%100+1) == 100) sleep(1);
		return false;
	}
	tempos.Form("GNetClientSoap : Get vector sizeof= %d   \n", fTaille_Retour);
	//GetDumpRawReal(vecteurc, 64, 0, NULL);
	fError.Infos(tempos);
	fComptbuffOK++;

	fSCBufferEndReached = false;
	Close();
	fStatus = ACQ_OK;

#else
	tempos.Form("GNetCLientSoap : Compile with NO_GSOAP , so GNetClientSoap::GetBufferSC inoperative" );
	fError.Infos(tempos);
	return false;
#endif
	return true;

}
//_________________________________________________________________________________

void GNetClientSoap::GetBuffer(bool afficher) {
	//Get data buffer
	TString tempos;
	int countinsidebuffer = 0;
	fStatus = ACQ_OK;
	int buffer_size = 0;
	int type = 0;
	if (fSCBufferEndReached) {// if end is reached get a new buffer
		if (!GetBufferSC(afficher)) {
			cout
					<< " debug  GNetClientSoap::GetBuffer no buffer available -> return\n";
			return;
		}

		countinsidebuffer = 0;
		fZoneData_cur_char = fZoneData_char;
		fBuffer ->SetExternalDataZone(fZoneData_cur_char, MFM_BLOB_HEADER_SIZE);
		fBuffer->SetType(fBuffer->TestType(fZoneData_cur_char));


		if (fBuffer->IsAMFMBuffer()) {
			fTimestampsbuffersize = 0;
			fBuffer = fBuffer;
		} else {
			fStatus = ACQ_UNKBUF;
			fError.TreatError(2, 0, "Buffer in  buffer unknown!");
			cout << "type " << type << endl;
			fBuffer->DumpBuffer(256, 0);
			fSCBufferEndReached = true;

		}
	} //end of SCBufferEndReached
	fZoneData_cur_int = (int*) fZoneData_cur_char;
	countinsidebuffer++;
	switch (fStatus) {
	case ACQ_OK:

		fBuffer ->SetExternalDataZone(fZoneData_cur_char, MFM_BLOB_HEADER_SIZE);
		buffer_size = fBuffer->GetBufSize();
		fBuffer->SetAttributs(true);
		fZoneData_cur_char += buffer_size;
		fZoneData_cur_int = (int*) fZoneData_cur_char;
		if ((fZoneData_cur_char >= fZoneData_char + fSize_SC_buffer)
				or (buffer_size <= 0)) {
			fSCBufferEndReached = true;
		}

		fNb_Buffers_Local++;
		if (fBuffer->IsEventBuffer()) {
			//fBuffer->DumpBuffer(256, 0);
			//cout <<" GNetClientSoap::GetBuffer fBuffer->GetNumBuf() ="<<fBuffer->GetNumBuf()<<"\n";
			if (fPremier_Buffer == 0)
				fPremier_Buffer = fBuffer->GetNumBuf();
			fNb_Buffers_Recus++;
			fNBuffers_Envoyes = fBuffer->GetNumBuf() - fPremier_Buffer + 1;
			if (fLastNumBuff == fBuffer->GetNumBuf()) {
				//fError.TreatError(1, 0,
				//	"Same Buffer Number...so it may be same buffer...");
			}
			fLastNumBuff = fBuffer->GetNumBuf();
		}

		break;
	default:
		if (afficher)
			fError.TreatError(1, 0, "Not a new buff");
	}
	return;
}

//_________________________________________________________________________________
void GNetClientSoap::ReadBuffer() {
	// Read a buffer
	struct timeval mt_reference;
	struct timeval mt_Timeout;
	struct timezone tz;
	Int_t time_out = 10;
	gettimeofday(&mt_Timeout, &tz);
	gettimeofday(&mt_reference, &tz);
	fStatus = ACQ_OK;
	while (true) {
		GetBuffer(false);
		gettimeofday(&mt_Timeout, &tz);
		if (((mt_reference.tv_sec - mt_Timeout.tv_sec) > time_out)) {
			fError.TreatError(1, 0, "Time out with no event");
			break;
		}
		if (fStatus != ACQ_NOCURRCTRLBUF)
			break;
	};
	if (fBuffer)
		fBuffer->SetAttributs(true);
}

//_________________________________________________________________________________

