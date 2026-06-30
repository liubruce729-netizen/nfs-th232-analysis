// File : GVTab.C
// Author: Jerome Chauveau / Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//class GVTab
// Manage Specific Tab with a embedded Canvas
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

#include <TList.h>
#include <Riostream.h>
#include <TCanvas.h>
#include <TRootEmbeddedCanvas.h>
#include <TGMsgBox.h>
#include "GVTab.h"
#include "Vigru.h"
#include "GVMyQuestion.h"

//______________________________________________________________________________
ClassImp( GVTab);
#ifdef _mdimode_
GVTab::GVTab(const TGWindow *p, TGMdiFrame *f) :
	TGTab(p) {

	fMain = (TGMdiFrame*)f;
	Connect("Selected(Int_t)", "GVTab", this, "SetPageAction(Int_t)");
	fAutoMode = kFALSE;
}
#else
GVTab::GVTab(const TGWindow *p, TGMainFrame *f) :
	TGTab(p) {

	fMain = (TGMainFrame*)f;
	Connect("Selected(Int_t)", "GVTab", this, "SetPageAction(Int_t)");
	fAutoMode = kFALSE;
}
#endif
//______________________________________________________________________________

GVTab::~GVTab() {

}
//______________________________________________________________________________

myTGCompositeFrame* GVTab::AddTab(const char *text, Int_t nbPadX, Int_t nbPadY) {

	TGString textstring;
	textstring = text;
	myTGCompositeFrame *cf;
	cf = NULL;
	Int_t limit_tab = 512;

	Int_t nbTab = GetNumberOfTabs();

	if ((nbPadY < 0) || (nbPadX < 0)) {
		//fError.TreatError(1, 0, " No tab add because bad nb Pads.");
		return cf;
	}

	if ((nbPadY == 0) || (nbPadX == 0)) {
		fError.TreatError(1, 0, " No tab add because bad parameter.");
		return cf;
	}

	if (nbTab > limit_tab) {
		TString tempo;
		tempo.Form("Limit of %d  tabs reached !", limit_tab);
		fError.TreatError(1, 0, tempo);
		return cf;
	}

	if (textstring.CompareTo("") == 0) {
		textstring.Form("Tab_%d", nbTab);
	}

	UInt_t w = GetWidth();
	UInt_t h = GetHeight();
	cf = new myTGCompositeFrame(this, w, h - 21);

	TGTab::AddTab((const char*) textstring.Data(), (TGCompositeFrame*) cf);

	TRootEmbeddedCanvas *EmbeddedCanvas = new TRootEmbeddedCanvas("emb", cf);
	cf->AddFrame(EmbeddedCanvas, new TGLayoutHints(kLHintsExpandX
			| kLHintsExpandY));
	EmbeddedCanvas->GetCanvas()->SetFillColor(19);
	cf->SetInfo(EmbeddedCanvas, textstring, nbPadX, nbPadY);

	SetTab(nbTab);
	DivideCurrentPage(nbPadX, nbPadY);
	Vigru *f = (Vigru*) GetMainFrame();
	f->ConnectionOfCanvas(EmbeddedCanvas->GetCanvas());

	Layout();
	MapSubwindows();
	MapWindow();

	return cf;
}

