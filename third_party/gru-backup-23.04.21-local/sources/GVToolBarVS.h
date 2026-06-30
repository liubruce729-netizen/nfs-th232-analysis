#ifndef __GVToolBarVS__
#define __GVToolBarVS__

#include <TSystem.h>
#include <TGMenu.h>
#include <TGFrame.h>
#include <TGToolBar.h>
#include <TGTab.h>
#include <TGFrame.h>
#include "GError.h"
#include "GVSpectraChooser.h"
#include "GVSetRefreshTimer.h"
#include "TGMdiFrame.h"

class GVToolBarVS: public TGToolBar
{


private:

  //ToolBarData_t t[3];
 GVSpectraChooser * fStatSpectraChooser;
#ifdef _mdimode_
 TGMdiFrame *fMain;
#else
  TGMainFrame *fMain;
#endif
  GError       fError;
  Pixel_t      white;
  TGTransientFrame *fSetPage, *fSetStat,*fReset;
  GVSetRefreshTimer
  *fSetRefreshTimer;
  GVSpectraChooser *fDisplaySpectraChooser;

public:

  GVToolBarVS(const TGWindow *p);

  void ResetAction();
  void FFTAction();
  void Zoom1Action();
  void Zoom2Action();
  void Zoom31Action();
  void Zoom32Action();
  void RemovePage();
  void AddPage();
  void SetPage();
  void SetLogX();
  void SetLogY();
  void SetLogZ();
  void RefreshPad();
  void RefreshPage();
  void SetRefresh();
  void Statistics();
  void MessageBoxNoHisto();
  void DisplayDisplayingTreeSev();
  void DisplayDisplayingTreeOne();
  void CancelZoom() ;
  ClassDef(GVToolBarVS,1)//Toolbar: Open config, Save & print

};
#endif
