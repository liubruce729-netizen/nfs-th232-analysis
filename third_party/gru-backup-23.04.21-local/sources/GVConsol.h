#ifndef  __GVConsol__
#define  __GVConsol__

#include "General.h"
#include <TGTextView.h>
#include <TGTextEntry.h>
#include <TGButton.h>
#include <TGFrame.h>
#include "GError.h"
#include <TBufferXML.h>
#include <TXMLEngine.h>
#include <TDOMParser.h>
#include <TXMLNode.h>

class GVConsol: public TGCompositeFrame
{
 private:


  TGTextView *fConsolTextView;

  TGTextButton *fBrowse,*fUpdate,*fClear;

  TGTextEntry *fFileNameEntry;

  TGGroupFrame *fOption;

  char *fBarre; //!

  char fLogFile[MAX_CARACTERES];

  GError fError;

  Bool_t fInitalStateConsol , fInitalStateLogFile;

  Pixel_t      black;

  Int_t  fActualOutput;//  0 = Standart  (in terminal), 1 = logfile, 2 = logfile and consol

 public:

  TGCheckButton *fCheckLogFile, *fCheckConsol;

  GVConsol(const TGWindow *p ,char* logfile= (char*)"file.log",bool  Logfilebool=false, bool  Consol=false);

  virtual ~GVConsol();

  void SetLogFile(char* logfile);

  char* GetLogFile(){return fLogFile;};

  void Update();
  void UpdateLogfile();
  void Browse();
  void ClearConsol();
  void RedirectOutput();
  void ConsolValidation();
  void CreateXML(TXMLEngine* xml,XMLNodePointer_t node);
  void ReadXML(TXMLEngine* xml, XMLNodePointer_t node);
 ClassDef(GVConsol,1)//ViGRU Consol: stdout et stderr could be redirected in it
};
#endif
