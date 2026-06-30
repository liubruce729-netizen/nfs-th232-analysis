// File : GBuffer.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GBuffer
//
// This class manage buffer in Ganil formatMFM_frame_common::
// The associated methods do dump....
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

#include <stdio.h>
#include <string>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "GBuffer.h"
#include "acq_codes_erreur.h"

using namespace std;

ClassImp ( GBuffer);

GBuffer::GBuffer(int _bufsize) {
	// constructor of GBuffer
	fGBuf_data = NULL;
	fGBuf_index = 0;//
	fGBuf_size = 0;
	fGBuf_increment = 0; // use in screen dump to memory where we are in dump
	fVerbose = 0; // use to  debug ( =0 no debug information, =10 all)

	fIsAEventType = false; // flag  to indicate if the buffer contain events
	fIsAIn2p3Type = false;
	fIsAMFMType = false;
	fGBuf_type = UNKNOWN_Idn;

	if (fVerbose > 1)
		cout << " buffer construction \n";
	strcpy(fDate, "");
	fRunNumber = 0;
	fIsALocalAlloc = false;
	fUsedEventsSize = 0;

}
//_______________________________________________________________________________
GBuffer::~GBuffer() {
	//destructor of GBuffer

	if ((fGBuf_data != NULL) && fIsALocalAlloc) {
		delete[] fGBuf_data;
		fGBuf_data = NULL;
	}
	fGBuf_size = 0;
	if (fVerbose > 0)
		cout << "Delete buffer." << "endl";
}
//_______________________________________________________________________________

int GBuffer::DumpBuffer(int dumpsize, int increment) {

	// Method to dump buffer on  output
	// if dumpsize =0 , dumpsize = standard = 256 and fGBbuf_increment raz

	cout << (GetDumpBuffer(dumpsize, increment)).Data();
	return (ACQ_OK);
}
//_______________________________________________________________________________

TString GBuffer::GetDumpBuffer(int dumpsize, int increment) {

	// Method to dump buffer on  output
	// if dumpsize =0 , dumpsize = standard = 256 and fGBbuf_increment raz

	if ((increment >= 0) and (increment < fGBuf_size))
		fGBuf_increment = increment;

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

	if (dumpsize == 0) {
		dumpsize = 256;
		fGBuf_increment = 0;
	}

	nbrline = (int) (dumpsize / nbrperline) + (int) ((dumpsize % nbrperline)
			!= 0);
	pShort = (unsigned short *) (fGBuf_data + fGBuf_increment);
	pChar = (unsigned char *) (fGBuf_data + fGBuf_increment);

	if (fGBuf_increment <= fGBuf_size - nbrperline) {
		if (fGBuf_increment == fGBuf_size)
			n = 1;
		else
			n = nbrline;
		for (i = 0; i < n; i++) {
			sprintf(tempo, "\n%5d %s ", fGBuf_increment, ": ");
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
			fGBuf_increment += nbrperline;
		}
		mydump += "\n";
	} else {
		mydump += "\n\t end of bloc \n";
	}

	return mydump;
}

//____________________________________________________________________
void GBuffer::SetBufSize(int size, bool forcetoalloc) {
	int oldsize = fGBuf_size;
	//static int nb=0;
	if ((forcetoalloc) or (size > fGBuf_size)) {
		fGBuf_size = size;
		//cout << "GBuffer::SetBufSize from "<<oldsize<< " to " << size<<  ", forcelocalalloc ="<< forcetoalloc<< " nb = "<<nb<<"\n";
		char* pointeurchar = NULL;
		if (fGBuf_size > 0)
			pointeurchar = new char[fGBuf_size];// raw data
		if ((fGBuf_data) && (fIsALocalAlloc)) {
			if (oldsize <= fGBuf_size)
				memcpy(pointeurchar, fGBuf_data, oldsize);
			else
				memcpy(pointeurchar, fGBuf_data, fGBuf_size);
			delete[] fGBuf_data;
			fGBuf_data = NULL;
		}
		fGBuf_data = pointeurchar;// raw data
		fIsALocalAlloc = true;
	}
}
/*
 //____________________________________________________________________
 void GBuffer::SetBufSize(int size, bool ifinferior) {
 // cette version ne passe pas dans valgrind et je ne sais pas pourquoi.
 int oldsize = fGBuf_size;
 fGBuf_size = size;

 if ((ifinferior) or (size > fGBuf_size)) {


 char* pointeurchar = NULL;
 //pointeurchar = new char[fGBuf_size];// raw data
 //pointeurchar = (char*) (realloc((void*) pointeurchar, size));// raw data
 pointeurchar = (char*) (malloc( size));
 // we prefer to make a realloc in place of new to be compatible with realloc in Dataparameter.cc

 if ((fGBuf_data) && (fIsALocalAlloc)) {
 if (oldsize <= fGBuf_size)
 memcpy(pointeurchar, fGBuf_data, oldsize);
 else
 memcpy(pointeurchar, fGBuf_data, fGBuf_size);
 delete [] fGBuf_data;
 fGBuf_data = NULL;

 }
 if (pointeurchar!=NULL)
 fGBuf_data = pointeurchar;// raw data
 fIsALocalAlloc = true;
 }


 }
 */
