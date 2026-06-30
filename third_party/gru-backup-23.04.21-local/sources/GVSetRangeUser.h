// File : GVSetRangeUser.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////
//
// Class GVSetRangeUser
//   Box  of GV application. Set Parameter to setup Range User
//////////////////////////////////////////////////////////////////////

#ifndef  __GVSetRangeUser__
#define  __GVSetRangeUser__


#include <TGFrame.h>
#include <TGButtonGroup.h>
#include <TGNumberEntry.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGLabel.h> 
#include <TGComboBox.h>
 

class GVSetRangeUser:public TGTransientFrame {

 private:

  TGTextButton     *fButtonOk, *fButtonCancel;
  TGButtonGroup    *fRangeRadioGroup;
  TGRadioButton    *fRadioPad,*fRadioPage,*fRadioAllPages;
  TGCompositeFrame *fGroupApplication;
  TGCompositeFrame *fGroupEndButton;
  TGGroupFrame     *fGroup,*fGroup2; //!
  
  TGCompositeFrame *fAboveFrame;

  TGMainFrame  *fMAin;
  TGNumberEntryField *fNumberEntryXMin,*fNumberEntryXMax,*fNumberEntryYMin,*fNumberEntryYMax,*fNumberEntryZMin,*fNumberEntryZMax;
  TGCheckButton * fChecksX, * fChecksY, *fChecksZ;

  TGMatrixLayout   *fLayoutGroup;//!
  TGLabel          *fLabNamePage,*fLabel1,*fLabel2,*fLabel3,*fLabel4,*fLabel5,*fLabel6,*fLabel7,*fLabel8,*fLabel9,*fLabel10;//!
  Double_t *fXmin;//!
  Double_t *fXmax;//!
  Double_t *fYmin;//!
  Double_t *fYmax;//!
  Double_t *fZmin;//!
  Double_t *fZmax;//!

  bool *fxbool;//!
  bool *fybool;//!
  bool *fzbool;//!
  int *fPadtaborall;//!
  bool fSetorunset;

  

 public:

 GVSetRangeUser(const TGWindow *p,const TGWindow *m,UInt_t w, UInt_t h, TGFrame *tg,bool *xbool,bool *ybool,bool *zbool,
			Double_t *xmin,Double_t *xmax,Double_t *ymin,Double_t*ymax,Double_t *zmin,Double_t *zmax,int *padtabeorall,bool setorunset);
  virtual       ~GVSetRangeUser();



  void DoOk();

  void DoCancel();
  void DoQuit();
  void CloseWindow();

  ClassDef(GVSetRangeUser,1)// Set Peak Find Dialog
};

#endif

  
  

  
