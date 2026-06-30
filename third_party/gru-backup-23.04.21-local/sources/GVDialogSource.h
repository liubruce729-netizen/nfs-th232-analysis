#ifndef __GVDialogSource__
#define __GVDialogSource__

#include "GError.h"
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGButtonGroup.h>
#include <TGButton.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>
#include <TGListBox.h>
#define GVDIAG_UNKOWN 0
#define GVDIAG_GRUNET 1
#define GVDIAG_ROOTFILE 2
#define GVDIAG_MEMORY 3
#define GVDIAG_SOAPNET 4
#define GVDIAG_MAX 5
class GVDialogSource: public TGTransientFrame
{

private:

  TGMainFrame *fMainBase;

  TGButtonGroup *fButGroupGruSoap, *fButGroupNetFile;

  TGNumberEntry  *fNumEntryA, *fNumEntryB, *fNumEntryPort;

  TGLabel *fLabelTitle, *fLabelFile, *fLabelHost,*fLabelPort,*fLabelRange,*fLabelType;

  TGRadioButton *fRadioType[GVDIAG_MAX];

  TGGroupFrame *fGroupSeeListServer, *fGroupAddServer,*fServerListGroupFrame;
  TGCompositeFrame *fAddServerSub2,*fAddServerSub1;

  TGCompositeFrame *fTitle, *fResetListServer, *fAddServer, *fEndButtons;
  TGListBox *fListBox;
  TGLayoutHints *fLayout1,*fLayout2,*fLayout3,*fLayout4,*fLayout5,*fLayout6,*fLayout7;
  TGCheckButton *fCheckMulti ;

  TGTextEntry  *fEntryHostOrFile;

  TGTextButton *fButtonBrowse, *fButtonOk,*fAddButtonList,*fTestButton,*fRemoveButton;
  TGMatrixLayout *fMatrix;
  TString fSourceType, fSourceName,fSourceNetFile;

  Int_t fPort,fPortdef; // current Port number for TCIP communication, and default port.

  Int_t fChange; // + 1 after each changement
  Int_t fFirstEntry,fLastEntry;
  Int_t fServerType;
  Pixel_t white, orange , black,red;

  GError fError;

public:

  GVDialogSource(const TGWindow *p,const TGWindow *m,UInt_t w, UInt_t h);

  virtual ~GVDialogSource();

  void OpenFile();

  void DoEnd();

  void AddListServers();
  void FillListBoxWithServerList();
  void RefreshListServers() ;
  void CloseWindow();

  void PressedGRUNet();
  void PressedSoapNet();
  void PressedFile();
  void PressedMemory();
  void TestServer();
  void RemoveServer();

  ClassDef(GVDialogSource,1)//Frame where user choose spectra sources: from net, from file ...

};
#endif
