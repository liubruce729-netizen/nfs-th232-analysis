// File : GVSpectraChooser.C
// Author: Jerome Chauveau & Luc Legeard

//////////////////////////////////////////////////////////////////////////////
//class GVSpectraChooser
//  Display a GVListTree to choose spectra. When
//           spectra are chosen, an action is done on all of them
//
////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <TTimer.h>
#include <TVirtualPadEditor.h>

#include "GVSpectraChooser.h"
#include "Vigru.h"

ClassImp( GVSpectraChooser)

GVSpectraChooser::GVSpectraChooser(const TGWindow* p, const TGWindow* main,
		UInt_t w, UInt_t h, TString windowName, Int_t typeTree,
		Int_t* oneselected, Int_t *operation, Int_t typeofspectra) :
	TGTransientFrame(p, main, w, h) {
	//w and h are the size of window
	//windowName is Window Name
	//typeTree  = 1 Family
	//typeTree  = 2 Page
	//typeTree  = 3 Name ( no folder)
	//if onselected* = 1 => Only one spectrum must be choosen and onselected will contain index of spectra
	//if onselected* = 1=> Several spectra are expected so Action flad will be set to true and onselected return number of selected spectra
	// operation = NULL  => no operation menu will be displayed
	// if operation  !=NULL  and *operation = 0 => replace   =1 display in same pad, get new spectra  without erase the previous spectrum
	//   = 2  display in same
	//   = 3 =>addition spectrum together, = 4 => substraction
	//
	//typeofspectra =1 for standart spectra , 3 for Raw and 2 for cut

	fTypeOfSpectra = typeofspectra;
	fListId = NULL;
	fMain = (TGMainFrame*) main;
	fListTree = NULL;
	fDB = NULL;
	fEst_Croissant = true;
	fEst_Decroissant = false;
	TString windoName;
	fOneselected = oneselected;
	fOneOrSeveral = *oneselected;
	*oneselected = -1;
	fTypeTree = typeTree;
	Pixel_t white;
	gClient->GetColorByName("white", white);
	fNumberOfItems = 0;
	Pixel_t blue;
	fpOperation = operation;
	if (fpOperation)
		fOperation = *operation;
	gClient->GetColorByName("light blue", blue);
	fGroupOperation = NULL;
	fRadioReplace = NULL;
	fRadioSame = NULL;
	fRadioAddition = NULL;

	TGHorizontalFrame *fblocPrincipal = new TGHorizontalFrame(this);
	TGVerticalFrame *fbloc = new TGVerticalFrame(fblocPrincipal);

	/*
	 fOrdre = new TGButtonGroup(fbloc , "Order",kVerticalFrame);
	 fCroissant = new TGRadioButton(fOrdre , "Increasing");
	 fCroissant->SetState(kButtonDown);
	 fCroissant->Connect("Clicked()", "GVSpectraChooser", this, "Croissant()");
	 fDecroissant = new TGRadioButton(fOrdre , "Decreasing");
	 fDecroissant->Connect("Clicked()", "GVSpectraChooser", this,
	 "Decroissant()");
	 fOrdre->Show();

	 fChoixType = new TGButtonGroup(fbloc , "Order by",kVerticalFrame);

	 fName = new TGRadioButton(fChoixType, "Name");
	 fName->Connect("Clicked()", "GVSpectraChooser", this, "ByName()");
	 fFamily = new TGRadioButton( fChoixType, "Familly");
	 fFamily->Connect("Clicked()", "GVSpectraChooser", this, "ByFamily()");
	 fPage = new TGRadioButton( fChoixType, "Page");
	 fPage->Connect("Clicked()", "GVSpectraChooser", this, "ByPage()");
	 fPage->SetEnabled(kFALSE);

	 fbloc->AddFrame(fOrdre, new TGLayoutHints(kLHintsExpandX,5,5,5,1));
	 fbloc->AddFrame(fChoixType, new TGLayoutHints(kLHintsExpandX,5,5,5,1));

	 fName ->SetBackgroundColor(white);
	 fFamily ->SetBackgroundColor(white);
	 fPage ->SetBackgroundColor(white);

	 fOrdre->SetBackgroundColor(white);
	 fCroissant->SetBackgroundColor(white);
	 fDecroissant->SetBackgroundColor(white);

	 fChoixType ->SetBackgroundColor(white);

	 */
	/*
	 if (typeTree==1) {
	 fFamily->SetState(kButtonDown);
	 }
	 if (typeTree==2) {
	 fPage->SetEnabled(kTRUE);
	 fPage->SetState(kButtonDown);
	 }
	 if (typeTree==3) {
	 fName->SetState(kButtonDown);
	 }
	 fChoixType->Show();
	 */

	fButtonOk = new TGTextButton(fbloc, "OK");
	fButtonCancel = new TGTextButton(fbloc, "Cancel");

	if (fpOperation) {
		fGroupOperation = new TGButtonGroup(fbloc, "Operation", kVerticalFrame);
		fRadioReplace = new TGRadioButton(fGroupOperation, "Replace");
		fRadioSame = new TGRadioButton(fGroupOperation, "Same");
		//fRadioAddition = new TGRadioButton(fGroupOperation, "Addition");
		//fRadioSubtraction = new TGRadioButton(fGroupOperation, "Subtraction");
		if (fOperation == 0)
			fRadioReplace->SetState(kButtonDown);
		if (fOperation == 1)
			fRadioSame->SetState(kButtonDown);
		if (fOperation == 2)
				fRadioSame->SetState(kButtonDown);
		/*if (fOperation == 3)
			fRadioAddition->SetState(kButtonDown);
		if (fOperation == 4)
			fRadioSubtraction->SetState(kButtonDown);*/
	}

	fButtonOk    ->Connect("Clicked()", "GVSpectraChooser", this, "DoOK()");
	fButtonCancel->Connect("Clicked()", "GVSpectraChooser", this, "DoFinish()");

	fbloc->AddFrame(fButtonOk, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 1));
	fbloc->AddFrame(fButtonCancel,
			new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 1));
	if (fpOperation)
		fbloc->AddFrame(fGroupOperation, new TGLayoutHints(kLHintsExpandX, 5,
				5, 5, 1));

	fTreeView = new TGCanvas(this, 250, 480, kSunkenFrame | kDoubleBorder);

	fblocPrincipal->AddFrame(fbloc, new TGLayoutHints(kLHintsNoHints, 5, 5, 5,
			1));
	fblocPrincipal->AddFrame(fTreeView, new TGLayoutHints(kLHintsExpandX
			| kLHintsExpandY, 5, 5, 5, 1));

	AddFrame(fblocPrincipal, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,
			5, 5, 5, 1));

	fblocPrincipal->SetBackgroundColor(blue);
	fbloc->SetBackgroundColor(blue);

	SetBackgroundColor(blue);
	CenterOnParent();
	SetWindowName(windoName.Data());

}

