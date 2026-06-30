// File : GVToolBarVS.C
// Author: luc Legeard
//////////////////////////////////////////////////////////////////////////////
//class GVToolBarVS
//  Toolbar  Vetical for spectrum page
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

#include "GVToolBarVS.h"
#include "Vigru.h"
#include "GVPad.h"
#include <TGToolBar.h>
#include <TGFileDialog.h>
#include <TGButton.h>
#include <TImage.h>
#include <TGPicture.h>
#include <TBox.h>
#include <TGMsgBox.h>
#include <TImage.h>

#include "GVSetPage.h"
#include "GVSetStat.h"
#include "GVSetReset.h"

#include "images/image_addpage.h"
#include "images/image_rempage.h"
#include "images/image_setpage.h"
#include "images/image_display.h"
#include "images/image_displaymulti.h"
#include "images/image_refreshpad.h"
#include "images/image_refreshpag.h"
#include "images/image_refreshtimer.h"
#include "images/image_logx.h"
#include "images/image_logz.h"
#include "images/image_logy.h"
#include "images/image_stat.h"

#include "images/image_z1.h"
#include "images/image_z2.h"
#include "images/image_z31.h"
#include "images/image_z32.h"
#include "images/image_rz.h"
#include "images/image_cancelzoom.h"
#include "images/image_FFT.h"

ClassImp(GVToolBarVS)

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
	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	"bld_save.xpm", // image bidon qui sera remplacée,
	0

};
//______________________________________________________________________________
static const char *tooltips[] = //tooltips
		{ "Add Page", "Remove Page",
				"Set Page", "Display one Histo", "Display several Histo",
				"Refresh Selected Pad", "Refresh Page", "Set Auto Refresh Page",
				"Change Log X", "Change Log Y", "Change Log Z","Statistics", "Zoom 1", "Zoom 2", "Zoom 31",
				"Zoom 32","Cancel Pad Zoom" , "Reset Histogram","Apply FFT",0 };
//______________________________________________________________________________
GVToolBarVS::GVToolBarVS(const TGWindow *p) :
	TGToolBar(p) {

	Int_t Nb_std_button = 0;
	Int_t Nb_extra_users_button = 19;
	Int_t no;
	Int_t spacenull;
	Int_t space;
	Int_t i;
	TImage *img[Nb_extra_users_button];
	const TGPicture *picture[Nb_extra_users_button];
	this->ChangeOptions(kVerticalFrame);


	no=0;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_addpage, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_addpage", img[no]->GetPixmap(),
			img[no]->GetMask());
	no=1;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_rempage, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_remopage", img[no]->GetPixmap(),
			img[no]->GetMask());
	no=2;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_setpage, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_setepage", img[no]->GetPixmap(),
			img[no]->GetMask());


	no=3;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_display, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_display", img[no]->GetPixmap(),
			img[no]->GetMask());

	no=4;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_displaymulti, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_displaymulti", img[no]->GetPixmap(),
			img[no]->GetMask());


	no=5;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_refreshpad, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_refreshpad", img[no]->GetPixmap(),
			img[no]->GetMask());

	no=6;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_refreshpag, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_refreshpag", img[no]->GetPixmap(),
			img[no]->GetMask());

	no=7;
		img[no] = TImage::Create();
		img[no]->SetImageBuffer(image_refreshtimer, TImage::kXpm);
		picture[no] = gClient->GetPicturePool()->GetPicture("image_refreshtimer", img[no]->GetPixmap(),
				img[no]->GetMask());


	no=8;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_logx, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_logx", img[no]->GetPixmap(),
			img[no]->GetMask());

	no=9;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_logy, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_logy", img[no]->GetPixmap(),
			img[no]->GetMask());

	no=10;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_logz, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_logz", img[no]->GetPixmap(),
			img[no]->GetMask());

	no=11;
		img[no] = TImage::Create();
		img[no]->SetImageBuffer(image_stat, TImage::kXpm);
		picture[no] = gClient->GetPicturePool()->GetPicture("image_stat", img[no]->GetPixmap(),
				img[no]->GetMask());

	no=12;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_z1, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_z1", img[no]->GetPixmap(),
			img[no]->GetMask());

	no=13;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_z2, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_z2", img[no]->GetPixmap(), img[no]->GetMask());

	no=14;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_z31, TImage::kXpm);
	picture[no] = gClient->GetPicturePool()->GetPicture("image_z31", img[no]->GetPixmap(), img[no]->GetMask());

	no=15;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_z32, TImage::kXpm);
	picture[no]
			= gClient->GetPicturePool()->GetPicture("image_z32", img[no]->GetPixmap(),
					img[no]->GetMask());




	no=16;
		img[no] = TImage::Create();
		img[no]->SetImageBuffer(image_cancelzoom, TImage::kXpm);
		picture[no]
				= gClient->GetPicturePool()->GetPicture("image_cancelzoom", img[no]->GetPixmap(),
						img[no]->GetMask());

	no=17;
	img[no] = TImage::Create();
	img[no]->SetImageBuffer(image_rz, TImage::kXpm);
	picture[no]
			= gClient->GetPicturePool()->GetPicture("image_rz", img[no]->GetPixmap(),
					img[no]->GetMask());

	no=18;
					img[no] = TImage::Create();
					img[no]->SetImageBuffer(image_FFT, TImage::kXpm);
					picture[no]
							= gClient->GetPicturePool()->GetPicture("image_FFT", img[no]->GetPixmap(),
									img[no]->GetMask());

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
		if (i==0)localspace =space;
		if (i==3)localspace =space;
		if (i==5)localspace =space;
		if (i==8)localspace =space;
		if (i==12)localspace =space;
		if (i==16)localspace =space;
		AddButton(fMain, &t[i], localspace);
		((TGPictureButton*)this->GetButton(t[i].fId))->SetPicture(picture[i-Nb_std_button]);
	}