//______________________________________________________________________________
void GVTab::DivideCurrentPage(Int_t nPadX, Int_t nPadY) {
	// divide current tab  nPadX . nPadY pads
	if ((nPadX < 1) || (nPadY < 1))
		return;
	TCanvas *canvas = GetCurrentCanvas();
	if (canvas) {
		canvas->cd();
		canvas->Clear();
		MyDivide(canvas, nPadX, nPadY, 0.002, 0.002);
		canvas->cd(1);
		canvas->Update();
		myTGCompositeFrame* myTG;
		myTG = (myTGCompositeFrame*) GetCurrentContainer();
		myTG->SetNPady(nPadY);
		myTG->SetNPadx(nPadX);
	} else {
		fError.TreatError(2, 0, "Impossible to divide canvas");
	}

}
//______________________________________________________________________________
void GVTab::MyDivide(TCanvas *canvas, Int_t nx, Int_t ny, Float_t xmargin,
		Float_t ymargin, Int_t color) {
	// Automatic GVpad generation by division.
	//
	//GVPad* mypad = (GVPad*) canvas;
	//mypad->GVPad::Divide( nx,  ny, 0.002,0.002, color);
	//return;

	TPad* mypad = (TPad*) canvas;

	if (!(mypad->IsEditable()))
		return;

	if (gThreadXAR) {
		void *arr[7];
		arr[1] = this;
		arr[2] = (void*) &nx;
		arr[3] = (void*) &ny;
		arr[4] = (void*) &xmargin;
		arr[5] = (void *) &ymargin;
		arr[6] = (void *) &color;
		if ((*gThreadXAR)("PDCD", 7, arr, 0))
			return;
	}

	TPad *padsav = (TPad*) gPad;
	mypad->cd();
	if (nx <= 0)
		nx = 1;
	if (ny <= 0)
		ny = 1;
	Int_t ix, iy;
	Double_t x1, y1, x2, y2;
	Double_t dx, dy;
	GVPad * gvpad;

	char *name = new char[strlen(GetName()) + 6];
	char *title = new char[strlen(GetTitle()) + 6];
	Int_t n = 0;
	if (color == 0)
		color = mypad->GetFillColor();
	if (xmargin > 0 && ymargin > 0) {
		//general case
		dy = 1 / Double_t(ny);
		dx = 1 / Double_t(nx);
		for (iy = 0; iy < ny; iy++) {
			y2 = 1 - iy * dy - ymargin;
			y1 = y2 - dy + 2* ymargin ;
			if (y1 < 0)
				y1 = 0;
			if (y1 > y2)
				continue;
			for (ix = 0; ix < nx; ix++) {
				x1 = ix * dx + xmargin;
				x2 = x1 + dx - 2* xmargin ;
				if (x1 > x2)
					continue;
				n++;
				sprintf(name, "%s_%d", GetName(), n);
				gvpad = new GVPad(fMain, name, name, x1, y1, x2, y2, color);
				gvpad->SetNumber(n);
				gvpad->Draw();
			}
		}
	} else {
		// special case when xmargin <= 0 && ymargin <= 0
		Double_t xl = mypad->GetLeftMargin();
		Double_t xr = mypad->GetRightMargin();
		Double_t yb = mypad->GetBottomMargin();
		Double_t yt = mypad->GetTopMargin();
		xl /= (1 - xl + xr) * nx;
		xr /= (1 - xl + xr) * nx;
		yb /= (1 - xl + xr) * ny;
		yt /= (1 - xl + xr) * ny;
		mypad->SetLeftMargin(xl);
		mypad->SetRightMargin(xr);
		mypad->SetBottomMargin(yb);
		mypad->SetTopMargin(yt);
		dx = (1 - xl - xr) / nx;
		dy = (1 - yb - yt) / ny;
		Int_t number = 0;
		for (Int_t i = 0; i < nx; i++) {
			x1 = i * dx + xl;
			x2 = x1 + dx;
			if (i == 0)
				x1 = 0;
			if (i == nx - 1)
				x2 = 1 - xr;
			for (Int_t j = 0; j < ny; j++) {
				number = j * nx + i + 1;
				y2 = 1 - j * dy - yt;
				y1 = y2 - dy;
				if (j == 0)
					y2 = 1 - yt;
				if (j == ny - 1)
					y1 = 0;
				sprintf(name, "%s_%d", GetName(), number);
				sprintf(title, "%s_%d", GetTitle(), number);
				gvpad = new GVPad(fMain, name, title, x1, y1, x2, y2);
				gvpad->SetNumber(number);
				gvpad->SetBorderMode(0);
				if (i == 0)
					gvpad->SetLeftMargin(xl * nx);
				else
					gvpad->SetLeftMargin(0);
				gvpad->SetRightMargin(0);
				gvpad->SetTopMargin(0);
				if (j == ny - 1)
					gvpad->SetBottomMargin(yb * ny);
				else
					gvpad->SetBottomMargin(0);
				gvpad->Draw();
			}
		}

	}
	delete[] name;
	delete[] title;
	mypad->Modified();
	if (padsav)
		padsav->cd();

}
//______________________________________________________________________________

