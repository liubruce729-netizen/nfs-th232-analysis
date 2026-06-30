/*
 MFMS3AlphaFrame.cc
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

#include "MFMS3AlphaFrame.h"

//_______________________________________________________________________________
MFMS3AlphaFrame::MFMS3AlphaFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, int headerSize) {
	/// Constructor for a exogam frame . the header is filled with unitblock_size, data source , frame type , revision , frame, size and header size value
	SetPointers();
}

//_______________________________________________________________________________
MFMS3AlphaFrame::MFMS3AlphaFrame() {
	/// Constructor for a empty exogam frame
}
//_______________________________________________________________________________
MFMS3AlphaFrame::~MFMS3AlphaFrame() {
/// destructor of NumExo frame

}
//_______________________________________________________________________________
uint16_t MFMS3AlphaFrame::GetSetup(uint16_t i) {

	uint16_t setup = 0;
	/// Compute and return the 2 bytes of CristalId()
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
void MFMS3AlphaFrame::SetSetup(uint16_t i,uint16_t setup) {
	/// Set 16 bits of Setup
	if (i==1)
	((MFM_numexo_frame*) pHeader)->Data.Data1 = setup;
	if (i==2)
		((MFM_numexo_frame*) pHeader)->Data.Data2 = setup;
	if (i==3)
			((MFM_numexo_frame*) pHeader)->Data.Data3 = setup;
}

//_______________________________________________________________________________
 uint16_t  MFMS3AlphaFrame::GetStatus(){
	uint16_t status = 0;
	/// Compute and return the 2 bytes of CristalId()
	status = ((MFM_numexo_frame*) pHeader)->Data.Data4;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16((&status));
	return ((status));
}
 //_______________________________________________________________________________
 void MFMS3AlphaFrame::SetStatus(uint16_t status) {
 	/// Set 16 bits of Status
 	((MFM_numexo_frame*) pHeader)->Data.Data4 = status;
 }
//_______________________________________________________________________________
 void      MFMS3AlphaFrame::SetEnergy(uint16_t energy){
	((MFM_numexo_frame*) pHeader)->Data.Data5 = energy;
}
//_______________________________________________________________________________
 uint16_t  MFMS3AlphaFrame::GetEnergy() {
	uint16_t energy = 0;
	/// Compute and return the 2 bytes of CristalId()
	energy = ((MFM_numexo_frame*) pHeader)->Data.Data5;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16((&energy));
	return ((energy));
}
//_______________________________________________________________________________
void MFMS3AlphaFrame::FillDataWithRamdomValue(uint64_t timestamp,
		uint32_t enventnumber) {

	/// Fill all data of frame with random or constant values to do test
	/// And report time stamp and event number
	SetFrameType (MFM_S3_ALPHA_FRAME_TYPE);
	SetStatus(1);
	SetSetup(1,16383);
	SetSetup(2,16382);
	SetSetup(3,16381);
	SetEnergy((uint32_t) enventnumber & 0x0000fff);
	MFMNumExoFrame::FillDataWithRamdomValue(timestamp,enventnumber);
}

//_______________________________________________________________________________
