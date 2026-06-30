// Author: Jerome Chauveau & Luc Legeard

//////////////////////////////////////////////////////////////////////////////
//class GVListTree
//List Tree which contains spectra families
//
//
////////////////////////////////////////////////////////////////////////////
//_________________________________________________________________________________________
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
 */

//imports
#include "GVListTree.h"
#include <Riostream.h>
#include <TGMsgBox.h>
#include <TGButton.h>
#include <TVirtualX.h>
#include "GVBrowser.h"
#include "GSpectrumIdentity.h"

ClassImp( GVListTree)

enum ECommandIdentifiers {
	M_MENU_DELETE, M_MENU_CANCEL, M_MENU_ADD,
};

//_________________________________________________________________________________________
//GVListTree::GVListTree(TGWindow* p = 0, UInt_t w = 1, UInt_t h = 1,
//		UInt_t options = 0, TGCompositeFrame *browser = 0, Int_t typeTree,
//		Int_t typeOfSpectra) :
//	TGListTree(p, w, h, options) {
GVListTree::GVListTree(TGWindow* p , UInt_t w , UInt_t h ,
		UInt_t options , TGCompositeFrame *browser , Int_t typeTree,
		Int_t typeOfSpectra) :
	TGListTree(p, w, h, options) {

	// constructor
	// Creat a graphical list of name of histograms
	// typeTree design the type of information that will be display on the tree
	// typeTree  = 1 => list of histograms ( no duplicata) sorted by familly
	// typeTree  = 2 => list of displayed histograms sorted by tab
	// typeTree  = 3 => list of histograms no sorted
	// Note : "fSelected" is TGListTreeItem pointer to selected item in list

	fBrowser = NULL;
	fListFamily = NULL;
	fListFamily_current_path = NULL;
	fMenu = NULL;
	fSpectraDB = NULL;
	fBrowser = browser;
	fBox = kTRUE;
	fTextEntry = NULL;
	fTypeTree = typeTree;
	fTypeOfSpectra = typeOfSpectra;

	fNumSpectra = 0;

	fPict1 = gClient->GetPicture("ofolder_t"); // picture of open folder
	fPict2 = gClient->GetPicture("folder_t"); // picture of close folder

	//react to mouse clicks
	gVirtualX->GrabButton(fId, kAnyButton, kAnyModifier, kButtonPressMask
			| kButtonReleaseMask, kNone, kNone);

	//popup
	fMenu = new TGPopupMenu(gClient->GetRoot());

	fListFamily = new TObjArray(1, 0);
	fListFamily_current_path = new TObjArray(1, 0);

	fListFamily->SetOwner();
	fListFamily_current_path->SetOwner();

	if (fBrowser) {//if browser(user can add families)
		fMenu->AddEntry("Cancel", M_MENU_CANCEL, 0, gClient->GetPicture(
				"tb_back.xpm")); // arrow
		fMenu->AddSeparator();
		fMenu->AddEntry("Add new child family", M_MENU_ADD, 0,
				gClient->GetPicture("tb_newfolder.xpm")); // folder (new folder)
		fMenu->AddSeparator();
		fMenu->AddEntry("Delete", M_MENU_DELETE, 0, gClient->GetPicture(
				"sm_delete.xpm")); //red "St André" cross
		fMenu->Connect("Activated(Int_t)", "GVListTree", this,
				"HandleMenu(Int_t)");
	}

}
//_________________________________________________________________________________________
GVListTree::~GVListTree() {
	Cleanup();
}

