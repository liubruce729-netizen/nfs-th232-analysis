#ifndef  __GVTab__
#define  __GVTab__

#include <TGFrame.h>
#include <TGTab.h>
#include <TRootEmbeddedCanvas.h>
#include <TGString.h>
#include <TList.h>
#include <TGFrame.h>
#include "GError.h"
#include "GVPad.h"
#include "TGMdiFrame.h"

//______________________________________________________________________________
class myTGCompositeFrame : public TGCompositeFrame {
private:
	// class to creat TGCompositeFrame
	// and memorize few associed information ( fCanvas,fNpadx,fNpady,TString fTabName;)

	TRootEmbeddedCanvas *fCanvas; // first (0) canvas of tab
	Int_t fNpadx; // n of pad in x of tab
	Int_t fNpady; // n of pad in y  of tab
	TString fTabName;

public:
	myTGCompositeFrame(const TGWindow* p = 0, UInt_t w = 1, UInt_t h = 1, UInt_t options = 0, Pixel_t back = GetDefaultFrameBackground()) :
		TGCompositeFrame( p , w , h ,options , back ) {
		fNpadx=0;
		fNpady=0;
		fCanvas=NULL;
		fTabName="";
	}


	TRootEmbeddedCanvas* GetTRootEmbeddedCanvas() {
		return fCanvas;
	}


	virtual TString GetTabName() {
		return fTabName;
	}

	virtual void SetTabName(TString tabname) {
		fTabName =tabname;
		TGCompositeFrame::SetName( tabname.Data());
	}

	virtual Int_t GetNPadx() {
		return fNpadx;
	}

	virtual Int_t GetNPady() {
		return fNpady;
	}

	virtual void SetTRootEmbeddedCanvas(TRootEmbeddedCanvas* canvas) {
		fCanvas=canvas;
	}
	virtual void SetNPadx(Int_t npadx) {
		fNpadx=npadx;
	}

	virtual void SetNPady(Int_t npady) {
		fNpady=npady;
	}
	virtual void SetInfo(TRootEmbeddedCanvas* canvas, TString tabname,
			Int_t npadx, Int_t npady) {
		if (npady<0)
			SetNPady(npady);
		if (npadx<0)
			SetNPadx(npadx);
		if (tabname)
			SetTabName(tabname);
		if (canvas)
			SetTRootEmbeddedCanvas(canvas);
	}


};
//______________________________________________________________________________
class GVTab : public TGTab {
private:

#ifdef _mdimode_
	TGMdiFrame *fMain;
#else
	TGMainFrame *fMain;
#endif
	GError fError;
	Bool_t fAutoMode;//if true: application add pages, else user set pages

public:

#ifdef _mdimode_
	GVTab(const TGWindow *p, TGMdiFrame *f);
#else
	GVTab(const TGWindow *p, TGMainFrame *f);
#endif
	virtual ~GVTab();

	virtual void SetAutoMode(Bool_t mode) {
		fAutoMode = mode;
	}

	virtual Bool_t GetAutoMode() {
		return fAutoMode;
	}

	virtual myTGCompositeFrame* AddTab(const char *text, Int_t nbPadX,
			Int_t nbPadY);

	virtual void RemoveCurrentPage();

	virtual TRootEmbeddedCanvas* GetCurrentEmbeddedCanvas();
	virtual TRootEmbeddedCanvas* GetEmbeddedCanvasAt(Int_t index);
	virtual TCanvas* GetCanvasAt(Int_t index =0);
	virtual TCanvas* GetCurrentCanvas();

	virtual void ComputePadXY(Int_t nbPads, Int_t *nbPadsX, Int_t *nbPadsY);
	virtual void SetPageChange(TString pageName, Int_t nbPadX, Int_t nbPadY);
	virtual void DivideCurrentPage(Int_t nPadX, Int_t nPadY);
	virtual void MyDivide(TCanvas *canvas, Int_t nx, Int_t ny,
			Float_t xmargin=0.01, Float_t ymargin=0.01, Int_t color = -1);
	virtual void RenameCurrentPage(TString name);
	virtual void RemoveTab(Int_t tabIndex = -1);
	virtual void SetPageAction(Int_t id);
	virtual void RefreshTab(bool with_return= true,bool getnewspe= true);

	virtual void ApplyZoomProportional(Int_t xmin,Int_t xmax,Int_t ymin,Int_t ymax) ;
	virtual void ApplySetRangeUser(bool inx,bool iny,bool inz, Double_t xmin, Double_t xmax,Double_t ymin,Double_t ymax,Double_t zmin, Double_t zmax);
	virtual void ApplyZoomProportionalAllPages(Int_t xmin,Int_t xmax,Int_t ymin,Int_t ymax);
	virtual void CancelZoom() ;
	virtual void ApplyPageLog(Bool_t logx, Bool_t logy, Bool_t logz);
	virtual void ApplyLogAllPages(Bool_t logx, Bool_t logy, Bool_t logz);
	virtual void RemovePageLog(Bool_t logx, Bool_t logy, Bool_t logz);
	virtual void PeakPageFind(Int_t NbPeaks,Float_t Resolution ,Double_t Sigma,Double_t Threshold,bool Display_polymarker);
	virtual void ApplyPageReset(bool question= false);
	virtual void CdFirstOrLastPad();
	virtual TPad* GetSelectedPad();
	virtual TPad* GetPad(Int_t notab, Int_t nopad);
	virtual Int_t GetNumberOfPad(Int_t nopage);
	virtual Int_t GetNoPad(Int_t level);
	virtual void MoveToNextPad(Int_t deplacement);
	virtual void SetStatOpt(TString statopt);
	virtual void SaveSpectra(TFile *file,Int_t type=-1);
	virtual void ReadSpectra(TFile *file);
	virtual void CreateXML(TXMLEngine* xml, XMLNodePointer_t node,bool only_current =false);
	virtual void ReadXML(TXMLEngine* xml, XMLNodePointer_t node,bool question =false);
	ClassDef(GVTab,1)//ViGRU manage Tab and included canvas
};

#endif//__GVTab__
