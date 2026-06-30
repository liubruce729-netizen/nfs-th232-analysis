// File : GVSetDuplication.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////
//
// Class GVSetDuplication
//   Box  of GV application.
//   Ask the king of Computation have to be done
//   and form which pad ( page, no pad)
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


#include "GVSetDuplication.h"
#include "GVPad.h"
#include "Vigru.h"
#include <TGLayout.h>
#include <TVirtualPadEditor.h>
#include <TGNumberEntry.h>
#include <TGMsgBox.h>
#include "math.h"
#include <TSystem.h>
#include <TROOT.h>
#include <TGClient.h>
#include <TStyle.h>
#include "GVPad.h"

ClassImp( GVSetDuplication)

GVSetDuplication::GVSetDuplication(const TGWindow *p, const TGWindow *m, UInt_t w,
		UInt_t h, Int_t* tab, Int_t* pad, Int_t* computation) :
	TGTransientFrame(p, m, w, h) {

	Int_t padmax, padmin, tabmax, tabmin;
	padmax = 150;
	padmin = 1;
	tabmax = 100;
	tabmin = 0;

	fComputation = *computation;
	fTab = *tab;
	fPad = *pad;
	fpComputation = computation;
	fpTab = tab;
	fpPad = pad;

	fMain = (Vigru*) m;

	SetWindowName("Configuration of Duplication");
	SetLayoutManager(new TGVerticalLayout(this));

	fGroup = new TGGroupFrame(this,
			"Define computation to apply and form which pad");

	fGroup->SetLayoutManager(new TGMatrixLayout(fGroup, 3, 3, 10));
	fGroup->AddFrame(new TGLabel(fGroup, "Computation"));
	fGroup->AddFrame(new TGLabel(fGroup, "Tab"));
	fGroup->AddFrame(new TGLabel(fGroup, "Pad"));
	fComboComputation = new TGComboBox(fGroup);
	fGroup->AddFrame(fComboComputation);
	fComboTab = new TGNumberEntry(fGroup);
	//fComboTab->SetNumLimits(TGNumberFormat::kNESInteger);
	fGroup->AddFrame(fComboTab);
	fComboPad = new TGNumberEntry(fGroup);
	//fComboPad->SetNumLimits(TGNumberFormat::kNESInteger);
	fGroup->AddFrame(fComboPad);

	fComboTab->SetIntNumber(0);
	fComboPad->SetIntNumber(1);

	const char *ComputationTextList[] = { "No Computation", "Standard Refresh", "Copy",
			"FFT", "FFThalf","Scatter","ZeroLess","ProjectionX","ProjectionY","ProfileX","ProfileY","User", 0 };

	fComboComputation->AddEntry(ComputationTextList[0],  GVPad::COMPUTATION_NULL);
	fComboComputation->AddEntry(ComputationTextList[1],  GVPad::COMPUTATION_REFRESH);
	fComboComputation->AddEntry(ComputationTextList[2],  GVPad::COMPUTATION_COPY);
	fComboComputation->AddEntry(ComputationTextList[3],  GVPad::COMPUTATION_FFT);
	fComboComputation->AddEntry(ComputationTextList[4],  GVPad::COMPUTATION_FFThalf);
	fComboComputation->AddEntry(ComputationTextList[5],  GVPad::COMPUTATION_SCATTER);
	fComboComputation->AddEntry(ComputationTextList[6],  GVPad::COMPUTATION_ZEROLESS);
	fComboComputation->AddEntry(ComputationTextList[7],  GVPad::COMPUTATION_PROJECTIONX);
	fComboComputation->AddEntry(ComputationTextList[8],  GVPad::COMPUTATION_PROJECTIONY);
	fComboComputation->AddEntry(ComputationTextList[9],  GVPad::COMPUTATION_PROFILEX);
	fComboComputation->AddEntry(ComputationTextList[10], GVPad::COMPUTATION_PROFILEY);
	fComboComputation->AddEntry(ComputationTextList[11], GVPad::COMPUTATION_USER);

	fComboPad    ->Resize(60, 20);
	fComboTab    ->Resize(60, 20);
	fComboComputation->Resize(100, 20);

	if (fPad > padmax)
		fPad = padmax;
	if (fTab > tabmax)
		fTab = tabmax;
	if (fPad < padmin)
		fPad = padmin;
	if (fTab > tabmin)
		fTab = tabmin;

	fGroup-> AddFrame(new TGLabel(fGroup, ""));
	fButtonOk = new TGTextButton(fGroup, "Ok");
	fButtonOk->Resize(50, 30);
	fButtonOk ->Connect("Clicked()", "GVSetDuplication", this, "DoOk()     ");
	fGroup-> AddFrame(fButtonOk);

	fButtonCancel = new TGTextButton(fGroup, "Cancel");
	fButtonCancel->Resize(50, 30);
	fButtonCancel->Connect("Clicked()", "GVSetDuplication", this, "DoCancel()");
	fGroup->AddFrame(fButtonCancel);

	AddFrame(fGroup);
	Layout();
	MapSubwindows();
	CenterOnParent();
	MapWindow();
	GetClient()->WaitFor(this);

}

GVSetDuplication::~GVSetDuplication() {
	Cleanup();
	// all object added with AddFrame are already removed with Cleanup
}

void GVSetDuplication::DoOk() {

	fComputation = fComboComputation->GetSelected();
	fTab = fComboTab->GetNumber();
	fPad = fComboPad->GetNumber();

	(*fpComputation) = fComputation;
	(*fpPad) = fPad;
	(*fpTab) = fTab;
	DoQuit();

}

void GVSetDuplication::DoCancel() {

	(*fpPad) = -1;
	(*fpTab) = -1;
	(*fpComputation) = -1;
	DoQuit();
}
void GVSetDuplication::DoQuit() {

	TTimer::SingleShot(150, "GVSetDuplication", this, "CloseWindow()");
	// Close the Ged editor if it was activated.
	if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
		TVirtualPadEditor::Terminate();
}
void GVSetDuplication::CloseWindow() {

	UnmapWindow();
	DeleteWindow();
}
