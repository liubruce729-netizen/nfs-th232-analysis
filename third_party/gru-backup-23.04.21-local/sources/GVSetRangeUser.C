// File : GVSetRangeUser.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////
//
// Class GVSetRangeUser
//   Box  of GV application.
//   Set Parameter to setup Range User
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


#include "GVSetRangeUser.h"
#include "Vigru.h"
#include <TGLayout.h>
#include <TVirtualPadEditor.h>
#include <TGMsgBox.h>
#include "math.h"
#include <TSystem.h>
#include <TROOT.h>
#include <TGClient.h>
#include <TStyle.h>
ClassImp(GVSetRangeUser)

GVSetRangeUser::GVSetRangeUser(const TGWindow *p,const TGWindow *m,UInt_t w,
		UInt_t h, TGFrame *tg,bool *xbool,bool *ybool,bool *zbool,
		Double_t *xmin,Double_t *xmax,Double_t *ymin,Double_t*ymax,Double_t *zmin,Double_t *zmax,int *padtaborall,bool setorunset):
TGTransientFrame(p, m, w, h) {

	fXmin        = xmin;
	fXmax        = xmax;
	fYmin        = ymin;
	fYmax        = ymax;
	fZmin        = zmin;
	fZmax        = zmax;
	fxbool       = xbool;
	fybool       = ybool;
	fzbool       = zbool;
	fPadtaborall = padtaborall;
	fSetorunset  = setorunset;
	
	fMain = (Vigru*)m;
	fAboveFrame = (TGMenuBar*)tg;
	if (setorunset)
		SetWindowName("Set Range User (Zoom) ");
	else 
		SetWindowName("Unset Range User (Unzoom) ");
		
	SetLayoutManager(new TGVerticalLayout(this));
	
        if (setorunset){
		fGroup = new TGGroupFrame(this,"");
		fGroup->SetLayoutManager(new TGMatrixLayout(fGroup,10,2,10));
		AddFrame(fGroup);

		fChecksX = new TGCheckButton(fGroup,"Range in X");
		fGroup->AddFrame(fChecksX);
		fLabel1 = new TGLabel(fGroup,"");
		fGroup->AddFrame(fLabel1);

		fLabel2 = new TGLabel(fGroup,"X Min ");
		fGroup->AddFrame(fLabel2);
		fNumberEntryXMin = new TGNumberEntryField(fGroup);
		fGroup->AddFrame(fNumberEntryXMin);

		fLabel3 = new TGLabel(fGroup,"X Max ");
		fGroup->AddFrame(fLabel3);
		fNumberEntryXMax = new TGNumberEntryField(fGroup);
		fGroup->AddFrame(fNumberEntryXMax);

		fChecksY = new TGCheckButton(fGroup,"Range in Y");
		fGroup->AddFrame(fChecksY);
		fLabel4 = new TGLabel(fGroup,"");
		fGroup->AddFrame(fLabel4);

		fLabel5 = new TGLabel(fGroup,"Y Min ");
		fGroup->AddFrame(fLabel5);
		fNumberEntryYMin = new TGNumberEntryField(fGroup);
		fGroup->AddFrame(fNumberEntryYMin);

		fLabel6 = new TGLabel(fGroup,"Y Max ");
		fGroup->AddFrame(fLabel6);
		fNumberEntryYMax = new TGNumberEntryField(fGroup);
		fGroup->AddFrame(fNumberEntryYMax);

		fChecksZ = new TGCheckButton(fGroup,"Range in Z");
		fGroup->AddFrame(fChecksZ);
		fLabel7 = new TGLabel(fGroup,"");
		fGroup->AddFrame(fLabel7);

		fLabel8 = new TGLabel(fGroup,"Z Min ");
		fGroup->AddFrame(fLabel8);
		fNumberEntryZMin = new TGNumberEntryField(fGroup);
		fGroup->AddFrame(fNumberEntryZMin);

		fLabel9 = new TGLabel(fGroup,"Z Max ");
		fGroup->AddFrame(fLabel9);
		fNumberEntryZMax = new TGNumberEntryField(fGroup);
		fGroup->AddFrame(fNumberEntryZMax);
	
		fNumberEntryXMin->SetNumber((*fXmin));
		fNumberEntryXMax->SetNumber((*fXmax));
		fNumberEntryYMin->SetNumber((*fYmin));
		fNumberEntryYMax->SetNumber((*fYmax));
		fNumberEntryZMin->SetNumber((*fZmin));
		fNumberEntryZMax->SetNumber((*fZmax));
		fChecksX->SetState(kButtonDown);
		fChecksY->SetState(kButtonUp);
		fChecksZ->SetState(kButtonUp);
		}

	fGroupApplication = new TGCompositeFrame(this);
	AddFrame(fGroupApplication);
	
	fRangeRadioGroup = new TGButtonGroup(fGroupApplication,"Effect of Set or Unset Range",kVerticalFrame);
	fRadioPad        = new TGRadioButton(fRangeRadioGroup,new TGHotString("Current Pad"));
	fRadioPage       = new TGRadioButton(fRangeRadioGroup,new TGHotString("Current Page"));
	fRadioAllPages   = new TGRadioButton(fRangeRadioGroup,new TGHotString("All Pages"));
	fRadioPage->SetState(kButtonDown);
	fGroupApplication->AddFrame(fRangeRadioGroup);
	
	
	fGroupEndButton  = new TGCompositeFrame(this);
	AddFrame(fGroupEndButton);
	SetLayoutManager(new TGVerticalLayout(this));

	fGroupEndButton->SetLayoutManager(new TGMatrixLayout(fGroupEndButton,1,3,20,20));
	
	fLabel10 = new TGLabel(fGroupEndButton,"");fGroupEndButton->AddFrame(fLabel10);
		
	fButtonOk = new TGTextButton(fGroupEndButton,"Ok" );
	fButtonOk->Resize(50, 30);
	fButtonOk ->Connect("Clicked()", "GVSetRangeUser", this, "DoOk() ");
	fGroupEndButton->AddFrame(fButtonOk,new TGLayoutHints(kLHintsExpandX));

	fButtonCancel = new TGTextButton(fGroupEndButton,"Cancel");
	fButtonCancel->Resize(50, 30);
	fButtonCancel->Connect("Clicked()", "GVSetRangeUser", this, "DoCancel()");
	fGroupEndButton->AddFrame(fButtonCancel,new TGLayoutHints(kLHintsExpandX));

	Layout();
	MapSubwindows();
	CenterOnParent();
	MapWindow();
	GetClient()->WaitFor(this);
}

