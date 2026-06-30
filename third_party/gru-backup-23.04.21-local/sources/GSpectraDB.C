// Author: Jerome Chauveau / Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//class GSpectraDB
// Twoo king of database are taken in account by this class
// - Array containing all spectrum identities
// - Array containing spectra indentities.
//
////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************

#include "GSpectraDB.h"

#include "Riostream.h"
#include <TGMsgBox.h>
#include <TXMLFile.h>
#include <TBuffer.h>
#include <TBufferXML.h>
#include <TXMLEngine.h>
#include <TDOMParser.h>
#include <TXMLNode.h>
#include <TROOT.h>
#include <sys/stat.h>

ClassImp( GSpectraDB)
//______________________________________________________________________________
//______________________________________________________________________________
GSpectraDB::GSpectraDB(Int_t s, Int_t lowerBound) :
	TObjArray(s, lowerBound) {
	//constructor
	SetOwner();
	fSpeIndex = 0;
}
//______________________________________________________________________________
GSpectraDB::~GSpectraDB() {
	//destructor
	gROOT->cd();
	DeleteAllIdentities(true, true);
}
//______________________________________________________________________________
void GSpectraDB::MakeDUMP() {
	// see GSpectraDB::MakeDUMP(const char* fileName, int nb, bool server)
	MakeDUMP(0);
}
//______________________________________________________________________________
void GSpectraDB::MakeDUMP(int nb) {
	// see GSpectraDB::MakeDUMP(const char* fileName, int nb, bool server)
	MakeDUMP("", nb);
}
//______________________________________________________________________________
void GSpectraDB::MakeDUMP(const char* fileName, int nb) {
	//make a DB dump in the specified filename
	// nb is the number of elemts in identity to dump (default =0 (all))
	GSpectrumIdentity id_tmp;
	GSpectrumIdentity *id = NULL;
	TestObject();
	if (GetLast() < 0) {
		fError.TreatError(0, 0, "Empty Spectra Data Base, so impossible dump");
		return;
	}

	if (strcmp(fileName, "") != 0) {
		ofstream file(fileName);
		file << (id_tmp.DumpHeader(nb)).Data();
		;
		for (Int_t i = 0; i <= GetLast(); i++) {
			id = (GSpectrumIdentity*) At(i);
			file << id->DumpId(nb).Data();
		}
		file << "DataBase Size = " << GetLast() + 1 << endl;
		file.close();
	} else {

		cout << ((id_tmp.DumpHeader(nb)).Data());
		for (Int_t i = 0; i <= GetLast(); i++) {
			id = (GSpectrumIdentity*) At(i);
			cout << id->DumpId(nb).Data();
		}
		cout << "DataBase Size = " << GetLast() + 1 << endl;
	}
}

//______________________________________________________________________________
TNamed* GSpectraDB::GetSpectrumByIndex(Int_t index) {
	//return the TH1 identified by the id parameter
	// return NULL if histogram do not existe
	GSpectrumIdentity* id = NULL;

	TestObject();

	if (GetLast() < 0) {
		fError.TreatError(1, 0,
				"Data base is empty so impossible to GetSpectrumByIndex");
		return (NULL);
	}
	if (GetLast() < index) {
		fError.TreatError(1, 0, "Index is to big to get histogram");
		return (NULL);
	}
	id = (GSpectrumIdentity*) At(index);

	if (id){
		return (id->GetSpectrum());
		}
	else
		return (NULL);

}
//______________________________________________________________________________
TNamed* GSpectraDB::GetSpectrumBySpeIndex(Int_t spe_index) {
	//return the TH1 identified by the id parameter
	// return NULL if histogram do not existe
	GSpectrumIdentity* id = NULL;

	TestObject();

	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity*) At(i);
		if (spe_index == id->GetSpeIndex())
			return (id ->GetSpectrum());
	}
	return (NULL);

}
//______________________________________________________________________________
TString GSpectraDB::GetNameByIndex(Int_t index) {
	//return the name identified by the index

	GSpectrumIdentity* id = NULL;
	TString name = "";
	id = (GSpectrumIdentity*) At(index);
	if (id)
		return (id ->GetSpectrumName());
	else
		return (name);

}
//______________________________________________________________________________

TString GSpectraDB::GetNameBySpeIndex(Int_t spe_index) {
	//return the name identified by the specific index

	GSpectrumIdentity* id = NULL;
	TString name = "";

	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity*) At(i);
		if (spe_index == id->GetSpeIndex())
			return (id ->GetSpectrumName());
	}
	TString message;
	message.Form ("No histo with specific index :%d", spe_index);
	fError.TreatError(2, -1, message);
	return (name);

}
//______________________________________________________________________________

Int_t GSpectraDB::GetSpeIndexByName(const char* namespectrum) {
	//return the specific index by the  name
	//else -1

	TestObject();

	for (Int_t i = 0; i <= GetLast(); i++) {

		if (!strcmp(namespectrum,
				((GSpectrumIdentity*) At(i))->GetSpectrumName().Data())) {
			return ((((GSpectrumIdentity*) At(i)))->GetSpeIndex());
		}
	}
	fError.TreatError(2, -1, "No spectrum with name :", namespectrum);
	return -1;
}
//______________________________________________________________________________
Int_t GSpectraDB::GetIndexByName(const char* namespectrum, const char* family) {
	//return the specific index by the  name
	//else -1
	TString tempo;
	TestObject();

	for (Int_t i = 0; i <= GetLast(); i++) {
		if ((!strcmp(namespectrum,
				((GSpectrumIdentity*) At(i))->GetSpectrumName().Data()))
				&& (!strcmp(family,
						((GSpectrumIdentity*) At(i))->GetFamily().Data())))
			return i;
	}
	tempo.Form("No histo with name/family :%s/%s", namespectrum, family);
	fError.TreatError(2, -1, tempo);
	return -1;
}
//______________________________________________________________________________
Int_t GSpectraDB::GetIndexByName(const char* namespectrum) {
	//return the specific index by the  name
	//else -1

	TestObject();

	for (Int_t i = 0; i <= GetLast(); i++) {
		if (!strcmp(namespectrum,
				((GSpectrumIdentity*) At(i))->GetSpectrumName().Data()))
			return i;
	}
	fError.TreatError(2, -1, "No histo with name :", namespectrum);
	return -1;
}

