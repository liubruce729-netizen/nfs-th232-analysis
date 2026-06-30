/*
 MFMS3RuthFrame.cc
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

#include "MFMS3RuthFrame.h"

//_______________________________________________________________________________
MFMS3RuthFrame::MFMS3RuthFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, int headerSize) {
	/// Constructor for a exogam frame . the header is filled with unitblock_size, data source , frame type , revision , frame, size and header size value
	SetPointers();
}

//_______________________________________________________________________________
MFMS3RuthFrame::MFMS3RuthFrame() {
	/// Constructor for a empty exogam frame
}
//_______________________________________________________________________________
MFMS3RuthFrame::~MFMS3RuthFrame() {
/// destructor of NumExo frame

}

//_______________________________________________________________________________
void MFMS3RuthFrame::FillDataWithRamdomValue(uint64_t timestamp,
		uint32_t enventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	SetFrameType (MFM_S3_RUTH_FRAME_TYPE);
	SetStatus(1);
	SetSetup(1,16383);
	SetSetup(2,16382);
	SetSetup(3,16381);
	SetEnergy((uint32_t) enventnumber & 0x0000fff);
	MFMNumExoFrame::FillDataWithRamdomValue(timestamp,enventnumber);
}

//_______________________________________________________________________________
