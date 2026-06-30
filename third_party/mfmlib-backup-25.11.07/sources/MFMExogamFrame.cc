/*
 MFMExogamFrame.cc
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

#include "MFMExogamFrame.h"

//_______________________________________________________________________________
MFMExogamFrame::MFMExogamFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, int headerSize) {
	/// Constructor for a exogam frame . the header is filled with unitblock_size, data source , frame type , revision , frame, size and header size value
	SetPointers();
	fCountNbEventCard =NULL;
}

//_______________________________________________________________________________
MFMExogamFrame::MFMExogamFrame() {
	/// Constructor for a empty exogam frame
	fCountNbEventCard =NULL;
}
//_______________________________________________________________________________
MFMExogamFrame::~MFMExogamFrame() {
/// destructor of Exogam frame
	if (fCountNbEventCard){
		delete [] fCountNbEventCard;
		fCountNbEventCard =NULL;
	}
}
//_______________________________________________________________________________
void MFMExogamFrame::SetUserDataPointer() {
	pUserData_char=(char*)&(((MFM_exo_header*) pHeader)->ExoData);
}
/*
//_______________________________________________________________________________
void MFMExogamFrame::SetAttributs(void * pt) {
	SetPointers(pt);
	MFMBlobFrame::SetAttributs(pt);
}*/
//_______________________________________________________________________________
void MFMExogamFrame::SetTimeStampFromExogamFrameData() {
	/// Computer, set attibut and return value of time stamp from  frame
	fTimeStamp = 0;
	uint64_t * timeStamp = &(fTimeStamp);
	memcpy(((char*) (&fTimeStamp)),
			((MFM_exo_header*) pHeader)->ExoEventInfo.EventTime, 6);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt64((timeStamp), 6);
}
//_______________________________________________________________________________

void MFMExogamFrame::SetEventNumberFromExogamFrameData() {
	/// Computer, set attibut and return value of event number from  frame
	fEventNumber = 0;
	char * eventNumber = (char*) &(fEventNumber);
	//memcpy(&fEventNumber,((char*)((MFM_exo_header*)pHeader)->exoEvtInfo.eventIdx),4);
	fEventNumber = ((MFM_exo_header*) pHeader)->ExoEventInfo.EventIdx;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt32((uint32_t *) (eventNumber), 4);
}
//_______________________________________________________________________________
void MFMExogamFrame::SetTimeStamp(uint64_t timestamp) {
	/// Set value of Time Stamp in frame

	char* pts = (char*) &timestamp;
	timestamp = timestamp & 0x0000ffffffffffff;
	memcpy(((MFM_exo_header*) pHeader)->ExoEventInfo.EventTime, pts, 6);
}
//_______________________________________________________________________________
void MFMExogamFrame::SetEventNumber(uint32_t eventnumber) {
	/// Set Event Number of frame
	((MFM_exo_header*) pHeader)->ExoEventInfo.EventIdx = eventnumber;
}

//_______________________________________________________________________________
void MFMExogamFrame::ExoSetCristalId(uint16_t cristalId) {
	/// Set 16 bits of CristalIdx
	((MFM_exo_header*) pHeader)->ExoData.CristalId = cristalId;
}
//_______________________________________________________________________________
void MFMExogamFrame::ExoSetCristalId(uint16_t tgRequest, uint16_t idBoard) {

				uint16_t ui, ui2;
				ui2 = 0;
				//trig request form 0 to 4
				ui2 =tgRequest  & NUMEXO_CHANNEL_ID_MASK;

				//id board from 5 to 15
				ui = idBoard & NUMEXO_BOARD_ID_MASK;
				ui = ui << 5;
				ui2 = ui2 | ui;
				ExoSetCristalId(ui2);
}
//_______________________________________________________________________________

uint16_t MFMExogamFrame::ExoGetCristalId() const{
	uint16_t cristal = 0;
/// Compute and return the 2 bytes of CristalId()
	cristal = ((MFM_exo_header*) pHeader)->ExoData.CristalId;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16((&cristal));
	return ((cristal));
}

