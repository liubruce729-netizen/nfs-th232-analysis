// File : GSpectrumIdentity.C
// Author: Luc Legeard

//////////////////////////////////////////////////////////////////////////////
//class GSpectrumIdentity
//Contains informations about a spectrum: specturm,name ,source name ,source type , family name
// a spectrum can be a TH1,Tgrah or a TGraph2D
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
 ***************************************************************************
 */
#include "GDevice.h"
#include "GListDevice.h"
#include "GSpectrumIdentity.h"
#include "GNetClientRoot.h"
#include "GClientMemory.h"
#include "TSpectrum.h"
#include <TMath.h>

#ifdef NET_LIB
#include "GNetClientSoap.h"
#include "GNetClientNarval.h"
//#define MAX_SIZE_GANIL_SPECTRUM  2097500  // 1024*1024*2 + header
#endif

#include "math.h"

ClassImp( GSpectrumIdentity)
//______________________________________________________________________

//Constructor
GSpectrumIdentity::GSpectrumIdentity(TNamed *sp, TString n, TString sName,
		TString sType, TString source, Int_t port, TString fam, Int_t nTab,
		Int_t nPad, Int_t BD_ind, bool start, TString type) :
	GBase() {
	//Constructor
	IdentityInit();
	fSpectrum = sp; //  histo object pointer
	fName = n; // Histogram Name
	fSourceName = sName; // Source name ( ex ganp891 or localhost or file.root)
	fSourceType = sType; // Source Type (gru  or soap...)
	fSource = source; //  net of file or (mem)localmemory
	fFamily = fam; // family name
	fNumTab = nTab; // nb of tab where histogram is displayed
	fNumPad = nPad; // nb of pad where histogram is displayed
	fSpeInd = 0; // index , unique number to identify histogram for a specific application
	fDBInd = BD_ind; // index inside DB (given by DB.At() )
	fPort = port; //  no port of tcpip in case of network source
	fStarted = start; // flag to informe if the histogram is in state stop or started
	fLineColor = 2;
	fType = type;
}

//______________________________________________________________________________

GSpectrumIdentity::GSpectrumIdentity(TString sName, TString sType,
		TString source, Int_t port) :
	GBase() {
	//Constructor use to define servers id
	IdentityInit();
	fSpectrum = NULL; // bidon
	fName = "SERVER"; // not used
	fSourceName = sName; // Source name ( ex ganp891 or localhost or file.root)
	fSourceType = sType; // Source Type (gru or ganil or soap...)
	fFamily = ""; // not used
	fNumTab = 0; // not used
	fNumPad = 0; // not used
	fSpeInd = 0; //  not used
	fDBInd = 0; // not used
	fSource = source; //  net of file or mem (local memory)
	fPort = port; //  no port of tcpip in case of network source
	fStarted = false; // not used
	fLineColor = -1;
	fTypeSpe = SERVERTYPE;
}

//______________________________________________________________________________

GSpectrumIdentity::GSpectrumIdentity(GSpectrumIdentity* id,
		bool with_instance_ofspectrum) :
	GBase() {
	//Constructor by copy
	CopyFrom(id, with_instance_ofspectrum);

}

//______________________________________________________________________________

void GSpectrumIdentity::CopyFrom(GSpectrumIdentity* id,
		bool with_instance_ofspectrum) {
	// copy information of an other  GSpectrumIdentity
	IdentityInit();
	if (with_instance_ofspectrum) {
		if ((id->fSpectrum)) {
			fSpectrum = (TNamed *) ((id->fSpectrum)->Clone());
			if (fSpectrum == NULL) {
				fError.TreatError(1, 0, "CopyFrom : Spectrum not cloned ");
			}
		}
	}
	fName = id->fName;
	fSourceName = id->fSourceName;
	fSourceType = id->fSourceType;
	fSource = id->fSource;
	fServerIdent = id->fServerIdent;
	fPort = id->fPort;
	fFamily = id->fFamily;
	fNumTab = id->fNumTab;
	fNumPad = id->fNumPad;
	fStarted = id->fStarted;
	fLocSameEvt = id->fLocSameEvt;
	fSpeInd = id->fSpeInd;
	fDBInd = id->fDBInd;
	fXmin = id->fXmin;
	fXmax = id->fXmax;
	fYmin = id->fYmin;
	fYmax = id->fYmax;
	fDrawStyle = id->fDrawStyle;
	fAction = id->fAction;
	fTypeSpe = id->fTypeSpe;
	fType = id->fType;
	fNbBits = id->fNbBits;
	fLineColor = id->fLineColor;
}
//______________________________________________________________________________
GSpectrumIdentity::GSpectrumIdentity() :
	GBase() {
	IdentityInit();
}

//______________________________________________________________________________

void GSpectrumIdentity::IdentityInit() {
	// Common initialization of constructor
	fSpectrum = NULL; // Graphic object pointer
	fName = ""; // Histogram Name
	fSourceName = ""; // Source name ( ex ganp891 or localhost or file.root)
	fSourceType = ""; // Source Type (gru or ganil or soap)
	fSource = ""; //  net of file or mem (local memory)
	fServerIdent = "";//
	fPort = 0; //  no port of tcpip in case of network source
	fFamily = ""; // family name
	fNumTab = -1; // nb of tab where histogram is displayed
	fNumPad = -1; // nb of pad where histogram is displayed
	fStarted = false; // flag to inform if the histogram is in state stop or started
	fLocSameEvt = false;//flag to validate the fact that a the spectra must have the same event statistic
	fSpeInd = 0; // index ,unique number to identify histogram for a specific application
	fDBInd = 0; // index inside DB (given by DB.At() )
	fXmin = 0; // in case of zoom memorization
	fXmax = 0; // in case of zoom memorization
	fYmin = 0; // in case of zoom memorization
	fYmax = 0; // in case of zoom memorization
	fType = ""; // type of Spectra ( TH1S, INT16....)
	fDrawStyle = "";
	fLineColor = 2;
	fAction = false;// flag to notice that a action will be done on this spectra (visualization....)
	fTypeSpe = USERTYPE; // flag to identify type of histogram ( 0 = no type or user, 1 = Raw, 2,...)
	fNbBits = -1; //

	int i = 0;
	fNbEltInId = 24;
	list_info = new const char*[fNbEltInId];
	list_server_info = new bool[fNbEltInId];

	list_info[i] = "HistoPoint";
	list_server_info[i++] = false;

	list_info[i] = "HistoName ";
	list_server_info[i++] = false;

	list_info[i] = "SourceName";
	list_server_info[i++] = true;

	list_info[i] = "SourceType";
	list_server_info[i++] = true;

	list_info[i] = "  Source  ";
	list_server_info[i++] = true;

	list_info[i] = "ServerIden";
	list_server_info[i++] = true;

	list_info[i] = "   Port   ";
	list_server_info[i++] = true;

	list_info[i] = "  Family  ";
	list_server_info[i++] = false;

	list_info[i] = "  Tab     ";
	list_server_info[i++] = false;

	list_info[i] = "  Pad     ";
	list_server_info[i++] = false;

	list_info[i] = " Started  ";
	list_server_info[i++] = false;

	list_info[i] = "LocSameE ";
	list_server_info[i++] = false;

	list_info[i] = "   Xmin  ";
	list_server_info[i++] = false;

	list_info[i] = "   Xmax  ";
	list_server_info[i++] = false;
	list_info[i] = "   Ymin  ";
	list_server_info[i++] = false;
	list_info[i] = "   Ymax  ";
	list_server_info[i++] = false;
	list_info[i] = " SpeIndex ";
	list_server_info[i++] = false;
	list_info[i] = "  DBIndex ";
	list_server_info[i++] = false;
	list_info[i] = "   Action ";
	list_server_info[i++] = false;
	list_info[i] = "TypeHisto ";
	list_server_info[i++] = false;
	list_info[i] = "  Type    ";
	list_server_info[i++] = false;
	list_info[i] = "  NbBits  ";
	list_server_info[i++] = false;
	list_info[i] = "DrawStyle ";
	list_server_info[i++] = false;
	list_info[i] = "ColorLine ";
	list_server_info[i++] = false;

	fVerbose = 0;
}

//______________________________________________________________________________
//destructor
GSpectrumIdentity::~GSpectrumIdentity() {


	if (fVerbose > 5)
		cout << " Debug : Delete  GSpectrumIdentity\n";
	if (list_info){
		delete [] list_info;
		list_info=NULL;
		}
	if (list_server_info){
		delete [] list_server_info;
		list_server_info=NULL;
		}
}

//______________________________________________________________________________

