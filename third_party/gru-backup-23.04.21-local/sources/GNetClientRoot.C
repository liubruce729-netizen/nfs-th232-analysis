// Author: $Author: legeard $
// Part of GRU
//
// GNetClientRoot : Client of a GNetServerRoot. It can ask for a spectrum Data base and
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


#include "GNetClientRoot.h"

ClassImp( GNetClientRoot);
//_________________________________________________________________________________________
GNetClientRoot::GNetClientRoot(char* host) {
	fType = GA_NET_ROOT;
	fVerbose = 0;
	SetDevice(host);
	fPortDef = 9090; //default Root Port
	SetPort(fPortDef); //Root Port
	fTempo = 60;// time max that this device stays open.
	fIsOpen = false;
	fSock = NULL;
}
//_________________________________________________________________________________________
GNetClientRoot::~GNetClientRoot() {
	Close();
}

//_________________________________________________________________________________________
void GNetClientRoot::InitClient()

{

}
//_________________________________________________________________________________________
TString *GNetClientRoot::GetListSpectra() {
	//return in a TString the liste of histogram
	// low and high have no effect in this method

	TString command = "GRU GET spectra LISTE";
	TString *liste = new TString();
	if (!fIsOpen) {
		Open();
		if (!fIsOpen) {
			fError.TreatError(2, 0, " Any connected server !");
			fStatus = ACQ_DISCONNECT;
			return NULL;
		}
	}
	fError.TreatError(0, 0, " Command envoyee (liste ): ", command.Data());
	GiveWords(fSock, &command);
	ReceiveWords(fSock, liste);
	Close(fTempo);
	return (liste);
}

//_________________________________________________________________________________________
TNamed* GNetClientRoot::GetSpectrum(const char *spectrumname, TNamed *old_sp) {
	//return  the histogram with "histoname"
	// old_histo: in case of refresh we use the odl histogram

	TString command;
	command.Form(" gru get spectrum %s", spectrumname);
	TNamed*sp;

	if (strcmp(spectrumname, "") == 0) {
		fError.TreatError(2, 0, "No name for spectrum");
		return (NULL);
	}

	if (!fIsOpen) {
		Open();
		if (!fIsOpen) {
			fError.TreatError(2, 0,
					"Any connected server for GetSpectrum(const char, TNamed*)!");
			fStatus = ACQ_DISCONNECT;
			return NULL;
		}
	}

	fError.TreatError(0, 0, "Send command :", command);

	GiveWords(fSock, &command);
	sp =(TNamed*) ReceiveObject(fSock);
	Close(fTempo);

	if ((old_sp) && (sp)) {
		delete (old_sp);
	}
	if ((old_sp) && (!sp))
		sp = (old_sp);
	return (sp);

}
//_________________________________________________________________________________________
TNamed* GNetClientRoot::GetSpectrum(const char *spectrumname,
		const char *family, TNamed *old_sp) {
	//return  the histogram with "histoname"
	// old_histo: in case of refresh we use the odl histogram
	TString tempo;
	TNamed*sp;
	TString command;
	command.Form("GRU get spectrum %s %s", spectrumname, family);

	if (strcmp(spectrumname, "") == 0) {
		tempo.Form("No name for spectrum");
		fError.TreatError(2, 0, tempo);
		return (NULL);
	}

	if (!fIsOpen) {
		Open();
		if (!fIsOpen) {
			fError.TreatError(2, 0,
					" Any connected server for GetSpectrum(const char*,const char, TNamed* ) ");
			fStatus = ACQ_DISCONNECT;
			return NULL;
		}
	}

	GiveWords(fSock, &command);
	sp =(TNamed*) ReceiveObject(fSock);
	Close(fTempo);
	//Close(fTempo);
	if ((old_sp) && (sp)) {
		delete (old_sp);
	}
	if ((old_sp) && (!sp))
		sp = (old_sp);
	return (sp);
}

