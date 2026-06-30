// File : .h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////
//
// Class GVMyQuestion
//   Box  of GV application.
//   fListOfQuestion Input is fListOfQuestion separated by ";"
//   out output number of selected answer (0-> n)
//////////////////////////////////////////////////////////////////////

#ifndef  __GVMyQuestion__
#define  __GVMyQuestion__


#include <TGFrame.h>
#include <TGButtonGroup.h>
#include <TGLayout.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGLabel.h>
#include <TGComboBox.h>
#include <TString.h>
#include "GError.h"

class GVMyQuestion:public TGTransientFrame {

 private:

  TGTextButton      **fButton;//!
  TGCompositeFrame  *fAboveFrame;//!
  TGVerticalFrame   *fLabelFrame ;//!
  TGLayoutHints     *fL4,*fL2,*fL1;//!
  TList             *fMsgList ;//!
  TGMainFrame       *fMAin;
  TGHorizontalFrame *fGroup; //!
  TString           fListOfQuestion;// list of questions
  Int_t             fNbofQuestions;
  Int_t             *fPtNbofQuestions;//!
  Int_t             fSelectedButton;
  GError            fError;
 public:

GVMyQuestion(const TGWindow *p,const TGWindow *m, TString questions,TString messageWindow, TString MessageBox,Int_t* keyreturn,UInt_t w=1, UInt_t h=1);

  virtual ~GVMyQuestion();

  void DoQuit();
  void DoOk0();
  void DoOk1();
  void DoOk2();
  void DoOk3();
  void DoOk4();
  void DoOk5();
  void DoOk6();
  void DoOk7();
  void DoOk8();
  void DoOk9();
  void DoOk10();
  void DoOk11();
  void DoOk12();

  void CloseWindow();

  ClassDef(GVMyQuestion,1)// Set question  dialog
};

#endif





