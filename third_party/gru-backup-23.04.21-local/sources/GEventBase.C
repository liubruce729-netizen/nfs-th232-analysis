// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GEventBase
//
// This class manage events..
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
#include <string>
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
#include "GEventBase.h"
#include "GBuffer.h"

extern "C" {
#include "GEN_TYPE.H"
#include "STR_EVT.H"
#include "ERR_GAN.H"
#include "EQUIPDES.H"
#include "acq_swap_buf.h"
#include "gan_acq_buf.h"
#include "acq_codes_erreur.h"
#include "math.h"
}

#include <iostream>
#include <fstream>
#include <cstdlib>

//_______________________________________________________________________________
ClassImp( GEventBase);

GEventBase::GEventBase(DataParameters* parameter) {
	//Constructor

	fStatus = ACQ_OK;
	fEventNumber = -1;
	fEventCount = 0;
	pParameter = parameter;
	fTimeStamp = 0;
	fMFM_Event = false;
	fLaw = 0;
	fProba = 0.6;
	SetRandomLaw(fLaw);
	SetRandomProba(fProba);
	fLipflop = true;
	fIsStrFilePresent= false;
	pDataArray = NULL;
	fDataArraySize = 0;
	fInit_event_done = false;
}
//_______________________________________________________________________________
GEventBase::~GEventBase() {
	//destructor
	if (pDataArray) {
			delete[] pDataArray;
			pDataArray = NULL;
		}
}
//______________________________________________________________________________

void GEventBase::SetRandomLaw(int law) {
	// set random law in case random generation ( 0 =  gauss, 1= white noise , 2 binomial);
	fLaw = law;
}
//______________________________________________________________________________

void GEventBase::SetRandomProba(Float_t proba) {
	// set random probability in case random generation
	fProba = proba;
}

//______________________________________________________________________________
void GEventBase::EventInit(char* Exp_Name) {
	// get Structure of event from ACTION Files
	// we suppose that a ACTIONS_"Exp_Name".CHC_STR and a ACTIONS_"Exp_Name".CHC_PAR are
	// present in current directory.
	// Exp_Name often designs the experiment name, ie "e444" by default the Exp_Name is "local"
	// If not, you have to copy them or realize them from a run file and with Inquire() method
	struct stat FileStat;
	char actionFileSTR[MAX_CARACTERES];
	char actionFilePAR[MAX_CARACTERES*2];
	char actionFilePAR2[MAX_CARACTERES];
	fStatus = ACQ_OK;
	TString tempos;
	TString HomeDir = gSystem->HomeDirectory();

	// for this moment, it is impossible to reinit a event thus if already done, we go out
	if (EventInitAlready()) {
		cout << " Event already initialized \n";
		return;
	}

	strcpy(actionFileSTR, "");
	strcpy(actionFilePAR, "");

	sprintf(actionFilePAR, "ACTIONS_%s.CHC_PAR", Exp_Name);
	strcpy(actionFilePAR2, actionFilePAR);

	// test de presence des fichiers
	if (stat(actionFilePAR, &FileStat) < 0) {
		strcpy(actionFilePAR, "");
		sprintf(actionFilePAR, "%s/ganacq_manip/%s/GECO/%s/ACQ/%s",
				HomeDir.Data(), Exp_Name, Exp_Name, actionFilePAR2);
		if (stat(actionFilePAR, &FileStat) < 0) {
			strcpy(actionFilePAR, "");
			sprintf(actionFilePAR, "%s/ganacq_manip/%s/das-save/%s",
					HomeDir.Data(), Exp_Name, actionFilePAR2);
			if (stat(actionFilePAR, &FileStat) < 0) {
				strcpy(actionFilePAR, Exp_Name);
				if (stat(actionFilePAR, &FileStat) < 0) {
					fError.TreatError(2, 0,
							"GEventBase::EventInit File ACTION not found");
					fError.TreatError(2, 0,
							"Event can't be initialized correctly");
					fInit_event_done= true;
					return;
				}
			}
		}
	}

	tempos = actionFilePAR;
	fError.TreatError(0, 0,  "Initialization of event with file",tempos);
	if (tempos.EndsWith(".CHC_PAR")) {
		tempos.ReplaceAll(".CHC_PAR", "");
	} else {
		fError.TreatError(2, 0,
				"File must be name like ACTIONS_experiment.CHC_PAR");
	}
	strcpy(actionFileSTR, "");
	sprintf(actionFileSTR, "%s.CHC_STR", tempos.Data());
	if (stat(actionFileSTR, &FileStat) >= 0) {
		fIsStrFilePresent = true;
		//cout <<"	cfIsStrFilePresent "<< fIsStrFilePresent<<"\n";
		tempos = actionFileSTR;
		fError.TreatError(0, 0, "Initialization of event with file",tempos);
	} else {
		strcpy(actionFileSTR, "");
		fIsStrFilePresent = false;
	}

	EventInitWithFileName(actionFilePAR, actionFileSTR);

	fInit_event_done = true;

}
//______________________________________________________________________________
void GEventBase::EventInitWithFileNameBase(char* actionFilePAR, char* actionFileSTR) {
	fStatus = ACQ_OK;
	if (pDataArray) {
		delete pDataArray;
		pDataArray = NULL;
	}

	fDataArraySize = GetDataParameters()->FillFromActionFile(actionFilePAR);
	pDataArray = new uint16_t[fDataArraySize + 1]; // Data buffer is allocated
	ClearData(0);
	return;
}
//______________________________________________________________________________
void GEventBase::ClearData(int value) {
	//Clear Physical Data  Array or give a fix value ( default =0);

	Int_t i=0;
	if ( pDataArray==NULL) return;
	for (i = 0; i <= fDataArraySize; i++)
		pDataArray[i] = value;
}
//______________________________________________________________________________
bool GEventBase::EventInitAlready() {
	return fInit_event_done;
}
//______________________________________________________________________________
void GEventBase::SetEventInitAlready(bool setinit) {
	 fInit_event_done=setinit;
}
//______________________________________________________________________________

