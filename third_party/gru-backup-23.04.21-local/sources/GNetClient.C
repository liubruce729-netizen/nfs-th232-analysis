// Author: $Author: legeard $
// Part of GRU
//
// GNetClient : base of net Client . Shoulb be virtual
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


#include "GNetClient.h"
#include "TCanvas.h"
#include "TProfile.h"

ClassImp( GNetClient);
//_________________________________________________________________________________________
GNetClient::GNetClient(char* host) {

	SetDevice(host);
	fPortDef = 9090; //default Root Port
	SetPort(fPortDef); //Root Port
	fIsOpen = false;

}
//_________________________________________________________________________________________
GNetClient::~GNetClient() {
	//Close();

}

//_________________________________________________________________________________________
void GNetClient::InitClient() {

}

//_________________________________________________________________________________________

void GNetClient::ResetAllSpectraOnServer(GSpectrumIdentity* id) {
	char cmd[MAX_CARACTERES];

	Int_t port;
	Bool_t SameServer = true;
	//TString spectrumname = id->GetSpectrumName();
	TString spectrumname = "";
	TString family = id->GetFamily();
	TString tempo;
	TString SourceName = id->GetSourceName();
	TString SourceType = id->GetSourceType();
	TString Source = id->GetSource();//net or file
	port = id->GetPort();

	if ((CompareWordsIgnoreCase(&Source, "FILE")) or (CompareWordsIgnoreCase(
			&Source, "GANIL"))) {
		fError.TreatError(1, 0,
				"Reset Spectrum in file or Ganil type, not implemented in GNetClient");
		return;
	}

	char command[MAX_CARACTERES];
	strcpy(command, "GRU spectra reset");
	strcpy(cmd, command);

	if (strcmp(SourceName.Data(), "") == 0) {
		tempo.Form(
				"GNetClient::ResetAllSpectraOnServer : No name for spectrum server");
		fError.TreatError(0, 0, tempo);
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
					" Any connected server fot ResetAllSpectraOnServer!");
			fStatus = ACQ_DISCONNECT;
			return;
		}
	}
	tempo.Form("Send command :%s to %s", command, fName);
	fError.TreatError(0, 0, tempo);
	SendCommand(command);
	Close();
	return;

}

//_________________________________________________________________________________________
void GNetClient::ResetSpectrumOnServer(GSpectrumIdentity* id) {

	char cmd[MAX_CARACTERES];
	Int_t port;
	Bool_t SameServer = true;
	TString spectrumname = id->GetSpectrumName();
	TString family = id->GetFamily();
	TString tempo;
	TString SourceName = id->GetSourceName();
	TString SourceType = id->GetSourceType();
	TString Source = id->GetSource();//net or file
	port = id->GetPort();

	if ((CompareWordsIgnoreCase(&Source, "FILE")) or (CompareWordsIgnoreCase(
			&Source, "GANIL"))) {
		fError.TreatError(1, 0,
				"Reset Spectrum in file or Ganil type, not implemented in GNetClient");
		return;
	}

	char command[MAX_CARACTERES];
	strcpy(command, "GRU spectrum reset ");
	strcat(command, spectrumname.Data());
	strcat(command, " ");
	strcat(command, family.Data());
	strcpy(cmd, command);

	if (strcmp(spectrumname.Data(), "") == 0) {
		tempo.Form("No name for spectrum");
		fError.TreatError(0, 0, tempo);
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
					" Any connected server fot ResetSpectrumOnServer!");
			fStatus = ACQ_DISCONNECT;
			return;
		}
	}

	fError.TreatError(0, 0, tempo);

	SendCommand(command);
	Close();
	return;
}
//_________________________________________________________________________________________

bool GNetClient::TestServer(int time) {
	// test server
	// return is true if server seem to be good GRU server and started
	// else return is false
	char command[MAX_CARACTERES];
	sprintf(command, "GRU TEST %d", time);

	bool retour = false;

	fError.TreatError(0, 0, "Send Commmand : ", command);

	retour = SendCommand(command);

	if (retour) {
		fError.TreatError(0, 0, " TEST OK");
		retour = true;
	} else {
		Close();
		fError.TreatError(1, 0, "Test no OK");
		retour = false;
	}
	return (retour);
}

//_________________________________________________________________________________________
bool GNetClient::SendCommand(const char* command, TString* stretour) {

	Int_t i, nb_words;
	TString tempo;
	TString* tempo2;
	tempo2 = NULL;
	bool retour = false;
	if (stretour != NULL)
		stretour->Form("NO_OK");
	char **commandsliced = NULL;
	int* commandsliced_int = NULL;
	nb_words = Slicer((char *) command, &commandsliced, &commandsliced_int);

	if (nb_words == 0) {
		return (retour);
	}

	cout << "-- Command send (" << nb_words << ")-- :";
	for (i = 0; i < nb_words; i++) {
		cout << " " << commandsliced[i];
	}
	cout << "\n";

	if (!(TestGruWord(commandsliced[0]))) {
		return (retour);
	}
	if ((nb_words > 2) and (CompareWordsIgnoreCase(commandsliced[1], "GET"))) {
		tempo2 = SendCommand1((char*) command);
		if (stretour != NULL)
			*stretour = *tempo2;
		if (tempo2)
			delete (tempo2);
		retour = true;
	} else {
		retour = SendCommand0((char*) command);
		if (CompareWordsIgnoreCase(commandsliced[1], "kill")) Close();
		if ((retour) and (stretour != NULL))
			stretour->Form("NO_OK");
	}

	if (commandsliced) {
			for (int i = 0; i < nb_words; i++) {
				delete[] commandsliced[i];
				commandsliced[i] = NULL;
			}
			delete[] commandsliced;
			commandsliced = NULL;
		}
		if (commandsliced_int) {
			delete []commandsliced_int;
			commandsliced_int = NULL;
		}

	if (!retour)
		fError.TreatError(1, fStatus, "GNetClient : grusoap command");

	return (retour);
}