GVSpectraChooser::~GVSpectraChooser() {
	Cleanup(); // delete all object included with AddFrame
	if (fListTree) {
		delete (fListTree);
		fListTree = NULL;
	}
}
/*------------------------------------------------------------------------------------------------------------------*/
void GVSpectraChooser::Init(GSpectraDB* DB) {
	fDB = DB;
	if (fDB == NULL) {
		fError.TreatError(2, 0,
				"Initialisation of GVSpectraChooser aborted, DataBase is null");
		return;
	}
	if (fDB->GetLast() < 0)
		return;
	if (fTypeTree == 1) {
		//fFamily->SetState(kButtonDown);
		fListTree = new GVListTree(fTreeView->GetViewPort(), 10, 10,
				kHorizontalFrame, NULL, fTypeTree, fTypeOfSpectra);
		fTreeView->SetContainer(fListTree);
		ByFamily();
	}
	if (fTypeTree == 2) {
		//fPage->SetEnabled(kTRUE);
		//fPage->SetState(kButtonDown);
		fListTree = new GVListTree(fTreeView->GetViewPort(), 10, 10,
				kHorizontalFrame, NULL, fTypeTree, fTypeOfSpectra);
		fTreeView->SetContainer(fListTree);
		ByPage();
	}
	if (fTypeTree == 3) {
		//fName->SetState(kButtonDown);
		fListTree = new GVListTree(fTreeView->GetViewPort(), 10, 10,
				kHorizontalFrame, NULL, fTypeTree, fTypeOfSpectra);
		fTreeView->SetContainer(fListTree);
		ByName();
	}

	GetClient()->WaitFor(this);

}
/*------------------------------------------------------------------------------------------------------------------*/
void GVSpectraChooser::DoOK() {

	if (!fListTree) {
		DoFinish();
		return;
	}

	CallSelectSpectra();
	if (fpOperation) {
		if (fRadioReplace->IsOn())
			fOperation = 0;
		if (fRadioSame->IsOn())
				fOperation = 1;
		/*
		if ((fRadioSame->IsOn())&&(fTypeOfSpectra !=2))
			fOperation = 1;
		if ((fRadioSame->IsOn())&&(fTypeOfSpectra ==2))
			fOperation = 2;
		if (fOperation==1)
			fOperation = 2;
		if (fRadioAddition->IsOn())
			fOperation = 3;
		if (fRadioSubtraction->IsOn())
			fOperation = 4;*/
		*fpOperation = fOperation;

	}

	DoFinish();
	return;
}

/*------------------------------------------------------------------------------------------------------------------*/
void GVSpectraChooser::CallSelectSpectra() {

	if (!fListTree)
		return;
	if (fDB == NULL)
		return;
	if (fDB->GetLast() < 0)
		return;
	fDB->ResetActions();

	if (fOneOrSeveral == 2) {
		fListId = fListTree->GetSelectedItemsID();
		*fOneselected = fListId->GetLast();
	}
	if (fOneOrSeveral == 1) {
		*fOneselected = fListTree->GetSelectedItemInD();
	}
}