//______________________________________________________________________________
Int_t GSpectraDB::AddLastInDB(GSpectrumIdentity* id, bool quiet) {
	Int_t ret;
	ret = -1;
	TestObject();
	if (IsHistoExiste((char*) ((id->GetSpectrumName()).Data()),
			(char*) ((id->GetFamily()).Data()),
			(char*) (id->GetSourceType().Data())) >= 0)
		ret = AddIdentity(id);
	else if (!quiet)
		fError.TreatError(1, 0, (id->GetSpectrumName()).Data(),
				" : Spectrum already existes in database!");
	return ret;
}
//______________________________________________________________________________
Int_t GSpectraDB::IsHistoExiste(const char* name, const char* family,
		const char* SourceType) {
	TestObject();
	Int_t i;
	i = -1;
	for (Int_t i = 0; i <= GetLast(); i++) {
		//cout << "Debug IsHistoExiste"<<((GSpectrumIdentity*) At(i))->GetSpectrumName().Data()<<"=?"<<name<<"\n";
		if (!strcmp(name,
				((GSpectrumIdentity*) At(i))->GetSpectrumName().Data())
				&& !strcmp(family,
						((GSpectrumIdentity*) At(i))->GetFamily().Data())
				&& !strcmp(SourceType,
						((GSpectrumIdentity*) At(i))->GetSourceType()))

			return i;
	}

	return i;
}
//______________________________________________________________________________
Int_t GSpectraDB::IsHistoExiste(const char* name, const char* family,
		Int_t typespe) {
	//return  index if histo already existe in database else return -1
	//test is done on spectra name and its family name
	// if typespe =-1 then no comparaison on typespe(histotype=1=> Raw)
	// typespe!=-1 then  comparaison on typespe is done


	TestObject();
	if (typespe == -1) {
		for (Int_t i = 0; i <= GetLast(); i++) {
			if (!strcmp(name,
					((GSpectrumIdentity*) At(i))->GetSpectrumName().Data())
					&& !strcmp(family,
							((GSpectrumIdentity*) At(i))->GetFamily().Data()))

				return i;
		}
	} else {
		for (Int_t i = 0; i <= GetLast(); i++) {
			if (!strcmp(name,
					((GSpectrumIdentity*) At(i))->GetSpectrumName().Data())
					&& !strcmp(family,
							((GSpectrumIdentity*) At(i))->GetFamily().Data())
					&& (typespe == ((GSpectrumIdentity*) At(i))->GetTypeSpe()))

				return i;
		}
	}
	return -1;
}
//______________________________________________________________________________
Int_t GSpectraDB::IsServerExiste(const char* SourceName, Int_t port) {
	//return  index if server already existe in database else return -1
	//test is done on spectra name and its family name

	TestObject();

	for (Int_t i = 0; i <= GetLast(); i++) {
		GSpectrumIdentity* id = (GSpectrumIdentity*) At(i);
		if (!strcmp(SourceName, id->GetSourceName().Data()) && (port
				== id->GetPort()))
			return i;
	}
	return -1;
}
//______________________________________________________________________________
TNamed* GSpectraDB::GetSpectrumByName(const char* namehisto) {
	//return the TH1 identified by the name
	// the returned histogram is the first found with the specified name

	TestObject();

	Int_t index;
	index = GetIndexByName(namehisto);
	if (index >= 0)
		return (GetSpectrumByIndex(index));
	else {
		fError.TreatError(2, -1, "No histo with name :", namehisto);
		return NULL;
	}
}
//______________________________________________________________________________
TNamed* GSpectraDB::GetSpectrumByName(const char* namehisto, const char* family) {
	//return the TH1 identified by the name
	// the returned histogram is the first found with the specified name

	TestObject();

	Int_t index;
	index = GetIndexByName(namehisto, family);
	if (index >= 0)
		return (GetSpectrumByIndex(index));
	else {
		fError.TreatError(2, -1, "No histo with name :", namehisto);
		return NULL;
	}
}
//______________________________________________________________________________
TObjArray* GSpectraDB::GetSpectraByID(TObjArray* ids) {
	//return a list of TH1 objects identified by ids parameters

	TestObject();

	TObjArray *spectra = new TObjArray(1, 0);
	for (Int_t i = 0; i <= ids->GetLast(); i++) {
		TString s = ((TObjArray*) ids->At(i))->GetName();
		spectra->Add(GetSpectrumByIndex(s.Atoi()));
	}

	return spectra;

}

//______________________________________________________________________________
Bool_t GSpectraDB::IsFromNet(Int_t tab, Int_t pad) {
	//test if a spectrum comes from network

	TestObject();

	Bool_t result = kFALSE;
	GSpectrumIdentity* id = NULL;
	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity*) At(i);
		if (id->GetNumPad() == pad && id->GetNumTab() == tab && id->GetSource()
				== "NET")
			result = kTRUE;
	}

	return result;
}

//______________________________________________________________________________
TObjArray* GSpectraDB::GetSpectraByNumTab(Int_t numTab) {
	//return all spectra drawn on a spcified tab

	TestObject();

	TObjArray * spectra = new TObjArray(1, 0);
	GSpectrumIdentity* identity = NULL;
	for (Int_t i = 0; i <= GetLast(); i++) {
		identity = (GSpectrumIdentity*) At(i);
		if (identity->GetNumTab() == numTab)
			spectra->Add(identity);
	}
	return spectra;
}
//______________________________________________________________________________
void GSpectraDB::RazPointSpectra() {
	//Set spectra pointers to NULL.

	TestObject();

	if (GetLast() < 0) {
		fError.TreatError(1, 0,
				"Data base is empty so impossible to delete histogram");
		return;
	}
	GSpectrumIdentity* id = NULL;
	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity*) At(i);
		id->SetSpectrum(NULL);
	}
}
//______________________________________________________________________________
void GSpectraDB::DeleteSpectra() {
	//Delete all Instance of histos from identities but keep all specifications.
	//Data base is not empty after DeleteHistos()

	TestObject();

	if (GetLast() < 0) {
		fError.TreatError(1, 0,
				"Data base is empty so impossible to delete histogram");
		return;
	}
	GSpectrumIdentity* id = NULL;
	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity*) At(i);
		id->DeleteSpectrumInstance();
	}
}

//______________________________________________________________________________
void GSpectraDB::DeleteIdentity(GSpectrumIdentity* identity,
		Bool_t delete_histo) {
	//remove an identity by its Indentity
	//(Instance of histogram are also deleted if delete_histo is true)
	TestObject();
	if (identity) {
		Remove(identity);
		if (delete_histo) {
			identity->DeleteSpectrumInstance();
		}
		delete (identity);
		identity = NULL;
		Compress();
		ReIndexation();
	}
}

