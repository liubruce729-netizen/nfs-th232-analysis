// File : GVSetPeakFind.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////
//
// Class GVSetPeakFind
//   Box  of GV application.  
//   Ask name of page and nb of wanted pad 
//////////////////////////////////////////////////////////////////////

#ifndef  __GVSetPeakFind__
#define  __GVSetPeakFind__


#include <TGFrame.h>
#include <TGButtonGroup.h>
#include <TGNumberEntry.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGLabel.h> 
#include <TGComboBox.h>
 

class GVSetPeakFind:public TGTransientFrame {

 private:

  TGTextButton     *fButtonOk, *fButtonCancel;

  TGCompositeFrame *fAboveFrame;

  TGMainFrame  *fMAin;
  TGNumberEntry *fNumberEntryResolution,*fNumberEntryThreshold,*fNumberEntrySigma,*fNumberEntryNbPeak;
  TGCheckButton * fChecks;

  TGMatrixLayout   *fLayoutGroup;//!
  TGLabel          *fLabNamePage,*fLabel1,*fLabel2,*fLabel3,*fLabel4,*fLabel5,*fLabel6;//!
  Int_t* fnbPeaks;//!
  Float_t *fresolution;//!
  Double_t *fsigma;//!
  Double_t *fthreshold;//!
  bool *fdisplay_polymarker;//!
  TGGroupFrame     *fGroup,*fGroup2; //!
  

 public:

 GVSetPeakFind(const TGWindow *p,const TGWindow *m,UInt_t w, UInt_t h, TGFrame *tg, Int_t* nbPeaks,Float_t *resolution ,Double_t *sigma,Double_t *threshold,bool *display_polymarker);
  virtual       ~GVSetPeakFind();

  void DoOk();

  void DoCancel();
  void DoQuit();
  void CloseWindow();

  ClassDef(GVSetPeakFind,1)// Set Peak Find Dialog
};

#endif

  
  

  
