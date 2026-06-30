// File : GDevice.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GDevice
//
// This class is a basic class to manage device (tape,file direcory, network...
//  in Ganil format
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
#include "GBuffer.h"

#include "GDevice.h"
#include "GEvent.h"

extern "C" {
#include "GEN_TYPE.H"
#include "STR_EVT.H"
#include "ERR_GAN.H"

#include "acq_codes_erreur.h"
}

//______________________________________________________________________________

ClassImp( GDevice);

GDevice::GDevice(void) {
	// Default constructor of device object;
	// equivalent to GDevice("noname")

	char Type_text[MAX_CARACTERES];

	strcpy(Type_text, "Tape");
	fBufferSize = BUFSIZE;
	fBuffer = NULL;
	Init();
	fPort = 0;

}

//______________________________________________________________________________

GDevice::GDevice(const char* _Name) {
	// Constructor/initialisator of device object
	// A "device" design  "something" which can containt run(s)
	// so it can be a directory , a file (in this case we have only on run)
	// or a tape
	// entry:
	// - Name , Name of device
	//


	fBufferSize = BUFSIZE;
	fBuffer = NULL;

	Init();
	GDevice::SetDevice(_Name);

}

//_____________________________________________________________________________

GDevice::~GDevice() {
	// destructor of GDevice object

	if (fVerbose > 5)
		cout << "\n Debug >Delete device " << endl;
	if (fBuffer) {

		delete (fBuffer);
		fBuffer = NULL;
	}
}
//_____________________________________________________________________________

void GDevice::Init() {
	// Called by every constructors.
	// Initialisation of main attributs
	//if (fBuffer) delete fBuffer;
	//fBuffer = new GBuffer(fBufferSize);

	fVerbose = 0;
	fType = GA_NOT_DEFINED;
	fWriteOrRead = false;
	fIsOpen = false;
	fBusy = false;

}
//______________________________________________________________________________

void GDevice::SetDevice(const char* Name1) {
	//close the device and
	//change name of device

	fStatus = ACQ_OK;
	if (fIsOpen == true)
		Close();
	strcpy(fName, Name1);
}
//______________________________________________________________________________

void GDevice::SetBufferSize(Int_t size) {
	//change the buffer size
	fBufferSize = size;
	if (fBuffer) {
		fBuffer->SetBufSize(size);

	}
}

//______________________________________________________________________________

void GDevice::SetPort(Int_t port) {
	//close the device and
	//change port number.( no action  in case of file)
	fStatus = ACQ_OK;
	if (fIsOpen == true)
		Close();
	fPort = port;
}
//______________________________________________________________________________

void GDevice::Infos() {
	// Give few informations about device : name , type ...;

	cout << "\n Device informations\n";
	cout << " Name                             : " << fName << "\n";
	cout << " Open ( 0 = no , 1 =Yes)          : " << fIsOpen << "\n";
	cout << " Type  1tape, 2file, 3dir,......) : " << GetType() << "\n";
	cout << " Buffer Size                      : " << fBuffer->GetBufSize()<< "\n";
	cout << " Status                           : " << fStatus << "\n";
}
//___________________________________________________________________________________

void GDevice::DumpBuffer(char* c) {
	// idem DumpBuffer(char c)
	// allows to take in account this kind of write : DumpBuffer("b") instead DumpBuffer('b');
	char b;
	b = c[0];
	DumpBuffer(b);
}
//___________________________________________________________________________________

void GDevice::ReadBuffer(GBuffer& _Buffer) {
	// Read a buffer ; it just calls the method 'GetBuffer' and copies it into the given buffer
	while (true) {
		ReadBuffer();
		if (fStatus != ACQ_NOCURRCTRLBUF)
			break;
	};
	if (fBuffer) {
		fBuffer->Equal(_Buffer);
		_Buffer.SetAttributs();
	}
}
//_____________________________________________________________________________

void GDevice::DumpBuffer(char c) {
	// Dump to console raw data  of a buffer
	// char =  'n' to dump next data of current dumped block
	//      =  'b' to dump next block (buffer) (in this case a new read of a buffer is done on device)

	bool contextopen = fIsOpen;

	TString tempo;
	fStatus = ACQ_OK;

	if (contextopen == false)
		Open('r');

	if (fIsOpen == false) {
		fError.TreatError(1, fStatus, "Device not opened");
		return;
	}

	if (GetType() == GT_TYPE_DIR) {
		fError.TreatError(1, fStatus, "No dump on a directory device :", fName);
		return;
	}

	switch (c) {
	case 'n':
		fStatus = fBuffer->DumpBuffer();
		break;
	case 'b':
		ReadBuffer();

		tempo.Form("---------------Dump buffer, nb : %d ---------------\n",
				fBuffer->fGBuf_index);
		cout << tempo.Data();
		if (fStatus != ACQ_OK)
			fError.TreatError(2, fStatus, "Read error");

		else
			fStatus = fBuffer->DumpBuffer(0);
		break;
	default:
		fError.TreatError(1, fStatus, " Invalid argument , must be 'n' or ' b'");
		break;
	}

	if (contextopen == false)
		Close();
}

////////////////////////////////////////fin /////////////////////////////////////