//______________________________________________________________________________
void GSpectraDB::DeleteIdentity(TNamed* spectrum, Bool_t delete_histo) {
	//remove an identity by its spectrum
	//(Instance of histogram are also deleted if delete_histo is true)

	TestObject();
	GSpectrumIdentity* id = NULL;
	bool somethingdone = false;
	if (spectrum) {
		for (Int_t i = 0; i <= GetLast(); i++) {
			id = (GSpectrumIdentity*) At(i);
			if (id->GetSpectrum() == spectrum) {
				Remove(id);
				if (delete_histo) {
					id->DeleteSpectrumInstance();
					spectrum = NULL;
				}
				delete (id);
				id = NULL;
				somethingdone = true;
			}
		}

		if (somethingdone)
			Compress();
		ReIndexation();
	}
}
//______________________________________________________________________________
void GSpectraDB::RemoveAllCuts() {
	//remove an identity if Spectra if is a cut
	//(Instance of spectra is also deleted )
	TNamed* sp;
	TestObject();
	if (GetLast() < 0) {
		// Data base is already empty
		return;
	}
	GSpectrumIdentity* identity = NULL;
	for (Int_t i = 0; i <= GetLast(); i++) {
		identity = (GSpectrumIdentity*) At(i);
		sp = identity->GetSpectrum();
		if (sp) {
			if (sp->InheritsFrom(TCutG::Class())) {
				identity->DeleteSpectrumInstance();
				Remove(identity);
				if (identity) {
					delete (identity);
					identity = NULL;
				}
			}
		}
	}
	Compress();
}

//______________________________________________________________________________
void GSpectraDB::DeleteIdentity(Int_t tab, Int_t pad, Bool_t delete_histo) {
	//remove an identity  from specific pad
	//(Instance of histogram are also deleted if delete_histo is true)
	TestObject();
	GSpectrumIdentity* id = NULL;
	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity*) At(i);
		if (id->GetNumTab() == tab && id->GetNumPad() == pad)//test numtab & numpad
		{
			DeleteIdentity(id, delete_histo);
		}
	}
	Compress();
}
//______________________________________________________________________________
void GSpectraDB::ChangeSpectrum(Int_t index, TNamed* spectrum) {
	// change de spectrum placed on index "index"

	GSpectrumIdentity * identity;
	identity = (GSpectrumIdentity*) At(index);
	identity->ChangeSpectrum(spectrum);
}
//______________________________________________________________________________
void GSpectraDB::ChangeSpectrumOnServer(Int_t index) {
	// change de spectrum placed on index "index"

	GSpectrumIdentity * identity;
	identity = (GSpectrumIdentity*) At(index);
	identity->ChangeSpectrumOnServer();

}
//______________________________________________________________________________
void GSpectraDB::ConstructAllRawInDB(){
	//Construc all Rawidentities in DataBase and delete all instances of histogrames
	// if  quiet = false , information message is sent
	// if raw = true ,all delete indentity , raw spectra  but not  user spectra 

	TestObject();

	GSpectrumIdentity* identity = NULL;
	
	for (Int_t i = 0; i <= GetLast(); i++) {
		identity = (GSpectrumIdentity*) At(i);
		identity->ConstructRawHisto();
		}

}
//______________________________________________________________________________
void GSpectraDB::DeleteAllIdentities(bool quiet, bool raw) {
	//remove all identities in DataBase and delete all instances of histogrames
	// if  quiet = false , information message is sent
	// if raw = true ,all delete indentity , raw spectra  but leave the user spectra 	
	TestObject();
	if (GetLast() < 0) {
		// Data base is already empty
		return;
	}
	GSpectrumIdentity* identity = NULL;
	for (Int_t i = 0; i <= GetLast(); i++) {
		identity = (GSpectrumIdentity*) At(i);
		if(raw) 
			identity->DeleteRawSpectrumInstance();
		else 
			identity->DeleteSpectrumInstance();
		Remove(identity);
		if (identity) {
			delete (identity);
			identity = NULL;
		}
	}
	Compress();
	if (!quiet)
		fError.TreatError(0, 0, "GSpectraDB is empty now");
}

//______________________________________________________________________________
void GSpectraDB::ReIndexation() {
	// Reindex all indentity
	// to do when DB have been modified

	TestObject();
	GSpectrumIdentity* identity = NULL;
	for (Int_t i = 0; i <= GetLast(); i++) {
		identity = (GSpectrumIdentity*) At(i);
		identity->SetDBIndex(i);
	}
}

//______________________________________________________________________________
void GSpectraDB::CleanIdentity(Int_t tab, Int_t pad, Bool_t delete_histo) {
	//Set identity from DB of Spectrum  specific pad
	//non viewed histogram  ( use in case of visualied histogram)
	TestObject();

	GSpectrumIdentity* id = NULL;
	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity*) At(i);
		if (id->GetNumTab() == tab && id->GetNumPad() == pad)//test numtab & numpad
		{
			id->DeleteSpectrumInstance();
			id->SetNumPad(-1);
			id->SetNumTab(-1);
		}
	}
}
//______________________________________________________________________________
GSpectrumIdentity* GSpectraDB::GetIdentity(const char* namespectrum) {
	//return identity identified by its histo pointer
	TString( tempo);
	TestObject();
	GSpectrumIdentity* id = NULL;
	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity*) At(i);
		if ((!strcmp(namespectrum, (id->GetSpectrumName().Data()))))
			return id;
	}

	tempo.Form("No histo with name :%s", namespectrum);
	fError.TreatError(2, -1, tempo);
	;
	return NULL;
}
//______________________________________________________________________________
GSpectrumIdentity* GSpectraDB::GetIdentity(const char* namespectrum,
		const char* family) {
	//return identity identified by its histo pointer
	TString( tempo);
	TestObject();
	GSpectrumIdentity* id = NULL;
	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity*) At(i);
		if ((!strcmp(namespectrum, (id->GetSpectrumName().Data())) && (!strcmp(
				family, id->GetFamily().Data()))))
			return id;
	}

	tempo.Form("No histo with name/family :%s/%s", namespectrum, family);
	fError.TreatError(2, -1, tempo);
	;

	return NULL;
}
//______________________________________________________________________________
GSpectrumIdentity* GSpectraDB::GetIdentity(Int_t tab, Int_t pad) {
	//return identity identified by its histo pointer

	TestObject();

	GSpectrumIdentity* id = NULL;
	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity*) At(i);
		if (id->GetNumTab() == tab && id->GetNumPad() == pad)
			return id;
	}
	return NULL;
}
//______________________________________________________________________________
GSpectrumIdentity* GSpectraDB::GetIdentity(TNamed* sp) {
	//return identity identify by its place in the GUI (tab,pad)

	TestObject();

	GSpectrumIdentity* id = NULL;
	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity*) At(i);
		if (id->GetSpectrum() == sp)
			return id;
	}
	return NULL;
}
//______________________________________________________________________________
GSpectrumIdentity* GSpectraDB::GetIdentity(Int_t index) {
	//return identity which has the correct index

	TestObject();

	return ((GSpectrumIdentity*) At(index));
}

