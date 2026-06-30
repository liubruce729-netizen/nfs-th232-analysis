// File : GVSetPage.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////
//
// Class GVSetPage
//   Box  of GV application.
//   Ask name of page and nb of wanted pad
//////////////////////////////////////////////////////////////////////

#ifndef  __GVSetPage__
#define  __GVSetPage__


#include <TGFrame.h>
#include <TGButtonGroup.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGLabel.h>
#include <TGComboBox.h>


class GVSetPage:public TGTransientFrame {

 private:

  TGTextButton     *fButtonOk, *fButtonCancel;

   TGCompositeFrame *fAboveFrame;

   TGMainFrame  *fMAin;

  TGTextEntry      *fEntryNamePage;//!
  TGMatrixLayout   *fLayoutGroup;//!
  TGLabel          *fLabNamePage,*fLabNbPads;//!
  TGComboBox       *fComboNbPadX;//!
  TGComboBox       *fComboNbPadY;//!
  TGGroupFrame     *fGroup; //!
  Int_t            fNbPadsXOld ;
  Int_t            fNbPadsYOld ;
  TString           fPageNameOld;
  Int_t             *fNbPadsX ;   //!
  Int_t             *fNbPadsY ;   //!
  TString           *fPageName;  //!
  bool              fNewTab;
  Int_t             fNbTab;

 public:

 GVSetPage(const TGWindow *p,const TGWindow *m,UInt_t w, UInt_t h, TString message,Int_t nbTab,TString* PageName,Int_t* nbPadsX,Int_t* nbPadsY);

  virtual       ~GVSetPage();

  void DoOk();

  void DoCancel();
  void DoQuit();
  void CloseWindow();

  ClassDef(GVSetPage,1)// Set Page  dialog
};

#endif





