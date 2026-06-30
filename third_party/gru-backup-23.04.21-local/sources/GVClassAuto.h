#ifndef __GVClassAuto__
#define __GVClassAuto__

#include <TGLabel.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>
#include <TGButtonGroup.h>
#include <TObjArray.h>


#include "GVBrowser.h"
#include "GVListTree.h"
#include "GVClassManual.h"
#include "GVBrowser.h"
#include "GSpectraDB.h"

class GVClassAuto: public TGCompositeFrame 
{
  
 private:

  TGLabel *labParent, *lab1, *lab2, *lab3,*lab4,*lab5,*lab6,*lab7,*lab8,*lab9,*lab10,*lab11,*lab12;

  TGRadioButton *fRadio1,   *fRadio2, *fRadio3;

  TGNumberEntry   *fEntry1,   *fEntry2, *fEntry3, *fEntry4, *fEntry5,*fEntry6,*fEntry8;

  TGTextEntry *fEntryParent,*fEntry7, *fEntryA, *fEntryB,*fEntry9,*fEntry10;

  TGTextButton  *fButtonOk, *fButtonReset;

  GVListTree *tree;

  GSpectraDB *spectraDB;

  TGCompositeFrame *fFrameParent,*fCenter, *fFrame1, *fFrame2, *fFrame3, *fOptions, *fButtons, *fOption1, *fOption2, *fOption3;

  Int_t option;

  GVClassManual *manual;

  TObjArray *families;

  GVBrowser *fBrowser;

  Pixel_t lightgrey,white;

 public:
  
  GVClassAuto(const TGWindow *p,UInt_t w, UInt_t h,GVClassManual* manual,GVBrowser *browser,GSpectraDB *DB);

  virtual ~GVClassAuto();

  void ClickOpt1();

  void ClickOpt2();

  void ClickOpt3();

  void AutoClassOpt1();

  void AutoClassOpt2();

  void AutoClassOpt3();

  void DoOk();

  ClassDef(GVClassAuto,1)
};

#endif