void GSpectrumIdentity::DeleteSpectrumInstance() {
	// delete instance of histogram but keep entry in DB and ans its specifications

	//if (GetSource() != "MEM") {
	//a ete commenté car dans le MEM on fait quand meme le clonage du spectre (3/2016)

	if (fSpectrum) {
		delete (fSpectrum); 
		fSpectrum = NULL;
	}
	//}
}
//______________________________________________________________________________
void GSpectrumIdentity::DeleteRawSpectrumInstance() {
	// Delete Raw histo 
	if (fTypeSpe == RAWTYPE) {
		DeleteSpectrumInstance();
	} 
}
//______________________________________________________________________________
void GSpectrumIdentity::ChangeCutSpectrum(TNamed* spectrum) {
	//Change Spectrum in case of spectrum is a cut
	// instance is not deleted
	//
	int np, npspectrum;
	if ((spectrum != NULL)) {
		if ((GetSpectrum() != NULL)) {
			if ((spectrum->InheritsFrom(TCutG::Class()))
					& (GetSpectrum()->InheritsFrom(TCutG::Class()))) {
				npspectrum = ((TCutG*) spectrum)->GetN();
				np = ((TCutG*) GetSpectrum())->GetN();
				for (int i = 1; i < np; i++)
					((TCutG*) GetSpectrum())->RemovePoint(i);
				((TCutG*) GetSpectrum())->Set(npspectrum);
				for (int i = 0; i < npspectrum; i++)
					((TCutG*) GetSpectrum())->SetPoint(i,
							(((TCutG*) spectrum)->GetX())[i],
							(((TCutG*) spectrum)->GetY())[i]);
			} else {
				fError.TreatError(1, 0,
						"GSpectrumIdentity::ChangeCutSpectrum: spectrum is not a TCutG.");
			}

		}
	}
}
//______________________________________________________________________________
void GSpectrumIdentity::ChangeSpectrum(TNamed* spectrum) {
	//Change SpectrumCopyFrom

	if ((spectrum != NULL)) {
		if ((spectrum->InheritsFrom("TCutG")) && ((GetSpectrum() != NULL))) {
			ChangeCutSpectrum(spectrum);
			//delete(spectrum);
			//spectrum =NULL;
		} else {
			TString name = GetSpectrumName();
			DeleteSpectrumInstance();
			spectrum ->SetName(name);
			SetSpectrum(spectrum);
		}
	} else {
		TString name = GetSpectrumName();
		DeleteSpectrumInstance();
		//spectrum ->SetName(name);
		SetSpectrum(spectrum);
	}
}
//______________________________________________________________________________
void GSpectrumIdentity::ChangeSpectrumOnServer() {

	//Change Spectrum On Server

	GNetClientRoot * newclientroot;
	newclientroot = NULL;
#ifdef NET_LIB
	GNetClientSoap* newclientsoap;
	newclientsoap=NULL;
#endif
	TString SourceType = GetSourceType();
	TString SourceName = GetSourceName();
	GSpectrumIdentity* id;
	id = (GSpectrumIdentity*) this;

	if ((GetSource() == "NET")) {
		if (SourceType == "GRU") {
			newclientroot = new GNetClientRoot((char*) SourceName.Data());
			if (newclientroot) {
				newclientroot->SendSpectrum(id);
			}
		}
#ifdef NET_LIB
		if (SourceType=="SOAP") {
			newclientsoap = new GNetClientSoap((char*)SourceName.Data());
			if (newclientsoap)
			fError.TreatError(1, 0,
					"Impossible to SendSpectrum with Soap .");
			//newclientsoap->SendSpectrum(id);
		}
#endif

		if (SourceType == "GANIL") {
			fError.TreatError(1, 0,
					"Impossible to Change a histogram which comes form Ganil Server.");
		}
	}
	if (GetSource() == "MEM") {
		fError.TreatError(1, 0,
				"Impossible to Change a histogram which comes form a file.");
	}

	if ((GetSource() == "FILE")) {
		fError.TreatError(1, 0,
				"Impossible to Change a histogram which comes form a file.");
	}

	if (newclientroot)
		delete (newclientroot);

#ifdef NET_LIB
	if (newclientsoap)
	delete (newclientsoap);
#endif
}
//______________________________________________________________________________
Color_t GSpectrumIdentity::GetLineColor(bool with_save) {
	// Get color line of spectrum
	Color_t color;
	color = -1;

	if (fSpectrum && with_save) {
		color = SaveLineColor();
		if (color != fLineColor)
			fLineColor = color;
	} else {
		color = fLineColor;
	}
	return color;
}
//______________________________________________________________________________
void GSpectrumIdentity::SetSpectrum(TNamed* sp) {
	fSpectrum = (TNamed*) sp;
	SetType("");
}
//______________________________________________________________________________
void GSpectrumIdentity::SetType(TString type) {
	if (type == "") {
		if (fSpectrum) {
			if (fSpectrum->InheritsFrom(TObject::Class())) {
				fType = fSpectrum ->ClassName();
			}
		} else {
			fType = type;
		}

	}
}
//______________________________________________________________________________

Color_t GSpectrumIdentity::SaveLineColor() {
	// Save  color line of spectrum in identity
	Color_t color;
	color = -1;
	if (fSpectrum) {
		if (fSpectrum->InheritsFrom(TH1::Class())) {
			color = ((TH1*) fSpectrum)->GetLineColor();
		}
		if (fSpectrum->InheritsFrom(TGraph::Class())) {
			color = ((TGraph*) fSpectrum)->GetLineColor();
		}
		if (fSpectrum->InheritsFrom(TGraph2D::Class())) {
			color = ((TGraph2D*) fSpectrum)->GetLineColor();
		}
		fLineColor = color;
	}
	return color;
}
//______________________________________________________________________________

void GSpectrumIdentity::SetLineColor(Color_t color) {
	// set color line of spectrum

	if (color > 0) {
		fLineColor = color;
		if (fSpectrum) {
			if (fSpectrum->InheritsFrom(TH1::Class())) {
				((TH1*) fSpectrum)->SetLineColor(color);
			}
			if (fSpectrum->InheritsFrom(TGraph::Class())) {
				((TGraph*) fSpectrum)->SetLineColor(color);
			}
			if (fSpectrum->InheritsFrom(TGraph2D::Class())) {
				((TGraph2D*) fSpectrum)->SetLineColor(color);
			}
		}
	}
}
//______________________________________________________________________________

void GSpectrumIdentity::SaveDims() {
	int dimension = GetDimension();
	
	if (!fSpectrum) return;
	if ((fSpectrum->InheritsFrom(TH1::Class())) and (dimension==1)){
		fYmin = ((TH1*) fSpectrum)->GetMinimum();
		fYmax = ((TH1*) fSpectrum)->GetMaximum();
	}else{		
		fYmin = GetYaxis()->GetFirst();
		fYmax = GetYaxis()->GetLast();	
	}
	
	fXmin = GetXaxis()->GetFirst();
	fXmax = GetXaxis()->GetLast();
			
}
//______________________________________________________________________________

void GSpectrumIdentity::SaveSpectrum(TFile *file, Int_t type) {
	// Save spectum of DB in root format
	// a file is supposed to be open
	// type : select specific type , default = -1 => all type

	bool test = true;
	if (!file) {
		fError.TreatError(1, 0, "Root file is not open");
		return;
	}
	Int_t localtype = GetTypeSpe();
	if (type >= 0) {
		if (type != localtype)
			return;
	}
	file->cd();
	TNamed* sp;
	TString family;
	TString currentfamily, nextcurrentfamily;
	TString subfamily;
	TObjArray * listefamily;
	family = "";
	subfamily = "";
	listefamily = NULL;
	family = GetFamily();
	currentfamily = family.Remove(TString::kBoth, '/');
	listefamily = currentfamily.Tokenize("/");
	currentfamily = "";
	TDirectory *testdir;

	for (int i = 0; i <= listefamily->GetLast(); i++) {
		subfamily = (listefamily->At(i))->GetName();
		nextcurrentfamily = currentfamily + "/" + subfamily;

		testdir = gDirectory ->GetDirectory(subfamily.Data());

		if (testdir) {
			testdir->cd();
		} else {

			//fprintf(stderr,"Missing directory analyzeHiMassTau\n");
			//}


			//test = gDirectory ->cd(subfamily.Data());
			//if (!test) {
			gDirectory->mkdir(subfamily.Data());
			test = gDirectory ->cd(subfamily.Data());
			if (!test) {
				fError.TreatError(2, 0,
						"No creation of subdirectory in TFile :", subfamily);
				fError.TreatError(2, 0,
						"May be a directoy and a spectrum have same name");
			}
			currentfamily = nextcurrentfamily;
		}
	}

	sp = GetSpectrum();
	if (sp)
		sp->Write();
	file->cd();
	listefamily->SetOwner();
	if (listefamily)
		delete (listefamily);
	return;
}
//______________________________________________________________________________

Bool_t GSpectrumIdentity::ReadSpectrum(TFile *file, Int_t type) {
	// Read spectrum  from file in  root format
	// a file is supposed to be open
	// return true if read is a success
	// type of spectra to read if <0 => all

	Bool_t ref;
	ref = kFALSE;
	bool test = kTRUE;
	TString tempos;
	if (!file) {
		fError.TreatError(1, 0, "Root file is not open");
		return ref;
	}
	Int_t localtype = GetTypeSpe();
	if (type >= 0) {
		if (type != localtype)
			return false;
	}
	file->cd();
	TNamed* sp, *sp2;
	TObject * spo = NULL;
	sp = NULL;
	sp2 = NULL;
	TString family, currentfamily;
	family = GetFamily();
	currentfamily = family.Remove(TString::kBoth, '/');
	currentfamily = "/" + currentfamily;
	test = file ->cd(family.Data());
	// debug file->ls();
	TString fullname;

    if (family ==""){
    	fullname =  GetSpectrumName();
    }else{
    	fullname = family + "/" + GetSpectrumName();
    }

	tempos = "No directory  in TFile " + fullname;
	if (!test) {
		fError.TreatError(2, 0, tempos);
		return ref;
	}

	file->GetObject(fullname.Data(), spo);
	sp = (TNamed*) spo;

	if (sp) {
		TString name = sp->GetName();
		sp->SetName("bidonamealors"); // we change name to avoid conflict
		gROOT->cd();
		sp2 = (TNamed*) (sp->Clone(name));
		if (sp2 == NULL) {
			fError.TreatError(1, 0, "Spectrum not cloned ");
		}
		ChangeSpectrum(sp2);
		file->cd();
		ref = kTRUE;
	} else {
		tempos = "GSpectrumIdentity::ReadSpectrum :No object "
				+ GetSpectrumName() + " in TFile : " + file->GetName()
				+ " and in directory " + currentfamily;
		fError.TreatError(2, 0, tempos);
	}
	return ref;
}
//______________________________________________________________________________
Int_t GSpectrumIdentity::TestServer() {

	TString name = GetSpectrumName();
	TString sourcename = GetSourceName();
	TString sourcetype = GetSourceType();
	TString source = GetSource();
	Int_t port = GetPort();
	Int_t retour = -1;

	if ((CompareWordsIgnoreCase(&sourcetype, "GRU"))
			&& (CompareWordsIgnoreCase(&source, "NET"))) {
		GNetClientRoot test;
		test.SetDevice(sourcename.Data());
		test.SetPort(port);
		test.Open();
		if (test.TestServer()) {
			retour = 1;
		} else {
			retour = 0;
		}
		test.Close();
	}
	if ((CompareWordsIgnoreCase(&source, "FILE"))) {
		struct stat FileStat;
		if (stat(sourcename.Data(), &FileStat) >= 0) {
			retour = 1;
		} else {
			retour = 0;
		}
	}
	if (CompareWordsIgnoreCase(&sourcetype, "MEM")) {
		retour = 1;
	}
#if NET_LIB
	if((CompareWordsIgnoreCase(&sourcetype,"SOAP"))&&(CompareWordsIgnoreCase(&source,"NET"))) {
		GNetClientSoap test;
		test.SetDevice( fSourceName.Data());
		test.SetPort(fPort);

		if (test.TestServer()) {
			retour =1;

		} else {
			retour =0;

		}
	}

#endif
	return retour;
}
//______________________________________________________________________________
void GSpectrumIdentity::SaveSpectrumTxt(ofstream *file) {
	// Save spectrum of DB in txt format
	// file is supposed to be open

	Double_t xbin, xmin, xmax, ybin, ymin, ymax;
	Double_t* val;
	Int_t dimension;
	Int_t j, i;
	Double_t x, y, z;

	xbin = 0;
	xmin = 0;
	xmax = 0;
	ybin = 0;
	ymin = 0;
	ymax = 0;

	if (fSpectrum) {
		*file << GetSpectrumName() << "\n";
		dimension = GetDimension();
		*file << "// " << dimension << "\n";

		if (fSpectrum->InheritsFrom(TH1::Class())) {
			TH1* sp;
			sp = ((TH1*) fSpectrum);
			if (dimension == 1) {
				xbin = sp->GetNbinsX();
				xmin = sp->GetXaxis()->GetXmin();
				xmax = sp->GetXaxis()->GetXmax();

				*file << xbin << "\n";
				*file << xmin << "\n";
				*file << xmax << "\n";
				for (i = 0; i < xbin; i++) {
					x = i;
					y = sp->GetBinContent(i);
					*file << x << "  " << y << "\n";
				}
			}
			if (dimension == 2) {
				xbin = sp->GetNbinsX();
				ybin = sp->GetNbinsY();
				xmin = sp->GetXaxis()->GetXmin();
				xmax = sp->GetXaxis()->GetXmax();
				ymin = sp->GetYaxis()->GetXmin();
				ymax = sp->GetYaxis()->GetXmax();
				*file << xbin << "\n";
				*file << xmin << "\n";
				*file << xmax << "\n";
				*file << ybin << "\n";
				*file << ymin << "\n";
				*file << ymax << "\n";

				for (i = 0; i < xbin; i++) {
					for (j = 0; j < ybin; j++) {
					}
					x = i;
					y = i;
					z = sp->GetBinContent(i, j);
					*file << x << "  " << y << "  " << z << "\n";
				}
			}

		}

		if (fSpectrum->InheritsFrom(TGraph::Class())) {
			TGraph* sp;
			sp = (TGraph*) fSpectrum;
			xbin = sp->GetN();
			xmin = sp->GetXaxis()->GetXmin();
			xmax = sp->GetXaxis()->GetXmax();
			*file << xbin << "\n";
			*file << xmin << "\n";
			*file << xmax << "\n";
			for (i = 0; i < xbin; i++) {
				val = (sp->GetX()) + i;
				sp->GetPoint(i, x, y);
				*file << x << "  " << y << "\n";
			}
		}

		if (fSpectrum->InheritsFrom(TGraph2D::Class())) {
			TGraph2D* sp;
			sp = (TGraph2D*) fSpectrum;
			xbin = sp->GetN();
			xmin = sp->GetXmin();
			ymin = sp->GetYmin();
			xmax = sp->GetXmax();
			ymax = sp->GetYmax();
			*file << xbin << "\n";
			*file << xmin << "\n";
			*file << xmax << "\n";
			*file << ybin << "\n";
			*file << ymin << "\n";
			*file << ymax << "\n";
			for (i = 0; i < xbin; i++) {
				val = (sp->GetX()) + i;
				x = *val;
				val = (sp->GetY()) + i;
				y = *val;
				val = (sp->GetY()) + i;
				z = *val;
				*file << x << "  " << y << "  " << z << "\n";
			}
		}
	}

}
//______________________________________________________________________________

