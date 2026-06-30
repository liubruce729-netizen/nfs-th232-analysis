// File : GFile.C
// Author: Luc Legeard
////////////////////////////////////////////////////////////////////////////////
// Class GFile
//
// This class manage device (tape,file or directory) in Ganil format
// The associated methods can do copies,duplication, verification, dump
// This class  can bee compiled in a shared library witch can be used in root
// Example of use in root :
// root[0] .L GRU.so      // Load shared library GTtape
// root[1] .L GRU.h      // Load associated include file
// root[2]  GFile tape       // Declaration of a new object "tape" with defaut
//                           // attribute  default Name : DEFAULT_NAME
//  New device by default Name : DEFAULT_NAME
// root[3] tape.Open('r')  // Open device in read mode
// root[4] tape.DumpBuffer('b')  // Dump the first block on output
// root[5] tape.DumpBuffer('n')  // Dump the next data of block on output
// root[6] tape.Close()    // Close the opened tape*
//
// Also, You can run a excutable "GRU" witch include ROOT and this class
// with a  prompt "GRU>"
//
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

#include <stdio.h>
#include <string>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mtio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <dirent.h>
#include <TObject.h>
#include <TH1.h>
#include "General.h"
#include "GBufferIn2p3.h"
#include "GEvent.h"
#include "GFile.h"
#include <stdlib.h>
#include <TString.h>
#include "TSystemDirectory.h"
#include <TSystem.h> 
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <TString.h>
#include <zlib.h>
using namespace std;

extern "C" {
#include "GEN_TYPE.H"
#include "STR_EVT.H"
#include "ERR_GAN.H"

#include "gan_acq_buf.h"
#include "acq_codes_erreur.h"
#include "acq_tcp_struct.h"
#include "gan_acq_swap_buf.h"
}

//______________________________________________________________________________

ClassImp( GFile);

GFile::GFile(void) {
	// Default constructor of device object;
	// equivalent to GFile(DEFAULT_NAME,2000)

	TString tempos;
	char Type_text[MAX_CARACTERES];
	GFileInit("./");

	if (GetType() == GT_TYPE_TAPE)
		strcpy(Type_text, "Tape");
	if (GetType() == GT_TYPE_FILE)
		strcpy(Type_text, "File");
	if (GetType() == GT_TYPE_DIR)
		strcpy(Type_text, "Directory");
}

//______________________________________________________________________________

GFile::GFile(const char* _Name) :
	GDevice(_Name) {
	// Constructor/initialisator of device object
	// A "device" design  "something" which can containt run(s)
	// so it can be a directory , a file (in this case we have only on run)
	// or a tape
	// entry:
	// - Name , Name of file or name of device
	//

	// Examples :
	// >> GFile file = GFile(); // Default device
	// >> GFile file = GFile("/home/marcel/RUN1"); // File
	// >> GFile dire = GFile("/home/marcel/run/"); // Directory

	char Type_text[MAX_CARACTERES];
	TString tempos;
	strcpy(Type_text, "Tape");
	GFileInit(_Name);

	if (GetType() == GT_TYPE_FILE)
		strcpy(Type_text, "File");
	if (GetType() == GT_TYPE_DIR)
		strcpy(Type_text, "Directory");

}
//____________________________________________________________________________
GFile::~GFile() {
	// destructor of GFile object
	if (fVerbose > 5)
		fError.Infos("Delete device GFile");
}

//_____________________________________________________________________________

void GFile::GFileInit(const char *_Name) {
	// Called by every constructors.
	// Initialisation of main attributs

	SetCompressionLevel();
	fBufferHeadersize = 0;
	fType = GT_TYPE_TAPE;
	fWriteOrRead = false;
	fLun = 0;
	fLungz = NULL;

	SetDevice(_Name);
	fNewRunNumber = 0;
	// fIs_protected = false;

}

//______________________________________________________________________________

void GFile::SetDevice(const char* Name1) { // change name of device
	// also, change Type, GenericName in accordance
	// with this new name
	int nbchar = 0;
	fStatus = ACQ_OK;
	GDevice::SetDevice(Name1);
	WhatType();
	if (GetType() == GT_TYPE_DIR) {
		nbchar = strlen(fName);
		if (fName[nbchar - 1] != '/')
			strcat(fName, "/");
	}
}

//______________________________________________________________________________

void GFile::Infos() {
	// Give few informations about device : name , type ...;
	// No entry.
	TString tempos;
	long position = 0;

	WhatType();
	GDevice::Infos();

	GetPos(&position);

	if ((GetType() == GT_TYPE_DIR) || (GetType() == GT_TYPE_FILE)) {

		tempos.Form(" Run(s) Size                      : %d", SizeRuns());

		fError.Infos(tempos);

		if (GetType() == GT_TYPE_DIR)
			fError.Infos(" Type                             : Directory");
		if (GetType() == GT_TYPE_FILE)
			fError.Infos(" Type                             : File");

	}
	tempos.Form(" Compression                      : %d", fCompressionLevel);
	fError.Infos(tempos);
	tempos.Form(" WriteOrRead                      : %d", fWriteOrRead);
	fError.Infos(tempos);
	tempos.Form(" Lun ( Logical unit number)       : %d", (int) fLun);
	fError.Infos(tempos);
	tempos.Form(" Lun ( Logical unit number gz)    : %lld", (long long) fLungz);
	fError.Infos(tempos);
	tempos.Form(" Position                         : %ld blocks or %ld Kbytes",
			position, (position * ((long) fBuffer->GetBufSize() / 1024)));
	fError.Infos(tempos);

}

//___________________________________________________________________________________
void GFile::WhatType() {
	// Type Automatic Detection ( Tape, File or Directory).
	// Result is in Type attibut.
	// No entry.
	bool Context_IsOpen = fIsOpen;
	int LocalStatus = 0;
	char mode = 'r';
	struct stat bufstat; // necessary structure to test existing of a device ( using 'stat' fonction)
	fType = GA_NOT_DEFINED;

	LocalStatus = stat((this->fName), &bufstat);

	if (LocalStatus == -1) // test de l'existance du device
	{
		if (fVerbose > 1)
			fError.TreatError(1, LocalStatus, "Device doesn't exist!");
		fType = GT_TYPE_FILE; // positionnnement par defaut
	} else {
		if ((bufstat.st_mode & S_IFDIR))
			fType = GT_TYPE_DIR;
		if ((bufstat.st_mode & S_IFREG))
			fType = GT_TYPE_FILE;
		if (Context_IsOpen == false) {
			Open(mode);
		}
	}
	if (fType == GA_NOT_DEFINED) {
		IsTape();
		if (fStatus == ACQ_OK)
			fType = GT_TYPE_TAPE;
		else {
			if ((bufstat.st_mode & S_IFDIR))
				fType = GT_TYPE_DIR;
			else
				fType = GT_TYPE_FILE;
		}
		if (Context_IsOpen == false)
			Close();
	}

}

//______________________________________________________________________________
void GFile::Skip(bool quiet) {
	// Method to skip blocks or runs.
	// If the object is a file then then method is equivalent to Skip('b',1,quiet).
	// If the object is a tape the this mehod is equivalent to Skip ('f',1,quiet).
	// see : GTskip(char b,int nbr,bool quiet)
	if (GetType() == GT_TYPE_FILE) {
		Skip('b', 1, quiet);
	}
	if (GetType() == GT_TYPE_TAPE) {
		Skip('f', 1, quiet);
	}
	if (GetType() == GT_TYPE_DIR) {
		cout << "\n No possible skip with a Directory Device.\n";
	}
}
//________________________________________________________________________

void GFile::Skip(char* b, int nbr, bool quiet) {
	// idem GTskip (char b,int nbr)
	// allow to take in acccount  Skip("f", 2) instead Skip('f', 2)

	char c;
	c = b[0];
	Skip(c, nbr, quiet);
}

//________________________________________________________________________

