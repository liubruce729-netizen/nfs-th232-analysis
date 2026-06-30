#ifndef GVPAD_H_
#define GVPAD_H_

#include <TGFrame.h>
#include <TGButtonGroup.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGLabel.h>
#include <TPad.h>
#include <TGComboBox.h>
#include <TVirtualPad.h>
#include "GError.h"
#include "GSpectra.h"
#include "GSpectraDB.h"
#include "TGMdiFrame.h"

class GVPad : public TPad{

private:
#ifdef _mdimode_
  TGMdiFrame* fMaino;
#else
  TGMainFrame* fMaino;
#endif
  GError       fError;
  Int_t        NoPad,NoPage;
  TString      fOption2D;
  GSpectraDB   *fPadDB;
  GSpectraDB   *fDB;
  bool         fPadlocal; // flag to notice if a Tpad have been instancied during construction of this GVpad
  Pixel_t      white, orange , black;
  TString      fStatOpt;
  bool fPalette ;
  Int_t  fZT ; // memorize fromtab of zoom ( in case of this gvpad is a zoom pad)
  Int_t  fZP ; // memorize frompad of zoom( in case of this gvpad is a zoom pad)
  Int_t  fComputation; // Computaion to apply on pad when a refresh is ask
  Int_t fDuplicationPad;// memorize fromtab Duplication
  Int_t fDuplicationTab;// memorize frompad Duplication
  TObjArray *fToreturn;// list of Tcut in pad


 public:
	  enum Duplicationing {
		  COMPUTATION_NULL,
		  COMPUTATION_REFRESH,
		  COMPUTATION_COPY,
		  COMPUTATION_FFT,
		  COMPUTATION_FFThalf,
		  COMPUTATION_SCATTER,
		  COMPUTATION_ZEROLESS,
		  COMPUTATION_PROJECTIONX,
		  COMPUTATION_PROJECTIONY,
		  COMPUTATION_PROFILEX,
		  COMPUTATION_PROFILEY,
		  COMPUTATION_USER    // for future
	  };

#ifdef _mdimode_
  GVPad(TGMdiFrame* main,const char* name, const char* title, Double_t xlow, Double_t ylow, Double_t xup, Double_t yup, Color_t color = -1, Short_t bordersize = -1, Short_t bordermode = -2);
#else
  GVPad(TGMainFrame* main,const char* name, const char* title, Double_t xlow, Double_t ylow, Double_t xup, Double_t yup, Color_t color = -1, Short_t bordersize = -1, Short_t bordermode = -2);
#endif

  void Initialization();
  virtual  ~GVPad();

  virtual GSpectraDB* GetPadDB(){return fPadDB;};
  virtual TPad* GetPad(){return this;};
  virtual void DisplaySpectra(GSpectraDB *DB,bool getnewone=true);
  virtual void DisplaySpectrum(GSpectrumIdentity* id,Int_t operation=0);
  virtual void AddDisplaySpectrum(GSpectrumIdentity* id,bool getnewone= true);
  virtual void RefreshPad(bool getnewone= true);
  virtual void ChangingLog(Bool_t logx, Bool_t logy, Bool_t logz);
  virtual void ApplyLog(Bool_t logx, Bool_t logy, Bool_t logz);
  virtual void RemoveLog(Bool_t logx, Bool_t logy, Bool_t logz);
  virtual void Divide(Int_t nx, Int_t ny, Float_t xmargin=0.000, Float_t ymargin=0.000, Int_t color=255*255*255);

  virtual void SetComputation(Int_t computation){fComputation= computation;};
  virtual void ApplyComputation(Int_t computation,Int_t fromtab,Int_t frompad);
  virtual void SetDuplicationTab(Int_t duplicationtab){fDuplicationTab = duplicationtab;};
  virtual void SetDuplicationPad(Int_t duplicationpad){fDuplicationPad  = duplicationpad;};
  virtual Int_t GetComputation(){return fComputation;};
  virtual Int_t GetDuplicationTab(){return fDuplicationTab;};
  virtual Int_t GetDuplicationPad(){return fDuplicationPad;};
  virtual void IntegralsInCuts();

  virtual void Copy(Int_t frompad, Int_t frompage);
  virtual void CopyOnlySpectra(Int_t frompad, Int_t frompage);
  virtual void CopyIfEmpty(Int_t fromtab, Int_t frompad);

/*virtual void ComputaionCopy(Int_t pad, Int_t page);
  virtual void ComputaionFFT(Int_t pad, Int_t page);
  virtual void ComputaionScatter(Int_t pad, Int_t page);
  virtual void ComputaionZeroLess(Int_t pad, Int_t page);
  */

  virtual void CreateXML(TXMLEngine* ml, XMLNodePointer_t node);
  virtual void ReadXML(TXMLEngine* ml, XMLNodePointer_t node);
  virtual TString GetStatOpt(){return fStatOpt;};
  virtual void    SetStatOpt(TString statopt,bool doaction=true);
  virtual TString GetOption2D(){return fOption2D;}
  virtual void    SetOption2D(TString option){fOption2D=option;}

  virtual void SaveSpectra(TFile* file,Int_t type);
  virtual void ReadSpectra(TFile* file);
  virtual void SaveSpectraTxt(ofstream *file);
  virtual void ResetSpectra(bool with_question);
  virtual void DrawPalette();
  virtual void DrawPanel();
  virtual TCutG* GetNewCutFromPad(const char* name);
  TObjArray* GetAllCutsFromPad();
  virtual void SetZP (Int_t zp){fZP= zp;}
  virtual Int_t GetZP (){return fZP;}
  virtual void SetZT (Int_t zt){fZT= zt;}
  virtual Int_t GetZT (){return fZT;}
  virtual void AutoZoom();
  virtual void FFT(bool refresh = true);
  virtual void Scatter(bool refresh = true);
  virtual void ReplaceDB(GSpectraDB * DB);
  virtual void ApplyZoom(Int_t xmin_pad,Int_t xmax_pad,Int_t ymin_pad,Int_t ymax_pad) ;
  virtual void ApplyZoomProportional(Int_t xmin,Int_t xmax,Int_t ymin,Int_t ymax) ;
  virtual void ApplySetRangeUser(bool inx,bool iny,bool inz, Double_t xmin, Double_t xmax,Double_t ymin,Double_t ymax,Double_t zmin, Double_t zmax);
  virtual void PeakFind(Int_t NbPeaks,Float_t Resolution ,Double_t Sigma,Double_t Threshold,bool Display_polymarker);
  virtual void CancelZoom();
  ClassDef(GVPad,1)// TPad whith a small SD of histogram.

};
#endif /*GVPAD_H_*/