bool GSpectrumIdentity::ReadSpectrumTxt(ifstream *file, Int_t textlines) {

	// Read spectrum from txt format
	// file is supposed to be open
	// exemple of file textlines =4
	/*
	 StartWave : 16384 samples
	 8660
	 8660
	 8660
	 8660
	 8659
	 8659
	 8661
	 8661
	 8660
	 */
	string mystring;
	Bool_t ref;
	ref = kFALSE;
	bool test = kTRUE;
	TString tempos;
	if (!file) {
		fError.TreatError(1, 0, "Root file is not open");
		return ref;
	}
	TNamed* sp;
	sp = NULL;

	TString family, currentfamily;
	family = GetFamily();
	currentfamily = family.Remove(TString::kBoth, '/');
	currentfamily = "/" + currentfamily;
	test = gDirectory ->cd(currentfamily.Data());

	tempos = "No directory : " + currentfamily;
	if (!test) {
		fError.TreatError(2, 0, tempos);
		return ref;
	}

	gDirectory->GetObject(GetSpectrumName(), sp);

	DeleteSpectrumInstance();
	Int_t i;

	Int_t size = 0;
	Int_t entier;
	entier = 0;
	for (i = 0; i < textlines; i++) {
		*file >> mystring;
		cout << mystring << "\n";
	}
	while (file->good()) {
		*file >> entier;
		cout << entier << "\n";
		size++;
	}
	size--;
	cout << " Size trace fichier TXT =" << size << "\n";

	file->clear();
	file->seekg(0, ios::beg);

	for (i = 0; i < textlines; i++) {
		*file >> mystring;
		cout << mystring << "\n";
	}
	sp = new TH1I("histo_txt", "histo_txt", size, 0, size);
	for (i = 0; i < size; i++) {
		*file >> entier;
		//cout <<entier<<"\n";
		((TH1I*) sp)-> SetBinContent(i + 1, entier);
	}
	((TH1I*) sp)->SetEntries(size);
	fSpectrum = sp;
	ref = true;
	return ref;

}

//______________________________________________________________________________

Bool_t GSpectrumIdentity::HasProperties(TString name, TString source,
		TString sourceName, TString sourceType, Int_t port, TString family) {
	return this->fName == name && this->fSource == source && this->fPort
			== port && this->fSourceName == sourceName && this->fSourceType
			== sourceType && this->fFamily == family;
}

//______________________________________________________________________________


TString GSpectrumIdentity::ToString() {
	TString separator = " ; ";
	TString tip = GetSpectrumName(); //0
	tip += separator;
	tip += GetSourceName(); //1
	tip += separator;
	tip += GetSourceType();//2
	tip += separator;
	tip += GetFamily();//3
	tip += separator;
	tip += GetDBIndex(); //4
	return tip;
}

//______________________________________________________________________________

TString GSpectrumIdentity::DumpHeader(Int_t nb) {
	//Dump all header infomation about each element contained in Identity.
	//nb is number max of value to diplay
	//all information are retuned in a TString
	// if "server" is true just server information are dumped
	bool server = (fTypeSpe==SERVERTYPE);
	TString st;
	TString tempos;
	Int_t i;
	if (nb == 0)
		nb = fNbEltInId;
	st = "|";
	for (i = 0; i < nb; i++) {
		//cout <<" Debug "<<i<< "  " << list_server_info[i]<<"  "<<list_info[i]<<endl;
		if (list_server_info[i] || !server) {
			tempos = list_info[i];
			st += tempos + "|";
		}
	}
	st += "\n";
	return (st);
}

//______________________________________________________________________________

TString GSpectrumIdentity::DumpId(Int_t nb) {
	//Dump all values of each element contained in Identity.
	//nb is number max of value to display
	//all information are retuned in a TString
	// if "server" is true just server information are dumped

	stringstream st;
	bool server = (fTypeSpe==SERVERTYPE);
	TString tst;
	TString tempos;
	if (nb == 0)
		nb = fNbEltInId;
	st << "|";
	Int_t i = 0;
	if (nb >= i && (list_server_info[i] || !server)) {
		st << hex << "0x";
		st << (Long64_t)(GetSpectrum());
		st << dec;
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetSpectrumName();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetSourceName();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetSourceType();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetSource();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetServerIdent();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetPort();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetFamily();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetNumTab();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetNumPad();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetStarted();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetLocSameEvt();
		st << "|";
	}

	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetXmin();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetXmax();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetYmin();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetYmax();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetSpeIndex();
		st << "|";
	}
	if (nb >= i++ || (list_server_info[i] || !server)) {
		st << GetDBIndex();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetAction();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetTypeSpe();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetType();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetNbBits();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetDrawStyle();
		st << "|";
	}
	if (nb >= i++ && (list_server_info[i] || !server)) {
		st << GetLineColor(false);
		st << "|";
	}

	st << "\n";
	tst = st.str();
	return (tst);

}

//______________________________________________________________________________

void GSpectrumIdentity::CreatXML(TXMLEngine* xml, XMLNodePointer_t node){
	//convert all values of each element contained in Identity in XML format
	
	bool server = (fTypeSpe==SERVERTYPE);
	// if "server" is true just server informations are converted
	TString tempo;
	if (server)
		tempo = "ServerIdentity";
	else
		tempo = "SpectrumIdentity";

	XMLNodePointer_t idnode = xml->NewChild(node, 0, tempo.Data());
	tempo = "";
	//tempo.Form("%lld", (Long64_t)(GetSpectrum()));
	tempo.Form("%lld", (Long64_t)(0));
	if (!server)
		xml->NewChild(idnode, 0, "HistoPoint", tempo.Data());
	if (!server)
		xml->NewChild(idnode, 0, "HistoName", GetSpectrumName());
	xml->NewChild(idnode, 0, "SourceName", GetSourceName());
	xml->NewChild(idnode, 0, "SourceType", GetSourceType());
	xml->NewChild(idnode, 0, "Source", GetSource());
	xml->NewChild(idnode, 0, "ServerIdent", GetServerIdent());
	tempo = "";
	tempo.Form("%d", (GetPort()));
	xml->NewChild(idnode, 0, "Port", tempo.Data());
	if (!server)
		xml->NewChild(idnode, 0, "Family", GetFamily());
	tempo = "";
	tempo.Form("%d", (Int_t)(GetNumTab()));
	if (!server)
		xml->NewChild(idnode, 0, "Tab", tempo.Data());
	tempo = "";
	tempo.Form("%d", (Int_t)(GetNumPad()));
	if (!server)
		xml->NewChild(idnode, 0, "Pad", tempo.Data());
	tempo = (Int_t)(GetStarted());
	if (!server)
		xml->NewChild(idnode, 0, "Started", tempo.Data());
	tempo = "";
	tempo.Form("%d", (Int_t)(GetSpeIndex()));
	if (!server)
		xml->NewChild(idnode, 0, "SpeIndex", tempo.Data());
	tempo = "";
	tempo.Form("%d", (Int_t)(GetDBIndex()));
	if (!server)
		xml->NewChild(idnode, 0, "DBIndex", tempo.Data());
	tempo = "";
	tempo.Form("%d", (Int_t)(GetXmin()));
	if (!server)
		xml->NewChild(idnode, 0, "Xmin", tempo.Data());
	tempo = "";
	tempo.Form("%d", (Int_t)(GetXmax()));
	if (!server)
		xml->NewChild(idnode, 0, "Xmax", tempo.Data());
	tempo = "";
	tempo.Form("%d", (Int_t)(GetYmin()));
	if (!server)
		xml->NewChild(idnode, 0, "Ymin", tempo.Data());
	tempo = "";
	tempo.Form("%d", (Int_t)(GetYmax()));
	if (!server)
		xml->NewChild(idnode, 0, "Ymax", tempo.Data());
	tempo = "";
	tempo.Form("%d", (Int_t)(GetAction()));
	if (!server)
		xml->NewChild(idnode, 0, "Action", tempo.Data());
	tempo = "";
	tempo.Form("%d", (Int_t)(GetTypeSpe()));
	if (!server)
		xml->NewChild(idnode, 0, "TypeHisto", tempo.Data());
	if (!server)
		xml->NewChild(idnode, 0, "Type", GetType().Data());
	tempo = "";
	tempo.Form("%d", (Int_t)(GetNbBits()));
	if (!server)
		xml->NewChild(idnode, 0, "NbBits", tempo.Data());
	tempo = "";
	if (!server)
		xml->NewChild(idnode, 0, "Style", GetDrawStyle());
	tempo = "";
	tempo.Form("%d", (Int_t) GetLineColor(false));
	if (!server)
		xml->NewChild(idnode, 0, "LineColor", tempo.Data());
}