void GFile::Skip(char b, int nbr, bool quiet) {
	// method;to skip blocks or runs .
	// usage : device.Skip (b, nbr)
	// - b is a charater = ( 'b','f','e','r')
	//  'b' to skip nbr blocks with a tape device or file device
	//  'f' to skip nbr files  with a tape
	//  'e' to go to the end of the tape device
	//  'r' to rewind the tape device
	// - nbr : number of skips  ( do not use for rewind and end of )
	// examples :
	// >> tape_read.Skip('f',5) , skip five runs on the tape device "tape_read"
	// >> file_read.Skip('e'),   go to end of file
	//
	// quiet = true => not standart message will be printed
	// default is false

	int fStatus = ACQ_OK;
	char c = 'r';
	bool opencontext = fIsOpen;

	if (GetType() == GT_TYPE_DIR) {
		cout << " No possible skip  with a Directory Device \n";
		return;
	}

	if (opencontext == false)
		Open(c);

	switch (b) {
	case 'B':
	case 'b':
		if (fVerbose > 1)
			cout << "\n Request to skip " << nbr << " block(s) on " << fName
					<< "\n";
		SkipBlock(nbr);
		switch (fStatus) {
		case ACQ_OK:
			if (fVerbose > 1)
				cout << "\n Skip " << nbr << " block(s)  Ok...\n";
			break;
		case ACQ_ENDOFFILE:
			cout << "\n End of file ..\n";
			break;
		default:
			fError.TreatError(2, fStatus, "skip block(s)");
			break;
		}
		break; // to skip blocks

	case 'F':
	case 'f':
		if (fVerbose > 1)
			cout << "\n Request to skip" << nbr << " file(s) on " << fName
					<< "\n";
		SkipFile(nbr);
		switch (fStatus) {
		case ACQ_OK:
			if (fVerbose > 1)
				cout << "\n Skip " << nbr << " file(s)  Ok...\n";
			break;
		case ACQ_ENDOFTAPE:
			if (!quiet)
				cout << "\n End of Tape ...\n";
			break;
		default:
			fError.TreatError(2, fStatus, "skip block(s)");
			break;
		}
		break; // to skip files

	case 'E':
	case 'e':
		int nbrfichier;
		if (!quiet)
			cout << "\n Request to skip to End Of ...\n ";
		SkipEO(&nbrfichier);
		switch (fStatus) {
		case ACQ_OK:
			cout << "\n End Of  Ok...\n";
			if (GetType() == GT_TYPE_TAPE)
				if (!quiet)
					cout << "\n " << nbrfichier << " files skipped \n";
			break;
		default:
			fError.TreatError(2, fStatus, "status EO");
			break;
		}
		break;

	case 'R':
	case 'r':
		if (!quiet)
			cout << "\n Request to rewind  ...... \n";
		SkipRewind();
		switch (fStatus) {

		case ACQ_OK:
			if (!quiet)
				cout << "\n Rewind OK ...\n";
			break;
		default:
			fError.TreatError(2, fStatus, "status EO");
			break;
		}
		break;
	default:
		cout << "\n You must specify skip type  (b,f,e or r)\n";
	}

	if (opencontext == false)
		Close();
}

//________________________________________________________________________

void GFile::Rewind(bool quiet) {
	// method to rewind a tape device or to place a at the begining of file in case of file device.
	// usage : device.Rewind()
	// equivalent to device.Skip('r');
	Skip('r', 0, quiet);
}
//__________________________________________________________________

void GFile::Open(char* mod) {
	// Idem Open(char mod)
	// allow to take in account this kind of write : Open("r") instead Open('r')
	char c;
	c = mod[0];
	Open(c);

}
//__________________________________________________________________
bool GFile::IsExiste() {

	if(fIsOpen) return true;
	else{
		struct stat sb;
		if (stat(fName, &sb) >= 0) {
			return true;
		}
	}
	return false;
}
//_________________________________________________________________

void GFile::Open(char mod) {
	// Open the device  in read  write, or new mode.
	// Input : mod (character)
	//         'r' Read mode
	//         'w' Write mode (file must already exist, and data will be overloaded)
	//         'a' Append mode
	//         'n' creation of new file and write, if the file is already exist
	//             then it the old file will be overloaded by a new one
	//
	// example :
	// >> tape.Open('r')

	int localstatus;
	int mode = 0;
	struct stat FileStat;
	TString tempos;

	if (fIsOpen == true) {
		if (fVerbose > 1)
			cout << "Debug 1>Device already opened ! \n";
		return;
	}

	if (GetType() == GT_TYPE_DIR) {
		if (fVerbose > 1)
			cout << "\nDebug 1>Device " << fName
					<< " is a directory can't be opened\n";
		fIsOpen = false;
		return;
	}
	fStatus = -1;

	switch (mod) {

	case 'n':
	case 'N':
		if ((localstatus = (stat(fName, &FileStat) >= 0)) && (GetType()
				== GT_TYPE_FILE))
			remove(fName);
		mode = O_CREAT | O_TRUNC | O_RDWR;
		MyOpen(fName, mode);

		fWriteOrRead = true;
		break;
	case 'w':
	case 'W':
		// Tester si le fichier existe sinon sortir en erreur
		mode = O_RDWR;
		MyOpen(fName, mode);
		fWriteOrRead = true;
		break;
	case 'a':
	case 'A':
		mode = O_APPEND;
		MyOpen(fName, mode);
		fWriteOrRead = true;
		break;
	case 'r':
	case 'R':
		if ((localstatus = (stat(fName, &FileStat) < 0)) && (GetType()
				== GT_TYPE_FILE)) {
			fError.TreatError(2, localstatus, "No file, so no opened file");
			return;
		}
		mode = O_RDONLY;

		MyOpen(fName, mode);

		break;

	default:

		fError.TreatError(
				2,
				0,
				"Option error,the letter must be 'a' to append, 'n' to create new file, 'w'to write or 'r' to read");
		return;
	}

	if ((fLungz != NULL ) or (fLun > 0))
		(fStatus = ACQ_OK);
	if (fStatus == ACQ_OK) {
		fIsOpen = true;
		switch (mod) {
		case 'n':
		case 'N':
		case 'w':
		case 'W':
		case 'a':
		case 'A':

			fWriteOrRead = true;
			if (fVerbose > 2)
				cout << "\nDebug 2>" << fName
						<< " is opened in write mode with Lun =" << fLungz
						<< ".\n";
			break;

		default:

			fWriteOrRead = false;
			if ((GetType() == GT_TYPE_FILE) || (GetType() == GT_TYPE_TAPE)) {
				ReadBuffer();
				if (fStatus == ACQ_BUFSIZNOTGOO) {
					fStatus = ACQ_OK;
					int oldsize = GetCurrentBuffer()->GetBufSize();
					int newsize = GetCurrentBuffer()->GetReadSize();
					SetBufferSize(newsize);
					tempos.Form("Buffer size change from %d to %d", oldsize,
							GetCurrentBuffer()->GetBufSize());

					fError.Infos(tempos);
					Rewind();
					ReadBuffer();
					if (fStatus != ACQ_OK)
						fError.TreatError(2, fStatus,
								"Impossible to set buffer size");
				}

				ReadBuffer();
				if (fBuffer->GetType() == EVENTH_Idn) {
					strcpy(fDate, GetCurrentBuffer()->GetDate_());
					fRunNumber = GetCurrentBuffer()->GetNumRun_();
				}
			}

			if (fVerbose > 2)
				cout << "\nDebug 2>" << fName
						<< " is opened in read mode with Lun =" << fLun
						<< ".\n";
			break;
		}
	} else {
		fError.TreatError(2, fStatus, "Device not opened ", fName);

	}

}
//_______________________________________________________________________________

void GFile::Close() {
	// Close the opened device.
	// If the device is already closed => no effect

	int LocalStatus;
	struct stat bufstat; // necessary structure to test existing of a device ( using 'stat' fonction )

	if (fIsOpen == false) {
		// if (GetType()!=GT_TYPE_DIR)  cout << "\n Device already closed !\n";// A directory is always closed !
		return;
	}

	LocalStatus = stat(fName, &bufstat);
	if (LocalStatus == -1) // test de l'existance du device
	{
		fError.TreatError(2, LocalStatus, fName,
				"device doesn't existe! So we supose closed");
		fIsOpen = false;
		return;
	}

	if (GetType() == GT_TYPE_DIR) {
		if (fVerbose > 1)
			cout << "\nDebug 1>Device is a directory " << fName
					<< ", no opening and no closing\n";
		fIsOpen = false;
		return;
	}

	fStatus = MyClose();

	if (fStatus < 0)
		fStatus = ACQ_ERRPARAM;
	else
		fStatus = ACQ_OK;

	if (fStatus == ACQ_OK) {
		if (fVerbose > 1)
			cout << "\n The Device " << fName << " is closed.\n";
		fIsOpen = false;
		fWriteOrRead = false;
	} else
		cout << "\n Status = " << fStatus << " Closing of Device " << fName
				<< ".\n";
}

//______________________________________________________________________________
void GFile::WriteBuffer(GBuffer& _Buffer) {
	WriteBuffer(_Buffer);
}
//______________________________________________________________________________
void GFile::WriteBuffer(GBuffer* _Buffer) {
	// write a buffer designed by _Buffer to the device

	int LocalStatus = ACQ_OK;
	int counter = 0;
	fStatus = ACQ_OK;

	if (((fWriteOrRead == true) && (fIsOpen == true))) {

		LocalStatus = MyWrite(_Buffer->fGBuf_data, _Buffer->GetBufSize());
		if (LocalStatus > 0)
			fStatus = ACQ_OK;
		else if (LocalStatus == 0)
			fStatus = ACQ_ENDOFFILE;
		else
			fStatus = ACQ_ERRPARAM;
		counter++;
		if (fStatus == ACQ_OK) {
			if (counter == 40) {
				cout << "\n";
				counter = 0;
			}
			if (fVerbose > 5)
				cout << "-";
		} else {
			fError.TreatError(3, fStatus, "Write Error");

		}
	} else {
		// B. Raine 7 juin 2005
		fStatus = ACQ_ERRPARAM;
		TString toto;
		toto = "Problem in write mode. WriteOrRead =";
		toto += fWriteOrRead;
		toto += "and  IsOpen = ";
		toto += fIsOpen;
		fError.TreatError(3, fStatus, toto);

	}
}
//________________________________________________________________

