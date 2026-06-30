/*
 MFMS3DeflectorFrame.cc
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

#include "MFMS3DeflectorFrame.h"

//_______________________________________________________________________________
MFMS3DeflectorFrame::MFMS3DeflectorFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, int headerSize) {
	/// Constructor for a frame . the header is filled with unitblock_size, data source , frame type , revision , frame, size and header size value
	SetPointers();
}
//_______________________________________________________________________________
MFMS3DeflectorFrame::MFMS3DeflectorFrame() {
	/// Constructor for a empty exogam frame
}
//_______________________________________________________________________________
MFMS3DeflectorFrame::~MFMS3DeflectorFrame() {
/// destructor of NumExo frame

}
//_______________________________________________________________________________
 void MFMS3DeflectorFrame::SetDeflector(uint16_t energy){
	((MFM_numexo_frame*) pHeader)->Data.Data5 = energy;
}
//_______________________________________________________________________________
 uint16_t  MFMS3DeflectorFrame::GetDeflector() {
	uint16_t energy = 0;
	/// Compute and return the 2 bytes of CristalId()
	energy = ((MFM_numexo_frame*) pHeader)->Data.Data5;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16((&energy));
	return ((energy));
}
//_______________________________________________________________________________
void MFMS3DeflectorFrame::FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	SetFrameType (MFM_S3_DEFLECTOR_FRAME_TYPE);
	SetDeflector((uint32_t) (enventnumber & 0x0000fff));
	MFMNumExoFrame::FillDataWithRamdomValue(timestamp,enventnumber);
}

//_______________________________________________________________________________
