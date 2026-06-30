#ifndef __GVClass__
#define __GVClass__

#include "GVClassAuto.h"
#include "GVBrowser.h"
#include "GVClassManual.h"
//#include "GSpectraDB.h"

#include <TGTab.h>
#include <TGFrame.h>

class GVClass: public TGTransientFrame
{

 private:
  
  GSpectraDB *spectraDB;

  GVClassManual *fManual;

  GVClassAuto *fAuto;

  TGTab *tab;

  TGCompositeFrame *tabAuto, *tabManual;

  TGGroupFrame *fGroupBrowser;

  GVBrowser *fBrowser;

  Pixel_t lightgrey;
  
 public:

  GVClass(GSpectraDB *DB,UInt_t w = 920, UInt_t h = 800);

  virtual ~GVClass();

  void CloseWindow();

  ClassDef(GVClass,1)

};
#endif
