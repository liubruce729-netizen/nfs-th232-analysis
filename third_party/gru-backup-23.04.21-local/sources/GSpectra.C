// File : GSpectra.C
// Author: Luc Legeard

//////////////////////////////////////////////////////////////////////////////
//
// Class GSpectra
// This class manage data base of spectra (histograms and profiles )in root  format
//  also convert Ganil spectra in Root format
//  recover(read) datas(spectras) or net sources.        |
//             -write spectras in files                              |
//             -save configuration (XML)
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

#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/mtio.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <math.h>
#include <TH1.h>
#include <TKey.h>
using namespace std;

#include <dirent.h>
#include <sys/ioctl.h>
#include <TObject.h>
#include <TSystem.h>
#include <TProfile.h>

#include <TFile.h>

#include <TObjString.h>
#include <Riostream.h>
#include <TString.h>
#include "Riostream.h"
#include <unistd.h>

#include <TXMLFile.h>
#include <TBuffer.h>
#include <TBufferXML.h>
#include <TXMLEngine.h>
#include <TDOMParser.h>
#include <TXMLNode.h>

#include <General.h>
#include <GRU.h>
#include <GDevice.h>
#include "General.h"
#include "GEvent.h"
#include "GSpectra.h"
//______________________________________________________________________________


ClassImp( GSpectra);

GSpectra::GSpectra(void) {
	// Default constructor of spectra object;

	gROOT->cd();

        fNb_Raw_spectra =0;
	fSpectraDB = new GSpectraDB(1, 0); //database: will contains all spectra informations
	fSpectraDB->SetOwner();
        fSpectraDB->SetVerbose( fVerbose);
	fDefaultHostName = "localhost";
	fDefaultSource = "MEM";
	fDefaultSourceType = "GRU";
	fDefaultHostPort = 9090;

	fError.Infos("Histogram data base created");

	fRootConfigFile = "";
	fListServerDB = new GSpectraDB(1, 0);//database: will contains only list of servers
	fListServerDB->SetOwner();
	fListDevice = new GListDevice();

#ifdef  NET_LIB
	fZoneData = new char[MAX_SIZE_GANIL_SPECTRUM]; // vector to receive a Ganil spectrum
	fSpectrumHeader=(tete_spec*) fZoneData; // pointer on header of Ganil spectrum
	fSpectrumHeaderSize = sizeof(TETE_SPEC); // size of
	fSpectrumData = (char*)(fZoneData + fSpectrumHeaderSize );
#endif

}
//_____________________________________________________________________________
GSpectra::~GSpectra() {
	// destructor of GSpectra object

	gROOT->cd();

	//CleanAllDB();
	if (fListDevice){
		fListDevice->Clear();
		delete (fListDevice);
		fListDevice =NULL;
		}
	

#ifdef NET_LIB
	if (fVerbose>5)
	fError.TreatDebug(fVerbose,0,"Delete fSpectra fZoneData");

	if (fZoneData) {
		delete [] fZoneData;
		fZoneData=NULL;
	}
#endif

	if (fVerbose > 5)
		fError.TreatDebug(fVerbose, 0, "Delete  fSpectraDB");

	if (fSpectraDB) {
		delete fSpectraDB;
		fSpectraDB = NULL;
	}

	if (fListServerDB) {
		delete (fListServerDB);
		fListServerDB = NULL;
	}

	if (fVerbose > 5)
		fError.TreatDebug(fVerbose, 1, "Delete Spectra done");

	//delete spectraListFile;
}

//_____________________________________________________________________________
void GSpectra::DumpSpectrumHeader() {
	// Dump informations about the current spectrum buffer (in Ganil format).
	// It doesn't dump data but dump informations from the spectrum header.
#ifdef NET_LIB
	gROOT->cd();
	if (fSpectrumHeader!=NULL) {
		fError.Infos("---------Spectrum description:---------------------");

		cout<<"\n>>num: "<<fSpectrumHeader->num_spectre;
		cout<<"\n>>name: "<<fSpectrumHeader->nom_spectre;
		cout<<"\n>>monoDim / biDim : "<<fSpectrumHeader->type_spectre;
		cout<<"\n>>version: "<<fSpectrumHeader->version;
		cout<<"\n>>spectrum size for X: "<<fSpectrumHeader->dim_x;
		cout<<"\n>>spectrum size for Y: "<<fSpectrumHeader->dim_y;
		cout<<"\n>>X unity: "<<fSpectrumHeader->unite_x;
		cout<<"\n>>Y unity: "<<fSpectrumHeader->unite_y;
		cout<<"\n>>X parameter: "<<fSpectrumHeader->nom_par_x;
		cout<<"\n>>Y parameter: "<<fSpectrumHeader->nom_par_y;

		if (fSpectrumHeader->commentaire!=NULL)
		cout<<"\n>>user comment: "<<fSpectrumHeader->commentaire;

		cout<<"\n\n\tRun description:";
		cout<<"\n>>num: "<<fSpectrumHeader->num_run;
		cout<<"\n>>name: "<<fSpectrumHeader->nom_run;

		cout<<"\n\n\tChannel description:";
		cout<<"\n>>necessary channel size: "<<fSpectrumHeader->taille_canal;
		cout<<"\n>>channel type: "<<fSpectrumHeader->type_canal;
		cout<<"\n>>channel number for X: min= "<<fSpectrumHeader->min_x
		<<" / max= "<<fSpectrumHeader->max_x;
		cout<<"\n>>channel number for Y: min= "<<fSpectrumHeader->min_y
		<<" / max= "<<fSpectrumHeader->max_y;
		cout<<"\n";
	} else
	fError.TreatError(1,0,"spectrum buffer not informed");
#endif
}

//____________________________________________________________________________

