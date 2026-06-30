// Authors: Luc Legeard/ J�r�me Chauveau
//////////////////////////////////////////////////////////////////////////////
//class GVDialogSource
//Frame where user choose spectra sources: from net, from file ...
//
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


#include "GVDialogSource.h"
#include <TGButton.h>
#include<TGFileDialog.h>
#include <TTimer.h>
#include <TVirtualPadEditor.h>
#include "Vigru.h"
#include <Riostream.h>
#include <TGMsgBox.h>
#include "GSpectra.h"
#include "GDevice.h"
#include "GNetClientRoot.h"
#if NET_LIB
#include "GNetClientSoap.h"

#endif
#include "TGNumberEntry.h"

ClassImp( GVDialogSource)
const char *files[] = { "ROOT files", "*.root", "*.1D", "*.2D", 0, 0 };

GVDialogSource::GVDialogSource(const TGWindow *p, const TGWindow *m, UInt_t w,
		UInt_t h) :
	TGTransientFrame(p, m, w, h) {
	fChange = 0;
	// Initialisation of pointers
	fButGroupGruSoap = NULL;
	fButGroupNetFile = NULL;
	fServerListGroupFrame = NULL;
	fNumEntryA = NULL;
	fNumEntryB = NULL;
	fNumEntryPort = NULL;
	fLabelTitle = NULL;
	fLabelFile = NULL;
	fLabelHost = NULL;

	for (int i = 0; i <  GVDIAG_MAX; i++) {
		fRadioType[i] = NULL;
	}

	fAddServer = NULL;
	fTitle = NULL;

	fEntryHostOrFile = NULL;
	fPort = 0;
	fPortdef = 9090;
	fPort = fPortdef;
	fButtonBrowse = NULL;
	fButtonOk = NULL;
	fMainBase = (TGMainFrame*) m;
	fServerType = GVDIAG_UNKOWN;
	// Initialisation of variables
	fSourceType = "GRU";
	fSourceName = "localhost";
	fServerType = GVDIAG_GRUNET;

	//colors
	if (!gClient->GetColorByName("white", white))
		fError.TreatError(1, 0, "White color not defined");
	if (!gClient->GetColorByName("orange", orange))
		fError.TreatError(1, 0, "Orange color not defined");
	if (!gClient->GetColorByName("black", black))
		fError.TreatError(1, 0, "Black color not defined");
	if (!gClient->GetColorByName("red", red))
		fError.TreatError(1, 0, "Red color not defined");
	//-----------------General Frame---------------------------
	SetWindowName("Setup sources");
	SetLayoutManager(new TGVerticalLayout(this));
	fLayout1 = new TGLayoutHints(kLHintsCenterY, 5, 5, 5, 5);
	fLayout2 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);
	fLayout3 = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10);
	fLayout4 = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY);
	fLayout5 = new TGLayoutHints(kLHintsCenterX | kLHintsLeft, 0, 20);
	fLayout6 = new TGLayoutHints(kLHintsCenterX | kLHintsLeft, 0, 5);
	fLayout7 = new TGLayoutHints(kLHintsNormal | kLHintsLeft, 0, 0);
	//-----------------AddServ Frame---------------------------
	fAddServer = new TGGroupFrame(this, "Add a Spectra Server");

	fLabelType = new TGLabel(fAddServer, "Type of Server");
	fAddServer->AddFrame(fLabelType, fLayout7);

	fAddServerSub2 = new TGCompositeFrame(fAddServer);
	fAddServerSub2->SetLayoutManager(new TGHorizontalLayout(fAddServerSub2));

	fButGroupNetFile = new TGButtonGroup(fAddServerSub2, "Type", kChildFrame
			| kHorizontalFrame);
	fButGroupNetFile->SetLayoutHints(new TGLayoutHints(
			kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
	fRadioType[GVDIAG_GRUNET-1] = new TGRadioButton(fButGroupNetFile, new TGHotString(
			"GRU_Net   "));
	fRadioType[GVDIAG_GRUNET-1]->Connect("Pressed()", "GVDialogSource", this,
			"PressedGRUNet()");
	fRadioType[GVDIAG_ROOTFILE-1] = new TGRadioButton(fButGroupNetFile, new TGHotString(
			"Root_File   "));
	fRadioType[GVDIAG_ROOTFILE-1]->Connect("Pressed()", "GVDialogSource", this, "PressedFile()");

	fRadioType[GVDIAG_MEMORY-1] = new TGRadioButton(fButGroupNetFile, new TGHotString(
			"Memory   "));
	fRadioType[GVDIAG_MEMORY-1]->Connect("Pressed()", "GVDialogSource", this,
			"PressedMemory()");

#if NET_LIB
	fRadioType[GVDIAG_SOAPNET-1] = new TGRadioButton(fButGroupNetFile, new TGHotString(
					"Soap_Net   "));
	fRadioType[GVDIAG_SOAPNET-1]->Connect("Pressed()", "GVDialogSource", this,
			"PressedSoapNet()");
#endif




	fRadioType[GVDIAG_GRUNET-1]->SetState(kButtonDown);
	fButGroupNetFile->SetBorderDrawn(kFALSE);
	fAddServerSub2->AddFrame(fButGroupNetFile, fLayout5);// line commneted

	fButGroupNetFile->Show();// line commneted
	fAddServer->AddFrame(fAddServerSub2, fLayout2);

	fAddServerSub1 = new TGCompositeFrame(fAddServer);
	fAddServerSub1->SetLayoutManager(new TGHorizontalLayout(fAddServerSub1));

	fLabelHost = new TGLabel(fAddServerSub1, "Host/File Name:");
	fAddServerSub1->AddFrame(fLabelHost, fLayout2);

	fEntryHostOrFile = new TGTextEntry(fAddServerSub1);
	fEntryHostOrFile->SetText("localhost");
	fAddServerSub1->AddFrame(fEntryHostOrFile, fLayout5);

	fButtonBrowse = new TGTextButton(fAddServerSub1, "Browser"); //Browser // line commneted
	fButtonBrowse->Connect("Clicked()", "GVDialogSource", this, "OpenFile()");// line commneted
	fButtonBrowse->Resize(50, 30);// line commneted
	fButtonBrowse->SetTextColor(black, kTRUE);// line commneted
	fButtonBrowse->SetBackgroundColor(white);// line commneted
	fButtonBrowse->SetEnabled(kFALSE);// line commneted
	fAddServerSub1->AddFrame(fButtonBrowse, fLayout5);// line commneted

	fLabelPort = new TGLabel(fAddServerSub1, "Port:");
	fAddServerSub1->AddFrame(fLabelPort, fLayout2);

	fNumEntryPort = new TGNumberEntry(fAddServerSub1);
	fNumEntryPort->SetIntNumber(fPortdef);
	fAddServerSub1->AddFrame(fNumEntryPort, fLayout5);
	fAddServer->AddFrame(fAddServerSub1, fLayout2);

	fAddButtonList = new TGTextButton(fAddServer, "Add Server");
	fAddButtonList->Connect("Clicked()", "GVDialogSource", this,
			"AddListServers()");
	fAddButtonList->SetTextColor(black, kTRUE);
	fAddButtonList->SetBackgroundColor(orange);
	fAddButtonList->Resize(200, 40);
	fAddServer->AddFrame(fAddButtonList, fLayout2);

	AddFrame(fAddServer, fLayout2);

	//-----------------fServerList-------------------------------
	fServerListGroupFrame = new TGGroupFrame(this, "Servers List");

	fListBox = new TGListBox(fServerListGroupFrame);
	fListBox->Resize(420, 250);
	FillListBoxWithServerList();
	fServerListGroupFrame->AddFrame(fListBox, fLayout2);

	fTestButton = new TGTextButton(fServerListGroupFrame,
			"Test Selected Server");
	fTestButton->Connect("Clicked()", "GVDialogSource", this, "TestServer()");
	fTestButton->Resize(180, 30);
	fServerListGroupFrame->AddFrame(fTestButton);

	fRemoveButton = new TGTextButton(fServerListGroupFrame,
			"Remove Selected Server");
	fRemoveButton->Connect("Clicked()", "GVDialogSource", this,
			"RemoveServer()");
	fRemoveButton->Resize(180, 30);
	fServerListGroupFrame->AddFrame(fRemoveButton);

	AddFrame(fServerListGroupFrame, fLayout2);
	//--------------------------------------------

	fButtonOk = new TGTextButton(this, "   OK   ");
	fButtonOk->SetTextColor(black, kTRUE);
	fButtonOk->SetBackgroundColor(orange);
	fButtonOk->Connect("Clicked()", "GVDialogSource", this, "DoEnd()");
	fButtonOk->Resize(50, 30);
	AddFrame(fButtonOk, fLayout4);

	Layout();
	MapSubwindows();
	MapWindow();
	GetClient()->WaitFor(this);
}

GVDialogSource::~GVDialogSource() {
	Cleanup();
}

void GVDialogSource::PressedGRUNet() {
	if (fButtonBrowse)
		fButtonBrowse->SetEnabled(kFALSE);
	if (fNumEntryPort) {
		fNumEntryPort->SetState(kTRUE);
		fNumEntryPort->SetIntNumber(fPortdef);
		fPort = fPortdef;
	}
	fServerType = GVDIAG_GRUNET;
}

void GVDialogSource::PressedSoapNet() {
	if (fButtonBrowse)
		fButtonBrowse->SetEnabled(kFALSE);
	if (fNumEntryPort) {
		fPort = 6603;
		fNumEntryPort->SetState(kTRUE);
		fNumEntryPort->SetIntNumber(fPort);
	}
	fServerType = GVDIAG_SOAPNET;
}

void GVDialogSource::PressedFile() {
	if (fButtonBrowse)
		fButtonBrowse->SetEnabled(kTRUE);
	fNumEntryPort->SetState(kFALSE);
	fServerType = GVDIAG_ROOTFILE;
}


void GVDialogSource::PressedMemory() {

	if (fButtonBrowse)
		fButtonBrowse->SetEnabled(kTRUE);
	fNumEntryPort->SetState(kFALSE);
	if (fButtonBrowse)
		fButtonBrowse->SetEnabled(kFALSE);
	if (fNumEntryPort) {
		fNumEntryPort->SetState(kFALSE);
	}
	fServerType = GVDIAG_MEMORY;
}
void GVDialogSource::OpenFile() {
	static TString dir(".");
	TGFileInfo fi;
	fi.fFileTypes = files;
	fi.fIniDir = StrDup(dir);
	new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);
	dir = fi.fIniDir;
	if (fi.fFilename != NULL) {
		fEntryHostOrFile->SetText(fi.fFilename);
		fSourceName = fi.fFilename;
	}
}

