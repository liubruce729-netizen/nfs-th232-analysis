// File : GVMyQuestion.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////
//
// Class GVMyQuestion
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


#include "GVMyQuestion.h"
#include "Vigru.h"
#include <TGLayout.h>
#include <TVirtualPadEditor.h>
#include <TGMsgBox.h>
#include "math.h"
#include <TSystem.h>
#include <TROOT.h>
#include <TGClient.h>
#include <TStyle.h>


ClassImp(GVMyQuestion)

GVMyQuestion::GVMyQuestion(const TGWindow *p, const TGWindow *m, TString questions, TString messageWindow, TString MessageBox,
		Int_t* keyreturn,UInt_t w,UInt_t h) :
	TGTransientFrame(p, m, w, h) {

	UInt_t nb, width, height;

	fMain = (Vigru*) m;
	fNbofQuestions = 0;
	fListOfQuestion = questions;
	fSelectedButton = 0;
	TObjArray * tabWords = NULL;
	TString tempos;
	nb = 0;
	width = 0;
	height = 0;
	tabWords = fListOfQuestion.Tokenize(";");
	fNbofQuestions = (tabWords->GetLast())+1;
	fPtNbofQuestions = keyreturn;

	const char *msg = MessageBox.Data();

	Int_t text_align = kTextCenterX | kTextCenterY;
	char *line;
	char *nextLine;
	TGLabel * label;
	fLabelFrame = new TGVerticalFrame(this, 60, 20);
	fL4 = new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX, 4,
			2, 2, 2);
	fL2 = new TGLayoutHints(kLHintsBottom  | kLHintsCenterX, 0, 0, 5, 5);
	fL1 = new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 3, 3, 0, 0);

	fGroup  = new TGHorizontalFrame(this, 60, 20, kFixedWidth);


	char* tmpMsg = new char[strlen(msg) + 1];
	nextLine = tmpMsg;
	TList* fMsgList = new TList;
	line = tmpMsg;
	strncpy(nextLine, msg, strlen(msg) + 1);

	while ((nextLine = strchr(line, '\n'))) {
		*nextLine = 0;
		label = new TGLabel(fLabelFrame, line);
		label->SetTextJustify(text_align);
		fMsgList->Add(label);
		fLabelFrame->AddFrame(label, fL4);
		line = nextLine + 1;
	}

	label = new TGLabel(fLabelFrame, line);
	label->SetTextJustify(text_align);
	fMsgList->Add(label);
	fLabelFrame->AddFrame(label, fL4);
	delete[] tmpMsg;

	fButton = new TGTextButton*[fNbofQuestions];

	if (fNbofQuestions > 12) {
		fError.TreatError(2, 0, "Number of buttons is too high");
		exit(0);
	}
	for (Int_t i = 0; i < fNbofQuestions; i++) {
		tempos = (TString)(tabWords->At(i))->GetName();
		fButton[i] = new TGTextButton(fGroup, tempos);
		fButton[i]->Associate(this);
		width = TMath::Max(width, fButton[i]->GetDefaultWidth());
		++nb;
		fGroup-> AddFrame(fButton[i], fL1);
	}

	if (fNbofQuestions > 0)
		fButton[0] ->Connect("Clicked()", "GVMyQuestion", this, "DoOk0()");
	if (fNbofQuestions > 1)
		fButton[1] ->Connect("Clicked()", "GVMyQuestion", this, "DoOk1()");
	if (fNbofQuestions > 2)
		fButton[2] ->Connect("Clicked()", "GVMyQuestion", this, "DoOk2()");
	if (fNbofQuestions > 3)
		fButton[3] ->Connect("Clicked()", "GVMyQuestion", this, "DoOk3()");
	if (fNbofQuestions > 4)
		fButton[4] ->Connect("Clicked()", "GVMyQuestion", this, "DoOk4()");
	if (fNbofQuestions > 5)
		fButton[5] ->Connect("Clicked()", "GVMyQuestion", this, "DoOk5()");
	if (fNbofQuestions > 6)
		fButton[6] ->Connect("Clicked()", "GVMyQuestion", this, "DoOk6()");
	if (fNbofQuestions > 7)
		fButton[7] ->Connect("Clicked()", "GVMyQuestion", this, "DoOk7()");
	if (fNbofQuestions > 8)
		fButton[8] ->Connect("Clicked()", "GVMyQuestion", this, "DoOk8()");
	if (fNbofQuestions > 9)
		fButton[9] ->Connect("Clicked()", "GVMyQuestion", this, "DoOk9()");
	if (fNbofQuestions > 10)
		fButton[10] ->Connect("Clicked()", "GVMyQuestion", this, "DoOk10()");
	if (fNbofQuestions > 11)
		fButton[11] ->Connect("Clicked()", "GVMyQuestion", this, "DoOk11()");
	if (fNbofQuestions > 12)
		fButton[12] ->Connect("Clicked()", "GVMyQuestion", this, "DoOk12()");

	fGroup->Resize((width + 20) * nb, GetDefaultHeight());

	AddFrame(fLabelFrame, fL4);
	AddFrame(fGroup,fL2);

	MapSubwindows();
	SetWindowName(messageWindow.Data());

	width = GetDefaultWidth();
	height = GetDefaultHeight();

	Resize(width, height);
	CenterOnParent();
	SetWMSize(width, height);
	SetWMSizeHints(width, height, width, height, 0, 0);
	SetMWMHints(kMWMDecorAll | kMWMDecorResizeH | kMWMDecorMaximize
			| kMWMDecorMinimize | kMWMDecorMenu, kMWMFuncAll | kMWMFuncResize
			| kMWMFuncMaximize | kMWMFuncMinimize, kMWMInputModeless);

	MapRaised();
	Layout();

	CenterOnParent();
	MapWindow();
	GetClient()->WaitFor(this);
}

