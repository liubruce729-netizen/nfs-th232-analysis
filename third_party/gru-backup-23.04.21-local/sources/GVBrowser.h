#ifndef __GVBrowser__
#define __GVBrowser__

#include <TGFrame.h>
#include "GSpectraDB.h"
#include "GVListTree.h"
#include <TCanvas.h>
#include <TGTextEntry.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <TObjArray.h>

class GVBrowser: public TGCompositeFrame
{
  
  
private:
  
  TCanvas *fTreeView;

  GSpectraDB *fSpectraDB;
  
  GVListTree *fListTree;

  TGTextEntry *fNameFamily;

  TGLabel *fLabFamily;

  TGCompositeFrame *fName;

  TGTextButton *fButtonName;

  TObjArray *fListTrees;

  Pixel_t lightgrey;

public:
  
  GVBrowser(const TGWindow *p,UInt_t w, UInt_t h,GSpectraDB *DB);

  virtual ~GVBrowser();

  void AddItem(GSpectrumIdentity* identity,TString folder);

  void EnableFamilyEdit();

  void AddFamily();

  void AddTreeToFill(GVListTree *tree);

  Bool_t IsPossibleToAdd();

  GVListTree* GetTree();

  void SeClassified(TString name,Bool_t classed);

  ClassDef(GVBrowser,1)//Frame with a GVListTree an a field to create families 

};
#endif
