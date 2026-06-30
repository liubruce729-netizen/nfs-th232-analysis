// File : GVSetPeakFind.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////
//
// Class GVSetPeakFind
//   Box  of GV application.
//   Set Paremeter to setup Peak find
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


#include "GVSetPeakFind.h"
#include "Vigru.h"
#include <TGLayout.h>
#include <TVirtualPadEditor.h>
#include <TGMsgBox.h>
#include "math.h"
#include <TSystem.h>
#include <TROOT.h>
#include <TGClient.h>
#include <TStyle.h>
ClassImp(GVSetPeakFind)

GVSetPeakFind::GVSetPeakFind(const TGWindow *p, const TGWindow *m, UInt_t w, UInt_t h,
		TGFrame *tg,Int_t* nbPeaks,Float_t *resolution ,Double_t *sigma,Double_t *threshold,bool *display_polymarker) :
TGTransientFrame(p, m, w, h) {

	fnbPeaks= nbPeaks;
	fresolution= resolution;
	fsigma=sigma;
	fdisplay_polymarker=display_polymarker;
	fthreshold=threshold;

	fMain = (Vigru*)m;
	fAboveFrame = (TGMenuBar*)tg;
	SetWindowName("Peak Find");
	SetLayoutManager(new TGVerticalLayout(this));


	fGroup = new TGGroupFrame(this,"Set Peak Find Parameters");
	fGroup->SetLayoutManager(new TGMatrixLayout(fGroup,6,2,10));
	AddFrame(fGroup);

	fLabel1 = new TGLabel(fGroup,"Nb of expected peaks ");
	fGroup->AddFrame(fLabel1);
	fNumberEntryNbPeak = new TGNumberEntry(fGroup);
	fGroup->AddFrame(fNumberEntryNbPeak);

	fLabel2 = new TGLabel(fGroup,"Sigma ");
	fGroup->AddFrame(fLabel2);
	fNumberEntrySigma = new TGNumberEntry(fGroup);
	fGroup->AddFrame(fNumberEntrySigma);

	fLabel3 = new TGLabel(fGroup,"Threshold ");
	fGroup->AddFrame(fLabel3);
	fNumberEntryThreshold = new TGNumberEntry(fGroup);
	fGroup->AddFrame(fNumberEntryThreshold);

	fLabel4 = new TGLabel(fGroup,"Resolution ");
	fGroup->AddFrame(fLabel4);
	fNumberEntryResolution = new TGNumberEntry(fGroup);
	fGroup->AddFrame(fNumberEntryResolution);

	fChecks = new TGCheckButton(fGroup,"Display PolyMarker");
	fGroup->AddFrame(fChecks);
	fLabel5 = new TGLabel(fGroup,"");
	fGroup->AddFrame(fLabel5);

	fNumberEntryNbPeak->SetNumber((*nbPeaks));
	fNumberEntryResolution->SetNumber((*resolution));
	fNumberEntrySigma->SetNumber((*sigma));
	fNumberEntryThreshold->SetNumber((*threshold)) ;

	if(*fdisplay_polymarker)
		fChecks->SetState(kButtonDown);
	else
		fChecks->SetState(kButtonUp);

	fButtonOk = new TGTextButton(fGroup,"Ok" );
	fButtonOk->Resize(50, 30);
	fButtonOk ->Connect("Clicked()", "GVSetPeakFind", this, "DoOk() ");
	fGroup->AddFrame(fButtonOk);

	fButtonCancel = new TGTextButton(fGroup,"Cancel");
	fButtonCancel->Resize(50, 30);
	fButtonCancel->Connect("Clicked()", "GVSetPeakFind", this, "DoCancel()");
	fGroup->AddFrame(fButtonCancel);

	Layout();
	MapSubwindows();
	CenterOnParent();
	MapWindow();
	GetClient()->WaitFor(this);
}

GVSetPeakFind::~GVSetPeakFind() {
	Cleanup();
	// all object added with AddFrame are already removed with Cleanup
}

void GVSetPeakFind::DoOk() {

	(*fnbPeaks)    = fNumberEntryNbPeak->GetIntNumber();
	(*fresolution) = fNumberEntryResolution->GetNumber();
	(*fsigma)      = fNumberEntrySigma->GetNumber() ;
	(*fthreshold)  = fNumberEntryThreshold->GetNumber() ;
	if (fChecks->IsOn())
		(*fdisplay_polymarker) = true;
	else
		(*fdisplay_polymarker) = false;

	DoQuit();

}

void GVSetPeakFind::DoCancel() {

	DoQuit();

}
void GVSetPeakFind::DoQuit() {

	TTimer::SingleShot(150, "GVSetPeakFind", this, "CloseWindow()");

	// Close the Ged editor if it was activated.
	if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
		TVirtualPadEditor::Terminate();
}
void GVSetPeakFind::CloseWindow() {

	UnmapWindow();

	//	delete (this)-> but a
	DeleteWindow();

}
