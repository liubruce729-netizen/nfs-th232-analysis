// File : Vigru.C
// Author:Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//class Vigru
//Description : The Main Frame of the GV application. It contains,
//a menu, a toolbar, spectras menu, zoom frames and all canvas
////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/


// Construction of application :
// ***** visible
// ----- invisible (just to construct arrangement)

//Vigru(TGMainFrame)
// ******************************************************************************
// * fMenuBar(GVMenuBar)                                                        *
// * fToolBar(GVToolBar)                                                        *
// *                                                                            *
// * -------- fMainWindows (TGTransientFrame)---------------------------------- *
// * |                                                                        | *
// * | *fTab(TGTab)********************************************************** | *
// * | *                                                                    * | *
// * | * *****fTabSpectra*****************************************          * | *
// * | * *  fToolBarVS(GVToolBarVS)                              *          * | *
// * | * *                                                       *          * | *
// * | * *  --fTabCanvas(TGCompositeFrame)---------------------  *          * | *
// * | * *  |  **fTabPage (GVTab)***************************** | *          * | *
// * | * *  |  *                                             * | *          * | *
// * | * *  |  *                                             * | *          * | *
// * | * *  |  *                                             * | *          * | *
// * | * *  |  *                                             * | *          * | *
// * | * *  |  *********************************************** | *          * | *
// * | * *  ---------------------------------------------------  *          * | *
// * | * *********************************************************          * | *
// * | *                                                                    * | *
// * | *   *****fTabZoom********************************************        * | *
// * | *   *  fToolBarVZ(GVToolBarVZ)                              *        * | *
// * | *   *                                                       *        * | *
// * | *   *  --fZoomCenter(TGCompositeFrame)--------------------  *        * | *
// * | *   *  |  **fTabZooms (GVTab)**************************** | *        * | *
// * | *   *  |  *                                             * | *        * | *
// * | *   *  |  *                                             * | *        * | *
// * | *   *  |  *                                             * | *        * | *
// * | *   *  |  *                                             * | *        * | *
// * | *   *  |  *********************************************** | *        * | *
// * | *   *  ---------------------------------------------------  *        * | *
// * | *   *********************************************************        * | *
// * | *                                                                    * | *
// * | *     *****fTabConsol******************************************      * | *
// * | *     *                                                       *      * | *
// * | *     *********************************************************      * | *
// * | ********************************************************************** | *
// * -------------------------------------------------------------------------- *
// * sliders
// ******************************************************************************

#include <sys/stat.h>
#include <iostream>
#include <math.h>
#include <Riostream.h>

#include  "Vigru.h"
#include  "GSpectrumIdentity.h"
#include  "GVToolBarVZ.h"
#include  "GVToolBarVS.h"
#include  "GVSetReset.h"
#include <TBrowser.h>
#include <TStyle.h>
#include <Riostream.h>
#include <TApplication.h>
#include <TCanvas.h>
#include <TGMsgBox.h>
#include <Riostream.h>
#include <TGFileDialog.h>
#include <TFile.h>
#include <TAttFill.h>
#include <TObjString.h>
#include <TBox.h>
#include <TGTextEditDialogs.h>
#include <TString.h>
#include <TROOT.h>
#include <TMath.h>
#include <TGFrame.h>
#include <TString.h>
#include <GVPad.h>
#include "../sources/GDevice.h"


ClassImp( Vigru)

//constructor
Vigru::Vigru(const TGWindow *p, UInt_t w, UInt_t h, TString config_file,
		TString rootfile, TString rootserver, TString soapserver,
		Int_t rootport, Int_t soapport, int mode) :
	TGMainFrame(p, w, h) {
	// if (p==NULL) p = gClient->GetRoot();
	Initialization(config_file, rootfile, rootserver, soapserver, rootport,
			soapport, mode);
}