//_______________________________________________________________________________
uint16_t MFMExogamFrame::ExoGetTGCristalId()const {
/// Compute and return extracted Trigger Request value of CristalId
	return (ExoGetCristalId() & NUMEXO_CHANNEL_ID_MASK);
}

//_______________________________________________________________________________
uint16_t MFMExogamFrame::ExoGetBoardId()const {
/// Compute and return id value of Board
	return ((ExoGetCristalId()>>5) & NUMEXO_BOARD_ID_MASK);
}
//_______________________________________________________________________________
uint16_t MFMExogamFrame::GetBoardId()const {
/// Compute and return id value of Board
	return (ExoGetBoardId());
}
//_______________________________________________________________________________

void MFMExogamFrame::ExoSetStatus(int i, uint16_t status) {
	if (i < 0 and i > EXO_NB_STATUS)
		cout << "MFMExogamFrame::ExoSetStatus Error of status index\n";
	if (i == 0)
		((MFM_exo_header*) pHeader)->ExoData.Status1 = status;
	if (i == 1)
		((MFM_exo_header*) pHeader)->ExoData.Status2 = status;
	if (i == 2)
		((MFM_exo_header*) pHeader)->ExoData.Status3 = status;
}
//_______________________________________________________________________________

uint16_t MFMExogamFrame::ExoGetStatus(int i) const{
	/// Set Status (0,1 or 2)
	uint16_t status;
	if (i < 0 and i > EXO_NB_STATUS) {
		cout << "MFMExogamFrame::ExGetStatus Error of status index\n";
		return 0;
	} else {
		if (i == 0)
			status = (((MFM_exo_header*) pHeader)->ExoData.Status1);
		if (i == 1)
			status = (((MFM_exo_header*) pHeader)->ExoData.Status2);
		if (i == 2)
			status = (((MFM_exo_header*) pHeader)->ExoData.Status3);
	}
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&status);
	return status;
}

//_______________________________________________________________________________

void MFMExogamFrame::ExoSetDetaT(uint16_t deltaT) {
	/// Set Deta Time in frame header
	((MFM_exo_header*) pHeader)->ExoData.DeltaT = deltaT;
}

//_______________________________________________________________________________

uint16_t MFMExogamFrame::ExoGetDeltaT() const{
	/// Compute and Return Deta Time from frame header
	uint16_t deltat;
	deltat = (((MFM_exo_header*) pHeader)->ExoData.DeltaT);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&deltat);
	return deltat;
}
//_______________________________________________________________________________
void MFMExogamFrame::ExoSetInnerM(int i, uint16_t inner) {
	/// Set Inner data , 6 MeV (i=0) or inner 20 MeV (i=1)
	if (i < 0 and i >= EXO_NB_INNER_M) {
		cout << "MFMExogamFrame::ExoSetInner6_20M Error of inner index\n";
		return;
	}
	if (i == 0)
		(((MFM_exo_header*) pHeader)->ExoData.Inner6M) = inner;
	if (i == 1)
		(((MFM_exo_header*) pHeader)->ExoData.Inner20M) = inner;

}

//_______________________________________________________________________________
uint16_t MFMExogamFrame::ExoGetInnerM(int i)const {
	/// Compute and return Inner data , 6 MeV (i=0) or inner 20 MeV (i=1)
	uint16_t inner;
	if (i < 0 and i >= EXO_NB_INNER_M) {
		cout << "MFMExogamFrame::ExoGetInnerM Error of inner index\n";
		return 0;
	}
	if (i == 0)
		inner = (((MFM_exo_header*) pHeader)->ExoData.Inner6M);
	if (i == 1)
		inner = (((MFM_exo_header*) pHeader)->ExoData.Inner20M);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&inner);
	return inner;
}