//_________________________________________________________________________________________
TObject* GNetClient::SendCommand(const char* command, TObject *obj) {
    // send command and wait a object un return in obj.

	Int_t port = GetPort();
	char httpmessage[512];
	sprintf(httpmessage, "http://%s:%d", GetDeviceName(), port);
	Int_t i, nb_words;
	int commanddone = 0;
	//bool retour = false;

	char **commandsliced = NULL;
	int* commandsliced_int = NULL;
	nb_words = Slicer((char*) command, &commandsliced, &commandsliced_int);
	if (nb_words == 0) {
		return (NULL);
	}

	cout << "-- Command send (" << nb_words << ")-- :";
	for (i = 0; i < nb_words; i++) {
		cout << " " << commandsliced[i];
	}
     cout <<"\n";

	if (!(TestGruWord(commandsliced[0]))) {
		return (NULL);
	}
	if ((nb_words > 2) and (CompareWordsIgnoreCase(commandsliced[1], "GET"))) {
		if (CompareWordsIgnoreCase(commandsliced[2], "spectra_db")) {
						obj = (TObject*)GetSpectraDB();
						commanddone++;
					}
		if (nb_words > 3) {
			if (CompareWordsIgnoreCase(commandsliced[2], "SPECTRUM")) {
				if (nb_words == 4) {
					obj = (TObject* )GetSpectrum(commandsliced[3], (TNamed*)obj);
					commanddone++;
				}
				if (nb_words > 4) {
					obj =  (TObject* )GetSpectrum(commandsliced[3], commandsliced[4], (TNamed*)obj);
					commanddone++;
				}
			}

		}
	}

	if (commandsliced) {
		for ( int i = 0;i<nb_words;i++){
				delete[] commandsliced[i];
				commandsliced[i]=NULL;
			}
			delete[] commandsliced;
			commandsliced= NULL;
		}
		if (commandsliced_int){
			delete commandsliced_int;
			commandsliced_int = NULL;
		}

	if (commanddone == 1) {
		return (obj);
	} else {

		fError.TreatError(1, 0, "GNetClient::SendCommand , not understood command ");
	}
	return (obj);
}

//________________________________________________________________________________
TString* GNetClient::RequestInfo() {
	//InfoControl to get realtime info while acq

	TString *streturn = NULL;
	int retour = SendCommand("GRU GET INFORMATION", streturn);
	if (retour == 0)
		fError.TreatError(1, 0, "GNetClient : No Pid");
	return (streturn);
}
//_________________________________________________________________________________
TString* GNetClient::GetPid() {
	//Method to get pid of GRUCore

	TString *streturn = NULL;
	int retour = SendCommand("GRU GET PID", streturn);
	if (retour == 0)
		fError.TreatError(1, 0, "GNetClient : No Pid");
	return (streturn);
}
//_________________________________________________________________________________
TString* GNetClient::GetListSpectra() {
	// Get the liste of histogram (spectrum) from Ganil Acquisition
	// low and high give limit of spectra number to load in list
	TString *listespectra = NULL;
	int retour = SendCommand("GRU GET SPECTRA LIST", listespectra);
	if (retour == 0)
		fError.TreatError(0, 0, "GNetClient : Liste Spectra emtpy");
	return (listespectra);
}
//_________________________________________________________________________________
TString* GNetClient::GetInfo() {
	// Get the info from Ganil Acquisition


	TString *Info = NULL;
	SendCommand("GRU GET INFO", Info);
	return (Info);
}

//_________________________________________________________________________________________

bool GNetClient::EndOfPage() {
	// Send message  to signal a EndOfPage

	char command[MAX_CARACTERES];
	sprintf(command, "GRU ENDOFPAGE");
	char cmd[MAX_CARACTERES];
	bool retour = false;

	strcpy(cmd, command);
	// we suppose that connexion is open

	if (!fIsOpen) {
		Open();
		if (!fIsOpen) {
			fError.TreatError(2, 0,
					" Any connected server for ENDOFPAGEServer!");
			fStatus = ACQ_DISCONNECT;
			return (false);
		}
	}
	if (SendCommand(command)){
		fError.TreatError(0, 0, "Retour code = GRU_GRU_ENDOFPAGE");
		retour = true;
	} else {
		Close();
		retour = false;
	}
	return (retour);
}
//_________________________________________________________________________________________