/*------------------------------------------------------------------------------------------------------------------*/
void GVSpectraChooser::DoFinish() {
	if (fListTree) {
		delete (fListTree);
		fListTree = NULL;
	}
	// Action done when Cancel button is selected
	TTimer::SingleShot(150, "GVSpectraChooser", this, "CloseWindow()");
	// Close the Ged editor if it was activated.
	if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
		TVirtualPadEditor::Terminate();
}
/*------------------------------------------------------------------------------------------------------------------*/
void GVSpectraChooser::CloseWindow() {
	UnmapWindow();
	//delete (this);
	DeleteWindow();
}
/*------------------------------------------------------------------------------------------------------------------*/
//Write by Mialndou

void GVSpectraChooser::Croissant() {
	if (fDB->GetLast() < 0)
		return;
	if (fEst_Decroissant) {
		fEst_Croissant = true;
		fEst_Decroissant = false;
		fDB->Reverse();
		if (fName->IsOn()) {
			ByName();
		}
		if (fFamily->IsOn()) {
			ByFamily();
		}
		if (fPage->IsOn()) {
			ByPage();
		}
	}
}
/*------------------------------------------------------------------------------------------------------------------*/
void GVSpectraChooser::Decroissant() {
	if (fDB->GetLast() < 0)
		return;
	if (fEst_Croissant) {
		fEst_Croissant = false;
		fEst_Decroissant = true;

		fDB->Reverse();

		if (fName->IsOn()) {
			ByName();
		}
		if (fFamily->IsOn()) {
			ByFamily();
		}
		if (fPage->IsOn()) {
			ByPage();
			fDB->Reverse();
		}
	}
}
/*------------------------------------------------------------------------------------------------------------------*/
void GVSpectraChooser::ByName() {
	if (fDB->GetLast() < 0)
		return;

	if (fListTree) {
		if (fNumberOfItems > 0) {
			fListTree->RecursiveDeleteItem(fListTree->GetFirstItem(),
					fListTree->GetFirstItem()->GetUserData());
			cout << "delete items\n";
			fNumberOfItems = 0;
			delete (fListTree);
			fListTree = new GVListTree(fTreeView->GetViewPort(), 10, 10,
					kHorizontalFrame, NULL, fTypeTree);
			fTreeView->SetContainer(fListTree);
		}
		if (fOneOrSeveral == 1) {
			GetListTree()->SetCheckBoxes(kFALSE);
		} else {
			GetListTree()->SetCheckBoxes(kTRUE);
		}

		fDB->ByName(0, fDB->GetLast());
		if (fNumberOfItems == 0) {
			fNumberOfItems = fListTree->FillFromDB(fDB);
			fListTree->ClearViewPort();
			//gClient->NeedRedraw(fListTree);
		}
		// ceci est a chang� car l'affichage fait plant� , mais le r�sultat attendu marche

		Layout();
		MapSubwindows();
		MapWindow();
	}
}
/*------------------------------------------------------------------------------------------------------------------*/
void GVSpectraChooser::ByFamily() {

	if (fDB->GetLast() < 0)
		return;
	if (fListTree) {

		if (fNumberOfItems > 0) {
			fListTree->RecursiveDeleteItem(fListTree->GetFirstItem(),
					fListTree->GetFirstItem()->GetUserData());
			fNumberOfItems = 0;
			cout << "delete items\n";
		}
		if (fOneOrSeveral == 1) {
			GetListTree()->SetCheckBoxes(kFALSE);
		} else {
			GetListTree()->SetCheckBoxes(kTRUE);
		}
		if (fNumberOfItems == 0) {
			fNumberOfItems = fListTree->FillFromDB(fDB);
			//gClient->NeedRedraw(fListTree);
			fListTree->ClearViewPort();
		}
		// ceci est a chang� car l'affichage fait plant� , mais le r�sultat attendu marche


		Layout();
		MapSubwindows();
		MapWindow();
	}
}
/*------------------------------------------------------------------------------------------------------------------*/
void GVSpectraChooser::ByPage() {
	if (fDB->GetLast() < 0)
		return;

	if (fListTree) {
		if (fNumberOfItems > 0) {
			fListTree->RecursiveDeleteItem(fListTree->GetFirstItem(),
					fListTree->GetFirstItem()->GetUserData());
			cout << "delete items\n";
			fNumberOfItems = 0;
		}
		if (fOneOrSeveral == 1) {
			GetListTree()->SetCheckBoxes(kFALSE);
		} else {
			GetListTree()->SetCheckBoxes(kTRUE);
		}
		fDB->ByPage(0, fDB->GetLast());
		if (fNumberOfItems == 0) {
			fNumberOfItems = fListTree->FillFromDB(fDB);
			//gClient->NeedRedraw(fListTree);
			fListTree->ClearViewPort();
		}
		// ceci est a chang� car l'affichage fait plant� , mais le r�sultat attendu marche
		Layout();
		MapSubwindows();
		MapWindow();
	}
}
/*------------------------------------------------------------------------------------------------------------------*/