void GFile::WriteBuffer() {
	// Write his internal buffer on device

	int LocalStatus = ACQ_OK;
	fStatus = ACQ_OK;
	// B. Raine 7 juin 2005

	if ((fWriteOrRead == true) && (fIsOpen == true)) {
		LocalStatus = MyWrite(fBuffer->fGBuf_data, fBuffer->GetBufSize());
		if (LocalStatus > 0)
			fStatus = ACQ_OK;
		else if (LocalStatus == 0)
			fStatus = ACQ_ENDOFFILE;
		else
			fStatus = ACQ_ERRPARAM;
		if (fVerbose > 5)
			cout << "-";
		if (fStatus != ACQ_OK) {
			fError.TreatError(3, fStatus, "Write Error");
		}
	} else {

		fStatus = ACQ_ERRPARAM;
		TString toto;
		toto = "Problem in write mode. WriteOrRead = ";
		toto += fWriteOrRead;
		toto += "and  IsOpen = ";
		toto += "fIsOpen";
		fError.TreatError(3, fStatus, toto);
	}
}

//____________________________________________________________________________

void GFile::ReadBuffer() {
	// read on GFile objet the curent buffer on device and fill the internal buffer
	// device object.
	if (!fBuffer)
		fError.TreatError(2, 0, "GFile::ReadBuffer() : Buffer doesn't exist");

	int Size = fBuffer->GetBufSize();
	fStatus = ACQ_OK;
	if (fIsOpen == true) {
		fBuffer->fGBuf_type = UNKNOWN_Idn;
		if (Size > 0) {
			Size = MyRead(fBuffer->fGBuf_data, Size);/* Pour retourner nombre de caracteres lus */
			if (Size <= 0) {
				if (Size == 0)
					fStatus = ACQ_ENDOFFILE;
				else
					fStatus = ACQ_ERRDURREAD;
				strcpy(((GBufferIn2p3*) fBuffer)->fGBuf_header, "        ");
				fBuffer->fGBuf_type = UNKNOWN_Idn;
			}
		} else {
			fStatus = ACQ_ERRPARAM;
			strcpy(((GBufferIn2p3*) fBuffer)->fGBuf_header, "        ");
			fBuffer->fGBuf_type = UNKNOWN_Idn;
		}

		fBuffer->fGBuf_increment = 0;
		if (fStatus != ACQ_OK) {
			fBuffer->fGBuf_index = 0;
		} else {
			((GBufferIn2p3*) fBuffer)->SetAttributs();
		}
		((GBufferIn2p3*) fBuffer)->fGBuf_header[IN2P3_HEADER_SIZE] = '\0';
	} else {
		fError.TreatError(3, fStatus, "Read device is not open");
		strcpy(((GBufferIn2p3*) fBuffer)->fGBuf_header, "        ");
		fBuffer->fGBuf_type = UNKNOWN_Idn;

	}
	if (fBuffer->fGBuf_type == EVENTH_Idn) {
		fRunNumber = fBuffer->GetNumRun();
		strcpy(fDate, fBuffer->GetDate());
	}
	if (fBuffer->fGBuf_type == FILEH_Idn) {
		if (Size != fBuffer->GetReadSize()) {
			fStatus = ACQ_BUFSIZNOTGOO;
		}
	}
	//cout << "Debug ReadBuffer typeread = "  << fBuffer->fGBuf_type << "  Status = "<<hex<<fStatus<<dec<< "\n";
}
//_____________________________________________________________________________
void GFile::ReadBuffer(GBuffer& _Buffer) {
	ReadBuffer(&_Buffer);
}

//_____________________________________________________________________________
void GFile::ReadBuffer(GBuffer* _Buffer) {
	// read on GFile objet the curent buffer on device and fill "_Buffer"
	// the internal buffer device  is also filled
	// input : Pointer on Buffer to fill during reading

	ReadBuffer();
	fBuffer->Equal(_Buffer); // copy Buffer to _Buffer
}

//____________________________________________________________________________________

bool GFile::IsARun(bool quiet) {
	// test if the device is a run file in IN2P3 format
	// if quiet = true standard  message are not displayed ( defaut = true)
	bool result = false;
	bool opencontext;
	opencontext = fIsOpen;
	WhatType();
	if (!(GetType() == GT_TYPE_FILE)) {
		return (result);
	}
	if (opencontext == false)
		Open('r'); //open origin device
	Rewind(true);
	ReadBuffer();
	if (fBuffer->fGBuf_type == FILEH_Idn)
		result = true;
	if (opencontext == false)
		Close();
	return (result);
}
//____________________________________________________________________________________

bool GFile::IsARootFile(bool quiet) {
	// test if the device is a run file in IN2P3 format
	// if quiet = true standard  message are not displayed ( defaut = true)
	bool result = false;

	int msize, j;
	msize = strlen(fName);

	WhatType();
	if (!(GetType() == GT_TYPE_FILE)) {
		return (result);
	}
	j = 0;
	msize = strlen(fName);
	j = msize - 1;
	if ((fName[j] == 't') && (fName[--j] == 'o') && (fName[--j] == 'o')
			&& (fName[--j] == 'r') && (fName[--j] == '.'))
		result = true;
	return (result);
}
//____________________________________________________________________________________
void GFile::ExtractHeaders(char* fileoutname) {
	GFile fileout(fileoutname);
	fileout.Close();
	fileout.SetBufferSize(GetBufferSize());
	fileout.SetCompressionLevel(0);
	fileout.Open('n');
	Open();
	Rewind();
	while (true) {
		ReadBuffer();
		if ((GetCurrentBuffer()->IsAEventBuffer())
				or (GetCurrentBuffer()->GetType() == SCALER_Idn))
			break;
		fileout.WriteBuffer(GetCurrentBuffer());
	}
	fileout.Close();
	Close();
}
//________________________________________________________________________________________________
int GFile::SizeRuns() {
	//return size of a Ganil run file or a total sum of runs in a directory.
	//else return 0;

	char input_dir[256];
	char input_file[256]; //input file name
	char input_file_wp[512]; //input file name with path
	strcpy(input_dir, fName);
	struct stat bufstat;
	int size = 0;

	if (GetType() == GT_TYPE_DIR) {
		int i;
		i = 0;
		TSystemDirectory inputdir("mydir", fName);
		TList* listfile;
		listfile = inputdir.GetListOfFiles();
		listfile->Sort(1);
		int size = 0;

		while (listfile->At(i)) {
			strcpy(input_file, (char*) (listfile->At(i++)->GetName()));
			sprintf(input_file_wp, "%s/%s", input_dir, input_file);
			SetDevice(input_file_wp);
			if (IsARun(true)) {
				stat(fName, &bufstat);
				size += (int) bufstat.st_size;
			}
		}
		SetDevice(input_dir);
		return (size);
	}

	if (GetType() == GT_TYPE_FILE) {

		//if (IsARun(true))  //  IsARun, do a rewind so make error in write/read mode
		{
			stat(fName, &bufstat);
			size = (int) bufstat.st_size;
		}

		return (size);
	}

	return (0);

}
//________________________________________________________________________________________________


