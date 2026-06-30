/*
 MFMVamosTACFrame.cc
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

#include "MFMVamosTACFrame.h"

//_______________________________________________________________________________

void MFMVamosTACFrame::SetStatus(int i, uint16_t status) {
	if (i < 0 and i > VAMOSTAC_NB_STATUS)
		cout << "MFMVamosTACFrame::VamosTACSetStatus Error of status index\n";
	if (i == 0)
		((MFM_TACvamos_frame*) pHeader)->Data.Status1 = status;
	if (i == 1)
		((MFM_TACvamos_frame*) pHeader)->Data.Status2 = status;
}
//_______________________________________________________________________________

uint16_t MFMVamosTACFrame::GetStatus(int i) const{
	/// Set Status (0,1 or 2)
	uint16_t status;
	if (i < 0 and i > VAMOSTAC_NB_STATUS) {
		cout << "MFMVamosTACFrame::ExGetStatus Error of status index\n";
		return 0;
	} else {
		if (i == 0)
			status = (((MFM_TACvamos_frame*) pHeader)->Data.Status1);
		if (i == 1)
			status = (((MFM_TACvamos_frame*) pHeader)->Data.Status2);
	}
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&status);
	return status;
}

//_______________________________________________________________________________

void MFMVamosTACFrame::SetTime(uint16_t time) {
	/// Set Time
	((MFM_TACvamos_frame*) pHeader)->Data.Time= time;
}

//_______________________________________________________________________________

uint16_t MFMVamosTACFrame::GetTime() const{
	/// Get Time
	uint16_t time;
	time = (((MFM_TACvamos_frame*) pHeader)->Data.Time);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&time);
	return time;
}
//_______________________________________________________________________________

void MFMVamosTACFrame::SetChecksum(uint16_t cksum) {
	/// Set Checksum data
	(((MFM_TACvamos_frame*) pHeader)->Data.Checksum) = cksum;
}
//_______________________________________________________________________________

uint16_t MFMVamosTACFrame::GetChecksum() const{
	/// Compute and return value of Checksum
	uint16_t cksum;
	cksum = (((MFM_TACvamos_frame*) pHeader)->Data.Checksum);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&cksum);
	return cksum;
}

//_______________________________________________________________________________
void MFMVamosTACFrame::FillDataWithRamdomValue(uint64_t timestamp,
		uint32_t eventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	int value = random();
	uint16_t uivalue = (uint16_t) (4294967296 * value / RAND_MAX);
	SetTime(uivalue);
	SetCristalId(8,112);
	SetEventNumber(eventnumber);
	SetTimeStamp(timestamp);
}

//_______________________________________________________________________________

void MFMVamosTACFrame::FillStat() {
	MFMCommonFrame::FillStat();
	uint16_t id ;
	id = GetCristalId();
	fCountNbEventCard[id]++;

}
//_______________________________________________________________________________
void MFMVamosTACFrame::InitStat() {
	MFMCommonFrame::InitStat();
	int i;
	fCountNbEventCard = new long long[65536];
	for ( i = 0;i<65536;i++){
		fCountNbEventCard[i]=0;
	}
}
//_______________________________________________________________________________
string  MFMVamosTACFrame::GetStat(string info)const{

	string display("");
	stringstream ss("");
	ss << MFMCommonFrame::GetStat( info);
    int i, j; int total =0;

	for ( i=0;i<65536;i++ ){

		if (fCountNbEventCard[i]!=0){
			j =i;
			ss << "Card "<< ((j>>5) & NUMEXO_BOARD_ID_MASK);
			j =i;
			ss << " Cristal  "<< (j& NUMEXO_CRYS_MASK );
			ss << " NbEvents = "<< fCountNbEventCard[i] <<"\n";
			total += fCountNbEventCard[i];
		}
	}
	ss<<"Total MFMVamosTACFrame       = "<< total<<"\n";
	display = ss.str();
	return display;
}
//_______________________________________________________________________________