//_________________________________________________________________________________________
void GNetClientRoot::GetSpectrum(GSpectrumIdentity* id) {
	//replace spectrum in spectrum indentity

	Int_t port;
	TNamed* sp;
	TString  tempos;
	Bool_t SameServer;
	TString spectrumname = id->GetSpectrumName();
	TString family = id->GetFamily();
	TString SourceName = id->GetSourceName();
	TString SourceType = id->GetSourceType();
	TString Source = id->GetSource();//net or file
	port = id->GetPort();

	SameServer = true;
	fError.TreatError(0, 0,
			"Get Spectrum file via  GNetClientRoot::GetSpectrum(GSpectrumIdentity* id)");
	if (Source.CompareTo("FILE") == 0) {
		fError.TreatError(1, 0,
				"Get Spectrum file via indentity, not implemented in GNetClientRoot");
		return;
	}
	if (SourceType.CompareTo("GANIL") == 0) {
		fError.TreatError(1, 0,
				"Get Ganil Spectrum via indentity, not implemented in GNetClientRoot");
		return;
	}

	TString command;
	command = "GRU get spectrum " + spectrumname + " "+ family;

	if (strcmp(spectrumname.Data(), "") == 0) {
		fError.TreatError(0, 0, "GNetClientRoot::GetSpectrum :No name for spectrum");
		return;
	}

	if ((SourceName == fName) && (GetPort() == port))
		SameServer = true;
	else
		SameServer = false;

	if (!SameServer) {
		if (fIsOpen)
			Close();
		SetPort(port);
		SetDevice(SourceName.Data());
	}

	if (!fIsOpen) {
		Open();
		if (!fIsOpen) {
			fError.TreatError(2, 0,
					" Any connected server for GetSpectrum(GSpectrumIdentity* id)!");
			fStatus = ACQ_DISCONNECT;
			return;
		}
	}

	tempos.Form("Send command :%s to %s", command.Data() , fName);
    fError.Infos(tempos);
	GiveWords(fSock, &command);
	sp = (TNamed*)ReceiveObject(fSock);

	Close(fTempo);

	if ((sp)) {
		id->DeleteSpectrumInstance();
		id->SetSpectrum(sp);
	}
	return;

}

//_________________________________________________________________________________________
void GNetClientRoot::SendSpectrum(GSpectrumIdentity* id) {
	//replace spectrum in spectrum indentity

	Int_t IdPort;

	Bool_t SameServer = true;

	TNamed* spectrum = id->GetSpectrum();
	TNamed* spectrum2;
	spectrum2 = spectrum;
	TString spectrumname = id->GetSpectrumName();
	TString family = id->GetFamily();
	TString tempo;
	TString SourceName = id->GetSourceName();
	TString SourceType = id->GetSourceType();
	TString Source = id->GetSource();//net or file


	IdPort = id->GetPort();
	if (spectrum == NULL) {
		fError.TreatError(1, 0,
				"Send Ganil Spectrum  impossible, Spectrum is null");
		return;
	}
	if (!(spectrum->InheritsFrom(TNamed::Class()))) {
		fError.TreatError(1, 0, "It's not a TNnamed : ", spectrumname);
		return;
	}

	fError.Infos(
			"Send Spectrum file via  GNetClientRoot::SendSpectrum(GSpectrumIdentity* id)");
	if (Source.CompareTo("FILE") == 0) {
		fError.TreatError(1, 0,
				"Send Spectrum file via indentity, not implemented in GNetClientRoot");
		return;
	}
	if (SourceType.CompareTo("GANIL") == 0) {
		fError.TreatError(1, 0,
				"Send Ganil Spectrum via indentity, not implemented in GNetClientRoot");
		return;
	}

	TString command;
	command.Form( "GRU send SPECTRUM %s %s", spectrumname.Data(),family.Data());

	if (strcmp(spectrumname.Data(), "") == 0) {
		tempo.Form("No name for spectrum");
		fError.TreatError(0, 0, tempo);
		return;
	}

	if ((SourceName == fName) && (GetPort() == IdPort))
		SameServer = true;
	else
		SameServer = false;

	if (fIsOpen) {
		if (!SameServer) {
			Close();
		}

	}

	if (!fIsOpen) {
		if (!SameServer) {
			SetPort(IdPort);
			SetDevice(SourceName.Data());
		}
		Open();
		if (!fIsOpen) {
			fError.TreatError(2, 0,
					" Any connected server for SendSpectrum(GSpectrumIdentity* id)!");
			fStatus = ACQ_DISCONNECT;
			return;
		}
	}

	fError.TreatError(0, 0, "Send command : ", command);
	GiveWords(fSock, &command);
	GiveObject(fSock,(TObject*) spectrum2);
	Close();
	return;
}