void GFile::Dir() {
	// List all the run of device tape
	// or list all the files  is cas of directory device or file device

	char c = 'r';
	bool opencontext = fIsOpen;

	int Run;
	char Filename_tempo[MAX_CARACTERES*2];
	bool endoftape = false; // Bollean to design the end of tape
	int Counter_files = 0;
	long position = 0; // position on tape
	long diff_position = 0; // difference of position beetween a run
	long rate = 0;
	int LocalStatus = 0;

	fStatus = ACQ_OK;

	if (fBusy == true) {
		fError.TreatError(1, 1, "Device is  busy. Dir canceled!");
		return;
	}

	struct stat bufstat; // necessary structure to test existing of a device ( using 'stat' fonction)
	LocalStatus = stat((this->fName), &bufstat);
	if (LocalStatus == -1) // test de l'existance du device
	{
		fError.TreatError(1, LocalStatus, "Device doesn't exist!");
		return;
	}

	SetBusy(true);

	cout << "\n Dir DevName = " << fName << " :\n";
	if (GetType() == GT_TYPE_TAPE) {
		Skip('r', 0);
		if (opencontext == false)
			Open(c);
		GetPos(&position);
		cout << "--->Position in blocks on tape : " << position << "\n";
		//  loop to files //
		do {
			Counter_files++;
			ReadBuffer();
			if (fStatus != ACQ_OK) {
				endoftape = true; //
				if (fStatus != ACQ_ENDOFFILE)
					fError.TreatError(2, fStatus,
							"Error in read first block of your file!");
			} else {
				// read the second bloc to have run name and date
				ReadBuffer();
				if (fStatus != ACQ_OK) {
					endoftape = true;
					if (fStatus != ACQ_ENDOFFILE)
						fError.TreatError(2, fStatus,
								"Error in read second block of your file!");

				} else {
					if (fBuffer->fGBuf_type == EVENTH_Idn) {
						Run = fBuffer->GetNumRun();
						strcpy(fDate, fBuffer->GetDate());
						sprintf(Filename_tempo, " Run %d : %s", Run, fDate);
						if (fVerbose > 5) {
							fBuffer->DumpBuffer(0);
						}
					} // endif if (strcmp....
					else
						sprintf(
								Filename_tempo,
								" By default : Run %d.dat ...but may be not a Ganil Run",
								(Counter_files));
				}// end if fStatus==...else
				cout << Filename_tempo << " \n";

				Skip('f', 1);
				GetPos(&position);
				rate = (long) (fBuffer->GetBufSize() / 1024);
				rate *= (long) (position - diff_position - 1);
				cout << "--->Tape position " << position
						<< " blocks. \t Run space = " << rate << " Kbytes\n";
				diff_position = position;
			}// end if fStatus==.. else...
		} while (endoftape != true);

		Counter_files--;
		sprintf(Filename_tempo, "\n Total of file  Counter_files %d \n",
				Counter_files);
		cout << " \n";
		if (opencontext == false)
			Close();
	} // end if Type ==...
	else {
		char tempo1[MAX_CARACTERES];
		int nbr1, nbr2;
		struct dirent *lecture;

		fStatus = ACQ_OK;
		DIR *rep;
		strcpy(tempo1, fName);
		nbr1 = strlen(tempo1);
		nbr2 = strlen(strrchr(tempo1, '/'));
		tempo1[nbr1 - nbr2] = '\0';// end of string in place of last '/'

		rep = opendir(tempo1);
		while ((lecture = readdir(rep))) {
			cout << lecture->d_name << "\t";
		}
		closedir(rep);
		cout << "\n";
	}
	SetBusy(false);
}
//________________________________________________________________________________________________

void GFile::Inquire(char *Exp_Name) {
	// read the two first blocks and display it and read  informations
	// from a file or from  from a tape
	// infomations are (buffer size, date...)
	// If Exp_name is not empty (!="") the ACTIONS_Exp_Name.CHC_STR (structure file) and
	// ACTIONS_Exp_Name.CHC_PAR (parameter file) created

	char c = 'r';
	bool opencontext = fIsOpen;
	GBufferIn2p3 * Buffer1; // buffer tempo for first buffer read
	GBufferIn2p3 * Buffer2; // buffer tempo for second buffer read
	char Tempo[fBuffer->GetBufSize()];
	char Filename[MAX_CARACTERES];
	char Tempo11[MAX_CARACTERES];
	char Tempo12[MAX_CARACTERES];
	char Tempo21[MAX_CARACTERES];
	int Nb_struct_bloc = 0; // nb  comment buffer  used to define structure
	int Struct_length; // size of structure inside comment buffer
	int Decompte = 0, Depart, Boucle, Debut;
	int Lun_tempo;
	TString tempoString;
	Lun_tempo = -1;
	TString HomeDir = gSystem->HomeDirectory();

	if (opencontext == false)
		Open(c);

	if (GetType() == GT_TYPE_DIR) {
		fError.TreatError(2, 0, "Impossible to inquire a Directory");
		return;
	}

	if (fIsOpen == false) {
		fError.TreatError(1, 0, "Device not opened");
		return;
	}

	if (fVerbose > 1)
		fError.Infos("Device not opened");

	//so getting the 2 first buffers in Buffer1 and Buffer 2
	//get information from second buffer and compose destination name
	//and write them to destination device


	if ((fStatus != ACQ_OK) && (fStatus != ACQ_ENDOFFILE)) {
		fError.TreatError(3, fStatus, "Error inquire!");
	}

	// Look at Buffer size on device and warning if we get a difference default value;
	Rewind();
	ReadBuffer();
	if (fStatus == ACQ_BUFSIZNOTGOO) {
		TString toto;
		toto.Form(
				"Buffer size of device  is equal to default value. \n    Change from %d to %d ",
				fBuffer->GetBufSize(), fBuffer->GetReadSize());
		fError.TreatError(3, 1, toto);
	}
//cout<<  " debub fBuffer->GetBufSize()= "<< fBuffer->GetBufSize()<< "  fBufferSize = "<< fBufferSize<<"\n";
	Buffer1 = new GBufferIn2p3(fBufferSize);
	Buffer2 = new GBufferIn2p3(fBufferSize);

	fBuffer->Equal(Buffer1);
	if (fStatus == ACQ_OK) {
		ReadBuffer();

		if ((fStatus != ACQ_OK) && (fStatus != ACQ_ENDOFFILE)) {
			fError.TreatError(3, fStatus, "Error inquire(read)!");
		}
		fBuffer->Equal(Buffer2);
	}

	if ((Buffer2->fGBuf_type == EVENTH_Idn) && (Buffer1->fGBuf_type
			== FILEH_Idn)) {
		strncpy(
				Tempo,
				Buffer2->GetBuffer_map_in2p3()->les_donnees.cas.Buf_eventh.NbBlocStrEvt,
				4);
		Tempo[4] = '\0';
		Nb_struct_bloc = atoi(Tempo);
		memcpy(Tempo11,
				Buffer1->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii, 63);
		memcpy(Tempo12,
				(Buffer1->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii + 63
						+ 17), 31);
		memcpy(Tempo21,
				Buffer2->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii, 78);
		Tempo11[63] = '\0';
		Tempo12[31] = '\0';
		Tempo21[78] = '\0';
		cout << "\n First buffer : Header : " << Buffer1->fGBuf_header << " \n";
		cout << Tempo11 << " \n";
		cout << Tempo12 << " \n";
		cout << "\n Second Buffer : Header : " << Buffer2->fGBuf_header
				<< " \n";
		cout << Tempo21 << " \n";
	} else {
		fError.TreatError(
				2,
				fStatus,
				"Error in getting infomation in  your file!\nMay be no Ganil format run !\n or your are not a the begining of a run (try a Skip('r') before))\n");
	}

	// Read of bloc COMMENT and write file ACTIONS_EXPNAME.CHC_STR and ACTIONS_EXPNAME.CHC_PAR
	if (strcmp(Exp_Name, "") != 0) {

		//  Open of a file ACTIONS_EXPNAME.CHC_STR

		sprintf(Filename, "ACTIONS_%s.CHC_STR", Exp_Name);

		tempoString.Form(" Write of %s file :", Filename);
		fError.TreatError(0, 0, tempoString);

		fStatus = open(Filename, O_RDWR | O_CREAT | O_TRUNC, 0664);

		if (fStatus == -1) { // Open error  */
			tempoString.Form("%s/%s", HomeDir.Data(), Filename);
			strcpy(Filename, tempoString.Data());
			fStatus = open(Filename, O_RDWR | O_CREAT | O_TRUNC, 0664);
			if (fStatus == -1) {// Open error  */
				fStatus = ACQ_STREVTTMP;
				fError.TreatError(2, fStatus, "Open error");
				return;
			}
		}
		Lun_tempo = fStatus;

		// good open of the file
		// Now we 'll read all blocks in  " COMMENT" buffer
		Decompte = Nb_struct_bloc;
		while (Decompte) {
			ReadBuffer();

			if ((fStatus != ACQ_OK) && (fStatus != ACQ_ENDOFFILE)) {

				fError.TreatError(2, fStatus, "Inquire !(read");
			}
			// Now , we get header
			if (strncmp(
					((GBufferIn2p3*) fBuffer)->GetBuffer_map_in2p3()->les_donnees.Ident,
					COMMENT_Id, 8) != 0) {
				fStatus = ACQ_BADFILESTRUCT;
				close(Lun_tempo);
				return;
			}

			// Recuperation de la taille de la structure.
			strncpy(
					Tempo,
					((GBufferIn2p3*) fBuffer)->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii,
					4);
			Tempo[4] = '\0';
			if (strcmp(Tempo, "****") == 0) {
				fStatus = ACQ_BADEVTSTRUCT; // Structure illisible.
				close(Lun_tempo);
				return;
			} else
				Struct_length = atoi(Tempo);
			// On va maintenant ecrire les 'Valeur' caracteres qui suivent
			// dans le fichier temporaire.
			Depart = 4;
			Debut = Depart;
			Boucle = Depart;
			while (Boucle++ < Struct_length + Depart) {
				// On remplace le code 0x0D par le code fin de ligne
				// et les 0x00 par ' '
				//   du systeme courant */

				if (((GBufferIn2p3*) fBuffer)->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii[Boucle]
						== 0x0d) {
					((GBufferIn2p3*) fBuffer)->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii[Boucle]
							= '\n';
				}
				if (((GBufferIn2p3*) fBuffer)->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii[Boucle]
						== 0x00) {
					((GBufferIn2p3*) fBuffer)->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii[Boucle]
							= ' ';
				}
			}// While(Boucle)
			// On ecrit tout en une seule fois */
			((GBufferIn2p3*) fBuffer)->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii[Debut
					+ Struct_length] = '\0';
			cout
					<< &((GBufferIn2p3*) fBuffer)->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii[Debut]
					<< "\n";
			fStatus
					= write(
							Lun_tempo,
							&((GBufferIn2p3*) fBuffer)->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii[Debut],
							Struct_length);
			if (fStatus == -1) {
				close(Lun_tempo);
				fStatus = ACQ_STREVTTMP;
				return;
			}
			Decompte--;
		} // While (decompte)
		close(Lun_tempo);
		fStatus = ACQ_OK;

		//  Open of a file ACTIONS_EXPNAME.CHC_PAR
		sprintf(Filename, "ACTIONS_%s.CHC_PAR", Exp_Name);

		tempoString.Form(" Write of %s file :", Filename);
		fError.TreatError(0, 0, tempoString);
		fStatus = open(Filename, O_RDWR | O_CREAT | O_TRUNC, 0664);
		if (fStatus == -1) {
			tempoString.Form("%s/%s", HomeDir.Data(), Filename);
			strcpy(Filename, tempoString.Data());
			fStatus = open(Filename, O_RDWR | O_CREAT | O_TRUNC, 0664);
			if (fStatus == -1) {// Open error  */
				fStatus = ACQ_STREVTTMP;
				fError.TreatError(2, fStatus, "Open ", Filename);
				return;
			}
		}
		Lun_tempo = fStatus;

		// good open of the file

		Decompte = 1;

		while (Decompte > 0) {

			ReadBuffer();

			if ((fStatus != ACQ_OK) && (fStatus != ACQ_ENDOFFILE)) {
				close(Lun_tempo);
				fError.TreatError(2, fStatus, "Inquire !(read)");
				return;
			}

			// Now , we get header
			if (strncmp(
					((GBufferIn2p3*) fBuffer)->GetBuffer_map_in2p3()->les_donnees.Ident,
					PARAM_Id, 8) != 0) {
				fStatus = ACQ_BADFILESTRUCT;
				close(Lun_tempo);

				return;
			}

			// get  Param Info
			Boucle = 0;
			c = 'a';
			// separator inside one par is ','
			// separator beetwen parameter is '\r' = 0x0d
			// final end of list of parameter is '!!!!'
			// end of liste for a buffer ( so we ahave to read the next PARAM block)
			while ((c != ' ') && (c != '!')) {
				// we replace  ',' by ' ' and  '.' by  '\n'
				c
						= (((GBufferIn2p3*) fBuffer)->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii[Boucle]);
				if (c == 0x0d) {
					((GBufferIn2p3*) fBuffer)->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii[Boucle]
							= '\n';
				}
				if (c == ',') {
					((GBufferIn2p3*) fBuffer)->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii[Boucle]
							= ' ';
				}
				Boucle++;
				if (Boucle >= (((GBufferIn2p3*) fBuffer)->GetBufSize()
						- (IN2P3_HEADER_SIZE))) {
					close(Lun_tempo);
					fStatus = ACQ_UNKBUF;
					fError.TreatError(2, fStatus,
							"in read of PARAM buffer !(read)");
					return;
				}
			} // end of While (c!=...
			// On ecrit tout en une seule fois
			Boucle--;
			((GBufferIn2p3*) fBuffer)->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii[Boucle]
					= '\0';
			fError.Infos(
					" Parameters list : \n",
					(char*) (&((GBufferIn2p3*) fBuffer)->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii[0]));

			fStatus
					= write(
							Lun_tempo,
							&((GBufferIn2p3*) fBuffer)->GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii[0],
							Boucle);
			if (fStatus == -1) {
				close(Lun_tempo);
				fStatus = ACQ_STREVTTMP;
				fError.TreatError(2, 0, "Close of Parameters file on error");
				return;
			}

			if (c == '!')
				Decompte--;

		}// end of  while (Decompte>...

		close(Lun_tempo);
		fStatus = ACQ_OK;
	}// end of if ( strcmp (Exp_Name, "" ....
	if (opencontext == false)
		Close();

}

