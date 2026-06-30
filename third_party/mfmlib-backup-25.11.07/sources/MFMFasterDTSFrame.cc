/*
 MFMFasterDTSFrame.cc
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

#include "MFMFasterDTSFrame.h"

//_______________________________________________________________________________
MFMFasterDTSFrame::MFMFasterDTSFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, int headerSize) {
	/// Constructor for a Faster frame . the header is filled with unitblock_size, data source , frame type , revision , frame, size and header size value
	SetPointers();
}
 
//_______________________________________________________________________________
MFMFasterDTSFrame::MFMFasterDTSFrame() {
	/// Constructor for a empty Faster frame

}
//_______________________________________________________________________________
MFMFasterDTSFrame::~MFMFasterDTSFrame() {
/// destructor of Faster frame
}
//_______________________________________________________________________________
void MFMFasterDTSFrame::SetUserDataPointer(){
	pUserData_char=(char*)&(((MFM_FasterDTS_frame*) pHeader)->Data);
}
//_______________________________________________________________________________

void MFMFasterDTSFrame::SetTimeStampFromFasterDTSFrameData() {
	/// Computer, set attibut and return value of time stamp from  frame
	fTimeStamp = 0;
	uint64_t * timestamp = &(fTimeStamp);
	memcpy(((char*) (&fTimeStamp)),
			((MFM_FasterDTS_frame*) pHeader)->EventInfo.Timestamp, 6);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt64((timestamp), 6);
}
//_______________________________________________________________________________

void MFMFasterDTSFrame::SetEventNumberFromFasterDTSFrameData() {
	/// Computer, set attibut and return value of event number from  frame
	fEventNumber = 0;
	char * eventNumber = (char*) &(fEventNumber);
	//memcpy(&fEventNumber,((char*)((MFM_Faster_header*)pHeader)->mutEvtInfo.eventIdx),4);
	fEventNumber = ((MFM_FasterDTS_frame*) pHeader)->EventInfo.EventIdx;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt32((uint32_t *) (eventNumber), 4);
}
//_______________________________________________________________________________
void MFMFasterDTSFrame::SetTimeStamp(uint64_t timestamp) {
	/// Set value of Time Stamp in frame
	char* pts = (char*) &timestamp;
	timestamp = timestamp & 0x0000ffffffffffff;
	memcpy(((MFM_FasterDTS_frame*) pHeader)->EventInfo.Timestamp, pts, 6);
}
//_______________________________________________________________________________
void MFMFasterDTSFrame::SetExternTimeStamp(uint64_t timestamp) {
	/// Set value of Time Stamp in frame

	char* pts = (char*) &timestamp;
	timestamp = timestamp & 0x0000ffffffffffff;
	memcpy(((MFM_FasterDTS_frame*) pHeader)->Data.ExternTimestamp, pts, 6);
}
//_______________________________________________________________________________
void MFMFasterDTSFrame::SetEventNumber(uint32_t eventnumber) {
	/// Set Event Number of frame
	((MFM_FasterDTS_frame*) pHeader)->EventInfo.EventIdx = eventnumber;
}
//_______________________________________________________________________________
void MFMFasterDTSFrame::FillDataWithRamdomValue(uint64_t timestamp,
		uint32_t enventnumber) {
	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	static bool flipflop = true ;
	SetEventNumber(enventnumber);
	SetTimeStamp(timestamp);
	SetExternTimeStamp(GetTimeStampUs(100000));
	
	SetBaseTime(8);
	SetExternBaseTime(10);
	if (flipflop){
		SetDeltaT(0x0A07010E0D);
		flipflop= false;
	}else{
		SetDeltaT(-0x0A07010E0D);
		flipflop= true;
	}
	
	SetIdOfExternSystem (0);
	((MFM_FasterDTS_frame*) pHeader)->Data.Reserved[0]       = 0x0; 
	((MFM_FasterDTS_frame*) pHeader)->Data.Reserved[1]       = 0xF; 
	((MFM_FasterDTS_frame*) pHeader)->Data.Reserved[2]       = 0xA; 
	((MFM_FasterDTS_frame*) pHeader)->Data.Reserved[3]       = 0x5; 
	((MFM_FasterDTS_frame*) pHeader)->Data.Reserved[4]       = 0x7; 
	((MFM_FasterDTS_frame*) pHeader)->Data.Reserved[5]       = 0xE; 
	((MFM_FasterDTS_frame*) pHeader)->Data.Reserved[6]       = 0x8; 
	((MFM_FasterDTS_frame*) pHeader)->Data.Reserved[7]       = 0x0; 
	SetChecksum(0x0);
}
//_______________________________________________________________________________
string MFMFasterDTSFrame::GetHeaderDisplay(char* infotext) const {
	stringstream ss;
	string display("");
	display = ss.str();
	ss << MFMCommonFrame::GetHeaderDisplay(infotext);
	ss << "  DeltaTS = "<<GetDeltaT() <<" " <<" | Faster BT = "<<GetBaseTime() <<" | Extern BT = " << GetExternBaseTime()<< " " <<" | Extern Time = " << GetExternTimestamp()<< " "<<endl;
	//ss << "  Faster Base Time = "<<GetBaseTime() <<" | Extern Base Time = " << GetExternBaseTime()<< " " <<" | Extern Time = " << GetExternTimestamp()<< " "<<endl;

	display = ss.str();
	return display;
}
//_______________________________________________________________________________
uint16_t MFMFasterDTSFrame::GetBaseTime() const {
	
	uint16_t bt = 0;
	char * pbt = (char*) &(bt);
	bt = ((MFM_FasterDTS_frame*) pHeader)->Data.TimeBase;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16((uint16_t *) (pbt));
	return bt;
}
//_______________________________________________________________________________
void  MFMFasterDTSFrame::SetBaseTime(uint16_t bt ) {
	((MFM_FasterDTS_frame*) pHeader)->Data.TimeBase = bt;
}
//_______________________________________________________________________________
uint16_t MFMFasterDTSFrame::GetExternBaseTime() const {
	
	uint16_t bt = 0;
	char * pbt = (char*) &(bt);
	bt = ((MFM_FasterDTS_frame*) pHeader)->Data.ExternTimeBase;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16((uint16_t *) (pbt));
	return bt;
}

//_______________________________________________________________________________
void  MFMFasterDTSFrame::SetExternBaseTime(uint16_t bt ) {
	((MFM_FasterDTS_frame*) pHeader)->Data.ExternTimeBase = bt;
}
//_______________________________________________________________________________
uint64_t MFMFasterDTSFrame::GetExternTimestamp() const {
	
	uint64_t timeStamp = 0;
	char * ptimeStamp = (char*) &(timeStamp);
	memcpy(ptimeStamp,
			((MFM_FasterDTS_frame*) pHeader)->Data.ExternTimestamp, 6);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt64((uint64_t*)ptimeStamp, 6);
	return timeStamp;
}

//_______________________________________________________________________________
 int64_t MFMFasterDTSFrame::GetDeltaT() const {
	
	int64_t dtimeStamp = 0;
	char * pdtimeStamp = (char *)&(dtimeStamp);
	
	dtimeStamp = (int64_t)((MFM_FasterDTS_frame*) pHeader)->Data.DeltaT;
	
	//memcpy(pdtimeStamp,
	//		(char *)((MFM_FasterDTS_frame*) pHeader)->Data.DeltaT, 8);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt64((uint64_t*) pdtimeStamp, 8);
	return dtimeStamp;
}
//_______________________________________________________________________________
void  MFMFasterDTSFrame::SetDeltaT( int64_t dt ) {
	((MFM_FasterDTS_frame*) pHeader)->Data.DeltaT = dt;
}
//_______________________________________________________________________________
uint16_t MFMFasterDTSFrame::GetIdOfExternSystem()const {
	// Get e
	uint16_t id = 0;
	char * pid = (char*) &(id);
	id = ((MFM_FasterDTS_frame*) pHeader)->Data.IdOfExternSystem;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16((uint16_t *) (pid));
	return id;
}
//_______________________________________________________________________________
void  MFMFasterDTSFrame::SetIdOfExternSystem(uint16_t id ){
	((MFM_FasterDTS_frame*) pHeader)->Data.IdOfExternSystem = id;
}
//_______________________________________________________________________________
uint16_t MFMFasterDTSFrame::GetChecksum() const {
	/// Compute and return value of Checksum from frame
	uint16_t ck = 0;
	char * pck = (char*) &(ck);
	ck = ((MFM_FasterDTS_frame*) pHeader)->Data.Checksum;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16((uint16_t *) (pck));
	return ck;
}
//_______________________________________________________________________________
void  MFMFasterDTSFrame::SetChecksum(uint16_t ck ) {
	((MFM_FasterDTS_frame*) pHeader)->Data.Checksum = ck;
}
//_______________________________________________________________________________

//_______________________________________________________________________________