//______________________________________________________________________________

void GSpectrumIdentity::ReadXML(TXMLEngine* xml, XMLNodePointer_t node) {
	//convert all values of each element contained in Identity in XML format
	// if "server" is true just server informations are converted
	TString NodeName;
	TString Content;
	bool server;
	IdentityInit();
	server = false;
	NodeName = xml->GetNodeName(node);
	if (NodeName == "ServerIdentity")
		server = true;
	Int_t count;
	count = 0;
	XMLNodePointer_t idnode = xml->GetChild(node);
	while (idnode != NULL) {
		NodeName = xml->GetNodeName(idnode);

		Content = xml->GetNodeContent(idnode);

		if (NodeName == "HistoPoint") {
			if (!server) {
				long long conv = (Content.Atoi());
				SetSpectrum((TNamed*) conv);
			}
		}
		if (NodeName == "HistoName") {
			if (!server)
				SetSpectrumName(Content);
		}
		if (NodeName == "SourceName")
			SetSourceName(Content);
		if (NodeName == "SourceType")
			SetSourceType(Content);
		if (NodeName == "Source")
			SetSource(Content);
		if (NodeName == "ServerIdent")
			SetServerIdent(Content);
		if (NodeName == "Port") {
			SetPort(Content.Atoi());
		}
		if (NodeName == "Family") {
			if (!server)
				SetFamily(Content);
		}
		if (NodeName == "Tab") {
			if (!server)
				SetNumTab(Content.Atoi());
		}
		if (NodeName == "Pad") {
			if (!server)
				SetNumPad(Content.Atoi());
		}
		if (NodeName == "Started") {
			if (!server)
				SetStarted((Int_t)(Content.Atoi() != 0));
		}
		if (NodeName == "SpeIndex") {
			if (!server)
				SetSpeIndex(Content.Atoi());
		}
		if (NodeName == "DBIndex") {
			if (!server)
				SetDBIndex(Content.Atoi());
		}
		if (NodeName == "Xmin") {
			SetXmin((Content.Atoi()));
		}
		if (NodeName == "Xmax") {
			SetXmax((Content.Atoi()));
		}
		if (NodeName == "Ymin") {
			if (!server)
				SetYmin((Content.Atoi()));
		}
		if (NodeName == "Ymax") {
			if (!server)
				SetYmax((Content.Atoi()));
		}
		if (NodeName == "Action") {
			if (!server)
				SetAction((Content.Atoi()));
		}
		if (NodeName == "TypeHisto") {
			if (!server)
				SetTypeSpe((enum TypeSpe)(Content.Atoi()));
		}
		if (NodeName == "Type") {
			if (!server)
				SetType(Content);
		}
		if (NodeName == "NbBits") {
			if (!server)
				SetNbBits((Content.Atoi()));
		}
		if (NodeName == "Style") {
			if (!server)
				SetDrawStyle(Content);
		}
		if (NodeName == "LineColor") {
			if (!server) {
				SetLineColor((Content.Atoi()));
			}
		}
		idnode = xml->GetNext(idnode);
		if (count++ > fNbEltInId)
			break;
	}
}
//______________________________________________________________________________
void GSpectrumIdentity::SetFamily(TString family) {
	// format Famly to avoid multiple "/"
	// and set it in GSpectrumIdentity object
	//TString tempo;
        char * tempo2;
	family.ReplaceAll("//", "/");
	family.ReplaceAll("///", "/");
	tempo2 = RemoveChar((char*) family.Data(), (char*) "/", false);
	
	fFamily = tempo2;
	delete [] tempo2;

}
//______________________________________________________________________________
void GSpectrumIdentity::Reset() {
	if (fSpectrum) {

		if (fSpectrum->InheritsFrom(TH1::Class()))
			((TH1*) fSpectrum)->Reset();
		/*
		 if (fSpectrum->InheritsFrom(TGraph::Class())) {
		 Int_t n, j;
		 TGraph* sp;

		 sp = (TGraph*) fSpectrum;
		 n = sp->GetN();
		 for (j = 0; j < n; j++) {
		 Int_t i = j;

		 sp ->SetPoint(i, i, 0);
		 }
		 }

		 if (fSpectrum->InheritsFrom(TGraph2D::Class())) {
		 TGraph2D* sp;

		 sp = (TGraph2D*) fSpectrum;
		 Int_t n = sp->GetN();
		 for (Int_t i = 0; i < n; i++) {
		 sp ->SetPoint(i, i, i, 0);
		 }
		 }
		 */
	}
}
//______________________________________________________________________________
Int_t GSpectrumIdentity::GetDimension() {
	//return dimension
	// 0 if spectrum is not instancied
	// 1 if TH1 or TGraph
	// 2 if TH2 of TGraph2D

	Int_t n;
	n = 0;
	if (fSpectrum) {

		if (fSpectrum->InheritsFrom(TH1::Class()))
			n = ((TH1*) fSpectrum)->GetDimension();

		if (fSpectrum->InheritsFrom(TGraph::Class()))
			n = 1;

		if (fSpectrum->InheritsFrom(TGraph2D::Class()))
			n = 2;

	}
	return n;
}
//______________________________________________________________________________
void GSpectrumIdentity::FillHisto(UShort_t value) {
	// if Spectrum is TH1 -> fill histo with value
	TNamed* th;
	th = GetSpectrum();

	if (th) {

		if (fSpectrum->InheritsFrom(TH1::Class()))
			if (GetStarted()) {
				((TH1*) th)->Fill(value);
			}
	}
}
//______________________________________________________________________________
void GSpectrumIdentity::ConstructRawHisto() {
	// Construct histo in case of Raw spectra ( type histo =1)
	// if Spectrum is null
	
	if ((fTypeSpe == RAWTYPE) && (fSpectrum == NULL)) {
		fSpectrum = new TH1I(fName, fName, fNbBits, 0, fNbBits);
		SetType("");
		Reset();
	} 
}

//______________________________________________________________________________
void GSpectrumIdentity::SetStarted(Bool_t start) {
	// set flag fStarted to start.
	fStarted = start;
}
//______________________________________________________________________________
Float_t GSpectrumIdentity::GetRatio() {

	Float_t ratio;
	ratio = 0;
	TNamed* spe = GetSpectrum();
	if (spe == NULL)
		return ratio;
	if (spe->InheritsFrom(TH1::Class())) {
		TH1* h1 = (TH1*) spe;
		ratio = (Float_t)(h1->GetXaxis()->GetXmax())
				- (h1->GetXaxis()->GetXmin()) / (h1->GetXaxis()->GetNbins());
	}

	if (spe->InheritsFrom(TGraph::Class())) {
		TGraph* gr = (TGraph*) spe;
		ratio = (Float_t)(gr->GetXaxis()->GetXmax())
				- (gr->GetXaxis()->GetXmin()) / (gr->GetXaxis()->GetNbins());
	}

	if (spe->InheritsFrom(TGraph2D::Class())) {
		TGraph2D* gr = (TGraph2D*) spe;
		ratio = (Float_t)(gr->GetXaxis()->GetXmax())
				- (gr->GetXaxis()->GetXmin()) / (gr->GetXaxis()->GetNbins());
	}
	return ratio;
}
//______________________________________________________________________________
Float_t GSpectrumIdentity::GetXminAxis() {
	if (!fSpectrum)return 0;
	return GetXaxis()->GetXmin();
}
//______________________________________________________________________________
Float_t GSpectrumIdentity::GetXmaxAxis() {
	if (!fSpectrum)return 0;
	return GetXaxis()->GetXmax();
}
//______________________________________________________________________________
Float_t GSpectrumIdentity::GetXaxisFirst() {
	if (!fSpectrum)return 0;
	return GetXaxis()->GetFirst();
}
//______________________________________________________________________________
Float_t GSpectrumIdentity::GetXaxisLast() {
	if (!fSpectrum)return 0;
	return GetXaxis()->GetLast();
}
//______________________________________________________________________________
void GSpectrumIdentity::PeakFind(Int_t nb_expected_peaks, Float_t resolution,
		Double_t sigma, Double_t threshold, bool display_polymarker) {
	// try to find peaks in spectrum
	// nb_expected_peak : nb max of pics to find
	// resolution :determines resolution of the neighboring peaks default value is 1 correspond to 3 sigma distance between peaks. Higher values allow higher resolution (smaller distance between peaks. May be set later through SetResolution.
	// sigma: sigma of searched peaks, for details we refer to manual
	// threshold: (default=0.05) peaks with amplitude less than threshold*highest_peak are discarded. 0<1
	// display_polymarker : Display or not polymarker ( polymarker= object is created and added to the list of functions of the histogram. The histogram is drawn with the specified option and the polymarker object drawn on top of the histogram. The polymarker coordinates correspond to the npeaks peaks found in the histogram.)

	TH1* histo;

	if (fSpectrum == NULL) {
		fError.TreatError(1, 0, "PeakFind impossible, Pointer is null");
		return;
	}

	if (fSpectrum->InheritsFrom(TH1::Class())) {
		// recherche des pics
		histo = (TH1*) fSpectrum;
		if (histo-> GetDimension() == 1) {

			Float_t tempof;

			char * option = (char*) "";
			if (!display_polymarker)
				option = (char*) "goff";

			TSpectrum equation((nb_expected_peaks) * 2, resolution);

			Int_t nfound = 0;

			nfound = equation.Search(histo, sigma, option, threshold);

			float pics_in_order[nfound];
			float pics_in_orderY[nfound];

			if (nfound != (nb_expected_peaks)) {

			}

			for (Int_t entier = 0; entier < nfound; entier++) {
				pics_in_order[entier] = equation.GetPositionX()[entier];
				pics_in_orderY[entier] = equation.GetPositionY()[entier];
			}

			for (Int_t entier = 0; entier < nfound; entier++) {
				for (Int_t entier1 = entier; entier1 < nfound; entier1++) {
					if (pics_in_order[entier] > pics_in_order[entier1]) {
						tempof = pics_in_order[entier];
						pics_in_order[entier] = pics_in_order[entier1];
						pics_in_order[entier1] = tempof;
						tempof = pics_in_orderY[entier];
						pics_in_orderY[entier] = pics_in_orderY[entier1];
						pics_in_orderY[entier1] = tempof;
					}
				}
			}
			TString tempo;
			tempo.Form(" %d Peaks found for %s", nfound,
					GetSpectrumName().Data());
			fError.TreatError(0, 0, tempo);

			for (Int_t entier = 0; entier < nfound; entier++) {
				cout << "Peak no:" << entier << "=> Xvalue= "
						<< pics_in_order[entier] << " Yvalue= "
						<< pics_in_orderY[entier] << "\n";
			}

		} else {
			fError.TreatError(1, 0, "PeakFind impossible, TH1 dimension != 1");
		}
	} else {
		fError.TreatError(1, 0, "PeakFind impossible, It isn't a TH1");
	}

}