//_______________________________________________________________________________


void GFile::SkipBlock(int NombreSkip) {

	struct mtop Control;
	fStatus = ACQ_OK;

	switch (GetType()) {
	case GT_TYPE_DIR:
		fError.TreatError(1, fStatus,
				" Not possible to skip with a Directory device.");
		fStatus = ACQ_ERRPARAM;
		break;
	case GT_TYPE_FILE:
		if (fLungz != NULL )
			fStatus = gzseek(fLungz, fBuffer->GetBufSize() * NombreSkip,
					SEEK_CUR);
		if (fLun > 0)
			fStatus = lseek(fLun, fBuffer->GetBufSize() * NombreSkip, SEEK_CUR);
		if (fStatus < 0)
			fStatus = ACQ_ERRPARAM;
		else
			fStatus = ACQ_OK;
		break;
	case GT_TYPE_TAPE:
		if (NombreSkip < 0) {
			Control.mt_count = -NombreSkip;
			Control.mt_op = MTBSR;
		} else {
			Control.mt_count = NombreSkip;
			Control.mt_op = MTFSR;
		}

		fStatus = ioctl(fLun, MTIOCTOP, &Control);
		if (fStatus != -1)
			fStatus = ACQ_OK;
		else
			fStatus = ACQ_ENDOFFILE;
		break;
	default:
		fStatus = ACQ_UNKNOWNDEV;
		cout << " Not possible to skip with  a unknown type Device\n";
		break;
	}
}

//_______________________________________________________________________________

void GFile::SkipFile(int NombreSkip) {
	// skip NombreSkip files

	struct mtop Control;
	fStatus = ACQ_OK;

	switch (GetType()) {
	case GT_TYPE_DIR:
		fStatus = ACQ_ERRPARAM;
		fError.TreatError(1, fStatus,
				" Not possible to skip with a Directory device");
		break;
	case GT_TYPE_FILE:
		fStatus = ACQ_ERRPARAM;
		fError.TreatError(1, fStatus,
				" Not possible to skip file with a File device");
		break;
	case GT_TYPE_TAPE:
		if (NombreSkip < 0) {
			Control.mt_count = -NombreSkip;
			Control.mt_op = MTBSF;
		} else {
			Control.mt_count = NombreSkip;
			Control.mt_op = MTFSF;
		}

		fStatus = ioctl(fLun, MTIOCTOP, &Control);
		if (fStatus != -1)
			fStatus = ACQ_OK;
		else
			fStatus = ACQ_ENDOFTAPE;
		break;
	default:
		fStatus = ACQ_UNKNOWNDEV;
		cout << " Not possible to skip with a unknown  device\n";
		break;
	}
}
//_______________________________________________________________________________

void GFile::SkipEO(int *NombreSkip) {
	// go to the end of
	//                 - Tape in case of tape device
	//                 - File in case of  file device

	struct mtop Control;
	int NbrTempo = 0;
	*NombreSkip = 0;
	fStatus = ACQ_OK;

	switch (GetType()) {

	case GT_TYPE_DIR:
		fStatus = ACQ_ERRPARAM;
		fError.TreatError(1, fStatus,
				"Not possible to skip with a Directory device");
		break;
	case GT_TYPE_TAPE:
		Control.mt_count = 1;
		Control.mt_op = MTFSF;
		while (fStatus != -1) { // Atteint la fin de bande

			fStatus = ioctl(fLun, MTIOCTOP, &Control);
			if (fStatus != -1)
				NbrTempo++;
		}
		NbrTempo--; // Pour supprimer le double 'Tape Mark' de fin
		fStatus = ACQ_OK;
		*NombreSkip = NbrTempo;
		break;
	case GT_TYPE_FILE:
		if (fLungz != NULL)
			fStatus = gzseek(fLungz, 0, SEEK_END);
		if (fLun > 0)
			fStatus = lseek(fLun, 0, SEEK_END);

		if (fStatus < 0) {
			fStatus = ACQ_ERRPARAM;
			fError.TreatError(2, fStatus, "Lseek problem!");
		} else
			fStatus = ACQ_OK;
		break;
	default:
		fStatus = ACQ_UNKNOWNDEV;
		cout << " Not possible to skip with a unknown  device\n";
		fError.TreatError(1, fStatus,
				"Not possible to skip with a unknown  device!");
		break;
	}
}