TCanvas* GVTab::GetCanvasAt(Int_t index) {
	TCanvas* canv;
	canv = NULL;
	if (GetCurrentEmbeddedCanvas())
		canv = GetEmbeddedCanvasAt(index)->GetCanvas();
	return canv;

}
//______________________________________________________________________________

TCanvas* GVTab::GetCurrentCanvas() {
	TCanvas* canv;
	TCanvas* canv2;
	canv = NULL;
	canv2 = NULL;
	canv2 = GetCurrentEmbeddedCanvas()->GetCanvas();
	if (canv2)
		canv = canv2;
	return canv;
}
//______________________________________________________________________________

TRootEmbeddedCanvas* GVTab::GetCurrentEmbeddedCanvas() {
	Int_t n = GetCurrent();
	return (GetEmbeddedCanvasAt(n));
}

//______________________________________________________________________________
TRootEmbeddedCanvas* GVTab::GetEmbeddedCanvasAt(Int_t index) {
	TRootEmbeddedCanvas *curembcanv;
	SetTab(index);
	curembcanv = NULL;
	myTGCompositeFrame* myTG;
	myTG = (myTGCompositeFrame*) GetCurrentContainer();
	if (myTG==NULL) fError.TreatError(2,0,"EmbeddedCanvas NULL!");
	curembcanv = myTG->GetTRootEmbeddedCanvas();
	return curembcanv;
}
//______________________________________________________________________________

void GVTab::RemoveTab(Int_t tabIndex) {
	TGTab::RemoveTab(tabIndex);
}

//______________________________________________________________________________

void GVTab::SetPageAction(Int_t id) {
	// Do actions when changing pages
	Vigru *f = (Vigru*) fMain;
	CdFirstOrLastPad();
	f->SetZPositionReset();
	f->SetSPositionReset();
	f->SetInfo(false);
}

//______________________________________________________________________________
void GVTab::CdFirstOrLastPad() {

	Int_t id;
	id = GetCurrent();
	TCanvas *curcanv;
	TRootEmbeddedCanvas *curembcanv;
	curcanv = NULL;
	curembcanv = NULL;
	//  if (id <0)


	{
		curembcanv = GetEmbeddedCanvasAt(id);
		if (curembcanv) {
			curcanv = (TCanvas*) curembcanv->GetCanvas();
			if (curcanv) {
				curcanv->cd(1);//select the canvas of the first tab

			}
		}
	}
}
//______________________________________________________________________________

Int_t GVTab::GetNumberOfPad(Int_t nopage) {

	TCanvas *c1 = GetCanvasAt(nopage);
	Int_t n = 0;
	//list of object present in current page
	TIter next(c1->GetListOfPrimitives());
	TObject *obj;
	//count number of pads present in current page
	while ((obj = next())) {
		if (obj->InheritsFrom("TPad"))
			n = n + 1;
	}
	return n;
}

//______________________________________________________________________________

void GVTab::RemoveCurrentPage() {
	//remove the current tab-
	Int_t current = 0;
	Int_t retval;

	if (GetNumberOfTabs() == 1)//if last page
	{
		EMsgBoxIcon icontype = kMBIconStop;
		new TGMsgBox(gClient->GetRoot(), fMain, "Warning",
				"you can't delete this page , it's your last one", icontype, 1,
				&retval);
	} else {
		EMsgBoxIcon icontype = kMBIconQuestion;
		new TGMsgBox(gClient->GetRoot(), fMain, "Warning",
				"Do you really want to delete this page and its contents?",
				icontype, 3, &retval);
		if (retval == 1) {
			current = GetCurrent();
			RemoveTab(current);
		}
	}
}
//______________________________________________________________________________
void GVTab::RefreshTab(bool with_return, bool getnewspe) {

	TVirtualPad *lastpad, *testpad;
	Int_t nopad;
	lastpad = NULL;
	GVPad * gvpad;
	GVPad * original;
	TCanvas* canvas = GetCurrentCanvas();
	original = (GVPad*) GetSelectedPad();
	nopad = 0;

	while (true) {
		nopad++;
		testpad = canvas->cd(nopad);
		canvas->Update();
		if (testpad == lastpad)
			break;
		gvpad = (GVPad*) GetSelectedPad();
		gvpad->RefreshPad(getnewspe);

		lastpad = testpad;
	}

	if (with_return)
		original->cd();
	original->Update();
}

