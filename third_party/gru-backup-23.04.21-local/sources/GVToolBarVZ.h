#ifndef __GVToolBarVZ__
#define __GVToolBarVZ__

#include <TGMenu.h>
#include <TGFrame.h>
#include <TGToolBar.h> 
#include <TGTab.h>
#include <TGFrame.h>
#include "GError.h"
#include "GVSpectraChooser.h"
#include "GVSetRefreshTimer.h"
class GVToolBarVZ: public TGToolBar 
{
  
  
private:

  //ToolBarData_t t[3];
 GVSpectraChooser * fStatSpectraChooser;
  TGMainFrame *fMain;
  GError       fError;
  Pixel_t      white;
  TGTransientFrame *fSetPage, *fSetStat;
  GVSetRefreshTimer *fSetRefreshTimer;
  GVSpectraChooser *fDisplaySpectraChooser;

public:
  
  GVToolBarVZ(const TGWindow *p);

 
  void Memorize();
  void ResetHisto();
  void AutoZoom();
  void CancelZoom();
  void DrawPanel(); 
  void DrawPalette();
  void SetLogX();
  void SetLogY();
  void SetLogZ();
  void FitPanel();
  void FitBG();
  void RefreshPage();
  void SetRefresh();

  void MessageBoxNoHisto();

  ClassDef(GVToolBarVZ,1)//Toolbar: Open config, Save & print

};
#endif
