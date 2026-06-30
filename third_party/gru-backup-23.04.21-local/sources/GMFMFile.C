// File : GMFMFile.C
// Author: Luc Legeard
////////////////////////////////////////////////////////////////////////////////
// Class GMFMFile.C
//
// This class manage device (file ) in MFM format
// The associated methods can do copies,duplication, verification, dump
// This class  can bee compiled in a shared library witch can be used in root
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
#include "GEvent.h"
#include "GDevice.h"
#include "acq_codes_erreur.h"
#include "GMFMFile.h"
#include <stdlib.h>
#include <TString.h>
#include "TSystemDirectory.h"
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <TString.h>
#include <zlib.h>
#include "GTtape_erreur.h"

#include <memory> // auto_ptr
#include <cstdlib>

using namespace std;
//______________________________________________________________________________

ClassImp( GMFMFile);

GMFMFile::GMFMFile(void) {
	// Default constructor of device object;
	// equivalent to GMFMFile(DEFAULT_NAME,2000)

	TString tempos;
	char Type_text[MAX_CARACTERES];
	Init((const char*)"./file.graw");
	strcpy(Type_text, "MFMfile");

	tempos.Form("New device  Name : %s fType :%s ", fName, Type_text);
	fError.Infos(tempos);
}
//______________________________________________________________________________

GMFMFile::GMFMFile(const char* _Name)  {
	// Constructor/initialisator of device object

	char Type_text[MAX_CARACTERES];
	TString tempos;
	strcpy(Type_text, "MFM");
	Init(_Name);

	if (GetType() == GT_TYPE_FILE)
		strcpy(Type_text, "File");
	if (GetType() == GT_TYPE_DIR)
		strcpy(Type_text, "Directory");
	tempos.Form("New device  Name : %s fType :%s ", fName, Type_text);
	fError.Infos(tempos);

}
//____________________________________________________________________________
GMFMFile::~GMFMFile() {
	// destructor of GMFMFile object
	if (fVerbose > 5)
		fError.Infos("Delete device GMFMFile");
}
//____________________________________________________________________________
void GMFMFile::IsATape() {
	fStatus = ACQ_ISNOTATAPE;
	return;
}
//_____________________________________________________________________________

void GMFMFile::Init(const char *_Name) {
	// Called by every constructors.
	// Initialisation of main attributs
	fBufferSize = MFM_BLOB_HEADER_SIZE;
	if (fBuffer ){
		if (!(fBuffer->IsAMFMBuffer())) {
		delete fBuffer;
		fBuffer = NULL;
	}
}
	if (fBuffer == NULL)
		fBuffer = (GBuffer*) (new GBufferMFM(fBufferSize));
	SetCompressionLevel(6);
	fBufferHeadersize = 0;
	fType = GT_TYPE_MFMFILE;
	fWriteOrRead = false;
	fLun = 0;
	fLungz = NULL;
	SetDevice(_Name);
	fNewRunNumber = 0;

	// fIs_protected = false;
}
//______________________________________________________________________________