//______________________________________________________________________________
void GSpectrumIdentity::FitPanel() {
	if (!fSpectrum)
		return;
	if (fSpectrum->InheritsFrom(TH1::Class())) {
		TH1* h1 = (TH1*) fSpectrum;
		h1->FitPanel();
	}

	if (fSpectrum->InheritsFrom(TGraph::Class())) {
		TGraph* tg = (TGraph*) fSpectrum;
		tg->FitPanel();
	}

	if (fSpectrum->InheritsFrom(TGraph2D::Class())) {
		TGraph2D* tg = (TGraph2D*) fSpectrum;
		tg->FitPanel();
	}
	return;
}
//______________________________________________________________________________
void GSpectrumIdentity::DrawPanel() {
	if (!fSpectrum)
		return;
	if (fSpectrum->InheritsFrom(TH1::Class())) {
		TH1* h1 = (TH1*) fSpectrum;
		h1->DrawPanel();
	}

	if (fSpectrum->InheritsFrom(TGraph::Class())) {
		TGraph* tg = (TGraph*) fSpectrum;
		tg->DrawPanel();
	}

	if (fSpectrum->InheritsFrom(TGraph2D::Class())) {
		fError.TreatError(1, 0, "No Draw Panel for TGraph2D");
	}
	return;
}

//______________________________________________________________________________
Float_t GSpectrumIdentity::GetMaximumBin() {
	Float_t ret;
	ret = 0;
	if (!fSpectrum)
		return ret;
	if (fSpectrum->InheritsFrom(TH1::Class())) {
		TH1* h1 = (TH1*) fSpectrum;
		ret = h1->GetMaximumBin();
	}

	if (fSpectrum->InheritsFrom(TGraph::Class())) {
		//TGraph* tg= (TGraph*)fSpectrum;
		//ret=tg->GetMaximumBin();
		ret = 0;
	}

	if (fSpectrum->InheritsFrom(TGraph2D::Class())) {
		//TGraph2D* tg= (TGraph2D*)fSpectrum;
		//ret=tg->GetMaximumBin();
		ret = 0;
	}
	return ret;
}
//______________________________________________________________________________
void GSpectrumIdentity::Scatter() {
	TH1 * clone;
	TString name;
	TString name2;
	TString title;
	if (!fSpectrum)
		return;
	if (fSpectrum->InheritsFrom(TH1::Class())) {
		title = ((TH1*) fSpectrum)->GetTitle();
		name = ((TH1*) fSpectrum)->GetName();
		clone = (TH1*) ScatterSpectrum((TH1*) fSpectrum);
		if (!clone) {
			fError.TreatError(2, 0, "Scatter() did not succed.");
			return;
		}
		name2 = name;
		ChangeSpectrum(clone);
		if (!(title.Contains("_Scatter"))) {
			title = title + "_Scatter";
		}
		((TH1*) fSpectrum)->SetTitle(title);
	} else {
		fError.TreatError(1, 0,
				"Scatter() is not available for TGraph,  TGrap2D...");
	}

}
//______________________________________________________________________________
TH1F* GSpectrumIdentity::ScatterSpectrum(TH1* sp) {
	TH1F * new_sp;
	Float_t ymin, ymax, value, amplitude;
	ymin = 0;
	ymax = 0;
	new_sp = NULL;
	if (!sp)
		return new_sp;
	if (GetDimension() != 1) {
		fError.TreatError(1, 0, "Scatter impossible Dimension != 1");
		return new_sp;
	}

	ymin = (Float_t)(sp->GetBinContent(1));
	ymax = ymin;

	for (Int_t i = 1; i <= sp->GetNbinsX(); i++) {
		value = (Float_t) sp->GetBinContent(i);
		if (ymin > value)
			ymin = (Double_t) value;
		if (ymax < value)
			ymax = (Double_t) value;
	}
	amplitude = ymax - ymin;
	new_sp = new TH1F((GetSpectrumName()).Data(), (GetSpectrumName()).Data(),
			sp->GetNbinsX(), ymin - amplitude / 10 - 2, ymax + amplitude / 10
					+ 2);
	new_sp->Reset();
	for (Int_t i = 1; i <= sp->GetNbinsX(); i++) {
		value = (Float_t) sp->GetBinContent(i);
		new_sp->Fill(value);

	}
	return new_sp;
}
//______________________________________________________________________________
void GSpectrumIdentity::FFT() {
	TH1 * clone;
	TString name;
	TString namefft;
	TString title;

	if (!fSpectrum)
		return;
	if (fSpectrum->InheritsFrom(TH1::Class())) {
		title = ((TH1*) fSpectrum)->GetTitle();
		name = ((TH1*) fSpectrum)->GetName();
		clone = ((TH1*) fSpectrum)->FFT(NULL, "MAG");
		if (!clone) {
			fError.TreatError(2, 0, "FFT() did not succed.");
			return;
		}
		namefft = name;
		ChangeSpectrum(clone);
		if (!(title.Contains("_FFT"))) {
			title = title + "_FFT";
		}
		((TH1*) fSpectrum)->SetTitle(title);
		Int_t nbbin = ((TH1*) fSpectrum)->GetNbinsX();
		((TH1*) fSpectrum)->SetEntries(nbbin);
	} else {
		fError.TreatError(1, 0,
				"FFT() is not available for TGraph,  TGrap2D...");
	}

}
//______________________________________________________________________________
void GSpectrumIdentity::FFThalf() {
	// give only positive part of fft and set O in x=0

	if (!fSpectrum)
		return;

	if (fSpectrum->InheritsFrom(TH1::Class())) {
		FFT();
		Int_t Ny, Nx, Nz;
		TH1* h = (TH1*) fSpectrum;
		Double_t xmin, xmax, ymin, ymax, zmin, zmax;
		Nx = h->GetNbinsX();
		Ny = h->GetNbinsY();
		Nz = h->GetNbinsZ();

		xmin = h->GetXaxis()->GetXmin();
		xmax = h->GetXaxis()->GetXmax();

		ymin = h->GetYaxis()->GetXmin();
		ymax = h->GetYaxis()->GetXmax();
		zmin = h->GetZaxis()->GetXmin();
		zmax = h->GetZaxis()->GetXmax();
		h->GetXaxis()->Set(Nx / 2, xmin, (xmax + xmin) / 2);
		h->GetYaxis()->Set(Ny / 2, ymin, (ymax + ymin) / 2);
		h->GetZaxis()->Set(Nz / 2, zmin, (zmax + zmin) / 2);

		h->SetEntries(Nx);
		ZeroLess();
	} else {
		fError.TreatError(1, 0,
				"FFThalf() is not available for TGraph,  TGrap2D...");
	}
}
//______________________________________________________________________________
void GSpectrumIdentity::ZeroLess() {

	if (!fSpectrum)
		return;
	if (fSpectrum->InheritsFrom(TH1::Class())) {
		((TH1*) fSpectrum)->SetBinContent(1, 1, 0);
	} else {
		fError.TreatError(1, 0,
				"ZeroLess() is not available for TGraph,  TGrap2D...");
	}
}