//______________________________________________________________________________
Int_t GSpectraDB::AddIdentity(TNamed *sp, TString n, TString sName,
		TString sType, TString source, Int_t port, TString fam, Int_t nTab,
		Int_t nPad, bool start) {
	//add an Identity to the DB
	//if name and family already exist, addition is rejected
	GSpectrumIdentity * id;
	Int_t ret = -1;
	TestObject();
	TString tempo;
	if (sp)
	 tempo = sp->GetName();
	 else
		 tempo =n;

	if (IsHistoExiste((char*) tempo.Data(), (char*) fam.Data()) < 0) {
		id = new GSpectrumIdentity(sp, n, sName, sType, source, port, fam,
				nTab, nPad, GetLast() + 1, start);

		AddIdentity(id);
		ret = GetLast();
	} else {
		tempo += " ";
		tempo += fam;
	}
	if (ret < 0)
		fError.TreatError(1, 0, "Spectrum not added in Data Base :",
				(char*) tempo.Data());
	return (ret);
}
//______________________________________________________________________________
Int_t GSpectraDB::AddIdentity(GSpectrumIdentity * id) {
	//add an Identity to the DB
	//if name and family already exist, addition is rejected and return -1
	TestObject();
	bool refresh_spectra=false;
	Int_t ret = -1;
	TString tempo = id->GetSpectrumName();
	TString tempo2 = id->GetFamily();
	//char *fam = (char*)(id->GetFamily().Data());
	int number_exists =IsHistoExiste((char*) tempo.Data(), (char*) tempo2.Data());
	if (number_exists < 0) {
		MyAddLast(id);
		ReIndexation();
		ret = GetLast();
	} else {
		tempo += " ";
		tempo += tempo2;
		if (((GetIdentity(number_exists)->GetSpectrum())!=id->GetSpectrum())and refresh_spectra ){
			GetIdentity(number_exists)->SetSpectrum(id->GetSpectrum()) ;
		}
	}

	if (ret < 0) {
		fError.TreatError(
				1,
				0,
				"GSpectraDB::AddIdentity(GSpectrumIdentity * id)  : Spectrum not added in Data Base :",
				(char*) tempo.Data());
				if (refresh_spectra) fError.TreatError(1,0," But This spectrum refreshed");

	}
	return (ret);
}
//______________________________________________________________________________
Int_t GSpectraDB::AddServerIdentity(TString sName, TString sType,
		TString source, Int_t port) {
	//add an Identity to the DB with only server information
	TestObject();
	TString tempo = "";
	Int_t ret = -1;

	if (IsServerExiste((char*) sName.Data(), port) < 0) {
		GSpectrumIdentity *id = new GSpectrumIdentity(sName, sType, source,
				port);
                id->SetTypeSpe(SERVERTYPE );
		MyAddLast(id);
		ret = GetLast();
	} else {
		tempo += sName + ":";
		tempo += port;
	}
	if (ret < 0) {
		fError.TreatError(1, 0, "Server not added in Data Base :",
				(char*) tempo.Data());
	}
	return (ret);
}

//______________________________________________________________________________
void GSpectraDB::MyAddLast(GSpectrumIdentity *id) {
	// add id on last position and control fSpeIndex
	TestObject();
	fSpeIndex++;

	id->SetSpeIndex(fSpeIndex);
	AddLast(id);
}
//______________________________________________________________________________
Int_t GSpectraDB::GetSpeIndex(TNamed* sp) {
	//return specific index of the GSpectrumIdentity which has the TH1 pointer parameter
	// return -1 if nothing is found

	TestObject();

	GSpectrumIdentity* identity = NULL;
	if (sp == NULL) {
		//cout << "Impossible to get  specific index in Database\n";
		return (-1);
	}
	for (Int_t i = 0; i <= GetLast(); i++) {
		identity = (GSpectrumIdentity*) At(i);
		if (identity->GetSpectrum() == sp)
			return identity->GetSpeIndex();
	}
	return -1;
}
//______________________________________________________________________________
Int_t GSpectraDB::GetIndex(TNamed* sp) {
	//return  index of the GSpectrumIdentity which has the TH1 pointer parameter
	// return -1 if nothing is found

	TestObject();

	GSpectrumIdentity* identity = NULL;
	if (sp == NULL) {
		//cout << "Impossible to get  index in Database\n";
		return (-1);
	}
	for (Int_t i = 0; i <= GetLast(); i++) {
		identity = (GSpectrumIdentity*) At(i);
		if (identity->GetSpectrum() == sp)
			return i;
	}
	return -1;
}
//______________________________________________________________________________
Color_t GSpectraDB::IsLineColorAlreadyExiste(Int_t indice) {
	// find if the color if identity At(indice) already existe from At(0) to At(indice)
	// return, in case of, the hightest color found else -1
	GSpectrumIdentity * id;
	id = (GSpectrumIdentity*) At(indice);
	return (IsLineColorAlreadyExiste(id), indice);

}
//______________________________________________________________________________
Color_t GSpectraDB::IsLineColorAlreadyExiste(GSpectrumIdentity* id,
		Int_t avoid_this_indice) {
	// find if the color  identity id already existe
	// return, in case of, the hightest color found else -1

	TestObject();

	GSpectrumIdentity * id2;
	Color_t color, color2, color_max;
	color = 0;
	color2 = 0;
	color_max = -1;
	bool samecolor;
	samecolor = false;
	Int_t max;

	if (avoid_this_indice > 0)
		max = avoid_this_indice - 1;
	else
		max = GetLast();
	color = id->GetLineColor();
	if (color >= 0) {
		for (Int_t j = 0; j <= max; j++) {
			if (avoid_this_indice != j) {
				id2 = (GSpectrumIdentity*) At(j);
				color2 = id2->GetLineColor();
				if (color_max < color2)
					color_max = color2;
				if (color2 == color)
					samecolor = true;
			}
		}
	}
	if (samecolor)
		color = color_max;
	else
		color = -1;
	return color;
}
//______________________________________________________________________________
void GSpectraDB::SaveXML(TString fileName) {
	//save DB in a XML file

	TestObject();

	TString Ttmp = "";
	// First create engine
	TXMLEngine* xml = new TXMLEngine;
	// Create main node of document tree
	XMLNodePointer_t mainnode = xml->NewChild(0, 0, "root");
	CreateXML(xml, 0, false);

	XMLDocPointer_t xmldoc = xml->NewDoc();
	xml->DocSetRootElement(xmldoc, mainnode);
	// Save document to file, 2 is step in text struicture
	xml->SaveDoc(xmldoc, fileName.Data(), 2);
	// Release memory before exit
	xml->FreeDoc(xmldoc);
	delete xml;
}

