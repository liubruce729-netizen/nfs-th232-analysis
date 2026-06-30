/*
 MFMCoboFrame.cc
 Copyright Acquisition group, GANIL Caen, France
 */

#include <iostream>
#include <cmath>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <cstdlib>
using namespace std;

#include "MFMCoboFrame.h"

//_______________________________________________________________________________
MFMCoboFrame::MFMCoboFrame(int unitBlock_size, int dataSource, int frameType,
		int revision, int frameSize, int headerSize, int itemSize, int nItems) {

	SetPointers();

}
//_______________________________________________________________________________
MFMCoboFrame::MFMCoboFrame() {

}
//_______________________________________________________________________________
MFMCoboFrame::~MFMCoboFrame() {
	//cout << "debug destructor of MFMCoboFrame::()\n";
}

//_______________________________________________________________________________
void MFMCoboFrame::SetAttributs(void * pt) {
	SetPointers(pt);
	MFMBasicFrame::SetAttributs(pt);
}

//_______________________________________________________________________________

void MFMCoboFrame::SetTimeStampFromCoboFrameData() {
	// Compute and return Time Stamp
	fTimeStamp = 0;
	uint64_t * ptimeStamp = &(fTimeStamp);
	memcpy(((char*) (&fTimeStamp)),
			((MFM_cobo_header*) pHeader)->coboEvtInfo.eventTime, 6);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt64((ptimeStamp), 6);
}
//_______________________________________________________________________________
int  MFMCoboFrame::GetItemSizeFromStructure(int type)const {

 if (type == MFM_COBO_FRAME_TYPE)
	return(sizeof(MFM_coboItem));
 if (type == MFM_COBOF_FRAME_TYPE)
	return (sizeof(MFM_cobofItem));
return 0;

}
//_______________________________________________________________________________
string MFMCoboFrame::GetHeaderDisplay(char* infotext) const{
	stringstream ss;
	string display("");
	display = ss.str();

	ss << MFMBasicFrame::GetHeaderDisplay(infotext);	
	ss << "   CoboIdx = " << CoboGetCoboIdx();
	ss << " CoboAsaIdx = " << CoboGetAsaIdx();
	display = ss.str();
	return display;
}
//_______________________________________________________________________________

void MFMCoboFrame::SetEventNumberFromCoboFrameData() {
	// compute and get Event Number
	fEventNumber = 0;
	char * peventNumber = (char*) &(fEventNumber);
	//memcpy(&fEventNumber,((char*)((MFM_cobo_header*)pHeader)->coboEvtInfo.eventIdx),4);
	fEventNumber = ((MFM_cobo_header*) pHeader)->coboEvtInfo.eventIdx;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt32((uint32_t *) (peventNumber), 4);
}
//_______________________________________________________________________________
void MFMCoboFrame::SetTimeStamp(uint64_t timestamp) {
	char* pts = (char*) &timestamp;
	timestamp = timestamp & 0x0000ffffffffffff;
	memcpy(((MFM_cobo_header*) pHeader)->coboEvtInfo.eventTime, pts, 6);
}

//_______________________________________________________________________________
void MFMCoboFrame::SetEventNumber(uint32_t eventnumber) {
	((MFM_cobo_header*) pHeader)->coboEvtInfo.eventIdx = eventnumber;
}
//_______________________________________________________________________________
//COBO
int MFMCoboFrame::CoboGetCoboIdx()const {
	return ((int) (((MFM_cobo_header*) pHeader)->coboBoard.coboIdx));
}
//_______________________________________
int MFMCoboFrame::CoboGetAsaIdx() const{
	return (int) (((MFM_cobo_header*) pHeader)->coboBoard.asaIdx);
}
//_______________________________________________________________________________
int MFMCoboFrame::CoboGetReadOffset() const{
	return (int) (((MFM_cobo_header*) pHeader)->coboBoard.readOffset);
}
//_______________________________________________________________________________
int MFMCoboFrame::CoboGetStatus() const{
	return (int) (((MFM_cobo_header*) pHeader)->coboBoard.status);
}

//_______________________________________________________________________________
char* MFMCoboFrame::CoboGetHitPat(int i) const {
	// 0<=i<=3
	return (char*) ((MFM_cobo_header*) pHeader)->coboHit.hitPat_0 + 9 * i;
}
//_______________________________________________________________________________
char* MFMCoboFrame::CoboGetMultip(int i) const{
	// 0<=i<=3
	return (char*) ((MFM_cobo_header*) pHeader)->coboHit.hitPat_0 + 9 * 4 + 2* i;
}