TH1I* GSpectra::Convert1D() {
	// convert internal fSpectrumData  buffer ( in Ganil Format)  to 1D Root histogram
	// to use this method , we suppose that fSpectrumData is filled
	gROOT->cd();
	TH1I* h1d = NULL;
#ifdef NET_LIB

	float x_ratio =1;
	char nom_par_x[17], nom_par_y[17], unite_x[17], unite_y[17], type_canal[5];

	char type_spectre[3];
	char nom_spectre[17]; // spectrum name
	Int_t dim_x, dim_y, min_x, min_y, max_x, max_y;
	//Int_t num_spectre; // spectrum number
	Int_t i;
	int taille_type_canal;// = 2 ou 4
	//Int_t magik;

	UShort_t* receive = ( UShort_t*) fSpectrumData;

	//magik = (Int_t)fSpectrumHeader->magik;
	//	num_spectre = (Int_t)fSpectrumHeader->num_spectre;
	strncpy(nom_spectre, fSpectrumHeader->nom_spectre, 15);
	nom_spectre[15] = '\0';
	dim_x = (Int_t)fSpectrumHeader->dim_x;
	dim_y = (Int_t)fSpectrumHeader->dim_y;
	strncpy(nom_par_x, fSpectrumHeader->nom_par_x, 15);
	nom_par_x[15]= '\0';
	strncpy(nom_par_y, fSpectrumHeader->nom_par_y, 15);
	nom_par_y[15]= '\0';
	strncpy(unite_x, fSpectrumHeader->unite_x, 7);
	unite_x[7]= '\0';
	strncpy(unite_y, fSpectrumHeader->unite_y, 7);
	unite_y[7]= '\0';
	min_x = (Int_t)fSpectrumHeader->min_x;
	max_x = (Int_t)fSpectrumHeader->max_x;
	min_y = (Int_t)fSpectrumHeader->min_y;
	max_y = (Int_t)fSpectrumHeader->max_y;
	//taille_canal = (Int_t)fSpectrumHeader->taille_canal;
	strncpy(type_canal, fSpectrumHeader->type_canal, 3);
	type_canal[3]='\0';
	strncpy(type_spectre, fSpectrumHeader->type_spectre, 2);
	type_spectre[2]= '\0';

	if (strcmp("I*2", type_canal)==0) {
		taille_type_canal = 2;
	} else {
		taille_type_canal = 4;
	}

	// to solve a acquisition bug in spectrum files
	if ((min_x==0)&&(max_x==0))
	max_x += dim_x;
	if ((min_y==0)&&(max_y==0))
	max_y += dim_y;

	if (dim_x!=0)
	x_ratio = (float)(max_x - min_x)/(float)dim_x;
	//if (dim_y!=0)	y_ratio = (float)(max_y - min_y)/(float)dim_y;

	// to solve a acquisition bug in spectrum files
	if ((min_x==0)&&(max_x==0))
	max_x += dim_x;
	if ((min_y==0)&&(max_y==0))
	max_y += dim_y;

	if (strcmp("1D", type_spectre)==0) {
		if ((taille_type_canal) == 4) {// canaux sur 4 octets
			h1d = new TH1I(nom_spectre,nom_spectre,dim_x,min_x,max_x);
			for (i=0; i<dim_x; i++) {
				h1d->Fill(i*x_ratio, *receive);
				receive++;
			}
		} else
		fError.TreatError(2, -1, "Error in spectrum 1D allocation");
	}
#endif
	return (h1d);
}

//_____________________________________________________________________________
TH2S* GSpectra::Convert2D() {
	// convert internal fSpectrumData buffer( in Ganil Format) to 2D Root histogram
	gROOT->cd();
	TH2S* h2d = NULL;
#ifdef NET_LIB

	float x_ratio =1, y_ratio= 1;
	char nom_par_x[17], nom_par_y[17], unite_x[17], unite_y[17], type_canal[5];

	char type_spectre[3];
	char nom_spectre[17]; // spectrum name
	Int_t dim_x, dim_y, min_x, min_y, max_x, max_y;

	Int_t i, j;
	int taille_type_canal;// = 2 ou 4

	UShort_t* receive = ( UShort_t*) fSpectrumData;

	//magik = (Int_t)fSpectrumHeader->magik;
	//num_spectre = (Int_t)fSpectrumHeader->num_spectre;
	strncpy(nom_spectre, fSpectrumHeader->nom_spectre, 15);
	nom_spectre[15] = '\0';
	dim_x = (Int_t)fSpectrumHeader->dim_x;
	dim_y = (Int_t)fSpectrumHeader->dim_y;
	strncpy(nom_par_x, fSpectrumHeader->nom_par_x, 15);
	nom_par_x[15]= '\0';
	strncpy(nom_par_y, fSpectrumHeader->nom_par_y, 15);
	nom_par_y[15]= '\0';
	strncpy(unite_x, fSpectrumHeader->unite_x, 7);
	unite_x[7]= '\0';
	strncpy(unite_y, fSpectrumHeader->unite_y, 7);
	unite_y[7]= '\0';
	min_x = (Int_t)fSpectrumHeader->min_x;
	max_x = (Int_t)fSpectrumHeader->max_x;
	min_y = (Int_t)fSpectrumHeader->min_y;
	max_y = (Int_t)fSpectrumHeader->max_y;
	//taille_canal = (Int_t)fSpectrumHeader->taille_canal;
	strncpy(type_canal, fSpectrumHeader->type_canal, 3);
	type_canal[3]='\0';
	strncpy(type_spectre, fSpectrumHeader->type_spectre, 2);
	type_spectre[2]= '\0';

	if (strcmp("I*2", type_canal)==0) {
		taille_type_canal = 2;
	} else {
		taille_type_canal = 4;
	}

	// to solve a acquisition bug in spectrum files
	if ((min_x==0)&&(max_x==0))




	max_x += dim_x;
	if ((min_y==0)&&(max_y==0))
	max_y += dim_y;

	if (dim_x!=0)
	x_ratio = (float)(max_x - min_x)/(float)dim_x;
	if (dim_y!=0)
	y_ratio = (float)(max_y - min_y)/(float)dim_y;

	// to solve a acquisition bug in spectrum files
	if ((min_x==0)&&(max_x==0))
	max_x += dim_x;
	if ((min_y==0)&&(max_y==0))
	max_y += dim_y;

	if (strcmp("2D", type_spectre)==0) {
		if ((taille_type_canal) == 2) {// canaux sur 2 octets
			h2d
			= new TH2S(nom_spectre,nom_spectre,dim_x,min_x,max_x,dim_y,min_y,max_y);
			for (j=0; j<dim_y; j++) {
				for (i=0; i<dim_x; i++) {
					h2d->Fill(i*x_ratio, j*y_ratio, *receive);
					receive++;
				}
			}
		} else
		fError.TreatError(2, -1, " in spectrum 2D allocation");
	}
#endif
	return (h2d);
}