void Vigru::Initialization(TString config_file, TString rootfile,
		TString rootserver, TString soapserver, Int_t rootport, Int_t soapport,
		int mode) {
	fTerminate = true;
	fMenuBar = NULL;
	fToolBar = NULL;
	fConsol = NULL;
	fTab = NULL;
	fTabPage = NULL;
	fTabZooms = NULL;
	fTabSpectra = NULL;
	fTabZoom = NULL;
	fTabCanvas = NULL;
	fTab0 = NULL;
	fZoomCenter = NULL;
	fMainBaseFrame_this = this;
	fVsliderS = NULL;
	fHsliderS = NULL;
	fVsliderZ = NULL;
	fHsliderZ = NULL;
	fTabConsol = NULL;
	fFromPad = NULL;
	fAcqNumexo = NULL;
	fLh = NULL;
	fLastConfigPageDirFile = "";
	fLastConfigDirFile = "";
	fLastConfigSpeFile = "";
	fLastConfigCutFile = "";
	fLastConfigDir = ".";
	fSpecificMode = mode;

	fPortdef = 9090;
	fStatusBar = NULL;
	fTip = NULL;
	fTimer = NULL;
	fTimerZoom = NULL;
	fPrintDialog = NULL;
	fToolBarVS = NULL;
	fToolBarVZ = NULL;
	fSpeManager = new GSpectra();
	fVerbose =0;
	//parameter for peak find
	fNbPeaks = 10;
	fResolution = 1.0;
	fSigma = 2.0;
	fThreshold = 0.05;
	fDisplay_polymarker = true;
	fVerboseDone =false;
	fResetOption = "hn";

	// make sure that the Gpad and GUI libs are loaded
	TApplication::NeedGraphicsLibs();
	gApplication->InitializeGraphics();

	//colors
	gClient->GetColorByName("white", white);
	gClient->GetColorByName("orange", orange);
	gClient->GetColorByName("#a0e0ff", blue);

	gStyle->SetPalette(1);
	TString tempo = GRU_VERSION;
	tempo = " Vigru  " + tempo;
	SetWindowName(tempo.Data());
	SetBackgroundColor(white);

	fMenuBar = new GVMenuBar(fMainBaseFrame_this, 1, 1, kHorizontalFrame);//menubar
	fToolBar = new GVToolBar(fMainBaseFrame_this);//toolbar

	fMainWindows = new TGTransientFrame(fMainBaseFrame_this);
	fMainWindows->SetBackgroundColor(white);

	fTab = new TGTab(fMainWindows, 0);

	//----------------------------------Spectra tab-------------------------------
	fTabSpectra = fTab->AddTab("Spectra");
	fTab->GetTabTab("Spectra")->ChangeBackground(0x00DFFF);
	fTabSpectra->SetLayoutManager(new TGHorizontalLayout(fTabSpectra));
	//fTabSpectra->SetBackgroundColor(white);
	fToolBarVS = new GVToolBarVS(fTabSpectra);

	fTabCanvas = new TGCompositeFrame(fTabSpectra);
	fTabCanvas->SetBackgroundColor(white);
	fTabPage = new GVTab(fTabCanvas, this);
	fTabPage->SetBackgroundColor(white);

	const char* text = "";
	fTabPage->AddTab(text, 2, 2);

	fVsliderS = new TGDoubleVSlider(fTabSpectra, 585);
	fHsliderS = new TGDoubleHSlider(fTabCanvas, 735);
	fVsliderS ->SetRange(0, 100);
	fHsliderS ->SetRange(0, 100);
	SetSPositionReset();

	fTabSpectra->AddFrame(fToolBarVS, new TGLayoutHints(kLHintsExpandY));
	fTabSpectra->AddFrame(fVsliderS, new TGLayoutHints(kLHintsExpandY));
	fTabSpectra->AddFrame(fTabCanvas, new TGLayoutHints(kLHintsRight
			| kLHintsExpandX | kLHintsExpandY));
	fTabCanvas->AddFrame(fTabPage, new TGLayoutHints(kLHintsExpandX
			| kLHintsExpandY));
	fTabCanvas->AddFrame(fHsliderS, new TGLayoutHints(kLHintsExpandX));

	fVsliderS->Connect("Released()", "Vigru", this, "SetVSPosition()");
	fHsliderS->Connect("Released()", "Vigru", this, "SetHSPosition()");
	if (false) {

		fTabSpectra = fTab->AddTab("Spectra");
		fTab->GetTabTab("Spectra")->ChangeBackground(0x00DFFF);
		fTabSpectra->SetLayoutManager(new TGHorizontalLayout(fTabSpectra));
		//fTabSpectra->SetBackgroundColor(white);
		fToolBarVS = new GVToolBarVS(fTabSpectra);

		fTabCanvas = new TGCompositeFrame(fTabSpectra);

		fTabCanvas->SetBackgroundColor(white);

		// J'en suis la ... cette ligna fait tout sauter dans le delete
		fTabPage = new GVTab(fTabCanvas, fMainBaseFrame_this);

		//	fTabPage->SetBackgroundColor(white);
		fTabCanvas = new TGCompositeFrame(fTabSpectra);
		fTabCanvas->SetBackgroundColor(white);
		const char* text = "";
		if (fTabPage)
			fTabPage->AddTab(text, 2, 2);

		fVsliderS = new TGDoubleVSlider(fTabSpectra, 585);
		fHsliderS = new TGDoubleHSlider(fTabCanvas, 735);
		fVsliderS ->SetRange(0, 100);
		fHsliderS ->SetRange(0, 100);
		SetSPositionReset();

		fTabSpectra->AddFrame(fToolBarVS, new TGLayoutHints(kLHintsExpandY));
		fTabSpectra->AddFrame(fVsliderS, new TGLayoutHints(kLHintsExpandY));
		fTabSpectra->AddFrame(fTabCanvas, new TGLayoutHints(kLHintsRight
				| kLHintsExpandX | kLHintsExpandY));

		//if (fTabPage and fTabCanvas)
		fTabCanvas->AddFrame(fTabPage, new TGLayoutHints(kLHintsExpandX
				|| kLHintsExpandY));

		fTabCanvas->AddFrame(fHsliderS, new TGLayoutHints(kLHintsExpandX));

		fVsliderS->Connect("Released()", "Vigru", fMainBaseFrame_this,
				"SetVSPosition()");
		fHsliderS->Connect("Released()", "Vigru", fMainBaseFrame_this,
				"SetHSPosition()");

	}

	//--------------------------------Zoom tab------------------------------------
	if (fTab) {
		fTabZoom = fTab->AddTab("Zoom");
		fTab->GetTabTab("Zoom")->ChangeBackground(blue);
		fTabZoom->SetLayoutManager(new TGHorizontalLayout(fTabZoom));
		fToolBarVZ = new GVToolBarVZ(fTabZoom);
		fZoomCenter = new TGCompositeFrame(fTabZoom);

		fVsliderZ = new TGDoubleVSlider(fTabZoom, 585);
		fHsliderZ = new TGDoubleHSlider(fZoomCenter, 735);
		fVsliderZ ->SetRange(0, 100);
		fHsliderZ ->SetRange(0, 100);
		SetZPositionReset();
		fTabZooms = new GVTab(fZoomCenter, fMainBaseFrame_this);
		fTabZooms->AddTab("zoom1", 1, 1);
		fTabZooms->AddTab("zoom2", 1, 1);
		fTabZooms->AddTab("zoom3", 1, 2);
		fTabZoom->AddFrame(fToolBarVZ, new TGLayoutHints(kLHintsExpandY));
		fTabZoom->AddFrame(fVsliderZ, new TGLayoutHints(kLHintsExpandY));
		fTabZoom->AddFrame(fZoomCenter, new TGLayoutHints(kLHintsRight
				| kLHintsExpandX | kLHintsExpandY));

		fZoomCenter->AddFrame(fTabZooms, new TGLayoutHints(kLHintsExpandX
				| kLHintsExpandY));
		fZoomCenter->AddFrame(fHsliderZ, new TGLayoutHints(kLHintsExpandX));
		fVsliderZ->Connect("Released()", "Vigru", fMainBaseFrame_this,
				"SetVZPosition()");
		fHsliderZ->Connect("Released()", "Vigru", fMainBaseFrame_this,
				"SetHZPosition()");
	}

	//-------------------------------TAB CONSOL------------------------------------
	if (fTab) {
		fTabConsol = fTab->AddTab("Consol");
		fConsol = new GVConsol(fTabConsol, (char*) "Vigru.log", true);
		fTabConsol->AddFrame(fConsol, new TGLayoutHints(kLHintsExpandX
				| kLHintsExpandY));
	}
	gSystem->Exec("rm -f log");//delete old log file
	//gSystem->RedirectOutput("log","a");
	//----------------------------------------------------------------------------

	//STATUSBAR
	fStatusBar = new TGStatusBar(fMainBaseFrame_this, 1000, 50);
	fStatusBar->SetForegroundColor(0x000000);
	fDisplayInfos = false;
	//TOOLTIP
	fTip = NULL;

	//PrintDialog
	fPrintDialog = NULL;

	if (fTab)
		fMainWindows->AddFrame(fTab, new TGLayoutHints(kLHintsExpandX
				| kLHintsExpandY));
	if (fMenuBar)
		AddFrame(fMenuBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

	AddFrame(fToolBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
	fLh = new TGHorizontal3DLine(fMainBaseFrame_this);
	AddFrame(fLh, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
	AddFrame(fMainWindows, new TGLayoutHints(kLHintsTop | kLHintsExpandX
			| kLHintsExpandY));

	AddFrame(fStatusBar, new TGLayoutHints(kLHintsBottom | kLHintsExpandX));
	//	cout << "  ****************Layout***************************\n";

	Layout();
	MapSubwindows();
	MapWindow();

	//---Timer---
	fTimer = new TTimer();
	fTimer->Connect("Timeout()", "Vigru", fMainBaseFrame_this, "RefreshPage()");

	fTimerZoom = new TTimer();
	fTimerZoom->Connect("Timeout()", "Vigru", fMainBaseFrame_this,
			"RefreshPage()");

	if (fTabPage)
		fTabPage->CdFirstOrLastPad();

	if (fTab)
		fTab->Connect("Selected(Int_t)", "Vigru", fMainBaseFrame_this,
				"SetPage(Int_t)");

	if (fTabPage)
		SetPage(0); //

	if (config_file != "")
		RestoreConfiguration(config_file, false);

	if (fSpecificMode == 1)
		Demo();
	if (fSpecificMode == 2) {
		if (soapserver != "" && soapport != 0) {
			Oscillo(soapserver, soapport);
			soapserver = "";
			soapport = 0;
		}
	}

	if (rootfile != "") {
		GetSpeManager()->GetServerDB()->AddServerIdentity(rootfile, "GRU",
				"FILE", 0);
	}

	if (rootserver != "" && rootport != 0) {
		GetSpeManager()->GetServerDB()->AddServerIdentity(rootserver, "GRU",
				"NET", rootport);
	}
	if (soapserver != "" && soapport != 0) {
		GetSpeManager()->GetServerDB()->AddServerIdentity(soapserver, "SOAP",
				"NET", soapport);
	}

	//GetClient()->WaitFor(fMainBaseFrame_this);
}
//______________________________________________________________________________
//destructor
Vigru::~Vigru() {

	
	if (fMenuBar) {
		//delete fMenuBar; le delete de ceci est en conflit avec Cleanup()
		//	fMenuBar= NULL;
	}
	if (fTimer) {
		delete fTimer;
		fTimer = NULL;
	}
	if (fTimerZoom) {
		delete fTimerZoom;
		fTimerZoom = NULL;
	}
	if (fSpeManager) {
		delete fSpeManager;
		fSpeManager = NULL;
	}
	if (fDemo) {
		delete fDemo;
		fDemo = NULL;
	}
	Cleanup();
}
//______________________________________________________________________________

Bool_t Vigru::TestObject(TObject* obj) {
	return ((obj->InheritsFrom(TH1::Class())) || (obj->InheritsFrom(
			TGraph::Class())) || (obj->InheritsFrom(TGraph2D::Class())));
}
//______________________________________________________________________________

void Vigru::CloseWindow() {
	//close the application
	
	UnmapWindow();
	DeleteWindow();//  launch a delete but after a short time like a thread.
	gApplication->SetReturnFromRun(fTerminate);
	gApplication->Terminate(fTerminate);
}

//______________________________________________________________________________
GVTab *Vigru::GetSpecOrZoomTab() {

	int level = fTab->GetCurrent();
	if (level == 0) {
		return fTabPage;
	}
	if (level == 1) {
		return fTabZooms;
	}
	return NULL;
}
//______________________________________________________________________________
void Vigru::ChangeTabZoom(Int_t tab) {
	//Display the tab which is in parameter
	fTab->SetTab(1);
	fTabZooms->SetTab(tab);
}
//______________________________________________________________________________
void Vigru::ChangeTabPage(Int_t tab) {
	fTab->SetTab(0);
	fTabPage->SetTab(tab);
}
//______________________________________________________________________________
void Vigru::SetPageChange(TString pageName, Int_t nbPadX, Int_t nbPadY) {
	fTabPage->SetPageChange(pageName, nbPadX, nbPadY);

}
//______________________________________________________________________________
void Vigru::SetVZPosition() {
	Int_t ymin, ymax, min, max;
	min = (Int_t) fVsliderZ ->GetMinPosition();
	max = (Int_t) fVsliderZ ->GetMaxPosition();
	ymax = 100 - min;
	ymin = 100 - max;
	fTabZooms->ApplyZoomProportional(-1, -1, ymin, ymax);
}
//______________________________________________________________________________
void Vigru::SetHZPosition() {
	Int_t xmin, xmax;
	xmin = (Int_t) fHsliderZ ->GetMinPosition();
	xmax = (Int_t) fHsliderZ ->GetMaxPosition();
	fTabZooms->ApplyZoomProportional(xmin, xmax, -1, -1);
}
//______________________________________________________________________________
void Vigru::SetZPositionReset() {
	fVsliderZ ->SetPosition(0, 100);
	fHsliderZ ->SetPosition(0, 100);
}
//______________________________________________________________________________
void Vigru::SetSPositionReset() {
	fVsliderS ->SetPosition(0, 100);
	fHsliderS ->SetPosition(0, 100);
}

//______________________________________________________________________________
void Vigru::SetVSPosition() {
	Int_t min, max, ymin, ymax;
	min = (Int_t) fVsliderS ->GetMinPosition();
	max = (Int_t) fVsliderS ->GetMaxPosition();
	ymax = 100 - min;
	ymin = 100 - max;

	fTabPage->ApplyZoomProportional(-1, -1, ymin, ymax);
}
//______________________________________________________________________________
void Vigru::SetHSPosition() {
	Int_t xmin, xmax;
	xmin = (Int_t) fHsliderS ->GetMinPosition();
	xmax = (Int_t) fHsliderS ->GetMaxPosition();
	fTabPage->ApplyZoomProportional(xmin, xmax, -1, -1);
}
//______________________________________________________________________________
void Vigru::ApplyZoomProportionalAllPages() {
	// Apply Zoom get on X and Y sliders on ProportionalAllPages

	int originalTab = 0;
	int originalSpeOrZoom = fTab->GetCurrent();
	GVTab * orginalGVTab;
	orginalGVTab = GetSpecOrZoomTab();
	originalTab = orginalGVTab ->GetCurrent();
	GVPad* originalPad = (GVPad*) GetSelectedPad();

	fTab->SetTab(0);

	Int_t xmin, xmax;
	xmin = (Int_t) fHsliderS ->GetMinPosition();
	xmax = (Int_t) fHsliderS ->GetMaxPosition();
	Int_t ymin, ymax;
	ymin = (Int_t) fHsliderZ ->GetMinPosition();
	ymax = (Int_t) fHsliderZ ->GetMaxPosition();
	fTabPage->ApplyZoomProportionalAllPages(xmin, xmax, ymin, ymax);

	fTab->SetTab(originalSpeOrZoom);
	orginalGVTab->SetTab(originalTab);
	originalPad->cd();
	originalPad->Update();

}

//______________________________________________________________________________
void Vigru::AddPage(TString pageName, Int_t nbPadX, Int_t nbPadY) {
	//	Add a new tab--
	TString pageNameLocal;
	Int_t limit_tab;
	limit_tab = 512;
	Int_t nbTab;

	TreatName(&pageName);

	nbTab = fTabPage->GetNumberOfTabs();

	if ((nbPadY < 0) || (nbPadX < 0)) {
		//fError.TreatError(1, 0, " No tab add because bad cancel.");
		return;
	}

	if ((nbPadY == 0) || (nbPadX == 0)) {
		fError.TreatError(1, 0, " No tab add because bad parameter.");
		return;
	}

	if (nbTab > limit_tab) {
		TString tempo;
		tempo.Form("Limit of %d  tabs reached !", limit_tab);
		fError.TreatError(1, 0, tempo);
		return;
	}

	if (pageName.CompareTo("") == 0) {
		pageNameLocal.Form("Tab_%d", nbTab);
	} else {
		pageNameLocal.Form("%s", pageName.Data());
	}

	fTabPage->SetTab(nbTab);
	fTabPage->AddTab(pageNameLocal.Data(), nbPadX, nbPadY);

	Layout();
	MapSubwindows();
	MapWindow();

}

//______________________________________________________________________________
TPad* Vigru::GetSelectedPad() {
	//return the current seletected pad
	TPad *Selected_pad = NULL;
	Selected_pad = (TPad *) gROOT->GetSelectedPad();
	if (!(Selected_pad->InheritsFrom("TPad"))) {
		int retval = 0;
		EMsgBoxIcon icontype = kMBIconStop;
		new TGMsgBox(gClient->GetRoot(), fMainBaseFrame_this, "Warning",
				"No seleted Pad", icontype, 1, &retval);
	}
	return Selected_pad;
}
//______________________________________________________________________________
void Vigru::Zoom(Int_t tab, Int_t pad) {

	GVPad * destinationPad;
	GVPad* originPad = (GVPad*) GetSelectedPad();
	GSpectraDB* DBlocal = originPad->GetPadDB();
	destinationPad = ((GVPad*) (fTabZooms->GetPad(tab, pad)));
	destinationPad->SetOption2D(originPad->GetOption2D());
	destinationPad->SetStatOpt(originPad->GetStatOpt(), false);
	destinationPad->SetZP(originPad->GetNumber());
	destinationPad->SetZT(fTabPage->GetCurrent());
	ChangeTabZoom(tab);
	destinationPad->cd();
	destinationPad->DisplaySpectra(DBlocal, false);
}

//______________________________________________________________________________
void Vigru::ZoomBack() {
	//Draw the modified spectrum from Zoom TAB to Spectra TAB

	GVPad * destinationPad;
	GVPad* zoomedPad = (GVPad*) GetSelectedPad();
	Int_t toPad, toTab;
	if (zoomedPad->GetNumber() == 0) {
		fError.TreatError(1, 0, "Impossible to zoom back back ground pad");
		return;
	}
	GSpectraDB *DBzoom = zoomedPad->GetPadDB();

	toTab = zoomedPad->GetZT();
	toPad = zoomedPad->GetZP();

	destinationPad = (GVPad*) fTabPage->GetPad(toTab, toPad);
	destinationPad->SetOption2D(zoomedPad->GetOption2D());
	destinationPad->SetStatOpt(zoomedPad->GetStatOpt());
	ChangeTabPage(toTab);
	destinationPad->cd();
	destinationPad->DisplaySpectra(DBzoom, false);

}

//______________________________________________________________________________

GSpectrumIdentity* Vigru::GetCurrentSpeId(bool quiet) {
	//return the Spectrum id (no i) of selected pad
	GVPad *pad = (GVPad *) GetSelectedPad();
	GSpectraDB* DB = pad->GetPadDB();
	GSpectrumIdentity * id;
	id = NULL;
	Int_t i;
	if (DB->GetLast() == 0) {
		i = 0;
		id = (GSpectrumIdentity*) (DB->At(i));
	} else {
		if ((DB->GetLast() > 0) and (!quiet))
			fError.TreatError(2, 0, "Too Much Spectrum!");
		if ((DB->GetLast() < 0) and (!quiet))
			fError.TreatError(2, 0, "No Spectrum!");
	}

	return id;
}

//______________________________________________________________________________
void Vigru::SaveConfiguration(TString FileName, bool force) {
	// Save configuration of vigru
	//FileName : file name

	FileName.ReplaceAll(".xml", "");
	FileName.ReplaceAll(".XML", "");
	TString strXML = FileName;
	strXML += ".xml";
	TString strROOT = FileName;
	strROOT += ".root";
	CreateXMLFile(strXML, strROOT, force);

}
//______________________________________________________________________________
void Vigru::SaveConfigurationPage(TString FileName, bool force) {
	// Save configuration of only a page of vigru
	//FileName : file name

	FileName.ReplaceAll(".xml", "");
	FileName.ReplaceAll(".XML", "");
	TString strXML = FileName;
	strXML += ".xml";
	TString strROOT = FileName;
	strROOT += ".root";
	CreateXMLPageFile(strXML, strROOT, force);

}
//______________________________________________________________________________
void Vigru::CreateXMLFile(TString xmlFile, TString Rootfile, bool force) {

	//Create an XML file containing configuration carateristics


	TString tempo;
	struct stat FileStat;

	//test if file  already exists
	if (stat(xmlFile.Data(), &FileStat) != 0) {
		if (!force) {
			fError.TreatError(1, 0,
					"XML file already exists, so no new created");
		} else {
			gSystem->Unlink(xmlFile.Data());
			fError.TreatError(1, 0, "old XML file deleted");
		}
	}

	// First create engine
	TXMLEngine* xml = new TXMLEngine();
	// Create main node of document tree
	XMLNodePointer_t mainnode = xml->NewChild(0, 0, "root"); //root= root name given in xmlfile

	// save associate Rootfile if it exists
	xml->NewChild(mainnode, 0, "RootAssociateFile", Rootfile);

	// save Configuration of Servers
	fSpeManager->GetServerDB()->CreateXML(xml, mainnode, true);

	// save Configuration of Tabs
	fTabPage->CreateXML(xml, mainnode);

	// save Configuration of spectra
	//fSpeManager->GetDB()->CreateXML(xml, mainnode, false);

	XMLDocPointer_t xmldoc = xml->NewDoc();
	xml->DocSetRootElement(xmldoc, mainnode);
	// Save document to file, 2 is step in text struicture


	xml->SaveDoc(xmldoc, xmlFile.Data(), 2);

	// Release memory before exit
	xml->FreeDoc(xmldoc);
	delete xml;

}
//______________________________________________________________________________
void Vigru::CreateXMLPageFile(TString xmlFile, TString Rootfile, bool force) {

	//Create an XML file containing configuration carateristics
	// only for one page

	TString tempo;
	struct stat FileStat;

	//test if file  already exists
	if (stat(xmlFile.Data(), &FileStat) != 0) {
		if (!force) {
			fError.TreatError(1, 0,
					"XML file already exists, so no new created");
		} else {
			gSystem->Unlink(xmlFile.Data());
			fError.TreatError(1, 0, "old XML file deleted");
		}
	}

	// First create engine
	TXMLEngine* xml = new TXMLEngine();
	// Create main node of document tree
	XMLNodePointer_t mainnode = xml->NewChild(0, 0, "root"); //root= root name given in xmlfile

	// save associate Rootfile if it exists
	xml->NewChild(mainnode, 0, "RootAssociateFile", Rootfile);

	// save Configuration of current Tab
	fTabPage->CreateXML(xml, mainnode, true);

	XMLDocPointer_t xmldoc = xml->NewDoc();
	xml->DocSetRootElement(xmldoc, mainnode);
	// Save document to file, 2 is step in text struicture


	xml->SaveDoc(xmldoc, xmlFile.Data(), 2);

	// Release memory before exit
	xml->FreeDoc(xmldoc);
	delete xml;

}
//______________________________________________________________________________

void Vigru::RestoreConfiguration(TString fileXML, bool question) {
	//Restore a configuration
	// lecture du fichier configuration xml
	if (IsFileExiste(fileXML) == false)
		return;
	TString fileRoot = "";
	TXMLEngine* xml = new TXMLEngine();

	Int_t NumberOfTabs;
	TString tempo, nodename, nodecontent;
	NumberOfTabs = 0;

	XMLDocPointer_t xmldoc = xml->ParseFile(fileXML.Data());
	if (xmldoc == 0) {
		fError.TreatError(2, -1, " Read of XmlFile:", fileXML.Data());
		delete xml;
		return;
	}

	tempo.Form("RestoreConfiguration fichier xml: %s", fileXML.Data());
	fError.Infos(tempo.Data());
	// take access to main node

	XMLNodePointer_t mainnode = xml->DocGetRootElement(xmldoc);
	XMLNodePointer_t basenode = xml->GetChild(mainnode);

	while (basenode) {

		nodename = xml->GetNodeName(basenode);
		nodecontent = xml->GetNodeContent(basenode);

		// associeted file
		if (nodename == "RootAssociateFile") {
			fileRoot = nodecontent;
			tempo.Form("Root Associate file %s", fileRoot.Data());
			fError.Infos(tempo.Data());
		}

		// log file
		if (nodename == "LogFile") {
			fConsol->ReadXML(xml, basenode);
		}

		// remplissage de la data base des reference des spectres
		if (nodename == "ListIdSpectra") {
			fSpeManager->GetDB()->Clear();
			fSpeManager->GetDB()-> ReadXML(xml, basenode, true);
		}

		// Read config page
		if (nodename == "Pages") {
			NumberOfTabs = nodecontent.Atoi();
			fTabPage->ReadXML(xml, basenode, question);
		}
		if (NumberOfTabs < -1)
			fError.TreatError(2, 0, "negative NumberOfTabs !");
		// replissage de la data base server
		if (nodename == "ListIdServers") {
			fSpeManager->GetServerDB()->Clear();
			fSpeManager->GetServerDB()-> ReadXML(xml, basenode, true);
		}
		basenode = xml->GetNext(basenode);
	}
	xml->FreeDoc(xmldoc);
	delete xml;

	fTabPage->GetPad(0, 1);
	GetSpeManager()->UpdateSpectraList();
	// application de la database au display/
	fTabPage->GetPad(0, 1);
	if (fileRoot != "") {
		if (ReadSpectra(fileRoot)) {
			gROOT->cd();

		}
	}
}
//______________________________________________________________________________
void Vigru::RestoreConfigurationPage(TString fileXML) {
	//Restore a configuration
	// lecture du fichier configuration xml
	// for only one page


	if (IsFileExiste(fileXML, true) == false)
		return;
	TString fileRoot = "";
	TXMLEngine* xml = new TXMLEngine();

	Int_t NumberOfTabs;
	TString tempo, nodename, nodecontent;
	NumberOfTabs = 0;

	XMLDocPointer_t xmldoc = xml->ParseFile(fileXML.Data());
	if (xmldoc == 0) {
		fError.TreatError(2, -1, " Read of XmlFile:", fileXML.Data());
		delete xml;
		return;
	}

	tempo.Form("RestoreConfigurationPage fichier xml: %s", fileXML.Data());
	fError.TreatError(0, 0, tempo.Data());
	// take access to main node

	XMLNodePointer_t mainnode = xml->DocGetRootElement(xmldoc);
	XMLNodePointer_t basenode = xml->GetChild(mainnode);

	while (basenode) {

		nodename = xml->GetNodeName(basenode);
		nodecontent = xml->GetNodeContent(basenode);

		// Read config page
		if (nodename == "Pages") {
			NumberOfTabs = nodecontent.Atoi();
			fTabPage->ReadXML(xml, basenode, true);
		}
		if (NumberOfTabs < -1)
			fError.TreatError(2, 0, "negative NumberOfTabs !");
		basenode = xml->GetNext(basenode);
	}
	xml->FreeDoc(xmldoc);
	delete xml;

	// application de la database au display/

	if (fileRoot != "") {
		if (ReadSpectra(fileRoot))
			RefreshAllPages(false);
	}

}

//______________________________________________________________________________
void Vigru::DeleteOldConf() {
	//delete all tabs->Current configuration
	for (int i = fTabPage->GetNumberOfTabs() - 1; i > 0; i--) {
		fTabPage->RemoveTab(i);
	}
	fTabPage->GetCanvasAt(0)->Clear();
}
//______________________________________________________________________________
void Vigru::SetStateTimer(Long_t time) {
	if (time == 0)
		fTimer->TurnOff();
	else {
		fTimer->SetTime(time);
		fTimer->TurnOn();
	}
}

//______________________________________________________________________________

void Vigru::SetStateTimerZoom(Long_t time) {
	//set the sh zoom interval
	if (time == 0)
		fTimerZoom->TurnOff();
	else {
		fTimerZoom->SetTime(time);
		fTimerZoom->TurnOn();
	}
}
//______________________________________________________________________________

void Vigru::RefreshAllPages(bool getnewspe) {
	//Refresh all pads of current page
	int originalTab = 0;
	int originalSpeOrZoom = fTab->GetCurrent();
	GVTab * orginalGVTab;
	orginalGVTab = GetSpecOrZoomTab();
	originalTab = orginalGVTab ->GetCurrent();
	GVPad* originalPad = (GVPad*) GetSelectedPad();

	fTab->SetTab(0);
	fTabPage->SetTab(0);

	for (int i = 0; i < fTabPage->GetNumberOfTabs(); i++) {
		ChangeTabPage(i);
		RefreshPage(false, getnewspe);
	}

	fTab->SetTab(originalSpeOrZoom);
	orginalGVTab->SetTab(originalTab);
	originalPad->cd();
	originalPad->Update();
	if ((GetSpeManager()-> GetListDevice()-> GetNetClientRoot()) != NULL) {
		((GNetClientRoot*) (GetSpeManager()->GetListDevice()->GetNetClientRoot()))->EndOfPage();
		GetSpeManager()-> GetListDevice()-> GetNetClientRoot()->Close();
	}
}
//______________________________________________________________________________

void Vigru::RefreshPage(bool with_return, bool getnewspe) {
	//Refresh all pads of current page
	GVTab * Tabs = GetSpecOrZoomTab();
	//Pause with time out
	if (Tabs == NULL)
		return;
	if (fAcqNumexo != NULL)
		OscilloRun();
	Tabs->RefreshTab(with_return, getnewspe);

	if ((GetSpeManager()-> GetListDevice()-> GetNetClientRoot()) != NULL) {
		((GNetClientRoot*) (GetSpeManager()->GetListDevice()->GetNetClientRoot()))->EndOfPage();
		GetSpeManager()-> GetListDevice()-> GetNetClientRoot()->Close();
	}
	if ((GetSpeManager()-> GetListDevice()-> GetNetClientSoap()) != NULL) {
			((GNetClientRoot*) (GetSpeManager()->GetListDevice()->GetNetClientSoap()))->EndOfPage();
		}

}

//______________________________________________________________________________

void Vigru::SendEndOfPage(GSpectrumIdentity* last_id_server) {

}
//______________________________________________________________________________

void Vigru::DisplayInfos(Int_t event, Int_t px, Int_t py, TObject* obj) {
	//Display infos in a tooltip
	if (fDisplayInfos == false)
		return;
	if (fTip != NULL) {
		delete fTip;
		fTip = 0;
	}

	GSpectrumIdentity *myId = GetCurrentSpeId(true);
	if (myId) {
		const char *objectInfo = obj->GetObjectInfo(px, py);
		TString tipText = " ";
		if (TestObject(obj)) {

			tipText += " <";
			tipText += obj->GetName();
			tipText += "> ";
			tipText += " : ";
			tipText += objectInfo;
			/*
			 TBox *b = new TBox(px - 10, py - 10, px + 10, py + 10);
			 fTip = new TGToolTip(fClient->GetDefaultRoot(), b, tipText, 50);
			 fTip->Show(px + 12, py + 12);
			 delete b;
			 */

			fStatusBar->SetText(tipText);
		}
	}
}

//______________________________________________________________________________
//display print dialog
void Vigru::DisplayPrintDialog() {
	fPrintDialog = new GVPrintDialog(gClient->GetRoot(), fMainBaseFrame_this,
			200, 280);
}
//______________________________________________________________________________

//print method
void Vigru::Print(TString printerName, Bool_t page, Bool_t printer) {
	TString psfilename = "vigru_tempo.ps";
	TString action;
	GVTab * Tabs;

	if (!page) {

		TPad * pad;
		pad = GetSelectedPad();
		if (pad)
			pad ->Print(psfilename);
		else {
			fError.TreatError(1, 0, "Print ! Inexistant canvas!");
			return;
		}
	} else {
		Tabs = GetSpecOrZoomTab();

		if (Tabs) {

			Int_t current = Tabs->GetCurrent();

			TCanvas *c1 = Tabs->GetCanvasAt(current);
			if (c1) {
				if (c1->InheritsFrom("TPad")) {
					c1->Print(psfilename);
				} else {
					fError.TreatError(1, 0, "Print ! Inexistant canvas!");
					return;
				}
			} else {
				fError.TreatError(1, 0, "Print ! Inexistant canvas!");
				return;
			}
		} else {
			fError.TreatError(1, 0, "Print ! Inexistant canvas!");
			return;
		}
	}

	if (printer) {
		fDefaultPrinterName = printerName;
		action = "lpr -P" + printerName + " " + psfilename;

	} else {
		action = "mv " + psfilename + " " + printerName;
	}
	fError.TreatError(0, 0, action.Data());
	gSystem->Exec(action.Data());//execute a shell command

}
//______________________________________________________________________________
void Vigru::RefreshPad() {
	//Refresh selected pad
	GVPad* gvpad = (GVPad*) GetSelectedPad();
	Int_t pad = gvpad->GetNumber();
	if (pad == 0)
		fError.TreatError(1, 0, "Pad 0 selected");
	gvpad->RefreshPad();

	if ((GetSpeManager()-> GetListDevice()-> GetNetClientRoot()) != NULL) {
		((GNetClientRoot*) (GetSpeManager()->GetListDevice()->GetNetClientRoot()))->EndOfPage();
		GetSpeManager()-> GetListDevice()-> GetNetClientRoot()->Close();
	}

}
//______________________________________________________________________________
void Vigru::ResetSpectrum(bool question) {
	GVPad* gvpad = (GVPad*) GetSelectedPad();
	gvpad-> ResetSpectra(question);
}
//______________________________________________________________________________
void Vigru::ResetSpectra(bool force) {

	TString pageName;
	pageName.Form("Reset Options");
	if (force == true or (GetResetOption())->Contains("n")) {
		new GVSetReset(gClient->GetRoot(), fMainBaseFrame_this, 190, 230,
				(char*) "Set Reset", GetResetOption());
	}

	if (GetResetOption()->Contains("c")) {
		GetResetOption()->Remove(TString::kBoth, 'c');

		return;
	}
	if (GetResetOption()->Contains("h"))
		ResetSpectrum(false);
	if (GetResetOption()->Contains("p"))
		ApplyPageReset(false);
	if (GetResetOption()->Contains("s"))
		AllServerSpectraReset(false);
}
//______________________________________________________________________________
void Vigru::DisplaySpectrum(Int_t ind, Int_t operation) {
	//Display one  spectrum from DB  selected by its index
	// operation 0=replace 2=same 1= same&getnewone 3=add 4=substration
	if (ind >= 0) {
		GSpectrumIdentity * identity;
		GSpectraDB* DB = GetSpeManager()->GetDB();
		identity = (GSpectrumIdentity*) DB->At(ind);
		GVPad* gvpad = (GVPad*) GetSelectedPad();
		Int_t pad = gvpad->GetNumber();
		if (pad == 0) {
			fError.TreatError(1, 0, "Vigru::DisplaySpectrum :Pad 0 selected");
			return;
		}
		gvpad->DisplaySpectrum(identity, operation);
	}
}

//______________________________________________________________________________
void Vigru::DisplayCuts() {
	Vigru *f = (Vigru*) fMainBaseFrame_this;
	GVSpectraChooser * Chooser;
	Int_t oneselected;
	oneselected = 1;
	Int_t operation;
	operation = 1;//  display in same pad and get a new one

	if (f->GetSpeManager()->GetDB()->GetLast() >= 0) {
		TString entete = "Display Spectra";
		Chooser = new GVSpectraChooser(gClient->GetRoot(), GetMainFrame(), 450,
				620, entete, 1, &oneselected, &operation, 2);
		if (!Chooser)
			return;

		Chooser->Init(f->GetSpeManager()->GetDB());

		if (oneselected >= 0) {
			GVPad* gvpad = (GVPad*) GetSelectedPad();
			Int_t pad = gvpad->GetNumber();
			if (pad == 0) {
				fError.TreatError(1, 0,
						"Vigru::DisplaySpectrum :Pad 0 selected");
				return;
			}

			//gvpad->GetPadDB()->RemoveAllCuts();

			DisplaySpectrum(oneselected, operation);
		}
	} else {
		MessageBoxNoHisto();
	}
}
//______________________________________________________________________________
void Vigru::ClearCuts() {
	GVPad* gvpad = (GVPad*) GetSelectedPad();
	gvpad->GetPadDB()->RemoveAllCuts();
}
//______________________________________________________________________________
void Vigru::DefineCuts() {

	gROOT->cd();
	MessageBoxWarning((char*) "Define a New Cut (contour) in SelectedPad");
	gROOT->SetEditorMode("CutG");

}

//______________________________________________________________________________
void Vigru::LoadPadCuts() {
	GVPad* gvpad = (GVPad*) GetSelectedPad();
	LoadSpectra(3, gvpad->GetPadDB());
	gvpad->RefreshPad(false);
}
//______________________________________________________________________________
void Vigru::SendPadCuts() {
	// send all cuts from a pad to server.
	// it can be use full to refresh cut to server when we have move cut on a pad
	gROOT->cd();

	TString tempos;
	GVPad* gvpad = (GVPad*) GetSelectedPad();

	GSpectrumIdentity * id;
	GSpectraDB* DB = NULL;
	TNamed* ClonedCut;
	if (gvpad)
		DB = gvpad->GetPadDB();

	if (DB == NULL) {
		fError.TreatError(1, 0, "No Pad Database");
		return;
	}

	if (DB -> GetLast() < 0) {
		tempos = "No Cut (contour) to send";
		fError.TreatError(1, 0, tempos);
		return;
	}

	for (Int_t i = 0; i <= DB->GetLast(); i++) {
		id = (GSpectrumIdentity*) DB->At(i);

		if (id ->GetTypeSpe() == 3) {
			if (id->GetSpectrum() != NULL) {
				id->SetSource("NET");
				ClonedCut = (TNamed*) (id->GetSpectrum()->Clone());
				if (ClonedCut == NULL) {
					fError.TreatError(1, 0, "SendPadCuts : Cut not cloned ");
				}
				Int_t oneselected = GetSpeManager()->GetDB()->GetIndexByName(
						id->GetSpectrumName(), id->GetFamily());
				if (oneselected >= 0) {
					ChangeSpectrum(oneselected, (TNamed*) ClonedCut);
				}
				//if (changesource) id->SetSource("FILE");
				else {
					fError.TreatError(1, 0,
							"Data base seemed empty, did you refresh list?");
				}
				if (ClonedCut != NULL)
					delete (ClonedCut);
			}
		}
	}
}
//______________________________________________________________________________
void Vigru::SendCuts() {
	// send a cut from a selected pad to server

	gROOT->cd();
	GVSpectraChooser * Chooser;
	Vigru *f = (Vigru*) fMainBaseFrame_this;
	TString tempos;
	const char *defaultnamecut = "CUTG";
	GVPad* gvpad = (GVPad*) GetSelectedPad();
	fCurrentCut = gvpad->GetNewCutFromPad(defaultnamecut);

	if (fCurrentCut) {
		Int_t oneselected;
		oneselected = 1;
		if (GetSpeManager()->GetDB()->GetLast() >= 0) {
			TString entete = "Choose a Cut";
			Chooser = new GVSpectraChooser(gClient->GetRoot(), GetMainFrame(),
					450, 620, entete, 1, &oneselected, NULL, 2);
			if (!Chooser)
				return;
			Chooser->Init(GetSpeManager()->GetDB());
			if (oneselected >= 0) {
				TObject* ClonedCut;
				ClonedCut = fCurrentCut->Clone();

				if (ClonedCut == NULL) {

					fError.TreatError(1, 0, "SendCuts : Cut not cloned ");
				}
				ChangeSpectrum(oneselected, (TNamed*) ClonedCut);
			}
			DisplaySpectrum(oneselected, 1);
		} else {
			tempos = "No Cut PreDefined (contour)";
			f->MessageBoxWarning((char*) (tempos.Data()));
			fError.TreatError(1, 0, tempos);
		}

	} else {
		tempos.Form("No new cut found with default name %s", defaultnamecut);
		MessageBoxWarning((char*) (tempos.Data()));
	}
}//______________________________________________________________________________
void Vigru::IntegralsInCuts() {
	GVPad* gvpad = (GVPad*) GetSelectedPad();
	gvpad-> IntegralsInCuts();
}
//______________________________________________________________________________
void Vigru::ChangeSpectrum(Int_t index, TNamed* spectrum) {
	//Change spectrum instance selected by its index
	//and apply change on server

	GetSpeManager()->GetDB()->ChangeSpectrum(index, spectrum);
	GetSpeManager()->GetDB()->ChangeSpectrumOnServer(index);
}
//______________________________________________________________________________
void Vigru::ApplyStatistics(TString statopt, bool page) {
	//display statistics on several spectra

	if (!page) {
		GVPad * gvpad;
		gvpad = (GVPad*) GetSelectedPad();
		gvpad->SetStatOpt(statopt);
	} else {
		GVTab * Tabs;

		int level = fTab->GetCurrent();
		if (level == 0) {
			Tabs = fTabPage;
		} else {
			Tabs = fTabZooms;
		}

		Tabs->SetStatOpt(statopt);
	}
}
//______________________________________________________________________________
void Vigru::DisplaySpectra(Int_t operation) {
	//display several spectra from DB
	// check "action" entry to get list of specta to diplay

	Int_t nbpage;
	nbpage = fTabPage->GetNumberOfTabs();
	if (nbpage < 0)
		fError.TreatError(2, 0, " DisplaySpectra negative nbpage ");
	Int_t MemoButton;
	fTabPage->SetAutoMode(kTRUE);
	Int_t nbSpectras = 0;
	GSpectraDB * DB = GetSpeManager()->GetDB();
	//	if (operation==0) fTabPage->SetTab(nbpage - 1);
	Int_t nbPadsX, nbPadsY, nbPadMax = 6;
	nbPadsX = nbPadMax;
	nbPadsY = nbPadMax;
	TCanvas* canvas;
	Int_t retval;
	TVirtualPad* lastpad;
	lastpad = NULL;
	Int_t current_tab = fTabPage->GetCurrent();
	GVPad* gvpad = (GVPad*) GetSelectedPad();
	Int_t current_pad = gvpad->GetNumber();

	for (Int_t i = 0; i <= DB->GetLast(); i++) {
		if (((GSpectrumIdentity*) DB->At(i))->GetAction())
			nbSpectras++;
	}

	Int_t indice;
	indice = -1;
	MemoButton = kMBYes;

	while (true) {

		if (nbSpectras == 0) {
			DB->ResetActions();
			return;
		}

		canvas = fTabPage->GetCurrentCanvas();
		TVirtualPad* testpad = canvas->cd(current_pad);

		if (testpad == NULL) {
			DB->ResetActions();
			fError.TreatError(2, 0, "Vigru::DisplaySpectra : no pad available");
			return;
		}
		if ((testpad == lastpad) && (operation == 0)) { // if testpad = testpad then whe did avanced so we have to change page

			bool test = fTabPage->SetTab(current_tab + 1);
			if (!test) {

				fTabPage->ComputePadXY(nbSpectras % (nbPadsX * nbPadsY),
						&nbPadsX, &nbPadsY);
				fTabPage->AddTab("", nbPadsX, nbPadsY);
				current_pad = 1;
				canvas = fTabPage->GetCurrentCanvas();
				canvas->cd(current_pad);
				canvas->Update();
			}
			if (canvas == NULL) {
				DB->ResetActions();
				fError.TreatError(2, 0,
						"Vigru::DisplaySpectra : no pad available");
				return;
			}
		}
		canvas->Update();
		lastpad = testpad;
		GVPad* gvpad = (GVPad*) GetSelectedPad();

		if (gvpad->GetPadDB()->GetLast() > 0) {

			if ((MemoButton = !kMBYesAll) && (MemoButton = !kMBNoAll)
					&& (operation == 0)) {
				EMsgBoxIcon icontype = kMBIconQuestion;
				Int_t button;
				button = kMBYes || kMBNo || kMBCancel || kMBYesAll || kMBNoAll;
				new TGMsgBox(
						gClient->GetRoot(),
						fMainBaseFrame_this,
						"Warning",
						"Do you really want to replace spectrum in selected pad?",
						icontype, button, &retval);
				MemoButton = retval;
			}
		}

		if ((MemoButton == kMBYes) || (MemoButton == kMBYesAll) || (operation
				!= 0)) {
			while (true) {

				indice++;
				GSpectrumIdentity* identity = (GSpectrumIdentity*) DB->At(
						indice);
				if (identity->GetAction()) {
					DisplaySpectrum(indice, operation);
					nbSpectras--;
					if (operation == 0)
						current_pad++;

					break;
				}
				if ((indice > DB->GetLast()) || (nbSpectras == 0)) {
					DB->ResetActions();

					return;
				}
			}//end of while (true)

		}

		if ((indice > DB->GetLast()) || (nbSpectras == 0)) {
			DB->ResetActions();

			return;
		}
	} //end of while (true)

	DB->ResetActions();

}
//______________________________________________________________________________
void Vigru::ChangingLog(Bool_t logx, Bool_t logy, Bool_t logz) {
	//log
	GVPad* gvpad = (GVPad*) GetSelectedPad();
	if (gvpad)
		gvpad->ChangingLog(logx, logy, logz);
}
//______________________________________________________________________________
void Vigru::ApplyLog(Bool_t logx, Bool_t logy, Bool_t logz) {
	//log
	GVPad* gvpad = (GVPad*) GetSelectedPad();
	if (gvpad)
		gvpad->ApplyLog(logx, logy, logz);
}
//______________________________________________________________________________
void Vigru::ApplyPageLog(Bool_t logx, Bool_t logy, Bool_t logz) {
	//log an a page

	GVTab* gbtab = GetSpecOrZoomTab();
	if (gbtab)
		gbtab->ApplyPageLog(logx, logy, logz);
}
//______________________________________________________________________________
void Vigru::ApplyAllPagesLog(Bool_t logx, Bool_t logy, Bool_t logz) {
	//apply log on all  pages
	int originalTab = 0;
	int originalSpeOrZoom = fTab->GetCurrent();
	GVTab * orginalGVTab;
	orginalGVTab = GetSpecOrZoomTab();
	originalTab = orginalGVTab ->GetCurrent();
	GVPad* originalPad = (GVPad*) GetSelectedPad();

	fTab->SetTab(0);
	fTabPage->ApplyLogAllPages(logx, logy, logz);

	fTab->SetTab(originalSpeOrZoom);
	orginalGVTab->SetTab(originalTab);
	originalPad->cd();
	originalPad->Update();
}
//______________________________________________________________________________
void Vigru::RemoveLog(Bool_t logx, Bool_t logy, Bool_t logz) {
	//log
	GVPad* gvpad = (GVPad*) GetSelectedPad();
	if (gvpad)
		gvpad->RemoveLog(logx, logy, logz);
}
//______________________________________________________________________________
void Vigru::RemovePageLog(Bool_t logx, Bool_t logy, Bool_t logz) {
	//log an a page

	GVTab* gbtab = GetSpecOrZoomTab();
	if (gbtab)
		gbtab->RemovePageLog(logx, logy, logz);
}
//______________________________________________________________________________
void Vigru::RemoveAllPagesLog(Bool_t logx, Bool_t logy, Bool_t logz) {
	//apply log on all  pages
	int originalTab = 0;
	int originalSpeOrZoom = fTab->GetCurrent();
	GVTab * orginalGVTab;
	orginalGVTab = GetSpecOrZoomTab();
	originalTab = orginalGVTab ->GetCurrent();
	GVPad* originalPad = (GVPad*) GetSelectedPad();

	fTab->SetTab(0);
	fTabPage->SetTab(0);

	for (int i = 0; i < fTabPage->GetNumberOfTabs(); i++) {
		ChangeTabPage(i);
		GVTab* gbtab = GetSpecOrZoomTab();
		if (gbtab)
			gbtab->RemovePageLog(logx, logy, logz);
	}

	fTab->SetTab(originalSpeOrZoom);
	orginalGVTab->SetTab(originalTab);
	originalPad->cd();
	originalPad->Update();
}
//______________________________________________________________________________
void Vigru::PeakFind() {
	//PeakFind
	GVPad* gvpad = (GVPad*) GetSelectedPad();
	if (gvpad)
		gvpad->PeakFind(fNbPeaks, fResolution, fSigma, fThreshold,
				fDisplay_polymarker);
}

//______________________________________________________________________________
void Vigru::DumpPadDataBase() {
	GVPad* gvpad = (GVPad*) GetSelectedPad();

	if (gvpad)
		gvpad->GetPadDB()->MakeDUMP();
}
//______________________________________________________________________________
void Vigru::PeakPageFind() {
	//PeakFind on whole page
	GVTab* gbtab = GetSpecOrZoomTab();
	if (gbtab)
		gbtab->PeakPageFind(fNbPeaks, fResolution, fSigma, fThreshold,
				fDisplay_polymarker);
}
//______________________________________________________________________________
void Vigru::ApplyRangeUserPage(bool inx, bool iny, bool inz, Double_t xmin,
		Double_t xmax, Double_t ymin, Double_t ymax, Double_t zmin,
		Double_t zmax,int padtaborall) {


	if(padtaborall==1){
		GVPad* gvpad = (GVPad*) GetSelectedPad();
		if (gvpad)
		gvpad->ApplySetRangeUser(inx, iny, inz, xmin, xmax, ymin, ymax, zmin,
				zmax);
		};
	if(padtaborall==2){
		GVTab* gbtab = GetSpecOrZoomTab();
		if (gbtab)
		gbtab->ApplySetRangeUser(inx, iny, inz, xmin, xmax, ymin, ymax, zmin,
				zmax);
				}
	if(padtaborall==3){	
		int originalTab = 0;
		int originalSpeOrZoom = fTab->GetCurrent();
		GVTab * orginalGVTab;
		orginalGVTab = GetSpecOrZoomTab();
		originalTab = orginalGVTab ->GetCurrent();
		GVPad* originalPad = (GVPad*) GetSelectedPad();
		fTab->SetTab(0);
		fTabPage->SetTab(0);		
		for (int i = 0; i < fTabPage->GetNumberOfTabs(); i++) {
			ChangeTabPage(i);
			GVTab * Tabs = GetSpecOrZoomTab();
			if (Tabs == NULL) return;
			Tabs->ApplySetRangeUser(inx, iny, inz, xmin, xmax, ymin, ymax, zmin,zmax);
		}
		fTab->SetTab(originalSpeOrZoom);
		orginalGVTab->SetTab(originalTab);
		originalPad->cd();
		}
}
//______________________________________________________________________________
void Vigru::CancelZoom(int padtaborall) {

	if(padtaborall==1){
		GVPad* gvpad = (GVPad*) GetSelectedPad();
		if (gvpad)
		gvpad->CancelZoom();
		};
	if(padtaborall==2){
		GVTab* gbtab = GetSpecOrZoomTab();
		if (gbtab)
		gbtab->CancelZoom();
		}
	if(padtaborall==3){	
		int originalTab = 0;
		int originalSpeOrZoom = fTab->GetCurrent();
		GVTab * orginalGVTab;
		orginalGVTab = GetSpecOrZoomTab();
		originalTab = orginalGVTab ->GetCurrent();
		GVPad* originalPad = (GVPad*) GetSelectedPad();
		fTab->SetTab(0);
		fTabPage->SetTab(0);		
		for (int i = 0; i < fTabPage->GetNumberOfTabs(); i++) {
			ChangeTabPage(i);
			GVTab * Tabs = GetSpecOrZoomTab();
			if (Tabs == NULL) return;
			Tabs->CancelZoom();
		}
		fTab->SetTab(originalSpeOrZoom);
		orginalGVTab->SetTab(originalTab);
		originalPad->cd();

		}
}
//______________________________________________________________________________
void Vigru::ApplyPageReset(bool question) {
	//Reset spectra on whole page
	TString tempos;
	Int_t retval = 1;
	if (question) {
		EMsgBoxIcon icontype = kMBIconQuestion;
		tempos.Form("Do you really want reset all histograms of THIS PAGE");
		new TGMsgBox(gClient->GetRoot(), fMainBaseFrame_this, "Warning",
				tempos, icontype, 3, &retval);
	}
	if (retval == 1) {
		GVTab* gbtab = GetSpecOrZoomTab();
		if (gbtab)
			gbtab->ApplyPageReset(question);
		gbtab->RefreshTab();
	}

}//______________________________________________________________________________
void Vigru::AllServerSpectraReset(bool question) {
	fSpeManager->GetServerDB()->ResetAllSpectraOnServer(fMainBaseFrame_this,
			question);
}

//______________________________________________________________________________

void Vigru::SetPage(Int_t id) {
	if (id == 2) {
		fConsol->Update();
	} else {
	}

	if (id == 0) {
		fTabPage->CdFirstOrLastPad();
	}
	if (id == 1) {
		fTabZooms->CdFirstOrLastPad();
	}
	fToolBar->SetInfo(false);
}
//______________________________________________________________________________

void Vigru::GoBack() {
	GVTab* gbtab = GetSpecOrZoomTab();
	if (gbtab)
		gbtab->MoveToNextPad(-1);
}
//______________________________________________________________________________
void Vigru::GoForward() {
	GVTab* gbtab = GetSpecOrZoomTab();
	if (gbtab)
		gbtab->MoveToNextPad(1);
}
//______________________________________________________________________________
void Vigru::SetInfo(bool info) {
	fToolBar->SetInfo(info);
}

//______________________________________________________________________________
void Vigru::TreatName(TString* name) {
	(*name).Remove(TString::kTrailing, ' ');
	(*name).Remove(TString::kTrailing, '\n');
	(*name).Remove(TString::kTrailing, ' ');
	(*name).Remove(TString::kTrailing, '\t');
	(*name).Remove(TString::kTrailing, ' ');
	(*name).Remove(TString::kTrailing, '\0');
}
//______________________________________________________________________________
void Vigru::FitPanel() {

	GSpectrumIdentity* id = GetCurrentSpeId();
	TPad *pad = GetSelectedPad();
	if (!pad)
		return;
	if (!id)
		return;
	id->FitPanel();
	pad->Update();
}
//______________________________________________________________________________
void Vigru::DrawPanel() {
	GVPad *pad = (GVPad*) GetSelectedPad();
	pad->DrawPanel();
}
//______________________________________________________________________________
void Vigru::DrawPalette() {
	GVPad *pad = (GVPad*) GetSelectedPad();
	pad->DrawPalette();
}
//______________________________________________________________________________
void Vigru::AutoZoom() {
	// do a automatic zoom , removing zeros
	GVPad *pad = (GVPad*) GetSelectedPad();
	pad->AutoZoom();
}
//______________________________________________________________________________
void Vigru::ApplyComputation(Int_t computation, Int_t fromtab, Int_t frompad) {
	// Apply selected process ( process) on current pad from pad (tab,pad)
	GVPad *gvpad = (GVPad*) GetSelectedPad();

	gvpad->ApplyComputation(computation, fromtab, frompad);

}
//______________________________________________________________________________

void Vigru::FFT() {
	// apply fast fourrier transform on all TH of pad
	GVPad *pad = (GVPad*) GetSelectedPad();
	pad->FFT();
}

//______________________________________________________________________________
void Vigru::ConnectInfos(bool connect) {
	//connects current canvas to information in status bar
	fDisplayInfos = connect;
	GVPad *pad = (GVPad *) GetSelectedPad();
	pad->SetCrosshair((int) connect);

}
//______________________________________________________________________________
void Vigru::ConnectionOfCanvas(TCanvas* canvas) {
	//connects current canvas to information in status bar
	canvas->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "Vigru",
			fMainBaseFrame_this, "DisplayInfos(Int_t, Int_t, Int_t,TObject*)");
}
//______________________________________________________________________________

void Vigru::OpenSaveConfig(Int_t numAction) {
	//Opens a dialog, save file dialog or open file dialog, according to the mode.

	//type of configuration files
	const char *FileConf[] = { "vigru configuration files", "*.xml", 0, 0 };

	//type of root files
	const char *FileRoot[] = { "root files", "*.root", 0, 0 };

	//type of root files
	const char *FileTxt[] = { "text files", "*.txt", 0, 0 };

	TString* lastfile;
	lastfile = NULL;
	EFileDialogMode mode;
	mode = kFDOpen;
	TGFileInfo fi;
	TGFileDialog *filedialog;

	switch (numAction) {
	case 1://load all config
		fi.fFileTypes = FileConf;
		mode = kFDOpen;
		lastfile = &fLastConfigDirFile;
		break;

	case 5://load page
		fi.fFileTypes = FileConf;
		mode = kFDOpen;
		lastfile = &fLastConfigDirFile;
		break;
	case 6://save page
		fi.fFileTypes = FileConf;
		mode = kFDSave;
		lastfile = &fLastConfigPageDirFile;

		if ((*lastfile).CompareTo("") == 0) {
			fLastConfigDir = ".";
			(*lastfile) = "vigru_page.xml";
		}
		break;

	case 2:
		fi.fFileTypes = FileConf;
		mode = kFDSave;
		lastfile = &fLastConfigDirFile;

		if ((*lastfile).CompareTo("") == 0) {
			fLastConfigDir = ".";
			(*lastfile) = "vigru.xml";
			numAction = 3;
		}
		break;

	case 3:
		fi.fFileTypes = FileConf;
		mode = kFDSave;
		lastfile = &fLastConfigDirFile;
		if ((*lastfile).CompareTo("") == 0) {
			fLastConfigDir = ".";
			(*lastfile) = "vigru.xml";
		}
		break;

	case 4:
		fi.fFileTypes = FileRoot;
		mode = kFDSave;
		lastfile = &fLastConfigSpeFile;

		if ((*lastfile).CompareTo("") == 0) {
			(*lastfile) = "vigru.root";
		}
		break;
	case 7:
		fi.fFileTypes = FileTxt;
		mode = kFDSave;
		lastfile = &fLastConfigSpeFileTxt;

		if ((*lastfile).CompareTo("") == 0) {
			(*lastfile) = "vigru.txt";
		}
		break;
	}
	fi.fIniDir = StrDup(fLastConfigDir.Data());
	fi.fFilename = StrDup((*lastfile).Data());
	if (numAction != 2) {
		filedialog = new TGFileDialog(gClient->GetRoot(), fMainBaseFrame_this,
				mode, &fi);
		if (filedialog ==NULL)
			fError.TreatError(2, 0, "filedialog not instancied");
		fLastConfigDir = fi.fIniDir;
		(*lastfile) = fi.fFilename;
	}

	if (fi.fFilename != NULL) {

		switch (numAction) {
		case 1:
			RestoreConfiguration((*lastfile));
			break;
		case 2:
			SaveConfiguration((*lastfile), true);
			break;
		case 3:
			SaveConfiguration((*lastfile), true);
			break;
		case 4:
			SaveSpectra((*lastfile));
			break;
		case 5:
			RestoreConfigurationPage((*lastfile));
			break;
		case 6:
			SaveConfigurationPage((*lastfile), true);
			break;
		case 7:
			SaveSpectra((*lastfile), false);
			break;
		}

	}

}

//______________________________________________________________________________
bool Vigru::ReadSpectra(TString filename) {
	TString tempo;
	bool ret = false;
	tempo = "Vigru::ReadSpectra : impossible to read file " + filename;
	TFile *file = TFile::Open(filename.Data(), "read");
	if (!file) {
		fError.TreatError(1, 0, tempo);
		return ret;
	}
	fTabPage->ReadSpectra(file);
	file->Close();
	ret = true;
	gROOT->cd();
	return ret;
}
//______________________________________________________________________________
void Vigru::SaveCuts(Int_t level) {
	//Save Cuts in root format
	// if level = 1 selected TCuts
	// if level = 2 save all cut from pad
	// if level = 3 save all cut of vigru

	Int_t type = 3;

	TString tempos;

	//type of root files
	const char *FileRoot[] = { "Cuts files", "*.root", 0, 0 };

	TString* lastfile;
	lastfile = NULL;
	EFileDialogMode mode;
	mode = kFDSave;
	TGFileInfo fi;
	TGFileDialog *filedialog;

	GSpectraDB * DB = NULL;

	fi.fFileTypes = FileRoot;
	mode = kFDSave;
	lastfile = &fLastConfigCutFile;

	if ((*lastfile).CompareTo("") == 0) {
		(*lastfile) = "grucuts.root";
	}

	fi.fIniDir = StrDup(fLastConfigDir.Data());
	fi.fFilename = StrDup((*lastfile).Data());

	filedialog = new TGFileDialog(gClient->GetRoot(), fMainBaseFrame_this,
			mode, &fi);
	if (filedialog == NULL)
		fError.TreatError(2, 0, "filedialog not instancied");
	fLastConfigDir = fi.fIniDir;
	(*lastfile) = fi.fFilename;
	tempos = "Vigru::ReadSpectra : impossible to create file " + (*lastfile);
	if (level == 3) {
		SaveSpectra(lastfile->Data(), true, type);
		return;
	}

	if (level == 1) {
		DB = GetSpeManager()->GetDB();

	}

	if (level == 2) {
		GVPad* gvpad = (GVPad*) GetSelectedPad();

		if (gvpad)
			DB = gvpad->GetPadDB();
	}

	if (fi.fFilename != NULL) {
		TFile *file = TFile::Open((*lastfile).Data(), "recreate");
		if (!file) {
			fError.TreatError(2, 0, tempos);
			return;
		}
		DB->SaveSpectra(file, type);
		file->Close();
		gROOT->cd();
	}
}

//______________________________________________________________________________
void Vigru::LoadSpectra(Int_t type, GSpectraDB * DB) {
	//Load Cuts in root format	TString tempo;
	//Type of spectra , <0 => all , 0 , 1 raw....
	TString tempos;

	//type of root files
	const char *FileRoot[] = { "Cuts files", "*.root", 0, 0 };
	GSpectraDB * localDB;
	localDB = GetSpeManager()->GetDB();
	if (DB != NULL)
		localDB = DB;
	TString* lastfile;
	lastfile = NULL;
	EFileDialogMode mode;
	mode = kFDOpen;
	TGFileInfo fi;
	TGFileDialog *filedialog;

	fi.fFileTypes = FileRoot;
	lastfile = &fLastConfigCutFile;

	if ((*lastfile).CompareTo("") == 0) {
		(*lastfile) = "grucuts.root";
	}

	fi.fIniDir = StrDup(fLastConfigDir.Data());
	fi.fFilename = StrDup((*lastfile).Data());

	filedialog = new TGFileDialog(gClient->GetRoot(), fMainBaseFrame_this,
			mode, &fi);
	if (filedialog == NULL)
		fError.TreatError(2, 0, "filedialog not instancied");
	fLastConfigDir = fi.fIniDir;
	(*lastfile) = fi.fFilename;
	tempos = "Vigru::LoadCuts : impossible to read file " + (*lastfile);
	if (fi.fFilename != NULL) {
		TFile *file = TFile::Open((*lastfile).Data(), "read");
		if (!file) {
			fError.TreatError(2, 0, tempos);
			return;
		}

		localDB->ReadSpectra(file, type);
		file->Close();
		gROOT->cd();
	}
}

//______________________________________________________________________________
void Vigru::SaveSpectra(TString filename, bool rootortxt, Int_t type) {
	//Save spectra in root format (rootortxt = true) or text format (rootortxt =false)
	// if type <0 save specific spectra ( raw, cuts.. see GSpectrumIdentity.h)
	TString tempo;
	tempo = "Vigru::SaveSpectra : impossible to create file " + filename;

	if (rootortxt) {
		TFile *file = TFile::Open(filename.Data(), "recreate");
		if (!file) {
			fError.TreatError(2, 0, tempo);
			return;
		}
		fTabPage->SaveSpectra(file, type);
		file->Close();
		gROOT->cd();
	} else {

		GVPad* cv = (GVPad*) GetSelectedPad();
		if (!cv)
			return;
		ofstream * file;
		file = new ofstream();
		file->open(filename.Data(), ios::out);
		if (!file) {
			fError.TreatError(2, 0, tempo);
			return;
		}

		*file << "// Spectra file in txt , Luc Legeard - Ganil\n";
		*file << "//- Spectrum Name\n";
		*file << "//- Dimension (1 or 2)\n";
		*file << "//- x bin\n";
		*file << "//- x min\n";
		*file << "//- x max\n";
		*file
				<< "//- y bin if dim =2 , ( in case of dim =2 and ybin = 0 , it is a TGraph2D)\n";
		*file << "//- x min if dim =2 \n";
		*file << "//- x max if dim =2 \n";
		*file << "//- list of x,y (and z if dimension=2)\n";
		*file
				<< "//--------------------------------------------------------------------\n";
		cv->SaveSpectraTxt(file);
		file->close();
	}

}
//______________________________________________________________________________
void Vigru::FitBG() {

	GSpectrumIdentity * id;
	TPad *cv = GetSelectedPad();
	if (!cv)
		return;
	id = GetCurrentSpeId();
	if (id)
		return id->FitBG();
	cv->Update();
}
//______________________________________________________________________________
void Vigru::MessageBoxNoHisto() {
	EMsgBoxIcon icontype = kMBIconExclamation;
	new TGMsgBox(gClient->GetRoot(), fMainBaseFrame_this, "Warning",
			"List of Histograms is empty", icontype, kMBOk);
}
//______________________________________________________________________________
void Vigru::MessageBoxWarning(char* text) {
	EMsgBoxIcon icontype = kMBIconExclamation;
	new TGMsgBox(gClient->GetRoot(), fMainBaseFrame_this, "Warning", text,
			icontype, kMBOk);
}
//______________________________________________________________________________
bool Vigru::IsFileExiste(TString filename, bool withtexterror) {

	struct stat bufstat; // necessary structure to test existing of a device ( using 'stat' fonction )
	int LocalStatus = stat(filename.Data(), &bufstat);
	bool ret;
	if (LocalStatus == -1) {
		if (withtexterror)
			fError.TreatError(2, LocalStatus, fName, " File doesn't existe");
		ret = false;
	} else
		ret = true;
	return ret;
}
//______________________________________________________________________________
void Vigru::Oscillo(TString soapserver, int soapport) {
#ifdef NET_LIB
	cout << " -----------------Oscillo Mode-------------\n";
	TString info;
	info = "4 Channels";
	SetPageChange(info, 2, 2);
	AddPage("With FFT",2, 4);
	fTabPage->SetTab(0);
	GVPad* gvpad = NULL;
	GSpectraDB* db = NULL;
	GSpectrumIdentity * id;

	int tab = 0;

	for (int padi = 1; padi < 5; padi++) {
		gvpad = (GVPad*) fTabPage->GetPad(tab, padi);
		if (gvpad == NULL) {
			fError.TreatError(2, 0, "Vigru::Oscillo, pointer on pad NULL");
		}
		db = gvpad->GetPadDB();
		if (db == NULL) {
			fError.TreatError(2, 0, "Vigru::Oscillo, pointer on db NULL");
		}
		db->DeleteAllIdentities();
		TString name;
		name.Form("Voie_%d", padi);
		TString sName = "localhost";
		TString sType = "GRU";
		TString source = "MEM";
		id = new GSpectrumIdentity(NULL, name, sName,
				sType, source, 9090, "", tab, padi, 0, true, (TString) "TH1I");
		id->SetAction(true);
		id->SetLineColor(602);
		db->MyAddLast(id);
	}
	tab=1;
	ChangeTabPage(tab);

	for (int padi = 1; padi < 5; padi++){

		gvpad = (GVPad*) fTabPage->GetPad(tab, padi*2-1);
				if (gvpad == NULL) {
					fError.TreatError(2, 0, "Vigru::Oscillo, pointer on pad NULL");
				}
				db = gvpad->GetPadDB();
				if (db == NULL) {
					fError.TreatError(2, 0, "Vigru::Oscillo, pointer on db NULL");
				}
				db->DeleteAllIdentities();
				TString name;
				name.Form("Voie_%d", padi);
				TString sName = "localhost";
				TString sType = "GRU";
				TString source = "MEM";
				id = new GSpectrumIdentity(NULL, name, sName,
						sType, source, 9090, "", tab, padi*2-1, 0, true, (TString) "TH1I");
				id->SetAction(true);
				id->SetLineColor(602);
				db->MyAddLast(id);



		gvpad = (GVPad*) fTabPage->GetPad(tab, padi*2);
		//gvpad->ApplyComputation(GVPad::COMPUTATION_FFThalf, 1, padi*2-1);
		gvpad->SetComputation(GVPad::COMPUTATION_FFThalf);
		gvpad->SetDuplicationPad(padi*2-1);
		gvpad->SetDuplicationTab(1);
}

	GetSpeManager()->GetServerDB()->AddServerIdentity("localhost", "MEM","MEM", 0);
	GNetClientSoap *net = new GNetClientSoap((char*) (soapserver.Data()));
	net->SetPort(soapport);
	net->Open();
	fAcqNumexo = new GAcqNumexo(net);
	fAcqNumexo->EventInit((char*) "local", (char*) "mfm",false,true);
	fAcqNumexo->SetSpectraMode(1);
	fAcqNumexo->InitUser();
		// mise a jour de la data base ,de vigru
		GetSpeManager()->GetDB()->DeleteAllIdentities();
		GetSpeManager()->UpdateSpectraList();
		fTabPage->SetTab(0);
#else
	cout << " -----------------Oscillo Mode impossible with no soap-------------\n";
#endif
	return;
}
//______________________________________________________________________________
void Vigru::OscilloRun() {
#ifdef NET_LIB
	if (fAcqNumexo)
		if ((fVerboseDone)==false){
			fAcqNumexo->SetVerbose(fVerbose);
			fVerboseDone = true;
		}
	fAcqNumexo->DoRun(4, 0);
	return;
#endif
}
//______________________________________________________________________________
void Vigru::Demo() {
	fError.TreatError(0, 0, " Demo  started ");
	GVPad* gvpad = NULL;
	GSpectraDB* db = NULL;
	GSpectrumIdentity * id;
	TString name;
	TString sName = "ganp615";
	TString sType = "GRU";
	TString source = "MEM";
	int padi = 1;
	int tab =0;
	GDemo* fDemo = new GDemo();

	GetSpeManager()->GetServerDB()->AddServerIdentity("localhost", "MEM","MEM", 0);
	fDemo->InitSpectra();
	fDemo->InGSpectra();
	fDemo->GSpectraInMem();
	fDemo->SpectreAliveT();
	SetPageChange("DEMO_1", 2, 2);
	AddPage("DEMO_2", 4, 3);
	SetPage(0);
	ChangeTabPage(0);
	GetSpeManager()->UpdateSpectraList();

	gvpad = (GVPad*) fTabPage->GetPad(tab, padi);
	if (gvpad == NULL) {
		fError.TreatError(2, 0, "Vigru::Demo, pointer on pad NULL");
					}
	db = gvpad->GetPadDB();
	if (db == NULL) {
		fError.TreatError(2, 0, "Vigru::Demo, pointer on db NULL");
		}
	db->DeleteAllIdentities();

   padi=1;tab=0;
   gvpad = (GVPad*) fTabPage->GetPad(tab, padi);
   db = gvpad->GetPadDB();
   id = new GSpectrumIdentity(NULL, "Histo2D_1", sName, sType, source, 9090, "Family/2D_ganp615_0", tab, padi*2-1, 0, true, (TString) "TH1I");
   id->SetAction(true);
   id->SetLineColor(602);
   db->MyAddLast(id);

   padi=2;tab=0;
   gvpad = (GVPad*) fTabPage->GetPad(tab, padi);
   db = gvpad->GetPadDB();
   id = new GSpectrumIdentity(NULL, "fHpx_0", sName, sType, source, 9090, "Px_Distribution_ganp615_0", tab, padi*2-1, 0, true, (TString) "TH1I");
   id->SetAction(true);
   id->SetLineColor(602);
   db->MyAddLast(id);

   padi=3;tab=0;
   gvpad = (GVPad*) fTabPage->GetPad(tab, padi);
   db = gvpad->GetPadDB();
   id = new GSpectrumIdentity(NULL, "fHpxpy_0", sName, sType, source, 9090, "Py_vs_px_ganp615_0", tab, padi*2-1, 0, true, (TString) "TH2I");
   id->SetAction(true);
   id->SetLineColor(602);
   db->MyAddLast(id);

   padi=4;tab=0;
   gvpad = (GVPad*) fTabPage->GetPad(tab, padi);
   db = gvpad->GetPadDB();
   id = new GSpectrumIdentity(NULL, "fHprof_0", sName, sType, source, 9090, "Profile_ganp615_0", tab, padi*2-1, 0, true, (TString) "TH1I");
   id->SetAction(true);
   id->SetLineColor(602);
    db->MyAddLast(id);


   RefreshPage(false, true);
  // RefreshAllPages(true);
  // RefreshAllPages(true);
	return;
}