//______________________________________________________________________________
Int_t GSpectraDB::CreateXMListedByTab(TXMLEngine *xml, XMLNodePointer_t node,
		Int_t tab, Int_t nb) {
	//Creat XML output of database with condition on number of tab
	//return number of  created entries
	// tab : no of tab (page)
	// nn : nb of pad in page
	TString tempo = nb;
	GSpectrumIdentity * id;
	Int_t i, j;
	i = 0;
	j = 0;

	TestObject();
	tempo.Form("%d", nb);
	XMLNodePointer_t NodeSpectrum = xml->NewChild(node, 0, "ListIdSpectra",
			tempo.Data());
	if (GetLast() < 0)
		return i;
	//	if (nb > GetLast()) nb = GetLast();
	while (nb > 0) {

		id = (GSpectrumIdentity *) At(j);

		if (j > GetLast())
			break;
		j++;
		if (id != NULL) {
			if (id->GetNumTab() == tab) {
				id->CreatXML(xml, NodeSpectrum);
				nb--;
				i++;
			}
		} else {
			fError.TreatError(1, GetLast(),
					"Return from CreateXMListedByTab with wrong identity.");
			return i;
		}
	}
	return i;
}
//______________________________________________________________________________
Int_t GSpectraDB::ReadXMListedByTab(TXMLEngine *xml, XMLNodePointer_t node,
		Int_t tab, Int_t nb) {
	//
	Int_t nbId;
	nbId = 0;
	TString tempo, tempo2;
	GSpectrumIdentity * id;
	//bool server;

	tempo2 = xml->GetNodeContent(node);
	tempo = xml->GetNodeName(node);
	//if (tempo == "ListIdServers") server = true;

	XMLNodePointer_t NodeId = xml->GetChild(node);
	for (Int_t i = 0; i < (Int_t) tempo2.Atoi(); i++) {
		id = new GSpectrumIdentity();
		id->ReadXML(xml, NodeId);
		if (id != NULL) {
			if (tab != id->GetNumTab())
				fError.TreatError(1, 0,
						"Get a GSpectrumIdentity with no matching tab");
			MyAddLast(id);
			nbId++;
		} else
			return nbId;
		NodeId = xml->GetNext(NodeId);
	}
	return nbId;
}
//______________________________________________________________________________
int GSpectraDB::CreateXML(TXMLEngine *xml, XMLNodePointer_t node, bool server) {
	// creat xml out put of a database
	TString tempo, tempo2;
	GSpectrumIdentity * id;
	Int_t j;
	j = 0;
	if (GetLast() < 0)
		return j;
	tempo = "";
	tempo.Form("%d", GetLast());
	if (server)
		tempo2 = "ListIdServers";
	else
		tempo2 = "ListIdSpectra";
	XMLNodePointer_t NodeList = xml->NewChild(node, 0, tempo2.Data(),
			tempo.Data());
	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity *) At(i);
		id->CreatXML(xml, NodeList);
		j++;
	}
	return j;
}
//______________________________________________________________________________
Int_t GSpectraDB::ReadXML(TXMLEngine *xml, XMLNodePointer_t node, bool server) {
	// read xml out put of a database
	TString tempo, tempo2;
	GSpectrumIdentity * id;
	server = false;
	Int_t nbId;
	nbId = 0;
	tempo2 = xml->GetNodeContent(node);
	tempo = xml->GetNodeName(node);

	if (tempo == "ListIdServers") {
		server = true;
	}
	Int_t ni = tempo2.Atoi();
	XMLNodePointer_t ServerId = xml->GetChild(node);

	for (Int_t i = 0; i <= ni; i++) {

		id = new GSpectrumIdentity();
		id->ReadXML(xml, ServerId);

		if (id) {
			MyAddLast(id);
			nbId++;
		} else
			return nbId;
		ServerId = xml->GetNext(ServerId);
	}

	return nbId;
}

//______________________________________________________________________________

void GSpectraDB::SaveSpectra(TFile *file, Int_t type) {

	// Save spectra of DB in root format
	// a file is supposed to be open
	// to select specific type set type = 1,2,3...  see GSpectrumIdentity.h
	// default type = 0 => all
	//

	if (GetLast() < 0) {
		fError.TreatError(1, 0, "Database is empty");
		return;
	}
	if (!file) {
		fError.TreatError(1, 0, "Root file is not open");
		return;
	}
	GSpectrumIdentity * id;

	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity *) At(i);
		if (id)
			id->SaveSpectrum(file, type);
	}
}

//______________________________________________________________________________
void GSpectraDB::SaveSpectraTxt(ofstream *file) {
	// Save spectra of DB in txt format
	// a file is supposed to be open

	GSpectrumIdentity * id;
	TString tempo;
	if (!file) {
		fError.TreatError(1, 0, "GVPad::SaveSpectraTxt Root file is not open");
		return;
	}
	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity *) At(i);
		if (id) {
			id->SaveSpectrumTxt(file);
		}

	}
}
//______________________________________________________________________________

Bool_t GSpectraDB::ReadSpectra(TFile *file, Int_t type) {
	//  read spectra of DB from root format
	// a file is supposed to be open
	// if  a read succeed return true
	// if type >0 select specific spectra ( see GSepectrumIdentity.h)
	Bool_t ref;
	ref = kFALSE;
	if (!file) {
		fError.TreatError(1, 0, "GSpectraDB::ReadSpectra Root file is not open");
		return ref;
	}

	GSpectrumIdentity * id;
	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity *) At(i);
		if (id) {
			ref = id->ReadSpectrum(file, type);
		}
	}
	return ref;
}