//______________________________________________________________________________
void GVTab::ApplyZoomProportionalAllPages(Int_t xmin, Int_t xmax, Int_t ymin,
		Int_t ymax) {
	int retval = 0;
	EMsgBoxIcon icontype = kMBIconQuestion;
	new TGMsgBox(gClient->GetRoot(), this, "Warning",
			"Do you really apply X and Y sliders on All Pages?", icontype, 3,
			&retval);
	if (retval == 1) {
		SetTab(0);
		for (int i = 0; i < GetNumberOfTabs(); i++) {
			SetTab(i);
			ApplyZoomProportional(xmin, xmax, ymin, ymax);

		}
	}

}
//______________________________________________________________________________
void GVTab::ApplyLogAllPages(Bool_t logx, Bool_t logy, Bool_t logz) {

	SetTab(0);
	for (int i = 0; i < GetNumberOfTabs(); i++) {
		SetTab(i);
		ApplyPageLog(logx, logy, logz);
	}
}
//______________________________________________________________________________
void GVTab::ApplyZoomProportional(Int_t xmin, Int_t xmax, Int_t ymin,
		Int_t ymax) {
	// apply a proportional zoom all tab

	TVirtualPad *lastpad, *testpad;
	Int_t nopad;
	lastpad = NULL;
	GVPad * gvpad;
	TCanvas* canvas = GetCurrentCanvas();
	nopad = 0;
	while (true) {
		nopad++;
		testpad = canvas->cd(nopad);
		canvas->Update();
		if (testpad == lastpad)
			break;
		gvpad = (GVPad*) GetSelectedPad();
		gvpad->ApplyZoomProportional(xmin, xmax, ymin, ymax);
		lastpad = testpad;
	}
}
//______________________________________________________________________________
void GVTab::ApplySetRangeUser(bool inx,bool iny,bool inz, Double_t xmin, Double_t xmax,Double_t ymin,Double_t ymax,Double_t zmin, Double_t zmax) {
	// apply a range  all tab of page

	TVirtualPad *lastpad, *testpad;
	Int_t nopad;
	lastpad = NULL;
	GVPad * gvpad;
	TCanvas* canvas = GetCurrentCanvas();
	nopad = 0;
	while (true) {
		nopad++;
		testpad = canvas->cd(nopad);
		canvas->Update();
		if (testpad == lastpad)
			break;
		gvpad = (GVPad*) GetSelectedPad();
		gvpad->ApplySetRangeUser(inx,iny,inz,xmin,xmax,ymin,ymax, zmin, zmax);
		lastpad = testpad;
	}
}
//______________________________________________________________________________
void GVTab::ApplyPageLog(Bool_t logx, Bool_t logy, Bool_t logz) {
	// apply a log all tab

	TVirtualPad *lastpad, *testpad;
	Int_t nopad;
	lastpad = NULL;
	GVPad * gvpad;
	TCanvas* canvas = GetCurrentCanvas();
	nopad = 0;
	while (true) {
		nopad++;
		testpad = canvas->cd(nopad);
		canvas->Update();
		if (testpad == lastpad)
			break;
		gvpad = (GVPad*) GetSelectedPad();
		gvpad->ApplyLog(logx, logy, logz);
		lastpad = testpad;
	}
}
//______________________________________________________________________________
void GVTab::CancelZoom() {
	// apply a unzoom all tab

	TVirtualPad *lastpad, *testpad;
	Int_t nopad;
	lastpad = NULL;
	GVPad * gvpad;
	TCanvas* canvas = GetCurrentCanvas();
	nopad = 0;
	while (true) {
		nopad++;
		testpad = canvas->cd(nopad);
		canvas->Update();
		if (testpad == lastpad)
			break;
		gvpad = (GVPad*) GetSelectedPad();
		gvpad->CancelZoom();
		lastpad = testpad;
	}
}
//______________________________________________________________________________
void GVTab::RemovePageLog(Bool_t logx, Bool_t logy, Bool_t logz) {
	// apply a log all tab

	TVirtualPad *lastpad, *testpad;
	Int_t nopad;
	lastpad = NULL;
	GVPad * gvpad;
	TCanvas* canvas = GetCurrentCanvas();
	nopad = 0;
	while (true) {
		nopad++;
		testpad = canvas->cd(nopad);
		canvas->Update();
		if (testpad == lastpad)
			break;
		gvpad = (GVPad*) GetSelectedPad();
		gvpad->RemoveLog(logx, logy, logz);
		lastpad = testpad;
	}
}
//______________________________________________________________________________
void GVTab::PeakPageFind(Int_t NbPeaks, Float_t Resolution, Double_t Sigma,
		Double_t hreshold, bool Display_polymarker) {
	//Peak Find on all page

	TVirtualPad *lastpad, *testpad;

	Int_t nopad;
	lastpad = NULL;
	GVPad * gvpad;
	TCanvas* canvas = GetCurrentCanvas();
	nopad = 0;
	while (true) {
		nopad++;
		testpad = canvas->cd(nopad);
		canvas->Update();
		if (testpad == lastpad)
			break;
		gvpad = (GVPad*) GetSelectedPad();
		gvpad->PeakFind(NbPeaks, Resolution, Sigma, hreshold,
				Display_polymarker);
		lastpad = testpad;
	}
}
//______________________________________________________________________________
void GVTab::ApplyPageReset(bool question) {
	//Peak Find on all page

	TVirtualPad *lastpad, *testpad;

	Int_t nopad;
	lastpad = NULL;
	GVPad * gvpad;
	TCanvas* canvas = GetCurrentCanvas();
	nopad = 0;
	while (true) {
		nopad++;
		testpad = canvas->cd(nopad);
		canvas->Update();
		if (testpad == lastpad)
			break;
		gvpad = (GVPad*) GetSelectedPad();
		gvpad->ResetSpectra(question);
		lastpad = testpad;
	}
}