GVMyQuestion::~GVMyQuestion() {
	Cleanup();
	// all object added with AddFrame are already removed with Cleanup
}

void GVMyQuestion::DoOk0() {
	fSelectedButton = 0;
	(*fPtNbofQuestions) = fSelectedButton;
	DoQuit();
}
void GVMyQuestion::DoOk1() {
	fSelectedButton = 1;
	(*fPtNbofQuestions) = fSelectedButton;
	DoQuit();
}

void GVMyQuestion::DoOk2() {
	fSelectedButton = 2;
	(*fPtNbofQuestions) = fSelectedButton;
	DoQuit();
}

void GVMyQuestion::DoOk3() {
	fSelectedButton = 3;
	(*fPtNbofQuestions) = fSelectedButton;
	DoQuit();
}

void GVMyQuestion::DoOk4() {
	fSelectedButton = 4;
	(*fPtNbofQuestions) = fSelectedButton;
	DoQuit();
}

void GVMyQuestion::DoOk5() {
	fSelectedButton = 5;
	(*fPtNbofQuestions) = fSelectedButton;
	DoQuit();
}

void GVMyQuestion::DoOk6() {
	fSelectedButton = 6;
	(*fPtNbofQuestions) = fSelectedButton;
	DoQuit();

}

void GVMyQuestion::DoOk7() {
	fSelectedButton = 7;
	(*fPtNbofQuestions) = fSelectedButton;
	DoQuit();
}

void GVMyQuestion::DoOk8() {
	fSelectedButton = 8;
	(*fPtNbofQuestions) = fSelectedButton;
	DoQuit();
}

void GVMyQuestion::DoOk9() {
	fSelectedButton = 9;
	(*fPtNbofQuestions) = fSelectedButton;
	DoQuit();
}

void GVMyQuestion::DoOk10() {
	fSelectedButton = 10;
	(*fPtNbofQuestions) = fSelectedButton;
	DoQuit();
}

void GVMyQuestion::DoOk11() {
	fSelectedButton = 11;
	(*fPtNbofQuestions) = fSelectedButton;
	DoQuit();
}

void GVMyQuestion::DoOk12() {
	fSelectedButton = 0;
	(*fPtNbofQuestions) = fSelectedButton;
	DoQuit();
}

void GVMyQuestion::DoQuit() {

	TTimer::SingleShot(150, "GVMyQuestion", this, "CloseWindow()");

	// Close the Ged editor if it was activated.
	if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
		TVirtualPadEditor::Terminate();
}
void GVMyQuestion::CloseWindow() {
	UnmapWindow();
	DeleteWindow();

}
