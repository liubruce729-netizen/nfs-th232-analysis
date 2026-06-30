// File : GListDevice.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GListDevice
//
// This class is a basic class to manage  a list of device device
// (tape,file directory, network...
//
////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/


#include <TObject.h>
#include "GListDevice.h"
//______________________________________________________________________________

ClassImp( GListDevice);

GListDevice::GListDevice(void) {
	// Default constructor of device object;
	for (int i = 1; i < NB_OF_DEVICES; i++) {
		fListDevices[i] = NULL;
	}
}

//_____________________________________________________________________________

GListDevice::~GListDevice() {
	// destructor of GListDevice object

	ClearList();

}
//_____________________________________________________________________________

void GListDevice::ClearList() {
	for (int i = 1; i < NB_OF_DEVICES; i++) {
		if (fListDevices[i])
			delete fListDevices[i];
		fListDevices[i] = NULL;
	}
}
//_____________________________________________________________________________