//_______________________________________________________________________________

void GFile::SkipRewind() {
	//Rewind a tape device or file device

	struct mtop Control;

	switch (GetType()) {

	case GT_TYPE_DIR:
		fError.TreatError(1, 0, " Not possible to skip with a Directory device");
		break;
	case GT_TYPE_TAPE:
		Control.mt_op = MTREW;
		Control.mt_count = 1; /* Nombre d'actions a effectuer   */

		fStatus = ioctl(fLun, MTIOCTOP, &Control);
		if (fStatus != -1)
			fStatus = ACQ_OK;
		break;
	case GT_TYPE_MFMFILE:
	case GT_TYPE_FILE:
		if (fLungz !=NULL )
			fStatus = gzseek(fLungz, 0, SEEK_SET);
		if (fLun > 0)
			fStatus = lseek(fLun, 0, SEEK_SET);

		if (fStatus < 0) {
			fStatus = ACQ_ERRPARAM;
			fError.TreatError(1, fStatus, "Lseek problem!");
		} else
			fStatus = ACQ_OK;
		break;
	default:
		fStatus = ACQ_UNKNOWNDEV;
		fError.TreatError(1, fStatus,
				"Not possible to skip with a unknown  device");
		break;
	}
}
//_______________________________________________________________________________
int GFile::MyRead(char * ptchar, int size) {
	int rsize = 0;
	if (fLun > 0)
		rsize = read(fLun, ptchar, size);
	if (fLungz !=NULL)
		rsize = gzread(fLungz, ptchar, size);
	return rsize;
}
//_______________________________________________________________________________
int GFile::MyWrite(char * ptchar, int size) {

	int rsize = 0;
	if (fLun > 0)
		rsize = write(fLun, ptchar, size);
	if (fLungz !=NULL)
		rsize = gzwrite(fLungz, ptchar, size);
	return rsize;
}
//_______________________________________________________________________________
long long GFile::MyOpen(char * name, int mode) {
	long long status = -1;
	char chmod[10];
	strcpy(chmod, "rb");
	if (mode == (O_CREAT | O_TRUNC | O_RDWR)) {
		strcpy(chmod, "");
		sprintf(chmod, "wb%d", fCompressionLevel);
	}

	if (mode == O_RDWR) {
		strcpy(chmod, "");
		sprintf(chmod, "wb%d", fCompressionLevel);
	}
	if (mode == O_APPEND) {
		strcpy(chmod, "ab");
		strcpy(chmod, "");
		sprintf(chmod, "ab%d", fCompressionLevel);
	}
	if (mode == O_RDONLY)
		strcpy(chmod, "rb");
	if ((GetType() == GT_TYPE_TAPE) or (fCompressionLevel == 0)) {
		fLun = open(name, mode, 0664);
		status = (long long) fLun;
		fLungz = NULL;
	} else {
		fLungz = gzopen(name, chmod);
		status = (long long) fLungz;
		fLun = 0;
	}
	return status;
}
//_______________________________________________________________________________
long long GFile::MyClose() {
	long long status = 0;
	if (fLun > 0)
		status = (long long) (close(fLun));
	if (fLungz !=NULL)
		status = gzclose(fLungz);
	return status;
}
//_______________________________________________________________________________

void GFile::IsTape() {
	// Is a tape  or not .
	// fStatus attribut return information

	struct mtget Mtget;

	fStatus = ioctl(fLun, MTIOCGET, &Mtget);

	if (fStatus != -1) {
		if (Mtget.mt_type != 0) {
			fStatus = ACQ_OK;
		} else
			fStatus = ACQ_ISNOTATAPE;
	} else
		fStatus = ACQ_ERRPARAM;

}
//_______________________________________________________________________________

void GFile::Eject() {
	// eject tape
	// fStatus attribut return ACQ_OK or ACK_ERR

	struct mtop Control;
	fStatus = ACQ_OK;
	switch (GetType()) {
	case GT_TYPE_TAPE:
		Control.mt_op = MTOFFL; // rewind + offline ( eject?)
		Control.mt_count = 1; // Nombre d'actions a effectuer
		if (fIsOpen == false)
			Open('r'); // it is necessary to have a opened device to do this iocl function
		fStatus = ioctl(fLun, MTIOCTOP, &Control);
		if (fStatus < 0) {
			fStatus = ACQ_ERRPARAM;
			fError.TreatError(2, fStatus, "Eject tape problem");
		} else {
			fStatus = ACQ_OK;
			cout << " Eject done\n ";
		}
		break;
	default:
		fError.TreatError(2, fStatus, "Not possible to eject a non Tape device");
	}
	Close();
}

//_______________________________________________________________________________

void GFile::GetPos(long* position) {
	// get position in blocks of a tape or a disk file

	fStatus = ACQ_OK;
	*position = 0;
	if (fIsOpen == false) { // if device is not opened , do nothing
		return;
	}
	switch (GetType()) {
	case GT_TYPE_FILE:
		if (fLun > 0)
			*position = lseek(fLun, 0, SEEK_CUR);
		if (fLungz !=NULL)
			*position = gzseek(fLungz, 0, SEEK_CUR);
		*position /= fBuffer->GetBufSize();
		fStatus = int(*position);
		if (fStatus < 0) {
			fStatus = ACQ_ERRPARAM;
			*position = 0;
			fError.TreatError(2, fStatus, "Unable to get file position!");
			return;
		}
		fStatus = ACQ_OK;
		break;
#ifndef MACOSX
		struct mtpos Mtpos;
	case GT_TYPE_TAPE:

		fStatus = ioctl(fLun, MTIOCPOS, &Mtpos);
		if (fStatus < 0) {
			fStatus = ACQ_ERRPARAM;
			*position = 0;
			fError.TreatError(2, fStatus, "Unable to get file position!");
			return;
		}
		*position = (Mtpos.mt_blkno);
		break;
#endif
	default:
		*position = 0;
		fStatus = ACQ_ERRPARAM;
	}

}