//_________________________________________________________________________________________
Bool_t GVListTree::HandleButton(Event_t *event) {
	//get event form mouss

	TGListTree::HandleButton(event);
	//only react on right button
	if (event->fCode == kButton3 && fSelected != NULL) {
		if (fBrowser != NULL)//in this case, the GVListTree is made to construct a tree structure
		{
			TString parent = "";
			if (fSelected->GetParent())
				parent = fSelected->GetParent()->GetText();
			TString selected = fSelected->GetText();
		} else {
			fMenu->PlaceMenu(event->fXRoot, event->fYRoot, kFALSE, kTRUE);
		}
	} else {
		if (fTextEntry) {
			char path[32];
			strcpy(path, "");
			GetPathnameFromItem(fSelected, path);
			fTextEntry->SetText(path);
		}
	}

	if (fBrowser == NULL)//in this case, the GVlistTree is made to applicate an action on selected nodes
	{
		if (fSelected != NULL) {
			if (fSelected->GetFirstChild() != NULL)
				CheckedItem(fSelected->GetFirstChild());
		}
	}
	gClient->NeedRedraw(this);
	return kTRUE;
}

//_________________________________________________________________________________________
void GVListTree::SetSpectraDB(GSpectraDB* DB) {
	fSpectraDB = DB;
}
//_________________________________________________________________________________________
void GVListTree::HandleMenu(Int_t id) {
	//popup handle

	switch (id) {
	case M_MENU_DELETE:
		DeleteItem(fSelected);
		break;
	case M_MENU_ADD:
		((GVBrowser*) fBrowser)->EnableFamilyEdit();
		break;
	}
}

//_________________________________________________________________________________________
void GVListTree::DeleteItem(TGListTreeItem *item) {
	//delete item
	TString name = item->GetText();

	//if (name !="Raw" && name!="Classified" /*&&RecoverParent(item)!=fFolderRaw*/) {
	((GVBrowser*) fBrowser)->SeClassified(item->GetText(), kFALSE);
	DeleteItem(item);
	gClient->NeedRedraw(this);

	//}
}
//_________________________________________________________________________________________

TGListTreeItem *GVListTree::AddFamily(TGListTreeItem *item, const char* name) {
	//add a new family

	TGListTreeItem *it = AddItem(item, name, fPict1, fPict2, fBox);
	if (fBox)
		it->CheckItem(kFALSE);
	fListFamily->Add(new TObjString(it->GetText()));
	return it;
}