//_______________________________________________________________________________
uint32_t MFMCoboFrame::CoboGetWindowOut() const{
	return (uint32_t)(((MFM_cobo_header*) pHeader)->coboOut.windowOut);
}
//_______________________________________________________________________________
char* MFMCoboFrame::CoboGetLastCell(int i)const {
	// 0<=i<=3
	return(char*)((MFM_cobo_header*) pHeader)->coboOut.lastCell_0+2*i;
}
//_______________________________________________________________________________
void MFMCoboFrame::CoboSetCoboIdx(int coboidx) {
	((MFM_cobo_header*) pHeader)->coboBoard.coboIdx = coboidx;
}
//_______________________________________
void MFMCoboFrame::CoboSetAsaIdx(int asaidx) {
	((MFM_cobo_header*) pHeader)->coboBoard.asaIdx = asaidx;
}
//_______________________________________________________________________________
void MFMCoboFrame::CoboSetReadOffset(int offset) {
	((MFM_cobo_header*) pHeader)->coboBoard.readOffset = offset;
}
//_______________________________________________________________________________
void MFMCoboFrame::CoboSetStatus(int status) {
	((MFM_cobo_header*) pHeader)->coboBoard.status = status;
}

//_______________________________________________________________________________
void MFMCoboFrame::CoboSetHitPat(int i, char * hitpat) {
	memcpy(((MFM_cobo_header*) pHeader)->coboHit.hitPat_0 + 9 * i, hitpat, 9);
}

//_______________________________________________________________________________
void MFMCoboFrame::CoboSetMultip(int i, char* multip) {
	memcpy(((MFM_cobo_header*) pHeader)->coboHit.hitPat_0 + 9 * 4 + 2 * i,
			multip, 2);
}
//_______________________________________________________________________________
void MFMCoboFrame::CoboSetWindowOut(uint32_t windowsout) {
((MFM_cobo_header*) pHeader)->coboOut.windowOut = windowsout;
}

