#ifndef __GVListTree__
#define __GVListTree__

#include <TGListTree.h>
#include <TGMenu.h>
#include <TGFrame.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TGTextEntry.h>
#include "GSpectraDB.h"
#include "GError.h"

class GVListTree: public TGListTree
{

 private:

  TGCompositeFrame *fBrowser;
  TObjArray        *fListFamily,*fListFamily_current_path;
  Pixel_t          orange;
  TGTextEntry      *fTextEntry;
  TGPopupMenu      *fMenu;
  const TGPicture  *fPict1, *fPict2;
  GSpectraDB       *fSpectraDB;
  Bool_t            fBox;
  GError            fError;
  Int_t             fTypeTree;
  Int_t             fTypeOfSpectra;
  Int_t             fNumSpectra;

 public:

  GVListTree(TGWindow *p =NULL ,UInt_t w=1, UInt_t h=1,UInt_t options=0,TGCompositeFrame *browser=NULL,Int_t typeTree=1,Int_t typeOfSpectra=1);

  virtual ~GVListTree();

  void DeleteItem(TGListTreeItem *item);

  Bool_t HandleButton(Event_t *event);

  void HandleMenu(Int_t id);

  TGListTreeItem* AddFamily(TGListTreeItem *item, const char* name);

  Bool_t IsSelectedFamily();

  Int_t  FillFromDB(GSpectraDB *DB);

  TGListTreeItem* RecoverParent(TString path);

  void CheckedItem(TGListTreeItem *item);

  TObjArray* GetSelectedItemsID();

  TObjArray* GetSelectedItemID();

  int GetSelectedItemInD();

  void CheckChecked(TGListTreeItem *item,TObjArray* list);

  TObjString* GetStringTipID(TGListTreeItem *item);

  Int_t GetTipID(TGListTreeItem *item);

  void SetCheckBoxes(Bool_t b);

  void SetSpectraDB(GSpectraDB* DB);

  void SetCheck(TGListTreeItem *item,Bool_t b);

  void Clean(TGListTreeItem *item);

  void SetTextEntry(TGTextEntry *entry);

  void AddItemFromId(GSpectrumIdentity* id);

  ClassDef(GVListTree,1)//List Tree which contains spectra families

};
#endif