//______________________________________________________________________________
void GSpectra::SetStarted(Int_t index, Int_t index2) {
	// valid started flag  of histogram of index or of histogram of index from index to index2
	if (index >= 0) {
		if ((index < index2)) {
			for (int i = index; i <= index2; i++)
				SetStarted(i, true);
		} else {
			SetStarted(index, true);
		}
	}
}

//______________________________________________________________________________
void GSpectra::SetStopped(Int_t index, Int_t index2) {
	// valid stopped flag  of histogram of index or of histogram of index from index to index2
	if (index >= 0) {
		if (index < index2)
			for (int i = index; i <= index2; i++)
				SetStarted(i, false);
		else
			SetStarted(index, false);
	}
}
//______________________________________________________________________________
void GSpectra::SetStopped(const char *histo) {
	// valid stopped flag  of histogram histo
	if (!strcmp(histo, "all")) {
		for (int i = 0; i <= GetLast(); i++) {
			SetStarted(i, false);
		}
	} else
		SetStarted(GetDB()->GetIndexByName(histo), false);
}
//______________________________________________________________________________
void GSpectra::SetStarted(const char *histo) {// valid started flag  of histogram histo
	if (!strcmp(histo, "all")) {
		for (int i = 0; i <= GetLast(); i++)
			SetStarted(i, true);
	} else
		SetStarted(GetDB()->GetIndexByName(histo), true);
}

//______________________________________________________________________________
void GSpectra::SetStarted(Int_t index, bool start) { // valid started or stopped  flag  of histogram of index index
	if (index >= 0)
		GetIdWithIndex(index)->SetStarted(start);
}

//______________________________________________________________________________
bool GSpectra::IsStarted(Int_t index) {
	// return started  flag
	return (GetIdWithIndex(index)->GetStarted());
}

//______________________________________________________________________________
bool GSpectra::IsStarted(const char* histo) {
	// return started  flag
	return (GetIdWithIndex(GetDB()->GetIndexByName(histo))->GetStarted());
}
//______________________________________________________________________________
void GSpectra::ConstructAllRawSpectra() {
	// Start all raw Histogram
	gROOT->cd();

	if (GetDB() == NULL) {
		fError.TreatError(2, 0,
				"No DataBase of spectra so impossible to start Raw Histograms.");
	}
	GetDB()->ConstructAllRawInDB();
}
//______________________________________________________________________________
void GSpectra::StartAllRawParameters() {
	// Start all raw Histogram
	gROOT->cd();

	if (GetDB() == NULL) {
		fError.TreatError(2, 0,
				"No DataBase of spectra so impossible to start Raw Histograms.");
	}
	GetDB()->SetStartedDB(true);
}