//_______________________________________________________________________________
void MFMExogamFrame::ExoSetOuter(int i, uint16_t outer) {
	/// Set Outer data in frame. ( outer A B C D)
	if (i < 0 and i >= EXO_NB_OUTER) {
		cout << "MFMExogamFrame::ExoSetOuter, Error of outer index\n";
		return;
	}
	if (i == 0)
		(((MFM_exo_header*) pHeader)->ExoData.Outer1) = outer;
	if (i == 1)
		(((MFM_exo_header*) pHeader)->ExoData.Outer2) = outer;
	if (i == 2)
		(((MFM_exo_header*) pHeader)->ExoData.Outer3) = outer;
	if (i == 3)
		(((MFM_exo_header*) pHeader)->ExoData.Outer4) = outer;
}
//_______________________________________________________________________________

uint16_t MFMExogamFrame::ExoGetOuter(int i)const {
	/// Compute and return  Outer (6 Mev) data  (6 Mev)  (Outer 1 if i=0 .... outer 4 if i=3))
	uint16_t outer;
	if (i < 0 and i >= EXO_NB_OUTER) {
		cout << "MFMExogamFrame::ExoSetOuter, Error of outer index\n";
		return 0;
	}
	if (i == 0)
		outer = (((MFM_exo_header*) pHeader)->ExoData.Outer1);
	if (i == 1)
		outer = (((MFM_exo_header*) pHeader)->ExoData.Outer2);
	if (i == 2)
		outer = (((MFM_exo_header*) pHeader)->ExoData.Outer3);
	if (i == 3)
		outer = (((MFM_exo_header*) pHeader)->ExoData.Outer4);

	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&outer);
	return outer;
}
//_______________________________________________________________________________

void MFMExogamFrame::ExoSetBGO(uint16_t bgo) {
	/// Set BGO in frame
	(((MFM_exo_header*) pHeader)->ExoData.BGO) = bgo;
}
//_______________________________________________________________________________

uint16_t MFMExogamFrame::ExoGetBGO()const {
	/// computer and return  BGO value from frame
	uint16_t bgo;
	bgo = (((MFM_exo_header*) pHeader)->ExoData.BGO);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&bgo);
	return bgo;
}
//_______________________________________________________________________________

void MFMExogamFrame::ExoSetCsi(uint16_t csi) {

	/// Set Csi data in frame
	(((MFM_exo_header*) pHeader)->ExoData.Csi) = csi;
}
//_______________________________________________________________________________

uint16_t MFMExogamFrame::ExoGetCsi() const{
	/// Compute and return Csi data from frame
	uint16_t csi;
	csi = (((MFM_exo_header*) pHeader)->ExoData.Csi);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&csi);
	return csi;
}
//_______________________________________________________________________________

void MFMExogamFrame::ExoSetInnerT(int i, uint16_t inner) {
	/// Set Inner Time data in frame ( i=0 for T=30%,  i=1 for T=60%,  i=2 for T=90%)
	if (i < 0 and i >= EXO_NB_INNER_T) {
		cout << "MFMExogamFrame::ExoSetInnerT, Error of innerT index\n";
		return;
	}

	if (i == 0)
		(((MFM_exo_header*) pHeader)->ExoData.InnerT30) = inner;
	if (i == 1)
		(((MFM_exo_header*) pHeader)->ExoData.InnerT60) = inner;
	if (i == 2)
		(((MFM_exo_header*) pHeader)->ExoData.InnerT90) = inner;
	return;

}
//_______________________________________________________________________________

uint16_t MFMExogamFrame::ExoGetInnerT(int i) const{
	/// Compute and return Innet time data from frame ( i=0 for T=30%,  i=1 for T=60%,  i=2 for T=90%)
	uint16_t innerT;
	if (i < 0 and i >= EXO_NB_INNER_T) {
		cout << "MFMExogamFrame::ExoGetInnerT, Error of innerT index\n";
		return 0;
	}
	if (i == 0)
		innerT = (((MFM_exo_header*) pHeader)->ExoData.InnerT30);
	if (i == 1)
		innerT = (((MFM_exo_header*) pHeader)->ExoData.InnerT60);
	if (i == 2)
		innerT = (((MFM_exo_header*) pHeader)->ExoData.InnerT90);

	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&innerT);
	return innerT;

}
//_______________________________________________________________________________
void MFMExogamFrame::ExoSetPara(int i, uint16_t value) {
	/// Generic method to set data in frame
	if (i < 0 and i >= EXO_NB_MAX_PARA) {
		cout << "MFMExogamFrame::ExoSetPara, Error of para index\n";
		return;
	}
	uint16_t * pvalue;

	//    pvalue =	(uint16_t *) (pHeader)+ NUMEXO_HEADERFRAMESIZE;
	pvalue = (uint16_t *) (&(((MFM_exo_header*) pHeader)->ExoData));
	pvalue[i] = value;
	//cout<<(int*)&(pvalue[i])<<"\n";;
	return;
}