GVSetRangeUser::~GVSetRangeUser() {
	Cleanup();
	// all object added with AddFrame are already removed with Cleanup
}

void GVSetRangeUser::DoOk() {
	(*fxbool) =false;
	(*fybool) =false;
	(*fzbool) =false;

	if (fSetorunset){
		if (fChecksX->IsOn()) (*fxbool) =true;
		if (fChecksY->IsOn()) (*fybool) =true;
		if (fChecksZ->IsOn()) (*fzbool) =true;
		(*fXmin) = fNumberEntryXMin->GetNumber();
		(*fXmax) = fNumberEntryXMax->GetNumber();
		(*fYmin) = fNumberEntryYMin->GetNumber();
		(*fYmax) = fNumberEntryYMax->GetNumber();
		(*fZmin) = fNumberEntryZMin->GetNumber();
		(*fZmax) = fNumberEntryZMax->GetNumber();
	}
	if (fRadioPad->IsOn())      *fPadtaborall = 1;
	if (fRadioPage->IsOn())     *fPadtaborall = 2;
	if (fRadioAllPages->IsOn()) *fPadtaborall = 3;
	

	DoQuit();

}

void GVSetRangeUser::DoCancel() {

	DoQuit();

}
void GVSetRangeUser::DoQuit() {

	TTimer::SingleShot(150, "GVSetRangeUser", this, "CloseWindow()");

	// Close the Ged editor if it was activated.
	if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
		TVirtualPadEditor::Terminate();
}
void GVSetRangeUser::CloseWindow() {

	UnmapWindow();

	//	delete (this)-> but a
	DeleteWindow();

}
