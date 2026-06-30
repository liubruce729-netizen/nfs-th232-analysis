
#ifndef  __Vigru__
#define  __Vigru__
#include <TApplication.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TGStatusBar.h>
#include <TG3DLine.h>
#include <TGLabel.h>
#include <TGString.h>
#include <TPad.h>
#include <TF1.h>
#include <TH1.h>
#include <TObjArray.h>
#include <TThread.h>
#include <TGToolTip.h>
#include <TString.h>
#include <TTimer.h>
#include <TGDoubleSlider.h>
#include "GSpectra.h"
#include "GSpectrumIdentity.h"
#include "GError.h"
#include "GVMenuBar.h"
#include "GVConsol.h"
#include "GVToolBarVS.h"
#include "GVToolBarVZ.h"
#include "GVTab.h"
#include "GVPad.h"
#include "GVToolBar.h"
#include "GVPrintDialog.h"
#include "GDemo.h"
#include "GAcqNumexo.h"
#ifdef _mdimode_
#include "TGMdi.h"
#include "TGMdiFrame.h"
#include "TGMdiMainFrame.h"
#endif

#ifdef _mdimode_

class Vigru:public TGMdiFrame {

#else

class Vigru:public TGMainFrame {

#endif

private:

  GVMenuBar        *fMenuBar;
  GVToolBar        *fToolBar;
  GVConsol         *fConsol;
  GVTab            *fTabPage, *fTabZooms;
  GDemo            *fDemo;
#ifdef _mdimode_
  TGMdiFrame		*fMainBaseFrame_this;
#else
  TGMainFrame         *fMainBaseFrame_this;
#endif

  TGTransientFrame    *fMainWindows;

  TGCompositeFrame    *fTabSpectra, *fTabZoom, *fTabCanvas, *fTab0, *fZoomCenter,  *fTabConsol;
  TPad                *fFromPad;
  GError              fError;
  Pixel_t             white, orange, grey, blue;
  TGHorizontal3DLine  *fLh ;
  TGTab               *fTab;
TString               fDefaultPrinterName;
  TGStatusBar         *fStatusBar;
  TGDoubleSlider      *fVsliderS,*fHsliderS,*fVsliderZ,*fHsliderZ;
  TGToolTip           *fTip;
  TTimer              *fTimer, *fTimerZoom;
  GVPrintDialog       *fPrintDialog;
  Int_t        fPortdef    ;
  GSpectra    *fSpeManager ;  // spectra manger
  GVToolBarVS *fToolBarVS ;
  GVToolBarVZ *fToolBarVZ;
   TString fLastConfigDirFile;
   TString fLastConfigPageDirFile;
   TString fLastConfigSpeFile;
   TString fLastConfigCutFile;
   TString fLastConfigSpeFileTxt;
   TString fResetOption; // contains option for reset h => pad reset, p => page reset, s => server + y/n for to ask question for each reset
 TString fLastConfigDir;
 TCutG *fCurrentCut;
 bool fDisplayInfos ;
 int fVerbose;
 bool fVerboseDone;
 GAcqNumexo * fAcqNumexo;

 int fSpecificMode;// specific mode , 0 no mode, 1, demo mode , 2 oscillo mode.
 // Memorization of parameters of peak find
 Int_t    fNbPeaks;
 Float_t  fResolution;
 Double_t fSigma;
 Double_t fThreshold;
 bool     fDisplay_polymarker;

 GSpectraDB *pExternSpectraDB;
 bool      fTerminate;
#ifndef NO_GSOAP

#endif

public:

#ifdef _mdimode_
 Vigru(TGMdiMainFrame *p, UInt_t w, UInt_t h,TString config_file="", TString rootfile="",TString rootserver="",TString soapserver="",Int_t rootport=0,Int_t soapport=0,int monde= 0);
#else
 Vigru(const TGWindow *p=NULL, UInt_t w=800, UInt_t h=700,TString config_file="", TString rootfile="",TString rootserver="",TString soapserver="",Int_t rootport=0,Int_t soapport=0,int monde= 0);
#endif
  void Initialization(TString config_file, TString rootfile="",TString rootserver="",TString soapserver="",Int_t rootport=0,Int_t soapport=0,int mode= 0);

  virtual ~Vigru();

  GSpectra* GetSpeManager(){return fSpeManager;};

  TString* GetResetOption(){return &fResetOption;};


  GVTab* GetTabPage(){return fTabPage;};

  GVTab* GetTabZoomPage(){return fTabZooms;};

  GVTab* GetSpecOrZoomTab();

  TPad*  GetSelectedPad();
  void  SetExternSpectraDB( GSpectraDB *DB){ pExternSpectraDB =DB;}

  Int_t*    GetNbPeaks()   {return &fNbPeaks;};
  Float_t*  GetResolution(){return &fResolution;};
  Double_t* GetSigma()     {return &fSigma;};
  Double_t* GetThreshold() {return &fThreshold;};
  bool*     GetDisplay_polymarker(){return &fDisplay_polymarker;};


