#include <TGMsgBox.h>
#include "../sources/General.h"
#include "Vigru.h"
#include "GVPad.h"
#include "GSpectraDB.h"
#include <TStyle.h>
#include <TXMLFile.h>
#include <TBuffer.h>
#include <TBufferXML.h>
#include <TXMLEngine.h>
#include <TDOMParser.h>
#include <TXMLNode.h>
#include <TROOT.h>

ClassImp( GVPad);

//______________________________________________________________________________
GVPad::GVPad(TGMainFrame * main, const char* name, const char* title,
		Double_t xlow, Double_t ylow, Double_t xup, Double_t yup,
		Color_t color, Short_t bordersize, Short_t bordermode) :
	TPad(name, title, xlow, ylow, xup, yup, color, bordersize, bordermode) {

	fMaino = main;
	Initialization();
}

//______________________________________________________________________________
void GVPad::Initialization() {
	//constructor
	fPadDB = new GSpectraDB();
	fOption2D = "col";
	fStatOpt = "nemri";
	fZP = 0;
	fZT = 0;
	fComputation = COMPUTATION_REFRESH;
	fDuplicationPad = 0;
	fDuplicationTab = 0;
	fToreturn = NULL;
	fDB = ((Vigru *) fMaino)->GetSpeManager()->GetDB();
	fPalette = false;
	if (!gClient->GetColorByName("white", white))
		fError.TreatError(1, 0, "White color not defined");
	if (!gClient->GetColorByName("orange", orange))
		fError.TreatError(1, 0, "Orange color not defined");
	if (!gClient->GetColorByName("black", black))
		fError.TreatError(1, 0, "Black color not defined");
	SetBit(kCannotMove, kTRUE);
	SetBorderMode(1);
}
//______________________________________________________________________________
GVPad::~GVPad() {
	//Destructor

	if (fPadDB) {
		delete (fPadDB);
		fPadDB = NULL;
	}
}

//_____________________________________________________________________________
TObjArray* GVPad::GetAllCutsFromPad() {
	//Get Cuts in Pad and return them in a TObjArray

	TObject *obj;
    Int_t size;
    TList * list;

    if (fToreturn==NULL){
    	fToreturn = new TObjArray();
    }
    fToreturn->Clear();
	list  = GetListOfPrimitives();
	size = list->GetSize();
	if (size > 0) {
		for (Int_t i = 0; i < size; i++) {
			obj = list->At(i);
			if ((TObject*) obj != NULL) {
				if (obj->InheritsFrom(TCutG::Class())) {
					fToreturn ->Add(obj);
				}
			}
		}
	}
	return fToreturn;
}
//_____________________________________________________________________________
TCutG* GVPad::GetNewCutFromPad(const char* name) {
	//Get Cut in Pad
	// first we try to get new cut
	// if not we try to get it in displayed spectrum
	TCutG *currentCutCUTG, *currentCut;
	currentCut = NULL;
    if (name == NULL) return currentCut;
	currentCutCUTG = (TCutG*) (gROOT->GetListOfSpecials()->FindObject(name));
	if (currentCutCUTG) {
		currentCut = currentCutCUTG;
	}
	return currentCut;
}
//______________________________________________________________________________
void GVPad::IntegralsInCuts() {

	if (fPadDB->GetLast() < 0)
		return;

	if (GetAllCutsFromPad()->GetSize()>0){
		fError.Infos("--------Integrals inside of cuts-------");
	fPadDB->IntegralsInCuts(GetAllCutsFromPad());

	}
	}
