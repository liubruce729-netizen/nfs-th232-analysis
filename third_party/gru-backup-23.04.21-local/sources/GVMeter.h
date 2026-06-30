// File :  GVMeter.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GVMeter
//
// This class manger is just to test Graphism of Root
//
/////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#ifndef __GVMeter__
#define __GVMeter__

#include "TSystem.h"
#include "TGFrame.h"
#include "TGWindow.h"
#include "TGSpeedo.h"
#include "TGTab.h"

class GVMeter: public TGMainFrame {

Float_t prev_load;
Int_t old_memUsage;

protected:
	const TGPicture *fBgnd; // picture used as mask
	TGSpeedo *fSpeedo; // analog meter
	TTimer *fTimer; // update timer
	Int_t fActInfo; // actual information value
	TGTab * testframe;
public:
	GVMeter(const TGWindow *p= NULL, int w = 500, int h=200);
	virtual ~GVMeter();
	void Update();
	

	void CloseWindow();
	TGSpeedo *GetSpeedo() const {
		return fSpeedo;
	}
	Int_t GetActInfo() const {
		return fActInfo;
	}
	void ToggleInfos();	

ClassDef(GVMeter, 1);
};

#endif