//_______________________________________________________________________________

uint16_t MFMExogamFrame::ExoGetPara(int i) const{
	/// Generic method to get data in frame
	uint16_t value;

	if (i < 0 and i >= EXO_NB_MAX_PARA) {
		cout << "MFMExogamFrame::ExoSetPara,Error of para index\n";
		return 0;
	}

	uint16_t *pvalue = (uint16_t *) (&(((MFM_exo_header*) pHeader)->ExoData));
	value = pvalue[i];

	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&value);
	return value;

}
//_______________________________________________________________________________

void MFMExogamFrame::ExoSetPadding(uint16_t padding) {
	/// Set Padding data
	(((MFM_exo_header*) pHeader)->ExoData.Padding) = padding;
}
//_______________________________________________________________________________

uint16_t MFMExogamFrame::ExoGetPadding() const{
	/// Compute and return value of Padding
	uint16_t padding;
	padding = (((MFM_exo_header*) pHeader)->ExoData.Padding);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&padding);
	return padding;
}
//_______________________________________________________________________________
void MFMExogamFrame::InitStat() {
	MFMCommonFrame::InitStat();
	int i;
	fCountNbEventCard = new long long[NUMEXO_MAX_NUMB_BOARDS];
	for ( i = 0;i<NUMEXO_MAX_NUMB_BOARDS;i++){
		fCountNbEventCard[i]=0;
	}
}
//____________________________________________________________________
void MFMExogamFrame::FillStat() {
	MFMCommonFrame::FillStat();
	uint16_t id ;
	id = ExoGetCristalId();
	fCountNbEventCard[id]++;

}
//____________________________________________________________________
string  MFMExogamFrame::GetStat(string info)const{

	string display("");
	stringstream ss("");
	ss << MFMCommonFrame::GetStat( info);
    int i, j; int total =0;

	for ( i=0;i<NUMEXO_MAX_NUMB_BOARDS;i++ ){

		if (fCountNbEventCard[i]!=0){
			j =i;
			ss << "Card "<< ((j>>5) & NUMEXO_BOARD_ID_MASK);
			j =i;
			ss << " Cristal  "<< (j& NUMEXO_CHANNEL_ID_MASK);
			ss << " NbEvents = "<< fCountNbEventCard[i] <<"\n";
			total += fCountNbEventCard[i];
		}
	}
	ss<<"Total Exogam Frames          = "<< total<<"\n";
	display = ss.str();
	return display;
}

//_______________________________________________________________________________
void MFMExogamFrame::FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	int value = random();
	int i = 0;
	uint16_t uivalue = (uint16_t) (4294967296 * value / RAND_MAX);
	for (i = 0; i < EXO_NB_MAX_PARA; i++) {
		ExoSetPara(i, uivalue);
	}
	ExoSetCristalId(8,116);
	SetEventNumber(enventnumber);
	SetTimeStamp(timestamp);
}

//_______________________________________________________________________________
string MFMExogamFrame::GetHeaderDisplay(char* infotext)const {
	stringstream ss;
	string display("");
	display = ss.str();
	ss << MFMCommonFrame::GetHeaderDisplay(infotext);
	ss << "   Cristal Id ="<<ExoGetTGCristalId();
	display = ss.str();
	return display;
//_______________________________________________________________________________
}
//_______________________________________________________________________________
