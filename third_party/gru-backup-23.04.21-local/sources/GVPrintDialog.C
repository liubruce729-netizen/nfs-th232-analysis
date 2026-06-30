#include "GVPrintDialog.h"
#include "Vigru.h"

#include <TVirtualPadEditor.h>
#include <TTimer.h>

ClassImp(GVPrintDialog)

GVPrintDialog::GVPrintDialog(const TGWindow *p,const TGWindow *m,UInt_t w, UInt_t h):TGTransientFrame(p,m,w,h)
{
   fMainBase = (TGMainFrame*)m;
   Vigru *f = (Vigru*)fMainBase;
   SetWindowName("Print Setup");
   gClient->GetColorByName("white", white);
   gClient->GetColorByName("black", black);
   SetLayoutManager(new TGVerticalLayout(this));
   fConfiguration      = new TGGroupFrame(this,"Print configuration");

   fPrint              = new TGButtonGroup(fConfiguration,"Print",kVerticalFrame);
   fPrintTo            = new TGButtonGroup(fConfiguration,"Print to",kVerticalFrame);

   fRadioPrinterString = new TGHotString("Printer");
   fRadioPrinter       = new TGRadioButton(fPrintTo,fRadioPrinterString);
   fRadioFileString    = new TGHotString("File(postcript Name)");
   fRadioFile          = new TGRadioButton(fPrintTo,fRadioFileString);
   fRadioPageString    = new TGHotString("Page");
   fRadioPage          = new TGRadioButton(fPrint,fRadioPageString);
   fRadioPadString     = new TGHotString("Only selected pad");
   fRadioPad           = new TGRadioButton(fPrint,fRadioPadString);
   fLabelPrinter       = new TGLabel(fConfiguration,"Printer name");
   fEntryPrinterName   = new TGTextEntry(fConfiguration);
   if (f->GetDefaultPrinterName().CompareTo(""))
		   fEntryPrinterName->SetText(f->GetDefaultPrinterName().Data());
   fButtons            = new TGCompositeFrame(this);
   fButtons->SetLayoutManager(new TGMatrixLayout(fButtons,0,2,10));
   fButtonOk           = new TGTextButton(fButtons,"Ok");
   fButtonCancel       =  new TGTextButton(fButtons,"Cancel");

   fButtonOk->Resize(50,30);
   fButtonCancel->Resize(50,30);
   fButtons->AddFrame(fButtonOk);
   fButtons->AddFrame(fButtonCancel);
   fConfiguration->AddFrame(fPrint);
   fConfiguration->AddFrame(fPrintTo);
   fConfiguration->AddFrame(fLabelPrinter);
   fConfiguration->AddFrame(fEntryPrinterName);
   AddFrame(fConfiguration,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
   AddFrame(fButtons,new TGLayoutHints(kLHintsRight));

   fPrint->Show();
   fPrintTo->Show();

   SetBackgroundColor(white);
   fConfiguration->SetBackgroundColor(white);
   fPrint->SetBackgroundColor(white);
   fPrintTo->SetBackgroundColor(white);
   fButtonOk->SetTextColor(black,kTRUE);
   fButtonOk->SetBackgroundColor(white);
   fButtonCancel->SetTextColor(black,kTRUE);
   fButtonCancel->SetBackgroundColor(white);
   fLabelPrinter->SetBackgroundColor(white);
   fRadioPrinter->SetBackgroundColor(white);
   fButtons->SetBackgroundColor(white);
   fRadioFile->SetBackgroundColor(white);
   fRadioPage->SetBackgroundColor(white);
   fRadioPad->SetBackgroundColor(white);

   Layout();
   MapSubwindows();
   CenterOnParent();
   MapWindow();

   //slots connections
   fRadioPrinter->Connect("Clicked()", "GVPrintDialog", this, "SetPrinter()");
   fRadioFile   ->Connect("Clicked()", "GVPrintDialog", this, "SetFile()");
   fButtonOk    ->Connect("Clicked()", "GVPrintDialog", this, "DoOk()");
   fButtonCancel->Connect("Clicked()", "GVPrintDialog", this, "DoCancel()");
   fRadioPrinter->SetDown();
   fRadioPage->SetDown();
   GetClient()->WaitFor(this);

}
GVPrintDialog::~GVPrintDialog()
{
Cleanup();
}

void GVPrintDialog::DoOk()
{

  Vigru *f = (Vigru*)fMainBase;
  TString ptrName = fEntryPrinterName->GetText();
  f->Print(ptrName,fRadioPage->IsOn(),fRadioPrinter->IsOn());
  DoCancel();
}

void GVPrintDialog::SetPrinter()
{
	fLabelPrinter->SetText("Printer Name");
}

void GVPrintDialog::SetFile()
{
	fLabelPrinter->SetText("FileName");
}

void GVPrintDialog::DoCancel(){
  TTimer::SingleShot(80, "GVPrintDialog", this, "CloseWindow()");
  // Close the Ged editor if it was activated.
  if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
    TVirtualPadEditor::Terminate();
}

void GVPrintDialog::CloseWindow()
{
	UnmapWindow();
	DeleteWindow();
}