//______________________________________________________________________________
void GSpectrumIdentity::ProjectionX() {

	TH1 * clone;
	TString name;
	TString name2;
	TString title;
	Int_t min, max;
	if (!fSpectrum)
		return;

	if (fSpectrum->InheritsFrom(TH2::Class())) {
		title = ((TH2*) fSpectrum)->GetTitle();
		name = ((TH2*) fSpectrum)->GetName();
		min = ((TH2*) fSpectrum)->GetYaxis()->GetFirst();
		max = ((TH2*) fSpectrum)->GetYaxis()->GetLast();
		max = ((TH2*) fSpectrum)->GetYaxis()->GetNbins();
		name2 = name + "_px";

		clone = ((TH2*) fSpectrum)->ProjectionX(name2.Data(), min, max);
		if (!clone) {
			fError.TreatError(2, 0, "ProjectionX() did not succeed.");
			return;
		}
		ChangeSpectrum(clone);
		if (!(title.Contains("_px"))) {
			title = title + "_px";
		}
		((TH1*) fSpectrum)->SetTitle(title);
	} else {
		fError.TreatError(1, 0,
				"ProjectionX() is not available for TH1, TGraph,  TGrap2D...");
	}

}
//______________________________________________________________________________
void GSpectrumIdentity::ProjectionY() {

	TH1 * clone;
	TString name;
	TString name2;
	TString title;
	Int_t min, max;
	if (!fSpectrum)
		return;
	if (fSpectrum->InheritsFrom(TH2::Class())) {
		title = ((TH2*) fSpectrum)->GetTitle();
		name = ((TH2*) fSpectrum)->GetName();
		min = ((TH2*) fSpectrum)->GetXaxis()->GetFirst();
		max = ((TH2*) fSpectrum)->GetXaxis()->GetLast();
		name2 = name + "_py";

		clone = ((TH2*) fSpectrum)->ProjectionY(name2.Data(), min, max);
		if (!clone) {
			fError.TreatError(2, 0, "ProjectionY() did not succeed.");
			return;
		}
		ChangeSpectrum(clone);
		if (!(title.Contains("_py"))) {
			title = title + "_py";
		}
		((TH1*) fSpectrum)->SetTitle(title);

	} else {
		fError.TreatError(1, 0,
				"ProjectionY() is not available for TH1, TGraph,  TGrap2D...");
	}
}
//______________________________________________________________________________
void GSpectrumIdentity::ProfileX() {
	TH1 * clone;
	TString name;
	TString name2;
	TString title;
	Int_t min, max;
	if (!fSpectrum)
		return;
	if (fSpectrum->InheritsFrom(TH2::Class())) {
		title = ((TH2*) fSpectrum)->GetTitle();
		name = ((TH2*) fSpectrum)->GetName();
		min = ((TH2*) fSpectrum)->GetYaxis()->GetFirst();
		max = ((TH2*) fSpectrum)->GetYaxis()->GetLast();
		max = ((TH2*) fSpectrum)->GetYaxis()->GetNbins();
		name2 = name + "_pfx";

		clone = (TH1*) (((TH2*) fSpectrum)->ProfileX(name2.Data(), min, max));
		if (!clone) {
			fError.TreatError(2, 0, "ProfileX() did not succeed.");
			return;
		}
		ChangeSpectrum(clone);
		if (!(title.Contains("_pfx"))) {
			title = title + "_pfx";
		}
		((TH1*) fSpectrum)->SetTitle(title);

	} else {
		fError.TreatError(1, 0,
				"ProfileX() is not available for TH1, TGraph,  TGrap2D...");
	}

}
//______________________________________________________________________________
void GSpectrumIdentity::ProfileY() {
	TH1 * clone;
	TString name;
	TString name2;
	TString title;
	Int_t min, max;
	if (!fSpectrum)
		return;
	if (fSpectrum->InheritsFrom(TH2::Class())) {
		title = ((TH2*) fSpectrum)->GetTitle();
		name = ((TH2*) fSpectrum)->GetName();
		min = ((TH2*) fSpectrum)->GetXaxis()->GetFirst();
		max = ((TH2*) fSpectrum)->GetXaxis()->GetLast();
		name2 = name + "_pfx";

		clone = (TH1*) (((TH2*) fSpectrum)->ProfileY(name2.Data(), min, max));
		if (!clone) {
			fError.TreatError(2, 0, "ProfileY() did not succeed.");
			return;
		}

		ChangeSpectrum(clone);
		if (!(title.Contains("_pfy"))) {
			title = title + "_pfy";
		}
		((TH1*) fSpectrum)->SetTitle(title);

	} else {
		fError.TreatError(1, 0,
				"ProfileY() is not available for TH1, TGraph,  TGrap2D...");
	}

}
//______________________________________________________________________________
void GSpectrumIdentity::IntegralsInCuts(TCutG* cut) {
	Double_t integral;
	Double_t meanx, meany;
	if (fSpectrum == NULL)
		return;
	if (fSpectrum->InheritsFrom(TH2::Class())) {
		integral = cut->IntegralHist((TH2*) fSpectrum);
		meanx = MeanHist(cut, 1);
		meany = MeanHist(cut, 2);
		TString tempos;
		tempos.Form("%s[%s]  S = %lf   Mx = %lf  My = %lf", fName.Data(),
				(cut->GetName()), integral, meanx, meany);
		fError.Infos(tempos);
	}
}
//______________________________________________________________________________
Double_t GSpectrumIdentity::MeanHist(TCutG* cut, Int_t xory) {
	// Compute mean x (if xory=1) or mean y (if xory=2) inside graphical cut
	Double_t mean = 0;
	Double_t axisvalue = 0;
	Double_t sum = 0;
	Double_t content = 0;
	if ((xory != 1) && (xory != 2))
		xory = 1;
	TH2*h = (TH2*) fSpectrum;
	if (!cut)
		return mean;
	if (!h)
		return mean;
	Int_t n = cut->GetN();
	Double_t xmin = 1e200;
	Double_t xmax = -xmin;
	Double_t ymin = xmin;
	Double_t ymax = xmax;
	for (Int_t i = 0; i < n; i++) {
		if (((cut->GetX())[i]) < xmin)
			xmin = ((cut->GetX())[i]);
		if (((cut->GetX())[i]) > xmax)
			xmax = ((cut->GetX())[i]);
		if (((cut->GetY())[i]) < ymin)
			ymin = ((cut->GetY())[i]);
		if (((cut->GetY())[i]) > ymax)
			ymax = ((cut->GetY())[i]);
	}
	TAxis *xaxis = h->GetXaxis();
	TAxis *yaxis = h->GetYaxis();
	Int_t binx1 = xaxis->FindBin(xmin);
	Int_t binx2 = xaxis->FindBin(xmax);
	Int_t biny1 = yaxis->FindBin(ymin);
	Int_t biny2 = yaxis->FindBin(ymax);
	Int_t nbinsx = h->GetNbinsX();

	// Loop on bins for which the bin center is in the cut

	Int_t bin, binx, biny;
	for (biny = biny1; biny <= biny2; biny++) {
		Double_t y = yaxis->GetBinCenter(biny);
		for (binx = binx1; binx <= binx2; binx++) {
			Double_t x = xaxis->GetBinCenter(binx);
			if (!(cut->IsInside(x, y)))
				continue;
			bin = binx + (nbinsx + 2) * biny;
			if (xory == 1)
				axisvalue = x;
			if (xory == 2)
				axisvalue = y;
			content = h->GetBinContent(bin);
			//         integral += h->GetBinContent(bin)*axisvalue;
			mean += content * axisvalue;
			sum += content;
		}
	}
	return (mean / sum);
}
//______________________________________________________________________________
TNamed * GSpectrumIdentity::GetSpectrumNetOrFileOrMem(GListDevice* listdev) {
	TNamed *sp = NULL;

	if (GetSource() == "NET") {
		sp = GetSpectrumNet(listdev);
		return (sp);
	}
	if (GetSource() == "FILE") {
		sp = GetSpectrumFile(listdev);
		return (sp);
	}

	if (GetSource() == "MEM") {
		sp = GetSpectrumMem(listdev);
		return (sp);
	}
	fError.TreatError(1, 0, "Impossible Get Spectrum net file or mem .");
	return (sp);
}

//______________________________________________________________________________

TNamed * GSpectrumIdentity::GetSpectrumNet(GListDevice* listdev) {
	//return a net spectrum

	TString SourceType;
	SourceType = GetSourceType();

	TNamed *sp;

	sp = NULL;

	if (SourceType == "GRU") {
		if (listdev->GetNetClientRoot() == NULL)
			listdev->SetNetClientRoot(new GNetClientRoot((char*) "localhost"));
		((GNetClientRoot*) (listdev->GetNetClientRoot()))->GetSpectrum(this);
		sp = GetSpectrum();
		listdev->SetNetClientRoot(listdev->GetNetClientRoot());
	}

#ifdef  NET_LIB
	else {

		if (SourceType=="SOAP") {
			if (listdev->GetNetClientSoap()==NULL)
			listdev->SetNetClientSoap(new GNetClientSoap((char*)"localhost"));
			listdev->SetMyDevice(listdev->GetNetClientSoap());
			((GNetClientSoap*)listdev->GetMyDevice())->GetSpectrum(this);
			sp=GetSpectrum();

		}
	}
#endif
	return sp;
}

//______________________________________________________________________________
TNamed * GSpectrumIdentity::GetSpectrumFile(GListDevice* listdev) {

	TFile *mfile = NULL;
	mfile = new TFile(GetSourceName());
	ReadSpectrum(mfile);
	return (GetSpectrum());
}
//______________________________________________________________________________
TNamed * GSpectrumIdentity::GetSpectrumMem(GListDevice* listdev) {

	TString SourceType;
	SourceType = GetSourceType();

	TNamed * sp;
	sp = NULL;

	if (listdev->GetClientMemory() == NULL)
		listdev->SetClientMemory(new GClientMemory((char*) "localhost"));
	listdev->SetMyDevice(listdev->GetClientMemory());

	((GClientMemory*) (listdev->GetMyDevice()))->GetSpectrum(this);
	sp = GetSpectrum();
	return sp;
}
//______________________________________________________________________________
void GSpectrumIdentity::GaussBg2(TF1 *fitfunc, Double_t gLowX, Double_t gUpX) {
	if (!fSpectrum)
		return;
	if (fSpectrum->InheritsFrom(TH1::Class())) {
		GaussBg2_h(fitfunc, gLowX, gUpX);
	} else {
		fError.TreatError(1, 0,
				"GaussBg2 is not available for TGraph,  TGrap2D...");
	}

}
//______________________________________________________________________________

Double_t gBinW;

Double_t GaussBg(Double_t *x, Double_t *par) {

	// par[0]  gauss constant
	// par[1]  gauss mean
	// par[2]  gauss width
	// par[3]  bg constant
	// par[4] bg slope


	static Float_t sqrt2pi = TMath::Sqrt(2 * TMath::Pi());
	static Float_t sqrt2 = TMath::Sqrt(2.);
	Double_t arg;
	arg = (x[0] - par[1]) / (sqrt2 * par[2]);
	Double_t fitval = gBinW / (sqrt2pi * par[2]) * par[0] * TMath::Exp(-arg
			* arg);
	fitval = fitval + par[3] + par[4] * x[0];
	return fitval;
}
//______________________________________________________________________________
void GSpectrumIdentity::GaussBg2_h(TF1 *fitfunc, Double_t gLowX, Double_t gUpX) {

	TH1F* hist = (TH1F *) fSpectrum;

	if (gLowX < gUpX) {

		// *** Obtaining and specifying the start values for the fit ***

		Double_t gContent = 0;
		Double_t gMean = 0;
		Double_t gSigma = 0;

		Double_t gBgconst = 0;
		Double_t gBgslope = 0;
		//  Double_t gBinW =0;

		gContent = hist->Integral(hist->FindBin(gLowX), hist->FindBin(gUpX));
		gMean = 0.5 * (gLowX + gUpX);
		gSigma = 0.3 * (gUpX - gLowX);
		gBgconst = 1;
		gBgslope = 0;
		gBinW = hist->GetBinWidth(1);

		cout << "________________________\n";
		cout << "The Start Values \n";
		cout << "Bin Width: " << gBinW << "\n";
		cout << "Content: " << gContent << "\n";
		cout << "Mean Value: " << gMean << "\n";
		cout << "Sigma: " << gSigma << " \n";
		cout << "Bg constant: " << gBgconst << "\n";
		cout << "Bg slope: " << gBgslope << "\n";
		cout << "________________________\n";

		fitfunc->SetParameters(gContent, gMean, gSigma);
		fitfunc->SetRange(gLowX, gUpX);
		fitfunc->SetLineColor(2);
		fitfunc->SetParName(0, "Content    ");
		fitfunc->SetParName(1, "Mean       ");
		fitfunc->SetParName(2, "Sigma      ");
		fitfunc->SetParName(3, "Bg constant");
		fitfunc->SetParName(4, "Bg slope   ");
	} else
		cout
				<< "Couldn't fit! Error: The Lower Limit is larger than the Upper Limit!"
				<< endl;
}