//______________________________________________________________________________
void GSpectra::AddAllParameterInDB(DataParameters *data_para, TString family,
		Int_t auto_level) {
	//Add all Parameter in BD spectra list
	// A spectrum is defined by  standart raw  parameter
	// All these spectra are included in our TMapfile.
	// if min and max are specified ( =! 0) so not all paramerters will be considered)
        // auto_level allow to auto select raw parameters in sub family if names of parameters contain "_"
	// auto_level = 1-> max of sub_family =1; auto level = 2-> max of sub_family =2; etc...
	// by default auto_level =0
	
	int i;
	i = 0;
        int error =0;
        TString tempos;
	if (!data_para) {
		fError.TreatError(2, 0, "No defined parameters\n");
		return;
	}
	fNb_Raw_spectra = data_para->GetNbParameters();
        
	for (i = 0; i < fNb_Raw_spectra; i++) {

		error += RawParameter(data_para, i, family, auto_level);

	}
	if (error>0){
		tempos.Form("%d raw spectra already existes in database , so no include again",error);
		fError.TreatError(1, 0,tempos);
	}
	// add a spectrum containing all raw parameter and increment it in case of non empty parameter.
	
	SumParameterSpectrum(data_para,family);
	tempos.Form("Number of Raw Spectra %d ",fNb_Raw_spectra);
        fError.TreatError ( 0,0,tempos);
        ConstructAllRawSpectra();      
}
//______________________________________________________________________________
void GSpectra::SumParameterSpectrum(DataParameters *data_para, TString family){ 
 if (fNb_Raw_spectra>0){
    	TH1I* SumRaws = new TH1I("SumRaws","SumRaws",fNb_Raw_spectra,0,fNb_Raw_spectra);
    	int index = AddSpectrum(SumRaws,family); 
    	GSpectrumIdentity * id;
    	id = (GSpectrumIdentity*) GetDB()->At(index);
    	id ->SetTypeSpe(RAWTYPE);
	}
}
//______________________________________________________________________________
int  GSpectra::RawParameter(DataParameters *data_para, int index,
		TString family, Int_t auto_level) {
	// Prepare the creation of a 1D histogram  for each raw  parameter
	// and add it in Spectra database
	// TString parName = data_para->GetParName(index);
	// index = index in raw parameter list
	// return 1 is is already in DB else 0 if OK
	int nb;
	nb = (int) data_para->GetNbitsFromIndex(index);
	Int_t nc = (Int_t) pow(2., (int) nb);
	Int_t indexa; // index of histogram in database
	Int_t local_auto_level = auto_level;
	TString para_name, tempos;
	GSpectrumIdentity * id;
	enum TypeSpe typespe = RAWTYPE; // 1 = raw spectra
	int already =0;

	para_name = data_para->GetParNameFromIndex(index);

	if (auto_level > 0) {
		TString current;
		current = "";
		TObjArray *array = para_name.Tokenize("_");
		Int_t end = array->GetLast();

		if (auto_level > end)
			local_auto_level = end;

		for (Int_t i = 0; i < local_auto_level; i++) {
			current = current + ((TObjString*) array->At(i))->GetName();

		}
		family = family + "/" + current;
	delete (array);
	}

	index = GetDB()->IsHistoExiste((char*) para_name.Data(),
			(char*) family.Data(), typespe);

	if (index >= 0) { 
		already =1;
		if (fVerbose >7) {
			tempos.Form("Spectrum already in DB : %s ",para_name.Data()); 
			fError.TreatError (1,0,tempos);
			}
	} else {
		id = new GSpectrumIdentity();
		id ->SetSpectrumName(para_name.Data());
		id ->SetStarted(false);
		id ->SetTypeSpe(typespe);
		id ->SetFamily(family);
		id ->SetNbBits(nc);
		id ->SetSourceName(gSystem->HostName());
		id ->SetSourceType("GRU");
		id ->SetSource("MEM");
		indexa = GetDB()->AddIdentity(id);
		if (indexa < 0) {
			fError.TreatError(
					2,
					indexa,
					"Pb in add histo in data base, negativ index for spectrum :",
					para_name.Data());
		}
	}
	return already;
}
//______________________________________________________________________________
Int_t GSpectra::AddCut(TNamed *spectrum, const char* family, Int_t port,
		const char* source_name, const char* sourcenetorfile) {
	int index = AddSpectrum(spectrum, family, port, source_name,
			sourcenetorfile);
	if (index > 0)
		GetIdWithIndex(index)->SetTypeSpe(CUTTYPE); // Set Type histo = Tcut (3)
	return index;
}
//______________________________________________________________________________
void GSpectra::GetCutFromFile(TNamed *spectrum, const char* family,
		const char* filename) {
	// read a cut from file and introduce it in DB


	TString tempos;
	TString name;
	TString source;
	TString sourcetype;
	TString directory;
	directory = family;
	TString sourcefile = "FILE";
	sourcetype = "GRU";
	GSpectrumIdentity * id;
	id = NULL;
	if (!spectrum) {
		fError.TreatError(2, 0, " Cut is not instancied");
	} else {
		name = spectrum->GetName();
		source = filename;
		tempos.Form("GSpectra::GetCutFromFile: impossible to read file %s",
				filename);
		TFile *file = TFile::Open(filename, "read");
		if (!file) {
			fError.TreatError(2, 0, tempos);
			return;
		}
		int nb = GetDB()->IsHistoExiste(family, filename);
		if (nb > 0) {
			id = (GSpectrumIdentity*) GetDB()->At(nb);
		} else {
			id = new GSpectrumIdentity(spectrum, name, source, sourcetype,
					sourcefile, 0, directory, 0, 0, 0, false);
			GetDB()->AddIdentity(id);
		}
		if ((id->ReadSpectrum(file)) ==false) {
			tempos.Form(
					"GSpectra::GetCutFromFile: impossible to get cut %s %s  from file %s",
					name.Data(), family, filename);
			fError.TreatError(1, 0, tempos);
		}
		if (!(id->GetSpectrum())->InheritsFrom(TCutG::Class())) {
			tempos.Form(
					"GSpectra::GetCutFromFile: Is not a TCutG spectrum %s %s ",
					name.Data(), family);
			fError.TreatError(1, 0, tempos);
		} else {
			id->SetTypeSpe(CUTTYPE);
		}
		file->Close();
		gROOT->cd();
	}
}

//______________________________________________________________________________
Int_t GSpectra::AddSpectrum(TNamed *spectrum, const char* family, Int_t port,
		const char* source_name, const char* sourcenetorfile) {
	// add a spectrum in our Database
	// return new index if add else return -1
	Int_t index;
	index = -1;
	if (spectrum == NULL) {
		fError.TreatError(1, 0, "Add a Null spectrum is not valide");
		return index;
	}
	gROOT->cd();

	TString histoname;
	TString host;
	TString source;

	if (sourcenetorfile == NULL) {
		source = fDefaultSource;
	} else {
		source = sourcenetorfile;
	}

	if (source_name == NULL) {
		host = gSystem->HostName();
	} else {
		host = source_name;
	}

	if (port == 0) {
		port = fDefaultHostPort;
	}

	index = GetDB()->AddIdentity(spectrum, (char*) spectrum->GetName(), host,
			"GRU", source, port, family);
	if (index < 0)
		fError.TreatError(2, index, "Impossible to add Spectrum");
	return index;
}

//______________________________________________________________________________

void GSpectra::RemoveSpectrum(TNamed *sp) {
	// remove spectra  from database
	// and delete it and ist identity

	gROOT->cd();
	// remove a spectrum in data base of histograms
	//fTMapeFile->cd();

	Int_t index = GetDB()->GetIndex(sp);
	if (index > 0)
		GetDB()->DeleteIdentity(index, true);
	sp = NULL;

}
//______________________________________________________________________________

void GSpectra::RemoveSpectrum(int no) {
	// remove spectrum by its no from database
	// and delete it and ist identity

	gROOT->cd();
	TH1 * histo;
	histo = (TH1*) GetSpectrum(no);

	RemoveSpectrum(histo);
}
//______________________________________________________________________________
void GSpectra::CleanAllDB() {
	// remove all  spectra by its no from database
	// and delete it and ist identity
	gROOT->cd();
	if (fVerbose > 5)
		fError.TreatError(0, 0, "Clean all DataBase");
	GetDB()->DeleteAllIdentities();
	GetServerDB()->DeleteAllIdentities();
}
//______________________________________________________________________________
void GSpectra::SpeSave(const char* filename) {
	// save all  spectra  of  memory in a "filename" file.
	gROOT->cd();

	if (strcmp(filename, "") != 0) {

		fError.TreatError(0, 0, "Saving spectra !");
		TFile *f1;
		f1 = new TFile(filename, "recreate", "f1");
		f1->cd();
		GetDB()->SaveSpectra(f1);
		f1->Close();
		if (f1) {
			delete f1;
			f1 = NULL;
		}
	} else {
		fError.TreatError(2, -1, " Error saving spectra");

	}

	gROOT->cd();
}