//______________________________________________________________________________
void GSpectraDB::UpdateFromXML(TXMLEngine* xml, XMLNodePointer_t node) {
	//
	TString tempo;
	TestObject();

	XMLNodePointer_t child = xml->GetChild(node);
	TString name = xml->GetNodeContent(child);
	child = xml->GetNext(child);
	TString source = xml->GetNodeContent(child);
	child = xml->GetNext(child);
	TString sourceType = xml->GetNodeContent(child);
	child = xml->GetNext(child);
	TString sourceName = xml->GetNodeContent(child);
	child = xml->GetNext(child);
	tempo = xml->GetNodeContent(child);
	Int_t port = (Int_t) tempo.Atoi();
	child = xml->GetNext(child);
	TString family = xml->GetNodeContent(child);

	GSpectrumIdentity *identity = NULL;

	for (Int_t i = 0; i <= GetLast(); i++) {
		identity = (GSpectrumIdentity*) At(i);
		if (identity->HasProperties(name, source, sourceName, sourceType, port,
				family) && identity->GetFamily() != "")
			identity->SetFamily(family);
	}

}
//______________________________________________________________________________
void GSpectraDB::ReplaceDB(GSpectraDB *DB) {
	// replace DB
	//DB is copied with spectra


	TestObject();

	GSpectrumIdentity * id1, *id2;
	id1 = NULL;
	id2 = NULL;
	if (DB == NULL)
		return;
	DeleteAllIdentities();
	for (Int_t i = 0; i <= DB->GetLast(); i++) {
		id1 = (GSpectrumIdentity*) DB->At(i);
		id2 = new GSpectrumIdentity(id1, true);
		AddIdentity(id2);
	}
}
//______________________________________________________________________________
void GSpectraDB::ReplaceDBOnlySpectra(GSpectraDB *DB) {
	// replace DB
	//DB is copied with spectra
	// only instance of spectra are copied not identity

	TestObject();

	GSpectrumIdentity * id1, *id2;
	id1 = NULL;
	id2 = NULL;
	if (DB == NULL)
		return;
	if (DB->GetLast() ==0) return;
	if (DB->GetLast() == GetLast()) {
		for (Int_t i = 0; i <= DB->GetLast(); i++) {
			id1 = (GSpectrumIdentity*) DB->At(i);
			id2 = (GSpectrumIdentity*) At(i);
			if (id1 && id2) {
				id2->SaveDims();
				id2->SaveLineColor();
				id2 ->ChangeSpectrum(
						(TNamed*) ((TObject*) (id1->GetSpectrum())->Clone()));
				if (id2->GetSpectrum() == NULL) {
					fError.TreatError(1, 0, "Spectrum not cloned ");
				}
				id2->ReportXYMinMax();
				id2->SetLineColor(id2->GetLineColor());
			}
		}
	} else {
		fError.TreatError(2, 0,
				"GSpectraDB::ReplaceOnlySpectraDB number of identities are not equal");
	}

}
//______________________________________________________________________________
void GSpectraDB::UpdateFromDB(GSpectraDB *DB) {
	//
	TestObject();

	GSpectrumIdentity * id1;
	id1 = NULL;
	if (DB == NULL)
		return;
	for (Int_t i = 0; i <= DB->GetLast(); i++) {
		id1 = (GSpectrumIdentity*) DB->At(i);
		AddIdentity(id1);
	}

}

//______________________________________________________________________________
TCutG* GSpectraDB::GetCut() {
	TCutG *currentCut, *CutDB;
	currentCut = NULL;
	if (GetLast() < 0)
		return currentCut;
	GSpectrumIdentity * id;

	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity*) (At(i));

		if (id->GetSpectrum())
			if (id->GetSpectrum()->InheritsFrom(TCutG::Class())) {
				CutDB = (TCutG*) (id->GetSpectrum());

				id->DumpId();
				currentCut = (TCutG*) (FindObject(id->GetSpectrumName()));
				if (currentCut == NULL)
					currentCut = CutDB;
			}
	}
	return currentCut;
}
//______________________________________________________________________________
GSpectraDB* GSpectraDB::CloneSpectraNull() {
	// do a copy of a GSpectraDB object
	// but all histogram pointer are null

	TestObject();

	GSpectraDB* newDB;
	newDB = new GSpectraDB();
	GSpectrumIdentity * tempoID;
	for (int i = 0; i <= GetLast(); i++) {
		tempoID = new GSpectrumIdentity(GetIdentity(i), false);
		tempoID->SetSpectrum(NULL);
		newDB->MyAddLast(tempoID);
	}
	newDB->ReIndexation();
	return newDB;
}
//______________________________________________________________________________
void GSpectraDB::SetPortDB(Int_t port) {
	Int_t i;
	GSpectrumIdentity * identity;
	for (i = 0; i <= GetLast(); i++) {
		identity = (GSpectrumIdentity*) At(i);
		identity->SetPort(port);
	}

}
//______________________________________________________________________________
void GSpectraDB::SetSourceDB(TString source) {
	Int_t i;
	GSpectrumIdentity * identity;
	for (i = 0; i <= GetLast(); i++) {
		identity = (GSpectrumIdentity*) At(i);
		identity->SetSource(source);
	}

}//______________________________________________________________________________
void GSpectraDB::SetSourceNameDB(TString sourcename) {
	Int_t i;
	GSpectrumIdentity * identity;
	for (i = 0; i <= GetLast(); i++) {
		identity = (GSpectrumIdentity*) At(i);
		identity->SetSourceName(sourcename);
	}

}
//______________________________________________________________________________
void GSpectraDB::RazDB() {
	for (int i = 0; i <= GetLast(); i++) {
		GSpectrumIdentity * id;
		id = (GSpectrumIdentity*) At(i);
		if (id)
			(id->Reset());
	}
}
//______________________________________________________________________________
void GSpectraDB::SetStartedDB(Bool_t start) {
	Int_t i;
	GSpectrumIdentity * identity;
	for (i = 0; i <= GetLast(); i++) {
		identity = (GSpectrumIdentity*) At(i);
		identity->SetStarted(start);
	}
}
//______________________________________________________________________________
void GSpectraDB::InstanceRawHistoFromAction() {
	// Set to false all action flags of identity
	TestObject();
	GSpectrumIdentity * identity;
	Int_t i;
	for (i = 0; i <= GetLast(); i++) {
		identity = (GSpectrumIdentity*) At(i);
		if (identity->GetTypeSpe() == 1)
			identity->SetAction(true);
	}
	ResetActions();
}

//______________________________________________________________________________
void GSpectraDB::ResetActions() {
	// Set to false all action flags of identity
	TestObject();
	GSpectrumIdentity * identity;

	for (int i = 0; i <= GetLast(); i++) {
		identity = (GSpectrumIdentity*) At(i);
		identity->SetAction(false);
	}
}

