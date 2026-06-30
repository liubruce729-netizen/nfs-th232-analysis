// File : GVSetPage.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////
//
// Class GVSetPage
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


#include "GVSetPage.h"
#include "Vigru.h"
#include <TGLayout.h>
#include <TVirtualPadEditor.h>
#include <TGMsgBox.h>
#include "math.h"
#include <TSystem.h>
#include <TROOT.h>
#include <TGClient.h>
#include <TStyle.h>
ClassImp(GVSetPage)

GVSetPage::GVSetPage(const TGWindow *p, const TGWindow *m, UInt_t w, UInt_t h, TString message, Int_t nbTab,TString* PageName,
		Int_t* nbPadsX,Int_t* nbPadsY) :
	TGTransientFrame(p, m, w, h) {

	fNbTab = nbTab;
	fNewTab = true;
	fNbPadsXOld = *nbPadsX;
	fNbPadsYOld = *nbPadsY;
	fPageNameOld = *PageName;
	fNbPadsX = nbPadsX;
	fNbPadsY = nbPadsY;
	fPageName = PageName;
	if (strcmp(message.Data(), "Set Page")==0) {
		fNewTab =false;
	}

	fMain = (Vigru*)m;
	SetWindowName(message.Data());
	SetLayoutManager(new TGVerticalLayout(this));

	if (fNewTab)
		fGroup = new TGGroupFrame(this,"Adding Histograms Page");
	else
		fGroup = new TGGroupFrame(this,"Setting Histograms Page");

	fGroup->SetLayoutManager(new TGMatrixLayout(fGroup,3,3,10));
	AddFrame(fGroup);

	fLabNamePage = new TGLabel(fGroup,"Name of current page: ");
	fGroup->AddFrame(fLabNamePage);

	fEntryNamePage = new TGTextEntry(fGroup);
	fGroup->AddFrame(fEntryNamePage);

	fGroup->AddFrame(new TGLabel(fGroup," "));
	fLabNbPads = new TGLabel(fGroup,"Nb of pads in X and Y ");
	fGroup->AddFrame(fLabNbPads);

	fComboNbPadX = new TGComboBox(fGroup);
	fComboNbPadY = new TGComboBox(fGroup);
	Int_t padmax =10;
	const char *nbPad[] = {"nc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", 0 };
	for (int i =0; i <=padmax; i++) {
		fComboNbPadX->AddEntry(nbPad[i], (i));
		fComboNbPadY->AddEntry(nbPad[i], (i));
	}

	fComboNbPadX->Resize(60, 20);	fComboNbPadY->Resize(60, 20);
	if (fNbPadsXOld>padmax)
		fNbPadsXOld =padmax;
	if (fNbPadsYOld>padmax)
		fNbPadsYOld =padmax;
	if (fNbPadsXOld<0)
		fNbPadsXOld =0;
	if (fNbPadsYOld<0)
		fNbPadsYOld =0;
	fComboNbPadX -> Select(fNbPadsXOld);
	fComboNbPadY -> Select(fNbPadsYOld);
	fGroup->AddFrame(fComboNbPadX);
	fGroup->AddFrame(fComboNbPadY);

	fButtonOk = new TGTextButton(fGroup,"Ok" );
	fButtonOk->Resize(50, 30);
	fButtonOk ->Connect("Clicked()", "GVSetPage", this, "DoOk()     ");
	fGroup-> AddFrame(fButtonOk);

	fButtonCancel = new TGTextButton(fGroup,"Cancel");
	fButtonCancel->Resize(50, 30);
	fButtonCancel->Connect("Clicked()", "GVSetPage", this, "DoCancel()");
	fGroup->AddFrame(fButtonCancel);

	if (fPageNameOld.CompareTo("")!=0)
		fEntryNamePage->SetText(fPageNameOld.Data());

	Layout();
	MapSubwindows();
	CenterOnParent();
	MapWindow();
	GetClient()->WaitFor(this);
}

GVSetPage::~GVSetPage() {
	Cleanup();
	// all object added with AddFrame are already removed with Cleanup
}

void GVSetPage::DoOk() {

	TString tempo;

	tempo= fEntryNamePage->GetDisplayText();
	Int_t nbPadsX =fComboNbPadX->GetSelected();
	Int_t nbPadsY =fComboNbPadY->GetSelected();

	Int_t retval;
	if (!fNewTab) {
		if ((nbPadsX != fNbPadsXOld)&&(nbPadsY != fNbPadsYOld)) {

			EMsgBoxIcon icontype = kMBIconQuestion;
			new TGMsgBox(gClient->GetRoot(), this,
					"Warning","Do you want really change the number of pad in this tab?\n Current histograms in this tab will be deleted !" ,
					icontype, 3, &retval);
			if (retval!=1) {
				DoCancel();
			}

		} else {
			nbPadsX = 0;
			nbPadsY = 0;
		}
	}
	(*fPageName)= tempo;
	(*fNbPadsX) = nbPadsX;
	(*fNbPadsY) = nbPadsY;
	DoQuit();

}

void GVSetPage::DoCancel() {

	(*fNbPadsX) = -1;
	(*fNbPadsY) = -1;
	(*fPageName) = "";
	DoQuit();

}
void GVSetPage::DoQuit() {

	TTimer::SingleShot(150, "GVSetPage", this, "CloseWindow()");

	// Close the Ged editor if it was activated.
	if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
		TVirtualPadEditor::Terminate();
}
void GVSetPage::CloseWindow() {

	UnmapWindow();

	//	delete (this)-> but a
	DeleteWindow();

}
