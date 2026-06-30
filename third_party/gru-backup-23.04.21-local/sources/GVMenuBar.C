// File : GVMenuBar.C
// Author: Jerome Chauveau & Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//class GVMenuBar
//Menu bar of the ViGRU application
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


/*-------------------------------------------------------------------/
 |class : GVMenuBar                                                   |
 |                                                                    |
 |Description : Menu bar of the GV application                        |
 |                                                                    |
 |                                                                    |
 |                                                                    |
 |                                                                    |
 |                                                                    |
 /-------------------------------------------------------------------*/

#include "GVMenuBar.h"
#include <Riostream.h>
#include<TGFileDialog.h>
#include <TObject.h>
#include "Vigru.h"
#include <TGTextEditDialogs.h>
#include <TRootCanvas.h>

ClassImp( GVMenuBar)

enum ETestCommandIdentifiers {
	M_FILE_OPEN,
	M_FILE_SAVE,
	M_FILE_SAVEAS,
	M_FILE_SPECTRA,
	M_FILE_SPECTRA_TXT,
	M_FILE_OPEN_PAGE,
	M_FILE_SAVE_PAGE,
	M_FILE_PRINT,
	M_FILE_EXIT,
	M_TOOLS_PAGE_LOGX,
	M_TOOLS_PAGE_LOGY,
	M_TOOLS_PAGE_LOGZ,
	M_TOOLS_ALL_PAGE_LOGX,
	M_TOOLS_ALL_PAGE_LOGY,
	M_TOOLS_ALL_PAGE_LOGZ,
	M_TOOLS_REMOVE_PAGE_LOGX,
	M_TOOLS_REMOVE_PAGE_LOGY,
	M_TOOLS_REMOVE_PAGE_LOGZ,
	M_TOOLS_REMOVE_ALL_PAGE_LOGX,
	M_TOOLS_REMOVE_ALL_PAGE_LOGY,
	M_TOOLS_REMOVE_ALL_PAGE_LOGZ,
	M_TOOLS_SET_RANGE_USER,
	M_TOOLS_UNSET_RANGE_USER,
	M_TOOLS_PEAK_FIND,
	M_TOOLS_PEAK_PAGE_FIND,
	M_TOOLS_RESET,
	M_TOOLS_ZOOM_ALL_PAGES,
	M_TOOLS_REFRESH_ALL_PAGES,
	M_CUT_DISPLAY_CUTS,
	M_CUT_CLEAR_CUTS,
	M_CUT_DEFINE_CUTS,
	M_CUT_SEND_CUTS,
	M_CUT_SEND_PAD_CUTS,
	M_CUT_SAVE_CUTS,
	M_CUT_LOAD_CUTS,
	M_CUT_INTEGRALS_IN_CUTS,
	M_EXPERT_REFRESH_PAGE,
	M_EXPERT_REFRESH_ALL_PAGE,
	M_EXPERT_DUMP_PAD_DATA_BASE,
	M_EXPERT_DUMP_SERVER_DATA_BASE,
	M_EXPERT_DUMP_SPECTRA_DATA_BASE,
	M_EXPERT_RESET_SERVER_DATA_BASE,
	M_EXPERT_RESET_SPECTRA_DATA_BASE,
	M_SETUP_SOURCES,
	M_SETUP_QSTART,
	M_DUPLICATION,
	M_CLASSIFICATION_MANUAL,
	M_CLASSIFICATION_AUTO,
	M_HELP_ABOUT,
};

