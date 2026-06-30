// File : GVSetStat.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////
//
// Class GVSetStat
//   Box  of GV application.
//   Set configuration of statistic information on each histogram
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


#include "GVSetStat.h"
#include "Vigru.h"
#include <TGLayout.h>
#include <TVirtualPadEditor.h>
#include <TGMsgBox.h>
#include "math.h"
#include <TSystem.h>
#include <TROOT.h>
#include <TGClient.h>
#include <TStyle.h>


ClassImp(GVSetStat)

GVSetStat::GVSetStat(const TGWindow *p, const TGWindow *m, UInt_t w, UInt_t h,
		TGFrame *tg, char *message, TString* StatOpt,bool* page) :
	TGTransientFrame(p, m, w, h) {

	pPage = page;

	fMain = (Vigru*)m;
	fAboveFrame = (TGCompositeFrame*)tg;
	fStatOpt = *StatOpt;
	pStatOpt = StatOpt;
	SetWindowName(message);
	fLayout = new TGVerticalLayout(this);
	SetLayoutManager(fLayout);
	fGroupStatistics = new TGGroupFrame(this,"Display Statistic on Histograms");
	gClient->GetColorByName("white", white);
	gClient->GetColorByName("black", black);
	fGroupStatistics->SetLayoutManager(new TGMatrixLayout(fGroupStatistics,5,2,5));

	fChecks[0] = new TGCheckButton(fGroupStatistics,"name");
	fChecks[1] = new TGCheckButton(fGroupStatistics,"entries");
	fChecks[2] = new TGCheckButton(fGroupStatistics,"mean");
	fChecks[3] = new TGCheckButton(fGroupStatistics,"RMS");
	fChecks[4] = new TGCheckButton(fGroupStatistics,"overflow");
	fChecks[5] = new TGCheckButton(fGroupStatistics,"underflow");
	fChecks[6] = new TGCheckButton(fGroupStatistics,"integral");

	for (int i=0; i<7; ++i) {
		fGroupStatistics->AddFrame(fChecks[i]);
		fChecks[i]->SetBackgroundColor(0x00EFFF); //RGB =>vert/bleu
	}

	fLabBidon = new TGLabel(fGroupStatistics, new TGHotString(""));
	fGroupStatistics->AddFrame(fLabBidon);

	fButtonApply1 = new TGTextButton(fGroupStatistics,"Selected pad");
	//fButtonApply1->SetBackgroundColor(white);
	fButtonApply1->SetTextColor(black, kTRUE);
	fButtonApply1->Resize(80, 20);
	fButtonApply2 = new TGTextButton(fGroupStatistics,"Current page");
	// fButtonApply2->SetBackgroundColor(white);
	fButtonApply2->SetTextColor(black, kTRUE);
	fButtonApply2->Resize(80, 20);

	fGroupStatistics-> AddFrame(fButtonApply1, new TGLayoutHints(kLHintsExpandX));
	fGroupStatistics-> AddFrame(fButtonApply2, new TGLayoutHints(kLHintsExpandX));

	if (fStatOpt.Contains("n"))
		fChecks[0]->SetState(kButtonDown);
	if (fStatOpt.Contains("e"))
		fChecks[1]->SetState(kButtonDown);
	if (fStatOpt.Contains("m"))
		fChecks[2]->SetState(kButtonDown);
	if (fStatOpt.Contains("r"))
		fChecks[3]->SetState(kButtonDown);
	if (fStatOpt.Contains("o"))
		fChecks[4]->SetState(kButtonDown);
	if (fStatOpt.Contains("u"))
		fChecks[5]->SetState(kButtonDown);
	if (fStatOpt.Contains("i"))
		fChecks[6]->SetState(kButtonDown);

	AddFrame(fGroupStatistics);
	fButtonCancel = new TGTextButton(this,"Cancel");
	fButtonCancel->Resize(100, 20);
	AddFrame(fButtonCancel);

	fButtonApply1->Connect("Clicked()",   "GVSetStat", this, "DoOkOne()");
	fButtonApply2->Connect("Clicked()",   "GVSetStat", this, "DoOkSeveral()");
	fButtonCancel->Connect("Clicked()",   "GVSetStat", this, "DoCancelo()");

	Layout();
	MapSubwindows();
	CenterOnParent();
	MapWindow();



	GetClient()->WaitFor(this);
	 *StatOpt = fStatOpt;


}

GVSetStat::~GVSetStat() {
	Cleanup();

}

void GVSetStat::DoOkOne() {

	*pPage = false;
	fStatOpt="";
	if (fChecks[0]->IsOn())
		fStatOpt=fStatOpt+"n";
	if (fChecks[1]->IsOn())
		fStatOpt=fStatOpt+"e";
	if (fChecks[2]->IsOn())
		fStatOpt=fStatOpt+"m";
	if (fChecks[3]->IsOn())
		fStatOpt=fStatOpt+"r";
	if (fChecks[4]->IsOn())
		fStatOpt=fStatOpt+"o";
	if (fChecks[5]->IsOn())
		fStatOpt=fStatOpt+"u";
	if (fChecks[6]->IsOn())
		fStatOpt=fStatOpt+"i";
	DoCancelo();
	return;
}

void GVSetStat::DoOkSeveral() {

	*pPage =true;
	fStatOpt="";
	if (fChecks[0]->IsOn())
		fStatOpt=fStatOpt+"n";
	if (fChecks[1]->IsOn())
		fStatOpt=fStatOpt+"e";
	if (fChecks[2]->IsOn())
		fStatOpt=fStatOpt+"m";
	if (fChecks[3]->IsOn())
		fStatOpt=fStatOpt+"r";
	if (fChecks[4]->IsOn())
		fStatOpt=fStatOpt+"o";
	if (fChecks[5]->IsOn())
		fStatOpt=fStatOpt+"u";
	if (fChecks[6]->IsOn())
		fStatOpt=fStatOpt+"i";
	DoCancelo();
	return;
}

void GVSetStat::DoCancelo() {

	 *pStatOpt = fStatOpt;
	TTimer::SingleShot(10, "GVSetStat", this, "CloseWindow()");

	// Close the Ged editor if it was activated.
	if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
		TVirtualPadEditor::Terminate();
}

void GVSetStat::CloseWindow() {

		UnmapWindow();
		DeleteWindow();

	}//delete this;