bool GNetClient::InitInput(char* ip, int port) {
	char command[MAX_CARACTERES];
	sprintf(command, "GRU INPUT NARVAL %s %d", ip, port);
	bool retour = SendCommand(command);
	if (retour)
		fError.TreatError(0, 0, "GNetClient : INPUT OK");
	else
		fError.TreatError(1, 0, "GNetClient : INPUT Not OK");
	return (retour);
}//_________________________________________________________________________________________

bool GNetClient::InitInputFile(char* name) {
	char command[MAX_CARACTERES];
	sprintf(command, "GRU INPUT FILE %s", name);
	bool retour = SendCommand(command);
	if (retour)
		fError.TreatError(0, 0, "GNetClient : INPUT OK");
	else
		fError.TreatError(1, 0, "GNetClient : INPUT Not OK");
	return (retour);
}//_________________________________________________________________________________________
bool GNetClient::InitGUser(int mode) {
	char command[MAX_CARACTERES];

	if (mode == 1)
		sprintf(command, "GRU INITACQ GUSER");
	else if (mode == 2)
		sprintf(command, "GRU INITACQ NO");
	else if (mode == 3)
		sprintf(command, "GRU INITACQ INFINITE");
	bool retour = SendCommand(command);
	if (retour)
		fError.TreatError(0, 0, "GNetClient : INITACQ-GUSER OK");
	else
		fError.TreatError(1, 0, "GNetClient : INITACQ-GUSER Not OK");
	return (retour);
}//_________________________________________________________________________________________

bool GNetClient::InitEvent(char* name) {
	char command[MAX_CARACTERES];
	sprintf(command, "GRU INITEVENT %s", name);
	bool retour = SendCommand(command);
	if (retour)
		fError.TreatError(0, 0, "GNetClient : INITEVENT OK");
	else
		fError.TreatError(1, 0, "GNetClient : INITEVENT Not OK");
	return (retour);
}

//_________________________________________________________________________________________
bool GNetClient::SetBuffer(int size) {
	char command[MAX_CARACTERES];
	sprintf(command, "GRU INPUT BUFFERSIZE %d", size);
	bool retour = SendCommand(command);
	if (retour)
		fError.TreatError(0, 0, "GNetClient : BUFFERSIZE OK");
	else
		fError.TreatError(1, 0, "GNetClient : BUFFERSIZE Not OK");
	return (retour);
}
//_________________________________________________________________________________________
bool GNetClient::StartRawSpectra() {

	char command[MAX_CARACTERES];
	sprintf(command, "GRU SPECTRA ALL");
	bool retour = SendCommand(command);
	if (retour)
		fError.TreatError(0, 0, "GNetClient : START-RAW-SPECTRA OK");
	else
		fError.TreatError(1, 0, "GNetClient : START-RAW-SPECTRA Not OK");
	return (retour);
}

//_________________________________________________________________________________________

bool GNetClient::SpectraSave(char *name) {
	char command[MAX_CARACTERES];
	sprintf(command, "GRU SPECTRA SAVE %s", name);
	bool retour = SendCommand(command);
	if (retour)
		fError.TreatError(0, 0, "GNetClient : SPECTRASAVE OK");
	else
		fError.TreatError(1, 0, "GNetClient : SPECTRASAVE Not OK");
	return (retour);

}//_________________________________________________________________________________________

bool GNetClient::Start() {
	char command[MAX_CARACTERES];
	sprintf(command, "GRU RUN START");
	bool retour = SendCommand(command);
	if (retour)
		fError.TreatError(0, 0, "GNetClient : RUN-START OK");
	else
		fError.TreatError(1, 0, "GNetClient : RUN-START Not OK");
	return (retour);
}
//_________________________________________________________________________________________

bool GNetClient::Pause(int time) {
	char command[MAX_CARACTERES];
	if (time < 0)
		sprintf(command, "GRU RUN PAUSE");
	else
		sprintf(command, "GRU RUN PAUSE %d", time);
	bool retour = SendCommand(command);
	if (retour)
		fError.TreatError(0, 0, "GNetClient : RUN-PAUSE OK");
	else
		fError.TreatError(1, 0, "GNetClient : RUN-PAUSE Not OK");
	return (retour);
}
//_________________________________________________________________________________________

bool GNetClient::Stop() {
	char command[MAX_CARACTERES];
	sprintf(command, "GRU RUN STOP");
	bool retour = SendCommand(command);
	if (retour)
		fError.TreatError(0, 0, "GNetClient : RUN-STOP OK");
	else
		fError.TreatError(1, 0, "GNetClient : RUN-STOP Not OK");
	return (retour);
}
//_________________________________________________________________________________________

bool GNetClient::Finish() {
	char command[MAX_CARACTERES];
	sprintf(command, "GRU RUN END");
	bool retour = SendCommand(command);
	if (retour)
		fError.TreatError(0, 0, "GNetClient : RUN-END OK");
	else
		fError.TreatError(1, 0, "GNetClient : RUN-END Not OK");
	return (retour);
}
//_________________________________________________________________________________________
//_________________________________________________________________________________________