//_________________________________________________________________________________
TNamed* GFile::GetSpectrum(const char *spectrumname, TNamed* old_sp) {

	// Retrieve a specific spectrum buffer from Ganil Acquisition files;
	// the result is returned  in a 1 dimension ROOT histogram (TH1I)
	// or a  the result is returned  in a  2 dimension ROOT histogram (TH2S)
	// entries: "spectrumname" have no signification in GFile and can be left NULL
	//          "old_histo" is old histogram if is existe to replace

	TETE_SPEC* BufferSpectrumHeader; //buffer of spectrum header

	char* buffer;
	char type_spectre[3];
	Int_t taille_canal, i, j;
	Float_t x_ratio = 1, y_ratio = 1;
	char nom_par_x[17], nom_par_y[17], unite_x[17], unite_y[17], type_canal[5];
	//Int_t magik;
	Int_t dim_x, dim_y, min_x, min_y, max_x, max_y;
	Int_t num_spectre; // spectrum number
	char nom_spectre[17]; // spectrum name
	char* nom_spectre2; // spectrum name without white space
	int sizebuffer;
	int Size, taille_type_canal;// = 2 ou 4
	bool opencontext;
	TH1I* h1d = NULL;
	TH2S* h2d = NULL;
	TH1* hret = NULL;
	//	TNamed* spret = NULL;
	int entry = 0;

	fStatus = ACQ_OK;
	sizebuffer = sizeof(TETE_SPEC);
	buffer = new char[sizebuffer];
	if (fVerbose > 9) {

		strcpy(buffer, "Bonjour\n");
		cout << buffer << "\n";
	}
	opencontext = fIsOpen;

	if (!fIsOpen) {
		Open();
		cout << "\t*** open file ! ***\n";
	}
	Rewind(true);
	if (old_sp)
		delete (old_sp);
	//lecture de l'entete
	Size = MyRead(buffer, sizebuffer);
	// BufferSpectrumHeader =(BUF_ENTSPEC*) buffer;
	BufferSpectrumHeader = (TETE_SPEC*) buffer;
	//      Size = fStatus;       /* Pour retourner nombre de caracteres lus */

	if (Size <= 0) {
		fStatus = ACQ_ERRFICSPECTRE;
		fError.TreatError(2, fStatus, "Spectrum file");
	}

	//  BufferSpectrumHeader =(BUF_ENTSPEC*) buffer;

	BufferSpectrumHeader = (TETE_SPEC*) buffer;

	cout << "  Spectrum analyse \n";

	//	magik = (Int_t) BufferSpectrumHeader->magik;
	num_spectre = (Int_t) BufferSpectrumHeader->num_spectre;
	strncpy(nom_spectre, BufferSpectrumHeader->nom_spectre, 15);
	nom_spectre[15] = '\0';
	nom_spectre2 = RemoveWhite(nom_spectre);
	dim_x = (Int_t) BufferSpectrumHeader->dim_x;
	dim_y = (Int_t) BufferSpectrumHeader->dim_y;
	strncpy(nom_par_x, BufferSpectrumHeader->nom_par_x, 15);
	nom_par_x[15] = '\0';
	strncpy(nom_par_y, BufferSpectrumHeader->nom_par_y, 15);
	nom_par_y[15] = '\0';
	strncpy(unite_x, BufferSpectrumHeader->unite_x, 7);
	unite_x[7] = '\0';
	strncpy(unite_y, BufferSpectrumHeader->unite_y, 7);
	unite_y[7] = '\0';
	min_x = (Int_t) BufferSpectrumHeader->min_x;
	max_x = (Int_t) BufferSpectrumHeader->max_x;
	min_y = (Int_t) BufferSpectrumHeader->min_y;
	max_y = (Int_t) BufferSpectrumHeader->max_y;
	taille_canal = (Int_t) BufferSpectrumHeader->taille_canal;
	strncpy(type_canal, BufferSpectrumHeader->type_canal, 3);
	type_canal[3] = '\0';
	strncpy(type_spectre, BufferSpectrumHeader->type_spectre, 2);
	type_spectre[2] = '\0';

	if (fVerbose) {
		cout << " sizeof(TETE_SPEC) = " << sizebuffer << "\n";
		cout << " Get spectra nb : " << num_spectre << "    Name :"
				<< nom_spectre2 << "\n";
		cout << "x_name:" << nom_par_x << "  y_name:" << nom_par_y << " dim x:"
				<< dim_x << " dim y:" << dim_y << "\n";
		cout << "min_x:" << min_x << "  min_y:" << min_y << " max_x:" << max_x
				<< " max_y:" << max_y << "\n";
		cout << " type_canal = " << type_canal << "   Taille Canal : "
				<< taille_canal << " Type spectre : " << type_spectre << "\n";
	}

	if (strcmp("I*2", type_canal) == 0) {
		taille_type_canal = 2;
	} else {
		taille_type_canal = 4;
	}

	// to solve a acquisition bug
	if ((min_x == 0) && (max_x == 0))
		max_x += dim_x - 1;
	if ((min_y == 0) && (max_y == 0))
		max_y += dim_y - 1;

	if (dim_x != 0)
		x_ratio = (float) (max_x - min_x + 1) / (float) dim_x;
	if (dim_y != 0)
		y_ratio = (float) (max_y - min_y + 1) / (float) dim_y;

	if (strcmp("1D", type_spectre) == 0) {
		if ((taille_type_canal) == 4) {// canaux sur 4 octets
			UInt_t receive;
			h1d = new TH1I(nom_spectre2, nom_spectre2, dim_x, min_x, max_x);
			sizebuffer = dim_x * (taille_type_canal);
			for (i = 0; i < dim_x; i++) {
				Size = MyRead((char*) (&receive), taille_type_canal);
				h1d->Fill(i * x_ratio, receive);
				entry += (int) receive;
			}
			h1d->SetEntries(entry);
			hret = h1d;
		} else
			fError.TreatError(2, fStatus, "in spectrum 1D allocation");

		//Size = fStatus;       /* Pour retourner nombre de caracteres lus */
		if (Size <= 0) {
			fStatus = ACQ_ERRFICSPECTRE;
			fError.TreatError(2, fStatus, "ERRFICSPECTRE");
		}
	}

	if (strcmp("2D", type_spectre) == 0) {

		if ((taille_type_canal) == 2) {// canaux sur 2 octets
			UShort_t receive;
			h2d = new TH2S(nom_spectre2, nom_spectre2, dim_x, min_x, max_x,
					dim_y, min_y, max_y);
			sizebuffer = dim_x * dim_y * (taille_type_canal);
			for (j = 0; j < dim_y; j++) {
				for (i = 0; i < dim_x; i++) {
					Size = MyRead((char*) (&receive), taille_type_canal);
					h2d->Fill(i * x_ratio, j * y_ratio, receive);
					entry += (int) receive;
				}
			}
			h2d->SetEntries(entry);
			hret = h2d;
		} else
			fError.TreatError(2, fStatus, "in spectrum 2D allocation\n");
	}

	if (fStatus != ACQ_OK) {
		fError.TreatError(2, fStatus, "somewhere get spectrum buffer ...");

	}

	if (opencontext == false) {
		Close();
		cout << "\t*** close file! ***\n";
	}
	//	spret = (TNamed*) hret;
	return (hret);
}

//____________________________________________________________________________
TString* GFile::GetListSpectra() {
	// this method do noting

	cout << " This GetListSpectra do nothing\n";
	return NULL;

}
//____________________________________________________________________________
void GFile::Convert1DtoTxt() {
	// convert   spectrum( in Ganil Format)  to 1D text histogram


	TETE_SPEC* BufferSpectrumHeader; //buffer of spectrum header

	char* buffer;
	char type_spectre[3];
	Int_t taille_canal, i;
	char nom_par_x[17], nom_par_y[17], unite_x[17], unite_y[17], type_canal[5];
	//	Int_t magik;
	Int_t dim_x, dim_y, min_x, min_y, max_x, max_y;
	Int_t num_spectre; // spectrum number
	char nom_spectre[17]; // spectrum name
	char* nom_spectre2; // spectrum name without white space
	Int_t sizebuffer;
	int Size, taille_type_canal;// = 2 ou 4
	bool opencontext;
	UInt_t receive = 0;
	ofstream outfile;
	char filename[MAX_CARACTERES];
	char *filename2;

	strcpy(filename, fName);

	filename2 = strtok(filename, ".");
	filename2 = strcat(filename2, ".txt");

	fStatus = ACQ_OK;
	sizebuffer = sizeof(TETE_SPEC);
	buffer = new char[sizebuffer];

	opencontext = fIsOpen;

	if (!fIsOpen) {
		Open();
		cout << "\t*** open file ! ***\n";
	}
	Rewind(true);
	//lecture de l'entete
	Size = MyRead(buffer, sizebuffer);
	// BufferSpectrumHeader =(BUF_ENTSPEC*) buffer;
	BufferSpectrumHeader = (TETE_SPEC*) buffer;
	//      Size = fStatus;       /* Pour retourner nombre de caracteres lus */

	if (Size <= 0) {
		fStatus = ACQ_ERRFICSPECTRE;
		fError.TreatError(2, fStatus, "ERRFICSPECTRE!");
	}

	//  BufferSpectrumHeader =(BUF_ENTSPEC*) buffer;

	BufferSpectrumHeader = (TETE_SPEC*) buffer;

	cout << "  Spectrum analyse \n";

	//magik = (Int_t) BufferSpectrumHeader->magik;
	num_spectre = (Int_t) BufferSpectrumHeader->num_spectre;
	strncpy(nom_spectre, BufferSpectrumHeader->nom_spectre, 15);
	nom_spectre[15] = '\0';
	nom_spectre2 = RemoveWhite(nom_spectre);
	dim_x = (Int_t) BufferSpectrumHeader->dim_x;
	dim_y = (Int_t) BufferSpectrumHeader->dim_y;
	strncpy(nom_par_x, BufferSpectrumHeader->nom_par_x, 15);
	nom_par_x[15] = '\0';
	strncpy(nom_par_y, BufferSpectrumHeader->nom_par_y, 15);
	nom_par_y[15] = '\0';
	strncpy(unite_x, BufferSpectrumHeader->unite_x, 7);
	unite_x[7] = '\0';
	strncpy(unite_y, BufferSpectrumHeader->unite_y, 7);
	unite_y[7] = '\0';
	min_x = (Int_t) BufferSpectrumHeader->min_x;
	max_x = (Int_t) BufferSpectrumHeader->max_x;
	min_y = (Int_t) BufferSpectrumHeader->min_y;
	max_y = (Int_t) BufferSpectrumHeader->max_y;
	taille_canal = (Int_t) BufferSpectrumHeader->taille_canal;
	strncpy(type_canal, BufferSpectrumHeader->type_canal, 3);
	type_canal[3] = '\0';
	strncpy(type_spectre, BufferSpectrumHeader->type_spectre, 2);
	type_spectre[2] = '\0';

	cout << " Get spectra nb : " << num_spectre << "    Name :" << nom_spectre2
			<< "\n";
	cout << "x_name:" << nom_par_x << "  y_name:" << nom_par_y << " dim x:"
			<< dim_x << " dim y:" << dim_y << "\n";
	cout << "min_x:" << min_x << "  min_y:" << min_y << " max_x:" << max_x
			<< " max_y:" << max_y << "\n";

	cout << " type_canal = " << type_canal << "   Taille Canal : "
			<< taille_canal << " Type spectre : " << type_spectre << "\n";
	//  cout <<" Magic number "<< magik <<"=? cafefade \n";

	if (strcmp("I*2", type_canal) == 0) {
		taille_type_canal = 2;
	} else {
		taille_type_canal = 4;
	}

	// to solve a acquisition bug

	if ((min_x == 0) && (max_x == 0))
		max_x += dim_x - 1;
	if ((min_y == 0) && (max_y == 0))
		max_y += dim_y - 1;

	if (strcmp("1D", type_spectre) == 0) {
		if ((taille_type_canal) == 4) {// canaux sur 4 octets
			sizebuffer = dim_x * (taille_type_canal);
			outfile.open(filename2, ofstream::out);
			for (i = 0; i < dim_x; i++) {
				Size = MyRead((char*) (&receive), taille_type_canal);
				outfile << i << " " << receive << "\n";

			}
			outfile.close();
		} else
			fError.TreatError(2, fStatus, "Error in spectrum 1D allocation\n!");

		//Size = fStatus;       /* Pour retourner nombre de caracteres lus */
		if (Size <= 0) {
			fError.TreatError(2, fStatus, "ERRFICSPECTRE!");
		}
	}
	if (strcmp("2D", type_spectre) == 0) {
		fError.TreatError(2, fStatus, "Error, it is 1D spectrum\n!");
	}

	if (fStatus != ACQ_OK) {
		fError.TreatError(2, fStatus, "somewhere get spectrum buffer!");

	}

	if (opencontext == false) {
		Close();
		cout << "\t*** close file! ***\n";
	}

}
//_____________________________________________________________________
Int_t GFile::GetBufSizeFromFILEH(bool rewind) {
	// Look at Buffer size on device and warning if we get a diffence default value;
	Int_t Local_Bufsize;
	Local_Bufsize = -1;

	if (fIsOpen) {
		if (GetType() == GT_TYPE_FILE) {
			if (rewind)
				Rewind(true);
		}
		if (GetType() == GT_TYPE_TAPE) {
			ReadBuffer();
			if (rewind)
				Rewind(true);
		}
		ReadBuffer();
		Local_Bufsize = GetCurrentBuffer()->GetReadSize();
		if (fBuffer->GetType() == FILEH_Idn) {
			Skip('b', -1);
			ReadBuffer();
			if (Local_Bufsize == -1) {
				fError.TreatError(2, 0,
						"Can not read buffer size in Header file");
			}
		}
	}
	return Local_Bufsize;
}
//_____________________________________________________________________________
int GFile::ReadRunNumDate(Int_t placeofbuffer, bool rewind) {
	//read date and run number of current run
	// we suppose to be at begin of file

	if (fBuffer->GetType() == EVENTH_Idn) {
		strcpy(fDate, GetCurrentBuffer()->GetDate_());
		fRunNumber = GetCurrentBuffer()->GetNumRun_();
	}
	return 1;

}