i=1;
	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "AddPage()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "RemovePage()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "SetPage()");

	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this,
			"DisplayDisplayingTreeOne()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this,
			"DisplayDisplayingTreeSev()");

	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "RefreshPad()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "RefreshPage()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "SetRefresh()");

	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "SetLogX()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "SetLogY()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "SetLogZ()");

	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "Statistics()");

	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "Zoom1Action()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "Zoom2Action()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "Zoom31Action()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "Zoom32Action()");

	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "CancelZoom()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "ResetAction()");
	GetButton(i++)->Connect("Clicked()", "GVToolBarVS", this, "FFTAction()");


}

//______________________________________________________________________________

void GVToolBarVS::ResetAction() {
	// modifie the Reset effect

	Vigru *f = (Vigru*)GetMainFrame();

	f->ResetSpectra();
}
//______________________________________________________________________________
void GVToolBarVS::FFTAction() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->FFT();
}
//______________________________________________________________________________
void GVToolBarVS::Zoom1Action() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->Zoom(0,1);
}
//______________________________________________________________________________
void GVToolBarVS::Zoom2Action() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->Zoom(1,1);

}
//______________________________________________________________________________
void GVToolBarVS::Zoom31Action() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->Zoom(2,1);
}
//______________________________________________________________________________
void GVToolBarVS::Zoom32Action() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->Zoom(2,2);
}
//______________________________________________________________________________
void GVToolBarVS::SetPage() {// modifie the name or the number of
	Int_t nbTab, nbPadsX, nbPadsY;
	nbTab =0;
	nbPadsX =0;
	nbPadsY =0;
	TString pageName, pageNameOld;


	Vigru *main = (Vigru*)GetMainFrame();


	pageName.Form("%s", main->GetTabPage()->GetTabTab(main->GetTabPage()->GetCurrent())->GetString());

	pageNameOld = pageName;

	TString tempo ="Set Page";

fSetPage=	new GVSetPage(gClient->GetRoot(),
			GetMainFrame(), 460, 150,tempo,nbTab,&pageName,&nbPadsX,&nbPadsY);

	main->SetPageChange(pageName, nbPadsX, nbPadsY);
}
//______________________________________________________________________________
void GVToolBarVS::AddPage() {
	//call the mainFrame addPage method
	int limit_tab;
	limit_tab = 512;
	Int_t nbTab;

	Int_t nbPadsX;
	Int_t nbPadsY;
	TString pageName, pageNameOld;

	Vigru *main = (Vigru*)GetMainFrame();

	nbPadsX=1;
	nbPadsY=1;
	nbTab = main->GetTabPage()->GetNumberOfTabs();
	pageName.Form(" Tab_%d", nbTab);
	pageNameOld = pageName;

	if (nbTab > limit_tab) {
		TString tempo;
		tempo.Form("Limit of %d  tabs reached !", limit_tab);
		fError.TreatError(1, 0, tempo);
		return;
	}
	TString tempo ="Add Page";
	fSetPage = new GVSetPage
	(gClient->GetRoot(),GetMainFrame(), 460, 150,tempo,nbTab,&pageName,&nbPadsX,&nbPadsY);
	main->AddPage(pageName, nbPadsX, nbPadsY);
}
//______________________________________________________________________________
void GVToolBarVS::RemovePage() {
	//call the mainframe removePage method

	Vigru *f = (Vigru*)GetMainFrame();

	f->GetTabPage()->RemoveCurrentPage();
}
//______________________________________________________________________________
void GVToolBarVS::RefreshPad() {


	Vigru *f = (Vigru*)GetMainFrame();

	f->RefreshPad();
}
//______________________________________________________________________________
void GVToolBarVS::RefreshPage() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->RefreshPage();
}
//______________________________________________________________________________
void GVToolBarVS::SetRefresh()
{
	// GVSetRefreshTimer

	Vigru *f = (Vigru*)GetMainFrame();

		Int_t time ;
		fSetRefreshTimer = new GVSetRefreshTimer(gClient->GetRoot(),GetMainFrame(),180, 95,this,&time) ;
		if (time >= 0)
   	f->SetStateTimer(time);

}
//______________________________________________________________________________
void GVToolBarVS::DisplayDisplayingTreeSev() {


	Vigru *f = (Vigru*)GetMainFrame();

	TString entete ="Display Spectrum";
	Int_t oneselected;
	oneselected =2;
	Int_t operation;
	operation =0;
	if (f->GetSpeManager()->GetDB()->GetLast()>=0) {
		fDisplaySpectraChooser = new GVSpectraChooser(gClient->GetRoot(),GetMainFrame(), 450, 620,entete,1,&oneselected,&operation,1);
		if (!fDisplaySpectraChooser)
			return;
		fDisplaySpectraChooser->Init(f->GetSpeManager()->GetDB());

		if (oneselected>=0)
			f->DisplaySpectra(operation);
	} else {
		MessageBoxNoHisto();
	}
}
//______________________________________________________________________________
void GVToolBarVS::DisplayDisplayingTreeOne() {

	Vigru *f = (Vigru*)GetMainFrame();

	Int_t oneselected;
	oneselected =1;
	Int_t operation;
	operation =0;
	if (f->GetSpeManager()->GetDB()->GetLast()>=0) {
		TString entete ="Display Spectra";
		fDisplaySpectraChooser = new GVSpectraChooser(gClient->GetRoot(),GetMainFrame(), 450, 620,entete,1,&oneselected,&operation,1);
		if (!fDisplaySpectraChooser)
			return;

 		fDisplaySpectraChooser->Init(f->GetSpeManager()->GetDB());

		if (oneselected>=0)
			f->DisplaySpectrum(oneselected,operation);
	} else {
		MessageBoxNoHisto();
	}
}