/*------------------------------------------------------*/
//constructor
GVMenuBar::GVMenuBar(const TGWindow *p, UInt_t w, UInt_t h, UInt_t option) :
	TGMenuBar(p, w, h, option) {

	fMain = (TGMainFrame*) p;

	fMenuFile = NULL;
	fSetRangeUser = NULL;
	fSetPeakFind= NULL;
	fMenuSetup = NULL;
	fMenuTools = NULL;
	fMenuCuts = NULL;
	fMenuExpert = NULL;
	fMenuDuplication = NULL;
	fMenuHelp = NULL;
	fMenuClassification = NULL;
	fAbout = NULL;
	fSource = NULL;
	fLastConfigDirFile = "";
	fLastConfigDir = "";
	fMenuBarItemLayout = NULL;
	fMenuBarHelpLayout = NULL;
	fSetDuplication = NULL;


	//colors
	gClient->GetColorByName("white", white);

	//---------------------Menu--------------------
	//pictures
	const TGPicture* pict1 = gClient->GetPicture("bld_open.png");
	const TGPicture* pict2 = gClient->GetPicture("bld_save.png");
	const TGPicture* pict3 = gClient->GetPicture("ed_print.png");
	const TGPicture* pict4 = gClient->GetPicture("ed_saveas.png");
	const TGPicture* pict5 = gClient->GetPicture("bld_exit.png");
	const TGPicture* pict6 = gClient->GetPicture("about.xpm");

	fMenuFile = new TGPopupMenu(gClient->GetRoot());
	fMenuFile->AddEntry("&Open Config", M_FILE_OPEN, 0, pict1);
	fMenuFile->AddEntry("&Save Config", M_FILE_SAVE, 0, pict2);
	fMenuFile->AddEntry("Save &As...", M_FILE_SAVEAS, 0, pict4);
	fMenuFile->AddSeparator();
	fMenuFile->AddEntry("&Open Config of a Page", M_FILE_OPEN_PAGE, 0, pict1);
	fMenuFile->AddEntry("&Save Config Current Page", M_FILE_SAVE_PAGE, 0, pict4);
	fMenuFile->AddSeparator();
	fMenuFile->AddEntry("&Print", M_FILE_PRINT, 0, pict3);
	fMenuFile->AddSeparator();
	fMenuFile->AddEntry("Save Spectra", M_FILE_SPECTRA);
	fMenuFile->AddEntry("Save Spectrum in TXT", M_FILE_SPECTRA_TXT);
	fMenuFile->AddSeparator();
	fMenuFile->AddEntry("E&xit", M_FILE_EXIT, 0, pict5);

	fMenuTools = new TGPopupMenu(gClient->GetRoot());
	fMenuTools->AddEntry("Apply logX on current page", M_TOOLS_PAGE_LOGX);
	fMenuTools->AddEntry("Apply logY on current page", M_TOOLS_PAGE_LOGY);
	fMenuTools->AddEntry("Apply logZ on current page", M_TOOLS_PAGE_LOGZ);
	fMenuTools->AddSeparator();
	fMenuTools->AddEntry("Apply logX on all pages", M_TOOLS_ALL_PAGE_LOGX);
	fMenuTools->AddEntry("Apply logY on all pages", M_TOOLS_ALL_PAGE_LOGY);
	fMenuTools->AddEntry("Apply logZ on all pages", M_TOOLS_ALL_PAGE_LOGZ);
	fMenuTools->AddSeparator();
	fMenuTools->AddEntry("Remove logX on current page",
			M_TOOLS_REMOVE_PAGE_LOGX);
	fMenuTools->AddEntry("Remove logY on current page",
			M_TOOLS_REMOVE_PAGE_LOGY);
	fMenuTools->AddEntry("Remove logZ on current page",
			M_TOOLS_REMOVE_PAGE_LOGZ);
	fMenuTools->AddSeparator();
	fMenuTools->AddEntry("Remove logX on all pages",
			M_TOOLS_REMOVE_ALL_PAGE_LOGX);
	fMenuTools->AddEntry("Remove logY on all pages",
			M_TOOLS_REMOVE_ALL_PAGE_LOGY);
	fMenuTools->AddEntry("Remove logZ on all pages",
			M_TOOLS_REMOVE_ALL_PAGE_LOGZ);
	fMenuTools->AddSeparator();
	fMenuTools->AddSeparator();
	fMenuTools->AddEntry("Set Range User (zoom)", M_TOOLS_SET_RANGE_USER);
	fMenuTools->AddEntry("Unset Range User (unzoom)", M_TOOLS_UNSET_RANGE_USER);
	fMenuTools->AddSeparator();
	fMenuTools->AddSeparator();
	fMenuTools->AddEntry("Peak Find on current pad", M_TOOLS_PEAK_FIND);
	fMenuTools->AddEntry("Peak Find on whole page", M_TOOLS_PEAK_PAGE_FIND);
	fMenuTools->AddSeparator();
	fMenuTools->AddEntry("Reset spectra on pad,page or server", M_TOOLS_RESET);
	fMenuTools->AddSeparator();
	fMenuTools->AddEntry("Zoom all Pages", M_TOOLS_ZOOM_ALL_PAGES);
	fMenuTools->AddSeparator();
	fMenuTools->AddEntry("Refresh all Pages", M_TOOLS_REFRESH_ALL_PAGES);
	fMenuTools->AddSeparator();

	fMenuCuts = new TGPopupMenu(gClient->GetRoot());
	fMenuCuts->AddEntry("Define Cuts", M_CUT_DEFINE_CUTS);
	fMenuCuts->AddEntry("Send new cut to Server", M_CUT_SEND_CUTS);
	fMenuCuts->AddEntry("Send pad  cuts to Server", M_CUT_SEND_PAD_CUTS);
	fMenuCuts->AddEntry("Display Cuts", M_CUT_DISPLAY_CUTS);
	fMenuCuts->AddEntry("Clear Cuts in selected pad", M_CUT_CLEAR_CUTS);
	fMenuCuts->AddSeparator();
	fMenuCuts->AddEntry("Save Pads Cuts", M_CUT_SAVE_CUTS);
	fMenuCuts->AddEntry("Replace Pads Cuts from file Cuts", M_CUT_LOAD_CUTS);
	fMenuCuts->AddSeparator();
	fMenuCuts->AddEntry("Integral in cuts of selected pad",
			M_CUT_INTEGRALS_IN_CUTS);
	fMenuCuts->AddSeparator();

	fMenuDuplication = new TGPopupMenu(gClient->GetRoot());
	fMenuDuplication->AddEntry("Choice a Duplication", M_DUPLICATION);
	fMenuDuplication->AddSeparator();

	fMenuExpert = new TGPopupMenu(gClient->GetRoot());
	fMenuExpert->AddEntry("Simple refresh page", M_EXPERT_REFRESH_PAGE);
	fMenuExpert->AddEntry("Simple refresh all pages", M_EXPERT_REFRESH_ALL_PAGE);
	fMenuExpert->AddEntry("Dump Pad Data Base", M_EXPERT_DUMP_PAD_DATA_BASE);
	fMenuExpert->AddEntry("Dump Server Data Base",
			M_EXPERT_DUMP_SERVER_DATA_BASE);
	fMenuExpert->AddEntry("Dump Spectra Data Base",
			M_EXPERT_DUMP_SPECTRA_DATA_BASE);
	fMenuExpert->AddSeparator();
	fMenuExpert->AddEntry("Erase Server Data Base",
			M_EXPERT_RESET_SERVER_DATA_BASE);
	fMenuExpert->AddEntry("Erase Spectra Data Base",
			M_EXPERT_RESET_SPECTRA_DATA_BASE);
	fMenuExpert->AddSeparator();

	fMenuSetup = new TGPopupMenu(gClient->GetRoot());
	fMenuSetup->AddEntry("Sources", M_SETUP_SOURCES);
	fMenuSetup->AddSeparator();

	fMenuHelp = new TGPopupMenu(gClient->GetRoot());
	fMenuHelp->AddEntry("&About", M_HELP_ABOUT, 0, pict6);
	//---------------------------------------------

	fMenuBarItemLayout
			= new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);
	fMenuBarHelpLayout = new TGLayoutHints(kLHintsTop | kLHintsRight);
	AddPopup("&File", fMenuFile, fMenuBarItemLayout);
	AddPopup("Tools", fMenuTools, fMenuBarItemLayout);
	AddPopup("Setup", fMenuSetup, fMenuBarItemLayout);
	AddPopup("Cuts", fMenuCuts, fMenuBarItemLayout);
	AddPopup("Duplication", fMenuDuplication, fMenuBarItemLayout);
	AddPopup("Expert", fMenuExpert, fMenuBarItemLayout);
	AddPopup("&Help", fMenuHelp, fMenuBarHelpLayout);

	//slots
	fMenuFile->Connect("Activated(Int_t)", "GVMenuBar", this,
			"HandleMenu(Int_t)");
	fMenuTools->Connect("Activated(Int_t)", "GVMenuBar", this,
			"HandleMenu(Int_t)");
	fMenuSetup->Connect("Activated(Int_t)", "GVMenuBar", this,
			"HandleMenu(Int_t)");
	fMenuCuts->Connect("Activated(Int_t)", "GVMenuBar", this,
			"HandleMenu(Int_t)");
	fMenuDuplication->Connect("Activated(Int_t)", "GVMenuBar", this,
			"HandleMenu(Int_t)");
	fMenuExpert->Connect("Activated(Int_t)", "GVMenuBar", this,
			"HandleMenu(Int_t)");
	fMenuHelp->Connect("Activated(Int_t)", "GVMenuBar", this,
			"HandleMenu(Int_t)");


}
//______________________________________________________________________________

