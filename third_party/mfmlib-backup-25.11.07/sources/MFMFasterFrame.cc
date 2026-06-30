/*
 MFMFasterFrame.cc
 Copyright Acquisition group, GANIL Caen, France
 */

#include <iostream>
#include <cmath>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
using namespace std;

#include "MFMFasterFrame.h"

//_______________________________________________________________________________
MFMFasterFrame::MFMFasterFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, int headerSize) {
	/// Constructor for a Faster frame . the header is filled with unitblock_size, data source , frame type , revision , frame, size and header size value
	SetPointers();
}

//_______________________________________________________________________________
MFMFasterFrame::MFMFasterFrame() {
	/// Constructor for a empty Faster frame

}
//_______________________________________________________________________________
MFMFasterFrame::~MFMFasterFrame() {
/// destructor of Faster frame
}
//_______________________________________________________________________________
void MFMFasterFrame::SetUserDataPointer(){
	pUserData_char=(char*)&(((MFM_Faster_frame*) pHeader)->Data);
}
/*
//_______________________________________________________________________________
void MFMFasterFrame::SetAttributs(void * pt) {
	SetPointers(pt);
	MFMBlobFrame::SetAttributs(pt);
;
}*/
//_______________________________________________________________________________

void MFMFasterFrame::SetTimeStampFromFasterFrameData() {
	/// Computer, set attibut and return value of time stamp from  frame
	fTimeStamp = 0;
	uint64_t * timeStamp = &(fTimeStamp);
	memcpy(((char*) (&fTimeStamp)),
			((MFM_Faster_frame*) pHeader)->EventInfo.EventTime, 6);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt64((timeStamp), 6);
}
//_______________________________________________________________________________

void MFMFasterFrame::SetEventNumberFromFasterFrameData() {
	/// Computer, set attibut and return value of event number from  frame
	fEventNumber = 0;
	char * eventNumber = (char*) &(fEventNumber);
	//memcpy(&fEventNumber,((char*)((MFM_Faster_header*)pHeader)->mutEvtInfo.eventIdx),4);
	fEventNumber = ((MFM_Faster_frame*) pHeader)->EventInfo.EventIdx;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt32((uint32_t *) (eventNumber), 4);
}
//_______________________________________________________________________________
void MFMFasterFrame::SetTimeStamp(uint64_t timestamp) {
	/// Set value of Time Stamp in frame

	char* pts = (char*) &timestamp;
	timestamp = timestamp & 0x0000ffffffffffff;
	memcpy(((MFM_Faster_frame*) pHeader)->EventInfo.EventTime, pts, 6);
}
//_______________________________________________________________________________
void MFMFasterFrame::SetEventNumber(uint32_t eventnumber) {
	/// Set Event Number of frame
	((MFM_Faster_frame*) pHeader)->EventInfo.EventIdx = eventnumber;
}
//_______________________________________________________________________________
void MFMFasterFrame::FillDataWithRamdomValue(uint64_t timestamp,
		uint32_t enventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	char i = 0;
	char * pchar =(char*)&(((MFM_Faster_frame*) pHeader)->Data);
	int usersize = FASTER_USERSIZE;
	SetEventNumber(enventnumber);
	SetTimeStamp(timestamp);
	for (i=0;i< usersize;i++)
		*(pchar+i) = i;
}
//_______________________________________________________________________________
string MFMFasterFrame::GetHeaderDisplay(char* infotext) const {
	stringstream ss;
	string display("");
	display = ss.str();
	ss << MFMCommonFrame::GetHeaderDisplay(infotext);
	ss << "  Board = "<<GetBoardId()<<" | Channel = " << GetChannelId()<< " " <<endl;
	display = ss.str();
	return display;
}
//_______________________________________________________________________________

//_______________________________________________________________________________
