// File : GVSetReset.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////
//
// Class GVSetReset
//   Box  of GV application.
//   Set configuration of Reset config
//////////////////////////////////////////////////////////////////////


// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/


#include "GVSetReset.h"
#include "Vigru.h"
#include <TGLayout.h>
#include <TVirtualPadEditor.h>
#include <TGMsgBox.h>
#include "math.h"
#include <TSystem.h>
#include <TROOT.h>
#include <TGClient.h>
#include <TStyle.h>

ClassImp( GVSetReset)

GVSetReset::GVSetReset(const TGWindow *p, const TGWindow *m, UInt_t w,
		UInt_t h,  char *message, TString* resetopt) :
TGTransientFrame(p, m, w, h) {


	fMain = (Vigru*)m;
	fResetOpt = *resetopt;
	pResetOpt = resetopt;
	SetWindowName(message);
	gClient->GetColorByName("white", white);
	gClient->GetColorByName("black", black);

	SetLayoutManager(new TGVerticalLayout(this));

	fGroupResets = new TGGroupFrame(this,"Which Spectra Reset?");

	//fGroupResets->SetLayoutManager(new TGMatrixLayout(fGroupResets,5,2,5));

	fResetRadioGroup = new TGButtonGroup(fGroupResets,"Effect of Reset",kVerticalFrame);
	fRadioPad = new TGRadioButton(fResetRadioGroup,new TGHotString("Pad"));
	fRadioPage = new TGRadioButton(fResetRadioGroup,new TGHotString("Page"));
	fRadioServers = new TGRadioButton(fResetRadioGroup,new TGHotString("Servers"));
	fGroupResets->AddFrame(fResetRadioGroup);

	fCheck = new TGCheckButton(fGroupResets,"Do not ask me this again");
	fGroupResets->AddFrame(fCheck);

	fRadioPad->SetBackgroundColor(0x00EFFF); //RGB =>vert/bleu


	fLabBidon = new TGLabel(fGroupResets, new TGHotString(""));
	fGroupResets->AddFrame(fLabBidon);

	if (fResetOpt.Contains("h"))
	fRadioPad->SetState(kButtonDown);
	if (fResetOpt.Contains("p"))
	fRadioPage->SetState(kButtonDown);
	if (fResetOpt.Contains("s"))
	fRadioServers->SetState(kButtonDown);

	if (fResetOpt.Contains("y"))
	fCheck->SetState(kButtonDown);

	if (fResetOpt.Contains("n"))
	fCheck->SetState(kButtonUp);

	AddFrame(fGroupResets);

	fButtons = new TGCompositeFrame(this);
	fButtons->SetLayoutManager(new TGMatrixLayout(fButtons,0,2,10));
	AddFrame(fButtons);

	fButtonApply1 = new TGTextButton(fButtons,"Apply");
	fButtonApply1->SetTextColor(black, kTRUE);
	fButtonApply1->Resize(50, 20);

	fButtonCancel = new TGTextButton(fButtons,"Cancel");
	fButtonCancel->Resize(50, 20);
	fButtonCancel->SetTextColor(black, kTRUE);

	fButtons->AddFrame(fButtonCancel,new TGLayoutHints(kLHintsExpandX));
	fButtons->AddFrame(fButtonApply1, new TGLayoutHints(kLHintsExpandX));

	fButtonApply1->Connect("Clicked()", "GVSetReset", this, "DoOkOne()");
	fButtonCancel->Connect("Clicked()", "GVSetReset", this, "DoCancelo()");

	Layout();
	MapSubwindows();
	CenterOnParent();
	MapWindow();

	GetClient()->WaitFor(this);
	*resetopt = fResetOpt;

}

GVSetReset::~GVSetReset() {
	Cleanup();

}

void GVSetReset::DoOkOne() {

	fResetOpt = "";
	if (fRadioPad->IsOn())
		fResetOpt = fResetOpt + "h";
	if (fRadioPage->IsOn())
		fResetOpt = fResetOpt + "p";
	if (fRadioServers->IsOn())
		fResetOpt = fResetOpt + "s";
	if (fCheck->IsOn())// == IsDown();
		fResetOpt = fResetOpt + "y";
	if (!(fCheck->IsOn()))
		fResetOpt = fResetOpt + "n";

	*pResetOpt = fResetOpt;

	TTimer::SingleShot(10, "GVSetReset", this, "CloseWindow()");
		// Close the Ged editor if it was activated.
		if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
			TVirtualPadEditor::Terminate();
}

void GVSetReset::DoCancelo() {
	fResetOpt = fResetOpt + "c";
	*pResetOpt = fResetOpt;
	TTimer::SingleShot(10, "GVSetReset", this, "CloseWindow()");

	// Close the Ged editor if it was activated.
	if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
		TVirtualPadEditor::Terminate();
}

void GVSetReset::CloseWindow() {

	UnmapWindow();
	DeleteWindow();

}//delete this;