//destructor
GVMenuBar::~GVMenuBar() {
	//il ne faut pas de Cleanup() , on a pas de addframe
	/*if (fSetRangeUser) {
		delete (fSetRangeUser);
		fSetRangeUser= NULL;
	}
	if (fSetPeakFind){
		delete (fSetPeakFind);
		fSetPeakFind= NULL;
	}
	if (fAbout){
		delete (fAbout);
		fAbout=NULL;
	}
	if (fSource){
		delete (fSource);
		fSource=NULL;
	}
//	if (fSetDuplication){
	//		delete (fSetDuplication);
		//	fSetDuplication=NULL;
		//}
		 *
		 */
	//Cleanup();
}
//______________________________________________________________________________

//called by user clicks
void GVMenuBar::HandleMenu(Int_t id) {
	Vigru *f = (Vigru*) fMain;
	bool inx;
	inx = true;
	bool iny;
	iny = true;
	bool inz;
	inz = false;
	Double_t xmin;
	xmin =  0.0;
	Double_t xmax;
	xmax = 0.0;
	Double_t ymin;	
	ymin = 0.0;
	Double_t ymax;
	ymax = 0.0;
	Double_t zmin;
	zmin = 0.0;
	Double_t zmax;
	zmax = 0.0;
	int padtaborall = 0;
	// Handle menu items.
	switch (id) {
	case M_FILE_OPEN:
		f->OpenSaveConfig(1);
		break;
	case M_FILE_SAVE:
		f->OpenSaveConfig(2);
		break;
	case M_FILE_SAVEAS:
		f->OpenSaveConfig(3);
		break;
	case M_FILE_SPECTRA:
		f->OpenSaveConfig(4);
		break;
	case M_FILE_OPEN_PAGE:
		f->OpenSaveConfig(5);
		break;
	case M_FILE_SAVE_PAGE:
		f->OpenSaveConfig(6);
		break;
	case M_FILE_SPECTRA_TXT:
		f->OpenSaveConfig(7);
		break;
	case M_FILE_PRINT:
		OpenPrintDialog();
		break;
	case M_FILE_EXIT:
		fMain->CloseWindow(); // terminate theApp no need to use SendCloseMessage()
		break;
	case M_SETUP_SOURCES:
		DisplaySource();
		break;
	case M_TOOLS_PAGE_LOGX:
		f->ApplyPageLog(true, false, false);
		break;
	case M_TOOLS_PAGE_LOGY:
		f->ApplyPageLog(false, true, false);
		break;
	case M_TOOLS_PAGE_LOGZ:
		f->ApplyPageLog(false, false, true);
		break;
	case M_TOOLS_ALL_PAGE_LOGX:
		f->ApplyAllPagesLog(true, false, false);
		break;
	case M_TOOLS_ALL_PAGE_LOGY:
		f->ApplyAllPagesLog(false, true, false);
		break;
	case M_TOOLS_ALL_PAGE_LOGZ:
		f->ApplyAllPagesLog(false, false, true);
		break;

	case M_TOOLS_REMOVE_PAGE_LOGX:
		f->RemovePageLog(true, false, false);
		break;
	case M_TOOLS_REMOVE_PAGE_LOGY:
		f->RemovePageLog(false, true, false);
		break;
	case M_TOOLS_REMOVE_PAGE_LOGZ:
		f->RemovePageLog(false, false, true);
		break;
	case M_TOOLS_REMOVE_ALL_PAGE_LOGX:
		f->RemoveAllPagesLog(true, false, false);
		break;
	case M_TOOLS_REMOVE_ALL_PAGE_LOGY:
		f->RemoveAllPagesLog(false, true, false);
		break;
	case M_TOOLS_REMOVE_ALL_PAGE_LOGZ:
		f->RemoveAllPagesLog(false, false, true);
		break;
	case M_TOOLS_SET_RANGE_USER:{
		fSetRangeUser = new GVSetRangeUser(gClient->GetRoot(), f, 320, 500,
				this, &inx, &iny, &inz, &xmin, &xmax, &ymin, &ymax, &zmin,
				&zmax,&padtaborall,true);
		if (padtaborall!=0){
			f->ApplyRangeUserPage(inx, iny, inz, xmin, xmax, ymin, ymax, zmin, zmax,padtaborall);
			}
		}
		break;
	case M_TOOLS_UNSET_RANGE_USER:{
		padtaborall =0;
		fSetRangeUser = new GVSetRangeUser(gClient->GetRoot(), f, 280, 180,
				this, &inx, &iny, &inz, &xmin, &xmax, &ymin, &ymax, &zmin,
				&zmax,&padtaborall,false);
		if (padtaborall!=0){
			f->CancelZoom(padtaborall);
			}
		}
		break;
	case M_TOOLS_PEAK_FIND:
		fSetPeakFind = new GVSetPeakFind(gClient->GetRoot(), f, 240, 235, this,
				f->GetNbPeaks(), f->GetResolution(), f->GetSigma(),
				f->GetThreshold(), f->GetDisplay_polymarker());
		f->PeakFind();

		break;
	case M_TOOLS_PEAK_PAGE_FIND:
		fSetPeakFind = new GVSetPeakFind(gClient->GetRoot(), f, 240, 235, this,
				f->GetNbPeaks(), f->GetResolution(), f->GetSigma(),
				f->GetThreshold(), f->GetDisplay_polymarker());
		f->PeakPageFind();
		break;
	case M_TOOLS_RESET:
		f->ResetSpectra(true);
		break;
	case M_TOOLS_ZOOM_ALL_PAGES:
		f->ApplyZoomProportionalAllPages();
		break;
	case M_TOOLS_REFRESH_ALL_PAGES:
		f->RefreshAllPages();
		break;
	case M_CUT_DEFINE_CUTS:
		DefineCuts();
		break;
	case M_CUT_SEND_CUTS:
		SendCuts();
		break;
	case M_CUT_SEND_PAD_CUTS:
		f->SendPadCuts();
		break;
	case M_CUT_DISPLAY_CUTS:
		DisplayCuts();
		break;
	case M_CUT_CLEAR_CUTS:
		ClearCuts();
		break;
	case M_CUT_SAVE_CUTS:
		SaveCuts();
		break;
	case M_CUT_LOAD_CUTS:
		f->LoadPadCuts();
		break;

	case M_CUT_INTEGRALS_IN_CUTS:
		IntegralsInCuts();
		break;
	case M_DUPLICATION:
		SetDuplication();
		break;
	case M_EXPERT_REFRESH_PAGE:
		f->RefreshPage(true, false);
		break;
	case M_EXPERT_REFRESH_ALL_PAGE:
		f->RefreshAllPages(false);
		break;
	case M_EXPERT_DUMP_PAD_DATA_BASE:
		f->DumpPadDataBase();
		break;
	case M_EXPERT_DUMP_SERVER_DATA_BASE:
		f->GetSpeManager()->GetServerDB()->MakeDUMP(0, true);
		break;
	case M_EXPERT_DUMP_SPECTRA_DATA_BASE:
		f->GetSpeManager()->GetDB()->MakeDUMP(0, false);
		break;
	case M_EXPERT_RESET_SERVER_DATA_BASE:
		f->GetSpeManager()->GetServerDB()->DeleteAllIdentities(false);
		break;
	case M_EXPERT_RESET_SPECTRA_DATA_BASE:
		f->GetSpeManager()->GetDB()->DeleteAllIdentities(false);
		break;

	case M_HELP_ABOUT:
		DisplayAbout();
		break;
	default:
		printf("Menu item /home/legeard/GRU/GRU/linux/vigru:%d selected\n", id);
		break;
	}
}

