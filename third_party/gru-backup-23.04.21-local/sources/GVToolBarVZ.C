// File : GVToolBarVZ.C
// Author: luc Legeard
//////////////////////////////////////////////////////////////////////////////
//class GVToolBarVZ
//  Toolbar  Vetical for zoom page
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

#include "GVToolBarVZ.h"
#include "Vigru.h"
#include <TGToolBar.h>
#include<TGFileDialog.h>
#include <TGButton.h>
#include <TImage.h>
#include <TGPicture.h>
#include <TBox.h>
#include <TGMsgBox.h>
#include <TImage.h>

#include "images/image_memorize.h"
#include "images/image_drawpanel.h"
#include "images/image_fitpanel.h"
#include "images/image_palette.h"
#include "images/image_refreshpag.h"
#include "images/image_refreshtimer.h"
#include "images/image_fitbg.h"
#include "images/image_logx.h"
#include "images/image_logz.h"
#include "images/image_logy.h"

#include "../sources/images/image_autozoom.h"
#include "images/image_cancelzoom.h"
#include "images/image_rz.h"



ClassImp(GVToolBarVZ)

static const char *images[] = //images
{

	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	0

};

static const char *tooltips[] = //tooltips
		{"Change Memorize", "Draw Panel", "Color Palette for 2D histo", "Refresh Page", "Set Auto Refresh",
				"Fit Panel", "Fit with background", "Change Log X","Change Log Y", "Change Log Z", "Auto Zoom XY",
				"Zoom Cancel","Reset Histogram", 0 };

GVToolBarVZ::GVToolBarVZ(const TGWindow *p) :
	TGToolBar(p) {
	Int_t Nb_std_button = 0;
	Int_t Nb_extra_users_button = 13;
	Int_t no;
	Int_t spacenull;
	Int_t space;
	Int_t i;
	TImage *img[Nb_extra_users_button];
	const TGPicture *picture[Nb_extra_users_button];
	this->ChangeOptions(kVerticalFrame);

	no=0;
		img[no] = TImage::Create();
		img[no]->SetImageBuffer(image_memorize, TImage::kXpm);
		picture[no] = gClient->GetPicturePool()->GetPicture("image_memorize", img[no]->GetPixmap(),
				img[no]->GetMask());
	no=1;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_drawpanel, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_drawpanel", img[no]->GetPixmap(),
			img[no]->GetMask());
	no=2;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_palette, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_palette", img[no]->GetPixmap(),
			img[no]->GetMask());
	no=3;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_refreshpag, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_refreshpag", img[no]->GetPixmap(),
			img[no]->GetMask());

	no=4;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_refreshtimer, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_refreshtimer", img[no]->GetPixmap(),
			img[no]->GetMask());

	no=5;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_fitpanel, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_fitpanel", img[no]->GetPixmap(),
			img[no]->GetMask());

	no=6;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_fitbg, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_fitbg", img[no]->GetPixmap(),
			img[no]->GetMask());

	no=7;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_logx, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_logx", img[no]->GetPixmap(),
			img[no]->GetMask());

	no=8;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_logy, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_logy", img[no]->GetPixmap(),
			img[no]->GetMask());

	no=9;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_logz, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_logz", img[no]->GetPixmap(),
			img[no]->GetMask());

	no=10;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_autozoom, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_autozoom", img[no]->GetPixmap(),
			img[no]->GetMask());

	no=11;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_cancelzoom, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_canclezoom", img[no]->GetPixmap(),
			img[no]->GetMask());

	no=12;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_rz, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_rz", img[no]->GetPixmap(), img[no]->GetMask());

	//	(60, 20, kHorizontalFrame | kRaisedFrame)
	Int_t localspace;

	spacenull=4;
	space=spacenull;
	localspace=spacenull;
	//colors
	gClient->GetColorByName("white", white);

	fMain = (TGMainFrame*)p;

	ToolBarData_t t[Nb_std_button+Nb_extra_users_button];
	for (i = 0; i<Nb_std_button+Nb_extra_users_button; i++) {
		// filling the ToolBarData_t with information
		t[i].fPixmap = images[i]; // icon file
		t[i].fTipText = tooltips[i]; // tool tip text
		t[i].fStayDown = kFALSE; // button behavior if clicked
		t[i].fId = i+1; // button id
		t[i].fButton = NULL; // button pointer
		if ((strlen(images[i])==0)||(strcmp(images[i], "bld_save.xpm")==0)) {
			localspace=space;
			continue;
		}
		AddButton(fMain, &t[i], localspace);
		space=0;
	}

	for (i = Nb_std_button; i<Nb_std_button+Nb_extra_users_button; i++) {

		localspace=spacenull;
		if (i==0)
			localspace =space;
		if (i==2)
			localspace =space;
		if (i==4)
			localspace =space;
		if (i==6)
			localspace =space;
		if (i==9)
			localspace =space;
		if (i==11)
			localspace =space;
		AddButton(fMain, &t[i], localspace);
		((TGPictureButton*)this->GetButton(t[i].fId))->SetPicture(picture[i-Nb_std_button]);
	}

	i=1;
	GetButton(i++)->Connect("Clicked()", "GVToolBarVZ", this, "Memorize()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVZ", this, "DrawPanel()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVZ", this, "DrawPalette()");

	GetButton(i++)->Connect("Clicked()", "GVToolBarVZ", this, "RefreshPage()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVZ", this, "SetRefresh()");

	GetButton(i++)->Connect("Clicked()", "GVToolBarVZ", this, "FitPanel()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVZ", this, "FitBG()");

	GetButton(i++)->Connect("Clicked()", "GVToolBarVZ", this, "SetLogX()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVZ", this, "SetLogY()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVZ", this, "SetLogZ()");


	GetButton(i++)->Connect("Clicked()", "GVToolBarVZ", this, "AutoZoom()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVZ", this, "CancelZoom()");

	GetButton(i++)->Connect("Clicked()", "GVToolBarVZ", this, "ResetHisto()");

}