bool GEventBase::IsRaz() {
	if (fEventNumber == -1)		return true;
	else return false;
}
//______________________________________________________________________________
void GEventBase::DumpEvent(char mode) {
	cout << (GetDumpEvent(mode).Data());
}
//______________________________________________________________________________
void GEventBase::DumpArray(char mode, bool nozero) {
	cout << (GetDumpArray(mode, nozero)).Data();
}
//______________________________________________________________________________
void GEventBase::DumpHeader() {
	cout << (GetDumpHeader()).Data();
}
//_____________________________________________________________________________
void GEventBase::DumpEventRaw(int dumpsize, int increment) {
	TString tempos;
	cout << (GetDumpEventRaw(dumpsize, increment)).Data();
}
//______________________________________________________________________________
TString GEventBase::GetDumpEventRaw(int dumpsize, int increment) {
	// Method to dump event
	// if dumpsize =0 , dumpsize = standard = 256 and fGBbuf_increment raz

	if ((increment > 0) && (increment > fEventReadSize))
		fEvt_increment = increment;
	if (dumpsize>fEventReadSize)dumpsize =fEventReadSize;

	TString tempos;
	TString mydump;
	int n = 1;
	int i, k;
	int nbrcol = 8; // nb de colonnes affich�es

	int asciimin = 32; // range min of a char to be ascii character
	char tempo[20];
	int asciimax = 127; // idem but max
	int nbrperline = 16; // nbr of bytes per line
	int nbrline = 16; // nbr lines
	unsigned short *pShort;
	unsigned char *pChar;

	if ((increment > 0) && (increment <= fEventReadSize))
		fEvt_increment = increment;

	if (dumpsize == 0) {
		dumpsize = 256;
		fEvt_increment = 0;
	}

	nbrline = (int) (dumpsize / nbrperline) + (int) ((dumpsize % nbrperline)!= 0);
	pShort = (unsigned short *) (pEventBrut_char + fEvt_increment);
	pChar = (unsigned char *) (pEventBrut_char + fEvt_increment);

	if (fEvt_increment ==0){
		tempos.Form("Dump of event no : %d Size = %d (in char size)",fEventNumber,fEventReadSize);
		fError.Infos(tempos);
	}


	if (fEvt_increment <= fEventReadSize - nbrperline) {
		if (fEvt_increment == fEventReadSize)
			n = 1;
		else
			n = nbrline;
		for (i = 0; i < n; i++) {
			sprintf(tempo, "\n%5d %s ", fEvt_increment, ": ");
			mydump += tempo;
			for (k = 0; k < nbrcol; k++) {
				sprintf(tempo, "%04x  ", *pShort++);
				mydump += tempo;
			}
			mydump += "  ";
			for (k = 0; k < nbrperline; k++) {
				if ((*pChar >= asciimin) && (*pChar < asciimax)) {
					sprintf(tempo, "%c", *pChar);
					mydump += tempo;
				} else
					mydump += ".";
				pChar++;
			}
			fEvt_increment += nbrperline;
		}
		mydump += "\n";
	} else {
		mydump += "\n\t end of bloc \n";
	}

	return mydump;
}