//______________________________________________________________________________
void GVPad::ReplaceDB(GSpectraDB * DB) {
	//Display all spectra included in DB on same pad
	if (DB->GetLast() < 0)
		return;
	fPadDB->ReplaceDB(DB);
}
//______________________________________________________________________________
void GVPad::DisplaySpectra(GSpectraDB * DB, bool getnewone) {
	//Display all spectra included in DB on same pad
	if (DB->GetLast() < 0)
		return;
	GSpectrumIdentity * id;
	Color_t color;
	color = 0;
	fPadDB->DeleteAllIdentities();

	for (Int_t i = 0; i <= DB->GetLast(); i++) {
		id = (GSpectrumIdentity*) (DB->At(i));
		//color_max = DB->IsLineColorAlreadyExiste(i);
		color = ((i + 2) % 10);
		if (color == 0)
			color = 1;
		id->SetLineColor(color);
		AddDisplaySpectrum(id, getnewone);
	}
	RefreshPad(getnewone);
}
//_
//______________________________________________________________________________
void GVPad::DisplaySpectrum(GSpectrumIdentity* id, Int_t operation) {
	//Display a new Spectrum
	bool getanewone;
	getanewone = ((operation == 0) || (operation == 1));

	if (operation == 0) {
		Clear(); // clear pad
		fPadDB->DeleteAllIdentities();
	}
	AddDisplaySpectrum(id, getanewone);
}
//______________________________________________________________________________
void GVPad::AddDisplaySpectrum(GSpectrumIdentity* identity, bool getanewone) {
	// Add a new spectra to display in pad

	Color_t color;
	GSpectrumIdentity * id;
	id = new GSpectrumIdentity(identity);
	Int_t nb = fPadDB->GetLast();
	fComputation = COMPUTATION_REFRESH;
	if (nb >= 0) {
		color = ((GSpectrumIdentity *) (fPadDB->At(nb)))->GetLineColor();
		color = ((color + 1) % 10);

		if (color == 0)
			color = 1;
		id->SetLineColor(color);
	}
	fPadDB->AddIdentity(id);
	RefreshPad(getanewone);

}