  void ChangeTabZoom(Int_t tab);
  void ChangeTabPage(Int_t tab);
  void SetVerbose(int verbose){fVerbose = verbose;}
  int  GetVerbose(){return fVerbose;}
  void AddPage(TString pageName = "",Int_t nbPadX =1,Int_t nbPadsY =1);
  void SetPageChange(TString pageName,Int_t nbPadX =0,Int_t nbPadsY =0);
  void ConnectInfos(bool connect);
  void ConnectionOfCanvas(TCanvas* canvas);

  void SetVZPosition();
  void SetHZPosition();
  void SetZPositionReset();
  void SetVSPosition();
  void SetHSPosition();
  void SetSPositionReset();

  void ResetSpectra(bool force = false);
  void ResetSpectrum(bool question = false );
  void ApplyPageReset(bool question = false);
  void AllServerSpectraReset(bool question = false);

  Bool_t TestObject(TObject* obj);

  GSpectrumIdentity* GetCurrentSpeId(bool quiet = false);
  void RefreshPad();
  void RefreshPage(bool with_return = true,bool getnewspe = true);
  void RefreshAllPages(bool getnewspe = true);

  void Zoom(Int_t tab,Int_t pad);
  void ZoomBack();

#ifdef _mdimode_
  Bool_t CloseWindow();
#else
 void CloseWindow();;
#endif

 void SaveConfiguration(TString xmlFile,bool force= false);
 void RestoreConfiguration(TString xmlFile,bool question = true);
 void SaveConfigurationPage(TString xmlFile,bool force= false);
 void RestoreConfigurationPage(TString xmlFile);
 void CreateXMLFile(TString xmlFile, TString Rootfile,bool force= false);
 void CreateXMLPageFile(TString xmlFile, TString Rootfile,bool force= false);
 void SaveSpectra(TString filename,bool rootortxt=true,Int_t type=-1) ;
 bool ReadSpectra(TString filename) ;
 void SaveCuts(Int_t level =3);
 void LoadSpectra(Int_t type,GSpectraDB *DB=NULL);
 void ApplyComputation(Int_t computation,Int_t tab,Int_t pad);

 void Oscillo(TString soapserver,int soapport);
 void OscilloRun();
 void Demo();

 void DeleteOldConf();

  void SetStateTimer(Long_t time);
  void SetStateTimerZoom(Long_t time);

  void LoadPadCuts();
  void DefineCuts() ;
  void SendCuts() ;
  void SendPadCuts();
  void DisplayCuts() ;
  void ClearCuts() ;
  void IntegralsInCuts();
  void DisplayInfos(Int_t event,Int_t px,Int_t py,TObject* obj);

  void DisplayPrintDialog();

  void Print(TString printerName,Bool_t page,Bool_t printer);
  void DisplaySpectrum(Int_t ind,Int_t operation=0);
  void DisplaySpectra(Int_t operation=0);
  void ChangeSpectrum(Int_t index,TNamed* spectrum);
  void ApplyStatistics(TString StatOpt,bool page);

  void ChangingLog(Bool_t logx, Bool_t logy, Bool_t logz);
  void ApplyLog(Bool_t logx, Bool_t logy, Bool_t logz);
  void ApplyPageLog(Bool_t logx, Bool_t logy, Bool_t logz);
  void ApplyAllPagesLog(Bool_t logx, Bool_t logy, Bool_t logz);
  void ApplyZoomProportionalAllPages();
  void RemoveLog(Bool_t logx, Bool_t logy, Bool_t logz);
  void RemovePageLog(Bool_t logx, Bool_t logy, Bool_t logz);
  void RemoveAllPagesLog(Bool_t logx, Bool_t logy, Bool_t logz);
  void ApplyRangeUserPage(bool inx,bool iny,bool inz, Double_t xmin, Double_t xmax,
  		Double_t ymin,Double_t ymax,Double_t zmin, Double_t zmax,int padtaborall);
  void PeakFind();
  void PeakPageFind();

  void DumpPadDataBase();
  void SetPage(Int_t id);
  void SetInfo(bool info);
  TString GetDefaultPrinterName(){return fDefaultPrinterName;}
  void TreatName(TString* name);
  void SendEndOfPage(GSpectrumIdentity* last_id_server);

  void FFT();
  void FitBG();
  void FitPanel();
  void DrawPanel();
  void DrawPalette();
  void AutoZoom();
  void CancelZoom(int padtaborall =1);
  void OpenSaveConfig(int numAction);
  void MessageBoxNoHisto();
  void MessageBoxWarning (char* text);
  bool IsFileExiste(TString filename, bool withtexterror=false);

  void GoForward();
  void GoBack();

  void SetTeminate(bool term){fTerminate = term;};

  ClassDef(Vigru,1)//ViGRU Main frame
};

#endif
