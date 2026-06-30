// File : GVBraowser.C
// Author: Jerome Chauveau & Luc Legeard

//////////////////////////////////////////////////////////////////////////////
//class GVBrowser
//Frame with a GVListTree an a field to create families
//
//
////////////////////////////////////////////////////////////////////////////


// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#include "GVBrowser.h"
#include "GSpectrumIdentity.h"
#include <Riostream.h>
#include <TGPicture.h>
#include <TGMsgBox.h>

ClassImp(GVBrowser)

GVBrowser::GVBrowser(const TGWindow *p, UInt_t w, UInt_t h, GSpectraDB *DB) :
	TGCompositeFrame(p, w, h) {

	fSpectraDB = DB;

	gClient->GetColorByName("#e0e0e0", lightgrey);
	//canvas containing the tree
	TGCanvas *fTreeView = new TGCanvas(this, 200, 500,
			kSunkenFrame | kDoubleBorder);
	//tree
	fListTree = new GVListTree(fTreeView->GetViewPort(),
			10, 10, kHorizontalFrame,this);
	fTreeView->SetContainer(fListTree);
	//add the canvas in browser frame
	AddFrame(fTreeView);

	fLabFamily = new TGLabel(this,"Family name :");
	AddFrame(fLabFamily);

	//frame family name
	fName = new TGCompositeFrame(this);
	fName->SetLayoutManager(new TGHorizontalLayout(fName));
	fNameFamily = new TGTextEntry(fName,"");
	fButtonName = new TGTextButton(fName,"ok");
	fName->AddFrame(fNameFamily);
	fName->AddFrame(fButtonName);

	AddFrame(fName);

	SetBackgroundColor(lightgrey);
	fName->SetBackgroundColor(lightgrey);
	fLabFamily->SetBackgroundColor(lightgrey);

	fNameFamily->SetState(kFALSE);
	fButtonName->SetEnabled(kFALSE);

	fButtonName->Connect("Clicked()", "GVBrowser", this, "AddFamily()");
	   GetClient()->WaitFor(this);
}
//_____________________________________________________________________________________________________________________________
GVBrowser::~GVBrowser() {
	DeleteWindow();
	/*if(fListTree)
	 delete fListTree;
	 cout<<"3"<<endl;
	 if(fNameFamily)
	 delete fNameFamily;
	 cout<<"4"<<endl;
	 if(fButtonName)
	 delete fButtonName;
	 if(listTrees)
	 {
	  listTrees->Delete();
	  if(listTrees) delete listTrees;
	 listTrees = NULL;
	 }
	 */
}
//_____________________________________________________________________________________________________________________________
void GVBrowser::AddItem(GSpectrumIdentity *identity, TString folder) {
	TString item = identity->GetSpectrumName();
	TString source = identity->GetSource();
	GSpectrumIdentity *id;
/*
	const TGPicture* pict;
	if (source == "NET")
		pict = gClient->GetPicture("marker29.xpm");
	else
		pict = gClient->GetPicture("marker30.xpm");
*/

	TGListTreeItem *it= NULL;


			fListTree->AddItemFromId(identity);

			char path[32];
			strcpy(path, "");
			fListTree->GetPathnameFromItem(it, path);
			TString str = path;
			TString family = "";
			TObjArray *array = str.Tokenize("/");
			for (Int_t i =0; i<array->GetLast(); i++) {
				family+="/";
				family+=((TObjString*)array->At(i))->GetName();
			}
			if (identity->GetFamily()=="Raw")
				identity->SetFamily(family);
			else {
				id = new GSpectrumIdentity(identity->GetSpectrum(),identity->GetSpectrumName(),identity->GetSourceName(),identity->GetSourceType(),
						identity->GetSource(),identity->GetPort(),family,-1,-1,fSpectraDB->GetLast()+1);
				fSpectraDB->MyAddLast(id);
				fSpectraDB->ReIndexation();
			}


	gClient->NeedRedraw(fListTree);
}
//_____________________________________________________________________________________________________________________________
void GVBrowser::EnableFamilyEdit() {
	fNameFamily->SetState(kTRUE);
	fButtonName->SetEnabled(kTRUE);
}
//_____________________________________________________________________________________________________________________________
void GVBrowser::AddFamily() {
	//TGPicture p = "pouet.xpm";
	TString selected = fListTree->GetSelected()->GetText();
	if (selected!="Raw") {
		fListTree->AddFamily(fListTree->GetSelected(), fNameFamily->GetText());
		/* GVListTree *t = (GVListTree*)listTrees->At(i);
		 char toto[32];
		 strcpy (toto,"");
		 fListTree->GetPathnameFromItem(fListTree->GetSelected(),toto);
		 TGListTreeItem *item2 = t->FindItemByPathname(toto);
		 t->AddFamily(item2,fNameFamily->GetText());
		 gClient->NeedRedraw(t);*/

		gClient->NeedRedraw(fListTree);
	}
	fNameFamily->SetState(kFALSE);
	fNameFamily->SetText("");
	fButtonName->SetEnabled(kFALSE);
}
//_____________________________________________________________________________________________________________________________

void GVBrowser::AddTreeToFill(GVListTree *tree) {
	fListTrees->Add(tree);

}
//_____________________________________________________________________________________________________________________________
Bool_t GVBrowser::IsPossibleToAdd() {
	return fListTree->IsSelectedFamily();
}
//_____________________________________________________________________________________________________________________________
GVListTree* GVBrowser::GetTree() {
	return fListTree;
}

//_____________________________________________________________________________________________________________________________
void GVBrowser::SeClassified(TString name, Bool_t classed) {
	if (classed==kTRUE) {
		TGListTreeItem *it = fListTree->FindItemByPathname("Raw/"+name);
		it->SetColor(3);
		fListTree->SetColorMode(TGListTree::kColorUnderline);
	} else {
		cout<<"DELETE "<<name<<endl;
		TGListTreeItem *it = fListTree->FindItemByPathname("Raw/"+name);
		if (it)
			it->SetColor(2);
		fListTree->SetColorMode(TGListTree::kColorUnderline);
	}
}
//_____________________________________________________________________________________________________________________________
