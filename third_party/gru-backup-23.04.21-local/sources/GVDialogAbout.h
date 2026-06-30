#ifndef __GVDialogAbout__
#define __GVDialogAbout__

#include <TGLabel.h>
#include <TGFrame.h>
#include "General.h"
class GVDialogAbout: public TGTransientFrame
{


private:
  TGLabel *fLabAbout1, *fLabAbout2;
  TGMainFrame *fMain;

public:

  GVDialogAbout(const TGWindow *p,const TGWindow *m,UInt_t w, UInt_t h);
  virtual void CloseWindow();
  virtual ~GVDialogAbout();
  Pixel_t white, black;
  ClassDef(GVDialogAbout,1)//About

};
#endif