//______________________________________________________________________________
//displays the About Frame
void GVMenuBar::DisplayAbout() {
	fAbout = new GVDialogAbout(gClient->GetRoot(), fMain, 420, 200);

}
//______________________________________________________________________________

//Displays dialog to configure net & file sources
void GVMenuBar::DisplaySource() {
	fSource = new GVDialogSource(gClient->GetRoot(), fMain, 540, 580);
}
//______________________________________________________________________________

//return the GVDialogSource dialog
GVDialogSource *GVMenuBar::GetDialogSource() {
	return fSource;
}
//______________________________________________________________________________
void GVMenuBar::DefineCuts() {
	Vigru *f = (Vigru*) fMain;
	f->DefineCuts();
}
//______________________________________________________________________________
void GVMenuBar::SendCuts() {
	Vigru *f = (Vigru*) fMain;
	f->SendCuts();
}
//______________________________________________________________________________
void GVMenuBar::DisplayCuts() {
	Vigru *f = (Vigru*) fMain;
	f->DisplayCuts();
}
//______________________________________________________________________________
void GVMenuBar::ClearCuts() {
	Vigru *f = (Vigru*) fMain;
	f->ClearCuts();
}
//______________________________________________________________________________
void GVMenuBar::SaveCuts() {
	Vigru *f = (Vigru*) fMain;
	Int_t level = 3;
	f->SaveCuts(level);
}

//______________________________________________________________________________
void GVMenuBar::IntegralsInCuts() {
	Vigru *f = (Vigru*) fMain;
	f->IntegralsInCuts();
}

//______________________________________________________________________________
void GVMenuBar::SetDuplication() {
	// modify the name Duplication and from which tab/pad the Computation must be apply

	Int_t computation, tab, pad;
	computation = -1;
	tab = -1;
	pad = -1;
	Vigru *main = (Vigru*) fMain;

	fSetDuplication = new GVSetDuplication(gClient->GetRoot(), main, 520, 215,
			&tab, &pad, &computation);

	if ((computation >= 0) && (tab >= 0) && (pad > 0)) {
		main->ApplyComputation(computation, tab, pad);
	}
}
//______________________________________________________________________________

//
void GVMenuBar::OpenPrintDialog() {
	Vigru *f = (Vigru*) fMain;
	f->DisplayPrintDialog();
}
//______________________________________________________________________________
