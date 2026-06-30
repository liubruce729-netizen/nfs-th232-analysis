/*
 MFMS3SynchroFrame.cc
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

#include "MFMS3SynchroFrame.h"

//_______________________________________________________________________________
MFMS3SynchroFrame::MFMS3SynchroFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, int headerSize) {
	/// Constructor for a exogam frame . the header is filled with unitblock_size, data source , frame type , revision , frame, size and header size value
	SetPointers();
}

//_______________________________________________________________________________
MFMS3SynchroFrame::MFMS3SynchroFrame() {
	/// Constructor for a empty exogam frame
}
//_______________________________________________________________________________
MFMS3SynchroFrame::~MFMS3SynchroFrame() {
/// destructor of NumExo frame

}
//_______________________________________________________________________________

uint64_t MFMS3SynchroFrame::GetPeriod() {
	uint64_t period;
	char* ptperiod ;
	ptperiod= (char*)&period;

	memcpy(((char*) (&ptperiod)),
				((MFM_syncho_frame*) pHeader)->Data.Period,6);

	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt64(((uint64_t* )(&ptperiod)),6);
	return ((period));
}
//_______________________________________________________________________________
void MFMS3SynchroFrame::SetPeriod(uint64_t period) {
	/// Set 16 bits of Setup
	char* pts = (char*) &period;
	period = period & 0x0000ffffffffffff;
	memcpy(((MFM_syncho_frame*) pHeader)->Data.Period, pts, 6);

}

//_______________________________________________________________________________
void MFMS3SynchroFrame::FillDataWithRamdomValue(uint64_t timestamp,
		uint32_t enventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	SetFrameType (MFM_S3_SYNC_FRAME_TYPE);
	((MFM_syncho_frame*) pHeader)->Data.Nothing1 = 0;
	((MFM_syncho_frame*) pHeader)->Data.Nothing2 = 0;
	SetPeriod(timestamp);
	MFMNumExoFrame::FillDataWithRamdomValue(timestamp,enventnumber);
}

//_______________________________________________________________________________
