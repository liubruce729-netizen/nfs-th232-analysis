#ifndef __GVToolBar__
#define __GVToolBar__

#include <TGMenu.h>
#include <TGFrame.h>
#include <TGToolBar.h>
#include <TGTab.h>
#include <TGFrame.h>
#include <TImage.h>
#include <TGPicture.h>
#include "GError.h"
#include "GVSpectraChooser.h"
class GVToolBar: public TGToolBar
{


private:

  //ToolBarData_t t[3];
 GVSpectraChooser * fStatSpectraChooser;
  TGMainFrame *fMain;
  GError       fError;
  Pixel_t      white;
  TGTransientFrame *fSetPage, *fSetStat;
  TString fStatOpt ;
  bool fInfoState,fCutState;
  GVSpectraChooser * fDisplaySpectraChooser;
  Int_t fNbPictureInfono,fNbPictureInfo,fNbButtonInfo;
  Int_t fNbPictureCut,fNbPictureCutSend,fNbButtonCut,fNbPictureCdBack,fNbPictureCdForward;
  TImage **fImg;//!
  const TGPicture **fPicture;//!

public:

  GVToolBar(const TGWindow *p);
  ~GVToolBar();
  void OpenFile();
  void SaveConfig();
  void OpenPrintDialog();

  void RefreshListAction();
  void SwitchInfo();
  void SetInfo(bool info);
  void DoCut();
  void GoBack();
  void GoForward() ;

  ClassDef(GVToolBar,1)//Toolbar: Open config, Save & print

};
#endif