//____________________________________________________________________
void GBuffer::SetExternalDataZone(void* pt, int size) {
	if ((fGBuf_data) && (fIsALocalAlloc)) {
		MyDelete();
	}
	fIsALocalAlloc = false;
	fGBuf_data = (char*) pt;
	fGBuf_size = size;
}

//____________________________________________________________________
bool GBuffer::IsAEventBuffer() {
	// return true if buffer contains events
	bool testin2P3 = false;
	bool testMFM = false;
	testin2P3 = ((fGBuf_type == EBYEDAT_Idn) || (fGBuf_type == EVENTDB_Idn)
			|| (fGBuf_type == EVENTDB_SWAP_Idn) || (fGBuf_type == EVENTCT_Idn)
			|| (fGBuf_type == EVENTCT_SWAP_Idn) || (fGBuf_type == EBYEDAT_Idn)
			|| (fGBuf_type == RAWDT32_Idn) || (fGBuf_type == JBUS_SWAP_Idn)
			|| (fGBuf_type == JBUS_Idn));

	//if ((fGBuf_type =MFMBASIC_Idn)&&(fGBuf_type <=MFM_MAX_Idn))
	//test=true;
	if (IsAMFMBuffer())
	testMFM = !((fGBuf_type == MFM_HELLO_FRAME_TYPE) ||
	 	   (fGBuf_type == MFM_XML_DATA_DESCRIPTION_FRAME_TYPE) ||
	 	   (fGBuf_type == MFM_XML_FILE_HEADER_FRAME_TYPE) );
	   
	 //cout << " fIsAEventType = " << testin2P3<< "||"<<testMFM	<<" fGBuf_type = "<<fGBuf_type<<"\n";		
	fIsAEventType = testin2P3 || testMFM;
	return fIsAEventType;
}
//____________________________________________________________________
bool GBuffer::IsAHeaderBuffer() {
	// return true if it is a header buffer
	bool test;
	test = ((fGBuf_type == FILEH_Idn) || (fGBuf_type == EVENTH_Idn)
			|| (fGBuf_type == COMMENT_Idn) || (fGBuf_type == PARAM_Idn));
	fIsAEventType = !test;
	fIsAHeaderType = test;
	return (test);
}
//____________________________________________________________________
bool GBuffer::IsAMFMBuffer() {
	// return true if it is a header buffer
	fIsAMFMType = false;
	if (fGBuf_type >= MFM_MIN_TYPE && fGBuf_type <= MFM_MAX_TYPE)
		fIsAMFMType = true;
	return (fIsAMFMType);
}

//_______________________________________________________________________________

void GBuffer::SetUsedEventsSize(unsigned int usedEventSize) {
	fUsedEventsSize = usedEventSize;
}

//_______________________________________________________________________________

void GBuffer::RazBuffer() {
	for (int i = 0; i < fGBuf_size; i++) {
		fGBuf_data[i] = 0;
	}
	fUsedEventsSize = 0;
}
//_______________________________________________________________________________

void GBuffer::DumpBufferHeader() {
	cout << (GetDumpBufferHeader().Data()) << "\n";
	return;
}

////////////////////////////////////////fin /////////////////////////////////////