void GVDialogSource::FillListBoxWithServerList() {
	Vigru *mainframe = (Vigru*) fMainBase;
	GSpectraDB* ServerDB;
	ServerDB = mainframe->GetSpeManager()->GetServerDB();
	char tmp[256];
	TString sourceName, sourceType, sourceNetFile;
	Int_t port;
	fListBox->RemoveAll();
	for (int i = 0; i <= ServerDB->GetLast(); ++i) {
		GSpectrumIdentity* id = (GSpectrumIdentity*) ServerDB->At(i);
		sourceName = id->GetSourceName();
		sourceType = id->GetSourceType();
		sourceNetFile = id->GetSource();
		port = id->GetPort();
		sprintf(tmp, "%s  %s %s %d ", sourceName.Data(), sourceType.Data(),
				sourceNetFile.Data(), port);
		fListBox->AddEntry(tmp, i);
		fListBox->MapSubwindows();
		fListBox->Layout();
	}

}
void GVDialogSource::RefreshListServers() {
	Vigru *mainframe = (Vigru*) fMainBase;
	mainframe->GetSpeManager()->UpdateSpectraList();
	fChange = 0;
}

void GVDialogSource::RemoveServer() {
	Vigru *mainframe = (Vigru*) fMainBase;
	int i = fListBox->GetSelected();
	EMsgBoxIcon icontype = kMBIconStop;
	TString message;
	Int_t retval = 0;

	if (i < 0) {
		message.Form("No source selected");
		new TGMsgBox(gClient->GetRoot(), this, "OK", message.Data(), icontype,
				1, &retval);
		return;
	}
	if (i > mainframe->GetSpeManager()->GetServerDB()->GetLast()) {
		message.Form("Propblem in selection please restart selection");
		new TGMsgBox(gClient->GetRoot(), this, "OK", message.Data(), icontype,
				1, &retval);
		return;
	}
	GSpectrumIdentity
			* id =
					(GSpectrumIdentity*) (mainframe->GetSpeManager()->GetServerDB()->At(
							i));
	mainframe->GetSpeManager()->GetServerDB()->DeleteIdentity(id);
	FillListBoxWithServerList();
}

