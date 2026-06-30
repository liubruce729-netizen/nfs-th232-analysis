#include "GVDialogAbout.h"

ClassImp(GVDialogAbout)

 const char *xpms[] = //images
    {
      "printer.xpm",
      "fileopen.xpm",
      "save.xpm",
      0
    };

  const char *tips[] = //tooltips
    {
      "print",
      "open config",
      "save config",
      0
    };

GVDialogAbout::GVDialogAbout(const TGWindow *p,const TGWindow *m, UInt_t w, UInt_t h):TGTransientFrame(p,m,w,h)
{
  char*  version = (char*)GRU_VERSION;
  fMain = (TGMainFrame*)m;

  fLabAbout1 = NULL;
  fLabAbout2 = NULL;

  gClient->GetColorByName("white", white);
  gClient->GetColorByName("black", black);

  SetWindowName("About");
  fLabAbout1 = new TGLabel(this,"Vigru : Visualisation Ganil Root Utilities ");
  fLabAbout1->SetForegroundColor(black);
  AddFrame(fLabAbout1,new TGLayoutHints(kLHintsTop | kLHintsCenterX));
  TString tempo;
  tempo.Form("Version : %s", version);
  AddFrame(new TGLabel(this,tempo.Data()));
  AddFrame(new TGLabel(this,"Made by Luc Legeard with help of Delphine Vacquez & Jerome Chauveau"));
  AddFrame(new TGLabel(this,"with ROOT librairies"));
  AddFrame(new TGLabel(this,"Info http://wiki.ganil.fr/gap/wiki/Documentation/Software/Gru"));
  fLabAbout2 = new TGLabel(this,"Contact: legeard@ganil.fr");
  fLabAbout2->SetForegroundColor(black);
  fLabAbout2->SetForegroundColor(black);
  AddFrame(fLabAbout2,new TGLayoutHints(kLHintsRight));

  CenterOnParent();
  Layout();
  MapSubwindows();
  MapWindow();
  GetClient()->WaitFor(this);
}

GVDialogAbout::~GVDialogAbout()
{
Cleanup();
}

void GVDialogAbout::CloseWindow()
{
	UnmapWindow();
	DeleteWindow();
}