//_________________________________________________________________________________________
Bool_t GVListTree::IsSelectedFamily() {
	//test is the selected is a family(!=spectrum)

	if (!fSelected)
		return kFALSE;
	else
		return fListFamily->Contains(fSelected->GetText());
}
//_________________________________________________________________________________________
void ResetTree() {
	//Reset tree and delete all items

	//TGListTree::this->DeleteItem(0);
}
//_________________________________________________________________________________________
Int_t GVListTree::FillFromDB(GSpectraDB *DB) {
	//fill the tree with a  GSpectraDB
	// if OnlyDisplayed is true only displayed
	// return number of build items
	Int_t NumberItems;
	ResetTree();
	NumberItems = 0;
	if (DB == NULL)
		return NumberItems;
	SetSpectraDB(DB);
	GSpectrumIdentity *id = NULL;

	for (Int_t i = 0; i <= fSpectraDB->GetLast(); i++) {
		id = (GSpectrumIdentity*) fSpectraDB->At(i);
		if (fTypeTree == 1) {
			//if (id->GetClone() <= 0) { // not a clone
			if (fTypeOfSpectra == 0) {
				AddItemFromId(id);
				NumberItems++;
			}
			if ((fTypeOfSpectra == 1) and (id ->GetTypeSpe() != 3)) {
				AddItemFromId(id);
				NumberItems++;
			}
			if ((fTypeOfSpectra == 2) and (id ->GetTypeSpe() == 3)) {
				AddItemFromId(id);
				NumberItems++;
			}
			//}

		}
		if (fTypeTree == 2) {
			if (id->GetNumTab() != -1 && id->GetSpectrum() != NULL) {
				if (fTypeOfSpectra == 0) {
					AddItemFromId(id);
					NumberItems++;
				}
				if ((fTypeOfSpectra == 1) and (id ->GetTypeSpe() != 3)) {
					AddItemFromId(id);
					NumberItems++;
				}
				if ((fTypeOfSpectra == 2) and (id ->GetTypeSpe() == 3)) {
					AddItemFromId(id);
					NumberItems++;
				}

			}
		}
		if (fTypeTree == 3) {
			if (id->GetNumTab() != -1 && id->GetSpectrum() != NULL) {
			}
			if (fTypeOfSpectra == 0) {
				AddItemFromId(id);
				NumberItems++;
			}
			if ((fTypeOfSpectra == 1) and (id ->GetTypeSpe() != 3)) {
				AddItemFromId(id);
				NumberItems++;
			}
			if ((fTypeOfSpectra == 2) and (id ->GetTypeSpe() == 3)) {
				AddItemFromId(id);
				NumberItems++;
			}

		}
	}
	return NumberItems;
}
//_________________________________________________________________________________________
void GVListTree::AddItemFromId(GSpectrumIdentity* id) {
	const TGPicture* pict;


	if (id->GetSource() == "NET")
		pict = gClient->GetPicture("h1_t.xpm");
		//pict = gClient->GetPicture("marker29.xpm"); //dark star
	else
		pict = gClient->GetPicture("fdisk_t.xpm"); //whire star


	TString directory;
	directory = "";

	if (fTypeTree == 1) {
		directory = id->GetFamily();
	}

	if (fTypeTree == 2) {
		directory.Form("Page_%d", id->GetNumTab());
	}

	if (fTypeTree == 3) {
		directory = "";
	}

	TGListTreeItem *currentfolder = RecoverParent(directory);
	TGListTreeItem *it = AddItem(currentfolder, id->GetSpectrumName(), pict,
			pict, fBox);
	if (fBox)
		it->CheckItem(kFALSE);
	it->SetTipText((id->ToString()).Data());

}
//_________________________________________________________________________________________
TGListTreeItem* GVListTree::RecoverParent(TString path) {
	//find the parent folder or create it if doesnt exist

	TGListTreeItem *item = FindItemByPathname(path.Data());

	TGListTreeItem *it = NULL;

	if (item) {
		return item;
	}
	bool creation = false;
	TString current;
	TString current_path;
	TString parent = "";
	TString fam = "";

	TGListTreeItem *it2 = NULL;
	TObjArray *array = path.Tokenize("/");
	Int_t end = array->GetLast();
	for (Int_t i = 0; i <= end; i++) {
		current = ((TObjString*) array->At(i))->GetName();

		//recover parent folder

		for (Int_t a = 0; a < i; a++) {
			parent += "/";
			parent += ((TObjString*) array->At(a))->GetName();
		}
		current_path = parent + "/" + current;
		creation = false;

		if (!(fListFamily_current_path)->Contains(current_path)) {
			fListFamily_current_path->Add(new TObjString(current_path.Data()));
			creation = true;
		} else {
			//cout<<"this folder already existe!=  "<<current <<" in " << path.Data()<<endl;
		}
		it2 = FindItemByPathname(parent.Data());
		if (creation)
			AddFamily(it2, current.Data());

		parent = "";

	}
	it = FindItemByPathname(path.Data());

	if (fTypeTree == 1) {
	}
	if (fTypeTree == 2) {
	}

	return it;
}

//_________________________________________________________________________________________
TObjArray* GVListTree::GetSelectedItemsID() {
	//get selected items

	TObjArray* IDs = new TObjArray(1, 0);

	//	CheckChecked(fFolderClassified, IDs);

	CheckChecked(0, IDs);
	return IDs;

}
//_________________________________________________________________________________________
TObjArray* GVListTree::GetSelectedItemID() {
	//get selected item
	TObjArray* ID = new TObjArray(1, 0);//will contains only one id
	if (fSelected) {
		ID->Add(GetStringTipID(fSelected));
	}
	return ID;
}
//_________________________________________________________________________________________
int GVListTree::GetSelectedItemInD() {
	Int_t value;
	value = -1;
	if (fSelected) {
		if (fSpectraDB != NULL) {
			Int_t indice = GetTipID(fSelected);
			if (indice >= 0) {
				((GSpectrumIdentity *) (fSpectraDB->At(indice)))->SetAction(
						true);
				value = indice;
			}
		}
	}
	return value;
}