//______________________________________________________________________________
void GVPad::RefreshPad(bool getanewone) {
	//Display/refresh spectra from internal DB to pad

	Vigru * main;
	main = (Vigru *) fMaino;
	GVTab * Tabs;
	Int_t nbpad = 0;
	Int_t tab = 0;

	Tabs = main->GetTabPage();
	tab = Tabs->GetCurrent();
	nbpad = GetNumber();
	gStyle->SetOptStat(fStatOpt);
	if (nbpad == 0) {
		Int_t retval;
		EMsgBoxIcon icontype = kMBIconQuestion;
		new TGMsgBox(
				gClient->GetRoot(),
				fMaino,
				"Warning",
				"It Impossible to display histogram in back pad!\n please selecte a other one!",
				icontype, 1, &retval);
		return;
	}

	switch (fComputation) {

	case COMPUTATION_REFRESH:
		//cout << "COMPUTATION_REFRESH\n";
		fPadDB->Refresh(getanewone, tab, nbpad, fOption2D,
				main->GetSpeManager()->GetListDevice());
		break;
	case COMPUTATION_COPY:
		cout << "COMPUTATION_COPY\n";
		Copy(fDuplicationTab, fDuplicationPad);
		fPadDB->Refresh(false, tab, nbpad, fOption2D,
				main->GetSpeManager()->GetListDevice());
		break;
	case COMPUTATION_FFT:
		//cout << "COMPUTATION_FFT\n";
		CopyOnlySpectra(fDuplicationTab, fDuplicationPad);
		//CopyIfEmpty(fDuplicationTab, fDuplicationPad);
		fPadDB->FFThalf();
		fPadDB->FFT();
		fPadDB->Refresh(false, tab, nbpad, fOption2D,
				main->GetSpeManager()->GetListDevice());
		break;
	case COMPUTATION_FFThalf:
		//cout << "COMPUTATION_FFThalf\n";
		CopyOnlySpectra(fDuplicationTab, fDuplicationPad);
		//CopyIfEmpty(fDuplicationTab, fDuplicationPad);
		fPadDB->FFThalf();
		fPadDB->Refresh(false, tab, nbpad, fOption2D,
				main->GetSpeManager()->GetListDevice());
		break;
	case COMPUTATION_SCATTER:
		//cout << "COMPUTATION_SCATTER\n";
		CopyOnlySpectra(fDuplicationTab, fDuplicationPad);
		fPadDB->Scatter();
		fPadDB->Refresh(false, tab, nbpad, fOption2D,
				main->GetSpeManager()->GetListDevice());
		break;
	case COMPUTATION_ZEROLESS:
		//cout << "COMPUTATION_ZEROLESS\n";
		CopyOnlySpectra(fDuplicationTab, fDuplicationPad);
		fPadDB->ZeroLess();
		fPadDB->Refresh(false, tab, nbpad, fOption2D,
				main->GetSpeManager()->GetListDevice());
		break;
	case COMPUTATION_PROJECTIONX:
		//cout << "COMPUTATION_PROJECTIONX\n";
		CopyOnlySpectra(fDuplicationTab, fDuplicationPad);
		fPadDB->ProjectionX();
		fPadDB->Refresh(false, tab, nbpad, fOption2D,
				main->GetSpeManager()->GetListDevice());
		break;
	case COMPUTATION_PROJECTIONY:
		//cout << "COMPUTATION_PROJECTIONY\n";
		CopyOnlySpectra(fDuplicationTab, fDuplicationPad);
		fPadDB->ProjectionY();
		fPadDB->Refresh(false, tab, nbpad, fOption2D,
				main->GetSpeManager()->GetListDevice());
		break;
	case COMPUTATION_PROFILEX:
		//cout << "COMPUTATION_PROFILEX\n";
		CopyOnlySpectra(fDuplicationTab, fDuplicationPad);
		fPadDB->ProfileX();
		fPadDB->Refresh(false, tab, nbpad, fOption2D,
				main->GetSpeManager()->GetListDevice());
		break;
	case COMPUTATION_PROFILEY:
		//cout << "COMPUTATION_PROFILEY\n";
		CopyOnlySpectra(fDuplicationTab, fDuplicationPad);
		fPadDB->ProfileY();
		fPadDB->Refresh(false, tab, nbpad, fOption2D,
				main->GetSpeManager()->GetListDevice());
		break;
	case COMPUTATION_NULL:
		return;
		break;
	default:
		fError.TreatError(2, 0, "Section of computation no possible");
		return;
		break;

	}

	ResizePad();
	Update();
}
//______________________________________________________________________________
void GVPad::CopyIfEmpty(Int_t fromtab, Int_t frompad) {
	GVPad* this_on = this;

	Int_t nb = this_on->GetPadDB()->GetLast();
	cout << "Debug  CopyIfEmpty "<<nb <<"\n";
	if (this_on->GetPadDB()->GetLast()< 0){
		Copy( fromtab,  frompad);
	}else{
		CopyOnlySpectra(fromtab, frompad);
	}
}
//______________________________________________________________________________
void GVPad::Copy(Int_t fromtab, Int_t frompad) {
	// copy all identity spectra with instance of spectra  from pad ( tab, page)
	// to selected pad

	Vigru * main;
	main = (Vigru *) fMaino;
	GVPad* originPad = (GVPad*) (main->GetTabPage()->GetPad(fromtab, frompad));
	GVPad* this_on = this;
	cout << "Debug  Copy\n";
	if (originPad) {
		//originPad->GetPadDB()->MakeDUMP();
		GSpectraDB* DBlocal = originPad->GetPadDB();
		this_on->GetPadDB()->ReplaceDB(DBlocal);
		SetOption2D(originPad->GetOption2D());
		SetStatOpt(originPad->GetStatOpt(), false);
		//GetPadDB()->MakeDUMP();
	}
}
//______________________________________________________________________________
void GVPad::CopyOnlySpectra(Int_t fromtab, Int_t frompad) {
	// like Copy(Int_t tab, Int_t pad) but only instance of spectra are copied (cloned)
	Vigru * main;
	main = (Vigru *) fMaino;
	int tab = main->GetTabPage()->GetCurrent();
	GVPad* originPad = (GVPad*) (main->GetTabPage()->GetPad(fromtab, frompad));
	GVPad* this_on = this;
	cout << "Debug  CopyOnlySpectra\n";
	if (originPad) {
		cout << "Debug beforeCopyOnlySpectra\n";
		originPad->GetPadDB()->MakeDUMP();
		GSpectraDB* DBlocal = originPad->GetPadDB();
		main->GetTabPage()->SetTab(tab);
		this_on->GetPadDB()->ReplaceDBOnlySpectra(DBlocal);
		cout << "Debug afterCopyOnlySpectra\n";
		this_on->GetPadDB()->MakeDUMP();
	}else{
		main->GetTabPage()->SetTab(tab);
	}
}
//______________________________________________________________________________
void GVPad::ApplyComputation(Int_t computation, Int_t fromtab, Int_t frompad) {
	bool getnewone;
	getnewone = false;
	SetComputation(computation);
	SetDuplicationPad(frompad);
	SetDuplicationTab(fromtab);
	if (GetComputation() == COMPUTATION_REFRESH) {
		getnewone = true;
		RefreshPad(getnewone);
		return;
	}
	if (GetComputation() != COMPUTATION_NULL) {
		Copy(fromtab, frompad);
		RefreshPad(getnewone);
		return;
	}
}

