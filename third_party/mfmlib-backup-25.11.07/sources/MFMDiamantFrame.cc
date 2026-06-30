/*
 MFMDiamantFrame.cc
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

#include "MFMDiamantFrame.h"

//_______________________________________________________________________________
MFMDiamantFrame::MFMDiamantFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, int headerSize) {
	/// Constructor . the header is filled with unitblock_size, data source , frame type , revision , frame, size and header size value
	SetPointers();
}

//_______________________________________________________________________________
MFMDiamantFrame::MFMDiamantFrame() {
	/// Constructor for a empty exogam frame
}
//_______________________________________________________________________________
MFMDiamantFrame::~MFMDiamantFrame() {
/// destructor of NumExo frame

}
//_______________________________________________________________________________

void MFMDiamantFrame::SetStatus(int i, uint16_t status) {
	if (i == 0){
		((MFM_dia_frame*) pHeader)->Data.Status1 = status;
		return;
	}
	if (i == 1){
		((MFM_dia_frame*) pHeader)->Data.Status2 = status;
		return;
	}
	cout << "MFMDiamantFrame::SetStatus Error of status index\n";
}
//_______________________________________________________________________________
void MFMDiamantFrame::SetUserDataPointer(){
pUserData_char = (char*) &(((MFM_dia_frame*) pHeader)->Data);
}
//_______________________________________________________________________________

uint16_t MFMDiamantFrame::GetStatus(int i)const {
	/// Set Status (0,1 or 2)
	uint16_t status;
	status =0;

	if (i < 0 and i > DIA_NB_STATUS) {
		cout << "MFMDiamantFrame::ExGetStatus Error of status index\n";
		return 0;
	} else {
		if (i == 0)
			status =(((MFM_dia_frame*) pHeader)->Data.Status1) ;

		if (i == 1)
			status = (((MFM_dia_frame*) pHeader)->Data.Status2);
	}
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&status);
	return status;
}

//_______________________________________________________________________________
void MFMDiamantFrame::SetEnergy(uint32_t energy) {
	/// Set Energy
		(((MFM_dia_frame*) pHeader)->Data.Energy) = energy;
}

//_______________________________________________________________________________
uint32_t MFMDiamantFrame::GetEnergy() const{
	/// GetEnergy
	uint32_t energy;
	energy=(((MFM_dia_frame*) pHeader)->Data.Energy);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt32(&energy);
	return energy;
}

//_______________________________________________________________________________

void MFMDiamantFrame::SetTop(uint32_t top) {
	/// Set Top in frame
	(((MFM_dia_frame*) pHeader)->Data.Top) = top;
}
//_______________________________________________________________________________

uint32_t MFMDiamantFrame::GetTop() const{
	/// computer and return  Top value from frame
	uint32_t top;
	top = (((MFM_dia_frame*) pHeader)->Data.Top);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt32(&top);
	return top;
}
//_______________________________________________________________________________
void MFMDiamantFrame::FillDataWithRamdomValue(uint64_t timestamp,
		uint32_t eventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	MFMNumExoFrame::FillDataWithRamdomValue(timestamp,eventnumber);
	float maxuint16 = pow(2,16);
	float maxuint32 = pow(2,32);
	float value = random();
	uint16_t uivalue16 = (uint16_t) (maxuint16 * (float)(value / RAND_MAX));
	uint32_t uivalue32 = (uint32_t) (maxuint32 * (float)(value / RAND_MAX));
	SetStatus(0,1);
	SetStatus(1,1);
	SetEnergy(uivalue32);
	SetTop(uivalue32 +1);
	SetChecksum(uivalue16);
	SetFrameType (MFM_DIAMANT_FRAME_TYPE);
}
//_______________________________________________________________________________
string MFMDiamantFrame::GetDumpData(char mode, bool nozero)  const {
	// Dump parameter Label and parameter value of the current event.
	// if enter parameter is true (default value), all zero parameter of event aren't dumped
	// mode = 'd' for decimal, 'b' for binary, 'h' for hexa, 'o' for octal

	stringstream ss;
	string display("");
        ss << "Energy = " <<  GetEnergy() <<  "  Top = " << GetTop() <<  "  Status1 = " << GetStatus(0) <<  "  Status2 = " << GetStatus(1)<<endl ; 
	
	display = ss.str();
	return display;
}
//_______________________________________________________________________________
