// File : GVSetStat.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////
//
// Class GVSetStat
//   Box  of GV application.  
//   Set configuration of statistic information on each histogram 
//////////////////////////////////////////////////////////////////////

#ifndef  __GVSetStat__
#define  __GVSetStat__


#include <TGFrame.h>
#include <TGButtonGroup.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGLabel.h> 
#include <TGComboBox.h>
#include "GError.h"

class GVSetStat:public TGTransientFrame {

 private:
 
  TGTransientFrame* fSetStat;
  TGCompositeFrame *fAboveFrame;
  TGMainFrame   *fMAin;
  TGVerticalLayout *fLayout;
  TGGroupFrame  *fGroupStatistics;
  TGCheckButton *fChecks[7];
  TGTextButton  *fButtonApply1,*fButtonApply2,*fButtonCancel;
  TGLabel       *fLabBidon;
  Pixel_t        white, black;
  GError         fError;
  TString        fStatOpt;
  TString*       pStatOpt;
  bool           *pPage; //!

 public:
  
  GVSetStat(const TGWindow *p,const TGWindow *m,UInt_t w, UInt_t h, TGFrame *tg,char*message,TString* StatOpt,bool* page);
  
  virtual       ~GVSetStat();
  
  void DoOkOne(); 
  void DoOkSeveral();

  void DoCancelo();

  void CloseWindow();

  ClassDef(GVSetStat,1)// Set Page  dialog
};

#endif

  
  

  