//_________________________________________________________________________________________
void GVListTree::CheckChecked(TGListTreeItem *item, TObjArray* list) {
	//fill the list with selected items
	//recursive method

	if (item == NULL)
		item = fFirst;
	Int_t max = fSpectraDB->GetLast();
	if (max < 0) {
		fError.TreatError(2, max,
				"Empty Spectra Data Base,So nothing is selected");
		return;
	}
	if (item == NULL)
		return;
	if (item->IsChecked() && item->GetFirstChild() == NULL
			&& (!fListFamily->Contains(item->GetText()))) {
		if (fSpectraDB != NULL) {
			if ((GetTipID(item)) <= max) {
				list->Add(GetStringTipID(item));
				((GSpectrumIdentity *) (fSpectraDB->At((GetTipID(item)))))->SetAction(
						true);

			} else {
				fError.TreatError(2, 0,
						"Error in GVListTree::CheckChecked: index not valid ");
			}
		}
	}

	if (item->GetFirstChild()) {

		CheckChecked(item->GetFirstChild(), list);
	}
	if (item->GetNextSibling()) {

		CheckChecked(item->GetNextSibling(), list);
	}

}
//_________________________________________________________________________________________
void GVListTree::CheckedItem(TGListTreeItem *item) {
	//recursive check boxes
	if (fSelected->IsChecked())
		item->CheckItem(kTRUE);
	else
		item->CheckItem(kFALSE);
	if (item->GetFirstChild())
		CheckedItem(item->GetFirstChild());
	if (item->GetNextSibling())
		CheckedItem(item->GetNextSibling());
}
//_________________________________________________________________________________________
TObjString* GVListTree::GetStringTipID(TGListTreeItem *item) {
	//return ID of an item
	//return item id stored in its tooltip
	TString tip = item->GetTipText();
	TString separator = " ; ";
	TObjArray *tab = tip.Tokenize(separator);
	return (TObjString*) tab->Last();
}
//_________________________________________________________________________________________
Int_t GVListTree::GetTipID(TGListTreeItem *item) {
	//return ID of an item
	//return item id stored in its tooltip
	TString temp;
	temp = "-1";
	TString tip = item->GetTipText();
	TString separator = " ; ";
	if (tip.Contains(separator)) {
		TObjArray *tab = tip.Tokenize(separator);
		temp = (tab->Last())->GetName();
	}
	return (temp.Atoi());
}
//_________________________________________________________________________________________
void GVListTree::SetCheckBoxes(Bool_t b) {
	//set check
	fBox = b;
}
//_________________________________________________________________________________________
void GVListTree::SetCheck(TGListTreeItem *item, Bool_t b) {
	//
	item->SetCheckBox(b);
	if (item->GetFirstChild())
		SetCheck(item->GetFirstChild(), b);
	if (item->GetNextSibling())
		SetCheck(item->GetNextSibling(), b);
}
//_________________________________________________________________________________________

void GVListTree::Clean(TGListTreeItem *item) {
	//clean the tree
	// remove spectra from the tree structure, only folder are keeped

	if (item != NULL) {
		if (fListFamily->Contains(item->GetText())) {
			Clean(item->GetFirstChild());
			Clean(item->GetNextSibling());
		}

		else {
			RemoveReference(item);
			Clean(item->GetNextSibling());
		}
	}

}
//_________________________________________________________________________________________

void GVListTree::SetTextEntry(TGTextEntry *entry) {
	//give an entry to fill with the name of the selected item
	fTextEntry = entry;
}