//_______________________________________________________________________________
void MFMCoboFrame::CoboSetLastCell(int i, char* cell) {
	memcpy(((MFM_cobo_header*) pHeader)->coboOut.lastCell_0+ 2 * i,cell, 2);
}
//_______________________________________________________________________________
void MFMCoboFrame::CoboGetParameters(int i, uint32_t *sample,
		uint32_t *buckidx, uint32_t *chanidx, uint32_t *agetidx) {
if (i==0)CoboRazCounts();

	if (GetFrameType() == MFM_COBO_FRAME_TYPE) {
		CoboGetParametersByItem((MFM_coboItem *) GetItem(i), sample, buckidx,
				chanidx, agetidx);
	}

	if (GetFrameType() == MFM_COBOF_FRAME_TYPE) {
		CoboGetParametersByItem((MFM_cobofItem *) GetItem(i), sample,  agetidx);

		// first method
		*chanidx = (((uint32_t)(i / (COBO_NB_AGET *2)))*2 + (i % 2))% COBO_NB_AGET_CHANNEL;
		*buckidx = ((uint32_t)(i / (COBO_NB_AGET * COBO_NB_AGET_CHANNEL)));

		//Second method
		//*chanidx =fChannelcount[*agetidx]++;
		//*buckidx= fBucketcount[*agetidx];

		if (fChannelcount[*agetidx]==(COBO_NB_AGET_CHANNEL)){
			 fChannelcount[*agetidx]=0;
			 fBucketcount[*agetidx]++;
			 }
	}
}
//_______________________________________________________________________________
void MFMCoboFrame::CoboGetParametersByItem(MFM_coboItem *item,uint32_t * sample,
		uint32_t *buckidx, uint32_t *chanidx,uint32_t *agetidx) {

	int type = GetFrameType();
	if (type == MFM_COBO_FRAME_TYPE) {
		uint32_t ui, ui2;
		ui2 = 0;
		char * pt_ui = (char*) &ui;
		memcpy(pt_ui, (char*) item, fItemSize);
		if (fLocalIsBigEndian != fFrameIsBigEndian)
			SwapInt32((uint32_t*) pt_ui, 4);

		//sample form 0 to 11
		ui2 = ui;
		ui2 = ui2 & COBO_SAMPLE_MASK;
		*sample = ui2;

		//buckdx from 14 to 22
		ui2 = ui;
		ui2 = ui2 >> 14;
		ui2 = ui2 & COBO_BUCKIDX_MASK;
		*buckidx = ui2;

		//chanidx from 23 to 29
		ui2 = ui;
		ui2 = ui2 >> 23;
		ui2 = ui2 & COBO_CHANIDX_MASK;
		*chanidx = ui2;

		//agetidx from 30 to 31
		ui2 = ui;
		ui2 = ui2 >> 30;
		ui2 = ui2 & COBO_AGETIDX_MASK;
		*agetidx = ui2;

	}else {
		fError.TreatError(2, 0,
				"MFMCoboFrame::CoboGetParametersByItem FrameType not MFM_COBO_FRAME_TYPE");
	}
}
//_______________________________________________________________________________
void MFMCoboFrame::CoboGetParametersByItem(MFM_cobofItem *item, uint32_t * sample, uint32_t *agetidx) {

	int type = GetFrameType();
	if (type == MFM_COBOF_FRAME_TYPE) {
		uint16_t ui;
		uint32_t ui2;
		ui2 = 0;
		*agetidx = 0;
		*sample = 0;
		char * pt_ui = (char*) &ui;
		memcpy(pt_ui, (char*) item, fItemSize);
		if (fLocalIsBigEndian != fFrameIsBigEndian)
			SwapInt16((uint16_t*) pt_ui);
		//sample form 0 to 11
		ui2 = ui;
		ui2 = ui2 & COBO_SAMPLE_MASK;
		*sample = ui2;

		//agetidx from  14 to 15
		ui2 = ui;
		ui2 = ui2 >> 14;
		ui2 = ui2 & COBO_AGETIDX_MASK;
		*agetidx = ui2;
	}else{
		fError.TreatError(2, 0,
				"MFMCoboFrame::CoboGetParametersByItem FrameType not MFM_COBOF_FRAME_TYPE");
	}
}
//_______________________________________________________________________________
void MFMCoboFrame::CoboRazCounts(){
for (int i=0;i>COBO_NB_AGET;i++){
	  fChannelcount[i]=0;
	  fBucketcount[i]=0;
	}
}
//_______________________________________________________________________________
void MFMCoboFrame::CoboSetParameters(uint32_t sample, uint32_t buckidx,
		uint32_t chanidx, uint32_t agetidx) {
	// to use only with MFM_COBOF_FRAME_TYPE

	int i = buckidx * (COBO_NB_AGET * COBO_NB_AGET_CHANNEL) + chanidx % 2
			+ (chanidx / 2) * (COBO_NB_AGET * 2) + 2 * agetidx;

	if (i >= GetDefinedNbItems()) {
		std::ostringstream tempos;
		tempos << "Index to hight " << i << " >= " << GetDefinedNbItems()
				<< "  agetidx = " << agetidx << "  chanidx = " << chanidx
				<< " " << "  buckidx = " << buckidx;
		fError.TreatError(2, 0, (tempos.str()).data());
	}
	CoboSetParametersByItem((MFM_cobofItem *) GetItem(i), sample, agetidx);
}
//_______________________________________________________________________________
void MFMCoboFrame::CoboSetParameters(int i, uint32_t sample, uint32_t buckidx,
		uint32_t chanidx, uint32_t agetidx) {
	int type = GetFrameType();

	if (type == MFM_COBO_FRAME_TYPE) {
		CoboSetParametersByItem((MFM_coboItem *) GetItem(i), sample, buckidx,
				chanidx, agetidx);
	}
	if (type == MFM_COBOF_FRAME_TYPE) {
		CoboSetParametersByItem((MFM_cobofItem *) GetItem(i), sample, agetidx);
	}
}
//_______________________________________________________________________________
void MFMCoboFrame::CoboSetParametersByItem(MFM_coboItem *item, uint32_t sample,
		uint32_t buckidx, uint32_t chanidx, uint32_t agetidx) {
	int type = GetFrameType();

	if (type == MFM_COBO_FRAME_TYPE) {

		// Methode to fill item in case of MFM_COBO_FRAME_TYPE frame
		uint32_t ui, ui2;
		ui2 = 0;

		//char * pt_ui = (char*) &ui;
		char * pt_ui2 = (char*) &ui2;
		//sample form 0 to 11
		ui2 = sample & COBO_SAMPLE_MASK;

		//buckdx from 14 to 22
		ui = buckidx & COBO_BUCKIDX_MASK;
		ui = ui << 14;
		ui2 = ui2 | ui;

		//chanidx from 23 to 29
		ui = chanidx & COBO_CHANIDX_MASK;
		ui = ui << 23;
		ui2 = ui2 | ui;

		//agetidx from 30 to 31
		ui = agetidx & COBO_AGETIDX_MASK;
		ui = ui << 30;
		ui2 = ui2 | ui;

		memcpy((char*) item, pt_ui2, fItemSize);
		}else{
			fError.TreatError(2,0,"MFMCoboFrame::CoboSetParametersByItem not a MFM_COBO_FRAME_TYPE ");
	}

}
//_______________________________________________________________________________
void MFMCoboFrame::CoboSetParametersByItem(MFM_cobofItem *item,
		uint32_t sample,  uint32_t agetidx) {
	int type = GetFrameType();

	if (type == MFM_COBOF_FRAME_TYPE) {
		uint16_t ui, ui2;
		ui2 = 0;

		char * pt_ui2 = (char*) &ui2;
		//sample form 0 to 11
		ui2 = sample & COBO_SAMPLE_MASK;

		//agetidx from 14 to 15
		ui = agetidx & COBO_AGETIDX_MASK;
		ui = ui << 14;
		ui2 = ui2 | ui;

		memcpy((char*) item, pt_ui2, fItemSize);

	}else{
		fError.TreatError(2,0,"MFMCoboFrame::CoboSetParametersByItem not a MFM_COBOF_FRAME_TYPE ");
	}
}
	
