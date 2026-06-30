// File : GVToolBar.C
// Author: J�r�me Chauveau
//////////////////////////////////////////////////////////////////////////////
//class GVToolBar
//ViGRU Toolbar
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

#include "GVToolBar.h"
#include "Vigru.h"
#include <TGToolBar.h>
#include<TGFileDialog.h>
#include <TGButton.h>
#include <TBox.h>
#include <TGMsgBox.h>

#include "images/image_refreshlist.h"
#include "images/image_info.h"
#include "images/image_infono.h"
#include "images/image_cut.h"
#include "images/image_cut_send.h"
#include "images/image_arrow_left.h"
#include "images/image_arrow_right.h"

ClassImp( GVToolBar)

static const char *images[] = //images
		{ "ed_print.png", //print.png
				"bld_open.png",// image
				"bld_save.png", // image
				"bld_save.xpm", // image bidon qui sera remplacée,
				"bld_save.xpm", // image bidon qui sera remplacée,
				"bld_save.xpm", // image bidon qui sera remplacée,
				"bld_save.xpm", // image bidon qui sera remplacée,
				"bld_save.xpm", // image bidon qui sera remplacée,
				0

		};

static const char *tooltips[] = //tooltips
		{ "Print", "Open config", "Save config", "Refresh Spectra List",
				"Enable/Disable status", "Define Cut & Send Cut to Server","Change pad left","Change pad right", 0 };

//______________________________________________________________________________
GVToolBar::GVToolBar(const TGWindow *p) :
	TGToolBar(p) {
	Int_t Nb_std_button = 3; // Defaut button given by root
	Int_t Nb_extra_users_button = 5; //Nb of own buttons
	Int_t Nb_extra_users_image = 2; // Nb of extra Images for user button
	Int_t no;
	Int_t spacenull;
	Int_t space;
	Int_t i;
	//fImg  = (TImage**)malloc((Nb_extra_users_button+Nb_extra_users_image)*sizeof(TImage*));
	//fPicture= (const TGPicture**)malloc((Nb_extra_users_button+Nb_extra_users_image)*sizeof(const TGPicture*));;
	fImg = new TImage*[Nb_extra_users_button + Nb_extra_users_image];
	fPicture = new const TGPicture*[Nb_extra_users_button
			+ Nb_extra_users_image];

	fInfoState = false;
	fCutState = false;
	no = 0;
	fImg[no] = TImage::Create();
	fImg[no]->SetImageBuffer(image_refreshlist, TImage::kXpm);
	fPicture[no] = gClient->GetPicturePool()->GetPicture("image_refrelist",
			fImg[no]->GetPixmap(), fImg[no]->GetMask());

	no = 1;
	fImg[no] = TImage::Create();
	fImg[no]->SetImageBuffer(image_info, TImage::kXpm);
	fPicture[no] = gClient->GetPicturePool()->GetPicture("image_info",
			fImg[no]->GetPixmap(), fImg[no]->GetMask());
	fNbPictureInfo = no;
	fNbButtonInfo = Nb_std_button + no + 1;

	no = 2;
	fImg[no] = TImage::Create();
	fImg[no]->SetImageBuffer(image_cut, TImage::kXpm);
	fPicture[no] = gClient->GetPicturePool()->GetPicture("image_cut",
			fImg[no]->GetPixmap(), fImg[no]->GetMask());
	fNbPictureCut = no;
	fNbButtonCut = Nb_std_button + no + 1;

	no = 3;// image fleche
	fImg[no] = TImage::Create();
	fImg[no]->SetImageBuffer(image_arrow_left, TImage::kXpm);
	fPicture[no] = gClient->GetPicturePool()->GetPicture("image_arrow_left",
			fImg[no]->GetPixmap(), fImg[no]->GetMask());
	fNbPictureInfono = no;

	no = 4;// image fleche
	fImg[no] = TImage::Create();
	fImg[no]->SetImageBuffer(image_arrow_right, TImage::kXpm);
	fPicture[no] = gClient->GetPicturePool()->GetPicture("image_arrow_right",
			fImg[no]->GetPixmap(), fImg[no]->GetMask());
	fNbPictureCutSend = no;


	//___ extra picture to do animation____

	no = 5;// image inverse pour le bouton  info.
			fImg[no] = TImage::Create();
			fImg[no]->SetImageBuffer(image_infono, TImage::kXpm);
			fPicture[no] = gClient->GetPicturePool()->GetPicture("image_infono",
					fImg[no]->GetPixmap(), fImg[no]->GetMask());
			fNbPictureInfono = no;

		no = 6;// image inverse pour le bouton  cut.
		fImg[no] = TImage::Create();
		fImg[no]->SetImageBuffer(image_cut_send, TImage::kXpm);
		fPicture[no] = gClient->GetPicturePool()->GetPicture("image_cut_send",
				fImg[no]->GetPixmap(), fImg[no]->GetMask());
		fNbPictureCutSend = no;



	//	(60, 20, kHorizontalFrame | kRaisedFrame)
	Int_t localspace;
	localspace = 0;
	space = 10;
	spacenull = 0;
	//colors
	gClient->GetColorByName("white", white);

	fMain = (TGMainFrame*) p;

	ToolBarData_t t[Nb_std_button + Nb_extra_users_button];

	for (i = 0; i < Nb_std_button + Nb_extra_users_button; i++) {
		// filling the ToolBarData_t with information
		t[i].fPixmap = images[i]; // icon file
		t[i].fTipText = tooltips[i]; // tool tip text
		t[i].fStayDown = kFALSE; // button behavior if clicked
		t[i].fId = i + 1; // button id
		t[i].fButton = NULL; // button pointer
		if ((strlen(images[i]) == 0)
				|| (strcmp(images[i], "bld_save.xpm") == 0)) {
			localspace = space;
			continue;
		}

		AddButton(fMain, &t[i], localspace);
		localspace = spacenull;
	}

	for (i = Nb_std_button; i < Nb_std_button + Nb_extra_users_button; i++) {

		localspace = spacenull;
		if ( i == 3)
			localspace = space;
		if ( i == 6)
					localspace = 6*space;
		AddButton(fMain, &t[i], localspace);
		((TGPictureButton*) this->GetButton(t[i].fId))->SetPicture(fPicture[i
				- Nb_std_button]);
	}

	GetButton(1)->Connect("Clicked()", "GVToolBar", this, "OpenPrintDialog()");
	GetButton(2)->Connect("Clicked()", "GVToolBar", this, "OpenFile()");
	GetButton(3)->Connect("Clicked()", "GVToolBar", this, "SaveConfig()");
	GetButton(4)->Connect("Clicked()", "GVToolBar", this, "RefreshListAction()");
	GetButton(fNbButtonInfo)->Connect("Clicked()", "GVToolBar", this,
			"SwitchInfo()");
	GetButton(fNbButtonCut)->Connect("Clicked()", "GVToolBar", this, "DoCut()");
	GetButton(7)->Connect("Clicked()", "GVToolBar", this, "GoBack()");
	GetButton(8)->Connect("Clicked()", "GVToolBar", this, "GoForward()");
}
//______________________________________________________________________________
GVToolBar::~GVToolBar() {
	Cleanup();
}
//______________________________________________________________________________
void GVToolBar::OpenFile() {
	Vigru *f = (Vigru*) fMain;
	f->OpenSaveConfig(1);
}
//______________________________________________________________________________
void GVToolBar::SaveConfig() {
	Vigru *f = (Vigru*) fMain;
	f->OpenSaveConfig(2);

}//______________________________________________________________________________
void GVToolBar::OpenPrintDialog() {
	Vigru *f = (Vigru*) fMain;
	f->DisplayPrintDialog();
}

