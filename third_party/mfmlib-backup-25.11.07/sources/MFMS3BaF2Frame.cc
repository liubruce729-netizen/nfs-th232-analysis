/*
 MFMS3BaF2Frame.cc
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

#include "MFMS3BaF2Frame.h"

//_______________________________________________________________________________
MFMS3BaF2Frame::MFMS3BaF2Frame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, int headerSize) {
	/// Constructor for a exogam frame . the header is filled with unitblock_size, data source , frame type , revision , frame, size and header size value
	SetPointers();
}

//_______________________________________________________________________________
MFMS3BaF2Frame::MFMS3BaF2Frame() {
	/// Constructor for a empty exogam frame
}
//_______________________________________________________________________________
MFMS3BaF2Frame::~MFMS3BaF2Frame() {
/// destructor of NumExo frame

}
//_______________________________________________________________________________
uint16_t MFMS3BaF2Frame::GetSetup() {
	uint16_t setup = 0;
	/// Compute and return the 2 bytes of CristalId()
	setup = ((MFM_numexo_frame*) pHeader)->Data.Data1;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16((&setup));
	return ((setup));
}
//_______________________________________________________________________________
void MFMS3BaF2Frame::SetSetup(uint16_t setup) {
	/// Set 16 bits of Setup
	((MFM_numexo_frame*) pHeader)->Data.Data1 = setup;
}

//_______________________________________________________________________________
 uint16_t  MFMS3BaF2Frame::GetStatus(){
	uint16_t status = 0;
	/// Compute and return the 2 bytes of CristalId()
	status = ((MFM_numexo_frame*) pHeader)->Data.Data4;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16((&status));
	return ((status));
}
 //_______________________________________________________________________________
 void MFMS3BaF2Frame::SetStatus(uint16_t status) {
 	/// Set 16 bits of Status
 	((MFM_numexo_frame*) pHeader)->Data.Data4 = status;
 }
//_______________________________________________________________________________
 void      MFMS3BaF2Frame::SetEnergy(uint16_t energy){
	((MFM_numexo_frame*) pHeader)->Data.Data5 = energy;
}
//_______________________________________________________________________________
 uint16_t  MFMS3BaF2Frame::GetEnergy() {
	uint16_t energy = 0;
	/// Compute and return the 2 bytes of CristalId()
	energy = ((MFM_numexo_frame*) pHeader)->Data.Data5;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16((&energy));
	return ((energy));
}
//_______________________________________________________________________________
void MFMS3BaF2Frame::FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	SetFrameType (MFM_S3_BAF2_FRAME_TYPE);
	SetStatus(1);
	SetSetup(16383);
	((MFM_numexo_frame*) pHeader)->Data.Data2 = 0;
	((MFM_numexo_frame*) pHeader)->Data.Data3 = 0;
	SetEnergy((uint32_t) (enventnumber & 0x0000fff));
	MFMNumExoFrame::FillDataWithRamdomValue(timestamp,enventnumber);
}

//_______________________________________________________________________________