//_______________________________________________________________________________
void MFMCoboFrame::FillDataWithRamdomValue(  uint64_t timestamp, uint32_t enventnumber,int nbitem) {
		// fill data but in this case we fill with a rampe and not with random data
        uint32_t samplemax= COBO_NB_SAMPLES;
        uint32_t cobo_nb_sample = COBO_NB_SAMPLES; // il pourrait varier mais on le fixe poru la simul
        uint32_t channelmax= COBO_NB_AGET_CHANNEL;
        uint32_t agetmax = COBO_NB_AGET;
	uint32_t count;
	count = 0;
	
	SetEventNumber(enventnumber);
	SetTimeStamp(timestamp);
	int static increment =0;
	int i, j;
	uint32_t value;
	uint32_t asad;
	uint32_t type = GetWantedFrameType();
	asad = CoboGetAsaIdx();
	for (i = 0; i < COBO_NB_AGET; i++) {
		for (j = 0; j < COBO_NB_AGET_CHANNEL; j++) {
			fNbentries[i][j] = 0;
			fCoef[i][j] = (int) (rand()%23)+1;
		}
	}

	for (uint32_t ageti = 0; ageti < agetmax; ageti++) {
		for (uint32_t channeli = 0; channeli < channelmax; channeli++) {
			for (uint32_t bucketi = 0; bucketi < samplemax; bucketi++) {
				value = bucketi;
				value = (int)((((float)(bucketi))/((float)(fCoef[ageti][channeli]))));
				if (bucketi == 0)
					value = asad;
				if (bucketi == 1)
					value = ageti;
				if (bucketi == 2)
					value = channeli;
				if (bucketi == samplemax-1)
					value=samplemax;
				fNbentries[ageti][channeli]++;
				if (type == MFM_COBO_FRAME_TYPE) {
					CoboSetParameters(count, value, bucketi, channeli, ageti);
				}
				if (type == MFM_COBOF_FRAME_TYPE) {
					CoboSetParameters(value, bucketi, channeli, ageti);
				}
				count++;
			}
		}
	}

	for (i = 0; i < COBO_NB_AGET; i++) {
		for (j = 0; j < COBO_NB_AGET_CHANNEL; j++) {
			if ((uint32_t)fNbentries[i][j] > samplemax)
				cout << "Error count " << asad << " " << i << " " << j << " = "
						<< fNbentries[i][j] << "\n";
		}
	}
	CoboSetReadOffset(COBO_NB_SAMPLES-cobo_nb_sample);
	CoboSetAsaIdx((increment++) % COBO_NB_ASAD);
}

//_______________________________________________________________________________
void MFMCoboFrame::InitStat() {
	MFMCommonFrame::InitStat();
	fCountCoboFrame.reserve(8);
	fCountEmptyCoboFrame.reserve(8);
	int i;
    	for ( i= 0;i<8;i++){
	        fCountCoboFrame[i]=0;
	       	fCountEmptyCoboFrame[i]=0;
	        }
}
//____________________________________________________________________
void MFMCoboFrame::FillStat() {

	MFMCommonFrame::FillStat();
    	int cobo= CoboGetCoboIdx();
	int asad= CoboGetAsaIdx();
	int indice;
	indice = cobo*4*cobo +asad ;
	fCountCoboFrame[indice]++;
	int framesize =GetFrameSize();
	int headersize= GetHeaderSize();
	if (framesize==headersize)fCountEmptyCoboFrame[indice]++;
}
//____________________________________________________________________
string MFMCoboFrame::GetStat(string info  )const {
	string display("");
	stringstream ss("");
	ss <<MFMCommonFrame::GetStat( info);
	if (GetCountFrame()!=0)
	for ( int i=0;i<8; i++){
		ss << "Number of CoboFrame (cobo = "<<i/4<<", asad = "<<i%4<<") = "<<fCountCoboFrame[i]<<endl;
		ss << " included number of empty cobo frame ( headersize = framesize)"<<fCountEmptyCoboFrame[i]<<endl;
	}
	display = ss.str();
	return display;
}
//____________________________________________________________________
