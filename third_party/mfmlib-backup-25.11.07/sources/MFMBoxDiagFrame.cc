/*
 MFMBoxDiagFrame.cc
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

#include "MFMBoxDiagFrame.h"

//_______________________________________________________________________________

void MFMBoxDiagFrame::SetStatus(int i, uint16_t status) {
	if (i < 0 and i > BOX_DIAG_NB_STATUS)
		cout << "MFMBoxDiagFrame::BoxDiagSetStatus Error of status index\n";
	if (i == 0)
		((MFM_BoxDiag_frame*) pHeader)->Data.Status1 = status;
	if (i == 1)
		((MFM_BoxDiag_frame*) pHeader)->Data.Status2 = status;
}
//_______________________________________________________________________________

uint16_t MFMBoxDiagFrame::GetStatus(int i) const{
	/// Set Status (0,1 or 2)
	uint16_t status;
	if (i < 0 and i > BOX_DIAG_NB_STATUS) {
		cout << "MFMBoxDiagFrame::ExGetStatus Error of status index\n";
		return 0;
	} else {
		if (i == 0)
			status = (((MFM_BoxDiag_frame*) pHeader)->Data.Status1);
		if (i == 1)
			status = (((MFM_BoxDiag_frame*) pHeader)->Data.Status2);
	}
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&status);
	return status;
}
//_______________________________________________________________________________
string MFMBoxDiagFrame::GetHeaderDisplay(char* infotext) const {
/// Return a string containing infomation of MFM Header\n
	/// if infotext is not NULL replace the standart "MFM header" title
	stringstream ss;
	ss.str("");
	string display("");
	ss << MFMCommonFrame::GetHeaderDisplay(infotext);
	display = ss.str();
	return display;
	}
//_______________________________________________________________________________

void MFMBoxDiagFrame::SetTimeStop(uint16_t timestop) {
	/// Set Time
	((MFM_BoxDiag_frame*) pHeader)->Data.TimeStop= timestop;
}
//_______________________________________________________________________________
void MFMBoxDiagFrame::SetUserDataPointer(){
	pUserData_char = (char*) &(((MFM_BoxDiag_frame*) pHeader)->Data);
}
//_______________________________________________________________________________

uint16_t MFMBoxDiagFrame::GetTimeStop() const{
	/// Get Time
	uint16_t time;
	time = (((MFM_BoxDiag_frame*) pHeader)->Data.TimeStop);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&time);
	return time;
}
//_______________________________________________________________________________

void MFMBoxDiagFrame::SetEnergy(uint16_t energy) {
	/// Set Energy
	((MFM_BoxDiag_frame*) pHeader)->Data.Energy = energy;
}

//_______________________________________________________________________________

uint16_t MFMBoxDiagFrame::GetEnergy() const{
	/// Get Energy
	uint16_t energy;
	energy = (((MFM_BoxDiag_frame*) pHeader)->Data.Energy);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&energy);
	return energy;
}
//_______________________________________________________________________________

void MFMBoxDiagFrame::SetTime(uint16_t time) {
	/// Set Time
	((MFM_BoxDiag_frame*) pHeader)->Data.Time= time;
}
//_______________________________________________________________________________

uint16_t MFMBoxDiagFrame::GetTime() const{
	/// Get Time
	uint16_t time;
	time = (((MFM_BoxDiag_frame*) pHeader)->Data.Time);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&time);
	return time;
}
//_______________________________________________________________________________
/*
void MFMBoxDiagFrame::SetChecksum(uint16_t cksum) {
	/// Set Checksum data
	(((MFM_BoxDiag_frame*) pHeader)->Data.Checksum) = cksum;
}
//_______________________________________________________________________________

uint16_t MFMBoxDiagFrame::GetChecksum() const{
	/// Compute and return value of Checksum
	uint16_t cksum;
	cksum = (((MFM_BoxDiag_frame*) pHeader)->Data.Checksum);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&cksum);
	return cksum;
}*/
//_______________________________________________________________________________
void MFMBoxDiagFrame::FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	MFMNumExoFrame::FillDataWithRamdomValue( timestamp, enventnumber);
	int value = random();
	uint16_t uivalue = (uint16_t) (4294967296 * value / RAND_MAX);
	SetTimeStop(uivalue+2);
	SetEnergy(uivalue);
	SetTime(uivalue+1);
}
//_______________________________________________________________________________