//______________________________________________________________________________

TPad* GVTab::GetSelectedPad() {
	//return the current seletected pad
	TPad *Selected_pad = NULL;
	Selected_pad = (TPad *) gROOT->GetSelectedPad();
	if (!(Selected_pad->InheritsFrom("TPad"))) {
		int retval = 0;
		EMsgBoxIcon icontype = kMBIconStop;
		new TGMsgBox(gClient->GetRoot(), this, "Warning", "No seleted Pad",
				icontype, 1, &retval);
	}
	return Selected_pad;
}
//______________________________________________________________________________

TPad* GVTab::GetPad(Int_t notab, Int_t nopad) {
	//return the  pad at page tab and padnumber nopad
	TCanvas *canvas = GetCanvasAt(notab);
	TPad *Selected_pad = (TPad *) canvas->GetPad(nopad);
	return Selected_pad;
}

//______________________________________________________________________________
 void GVTab::MoveToNextPad(int deplacement){

Int_t level = GetCurrent();
Int_t newpos =0;
Int_t actual = GetNoPad(level);

 newpos = actual +deplacement;
 Int_t max = GetNumberOfPad(level);
 if (newpos > max)  newpos =1;
 if (newpos<1) newpos = max;
 TPad* pad = GetPad(level,newpos);
 pad->cd();
 TCanvas *canvas = GetCanvasAt(level);
 canvas->Update();
 }

//______________________________________________________________________________

Int_t GVTab::GetNoPad(Int_t level) {

	TCanvas* canvas = GetCanvasAt(level);
	TVirtualPad *lastpad, *testpad;
	Int_t nopad;
	lastpad = NULL;
	GVPad * original;
	original = (GVPad*) GetSelectedPad();
	nopad = 0;

	while (true) {
		nopad++;
		testpad = canvas->cd(nopad);

		if (testpad == lastpad)
			break;
		if (testpad == original)
				break;

		lastpad = testpad;
	}

	original->cd();
	original->Update();


	return nopad;
}

//______________________________________________________________________________

void GVTab::SetStatOpt(TString statopt) {// set specific option on page
	TVirtualPad *lastpad, *testpad;
	Int_t nopad;
	lastpad = NULL;
	GVPad * gvpad;
	TCanvas* canvas = GetCurrentCanvas();

	nopad = 0;
	while (true) {
		nopad++;
		testpad = canvas->cd(nopad);
		canvas->Update();
		if (testpad == lastpad)
		break;
		gvpad = (GVPad*) GetSelectedPad();
		gvpad->SetStatOpt(statopt);
		lastpad = testpad;
	}
}

