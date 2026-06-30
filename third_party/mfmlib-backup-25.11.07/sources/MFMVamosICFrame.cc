/*
 MFMVamosICFrame.cc
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

#include "MFMVamosICFrame.h"

//_______________________________________________________________________________

void MFMVamosICFrame::SetStatus(int i, uint16_t status) {
	if (i < 0 and i > VAMOSIC_NB_STATUS)
		cout << "MFMVamosICFrame::VamosICSetStatus Error of status index\n";
	if (i == 0)
		((MFM_ICvamos_frame*) pHeader)->Data.Status1 = status;
	if (i == 1)
		((MFM_ICvamos_frame*) pHeader)->Data.Status2 = status;
}
//_______________________________________________________________________________

uint16_t MFMVamosICFrame::GetStatus(int i) const{
	/// Set Status (0,1 or 2)
	uint16_t status;
	if (i < 0 and i > VAMOSIC_NB_STATUS) {
		cout << "MFMVamosICFrame::ExGetStatus Error of status index\n";
		return 0;
	} else {
		if (i == 0)
			status = (((MFM_ICvamos_frame*) pHeader)->Data.Status1);
		if (i == 1)
			status = (((MFM_ICvamos_frame*) pHeader)->Data.Status2);
	}
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&status);
	return status;
}

//_______________________________________________________________________________

void MFMVamosICFrame::SetEnergy(uint16_t energy) {
	/// Set Energy
	((MFM_ICvamos_frame*) pHeader)->Data.Energy = energy;
}

//_______________________________________________________________________________

uint16_t MFMVamosICFrame::GetEnergy() const{
	/// Get Energy
	uint16_t energy;
	energy = (((MFM_ICvamos_frame*) pHeader)->Data.Energy);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&energy);
	return energy;
}

//_______________________________________________________________________________
void MFMVamosICFrame::FillEventRandomConst(uint64_t timestamp,
		uint32_t enventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	int value = random();
	uint16_t uivalue = (uint16_t) (4294967296 * value / RAND_MAX);
	SetEnergy(uivalue);
	SetCristalId(8,112);
	SetEventNumber(enventnumber);
	SetTimeStamp(timestamp);
}
//____________________________________________________________________
void MFMVamosICFrame::FillStat() {
	MFMCommonFrame::FillStat();
	uint16_t id ;
	id = GetCristalId();
	fCountNbEventCard[id]++;

}
//_______________________________________________________________________________
void MFMVamosICFrame::InitStat() {
	MFMCommonFrame::InitStat();
	int i;
	fCountNbEventCard = new long long[65536];
	for ( i = 0;i<65536;i++){
		fCountNbEventCard[i]=0;
	}

}
//____________________________________________________________________
string  MFMVamosICFrame::GetStat(string info)const{

	string display("");
	stringstream ss("");
	ss << MFMCommonFrame::GetStat( info);
    int i, j; int total =0;

	for ( i=0;i<65536;i++ ){

		if (fCountNbEventCard[i]!=0){
			j =i;
			ss << "Card "<< ((j>>5) & NUMEXO_CRYS_MASK);
			j =i;
			ss << " Cristal  "<< (j& NUMEXO_BOARD_ID_MASK );
			ss << " NbEvents = "<< fCountNbEventCard[i] <<"\n";
			total += fCountNbEventCard[i];
		}
	}
	ss<<"Total MFMVamosICFrame       = "<< total<<"\n";
	display = ss.str();
	return display;
}