void GVDialogSource::TestServer() {
	Vigru *mainframe = (Vigru*) fMainBase;
	EMsgBoxIcon icontypeStop = kMBIconStop;
	EMsgBoxIcon icontypeGood = kMBIconAsterisk ;
	TString message;
	Int_t retval = 0;
	int port =0;
	int i = fListBox->GetSelected();
	if (i < 0) {
		message.Form("No source selected");
		new TGMsgBox(gClient->GetRoot(), this, "OK", message.Data(), icontypeGood,
				1, &retval);
		return;
	}

	GSpectrumIdentity
			* id =
					(GSpectrumIdentity*) (mainframe->GetSpeManager()->GetServerDB()->At(
							i));
	Int_t test = id->TestServer();
	port = id->GetPort();
	message.Form("Test server OK %s:%d", (id->GetSourceName().Data()), port);
	if (test > 0) {
		message.Form("Test server OK %s:%d", (id->GetSourceName().Data()),
				port);
		new TGMsgBox(gClient->GetRoot(), this, "OK", message.Data(), icontypeGood,
				1, &retval);

	}
	if (test == 0) {
		{
			message.Form("Wrong Server %s:%d", fSourceName.Data(), port);
			new TGMsgBox(gClient->GetRoot(), this, "Warning", message.Data(),
					icontypeStop, 1, &retval);
		}

		if (test < 0) {
			new TGMsgBox(gClient->GetRoot(), this, "Warning",
					"Test not available for this server", icontypeStop, 1, &retval);
			fError.TreatError(0, 0,
					"Test only available for Gru and Soap network server!");
		}
	}
}