//______________________________________________________________________________
void GVToolBarVZ::Memorize(){

	Vigru *f = (Vigru*)GetMainFrame();

		f->ZoomBack();
}
//______________________________________________________________________________
void GVToolBarVZ::DrawPanel(){

	Vigru *f = (Vigru*)GetMainFrame();

		f->DrawPanel();
}
//______________________________________________________________________________
void GVToolBarVZ::DrawPalette(){

	Vigru *f = (Vigru*)GetMainFrame();

	f->DrawPalette();
}

//______________________________________________________________________________
void GVToolBarVZ::FitPanel() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->FitPanel();
}
//______________________________________________________________________________
void GVToolBarVZ::FitBG() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->FitBG();
}

//______________________________________________________________________________
void GVToolBarVZ::RefreshPage() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->RefreshPage();
}
//______________________________________________________________________________
void GVToolBarVZ::SetRefresh() {// GVZetRefreshTimer

	Vigru *f = (Vigru*)GetMainFrame();

	Int_t time;
	fSetRefreshTimer = new GVSetRefreshTimer(gClient->GetRoot(),GetMainFrame(),180, 95,this,&time);
	if (time >= 0)
		f->SetStateTimer(time);
}
//______________________________________________________________________________
void GVToolBarVZ::SetLogX() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->ChangingLog(true, false, false);
}
//______________________________________________________________________________
void GVToolBarVZ::SetLogY() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->ChangingLog(false, true, false);
}
//______________________________________________________________________________
void GVToolBarVZ::SetLogZ() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->ChangingLog(false, false, true);
}
//______________________________________________________________________________
void GVToolBarVZ::AutoZoom() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->AutoZoom();
}
//______________________________________________________________________________
void GVToolBarVZ::CancelZoom() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->CancelZoom();
}
//______________________________________________________________________________
void GVToolBarVZ::ResetHisto() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->ResetSpectra();
}
//______________________________________________________________________________
void GVToolBarVZ::MessageBoxNoHisto() {

	Vigru *f = (Vigru*)GetMainFrame();

	EMsgBoxIcon icontype = kMBIconExclamation;
	//gClient->GetRoot()
	new TGMsgBox(gClient->GetRoot(), f,
			"Warning", "List of Spectra is empty, try a \"Refresh Spectra List\"",
			icontype, kMBOk);
}
//______________________________________________________________________________