//______________________________________________________________________________
void GVTab::SetPageChange(TString pageName, Int_t nbPadX, Int_t nbPadY) {
	// change name and/or nb of pad
	// if nbPad =0 , no change in number of nPad

	if ((nbPadX != 0) && (nbPadY != 0)) {
		DivideCurrentPage(nbPadX, nbPadY);
	}
	if (pageName.CompareTo("") != 0)
		RenameCurrentPage(pageName);
}

//______________________________________________________________________________
void GVTab::RenameCurrentPage(TString name) {
	//rename the current page
	TGString *tempo = new TGString(name.Data());
	TGTabElement* curtab = GetCurrentTab();
	curtab->SetText(tempo);
	curtab->Resize(curtab->GetDefaultSize());

	myTGCompositeFrame* myTG;
	myTG = (myTGCompositeFrame*) GetCurrentContainer();
	myTG->SetName(name);

}
//______________________________________________________________________________
void GVTab::ComputePadXY(Int_t nbPads, Int_t *nbPadsX, Int_t *nbPadsY) {
	Int_t padmax = 10;
	switch (nbPads) {
	case 1://nothing is done: no division is needed
		break;
	case 2:
		*nbPadsX = 1;
		*nbPadsY = 2;
		break;
	case 3:
	case 4:
		*nbPadsX = 2;
		*nbPadsY = 2;
		break;
	case 5:
	case 6:
		*nbPadsX = 2;
		*nbPadsY = 3;
		break;
	case 7:
	case 8:
		*nbPadsX = 2;
		*nbPadsY = 4;
		break;
	case 9:
		*nbPadsX = 3;
		*nbPadsY = 3;
		break;
	case 10:
		*nbPadsX = 2;
		*nbPadsY = 5;
		break;
	case 11:
	case 12:
		*nbPadsX = 3;
		*nbPadsY = 4;
		break;
	case 13:
	case 14:
	case 15:
		*nbPadsX = 3;
		*nbPadsY = 5;
		break;
	case 16:
		*nbPadsX = 4;
		*nbPadsY = 4;
		break;
	case 17:
	case 18:
		*nbPadsX = 3;
		*nbPadsY = 6;
		break;
	case 19:
	case 20:
		*nbPadsX = 4;
		*nbPadsY = 5;
		break;
	case 21:
	case 22:
	case 23:
	case 24:
		*nbPadsX = 4;
		*nbPadsY = 6;
		break;
	case 25:
		*nbPadsX = 5;
		*nbPadsY = 5;
		break;
	case 26:
	case 27:
	case 28:
		*nbPadsX = 4;
		*nbPadsY = 7;
	case 29:
	case 30:
	case 31:
	case 32:
		*nbPadsX = 4;
		*nbPadsY = 8;
		break;
	case 33:
	case 34:
	case 35:
	case 36:
		*nbPadsX = 6;
		*nbPadsY = 6;
		break;
	case 37:
	case 38:
	case 39:
	case 40:
		*nbPadsX = 5;
		*nbPadsY = 8;
		break;
	case 41:
	case 42:
	case 43:
	case 44:
	case 45:
		*nbPadsX = 5;
		*nbPadsY = 9;
		break;
	case 46:
	case 47:
	case 48:
	case 49:
	case 50:
	case 51:
	case 52:
	case 53:
	case 54:
	case 55:
	case 56:
		*nbPadsX = 7;
		*nbPadsY = 8;
		break;
	case 57:
	case 58:
	case 59:
	case 60:
	case 61:
	case 62:
	case 63:
	case 64:
		*nbPadsX = 8;
		*nbPadsY = 8;
		break;
	case 65:
	case 66:
	case 67:
	case 68:
	case 69:
	case 70:
		*nbPadsX = 7;
		*nbPadsY = 10;
		break;
	case 71:
	case 72:
	case 73:
	case 74:
	case 75:
	case 76:
	case 77:
	case 78:
	case 79:
	case 80:
		*nbPadsX = 8;
		*nbPadsY = 10;
		break;
	case 81:
	case 82:
	case 83:
	case 84:
	case 85:
	case 86:
	case 87:
	case 88:
	case 89:
	case 90:
		*nbPadsX = 9;
		*nbPadsY = 10;
		break;
	case 91:
	case 92:
	case 93:
	case 94:
	case 95:
	case 96:
	case 97:
	case 98:
	case 99:
	case 100:
		*nbPadsX = 10;
		*nbPadsY = 10;
		break;
	default:
		*nbPadsX = padmax;
		*nbPadsY = padmax;
	}

}
//______________________________________________________________________________
void GVTab::CreateXML(TXMLEngine* xml, XMLNodePointer_t node, bool only_current) {
	//create configuration to a XML file

	TString tempo;
	TString tabname;
	Int_t npadx, npady;
	GVPad * gvpad;
	Int_t NumberOfTabs = GetNumberOfTabs();
	tempo.Form("%d", NumberOfTabs);
	XMLNodePointer_t tabsnode = xml->NewChild(node, 0, "Pages", tempo.Data());
	myTGCompositeFrame *myTG;
	TGTabElement* curtab;
	Int_t currenttab;
	if (only_current) {
		curtab = GetCurrentTab();
		currenttab = GetCurrent();
		tempo.Form("%d", 0);
		XMLNodePointer_t tabnode = xml->NewChild(tabsnode, 0, "Tab",
				tempo.Data());

		curtab = GetCurrentTab();

		myTG = (myTGCompositeFrame*) GetCurrentContainer();
		npadx = myTG->GetNPadx();
		npady = myTG->GetNPady();
		tabname = myTG->GetTabName();
		tabname = (curtab->GetText())->Copy();
		xml->NewChild(tabnode, 0, "TabName", tabname);
		tempo.Form("%d", npadx);
		xml->NewChild(tabnode, 0, "npadx", tempo.Data());
		tempo.Form("%d", npady);
		xml->NewChild(tabnode, 0, "npady", tempo.Data());

		for (Int_t j = 1; j <= npadx * npady; j++) {
			gvpad = (GVPad*) GetPad(currenttab, j);
			if (gvpad) {
				gvpad->CreateXML(xml, tabnode);
			}
		}

	} else {
		for (Int_t i = 0; i < NumberOfTabs; i++) {

			tempo.Form("%d", i);
			XMLNodePointer_t tabnode = xml->NewChild(tabsnode, 0, "Tab",
					tempo.Data());
			SetTab(i);

			curtab = GetCurrentTab();

			myTG = (myTGCompositeFrame*) GetCurrentContainer();
			npadx = myTG->GetNPadx();
			npady = myTG->GetNPady();
			tabname = myTG->GetTabName();
			tabname = (curtab->GetText())->Copy();
			xml->NewChild(tabnode, 0, "TabName", tabname);
			tempo.Form("%d", npadx);
			xml->NewChild(tabnode, 0, "npadx", tempo.Data());
			tempo.Form("%d", npady);
			xml->NewChild(tabnode, 0, "npady", tempo.Data());

			for (Int_t j = 1; j <= npadx * npady; j++) {
				gvpad = (GVPad*) GetPad(i, j);
				if (gvpad) {
					gvpad->CreateXML(xml, tabnode);
				}

			}
		}
	}

}
//______________________________________________________________________________