void GVDialogSource::AddListServers() {

	Vigru *mainframe = (Vigru*) fMainBase;
	fSourceName = fEntryHostOrFile->GetText();
	fPort = (Int_t) fNumEntryPort->GetNumber();
	if (fServerType == GVDIAG_GRUNET) {
		fSourceType = "GRU";
		fSourceNetFile = "NET";
		fPort = (Int_t) fNumEntryPort->GetNumber();
		fChange++;
	}
	if (fServerType == GVDIAG_SOAPNET) {
		fSourceType = "SOAP";
		fSourceNetFile = "NET";
		fPort = (Int_t) fNumEntryPort->GetNumber();
		fChange++;
	}

	if (fServerType == GVDIAG_ROOTFILE) {
		fSourceType = "GRU";
		fSourceNetFile = "FILE";
		fPort = 0;
		fChange++;
	}
	if (fServerType == GVDIAG_MEMORY) {
		fSourceType = "MEM";
		fSourceNetFile = "MEM";
		fPort = 0;
		fChange++;
	}

	mainframe->GetSpeManager()->GetServerDB()-> AddServerIdentity(fSourceName,
			fSourceType, fSourceNetFile, fPort);
	//mainframe->GetSpeManager()->GetServerDB()->MakeDUMP();
	FillListBoxWithServerList();
}

void GVDialogSource::DoEnd() {
	//AddListServers();
	//update list spectra;

	if (fChange > 0)
		RefreshListServers();
	TTimer::SingleShot(150, "GVDialogSource", this, "CloseWindow()");
	// Close the Ged editor if it was activated.
	if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
		TVirtualPadEditor::Terminate();
}

void GVDialogSource::CloseWindow() {
	//delete this;
	UnmapWindow();
	DeleteWindow();
	//Cleanup();
}

