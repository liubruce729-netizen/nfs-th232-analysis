/*
 MFMS3eGUNFrame.cc
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

#include "MFMS3eGUNFrame.h"

//_______________________________________________________________________________
MFMS3eGUNFrame::MFMS3eGUNFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, int headerSize) {
	/// Constructor for a exogam frame . the header is filled with unitblock_size, data source , frame type , revision , frame, size and header size value
	SetPointers();
}

//_______________________________________________________________________________
MFMS3eGUNFrame::MFMS3eGUNFrame() {
	/// Constructor for a empty exogam frame
}
//_______________________________________________________________________________
MFMS3eGUNFrame::~MFMS3eGUNFrame() {
/// destructor of NumExo frame

}
//_______________________________________________________________________________
uint16_t MFMS3eGUNFrame::GetSetup(uint16_t i) {

	uint16_t setup = 0;
	/// Compute and return the 2 bytes of Setup
	if (i==1)
	setup = ((MFM_numexo_frame*) pHeader)->Data.Data1;
	if (i==2)
		setup = ((MFM_numexo_frame*) pHeader)->Data.Data2;
	if (i==3)
			setup = ((MFM_numexo_frame*) pHeader)->Data.Data3;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16((&setup));
	return ((setup));
}
//_______________________________________________________________________________
void MFMS3eGUNFrame::SetSetup(uint16_t i,uint16_t setup) {
	/// Set 16 bits of Setup
	if (i==1)
	((MFM_numexo_frame*) pHeader)->Data.Data1 = setup;
	if (i==2)
		((MFM_numexo_frame*) pHeader)->Data.Data2 = setup;
	if (i==3)
			((MFM_numexo_frame*) pHeader)->Data.Data3 = setup;
}

//_______________________________________________________________________________
 uint16_t  MFMS3eGUNFrame::GetCup(){
	uint16_t cup = 0;
	/// Compute and return the 2 bytes of cup
	cup = ((MFM_numexo_frame*) pHeader)->Data.Data4;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&cup);
	return (cup);
}
 //_______________________________________________________________________________
 void MFMS3eGUNFrame::SetCup(uint16_t cup) {
 	/// Set 16 bits of Status
 	((MFM_numexo_frame*) pHeader)->Data.Data4 = cup;
 }
//_______________________________________________________________________________
 void MFMS3eGUNFrame::SetGrid(uint16_t grid){
	((MFM_numexo_frame*) pHeader)->Data.Data5 = grid;
}
//_______________________________________________________________________________
 uint16_t  MFMS3eGUNFrame::GetGrid() {
	uint16_t grid = 0;
	/// Compute and return the 2 bytes of Grid
	grid = ((MFM_numexo_frame*) pHeader)->Data.Data5;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&grid);
	return (grid);
}
//_______________________________________________________________________________
void MFMS3eGUNFrame::FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	SetFrameType (MFM_S3_EGUN_FRAME_TYPE);
	SetSetup(1,16383);
	SetSetup(2,16382);
	SetSetup(3,16381);
	SetGrid((uint32_t) (enventnumber & 0x0000fff));
	SetCup((uint32_t) (enventnumber & 0x0000fff));
	MFMNumExoFrame::FillDataWithRamdomValue(timestamp,enventnumber);
}

//_______________________________________________________________________________