//______________________________________________________________________________
TString GEventBase::GetDumpArray(char mode, bool nozero) {
	// Dump parameter Label and parameter value of the current event.
	// if enter parameter is true (default value), all zero parameter of event aren't dumped
	// mode = 'd' for decimal, 'b' for binary, 'h' for hexa, 'o' for octal
	Int_t i, j, maxbin, presentation = 0, max_presentation = 5;
	char tempo[MAX_CARACTERES*2];
	char Bin[MAX_CARACTERES];
	char Bin2[MAX_CARACTERES];
	TString stretrun;
	uint16_t value;
	int label;
	int reste;
	char one = '1', zero = '0';
	int DecNumber = 0;
	stretrun = "";
	if (mode == 'b')
		max_presentation = 3;

	if (fEventNumber == -1) {
		fError.TreatError(2, 0,
				"No Event , so no event dump. Get a new event buffer");
	} else {
		sprintf(
				tempo,
				"- Dumping Event, nb :%d --timestamps: %lld --Length = %d----------\n",
				fEventNumber, fTimeStamp, fDataArraySize);
		stretrun += tempo;
		for (i = 0; i < fDataArraySize; i++) {
			if (((int) pDataArray[i] != 0) || (nozero == false)) {
				label = 0;
				value = 0;
				value = (uint16_t) pDataArray[i]; // cast in (uint16_t) to be sur >0
				label = (uint16_t)(GetDataParameters()->GetLabel(i));
				switch (mode) {
				case 'd':// decimal display
					sprintf(tempo, "|%5d = %5d", label, value);
					break;
				case 'o':// octal display
					sprintf(tempo, "|%5o = %5o", label, value);
					break;
				case 'h':// hexa display
					sprintf(tempo, "|%5x = %5x", label, value);
					break;
				case 'b': // binary display
					DecNumber = (int) value;
					Bin[0] = '\0';
					Bin[1] = zero;
					Bin2[1] = '\0';
					Bin2[0] = zero;
					j = 1;
					if (DecNumber != 0) {
						while (DecNumber >= 1) {
							reste = DecNumber % 2;
							DecNumber -= reste;
							DecNumber /= 2;
							if (reste)
								Bin[j] = one;
							else
								Bin[j] = zero;
							j++;
						}
						j--;
						maxbin = j;
						while (j >= 0) {
							Bin2[j] = Bin[maxbin - j];
							j--;
						}
					}
					sprintf(tempo, "|%5d = %16s", label, Bin2);
					break;
				}
				stretrun += tempo;
				presentation++;
			}
			if (presentation == max_presentation) {
				stretrun += "|\n";
				presentation = 0;
			}
		}
		if (presentation != 0)
			stretrun += "|\n";
	}
	return stretrun;
}
//______________________________________________________________________________

////////////////////////////////////////fin /////////////////////////////////////