//______________________________________________________________________________
void GVPad::RemoveLog(Bool_t logx, Bool_t logy, Bool_t logz) {

	if (logx) {
		SetLogx(0);
	}
	if (logy) {
		SetLogy(0);
	}
	if (logz) {
		SetLogz(0);
	}
	Update();
}
//______________________________________________________________________________
void GVPad::ApplyLog(Bool_t logx, Bool_t logy, Bool_t logz) {

	if (logx) {
		SetLogx(1);
	}
	if (logy) {
		SetLogy(1);
	}
	if (logz) {
		SetLogz(1);
	}

	Update();
}
//______________________________________________________________________________
void GVPad::ChangingLog(Bool_t logx, Bool_t logy, Bool_t logz) {

	Bool_t localx, localz, localy;
	localx = kFALSE;
	localz = kFALSE;
	localy = kFALSE;

	if (logx) {
		if ((GetLogx() == 1)) {
			localx = kFALSE;
		} else {
			localx = kTRUE;
		}
		SetLogx(int(localx));
	}
	if (logy) {
		if ((GetLogy() == 1)) {
			localy = kFALSE;
		} else {
			localy = kTRUE;
		}
		SetLogy(int(localy));
	}
	if (logz) {
		if ((GetLogz() == 1)) {
			localz = kFALSE;
		} else {
			localz = kTRUE;
		}
		SetLogz(int(localz));
	}

	Update();
}
//______________________________________________________________________________
void GVPad::Divide(Int_t nx, Int_t ny, Float_t xmargin, Float_t ymargin,
		Int_t color) {

	if (!(IsEditable()))
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
	cd();
	if (nx <= 0)
		nx = 1;
	if (ny <= 0)
		ny = 1;
	Int_t ix, iy;
	Double_t x1, y1, x2, y2;
	Double_t dx, dy;
	GVPad *gvpad;
	char *name = new char[strlen(GetName()) + 6];
	char *title = new char[strlen(GetTitle()) + 6];
	Int_t n = 0;
	if (color == 0)
		color = GetFillColor();
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
				gvpad = new GVPad(fMaino, name, name, x1, y1, x2, y2, color);
				gvpad->GetPad()->SetNumber(n);
				gvpad->GetPad()->Draw();
			}
		}
	} else {
		// special case when xmargin <= 0 && ymargin <= 0
		Double_t xl = GetPad()->GetLeftMargin();
		Double_t xr = GetPad()->GetRightMargin();
		Double_t yb = GetPad()->GetBottomMargin();
		Double_t yt = GetPad()->GetTopMargin();
		xl /= (1 - xl + xr) * nx;
		xr /= (1 - xl + xr) * nx;
		yb /= (1 - xl + xr) * ny;
		yt /= (1 - xl + xr) * ny;
		GetPad()->SetLeftMargin(xl);
		GetPad()->SetRightMargin(xr);
		GetPad()->SetBottomMargin(yb);
		GetPad()->SetTopMargin(yt);
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
				gvpad = new GVPad(fMaino, name, title, x1, y1, x2, y2);
				gvpad->GetPad()->SetNumber(number);
				gvpad->GetPad()->SetBorderMode(0);
				if (i == 0)
					gvpad->GetPad()->SetLeftMargin(xl * nx);
				else
					gvpad->GetPad()->SetLeftMargin(0);
				gvpad->GetPad()->SetRightMargin(0);
				gvpad->GetPad()->SetTopMargin(0);
				if (j == ny - 1)
					gvpad->GetPad()->SetBottomMargin(yb * ny);
				else
					gvpad->GetPad()->SetBottomMargin(0);
				gvpad->GetPad()->Draw();
			}
		}
	}
	delete[] name;
	delete[] title;
	GetPad()->Modified();
	if (padsav)
		padsav->cd();
}
//*/
//______________________________________________________________________________
void GVPad::ResetSpectra(bool with_question) {

	Vigru * main;
	TString tempo;
	bool ref;
	ref = false;
	main = (Vigru *) fMaino;
	ref = fPadDB->ResetSpectra(main, with_question);
	if (ref)
		RefreshPad();
}
//______________________________________________________________________________
void GVPad::SaveSpectra(TFile *file , Int_t type) {

	// Save spectra of DB in root format
	// a file is supposed to be open
	// type : select type of spectra (default =-1 => all)

	if (!file) {
		fError.TreatError(1, 0, " GVPad::SaveSpectra Root file is not open");
		return;
	}
	fPadDB->SaveSpectra(file,type);
}
//______________________________________________________________________________
void GVPad::ReadSpectra(TFile *file) {

	//  spectra of DB in root format
	// a file is supposed to be open

	bool ref;
	ref = false;
	if (!file) {
		fError.TreatError(1, 0, "GVPad::ReadSpectra Root file is not open");
		return;
	}
	ref = fPadDB->ReadSpectra(file);

	if (ref) {
		RefreshPad(false);
	}

}
//______________________________________________________________________________
void GVPad::SaveSpectraTxt(ofstream *file) {
	// Save spectra of DB in txt format
	// a file is supposed to be open

	fPadDB->SaveSpectraTxt(file);

}
//______________________________________________________________________________
void GVPad::SetStatOpt(TString statopt, bool doaction) {
	fStatOpt = statopt;
	if (doaction) {
		gStyle->SetOptStat(fStatOpt);
		Update();
	}
}
//______________________________________________________________________________
void GVPad::DrawPalette() {
	//palette
	TString option = "";
	if (!fPalette) {
		option = "colz";
		fPalette = true;
	} else {
		option = "col";
		fPalette = false;
	}
	fOption2D = option;
	RefreshPad(false);
}
//______________________________________________________________________________
void GVPad::DrawPanel() {
	GSpectrumIdentity * id;
	for (Int_t i = 0; i <= fPadDB->GetLast(); i++) {
		id = (GSpectrumIdentity*) fPadDB->At(i);
		id->DrawPanel();
	}
}

