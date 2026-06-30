/*
 MFMMergeFrame.cc
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

#include "MFMMergeFrame.h"

//_______________________________________________________________________________
MFMMergeFrame::MFMMergeFrame(int unitBlock_size, int dataSource, int frameType,
		int revision, int frameSize, int headerSize, int itemSize, int nItems) {
	/// Constructor of Merge frame . The Header is filled with unitBlock_size ... nItems data.

	SetPointers();

}
//_______________________________________________________________________________
MFMMergeFrame::MFMMergeFrame() {
	/// Constructor of a empty frame
	pCurrentPointerForAdd = NULL;
	pCurrentPointerForRead = NULL;
	//	cout << "debug constructor of MFMMergeFrame::MFMMergeFrame()\n";
}
//_______________________________________________________________________________
MFMMergeFrame::~MFMMergeFrame() {
	///destructor of MFMMergeFrame
}
//_______________________________________________________________________________
void MFMMergeFrame::SetAttributs(void * pt) {
	/// Initialize a set of attributs (frame size, endianess, type ...) and pointers of frame\n
	/// reading and computing data comming from header of frame\n
	/// if pt==NULL initialization is done with current value of main pointer of frame (pData)\n
	/// else initialization is done with pData = pt
	SetPointers(pt);
	MFMBasicFrame::SetAttributs(pt);
	ResetAdd();
	ResetReadInMem();
}
//_______________________________________________________________________________
void MFMMergeFrame::ResetReadInMem() {
	/// reset read pointer to the begin of first inside frame
	pCurrentPointerForRead = pData_char + fHeaderSize;
}
//_______________________________________________________________________________
void MFMMergeFrame::ResetAdd() {
	/// reset add pointer  to the begin of memorize of Merge frame dedicated to inside frame
	pCurrentPointerForAdd = pData_char + fHeaderSize;
}
//_______________________________________________________________________________

void MFMMergeFrame::SetTimeStampFromMergeFrameData() {
	/// Compute time stamp and fill fTimeStamp attribut. return value of TimeStamp
	fTimeStamp = 0;
	uint64_t * timeStamp = &(fTimeStamp);

	if (GetFrameType() == MFM_MERGE_TS_FRAME_TYPE) {
		memcpy(((char*) (&fTimeStamp)),
				((MFM_Merge_TSheader*) pHeader)->MergeEvtInfo.eventTime, 6);
		if (fLocalIsBigEndian != fFrameIsBigEndian)
			SwapInt64((timeStamp), 6);
	}
	if (GetFrameType() == MFM_MERGE_EN_FRAME_TYPE) {
		//nothing to do and we leave fTimeStamp to 0
		fTimeStamp = 0;
	}
}
//_______________________________________________________________________________

uint32_t MFMMergeFrame::GetDeltaTime() const {
	/// Compute time stamp and fill fTimeStamp attribut. return value of TimeStamp
	uint32_t DeltaTime = 0;
	uint32_t * deltatime = &(DeltaTime);

	if (GetFrameType() == MFM_MERGE_TS_FRAME_TYPE) {
		DeltaTime = ((MFM_Merge_TSheader*) pHeader)->MergeEvtInfo.deltaTime;

		if (fLocalIsBigEndian != fFrameIsBigEndian)
			SwapInt32((deltatime), 4);
	}
	if (GetFrameType() == MFM_MERGE_EN_FRAME_TYPE) {
		//nothing to do and we leave fTimeStamp to 0
		DeltaTime = 0;
	}
	return DeltaTime;
}
//_______________________________________________________________________________

void MFMMergeFrame::SetEventNumberFromMergeFrameData() {

	fEventNumber = 0;
	char * eventNumber = (char*) &(fEventNumber);

	if (GetFrameType() == MFM_MERGE_TS_FRAME_TYPE) {
		//nothing to do and we leave fEventNumber to 0
		fEventNumber = 0;
	}

	if (GetFrameType() == MFM_MERGE_EN_FRAME_TYPE) {
		fEventNumber = ((MFM_Merge_ENheader*) pHeader)->MergeEvtInfo.eventIdx;
		if (fLocalIsBigEndian != fFrameIsBigEndian)
			SwapInt32((uint32_t *) (eventNumber), 4);
	}
}
//_______________________________________________________________________________
void MFMMergeFrame::SetTimeStamp(uint64_t timestamp) {
	/// Set Time stamp data in frame
	char* pts = (char*) &timestamp;
	timestamp = timestamp & 0x0000ffffffffffff;

	if (GetFrameType() == MFM_MERGE_TS_FRAME_TYPE)
		memcpy(((MFM_Merge_TSheader*) pHeader)->MergeEvtInfo.eventTime, pts, 6);
	if (GetFrameType() == MFM_MERGE_EN_FRAME_TYPE) {//do nothing
	}
}
//_______________________________________________________________________________
void MFMMergeFrame::SetDeltaTime(uint32_t deltatime) {
	/// Set Delta Time data in frame

	if (GetFrameType() == MFM_MERGE_TS_FRAME_TYPE)
		((MFM_Merge_TSheader*) pHeader)->MergeEvtInfo.deltaTime=deltatime;
	if (GetFrameType() == MFM_MERGE_EN_FRAME_TYPE) {//do nothing
	}
}
//_______________________________________________________________________________
void MFMMergeFrame::SetEventNumber(uint32_t eventnumber) {
	/// Set Event Number data in frame
	if (GetFrameType() == MFM_MERGE_TS_FRAME_TYPE) {//do nothing

	}
	if (GetFrameType() == MFM_MERGE_EN_FRAME_TYPE)
		((MFM_Merge_ENheader*) pHeader)->MergeEvtInfo.eventIdx = eventnumber;
}
//_______________________________________________________________________________
void MFMMergeFrame::AddFrame(MFMCommonFrame* frame) {
	/// add a "frame" in free memory of Merge Frame
	int size = frame->GetFrameSize();
	uint32_t availablesize = fFrameSize - ((pCurrentPointerForAdd - pData_char)
			+ size);
	if (availablesize < (uint32_t) fFrameSize) {
		memcpy(pCurrentPointerForAdd, frame->GetPointHeader(), size);
		pCurrentPointerForAdd += size;
	} else {
		cout << " Error in MFMMergeFrame::AddFrame , no more place ";
		cout << " pCurrentPointerForAdd = " << (int*) pCurrentPointerForAdd
				<< " pData_char = " << (int*) pData_char << "availablesize "
				<< availablesize << "\n";
	}
}
//_______________________________________________________________________________
void MFMMergeFrame::ReadInFrame(MFMCommonFrame* frame) {
	// Read inside frame in Merge frame and return data in "frame"
	// if frame is not enough big , frame is automaticaly resized
	int readsize = frame->ReadInMem(&pCurrentPointerForRead);
	if (readsize <= 0)
		cout << "Error in MFMMergeFrame::ReadInFrame , no more place ";
}
//____________________________________________________________________________
bool MFMMergeFrame::HasTimeStamp()const{ return (fFrameType == MFM_MERGE_TS_FRAME_TYPE);}
//____________________________________________________________________________
bool MFMMergeFrame::HasEventNumber()const { return (fFrameType == MFM_MERGE_EN_FRAME_TYPE);}
//____________________________________________________________________________
string MFMMergeFrame::GetStat(string info)const {

	string display("");
	stringstream ss("");
	ss << MFMBasicFrame::GetStat(info);
	display = ss.str();

	return display;
}

//_______________________________________________________________________________
string MFMMergeFrame::GetHeaderDisplay(char* infotext) const {
	stringstream ss;
	string display("");
	display = ss.str();
	ss << MFMBasicFrame::GetHeaderDisplay(infotext);
	display = ss.str();
	return display;
}
//_______________________________________________________________________________
