// File : GBufferMFM.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GBufferMFM
//
// This class manage MFM buffer
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

#include "GBufferMFM.h"
#include "acq_codes_erreur.h"
#include <fstream>

#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;

ClassImp ( GBufferMFM);

GBufferMFM::GBufferMFM(int _bufsize) {
	// constructor of GBufferMFM
	fGBuf_data = NULL;
	fGBuf_index = 0;//
	fGBuf_increment = 0; // use in screen dump to memory where we are in dump
	fVerbose = 0; // use to  debug ( =0 no debug information, =10 all)
	if (_bufsize < MFM_BLOB_HEADER_SIZE)
		fGBuf_size = MFM_BLOB_HEADER_SIZE;
	else
		fGBuf_size = _bufsize; // Buffer size
	fIsAEventType = false; // flag  to indicate if the buffer contain events
	fGBuf_type = UNKNOWN_Idn;
	//fGBuf_header[0] = '\0';

	strcpy(fDate, "");
	fRunNumber = 0;

	fUsedEventsSize = 0;
	pFrame = new MFMCommonFrame(1, 0, 0, 0, fGBuf_size);
	fGBuf_data = (char*) pFrame->GetDataPointer();
	fIsALocalAlloc = true; // remis a true car plantage .le 7/7/17

}

//_______________________________________________________________________________
GBufferMFM::~GBufferMFM() {
	//destructor of GBufferMFM

	if (pFrame) {
		delete pFrame;
		pFrame = NULL;
		fGBuf_data = NULL;
	}

	if (fVerbose > 0)
		cout << "Delete buffer." << "endl";
}

//____________________________________________________
void GBufferMFM::Equal(GBuffer& buf1) {
	Equal(buf1);
}
//____________________________________________________
void GBufferMFM::Equal(GBuffer* buf1) {
	// buffer copy method
	//Example
	//>> Buf1.GBequal(Buf2)
	// all data from buffer Buf1 are copied in buffer Buf2

	memcpy(buf1->fGBuf_data, fGBuf_data, fGBuf_size);
	buf1->fGBuf_increment = fGBuf_increment;
	buf1->fGBuf_index = fGBuf_index;
	buf1->fGBuf_type = fGBuf_type;
	buf1->SetVerbose(fVerbose);
	buf1->fRunNumber = fRunNumber;
	strcpy(buf1->fDate, fDate);
}

//____________________________________________________________________
void GBufferMFM::SetAttributs(bool quiet) {
	// used only in ReadBuffer method of a device()
	// set few attributes after a buffer is read on device
	pFrame->SetAttributs(fGBuf_data);
	fGBuf_type = TestType(fGBuf_data);
	fGBuf_size = pFrame->GetFrameSize();
	fIsAEventType = true;
	fIsAMFMType = true;
	//fGBuf_index =GetNumBuf();
	//cout <<"debug GBufferMFM::SetAttributs"<< fGBuf_size<<"\n";
	// TODO fGBuf_index =
	// TODO fGBuf_Number =
}
//____________________________________________________________________
void GBufferMFM::MyDelete() {
	delete[] fGBuf_data;
	fGBuf_data = NULL;
}
//____________________________________________________________________
int GBufferMFM::GetHeaderSize() {
	int size = pFrame->GetHeaderSize();
	return (size);
}
//______________________________________________________________________________
void GBufferMFM::SetExternalDataZone(void* pt, int size) {
	//cout << " debug GBufferMFM::SetExternalDataZone"<<endl;
	GBuffer::SetExternalDataZone(pt, size);
	SetAttributs();
}
//____________________________________________________________________
int GBufferMFM::GetBufSizeFromBuffer() {
	// method to get bufsize  form header of buffer
	// size = (number of UnitBlock) *  UnitBlockSize
	pFrame->SetUnitBlockSizeFromFrameData();
	int size = pFrame->GetFrameSize();
	SetReadSize(size);
	return size;
}
//_______________________________________________________________________________
Int_t GBufferMFM::TestType(char * pt) {
	Int_t typeevent;
	Int_t typebuffer;
	typebuffer = UNKNOWN_Idn;
	char * local_data;
	if (pt == NULL)
		local_data = fGBuf_data;
	else
		local_data = pt;

	MFMCommonFrame tempo;
	tempo.SetPointers(local_data);
	tempo.SetAttributs();
	typeevent = tempo.GetFrameType();
	typebuffer= typeevent;

	return typebuffer;
}

//_______________________________________________________________________________

void GBufferMFM::MakeMFMHeader(UInt_t blockNumber, Short_t sourceId,
		Short_t destinationId, Short_t dataStreamNumber, Short_t numOfEvents) {
	fError.TreatError(2,0,"GBufferMFM::MakeMFMHeader not ready! ");
	// write header and end of MFM buffer
	/*
	 pBasicFrame->MFM_make_header(int unitBlock_size, int dataSource,
	 int frameType, int revision, int frameSize, int headerSize,
	 int itemSize, int nItems);
	 */

}

//_______________________________________________________________________________
bool GBufferMFM::IsAMFMBuffer() {
	// return true if is a IN2P3 buffer
	bool test = false;
	if ((MFM_MIN_TYPE <= fGBuf_type) and (MFM_MAX_TYPE >= fGBuf_type))
		test = true;
	return test;
}

//_______________________________________________________________________________
TString GBufferMFM::GetDumpBuffer(int dumpsize, int increment) {
	return (pFrame->GetDumpRaw(dumpsize, increment));
}

//______________________________________________________________________________
TString GBufferMFM::GetDumpBufferHeader() {
	TString mydump;
	mydump = pFrame->GetHeaderDisplay();
	return mydump;
}
//______________________________________________________________________________
void GBufferMFM::SetBufSize(int size) {
	static int nb=0;
	//int old_size = fGBuf_size;
	fGBuf_size = size;
	nb++;
//	cout << "GBufferMFM::SetBufSize "<<size<<"\n";
//	cout<< "GBufferMFM::SetBufSize " <<( long long )fGBuf_data <<"  "<< fIsALocalAlloc<<"\n";
	if ((fGBuf_data) && (fIsALocalAlloc)) {
		//cout << "GBufferMFM::SetBufSize from "<<old_size<< " to " << size<<  ", fIsALocalAlloc ="<< fIsALocalAlloc<< " "<<nb<<"\n";
		pFrame->SetBufferSize(size);
		fGBuf_data = (char*) (pFrame->GetPointHeader());
		fGBuf_size = pFrame->GetBufferSize();
	}
	fIsALocalAlloc = true;
}
//_______________________________________________________________________________
void GBufferMFM::DumpEventBufferData() {
	TString mydump;
	TString tempos;
}
//_______________________________________________________________________________