//______________________________________________________________________________

void GVPad::AutoZoom() {

	fPadDB->AutoZoom();
	RefreshPad(false);
	return;
}
//______________________________________________________________________________

void GVPad::FFT(bool refresh) {
	fPadDB->FFT();
	RefreshPad(false);
	return;
}
//______________________________________________________________________________

void GVPad::Scatter(bool refresh) {
	fPadDB->Scatter();
	RefreshPad(false);
	return;
}
///______________________________________________________________________________
void GVPad::ApplyZoomProportional(Int_t xmin, Int_t xmax, Int_t ymin,
		Int_t ymax) {
	// value of xmin_pad,xmax_pad,ymin_pad,ymax_pad are given between 0 and 100
         cout << " Debug GVPad ApplyZoomProportional " <<xmin<<"  "<<xmax<<"  "<<ymin<<"  "<<ymax<<"\n";
	fPadDB->ApplyZoomProportional(xmin, xmax, ymin, ymax);
	RefreshPad(false);
}
///______________________________________________________________________________
void GVPad::ApplySetRangeUser(bool inx,bool iny,bool inz, Double_t xmin, Double_t xmax,
		Double_t ymin,Double_t ymax,Double_t zmin, Double_t zmax) {

	fPadDB->ApplySetRangeUser(inx, iny, inz,  xmin,  xmax, ymin, ymax, zmin,  zmax);
	RefreshPad(false);
}
//______________________________________________________________________________
void GVPad::ApplyZoom(Int_t xmin_pad, Int_t xmax_pad, Int_t ymin_pad,
		Int_t ymax_pad) {

	fPadDB->ApplyZoom(xmin_pad, xmax_pad, ymin_pad, ymax_pad);
	RefreshPad(false);
}
//______________________________________________________________________________
void GVPad::PeakFind(Int_t NbPeaks, Float_t Resolution, Double_t Sigma,
		Double_t Threshold, bool Display_polymarker) {
	fPadDB->PeakFind(NbPeaks, Resolution, Sigma, Threshold, Display_polymarker);
	Update();
}
//______________________________________________________________________________
void GVPad::CancelZoom() {
	fPadDB->CancelZoom();
	RefreshPad(false);
}

