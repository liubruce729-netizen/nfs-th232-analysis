#ifndef __GVClassManual__
#define __GVClassManual__

#include <TGLabel.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TObjArray.h>
#include <TGScrollBar.h>
#include <TGCanvas.h>

#include "GVBrowser.h"
#include "GSpectraDB.h"

class GVClassManual: public TGCompositeFrame 
{
  
  
private:

  TGLabel *fTest;

  TGTextButton *fButtonOk, *fButtonCancel;

  TGPictureButton *fButtonAdd, *fButtonRemove;

  TGCheckButton *fCheckAll;

  TGCompositeFrame *fButtons, *fCenter, *fTableSpectras, *fArrows, *fTitles, *fScrollTable;

  TGCanvas *fCanvas;

  GVBrowser *fBrowser;

  GSpectraDB *spectraDB;

  TGMainFrame *fMain;

  TObjArray  *spectrasInfoList;

  TGGroupFrame *fGroupTable;

  TGVScrollBar *vScroll;
  
  Pixel_t lightgrey,white,black;

public:
  
  GVClassManual(const TGWindow *p,UInt_t w, UInt_t h,GVBrowser *browser,GSpectraDB* DB);

  virtual ~GVClassManual();

  void CloseWindow();

  void DoCancel();

  void DoOk();

  void AddSpectra();

  void ReConstruct();
  
  void Show();

  void FillTableSpectras(TObjArray *list, Int_t type, Bool_t append);

  void FillFromDB(GSpectraDB *DB);

  void SelectAll();

  void AddTreeToFill(GVListTree *tree);

  TObjArray * TestFill(TString str);

  TObjArray* GetSpectraNames();

  void RemoveSpectra();

  ClassDef(GVClassManual,1)//Frame to manual classification. User can choose spectra which he wants to class

};
#endif