//______________________________________________________________________________
void GSpectraDB::TestObject() {
	// test existence of this object
	// if this doesn't exist message is sent and application is stopped
	if (this == NULL) {
		GError error;
		error.TreatError(4, -1,
				"Inexistent Data Base, so impossible to use it!");
		return;
	}
}

//______________________________________________________________________________
void GSpectraDB::ByName(int premier, int dernier) {
	int vmin = premier;
	int vmax = dernier;
	GSpectrumIdentity * spectreSep;
	spectreSep = (GSpectrumIdentity *) At((premier + dernier) / 2);
	TString name = spectreSep->GetSpectrumName();
	GSpectrumIdentity * temp;
	do {
		TString TempName;
		TempName = ((GSpectrumIdentity *) At(vmin))->GetSpectrumName();
		while (((name.CompareTo(TempName)) == 1) && (vmin <= vmax)) {
			vmin++;
			TempName = ((GSpectrumIdentity *) At(vmin))->GetSpectrumName();
		}

		TempName = ((GSpectrumIdentity *) At(vmax))->GetSpectrumName();
		while (((TempName.CompareTo(name)) == 1) && (vmax > 0)) {
			vmax--;
			TempName = ((GSpectrumIdentity *) At(vmax))->GetSpectrumName();
		}
		if (vmin <= vmax) {
			temp = (GSpectrumIdentity *) At(vmin);
			AddAt(At(vmax), vmin++);
			AddAt(temp, vmax--);
		}
	} while (vmin <= vmax);

	if (premier < vmax) {
		ByName(premier, vmax);
	}
	if (vmin < dernier) {
		ByName(vmin, dernier);
	}
}

//______________________________________________________________________________
void GSpectraDB::Reverse() {
	int x = GetLast();
	int milieu = x / 2;
	int i = 0;
	while ((i <= milieu) || (x >= milieu)) {
		GSpectrumIdentity * temp;
		temp = (GSpectrumIdentity *) At(x);
		AddAt(At(i), x);
		AddAt(temp, i);
		i++;
		x--;
	}
}

//______________________________________________________________________________
void GSpectraDB::ByPage(int premier, int dernier) {
	int vmin = premier;
	int vmax = dernier;
	GSpectrumIdentity * spectreSep;
	spectreSep = (GSpectrumIdentity *) At((premier + dernier) / 2);
	GSpectrumIdentity * temp;
	do {
		while ((((GSpectrumIdentity *) At(vmin))->GetNumTab()
				< spectreSep->GetNumTab()) && (vmin <= vmax)) {
			vmin++;
		}

		while ((((GSpectrumIdentity *) At(vmax))->GetNumTab()
				> spectreSep->GetNumTab()) && (vmax >= 0)) {
			vmax--;
		}
		if (vmin <= vmax) {
			temp = (GSpectrumIdentity *) At(vmin);
			AddAt(At(vmax), vmin++);
			AddAt(temp, vmax--);
		}
	} while (vmin <= vmax);
	if (premier < vmax) {
		ByName(premier, vmax);
	}
	if (vmin < dernier) {
		ByName(vmin, dernier);
	}
}
//______________________________________________________________________________
Bool_t GSpectraDB::ResetAllSpectraOnServer(const TGWindow* main,
		Bool_t question) {

	GSpectrumIdentity * idserver;
	TString tempo;
	bool ref;
	ref = false;

	for (Int_t i = 0; i <= GetLast(); i++) {

		idserver = (GSpectrumIdentity *) At(i);

		if (idserver) {

			tempo = idserver->GetSourceName();
			Int_t retval = 1;
			if (question) {
				EMsgBoxIcon icontype = kMBIconQuestion;
				tempo.Form(
						"Do you really want reset All spectra from this server :%s",
						tempo.Data());
				new TGMsgBox(gClient->GetRoot(), main, "Warning", tempo,
						icontype, 3, &retval);
			}
			if (retval == 1) {
				idserver->ResetAllSpectraOnServer();
				ref = true;
			}
		}
	}

	return ref;
}
//______________________________________________________________________________
Bool_t GSpectraDB::ResetSpectra(const TGWindow* main, Bool_t question) {

	GSpectrumIdentity * id;
	TString tempo;
	bool ref;
	ref = false;

	for (Int_t i = 0; i <= GetLast(); i++) {

		id = (GSpectrumIdentity *) At(i);

		if (id) {
			tempo = id->GetSpectrumName();
			Int_t retval = 1;
			if (question) {
				EMsgBoxIcon icontype = kMBIconQuestion;
				tempo.Form("Do you really want reset this histogram :%s",
						tempo.Data());
				new TGMsgBox(gClient->GetRoot(), main, "Warning", tempo,
						icontype, 3, &retval);

			}
			if (retval == 1) {
				if (id->GetSpectrum() !=NULL) {
					id->ResetSpectrumOnServer();
					ref = true;
				}
			}
		}
	}
	return ref;
}

