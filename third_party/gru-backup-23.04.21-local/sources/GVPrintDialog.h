#ifndef  __GVPrintDialog__
#define  __GVPrintDialog__


#include <TGFrame.h>
#include <TGButtonGroup.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGLabel.h>
#include <TGString.h>

class GVPrintDialog:public TGTransientFrame {
 private:

  TGGroupFrame *fConfiguration;

  TGButtonGroup *fPrint, *fPrintTo;

  TGRadioButton *fRadioPrinter, *fRadioFile, *fRadioPage, *fRadioPad;

  TGTextButton  *fButtonOk,*fButtonCancel;

  TGTextEntry   *fEntryPrinterName;

  TGCompositeFrame *fButtons;

  TGMainFrame* fMainBase ;

  TGLabel       *fLabelPrinter;

  TGHotString   *fRadioPageString, *fRadioFileString,*fRadioPrinterString,*fRadioPadString;

  Pixel_t        white,black;

 public:

  GVPrintDialog(const TGWindow *p,const TGWindow *m,UInt_t w, UInt_t h);

  virtual ~GVPrintDialog();

  void DoOk();

  void DoCancel();
  void  SetPrinter();
  void  SetFile();

  void CloseWindow();

  ClassDef(GVPrintDialog,1)//Print dialog
};

#endif





