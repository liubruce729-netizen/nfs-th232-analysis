/*
 MFMRibfFrame.cc
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

#include "MFMRibfFrame.h"

//_______________________________________________________________________________
MFMRibfFrame::MFMRibfFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, int headerSize) {
	/// Constructor for a Ribf frame . the header is filled with unitblock_size, data source , frame type , revision , frame, size and header size value
	SetPointers();
}

//_______________________________________________________________________________
MFMRibfFrame::MFMRibfFrame() {
	/// Constructor for a empty Ribf frame

}
//_______________________________________________________________________________
MFMRibfFrame::~MFMRibfFrame() {
/// destructor of Ribf frame
}
//_______________________________________________________________________________
void MFMRibfFrame::SetUserDataPointer(){
	pUserData_char=(char*)&(((MFM_Ribf_header*) pHeader)->Data);
}
/*
//_______________________________________________________________________________
void MFMRibfFrame::SetAttributs(void * pt) {
	SetPointers(pt);
	MFMBlobFrame::SetAttributs(pt);
	//SetTimeStampFromFrameData();
	//SetEventNumberFromFrameData();
}
 */
//_______________________________________________________________________________

void MFMRibfFrame::SetTimeStampFromRibfFrameData() {
	/// Computer, set attibut and return value of time stamp from  frame
	fTimeStamp = 0;
	uint64_t * timeStamp = &(fTimeStamp);
	memcpy(((char*) (&fTimeStamp)),
			((MFM_Ribf_header*) pHeader)->EventInfo.EventTime, 6);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt64((timeStamp), 6);
}
//_______________________________________________________________________________

void MFMRibfFrame::SetEventNumberFromRibfFrameData() {
	/// Computer, set attibut and return value of event number from  frame
	fEventNumber = 0;
	char * eventNumber = (char*) &(fEventNumber);
	//memcpy(&fEventNumber,((char*)((MFM_Ribf_header*)pHeader)->mutEvtInfo.eventIdx),4);
	fEventNumber = ((MFM_Ribf_header*) pHeader)->EventInfo.EventIdx;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt32((uint32_t *) (eventNumber), 4);
}
//_______________________________________________________________________________
void MFMRibfFrame::SetTimeStamp(uint64_t timestamp) {
	/// Set value of Time Stamp in frame

	char* pts = (char*) &timestamp;
	timestamp = timestamp & 0x0000ffffffffffff;
	memcpy(((MFM_Ribf_header*) pHeader)->EventInfo.EventTime, pts, 6);
}
//_______________________________________________________________________________
void MFMRibfFrame::SetEventNumber(uint32_t eventnumber) {
	/// Set Event Number of frame
	((MFM_Ribf_header*) pHeader)->EventInfo.EventIdx = eventnumber;
}
//_______________________________________________________________________________
void MFMRibfFrame::FillDataWithRamdomValue(uint64_t timestamp,
		uint32_t enventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	char i = 0;
	char * pchar =(char*)&(((MFM_Ribf_header*) pHeader)->Data);
	int usersize =RIBF_USERSIZE;
	SetEventNumber(enventnumber);
	SetTimeStamp(timestamp);
	for (i=0;i< usersize;i++)
		*(pchar+i) =i;
}
//_______________________________________________________________________________
string MFMRibfFrame::GetHeaderDisplay(char* infotext) const {
	stringstream ss;
	string display("");
	display = ss.str();
	ss << MFMCommonFrame::GetHeaderDisplay(infotext);
	display = ss.str();
	return display;
}
//_______________________________________________________________________________