//______________________________________________________________________________
void GSpectraDB::CancelZoom() {
	// apply CancelZoom on all DB
	GSpectrumIdentity * id;
	Int_t k;

	for (k = 0; k <= GetLast(); k++) {
		id = (GSpectrumIdentity *) At(k);
		id->CancelZoom();
	}
}
//______________________________________________________________________________
void GSpectraDB::ApplyZoom(Int_t xmin_pad, Int_t xmax_pad, Int_t ymin_pad,
		Int_t ymax_pad) {
	Int_t k;
	GSpectrumIdentity * id;
	for (k = 0; k <= GetLast(); k++) {
		id = (GSpectrumIdentity *) At(k);
		id->ApplyZoom(xmin_pad, xmax_pad, ymin_pad, ymax_pad);
	}
}
//______________________________________________________________________________
void GSpectraDB::PeakFind(Int_t NbPeaks, Float_t Resolution, Double_t Sigma,
		Double_t Threshold, bool Display_polymarker) {
	// find peak on spectra
	Int_t k;
	GSpectrumIdentity * id;

	for (k = 0; k <= GetLast(); k++) {
		id = (GSpectrumIdentity *) At(k);
		id->PeakFind(NbPeaks, Resolution, Sigma, Threshold, Display_polymarker);
	}
}
//______________________________________________________________________________
void GSpectraDB::IntegralsInCuts(TObjArray * listcuts) {
	// give integral of histogram inside cuts

	Int_t i, k;
	GSpectrumIdentity * idi;

	TCutG* cutk;
	TObject* objk;

	for (k = 0; k < listcuts->GetSize(); k++) {
		objk = (TObject*) (listcuts->At(k));
		if (objk != NULL) {
			if (objk->InheritsFrom(TCutG::Class())) {
				cutk = (TCutG*) objk;
				for (i = 0; i <= GetLast(); i++) {
					idi = (GSpectrumIdentity *) At(i);
					idi->IntegralsInCuts(cutk);
				}
			}
		}
	}
}
//______________________________________________________________________________
void GSpectraDB::Refresh(bool getanewone, int tab, int nbpad, TString option2D,
		GListDevice* listdev) {
	bool force_same;
	force_same = false;
	TNamed *spnew = NULL;
	GSpectrumIdentity * id;
	for (Int_t i = 0; i <= GetLast(); i++) {
		id = (GSpectrumIdentity *) At(i);
		force_same = (!(i == 0));
		if (id == NULL)
			break;
		spnew = id->GetSpectrum();//get old value
		id->SaveOption();
		if (getanewone) {
			spnew = NULL;
			spnew = (TNamed *) id->GetSpectrumNetOrFileOrMem(listdev);
			if (spnew == NULL) {
				fError.TreatError(2, 0,
						"GSpectraDB::RefreshPad : histogram = NULL");
				return;
			}
			id->SetSpectrum(spnew);
		};

		id->SetNumPad(nbpad);
		id->SetNumTab(tab);
		if (spnew == NULL) {
			//fError.TreatError(2, 0, "GSpectraDB::RefreshPad : histogram = NULL!");
			//return;
		}

		id->MyDraw(force_same, option2D);
	}
}
//______________________________________________________________________________
void GSpectraDB::FFT() {

	GSpectrumIdentity * id;
	for (Int_t k = 0; k <= GetLast(); k++) {
		id = (GSpectrumIdentity *) At(k);
		id->SaveOption();
		id->FFT();
		id->RestaureOption();
	}
}
//______________________________________________________________________________
void GSpectraDB::FFThalf() {

	GSpectrumIdentity * id;
	for (Int_t k = 0; k <= GetLast(); k++) {
		id = (GSpectrumIdentity *) At(k);
		id->SaveOption();
		id->FFThalf();
		id->RestaureOption();
	}
}
//______________________________________________________________________________
void GSpectraDB::ZeroLess() {

	GSpectrumIdentity * id;
	for (Int_t k = 0; k <= GetLast(); k++) {
		id = (GSpectrumIdentity *) At(k);
		id->ZeroLess();
	}
}
//______________________________________________________________________________
void GSpectraDB::Scatter() {

	GSpectrumIdentity * id;
	for (Int_t k = 0; k <= GetLast(); k++) {
		id = (GSpectrumIdentity *) At(k);
		id->SaveOption();
		id->Scatter();
		id->RestaureOption();
	}
}
//______________________________________________________________________________
void GSpectraDB::ProjectionX() {

	GSpectrumIdentity * id;
	for (Int_t k = 0; k <= GetLast(); k++) {
		id = (GSpectrumIdentity *) At(k);
		id->SaveOption();
		id->ProjectionX();
		id->RestaureOption();
	}
}
//______________________________________________________________________________
void GSpectraDB::ProjectionY() {

	GSpectrumIdentity * id;
	for (Int_t k = 0; k <= GetLast(); k++) {
		id = (GSpectrumIdentity *) At(k);
		id->SaveOption();
		id->ProjectionY();
		id->RestaureOption();
	}
}
//______________________________________________________________________________
void GSpectraDB::ProfileX() {

	GSpectrumIdentity * id;
	for (Int_t k = 0; k <= GetLast(); k++) {
		id = (GSpectrumIdentity *) At(k);
		id->SaveOption();
		id->ProfileX();
		id->RestaureOption();
	}
}
//______________________________________________________________________________
void GSpectraDB::ProfileY() {

	GSpectrumIdentity * id;
	for (Int_t k = 0; k <= GetLast(); k++) {
		id = (GSpectrumIdentity *) At(k);
		id->SaveOption();
		id->ProfileY();
		id->RestaureOption();
	}
}
//______________________________________________________________________________
void GSpectraDB::AutoZoom() {
	Int_t xmin, xmax, ymin, ymax, k;
	Int_t xmin_pad = 0, xmax_pad = 0, ymin_pad = 0, ymax_pad = 0;

	GSpectrumIdentity * id;

	for (k = 0; k <= GetLast(); k++) {
		id = (GSpectrumIdentity *) At(k);

		id->AutoZoom(&xmin, &xmax, &ymin, &ymax);

		if (k == 0) {
			xmin_pad = xmin;
			xmax_pad = xmax;
			ymin_pad = ymin;
			ymax_pad = ymax;
		} else {
			if (xmin_pad > xmin)
				xmin_pad = xmin;
			if (xmax_pad < xmax)
				xmax_pad = xmax;
			if (ymin_pad > ymin)
				ymin_pad = ymin;
			if (ymax_pad < ymax)
				ymax_pad = ymax;
		}
	}
	ApplyZoom(xmin_pad, xmax_pad, ymin_pad, ymax_pad);
}
//______________________________________________________________________________
void GSpectraDB::ApplyZoomProportional(Int_t xmin, Int_t xmax, Int_t ymin,
		Int_t ymax) {

	GSpectrumIdentity * id;
	Int_t k;
	for (k = 0; k <= GetLast(); k++) {
		id = (GSpectrumIdentity *) At(k);
		id->ApplyZoomProportional(xmin, xmax, ymin, ymax);
	}
}
//______________________________________________________________________________
void GSpectraDB::ApplySetRangeUser(bool inx,bool iny,bool inz, Double_t xmin, Double_t xmax,
		Double_t ymin,Double_t ymax,Double_t zmin, Double_t zmax) {

	GSpectrumIdentity * id;
	Int_t k;
	for (k = 0; k <= GetLast(); k++) {
		id = (GSpectrumIdentity *) At(k);
		id->ApplySetRangeUser( inx, iny, inz,  xmin,  xmax,ymin, ymax, zmin,  zmax);
	}
}
//______________________________________________________________________________
TString GSpectraDB::GetSpectraList() {
	TString stretrun;
	GSpectrumIdentity *id;
	stretrun = "";
	for (Int_t i = 0; i <= GetLast(); i++) {
		id =(GSpectrumIdentity*) At(i);
		stretrun += id->GetFamily();
		stretrun += "/";
		stretrun += id->GetSpectrumName();
		stretrun += " ";
	}
	return stretrun;
}
//______________________________________________________________________________

void GSpectraDB::SpectraList() {
	fError.Infos("---List of Spectra---");
	fError.Infos(GetSpectraList());
}