//_________________________________________________________________________________________

GSpectraDB* GNetClientRoot::GetSpectraDB() {
	//return  a Data base of spectra

	TString tempo;
	TString command = "GRU get spectra_db";
	GSpectraDB *spectraDB = NULL;
	if (!fIsOpen) {
		Open();
		if (!fIsOpen) {
			fError.TreatError(2, 0, "Any connected server for GetSpectraDB!");
			fStatus = ACQ_DISCONNECT;
			return (NULL);
		}
	}
	tempo.Form("GNetClientRoot::GetSpectraDB() Send command :%s ", command.Data());
	fError.TreatError(0, 0, tempo);

	GiveWords(fSock, &command);
	spectraDB =(GSpectraDB *) (ReceiveObject(fSock));
	Close(fTempo);
	return (spectraDB);
}

//_________________________________________________________________________________________
void GNetClientRoot::SpeSave(char* filename) {
	//to save histograms of root server

	fError.TreatError(2, fStatus, " Not yet implemented");
}
//_________________________________________________________________________________________
void GNetClientRoot::Open(char* mod) {
	//set and open network root client
	Open();
}
//_________________________________________________________________________________________
Int_t GNetClientRoot::TestPortFree(Int_t port, char* name) {
	//test a port by a open/close
	// return port if port is free
	// return O if port is not free
	// return -1 if pb
	fSock = NULL;
	TString tempo;
	TSocket *sock;
	Int_t ret;
	ret = port;
	if (fVerbose) {
		tempo.Form("TestPort to : %s on port : %d ", name, port);
		fError.TreatError(0, 0, tempo);
	}

	if (strcmp(name, "") == 0) {
		fError.TreatError(2, fStatus,
				"GNetClientRoot device name not valid in TestPort:", name);
		ret = -1;
		return ret;
	}
	sock = NULL;
	sock = new TSocket(name, port);

	if (sock !=NULL) {
		if (sock->IsValid()) {
			ret = 0;
			if (fVerbose) {
				tempo.Form("TestPort to : %s on port : %d done so no free",
						name, port);
				fError.TreatError(0, 0, tempo);
			}
			sock->Close();
		}
	}
	return ret;
}

//_________________________________________________________________________________________
void GNetClientRoot::Open(char mod) {
	//set and open network root client
	if (fIsOpen)
		return;
	if (fSock != NULL)
		fError.TreatError(1, 0, "Open with a non null socket");

	TString tempo;
	if (strcmp(fName, "") == 0) {
		fError.TreatError(2, fStatus, "GNetClientRoot device name not valid :",
				fName);
		return;
	}

	gettimeofday(&fMt_permitclose, &fTz);
	fSock = NULL;

	fSock = new TSocket(fName, GetPort());
	if (fSock != NULL) {
		if (fSock->IsValid()) {
			tempo.Form("Connection to : %s on port : %d done", fName, GetPort());
			fError.TreatError(0, 0, tempo);
			fIsOpen = true;
		}
	}

	TString info;
	info = fName;
	info += ":";
	info += GetPort();
	if (!fIsOpen)
		fError.TreatError(2, fStatus, "in connection to", info);
}
//______________________________________________________________________________

void GNetClientRoot::Close() {
	MyClose();
}
//______________________________________________________________________________