void GMFMFile::SetDevice(const char* Name1) { // change name of device
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
//___________________________________________________________________________________

void GMFMFile::WhatType() {
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
		fType = GA_NOT_DEFINED; // positionnnement par defaut
	} else {
		if ((bufstat.st_mode & S_IFDIR))
			fType = GT_TYPE_DIR;

		if ((bufstat.st_mode & S_IFREG))
			fType = GT_TYPE_MFMFILE;

		if (Context_IsOpen == false)

			Open(mode);
	}
}
//______________________________________________________________________________
void GMFMFile::Skip(bool quiet) {
	// Method to skip blocks or runs.
	// If the object is a file then then method is equivalent to Skip('b',1,quiet).
	// If the object is a tape the this mehod is equivalent to Skip ('f',1,quiet).
	// see : GTskip(char b,int nbr,bool quiet)
	if (GetType() == GT_TYPE_MFMFILE) {
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

void GMFMFile::Skip(char* b, int nbr, bool quiet) {
	// idem GTskip (char b,int nbr)
	// allow to take in acccount  Skip("f", 2) instead Skip('f', 2)

	char c;
	c = b[0];
	Skip(c, nbr, quiet);
}
//________________________________________________________________________

void GMFMFile::Skip(char b, int nbr, bool quiet) {
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
	fError.TreatError(1, 0, " No possible skip  with MFM file");

}

//________________________________________________________________________

void GMFMFile::Rewind(bool quiet) {
	// method to rewind a tape device or to place a at the begining of file in case of file device.
	// usage : device.Rewind()
	// equivalent to device.Skip('r');
	SkipRewind();
}
//__________________________________________________________________

void GMFMFile::Open(char* mod) {
	// Idem Open(char mod)
	// allow to take in account this kind of write : Open("r") instead Open('r')
	char c;
	c = mod[0];
	Open(c);

}
//_________________________________________________________________

void GMFMFile::Open(char mod) {
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
				== GT_TYPE_MFMFILE))
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
				== GT_TYPE_MFMFILE)) {
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

//____________________________________________________________________________

void GMFMFile::ReadBuffer() {
	// read on GMFMFile object the current buffer on device and fill the internal buffer
	// device object.

	//int SizeHead = fBuffer->GetHeaderSize();
	int SizeHead = MFM_BLOB_HEADER_SIZE;
	int Size =0;
	int oldsize =0;
	int returnsize;
	fStatus = ACQ_OK;
	if (fIsOpen == true) {
		if (SizeHead > 0) {

			if (fBuffer->GetBufSize() < SizeHead) {
				cout << " est celui ci?\n";
				((GBufferMFM*)fBuffer)->GBufferMFM::SetBufSize(SizeHead);
			}
			returnsize = MyRead(fBuffer->fGBuf_data, SizeHead);

			if (returnsize == SizeHead) {
				fStatus = ACQ_OK;
			} else {
				if (fStatus == 0) {
					fStatus = ACQ_ENDOFFILE;
				} else {
					fStatus = ACQ_ERRDURREAD;
				}
				fBuffer->fGBuf_type = UNKNOWN_Idn;
			}

			oldsize = fBuffer->GetBufSize();
			fBuffer->SetAttributs();
			Size = fBuffer->GetBufSizeFromBuffer();
			if (Size <= 0) {
				fStatus = ACQ_ENDOFFILE;
			}
			if (fStatus == ACQ_OK) {
				if (oldsize < Size) {

					((GBufferMFM*) fBuffer)->GBufferMFM::SetBufSize(Size);//raise size of buffer if necessary
				}
				returnsize = MyRead((fBuffer->fGBuf_data) + SizeHead, Size- SizeHead);
				if (returnsize == Size - SizeHead) {
					fStatus = ACQ_OK;
				} else {
					if (fStatus == 0) {
						fStatus = ACQ_ENDOFFILE;
					} else
						fStatus = ACQ_ERRDURREAD;

					fBuffer->fGBuf_type = UNKNOWN_Idn;
				}

			} else {
				fBuffer->fGBuf_type = UNKNOWN_Idn;
			}
		}
		fBuffer->fGBuf_increment = 0;
		if (fStatus != ACQ_OK) {
			fBuffer->fGBuf_index = 0;
		} else {
			fBuffer->SetAttributs();
		}

	} else {
		fError.TreatError(3, fStatus, "Read device is not open");
		fBuffer->fGBuf_type = UNKNOWN_Idn;

	}

}
//_____________________________________________________________________________
void GMFMFile::ReadBuffer(GBuffer& _Buffer) {
	ReadBuffer(&_Buffer);
}

//_____________________________________________________________________________
void GMFMFile::ReadBuffer(GBuffer* _Buffer) {
	// read on GMFMFile objet the curent buffer on device and fill "_Buffer"
	// the internal buffer device  is also filled
	// input : Pointer on Buffer to fill during reading

	ReadBuffer();
	fBuffer->Equal(_Buffer); // copy Buffer to _Buffer
}

//______________________________________________________________________________
void GMFMFile::WriteBuffer(GBuffer& _Buffer) {
	WriteBuffer(_Buffer);
}
//______________________________________________________________________________
void GMFMFile::WriteBuffer(GBuffer* _Buffer) {
	// write a buffer designed by _Buffer to the device

	int LocalStatus = ACQ_OK;
	int counter = 0;
	fStatus = ACQ_OK;
	//cout <<" debug GMFMFile::WriteBuffer Buffer->fGBuf_data "<<(int*) _Buffer->fGBuf_data<< "  size = "<<_Buffer->GetBufSize()<<"\n";

	if (((fWriteOrRead == true) && (fIsOpen == true))) {

		LocalStatus
				= GFile::MyWrite(_Buffer->fGBuf_data, _Buffer->GetBufSize());
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

void GMFMFile::WriteBuffer() {
	// Write his internal buffer on device

	int LocalStatus = ACQ_OK;
	fStatus = ACQ_OK;
	// B. Raine 7 juin 2005

	if ((fWriteOrRead == true) && (fIsOpen == true)) {
		LocalStatus
				= GFile::MyWrite(fBuffer->fGBuf_data, fBuffer->GetBufSize());
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

//_______________________________________________________________________________

void GMFMFile::SkipBlock(int NombreSkip) {

	struct mtop Control;
	fStatus = ACQ_OK;

	switch (GetType()) {
	case GT_TYPE_DIR:
		fError.TreatError(1, fStatus,
				" Not possible to skip with a Directory device.");
		fStatus = ACQ_ERRPARAM;
		break;
	case GT_TYPE_FILE:
		if (fLungz !=NULL )
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

void GMFMFile::SkipEO(int *NombreSkip) {
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

bool GMFMFile::IsARun(bool quiet) {
	// test if the device is a run file in IN2P3 format
	// if quiet = true standard  message are not displayed ( defaut = true)
	bool result = false;
	//TODO
	return (result);
}
//_______________________________________________________________________________

void GMFMFile::SkipRewind() {
	//Rewind a tape device or file device

	struct mtop Control;

	switch (GetType()) {

	case GT_TYPE_DIR:
		fError.TreatError(1, 0, " Not possible to skip with a Directory device");
		break;
	case GT_TYPE_TAPE:
		Control.mt_op = MTREW;
		Control.mt_count = 1; // Nombre d'actions a effectuer

		fStatus = ioctl(fLun, MTIOCTOP, &Control);
		if (fStatus != -1)
			fStatus = ACQ_OK;
		break;
	case GT_TYPE_FILE:
	case GT_TYPE_MFMFILE:
		if (fLungz != NULL)
			fStatus = gzseek(fLungz, 0, SEEK_SET);
		if (fLun > 0)
			fStatus = lseek(fLun, 0, SEEK_SET);

		if (fStatus < 0) {
			fStatus = ACQ_ERRPARAM;
			fError.TreatError(1, fStatus, "Lseek problem!");
		} else
			fStatus = ACQ_OK;
		break;

//	case GT_TYPE_MFMFILE:
//		if (fLungz != NULL)
//			fStatus = gzseek(fLungz, 0, SEEK_SET);
//		if (fLun > 0)
//			fStatus = lseek(fLun, 0, SEEK_SET);
//
//		if (fStatus < 0) {
//			fStatus = ACQ_ERRPARAM;
//			fError.TreatError(1, fStatus, "Lseek problem!");
//		} else
//			fStatus = ACQ_OK;
//		break;
//
	default:
		fStatus = ACQ_UNKNOWNDEV;
		fError.TreatError(1, fStatus,
				"Not possible to skip with a unknown  device");
		break;
	}
}

//_______________________________________________________________________________
int GMFMFile::MyRead(char * ptchar, int size) {
	int rsize = 0;
	if (fLun > 0)
		rsize = read(fLun, ptchar, size);
	if (fLungz != NULL)
		rsize = gzread(fLungz, ptchar, size);
	return rsize;
}

//_______________________________________________________________________________
long long int GMFMFile::MyOpen(char * name, int mode) {
	long long int status = -1;
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
		status = (long long int) fLun;
		fLungz = NULL;
	} else {
		fLungz = gzopen(name, chmod);
		status = (long long int) fLungz;
		fLun = 0;
	}

	return status;
}
//_______________________________________________________________________________
int GMFMFile::MyClose() {
	int status = 0;
	if (fLun > 0)
		status = close(fLun);
	if (fLungz != NULL)
		status = gzclose(fLungz);
	return status;
}

/*
//_______________________________________________________________________________

void GMFMFile::GetPos(long* position) {
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
		if (fLungz != NULL)
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
*/