//______________________________________________________________________________

void GSpectra::FillRawSpectra(GEventBase* _event) {
	// we increment started raw  histograms histograms
	//in this methode , we suppose that raw histograms are placed in order (0,1..->n)
	// in spectrum database GetDB()

	gROOT->cd();
	Int_t index;
	UShort_t label, value, i, evtsize;
	GSpectrumIdentity * id, *id2;
	TString hname;
	bool started;

	// recherche de differnce d'index entre la liste des parametre et le liste des spectres

	if (!GetDB()) {
		fError.TreatError(2, 0, "No raw hitogram to increment in DataBase");
		return;
	}
	hname = GetDB()->GetNameByIndex(0);

	evtsize = (_event->GetArrayLabelValueSize()) / 2;
	if (evtsize) {
		for (i = 0; i < evtsize; i++) {
			value = _event->GetArrayLabelValue_Value(i);
			label = _event->GetArrayLabelValue_Label(i);
			index = _event->GetDataParameters()->GetIndex(label);
			id = (GSpectrumIdentity*) GetDB()->At(index);
			if (id){
				started = id->GetStarted();
				if (started ){
					id->FillHisto(value);
					id2 = (GSpectrumIdentity*)(GetDB()->At(fNb_Raw_spectra));// get sum histo( at end of DB)
					id2->FillHisto(index);
				} 
			}else { fError.TreatError(2, 0, "GSpectra::FillRaw , GSpectrumIdentity null");}
		}	
	}
}
//______________________________________________________________________________
GSpectrumIdentity* GSpectra::GetIdWithIndex(Int_t index) {
	if (index >= 0)
		return (GetDB()->GetIdentity(index));
	else
		return NULL;
}
//_________________________________________________________________________________
GSpectrumIdentity* GSpectra::GetIdentity(const char* sp, const char* familly) {
	return (GetDB()->GetIdentity(sp, familly));
}
//_________________________________________________________________________________
GSpectrumIdentity* GSpectra::GetIdentity(const char* sp) {
	return (GetDB()->GetIdentity(sp));
}
//_________________________________________________________________________________
TNamed* GSpectra::GetSpectrum(Int_t index) {
	return (GetDB()->GetSpectrumByIndex(index));
}
//_________________________________________________________________________________
TNamed* GSpectra::GetSpectrum(const char* sp) {
	return (GetDB()->GetSpectrumByName(sp));
}
//_________________________________________________________________________________
TNamed* GSpectra::GetSpectrum(const char* sp, const char* familly) {
	return (GetDB()->GetSpectrumByName(sp, familly));
}
//_________________________________________________________________________________
void GSpectra::SetSynchroOnEvt(const char* sp, bool value) {
	GetIdentity(sp)->SetLocSameEvt(value);
}
//_________________________________________________________________________________
void GSpectra::SetSynchroOnEvt(const char* sp, const char* familly, bool value) {
	GetIdentity(sp, familly)->SetLocSameEvt(value);
}
//_________________________________________________________________________________
TH1* GSpectra::GetHisto(const char* histo) {
	TH1 *th;
	th = NULL;
	TNamed * sp;
	sp = GetSpectrum(histo);
	if (sp) {
		if (sp ->InheritsFrom(TH1::Class()))
			th = (TH1*) sp;
	}
	return th;
}
//_________________________________________________________________________________
TH1* GSpectra::GetHisto(Int_t index) {
	TH1 *th;
	th = NULL;
	TNamed * sp;
	sp = GetSpectrum(index);
	if (sp) {
		if (sp ->InheritsFrom(TH1::Class()))
			th = (TH1*) sp;
	}
	return th;
}
//_________________________________________________________________________________
void GSpectra::RazDB() { // reset  all spectra
	gROOT->cd();
	GetDB()->RazDB();
}
//_________________________________________________________________________________
void GSpectra::RemoveDB() {
	// remove  all spectra
	gROOT->cd();
	GetDB()->DeleteAllIdentities();
	
}
//_________________________________________________________________________________
Int_t GSpectra::GetLast() {
	// get order number of last entry
	return GetDB()->GetLast();
}
//_________________________________________________________________________________
void GSpectra::ReplaceDB(GSpectraDB* newDB) {
	// replace the internal DB of spectra
	if (GetDB())
		delete (GetDB());
	SetDB(newDB);
}
//_________________________________________________________________________________
Int_t GSpectra::UpdateSpectraList() {
	// upage List Spectra
	Int_t nb = 0;
	GSpectrumIdentity * idserv =NULL;

	if (fListServerDB == NULL)
		return nb;
	if (fListServerDB->GetLast() < 0)
		return nb;
	for (Int_t i = 0; i <= fListServerDB->GetLast(); i++) {
		idserv = (GSpectrumIdentity *) fListServerDB->At(i);

		if (idserv->GetSource() == "NET") {
			if (idserv->GetSourceType() == "GANIL") {
				nb += UpdateSpectraListGanilNet(idserv);
			}
			if (idserv->GetSourceType() == "GRU") {
				nb += UpdateSpectraListGruNet(idserv);
			}
			if (idserv->GetSourceType() == "SOAP") {
				nb += UpdateSpectraListSoapNet(idserv);
			}
		}
		if (idserv->GetSource() == "FILE") {

			if (idserv->GetSourceType() == "GANIL") {
				nb += UpdateSpectraListGanilFile(idserv);
			} else {
				nb += UpdateSpectraListGruFile(idserv);
			}
		}
		if (idserv->GetSource() == "MEM") {
			nb += UpdateSpectraListMemory(idserv);
		}
	}

	return nb;
}
//______________________________________________________________________________
Int_t GSpectra::UpdateSpectraListGanilFile(GSpectrumIdentity *idserv) {
	//create a TFile to recover histograms contained in the file
	TFile *mfile = NULL;
	TString SourceName;
	TString SourceType;
	SourceName = idserv->GetSourceName();
	SourceType = idserv->GetSourceType();
	Int_t nb, ret;
	nb = 0;

	if (fListDevice->GetTape() == NULL)
		fListDevice->SetTape(new GTape());

	mfile = new TFile(SourceName);
	if (SourceName == "")
		return nb;
	if (!mfile) {
		fError.TreatError(2, -2, "No File with this name :", SourceName);
		return nb;

	}
	if (SourceName == "")
		return nb;

	TNamed * sp;
	TNamed *sp2 = NULL;
	fListDevice->GetTape()->SetDevice(SourceName);

	if (fListDevice->GetTape() == NULL) {
		return nb;
	}
	sp = fListDevice->GetTape()->GetSpectrum("pipo", sp2);
	GSpectrumIdentity * id;
	id = new GSpectrumIdentity(NULL, sp->GetName(), SourceName, SourceType,
			"FILE", 0, "", -1, -1, fSpectraDB->GetLast() + 1);
	ret = fSpectraDB->AddIdentity(id);
	if (ret < 0) {
		delete (id);
		id = NULL;
	} else {
		fSpectraDB->ReIndexation();
	}

	fListDevice->GetTape()->Close();
	return 1;

}
//______________________________________________________________________________
Int_t GSpectra::UpdateSpectraListGruFile(GSpectrumIdentity *idserv) {
	//Update spectra contained in a file. Spectra are put in the fSpectraDB
	//check source type
	gROOT->cd();
	Int_t nb = 0;
	TString SourceName;
	TString SourceType;
	SourceName = idserv->GetSourceName();
	SourceType = idserv->GetSourceType();
	TString tempos;

	if (SourceName == "")
		return nb;

	//create a TFile to recover histograms contained in the file
	TFile *mfile = NULL;
	mfile = new TFile(SourceName);

	if (!mfile) {
		fError.TreatError(2, -2, "No File with this name :", SourceName);
		return nb;

	}

	mfile->cd();
	TDirectory *dir;
	dir = mfile->GetDirectory("");
	nb = ScanDirectoryFile(mfile, dir);

	gROOT->cd();
	fSpectraDB->ReIndexation();
	if (nb == 0)
		fError.TreatError(1, 0, "File seems to be empty :", SourceName);
	return nb;
}
//______________________________________________________________________________
Int_t GSpectra::ScanDirectoryFile(TFile * file, TDirectory * dir) {
	Int_t nb = 0;
	TList* list;
	list = dir->GetListOfKeys();
	TNamed* obj;
	TString tempos, path, filename, family;
	TKey* key;

	path = dir->GetPath();

	if (list->GetSize() != 0) {
		for (int i = 0; i < list->GetSize(); i++) {
			key = (TKey*) (list->At(i));
			tempos = key->GetName();
			obj = (TNamed*) ((key)->ReadObj());
			//obj = (TNamed*) (file->Get(tempos));

			if (obj) {
				if (obj->InheritsFrom(TDirectory::Class())) {
					file->cd(((TDirectory*) obj)->GetPath());
					nb += ScanDirectoryFile(file, (TDirectory*) obj);
				} else {
					nb = nb + AddSpectrumFromFile(file, obj, path);

				}
			}

		}

	}
	return nb;
}
//______________________________________________________________________________
Int_t GSpectra::AddSpectrumFromFile(TFile * file, TNamed * sp, TString path) {
	TString SourceName;

	SourceName = file->GetName();
	TString tempos;

	TString filename;
	TString family;

	TObjArray *array = path.Tokenize(":");

	filename = ((TObjString*) array->At(0))->GetName();
	family = ((TObjString*) array->At(1))->GetName();

	int ret = 0;
	GSpectrumIdentity * id;
	GSpectrumIdentity idtest;
	Int_t port = 0;

	if (sp) {
		if (idtest.SpectrumTest((TNamed*) sp)) {

			id = new GSpectrumIdentity(NULL, sp->GetName(), SourceName, "GRU",
					"FILE", port, SourceName, -1, -1, 0, 0);
			id->SetSpectrumName(sp->GetName());
			id->SetSourceName(filename);
			id->SetSourceType("GRU");
			id->SetSource("FILE");
			id->SetPort(port);
			id->SetFamily(family);
			if (sp->InheritsFrom(TCutG::Class()))
				id->SetTypeSpe(CUTTYPE);
			ret = GetDB()->AddIdentity(id);
			if (ret < 0) {
				delete (id);
				id = NULL;
				ret = 0;
			} else {
				ret = 1;
			}
		}
	}

	return ret;
}

