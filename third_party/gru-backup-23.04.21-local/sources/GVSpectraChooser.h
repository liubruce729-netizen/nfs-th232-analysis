#ifndef  __GVSpectraChooser__
#define  __GVSpectraChooser__

#include <TGFrame.h>
#include <TGCanvas.h>
#include <TGButton.h>
#include <TString.h>

#include "GSpectraDB.h"
#include "GVListTree.h"

class GVSpectraChooser : public TGTransientFrame {

private:

	TGButtonGroup *fOrdre;
	TGRadioButton *fCroissant;
	TGRadioButton *fDecroissant;

	TGButtonGroup *fChoixType;
	TGRadioButton *fName;
	TGRadioButton *fFamily;
	TGRadioButton *fPage;
    TObjArray* fListId;
	TGCanvas *fTreeView;

	TGTextButton *fButtonOk, *fButtonCancel;
	TGButtonGroup *fGroupOperation;
	TGRadioButton *fRadioReplace;
	TGRadioButton *fRadioSame;
	TGRadioButton *fRadioAddition;
	TGRadioButton *fRadioSubtraction;

	Int_t fMode; //selected action mode ( visualisation, logarithme, zoom..)

	int fTypeTree;
    int fTypeOfSpectra;// 0 all, 1 only Spectra, 2 Only Tcut
	TGMainFrame *fMain;

	GVListTree *fListTree;

	Int_t *fOneselected; //!
	Int_t fOneOrSeveral;
	Int_t fNumberOfItems; // number of created items
	GSpectraDB * fDB; // Database on wich GVSpectraChooser will work

	bool fEst_Croissant;
	bool fEst_Decroissant;
	Int_t *fpOperation; //!
	Int_t fOperation; //!
	GError fError;

public:

	GVSpectraChooser(const TGWindow* p, const TGWindow* main, UInt_t w,
			UInt_t h, TString windowName, Int_t typeTree,
			Int_t* oneselected =NULL,Int_t *operation=NULL,Int_t typeofspectra=1);

	virtual ~GVSpectraChooser();

	GVListTree* GetListTree() {
		return fListTree;
	}
	;

	void DoOK();

	void Init(GSpectraDB* DB);

	void DoFinish();

	void CallSelectSpectra();

	void CloseWindow();
	//write by Mlilandou
	void ByName();
	void ByPage();
	void ByFamily();
	void Decroissant();
	void Croissant();
	void SetTypeofSpectra(Int_t type){fTypeOfSpectra=type;};

ClassDef(GVSpectraChooser,1)
	//Frame displaying a tree where user choose spectra on which he wants to do an action(log, stats, display...)
};
#endif