//_____________________________________________________________________________
void GFile::Convert2DtoTxt() {
	// convert  spectrum ( in Ganil Format) to 2D txt  histogram


	TETE_SPEC* BufferSpectrumHeader; //buffer of spectrum header
	//BUF_ENTSPEC* BufferSpectrumHeader;  //buffer of spectrum header
	char* buffer;
	char type_spectre[3];
	Int_t taille_canal, i, j;
	Float_t x_ratio = 1, y_ratio = 1;
	char nom_par_x[17], nom_par_y[17], unite_x[17], unite_y[17], type_canal[5];
	//Int_t magik;
	Int_t dim_x, dim_y, min_x, min_y, max_x, max_y;
	Int_t num_spectre; // spectrum number
	char nom_spectre[17]; // spectrum name
	char* nom_spectre2; // spectrum name without white space
	Int_t sizebuffer;
	int Size, taille_type_canal;// = 2 ou 4
	bool opencontext;
	UShort_t receive = 0;

	ofstream outfile;
	char filename[MAX_CARACTERES];
	char *filename2;

	strcpy(filename, fName);

	filename2 = strtok(filename, ".");
	filename2 = strcat(filename2, ".txt");

	fStatus = ACQ_OK;
	sizebuffer = sizeof(TETE_SPEC);
	//sizebuffer = sizeof(BUF_ENTSPEC);
	buffer = new char[sizebuffer];

	opencontext = fIsOpen;

	if (!fIsOpen) {
		Open();
		cout << "\t*** open file ! ***\n";
	}
	Rewind(true);
	//lecture de l'entete
	Size = MyRead(buffer, sizebuffer);
	// BufferSpectrumHeader =(BUF_ENTSPEC*) buffer;
	BufferSpectrumHeader = (TETE_SPEC*) buffer;
	// Size = fStatus;       /* Pour retourner nombre de caracteres lus */

	if (Size <= 0) {
		fError.TreatError(2, fStatus, "ERRFICSPECTRE size ");
	}

	//  BufferSpectrumHeader =(BUF_ENTSPEC*) buffer;

	BufferSpectrumHeader = (TETE_SPEC*) buffer;

	cout << "  Spectrum analyse \n";

	//magik = (Int_t) BufferSpectrumHeader->magik;
	num_spectre = (Int_t) BufferSpectrumHeader->num_spectre;
	strncpy(nom_spectre, BufferSpectrumHeader->nom_spectre, 16);
	nom_spectre[16] = '\0';
	nom_spectre2 = RemoveWhite(nom_spectre);
	dim_x = (Int_t) BufferSpectrumHeader->dim_x;
	dim_y = (Int_t) BufferSpectrumHeader->dim_y;
	strncpy(nom_par_x, BufferSpectrumHeader->nom_par_x, 15);
	nom_par_x[15] = '\0';
	strncpy(nom_par_y, BufferSpectrumHeader->nom_par_y, 15);
	nom_par_y[15] = '\0';
	strncpy(unite_x, BufferSpectrumHeader->unite_x, 7);
	unite_x[7] = '\0';
	strncpy(unite_y, BufferSpectrumHeader->unite_y, 7);
	unite_y[7] = '\0';
	min_x = (Int_t) BufferSpectrumHeader->min_x;
	max_x = (Int_t) BufferSpectrumHeader->max_x;
	min_y = (Int_t) BufferSpectrumHeader->min_y;
	max_y = (Int_t) BufferSpectrumHeader->max_y;
	taille_canal = (Int_t) BufferSpectrumHeader->taille_canal;
	strncpy(type_canal, BufferSpectrumHeader->type_canal, 3);
	type_canal[3] = '\0';
	strncpy(type_spectre, BufferSpectrumHeader->type_spectre, 2);
	type_spectre[2] = '\0';

	cout << " Get spectra nb : " << num_spectre << "    Name :" << nom_spectre2
			<< "\n";
	cout << "x_name:" << nom_par_x << "  y_name:" << nom_par_y << " dim x:"
			<< dim_x << " dim y:" << dim_y << "\n";
	cout << "min_x:" << min_x << "  min_y:" << min_y << " max_x:" << max_x
			<< " max_y:" << max_y << "\n";

	cout << " type_canal = " << type_canal << "   Taille Canal : "
			<< taille_canal << " Type spectre : " << type_spectre << "\n";
	//  cout <<" Magic number "<< magik <<"=? cafefade \n";

	if (strcmp("I*2", type_canal) == 0) {
		taille_type_canal = 2;
	} else {
		taille_type_canal = 4;
	}

	// to solve a acquisition bug
	if ((min_x == 0) && (max_x == 0))
		max_x += dim_x - 1;
	if ((min_y == 0) && (max_y == 0))
		max_y += dim_y - 1;

	if (dim_x != 0)
		x_ratio = (float) (max_x - min_x + 1) / (float) dim_x;
	if (dim_y != 0)
		y_ratio = (float) (max_y - min_y + 1) / (float) dim_y;

	if (strcmp("2D", type_spectre) == 0) {
		if ((taille_type_canal) == 2) {// canaux sur 2 octets
			sizebuffer = dim_x * dim_y * (taille_type_canal);
			outfile.open(filename2, ofstream::out);
			cout << x_ratio << "  " << y_ratio << " \n";
			for (j = 0; j < dim_y; j++) {
				for (i = 0; i < dim_x; i++) {
					Size = MyRead((char*) (&receive), taille_type_canal);
					outfile << i << " " << j << " " << receive << "\n";
				}
			}
			outfile.close();
		} else
			fError.TreatError(2, fStatus, "Error in spectrum 2D allocation\n");

		//Size = fStatus;       /* Pour retourner nombre de caracteres lus */
		if (Size <= 0) {
			fError.TreatError(2, fStatus, "ERRFICSPECTRE");

		}
	}

	if (strcmp("1D", type_spectre) == 0) {
		fError.TreatError(2, fStatus, "Error, it is 1D spectrum !");
	}

	if (fStatus != ACQ_OK) {
		fError.TreatError(2, fStatus, " somewhere get spectrum buffer!\n");

	}

	if (opencontext == false) {
		Close();
		cout << "\t*** close file! ***\n";
	}
}
//_________________________________________________________________________________
////////////////////////////////////////fin /////////////////////////////////////