//______________________________________________________________________________
Int_t GSpectra::UpdateSpectraListMemory(GSpectrumIdentity *idserv) {
	//Update spectras available on memory. Spectras are put in the spectraDB

	gROOT->cd();
	Int_t port = 0;// we are in file case => fPort =0
	Int_t nb = 0;
	TString SourceName;
	TString SourceType;
	TString tempo;

	if (fListDevice->GetClientMemory() == NULL) {

		GSpectra * gsp = (GSpectra*) gROOT->FindObjectAny("/GRU/GSpectra");
		if (gsp == NULL) {
			fError.TreatError(2, 0,
					"GSpectra::UpdateSpectraListMemory no GSpectra object in memory");
			return 0;
		}
		GClientMemory * clientmem = new GClientMemory();
		GSpectraDB* db = gsp->GetDB();
		clientmem->InitClient(db);

		fListDevice->SetClientMemory(clientmem);
	}
	SourceName = idserv->GetSourceName();
	SourceType = idserv->GetSourceType();
	port = idserv->GetPort();
	fListDevice->SetMyDevice(fListDevice->GetClientMemory());
	if (SourceName != "") {
		if (fListDevice->GetMyDevice() != NULL) {
			fListDevice->GetMyDevice() ->SetDevice(SourceName.Data());
			fListDevice->GetMyDevice() ->SetPort(port);
			fListDevice->GetMyDevice()->Open();//OPEN client
			GSpectraDB
					*DB =
							((GClientMemory*) (fListDevice->GetMyDevice()))->GetSpectraDB();
			fSpectraDB->UpdateFromDB(DB);
			fListDevice->GetMyDevice()->Close();
			return nb;
		} else {
			tempo.Form("Bad server , Type client =%d ",
					fListDevice->GetMyDevice()->GetType());
			fError.TreatError(1, -1, tempo);
			fListDevice->GetMyDevice()->Close();
			return nb;
		}
	}
	return nb;
}
//______________________________________________________________________________
Int_t GSpectra::ScanDirectoryMemory(char * dir ) {


	TObject * fol = (GSpectra*) gROOT->FindObjectAny(dir);
		if (fol == NULL) {
			fError.TreatError(2, 0,
					"GSpectra::ScanDirectoryMemory no  object in memory");
			return 0;
		}
	if (fol->InheritsFrom(TFolder::Class())){
		return ScanDirectoryMemory((TFolder*)fol);
	}else {
		fError.TreatError(2, 0,
							"GSpectra::ScanDirectoryMemory no Folder object in memory");
		return 0;
	}

}
//______________________________________________________________________________
Int_t GSpectra::ScanDirectoryMemory(TFolder* folder,char* path) {
	// scan a TFolder get all spectra and references them  in Data Base (GSpectraDB)
	TCollection * list = folder->GetListOfFolders();
	int total = 0;
	int nb = list->GetEntries();

	char pathplus[256];
	char pathnext[512];
	strcpy(pathplus,folder->GetName());
	if (path!=NULL){
		sprintf(pathnext,"%s/%s",path,pathplus);
	}else{
		strcpy(pathnext,pathplus);
	}
	TObject* obj = NULL;
	if (fVerbose) cout << " Nb entries folder = " << nb << " on path "<<pathnext<<"\n";
	TIter iter = list->begin();
	iter.Reset();
	for (int i = 0; i < nb; i++) {
		obj = iter.Next();
		if (obj == NULL) {
			cout << " NULL objet in folder\n";
		} else {
			if (fVerbose) cout << i << " Name of object= " << obj->GetName() << "\n";
			if (obj ->InheritsFrom(TFolder::Class())) {
				total += ScanDirectoryMemory((TFolder*) obj,pathnext);
			} else {
				if (obj ->InheritsFrom(TNamed::Class())) {
					AddSpectrumFromMemory((TNamed*) obj, (char*) pathnext);
					total ++;
				}
			}
		}
	}
	return total;
}

