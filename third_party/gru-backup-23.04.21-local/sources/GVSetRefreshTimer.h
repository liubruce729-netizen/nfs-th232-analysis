// File : GVSetRefreshTimer.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////
//
// Class GVSetRefreshTimer
//   Box  of GV application.  
//   Ask name of page and nb of wanted pad 
//////////////////////////////////////////////////////////////////////

#ifndef  __GVSetRefreshTimer__
#define  __GVSetRefreshTimer__

#include <TGFrame.h>
#include <TGButtonGroup.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGLabel.h> 
#include <TGComboBox.h>

class GVSetRefreshTimer : public TGTransientFrame {

private:

	TGTextButton *fButtonOk, *fButtonCancel;
	TGCompositeFrame *fAboveFrame;
	TGMainFrame *fMAin;
	Int_t *fNb;//!
	TGMatrixLayout *fLayoutGroup;//!
	TGLabel *fLabName;//!
	TGComboBox *fComboNb;//! 
	TGGroupFrame *fGroup; //!


public:

	GVSetRefreshTimer(const TGWindow *p, const TGWindow *m, UInt_t w, UInt_t h,
			TGFrame *tg, Int_t* nb);

	virtual ~GVSetRefreshTimer();

	void DoOk();
	void DoCancel();
	void DoQuit();
	void CloseWindow();

	ClassDef(GVSetRefreshTimer,1)// Set Page  dialog
};

#endif

