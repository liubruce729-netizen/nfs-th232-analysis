// File : GVSetReset.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////
//
// Class GVSetReset
//   Box  of GV application.
//   Set configuration of Reset of spectra
//////////////////////////////////////////////////////////////////////

#ifndef  __GVSetReset__
#define  __GVSetReset__


#include <TGFrame.h>
#include <TGButtonGroup.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGLabel.h>
#include <TGComboBox.h>
#include "GError.h"

class GVSetReset:public TGTransientFrame {

 private:

  TGTransientFrame* fSetStat;
  TGCompositeFrame *fButtons;
  TGMainFrame   *fMAin;
  TGVerticalLayout *fLayout;
  TGGroupFrame  *fGroupResets;
  TGCheckButton *fCheck;

  TGRadioButton *fRadioPad;
  TGRadioButton *fRadioPage;
  TGRadioButton *fRadioServers;

  TGButtonGroup *fResetRadioGroup;

  TGTextButton  *fButtonApply1,*fButtonCancel;
  TGLabel       *fLabBidon;
  Pixel_t        white, black;
  GError         fError;
  TString        fResetOpt;
  TString*       pResetOpt;


 public:

  GVSetReset(const TGWindow *p,const TGWindow *m,UInt_t w, UInt_t h,char*message,TString* resetopt);

  virtual       ~GVSetReset();

  void DoOkOne();


  void DoCancelo();

  void CloseWindow();

  ClassDef(GVSetReset,1)// Set Page  dialog
};

#endif