//______________________________________________________________________________
Int_t GSpectra::AddSpectrumFromMemory(TNamed * sp, char* path) {

	TString tempos;
	TString family = path;

	int ret = 0;
	GSpectrumIdentity * id;
	GSpectrumIdentity idtest;
	Int_t port = 0;

	if (sp) {
		if (idtest.SpectrumTest((TNamed*) sp)) {

			id = new GSpectrumIdentity(NULL, sp->GetName(), path, "GRU",
					"MEM", port, "local", -1, -1, 0, 0);
			id->SetSpectrum(sp);
			id->SetSpectrumName(sp->GetName());
			id->SetSourceName("Local");
			id->SetSourceType("GRU");
			id->SetSource("MEM");
			id->SetPort(port);
			id->SetFamily(family);
			if (sp->InheritsFrom(TCutG::Class()))
				id->SetTypeSpe(CUTTYPE);
			ret = GetDB()->AddIdentity(id);
			if (ret < 0) {
				delete (id);
				id = NULL;
				ret = 0;
			} else {
				ret = 1;
			}
		}
	}

	return ret;
}
//______________________________________________________________________________
void GSpectra::ReferenceInMemory() {
	//Delete all Instance of histos from identities but keep all specifications.
	//Data base is not empty after DeleteHistos()

	TFolder * dir = NULL;
	TObject * obj = NULL;
	GSpectra * gsp = NULL;
	obj = (gROOT->FindObjectAny("/GRU/GSpectra"));

	if (gsp != NULL) {
		fError.TreatError(1, 0,
				"GSpectra::ReferenceInMemory /GRU/GSpectra already created in memory");
		if (!(obj->InheritsFrom("TFolder"))) {
			fError.TreatError(1, 0,
					"GSpectra::ReferenceInMemory but /GRU/GSpectra is not a GSpectra object");
			return;
		}
	} else {
		obj = gROOT->FindObjectAny("/GRU");
		if (obj != NULL) {
			if (obj->InheritsFrom("TFolder")) {
				dir = (TFolder*) obj;
			} else {
				fError.TreatError(1, 0,
						"GSpectra::ReferenceInMemory Object GRU already exist but is not a directory");
				return;
			}
		} else {
			dir = gROOT->GetRootFolder()->AddFolder("GRU", "Directory for GRU");
		}
		dir->Add(this);
	}
	// test
	gsp = (GSpectra*) (gROOT->FindObjectAny("/GRU/GSpectra"));

	if (gsp != this)
		fError.TreatError(2, 0,
				"GSpectra::ReferenceInMemory GSpectra has not been retreived correctly");
}