void GVTab::ReadSpectra(TFile *file) {
	//Read Spectra  in a file in root format
	//file suposed be open

	Int_t npadx, npady;
	GVPad * gvpad;

	if (!file) {
		fError.TreatError(1, 0, "Root file is not open");
		return;
	}

	Int_t NumberOfTabs = GetNumberOfTabs();

	myTGCompositeFrame *myTG;

	for (Int_t i = 0; i < NumberOfTabs; i++) {

		SetTab(i);
		myTG = (myTGCompositeFrame*) GetCurrentContainer();
		npadx = myTG->GetNPadx();
		npady = myTG->GetNPady();

		for (Int_t j = 1; j <= npadx * npady; j++) {
			gvpad = (GVPad*) GetPad(i, j);
			gvpad->cd();
			if (gvpad) {
				gvpad->ReadSpectra(file);
			}

		}
	}
}
//______________________________________________________________________________

void GVTab::SaveSpectra(TFile *file, Int_t type) {
	//Save Spectra  in a file in root format
	//file suposed be open
	// if type >=0 select specific spectra ( see GSepectrumIdentity.h)

	Int_t npadx, npady;
	GVPad * gvpad;

	if (!file) {
		fError.TreatError(1, 0, "Root file is not open");
		return;
	}
	Int_t NumberOfTabs = GetNumberOfTabs();
	myTGCompositeFrame *myTG;
	for (Int_t i = 0; i < NumberOfTabs; i++) {

		SetTab(i);
		myTG = (myTGCompositeFrame*) GetCurrentContainer();
		npadx = myTG->GetNPadx();
		npady = myTG->GetNPady();

		for (Int_t j = 1; j <= npadx * npady; j++) {
			gvpad = (GVPad*) GetPad(i, j);
			gvpad ->cd();
			if (gvpad) {
				gvpad->SaveSpectra(file, type);
			}

		}
	}
}