//______________________________________________________________________________
void GVPad::CreateXML(TXMLEngine* xml, XMLNodePointer_t node) {
	//create configuration to a XML file
	TString tempos;
	tempos = "GVPad";

	XMLNodePointer_t idnode = xml->NewChild(node, 0, tempos.Data());
	xml->NewChild(idnode, 0, "Option2D", fOption2D.Data());
	xml->NewChild(idnode, 0, "StatOpt", fStatOpt.Data());
	tempos.Form("%d", (Int_t) fPalette);
	xml->NewChild(idnode, 0, "Palette", tempos.Data());
	tempos = "";
	tempos.Form("%d", (Int_t)(GetLogx()));
	xml->NewChild(idnode, 0, "Logx", tempos.Data());
	tempos = "";
	tempos.Form("%d", (Int_t)(GetLogy()));
	xml->NewChild(idnode, 0, "Logy", tempos.Data());
	tempos = "";
	tempos.Form("%d", (Int_t)(GetLogz()));
	xml->NewChild(idnode, 0, "Logz", tempos.Data());
	tempos = "";
	tempos.Form("%d", GetComputation());
	xml->NewChild(idnode, 0, "ComputationType", tempos.Data());
	tempos = "";
	tempos.Form("%d", GetDuplicationTab());
	xml->NewChild(idnode, 0, "ComputationTab", tempos.Data());
	tempos = "";
	tempos.Form("%d", GetDuplicationPad());
	xml->NewChild(idnode, 0, "ComputationPad", tempos.Data());

	fPadDB->CreateXML(xml, idnode, false);
}
//______________________________________________________________________________
void GVPad::ReadXML(TXMLEngine* xml, XMLNodePointer_t node) {
	//Load configuration from a XML file

	TString nodename;
	TString nodecontent;
	fPadDB->DeleteAllIdentities();
	nodename = xml->GetNodeName(node);
	nodecontent = xml->GetNodeContent(node);

	if (nodename == "GVPad") {
		XMLNodePointer_t basenode = xml->GetChild(node);
		while (basenode) {
			nodename = xml->GetNodeName(basenode);
			nodecontent = xml->GetNodeContent(basenode);
			if (nodename == "StatOpt") {
				fStatOpt = nodecontent;
			}
			if (nodename == "Option2D") {
				fOption2D = nodecontent;
			}
			if (nodename == "Palette") {
				fPalette = (bool) nodecontent.Atoi();
			}
			if (nodename == "ListIdSpectra") {
				fPadDB->ReadXML(xml, basenode, false);
				fPadDB->RazPointSpectra();
			}
			if (nodename == "Logx") {
				SetLogx((nodecontent.Atoi()));
			}
			if (nodename == "Logy") {
				SetLogy((nodecontent.Atoi()));
			}
			if (nodename == "Logz") {
				SetLogz((nodecontent.Atoi()));
			}
			if (nodename == "ComputationType") {
				SetComputation((nodecontent.Atoi()));
			}
			if (nodename == "ComputationTab") {
				SetDuplicationTab((nodecontent.Atoi()));
			}
			if (nodename == "ComputationPad") {
				SetDuplicationPad((nodecontent.Atoi()));
			}

			basenode = xml->GetNext(basenode);
		}

		return;
	}
}
//______________________________________________________________________________