//______________________________________________________________________________
Int_t GSpectra::UpdateSpectraListGruNet(GSpectrumIdentity *idserv) {
	//Update spectra available on the network. Spectra are put in the spectraDB

	gROOT->cd();
	Int_t port = 0;// we are in file case => fPort =0
	Int_t nb = 0;
	TString SourceName;
	TString SourceType;
	TString tempo;
	GNetClientRoot *mynet = NULL;

	if (fListDevice->GetNetClientRoot() == NULL) {
		mynet = new GNetClientRoot((char*) "localhost");
		fListDevice->SetNetClientRoot(mynet);
	}
	fListDevice->SetMyDevice(fListDevice->GetNetClientRoot());

	SourceName = idserv->GetSourceName();
	SourceType = idserv->GetSourceType();
	port = idserv->GetPort();

	if (SourceName != "") {
		if (fListDevice->GetMyDevice() != NULL) {
			fListDevice->GetMyDevice() ->SetDevice(SourceName.Data());
			fListDevice->GetMyDevice() ->SetPort(port);
			fListDevice->GetMyDevice()->Open();//OPEN client
			//((GNetClientRoot*) fListDevice->GetMyDevice())->TestServer();
			GSpectraDB
					*DB =
							((GNetClientRoot*) fListDevice->GetMyDevice())->GetSpectraDB();
			fSpectraDB->UpdateFromDB(DB);
			fListDevice->GetMyDevice()->Close();
			return nb;
		} else {
			tempo.Form("Bad server , Type client =%d ",
					fListDevice->GetMyDevice()->GetType());
			fError.TreatError(1, -1, tempo);
			fListDevice->GetMyDevice()->Close();
			return nb;
		}
	}
	return nb;
}
//______________________________________________________________________________
Int_t GSpectra::UpdateSpectraListSoapNet(GSpectrumIdentity *idserv) {
	//Update spectras available on the network. Spectras are put in the spectraDB
	gROOT->cd();
	Int_t nb = 0;

#ifdef  NET_LIB
	Int_t port = 0;// we are in file case => fPort =0
	GNetClientSoap *mynet = NULL;
	TString SourceName;
	TString SourceType;
	TString tempo;

	if (fListDevice->GetNetClientSoap() == NULL) {
		mynet = new GNetClientSoap((char*) "localhost");
		fListDevice->SetNetClientSoap(mynet);
	}
	fListDevice->SetMyDevice(fListDevice->GetNetClientSoap());
	//fNetClient = fListDevice->SetGNetClientRoot();

	SourceName = idserv->GetSourceName();
	SourceType = idserv->GetSourceType();
	port = idserv->GetPort();

	if (SourceName != "") {
		if (fListDevice->GetMyDevice() != NULL) {
			fListDevice->GetMyDevice() ->SetDevice(SourceName.Data());
			fListDevice->GetMyDevice() ->SetPort(port);
			fListDevice->GetMyDevice()->Open();//OPEN client
			//((GNetClientRoot*) fListDevice->GetMyDevice())->TestServer();
			GSpectraDB
			*DB =
			((GNetClientRoot*) fListDevice->GetMyDevice())->GetSpectraDB();
			DB->MakeDUMP();
			fSpectraDB->UpdateFromDB(DB);
			fListDevice->GetMyDevice()->Close();
			return nb;
		} else {
			tempo.Form("Bad server , Type client =%d ",
					fListDevice->GetMyDevice()->GetType());
			fError.TreatError(1, -1, tempo);
			fListDevice->GetMyDevice()->Close();
			return nb;
		}
	}
#endif

	return nb;

}

//______________________________________________________________________________
Int_t GSpectra::UpdateSpectraListGanilNet(GSpectrumIdentity *idserv) {

	fError.TreatError(1, -1, "UpdateSpectraListGanilNet function is obsolete");
	
        cout <<idserv<<"\n";
	return 0;
}

//______________________________________________________________________________
void GSpectra::ReadXML(TXMLEngine *xml, XMLNodePointer_t node, bool server) {
	//Create an XML file containing configuration carateristics
	GSpectraDB * DB;

	if (server)
		DB = fListServerDB;
	else
		DB = fSpectraDB;

	if (DB) {

		DB->ReadXML(xml, node, server);
	}

}

//______________________________________________________________________________
void GSpectra::CreateXML(TXMLEngine *xml, XMLNodePointer_t node, bool server) {
	//Create an XML file containing configuration carateristics

	GSpectraDB * DB;

	TString tempo, tempo2;

	if (server)
		DB = fListServerDB;
	else
		DB = fSpectraDB;

	if (DB) {
		if (DB->GetLast() >= 0) {
			DB->CreateXML(xml, node, server);
		}
	}
}
//______________________________________________________________________________
TNamed * GSpectra::GetSpectrumNetOrFileOrMem(GSpectrumIdentity* id) {
	//
	TNamed *sp = NULL;
	if (id->GetSource() == "NET") {
		sp = GetSpectrumNet(id);
		return (sp);
		if (id->GetSource() == "FILE")
			sp = GetSpectrumFile(id);
		return (sp);
	}
	if (id->GetSource() == "MEM") {
		sp = GetSpectrumMem(id);
		return (sp);
	}
	fError.TreatError(1, 0, "Impossible Get Spectrum net file or mem.");
	return (sp);
}

//______________________________________________________________________________
void GSpectra::ResetSpectrumOnServer(GSpectrumIdentity* id) {
	//Reset Spectrum On Server

	TString SourceType = id->GetSourceType();

	if (id->GetSource() == "NET") {

		if (SourceType == "GRU") {
			if (fListDevice->GetNetClientRoot() == NULL)
				fListDevice->SetNetClientRoot(new GNetClientRoot(
						(char*) "localhost"));

			fListDevice->SetMyDevice(fListDevice->GetNetClientRoot());
			((GNetClient*) (fListDevice->GetMyDevice()))->ResetSpectrumOnServer(
					id);
		} else {
			fError.TreatError(1, 0,
					"Impossible to reset a histogram which comes form Ganil Server.");
		}
		return;
	}

	if (id->GetSource() == "MEM") {
		if (fListDevice->GetClientMemory() == NULL)
			fListDevice->SetClientMemory(new GClientMemory((char*) "localhost"));
		((GClientMemory*) (fListDevice->GetClientMemory()))->ResetSpectrumOnServer(
				id);

		return;
	}
	fError.TreatError(1, 0,
			"Impossible to reset a histogram which comes form a file.");
	return;

}
//______________________________________________________________________________
TNamed * GSpectra::GetSpectrumNet(GSpectrumIdentity* id) {
	//return a net spectrum
	TString SourceType;
	if (!id)
		return NULL;

	return (id->GetSpectrumNet(fListDevice));
}

//______________________________________________________________________________
TNamed * GSpectra::GetSpectrumMem(GSpectrumIdentity* id) {
	//return a net spectrum
	TString SourceType;
	if (!id)
		return NULL;
	return (id->GetSpectrumMem(fListDevice));
}
//______________________________________________________________________________
TNamed* GSpectra::GetSpectrumFile(GSpectrumIdentity* id) {
	//return a file spectrum
	TFile *mfile = NULL;
	mfile = new TFile(id->GetSourceName());
	id->ReadSpectrum(mfile);
	return (id->GetSpectrum());
}
//______________________________________________________________________________
GSpectraDB* GSpectra::GetServerDB() {
	return fListServerDB;
}

//_________________________________________________________________________________
////////////////////////////////////////fin /////////////////////////////////////