//______________________________________________________________________________
void GVTab::ReadXML(TXMLEngine* xml, XMLNodePointer_t node, bool question) {
	//Load configuration from a XML file
	TString TabName;
	TString nodename;
	TString nodecontent;
	TString tempo;
	Int_t  NumberOfTabs, NumberOfPads;
	NumberOfTabs = 0;
	NumberOfPads = 0;
	bool pagecreated;
	Int_t npadx, npady;
	GVPad * gvpad;

	Int_t retval;

	npadx = 0;
	npady = 0;

	pagecreated = false;

	nodecontent = xml->GetNodeContent(node);
	nodename = xml->GetNodeName(node);

	XMLNodePointer_t nodetab = xml->GetChild(node);

	if (question) {
		new GVMyQuestion(gClient->GetRoot(), this,
				"Add Config;Replace Config;Cancel", "Apply Configuration File",
				"Want do you whant to do with this configration file?", &retval);

		if (retval == 0) {
			//add
		}
		if (retval == 1) {
			for (int i = GetNumberOfTabs() - 1; i >= 0; i--) {
				SetTab((i - 1));
				RemoveTab(i);
			}
		}
		if (retval == 2) {
			return;
		}
	} else {
		for (int i = GetNumberOfTabs() - 1; i >= 0; i--) {
			SetTab((i - 1));
			RemoveTab(i);
		}
	}

	NumberOfTabs = GetNumberOfTabs();
	NumberOfTabs--;
	SetTab(NumberOfTabs);

	while (nodetab != NULL ) {

		nodecontent = xml->GetNodeContent(nodetab);
		nodename = xml->GetNodeName(nodetab);

		if (nodename == "Tab") {
			pagecreated = false;
			gvpad = NULL;
			NumberOfPads = 0;
			NumberOfTabs++;
			npady = 0;
			npadx = 0;
			XMLNodePointer_t nodetab1 = xml->GetChild(nodetab);
			while (nodetab1 != NULL) {
				nodecontent = xml->GetNodeContent(nodetab1);
				nodename = xml->GetNodeName(nodetab1);
				if (nodename == "TabName")
					TabName = nodecontent;
				if (nodename == "npadx") {
					npadx = nodecontent.Atoi();
				}
				if (nodename == "npady") {
					npady = nodecontent.Atoi();
				}
				if ((npady != 0) && (npadx != 0) && (pagecreated == false)) {
					//if (NumberOfTabs==1)
					//	SetPageChange(TabName.Data(), npadx, npady);
					//else
					{

						AddTab(TabName.Data(), npadx, npady);
					}
					SetTab(NumberOfTabs);
					pagecreated = true;
				}
				if (pagecreated) {
					if (nodename == "GVPad") {
						NumberOfPads++;
						gvpad = NULL;
						gvpad = (GVPad*) GetPad(NumberOfTabs, NumberOfPads);
						if (gvpad)
							gvpad->ReadXML(xml, nodetab1);
						else {
							tempo.Form(
									"Get pad in Tab = %d (%s)and Pad =%d, with npadx = %dand npady = %d",
									NumberOfTabs, TabName.Data(), NumberOfPads,
									npadx, npady);
							fError.TreatError(2, 0, tempo);
						}

					}

				}
				nodetab1 = xml->GetNext(nodetab1);
			}
		}//if (nodename =="Tab")
		nodetab = xml->GetNext(nodetab);
	}
}