//______________________________________________________________________________
void GVToolBar::RefreshListAction() {
	Vigru *f = (Vigru*) fMain;
	f->GetSpeManager()->GetDB()->DeleteAllIdentities();
	f->GetSpeManager()->UpdateSpectraList();
}
//______________________________________________________________________________
void GVToolBar::SwitchInfo() {
	SetInfo(!fInfoState);
}
//______________________________________________________________________________
void GVToolBar::SetInfo(bool info) {
	Vigru *f = (Vigru*) fMain;
	if (!info) {
		((TGPictureButton*) this->GetButton(fNbButtonInfo))->SetPicture(
				fPicture[fNbPictureInfo]);
		fInfoState = false;
	}
	if (info) {
		((TGPictureButton*) this->GetButton(fNbButtonInfo))->SetPicture(
				fPicture[fNbPictureInfono]);
		fInfoState = true;
	}
	f->ConnectInfos(info);

}

//______________________________________________________________________________
void GVToolBar::DoCut() {
	Vigru *f = (Vigru*) fMain;

	if (fCutState) {
		((TGPictureButton*) this->GetButton(fNbButtonCut))->SetPicture(
				fPicture[fNbPictureCut]);
		fCutState = false;
		f->SendCuts();
	} else {
		((TGPictureButton*) this->GetButton(fNbButtonCut))->SetPicture(
				fPicture[fNbPictureCutSend]);
		fCutState = true;
		f->DefineCuts();
	}
}

//______________________________________________________________________________
void GVToolBar::GoBack() {
	Vigru *f = (Vigru*) fMain;
	f->GoBack();
}
//______________________________________________________________________________
void GVToolBar::GoForward() {
	Vigru *f = (Vigru*) fMain;
	f->GoForward();
}
//______________________________________________________________________________
