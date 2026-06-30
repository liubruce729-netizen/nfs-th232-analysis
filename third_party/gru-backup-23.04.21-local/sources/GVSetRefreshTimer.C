// File : GVSetRefreshTimer.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////
//
// Class GVSetRefreshTimer
//   Box  of GV application.
//   Ask name of page and nb of wanted pad
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


#include "GVSetRefreshTimer.h"
#include "Vigru.h"
#include <TGLayout.h>
#include <TVirtualPadEditor.h>
#include <TGMsgBox.h>
#include "math.h"

ClassImp( GVSetRefreshTimer)

GVSetRefreshTimer::GVSetRefreshTimer(const TGWindow *p, const TGWindow *m,
		UInt_t w, UInt_t h, TGFrame *tg, Int_t* nb) :
	TGTransientFrame(p, m, w, h) {
	fNb = nb;
	TString message;
	message = "Set Auto Refresh Timer ";

	fMain = (Vigru*) m;
	fAboveFrame = (TGCompositeFrame*) tg;
	SetWindowName(message.Data());
	SetLayoutManager(new TGVerticalLayout(this));

	fGroup = new TGGroupFrame(this, message.Data());

	fGroup->SetLayoutManager(new TGMatrixLayout(fGroup, 2, 2, 10));
	AddFrame(fGroup);

	//fGroup->AddFrame(new TGLabel(fGroup," "));
	fLabName = new TGLabel(fGroup, "Time : ");
	fGroup->AddFrame(fLabName);

	fComboNb = new TGComboBox(fGroup);
	const char *times[] = //refresh times: to put in combo box
			{ "(none)","1  s", "5  s", "10 s", "40 s", "1 mn", "2 mn", "5 mn", 0 };

	for (int i = 0; times[i]; i++) {
		fComboNb->AddEntry(times[i], (i));

	}

	fComboNb->Resize(70, 20);

	fGroup->AddFrame(fComboNb);
	fComboNb->Select(0);

	fButtonOk = new TGTextButton(fGroup, "Ok");
	fButtonOk->Resize(50, 30);
	fButtonOk ->Connect("Clicked()", "GVSetRefreshTimer", this, "DoOk()     ");
	fGroup-> AddFrame(fButtonOk);

	fButtonCancel = new TGTextButton(fGroup, "Cancel");
	fButtonCancel->Resize(50, 30);
	fButtonCancel->Connect("Clicked()", "GVSetRefreshTimer", this, "DoCancel()");
	fGroup->AddFrame(fButtonCancel);

	Layout();
	MapSubwindows();
	CenterOnParent();
	MapWindow();
	GetClient()->WaitFor(this);
}

GVSetRefreshTimer::~GVSetRefreshTimer() {
	Cleanup();
	// all object added with AddFrame are already removed with Cleanup
}

void GVSetRefreshTimer::DoOk() {

	Int_t time = fComboNb->GetSelected();
	Long_t interval = 0;
	switch (time) {
	case 1:
		interval = 1000;
		break;
	case 2:
		interval = 5000;
		break;
	case 3:
		interval = 10000;
		break;
	case 4:
		interval = 40000;
		break;
	case 5:
		interval = 60000;
		break;
	case 6:
		interval = 120000;
		break;
	case 7:
		interval = 30000;
		break;
	}

	(*fNb) = interval;
	DoQuit();

}

void GVSetRefreshTimer::DoCancel() {
	(*fNb) = -1;
	DoQuit();
}
void GVSetRefreshTimer::DoQuit() {

	TTimer::SingleShot(150, "GVSetRefreshTimer", this, "CloseWindow()");

	// Close the Ged editor if it was activated.
	if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
		TVirtualPadEditor::Terminate();
}
void GVSetRefreshTimer::CloseWindow() {

	UnmapWindow();
	DeleteWindow();

}