//______________________________________________________________________________
void GSpectrumIdentity::FitBG() {
	gBinW = 0;
	Float_t ratio = GetRatio();
	if (ratio == 0)
		return;
	TF1 *fitfunc = new TF1("fitfunc", GaussBg, 0, 1, 5);
	Float_t xmin = (GetXaxisFirst()) * ratio + GetXminAxis();
	Float_t xmax = (GetXaxisLast()) * ratio + GetXminAxis();

	//Float_t xmin=(h1->GetXaxis()->GetFirst()) *ratio +h1->GetXaxis()->GetXmin();
	//Float_t xmax=(h1->GetXaxis()->GetLast())*ratio+ h1->GetXaxis()->GetXmin();

	cout << " xmin : " << xmin << "     xmax : " << xmax << " -  "
			<< GetXaxisFirst() << " - " << GetXaxisLast() << "\n";

	GaussBg2(fitfunc, xmin, xmax);
	fitfunc->SetRange(xmin, xmax);
	Float_t maxX = (GetMaximumBin()) * ratio + GetXminAxis();
	Float_t large = (Int_t)(xmax - xmin) / 4;//arbitraire!!

	//cout << " Max on : "<< maxX  << "  Largeur :"<<large<< "\n";
	double maxX2;
	maxX2 = maxX;
	fitfunc->SetParameter((int) large, maxX2);
	fitfunc->SetLineColor(4);

	Fit("fitfunc", "R", "same");

	if (fitfunc)
		delete (fitfunc);

}
//______________________________________________________________________________
void GSpectrumIdentity::Fit(TF1 *f1, Option_t *option, Option_t *goption,
		Axis_t xmin, Axis_t xmax) {

	if (fSpectrum->InheritsFrom(TH1::Class())) {
		TH1* h1 = (TH1*) fSpectrum;
		h1->Fit(f1, option, goption, xmin, xmax);
	}

	if (fSpectrum->InheritsFrom(TGraph::Class())) {
		TGraph* tg = (TGraph*) fSpectrum;
		tg->Fit(f1, option, goption, xmin, xmax);

	}

	if (fSpectrum->InheritsFrom(TGraph2D::Class())) {
		TGraph2D* tg = (TGraph2D*) fSpectrum;
		tg->Fit(((TF2*) f1), option, goption);

	}

}
//______________________________________________________________________________
void GSpectrumIdentity::Fit(const char *formula, Option_t *option,
		Option_t *goption, Axis_t xmin, Axis_t xmax) {

	if (fSpectrum->InheritsFrom(TH1::Class())) {
		TH1* h1 = (TH1*) fSpectrum;
		h1->Fit(formula, option, goption, xmin, xmax);
	}

	if (fSpectrum->InheritsFrom(TGraph::Class())) {
		TGraph* tg = (TGraph*) fSpectrum;
		tg->Fit(formula, option, goption, xmin, xmax);
	}

	if (fSpectrum->InheritsFrom(TGraph2D::Class())) {
		TGraph2D* tg = (TGraph2D*) fSpectrum;
		tg->Fit(formula, option, goption);
	}

}
//______________________________________________________________________________
void GSpectrumIdentity::RestaureOption() {
	if (fSpectrum) {
		SetLineColor(fLineColor);
		ReportXYMinMax();
	}
}
//______________________________________________________________________________
void GSpectrumIdentity::SaveOption() {
	if (fSpectrum) {
		SaveDims();
		SaveLineColor();
	}
}
//______________________________________________________________________________
void GSpectrumIdentity::MyDraw(bool same, TString option2D) {
	TString opt;
	TString opt_tot;
	opt = "";

	TString opt_same;
	opt_same = "";

	if (fSpectrum) {
		RestaureOption();

		if (fSpectrum->InheritsFrom(TH1::Class())) {
			if (same)
				opt_same = "SAME";

			TH1* histo = (TH1*) fSpectrum;

			if (histo-> GetDimension() > 1) {
				opt = histo->GetDrawOption();
				opt_tot = opt + option2D + opt_same;
				histo->Draw(opt_tot);
			} else {
				histo->GetXaxis()-> SetTickLength(0.02);
				histo->GetYaxis()-> SetTickLength(0.02);
				opt = histo->GetDrawOption();
				opt_tot = opt + opt_same;
				histo->Draw(opt_tot);
			}
		}

		if (fSpectrum->InheritsFrom(TGraph::Class())) {
			if (!same)
				opt_same = "A";
			TGraph* tg = (TGraph*) fSpectrum;
			tg->SetTitle((tg->GetName()));
			opt = tg->GetDrawOption();
			opt_tot = opt + "L" + " " + opt_same;
			tg->Draw(opt_tot);
		}

		if (fSpectrum->InheritsFrom(TGraph2D::Class())) {
			if (!same)
				opt_same = "A";
			TGraph2D* tg = (TGraph2D*) fSpectrum;

			tg->SetTitle((tg->GetName()));
			opt = tg->GetDrawOption();
			opt_tot = opt + option2D + "L" + opt_same;
			tg->Draw(opt_tot);
		}
	} else {
		fError.TreatError(2, 0,
				"No Instance of spectrum to display in GSpectrumIdentity::MyDraw");
	}

}
//______________________________________________________________________________
void GSpectrumIdentity::ReportXYMinMax() {

	if (fSpectrum) {

		if (fSpectrum->InheritsFrom(TH1::Class())) {
			TH1* histo = (TH1*) fSpectrum;

			Int_t xmin = GetXmin();
			Int_t xmax = GetXmax();
			Int_t ymin = GetYmin();
			Int_t ymax = GetYmax();
			//Int_t zmin = GetZmin();
			//Int_t zmax = GetZmax();
			//Int_t xmin2 = histo->GetXaxis()->GetXmin();
			//Int_t xmax2 = histo->GetXaxis()->GetXmax();
			//Int_t ymin2 = histo->GetYaxis()->GetXmin();
			//Int_t ymax2 = histo->GetYaxis()->GetXmax();
			Int_t xmin2 = histo->GetXaxis()->GetFirst();
			Int_t xmax2 = histo->GetXaxis()->GetLast();
			Int_t ymin2 = histo->GetYaxis()->GetFirst();
			Int_t ymax2 = histo->GetYaxis()->GetLast();
			//Int_t zmin2 = histo->GetZaxis()->GetXmin();
			//Int_t zmax2 = histo->GetZaxis()->GetXmax();

			if (xmin < xmin2)
				xmin = xmin2;
			if (xmax > xmax2)
				xmax = xmax2;
			if (ymin < ymin2)
				ymin = ymin2;
			if (ymax > ymax2)
				ymax = ymax2;
			//if (zmin<zmin2) zmin = zmin2;
			//if (zmax>zmax2) zmax = zmax2;
			histo->GetXaxis()->SetRange(xmin, xmax);
			histo->GetYaxis()->SetRange(ymin, ymax);
			//histo->GetZaxis()->SetRange(zmin, zmax);

		}

		if (fSpectrum->InheritsFrom(TGraph::Class())) {
			TGraph* tg = (TGraph*) fSpectrum;
			tg->GetXaxis()->SetRange(GetXmin(), GetXmax());
			tg->GetYaxis()->SetRange(GetYmin(), GetYmax());
			//tg->GetZaxis()->SetRange(GetZmin(), GetZmax());
		}

		if (fSpectrum->InheritsFrom(TGraph2D::Class())) {

			TGraph2D* tg = (TGraph2D*) fSpectrum;
			tg->GetXaxis()->SetRange(GetXmin(), GetXmax());
			tg->GetYaxis()->SetRange(GetYmin(), GetYmax());
			//			tg->GetZaxis()->SetRange(GetZmin(), GetZmax());

		}

	} else {
		fError.TreatError(2, 0,
				"No Instance of spectrum to display in GSpectrumIdentity::ReportXYMinMax");
	}

}
//______________________________________________________________________________

void GSpectrumIdentity::ResetAllSpectraOnServer() {
	//Reset Spectrum On Server

	GNetClientRoot * newclientroot;
	newclientroot = NULL;
#ifdef NET_LIB
	GNetClientSoap* newclientsoap;
	newclientsoap=NULL;
#endif
	TString SourceType = GetSourceType();
	TString SourceName = GetSourceName();
	Int_t port = GetPort();
	GSpectrumIdentity* id;
	id = (GSpectrumIdentity*) this;

	if (GetSource() == "NET") {
		if (SourceType == "GRU") {

			newclientroot = new GNetClientRoot((char*) (SourceName.Data()));
			newclientroot->SetPort(port);
			if (newclientroot)
				newclientroot->ResetAllSpectraOnServer(id);

		}

#ifdef NET_LIB
		if (SourceType=="SOAP") {
			newclientsoap = new GNetClientSoap((char*)SourceName.Data());
			newclientsoap->SetPort(port);
			if (newclientsoap)
			newclientsoap->ResetAllSpectraOnServer(id);
		}
#endif

		if (SourceType == "GANIL") {
			fError.TreatError(1, 0,
					"Impossible to reset spectra which comes form Ganil Server.");
		}
	}
	if (GetSource() == "MEM") {
		fError.TreatError(1, 0,
				"Impossible to reset a spectra which comes form a MEM.");
	}

	if (GetSource() == "FILE") {
		fError.TreatError(1, 0,
				"Impossible to reset a spectra which comes form a file.");
	}

	if (newclientroot)
		delete (newclientroot);

#ifdef NET_LIB
	if (newclientsoap)
	delete (newclientsoap);
#endif
}

//______________________________________________________________________________

void GSpectrumIdentity::ResetSpectrumOnServer() {
	//Reset Spectrum On Server


	GNetClientRoot * newclientroot;
	newclientroot = NULL;
#ifdef NET_LIB
	GNetClientSoap* newclientsoap;
	newclientsoap=NULL;
#endif
	TString SourceType = GetSourceType();
	TString SourceName = GetSourceName();
	GSpectrumIdentity* id;
	id = (GSpectrumIdentity*) this;

	if (GetSource() == "NET") {
		if (SourceType == "GRU") {
			newclientroot = new GNetClientRoot((char*) SourceName.Data());
			if (newclientroot)
				((GNetClient*) newclientroot)->ResetSpectrumOnServer(id);
		}
#ifdef NET_LIB
		if (SourceType=="SOAP") {
			newclientsoap = new GNetClientSoap((char*)SourceName.Data());
			if (newclientsoap)
			newclientsoap->ResetSpectrumOnServer(id);
		}
#endif

		if (SourceType == "GANIL") {
			fError.TreatError(1, 0,
					"Impossible to reset a histogram which comes form Ganil Server.");
		}
	}
	if (GetSource() == "MEM") {
		fError.TreatError(1, 0,
				"Impossible to reset a histogram which comes form a file.");
	}

	if (GetSource() == "FILE") {
		fError.TreatError(1, 0,
				"Impossible to reset a histogram which comes form a file.");
	}

	if (newclientroot)
		delete (newclientroot);

#ifdef NET_LIB
	if (newclientsoap)
	delete (newclientsoap);
#endif
}

