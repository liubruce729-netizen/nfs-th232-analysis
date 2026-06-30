#ifndef __GVMenuBar__
#define __GVMenuBar__

#include <TGMenu.h>
#include <RQ_OBJECT.h>
#include <TQObject.h>
#include <TGFrame.h>
#include <TQObject.h>
#include <TGFileDialog.h>
#include "GVDialogAbout.h"
#include "GVDialogSource.h"
#include "GVSpectraChooser.h"
#include "GVSetPeakFind.h"
#include "GVSetRangeUser.h"
#include "GSpectra.h"
#include "TGMdiFrame.h"
#include "GVSetDuplication.h"

class GVMenuBar: public TGMenuBar
{


private:
#ifdef _mdimode_
	  TGMdiFrame         *fMain;
#else
	  TGMainFrame         *fMain;
#endif
  TGPopupMenu     *fMenuFile, *fMenuSetup, *fMenuTools,*fMenuDuplication,*fMenuCuts, *fMenuExpert,*fMenuHelp, *fMenuClassification;


  GVDialogAbout    *fAbout;

  GVDialogSource   *fSource;
  GVSetDuplication *fSetDuplication;//!
 TString fLastConfigDirFile;
 TString fLastConfigDir;
 GVSpectraChooser *fDisplaySpectraChooser;
  //GVClassManual    *fClassManual;

  TGLayoutHints       *fMenuBarItemLayout, *fMenuBarHelpLayout;
  GVSetPeakFind  *fSetPeakFind;
  GVSetRangeUser *fSetRangeUser;
  Pixel_t             white;


public:

  GVMenuBar(const TGWindow *p, UInt_t w, UInt_t h, UInt_t option);

  virtual ~GVMenuBar();

  void HandleMenu(Int_t id);

  void DisplayAbout();
  void DisplaySource();
  void DefineCuts();
  void SendCuts();
  void DisplayCuts();
  void ClearCuts();
  void SaveCuts();
  void IntegralsInCuts();
  void SetDuplication();
  GVDialogSource *GetDialogSource();

  void OpenPrintDialog();


  ClassDef(GVMenuBar,1)//ViGRU menu bar
};

#endif