void GNetClientRoot::Close(Int_t tempo) {
	MyClose(tempo);
}
//_________________________________________________________________________________________
void GNetClientRoot::MyClose(Int_t tempo) {
	//close network root client
	//if tempo =0 close is immediat
	//if tempo >0  , the connection is not close until tempo seconds

	if (fIsOpen) {
		gettimeofday(&fMt_reference, &fTz);
		Int_t current_diff_time = (fMt_reference.tv_sec
				- fMt_permitclose.tv_sec);

		if (tempo != 0) {
			if ((current_diff_time) < tempo){
				return;
			}
		}

		//set and opent network root client

		fSock->Close();
		fIsOpen = false;
		if (fSock) {
			delete (fSock);
			fSock = NULL;
		}
	}
}


//_________________________________________________________________________________________
bool GNetClientRoot::SendCommand0(char* command) {
	// send simple command with a bool in return

	TString tempos;
	if (!fIsOpen) {
		Open();
		if (!fIsOpen) {
			fError.TreatError(2, 0, " Any connected server for command :",
					"command");
			fStatus = ACQ_DISCONNECT;
			return false;
		}
	}

	tempos = command;
	GiveWords(fSock, &tempos);
	ReceiveWords(fSock, &tempos);
	if (strcmp(tempos.Data(), "GRU_OK") == 0) {
		fError.TreatError(0, 0, " Retour recu = GRU_OK");
	} else {
		//Close(fTempo);
	}
	Close(fTempo);
	return true;
}
//_________________________________________________________________________________________

TString* GNetClientRoot::SendCommand1(char* command) {
	// send simple command with a string in return
	TString* listwords;
	TString tempos;
	listwords = new TString("");
	if (!fIsOpen) {
		Open();
		if (!fIsOpen) {
			fError.TreatError(2, 0, " Any connected server for command :",
					"command");
			fStatus = ACQ_DISCONNECT;
			return listwords;
		}
	}

	tempos = command;
	GiveWords(fSock, &tempos);
	ReceiveWords(fSock, listwords);
	Close(fTempo);
	return listwords;
}

//_________________________________________________________________________________________
void GNetClientRoot::GiveWords(TSocket *localsock, TString* words) {
	TMessage mess(kMESS_STRING);
	mess.Reset();
	mess.SetWriteMode();
	mess.WriteTString((const TString) (*words));
	localsock->Send(mess); // send message

}
//_________________________________________________________________________________________
int GNetClientRoot::ReceiveWords(TSocket *localsock, TString* recp) {
	TMessage *mess = NULL;
	int ret = localsock->Recv(mess);
	if (mess) {

		if (mess->What() == kMESS_STRING) {
			mess->SetReadMode();
			mess->ReadTString(*recp);
		} else {
			ret = 0;
			fError.TreatError(1, 1, "ReceiveWords :Not expected format ");
		}
		delete mess;
	} else {
		ret = 0;
		fError.TreatError(1, 1, "ReceiveWords : message null ");
	}
	return ret;
}

//_________________________________________________________________________________________
void GNetClientRoot::GiveObject(TSocket *localsock, TObject * obj) {
	TMessage mess(kMESS_OBJECT);
	mess.Reset();
	mess.WriteObject(obj);
	localsock->Send(mess); // send message
}
//_________________________________________________________________________________________
TObject * GNetClientRoot::ReceiveObject(TSocket *localsock) {

	TMessage *message = NULL;
	TObject *obj;
	TString retour;
	UInt_t what;

	obj = NULL;
	what = 0;

	localsock->Recv(message);

	if (message) {
		what = message->What();
		if (what == kMESS_STRING) {
			message->ReadTString(retour);
			fError.TreatError(
					2,
					0,
					" not expected text in GNetClientRoot:: ReceiveObject, So you can suppose that object is null");
			delete message;
			return (NULL);
		}

		if (what == kMESS_OBJECT) {
			obj = message->ReadObject(message->GetClass());
			if (obj) {
				if (obj->InheritsFrom(TObject::Class())) {
					delete message;
					return ((TObject*) obj);
				} else {
					delete message;
					delete obj ;
					obj= NULL;
					fError.TreatError(2, 1, "returned object not a TObject");
					return (NULL);
				}
			} else {
				fError.TreatError(2, 1, "returned empty object");
				return (NULL);
			}
		}
	}
	fError.TreatError(2, what,
			"= Message  ( not a TString or TObject in ReceiveObject )");
	return (NULL);
}