//______________________________________________________________________________
void GSpectrumIdentity::CancelZoom() {

	if (!fSpectrum)
		return;
	if (fSpectrum->InheritsFrom(TH1::Class())) {
		TH1* h = (TH1*) fSpectrum;
		h->GetXaxis()->UnZoom();
		h->GetYaxis()->UnZoom();
	}

	if (fSpectrum->InheritsFrom(TGraph::Class())) {
		TGraph* tg = (TGraph*) fSpectrum;
		tg->GetXaxis()->UnZoom();
		tg->GetYaxis()->UnZoom();
	}

	if (fSpectrum->InheritsFrom(TGraph2D::Class())) {
		TGraph2D* tg = (TGraph2D*) fSpectrum;
		tg->GetXaxis()->UnZoom();
		tg->GetYaxis()->UnZoom();
	}

}
//______________________________________________________________________________
void GSpectrumIdentity::ApplyZoom(Int_t xmin_pad, Int_t xmax_pad,
		Int_t ymin_pad, Int_t ymax_pad) {
        int dimension = GetDimension();
        
  
	if (!fSpectrum)
		return;
	if (fSpectrum->InheritsFrom(TH1::Class())) {
		TH1* h = (TH1*) fSpectrum;
		h->GetXaxis()->SetRange(xmin_pad, xmax_pad);
		if (dimension == 1){
			h->GetYaxis()->SetRangeUser(ymin_pad, ymax_pad);
			}
		else
			h->GetYaxis()->SetRange(ymin_pad, ymax_pad);
	}

	if (fSpectrum->InheritsFrom(TGraph::Class())) {
		TGraph* tg = (TGraph*) fSpectrum;
		tg->GetXaxis()->SetRange(xmin_pad, xmax_pad);
		if (dimension ==1)
			tg->GetYaxis()->SetRangeUser(ymin_pad, ymax_pad);
		else
			tg->GetYaxis()->SetRange(ymin_pad, ymax_pad);
	}

	if (fSpectrum->InheritsFrom(TGraph2D::Class())) {
		TGraph2D* tg = (TGraph2D*) fSpectrum;
		tg->GetXaxis()->SetRange(xmin_pad, xmax_pad);
		tg->GetYaxis()->SetRange(ymin_pad, ymax_pad);
	}
}
//______________________________________________________________________________
void GSpectrumIdentity::AutoZoom(Int_t *xmin, Int_t *xmax, Int_t *ymin,
		Int_t *ymax) {

	Int_t i, j;

	Int_t Ny, Nx, marge_x, marge_y;
	Bool_t xmax_found = false, xmin_found = false, ymin_found = false,
			ymax_found = false;
	if (!fSpectrum)
		return;
	if (fSpectrum->InheritsFrom(TH1::Class())) {
		TH1* h = (TH1*) fSpectrum;

		xmax_found = false;
		xmin_found = false;
		ymin_found = false;
		ymax_found = false;

		Nx = h->GetNbinsX();
		Ny = h->GetNbinsY();
		*xmin = 1;
		*xmax = Nx;
		*ymin = 1;
		*ymax = Ny;
		for (i = 1; i <= Nx; i++) {
			for (j = 1; j <= Ny; j++) {
				if (h->GetBinContent(i, j) != 0) {
					*xmin = i;
					xmin_found = true;
					break;
				}
				if (xmin_found)
					break;
			}
			if (xmin_found)
				break;
		}

		for (j = 1; j <= Ny; j++) {
			for (i = 1; i <= Nx; i++) {
				if (h->GetBinContent(i, j) != 0) {
					*ymin = j;
					ymin_found = true;
					break;
				}
				if (ymin_found)
					break;
			}
			if (ymin_found)
				break;
		}

		for (i = Nx; i >= 1; i--) {
			for (j = Ny; j >= 1; j--) {
				if (h->GetBinContent(i, j) != 0) {
					*xmax = i;
					xmax_found = true;
					break;
				}
				if (xmax_found)
					break;
			}
			if (xmax_found)
				break;
		}

		for (j = Ny; j >= 1; j--) {
			for (i = Nx; i >= 1; i--) {
				if (h->GetBinContent(i, j) != 0) {
					*ymax = j;
					ymax_found = true;
					break;
				}
				if (ymax_found)
					break;
			}
			if (ymax_found)
				break;
		}

		marge_x = (Int_t)(Float_t(*xmax - *xmin) / 10) + 1;
		marge_y = (Int_t)(Float_t(*ymax - *ymin) / 10) + 1;

		*xmin = *xmin - marge_x;
		if (*xmin < 0)
			*xmin = 0;
		*xmax = *xmax + marge_x;
		if (*xmax > Nx)
			*xmax = Nx;
		*ymin = *ymin - marge_y;
		if (*ymin < 0)
			*ymin = 0;
		*ymax = *ymax + marge_y;
		if (*ymax > Ny)
			*ymax = Ny;

	} else {
		fError.TreatError(1, 0, "AutoZoom only available for TH1,TH2 or TH3");
	}
	// end of if (fSpectrum->InheritsFrom(TH1::Class()))

}
//______________________________________________________________________________
void GSpectrumIdentity::ApplyZoomProportional(Int_t xmin, Int_t xmax,
		Int_t ymin, Int_t ymax) {

	Int_t xmin_pad = 0, xmax_pad = 0, ymin_pad = 0, ymax_pad = 0;
	Int_t dimension = 0;
	Int_t xnbin, ynbin;
	Double_t diff,Ymaxi,Ymini;

	if (!fSpectrum)
		return;

	TH1* h = (TH1*) fSpectrum;
	xnbin = GetXaxis()->GetNbins();
	ynbin = GetYaxis()->GetNbins();
	Ymaxi= h->GetMaximum();
	Ymini= h->GetMinimum() ;
	dimension = GetDimension();
	diff = Ymaxi-Ymini;
	
	if (xmin >= 0)
		xmin_pad = (Int_t) xnbin * xmin / 100;
	if (xmax >= 0)
		xmax_pad = (Int_t) xnbin * xmax / 100;
		
	if (dimension == 1) {		
		if (ymin >= 0)
			ymin_pad = Ymini   +  diff * ymin / 100;
		if (ymax >= 0)
			ymax_pad = Ymini   +  diff * ymax / 100;
	}else{
		
		if (ymin >= 0)
			ymin_pad = (Int_t) ynbin * ymin / 100;
		if (ymax >= 0)
			ymax_pad = (Int_t) ynbin * ymax / 100;
	}

	if ((xmin >= 0) || (xmax >= 0))
		GetXaxis()->SetRange(xmin_pad, xmax_pad);

	if ((ymin >= 0) || (ymax >= 0)) {
		if (dimension == 1) {
			GetYaxis()->SetRangeUser(ymin_pad, ymax_pad);
		} else{
			GetYaxis()->SetRange(ymin_pad, ymax_pad);

		}
	}
}

//______________________________________________________________________________
void GSpectrumIdentity::ApplySetRangeUser(bool inx, bool iny, bool inz,
		Double_t xmin, Double_t xmax, Double_t ymin, Double_t ymax,
		Double_t zmin, Double_t zmax) {


	if (!fSpectrum)  return;
	if ((inx!=0) and (GetXaxis()!=NULL)){
		GetXaxis()->SetRangeUser(xmin, xmax);
		}
	if ((iny!=0) and (GetYaxis()!=NULL)){
		GetYaxis()->SetRangeUser(ymin, ymax);
		}
	if ((inz!=0) and (GetZaxis()!=NULL))
		GetZaxis()->SetRangeUser(zmin, zmax);
	
}
//______________________________________________________________________________

bool GSpectrumIdentity::SpectrumTest(TNamed*spectrum) {
	// test if obj Inherits from TH1,TGraph, TGraph2D
	// return true else false

	if (!spectrum)  return false;

	if (spectrum->InheritsFrom(TH1::Class())) {
		return true;
	}
	if (spectrum->InheritsFrom(TGraph::Class())) {
		return true;
	}
	if (spectrum->InheritsFrom(TGraph2D::Class())) {
		return true;
	}
	return false;
}
//______________________________________________________________________________
TAxis*  GSpectrumIdentity::GetXaxis (){
	TAxis *axe=NULL;
	if (!fSpectrum)  return NULL;
	if (fSpectrum->InheritsFrom(TH1::Class())){
		TH1* h = (TH1*) fSpectrum;
		axe =h->GetXaxis();
	}
	
	if (fSpectrum->InheritsFrom(TGraph::Class())) {
		TGraph* h = (TGraph*) fSpectrum;
		axe =h->GetXaxis();
	}

	if (fSpectrum->InheritsFrom(TGraph2D::Class())) {
		TGraph2D* h = (TGraph2D*) fSpectrum;
		axe =h->GetXaxis();
	}
return axe;
}
//______________________________________________________________________________
TAxis*  GSpectrumIdentity::GetYaxis (){
	TAxis *axe=NULL;

	if (fSpectrum->InheritsFrom(TH1::Class())){
		TH1* h = (TH1*) fSpectrum;
		axe =h->GetYaxis();
	}
	
	if (fSpectrum->InheritsFrom(TGraph::Class())) {
		TGraph* h = (TGraph*) fSpectrum;
		axe =h->GetYaxis();
	}

	if (fSpectrum->InheritsFrom(TGraph2D::Class())) {
		TGraph2D* h = (TGraph2D*) fSpectrum;
		axe =h->GetYaxis();
	}
return axe;
}
//______________________________________________________________________________
TAxis*  GSpectrumIdentity::GetZaxis (){
	TAxis *axe=NULL;

	if (fSpectrum->InheritsFrom(TH1::Class())){
		TH1* h = (TH1*) fSpectrum;
		axe =h->GetZaxis();
	}
	if (fSpectrum->InheritsFrom(TGraph2D::Class())) {
		TGraph2D* h = (TGraph2D*) fSpectrum;
		axe =h->GetZaxis();
	}
return axe;
}
//______________________________________________________________________________