//______________________________________________________________________________

void GVToolBarVS::Statistics() {
	// modifie the name or the number of Statistics

    bool page;
    page =false;

	Vigru *f = (Vigru*)GetMainFrame();

	GVPad* gvpad ;
	TString StatOpt;
	gvpad= (GVPad*)f->GetSelectedPad();
	StatOpt=gvpad->GetStatOpt();



	if (f->GetSpeManager()->GetDB()->GetLast()>=0) {
		TString pageName;
		pageName.Form("test");
		fSetStat = new GVSetStat(gClient->GetRoot(),
				f,
				190, 230,this,
				(char*)"Set Stat",
				&StatOpt, &page);
	} else {
		MessageBoxNoHisto();
		return;
	}
	f->ApplyStatistics(StatOpt,page);
}
//______________________________________________________________________________

void GVToolBarVS::SetLogX() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->ChangingLog(true, false, false);
}
//______________________________________________________________________________
void GVToolBarVS::SetLogY() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->ChangingLog(false, true, false);
}//______________________________________________________________________________
void GVToolBarVS::SetLogZ() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->ChangingLog(false, false, true);
}
//______________________________________________________________________________
void GVToolBarVS::MessageBoxNoHisto() {

	Vigru *f = (Vigru*)GetMainFrame();

	EMsgBoxIcon icontype = kMBIconExclamation;
	//gClient->GetRoot()
	new TGMsgBox(gClient->GetRoot(), f,
			"Warning", "List of Spectra is empty, try a \"Refresh Spectra List\"",
			icontype, kMBOk);
}
//______________________________________________________________________________
void GVToolBarVS::CancelZoom() {

	Vigru *f = (Vigru*)GetMainFrame();

	f->CancelZoom();
}
//______________________________________________________________________________
