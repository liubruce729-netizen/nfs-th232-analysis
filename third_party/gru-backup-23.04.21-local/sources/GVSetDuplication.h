// File : GVSetDuplication.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////
//
// Class GVSetDuplication
//   Box  of GV application.
//   Ask the king of Computation have to be done
//   and form which pad ( page, no pad)
//////////////////////////////////////////////////////////////////////

#ifndef  __GVSetDuplication__
#define  __GVSetDuplication__

#include <TGFrame.h>
#include <TGButtonGroup.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <TGComboBox.h>

class GVSetDuplication: public TGTransientFrame {

private:

	TGTextButton *fButtonOk, *fButtonCancel;

	TGMainFrame *fMAin;
	TGMatrixLayout *fLayoutGroup;//!
	TGLabel *fLabelPad, *fLabelTab;//!

	TGNumberEntry *fComboPad;//!
	TGNumberEntry *fComboTab;//!
	TGComboBox *fComboComputation;//!

	TGGroupFrame *fGroup; //!

	Int_t fNbTab;
	Int_t fTab, fPad, fComputation;
	Int_t *fpTab;//!!
	Int_t *fpPad;//!
	Int_t *fpComputation;//!
public:

	GVSetDuplication(const TGWindow *p, const TGWindow *m, UInt_t w, UInt_t h,
			Int_t* tab, Int_t* pad, Int_t* computation);

	virtual ~GVSetDuplication();

	void DoOk();

	void DoCancel();
	void DoQuit();
	void CloseWindow();

ClassDef(GVSetDuplication,1)// Set Page  dialog
};

#endif

