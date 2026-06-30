/*
 MFMChimeraFrame.cc
 Copyright Acquisition group, GANIL Caen, France
 */

// e.d.f. 2016  (test chimera daq)

#include <iostream>
#include <cmath>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
using namespace std;

#include "MFMChimeraFrame.h"

//_______________________________________________________________________________
MFMChimeraFrame::MFMChimeraFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, int headerSize) {
	/// Constructor for a Chimera/Blob frame. the header is filled with unitblock_size, data source, 
	/// frame type , revision , frame, size and header size value
	SetPointers();
}

//_______________________________________________________________________________
MFMChimeraFrame::MFMChimeraFrame() {
	/// Constructor for a empty Chimera frame
}
//_______________________________________________________________________________
MFMChimeraFrame::~MFMChimeraFrame() {
	/// destructor of Chimera frame
}
//_______________________________________________________________________________
void MFMChimeraFrame::SetUserDataPointer(){
pUserData_char = (char*) &(((MFM_CHI_frame*) pHeader)->CHIData);
}
//_______________________________________________________________________________
void MFMChimeraFrame::SetAttributs(void * pt) {
	SetPointers(pt);
	MFMBlobFrame::SetAttributs(pt);
}
//_______________________________________________________________________________

void MFMChimeraFrame::SetTimeStampFromChimeraFrameData() {
	/// Computer, set attibut and return value of time stamp from  frame
	fTimeStamp = 0;
	uint64_t * timeStamp = &(fTimeStamp);
	memcpy(((char*) (&fTimeStamp)),
			((MFM_CHI_frame*) pHeader)->CHIEventInfo.EventTime, 6);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt64((timeStamp), 6);

}
//_______________________________________________________________________________

void MFMChimeraFrame::SetEventNumberFromChimeraFrameData() {
	/// Computer, set attibut and return value of event number from  frame
	fEventNumber = 0;
	char * eventNumber = (char*) &(fEventNumber);
	//memcpy(&fEventNumber,((char*)((MFM_hel_header*)pHeader)->helEvtInfo.eventIdx),4);
	fEventNumber = ((MFM_CHI_frame*) pHeader)->CHIEventInfo.EventIdx;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt32((uint32_t *) (eventNumber), 4);
}

//_______________________________________________________________________________
void MFMChimeraFrame::SetTimeStamp(uint64_t timestamp) {
	/// Set value of Time Stamp in frame

	char* pts = (char*) &timestamp;
	timestamp = timestamp & 0x0000ffffffffffff;
	memcpy(((MFM_CHI_frame*) pHeader)->CHIEventInfo.EventTime, pts, 6);
}
//_______________________________________________________________________________
void MFMChimeraFrame::SetEventNumber(uint32_t eventnumber) {
	/// Set Event Number of frame
	((MFM_CHI_frame*) pHeader)->CHIEventInfo.EventIdx = eventnumber;
}
//_______________________________________________________________________________
void MFMChimeraFrame::FillDataWithRamdomValue(uint64_t timestamp,
		uint32_t enventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	SetEventNumber(enventnumber);
	SetTimeStamp(timestamp);
	SetUserDataPointer();
	int i=0;
	int usersize = CHI_USERSIZE;
		//char * pchar =(char*)&(((MFM_Fazia_Header*) pHeader)->Data);(char*)&(((MFM_CHI_frame*) pHeader)->CHIData);
			for (i=0;i< usersize;i++)  *((char*)GetPointUserData()+i) =i;
}

