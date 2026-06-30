// File : GVConsol.C
// Author: Jerome Chauveau
//////////////////////////////////////////////////////////////////////////////
//class GVConsol
//Frame containing a TextView in which stdout & stderr could be redirected
//
////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#include <sys/stat.h>
#include "GVConsol.h"
#include <TObject.h>
#include <TApplication.h>
#include <TGFileDialog.h>
#include "Riostream.h"

#include "Vigru.h"
#include <dirent.h>
#include <sys/mtio.h>


ClassImp(GVConsol)

  GVConsol::GVConsol(const TGWindow *p ,char *logfile ,bool  Logfilebool , bool  Consol):TGCompositeFrame(p)
{
  gClient->GetColorByName("black", black);
  fConsolTextView = new TGTextView(this);
  fConsolTextView->SetForegroundColor(black);
  fActualOutput =0;
  fBarre =(char*)"-----------------------------------------------------------------------------------\n";

  strcpy (fLogFile,"");
  fInitalStateConsol =Consol;
  fInitalStateLogFile=Logfilebool ;
  if (logfile)
    SetLogFile(logfile);
  else
    {
    Logfilebool =false;
    fInitalStateConsol=false;
    }

  fOption       = new TGGroupFrame (this,"Option");
  fCheckLogFile = new TGCheckButton(fOption,"Save in a log file");
  fFileNameEntry= new TGTextEntry  (fOption,"");
  fBrowse       = new TGTextButton (fOption,"Browse");
  fUpdate       = new TGTextButton (fOption,"Update");
  fClear        = new TGTextButton (fOption,"Clear");
  fCheckConsol  = new TGCheckButton(fOption,"Consol");
   fFileNameEntry->SetEdited(kFALSE);
  fOption->SetLayoutManager(new TGHorizontalLayout(fOption));
  fOption->AddFrame(fCheckLogFile);
  fOption->AddFrame(fFileNameEntry);
  fOption->AddFrame(fBrowse);
  fOption->AddFrame(fUpdate,new TGLayoutHints(kLHintsCenterX));
  fOption->AddFrame(fClear,new TGLayoutHints(kLHintsCenterX));
  fOption->AddFrame(fCheckConsol,new TGLayoutHints(kLHintsRight));

  fFileNameEntry->SetText(fLogFile);

  AddFrame(fConsolTextView,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  AddFrame(fOption,new TGLayoutHints(kLHintsExpandX));
  fConsolTextView->Resize(fConsolTextView->GetDefaultSize());

  fFileNameEntry->Connect("ReturnPressed()","GVConsol",this,"UpdateLogfile()");
  fBrowse       ->Connect("Clicked()","GVConsol",this,"Browse()");
  fUpdate       ->Connect("Clicked()","GVConsol",this,"Update()");
  fCheckLogFile ->Connect("Clicked()","GVConsol",this,"RedirectOutput()");
  fClear        ->Connect("Clicked()","GVConsol",this,"ClearConsol()");

}
  /*------------------------------------------------------*/
GVConsol::~GVConsol()
{
	Cleanup();

//DeleteWindow();
}

void GVConsol::SetLogFile(char* logfile)
{
  if (logfile){
    if (strcmp (logfile,"")==0)
      strcpy (fLogFile,"Consol.log");
    else
      strcpy (fLogFile,logfile);
  }

}
/*------------------------------------------------------*/
void GVConsol::ClearConsol()
{
  fConsolTextView->Clear();
  struct stat FileStat;
  if (stat(fLogFile,&FileStat)>=0){
    char Commande[MAX_CARACTERES*2];
    sprintf(Commande,"rm %s",fLogFile);
    gSystem->Exec(Commande);
  }
  Update();
}

/*------------------------------------------------------*/
void GVConsol::Update()
{
  RedirectOutput();// allow to close and open log file
  if (fLogFile){
    struct stat FileStat;
    if((stat(fLogFile,&FileStat)>=0)&&fCheckConsol->IsOn()&&fCheckLogFile->IsOn() )
      {
	fConsolTextView->Clear();
	fConsolTextView->LoadFile(fLogFile);
      }
  }
  fConsolTextView->ShowBottom();
}
/*------------------------------------------------------*/
void GVConsol::UpdateLogfile()
{
  // cout << fBarre;cout << "UpdateLogfile\n"<<fBarre;
 TString fname = fFileNameEntry->GetDisplayText();
 TString action ;
 action.Format("cp %s %s_old ",fLogFile,fLogFile);
 strcpy (fLogFile,fname.Data());
 gSystem->Exec(action.Data());
 Update();
}

void GVConsol::Browse()
{
  static TString dir(".");
  TGFileInfo fi;
  fi.fIniDir    = StrDup(dir);
  new TGFileDialog(gClient->GetRoot(), this, kFDSave, &fi);
  dir = fi.fIniDir;
  if(fi.fFilename!=NULL)
    {
      fFileNameEntry->SetText(fi.fFilename);
      UpdateLogfile();
      Update();
    }

}
/*------------------------------------------------------*/
void GVConsol::RedirectOutput()
{
  int Status;
  static bool checkconsolmemory;

 if((fCheckLogFile->IsOn()))
    {

      TString fname = fFileNameEntry->GetDisplayText();
      TString action ;
      action.Format("cp %s %s_old ",fLogFile,fLogFile);
      strcpy (fLogFile,fname.Data());
      gSystem->Exec(action.Data());

      if  (!fCheckConsol ->IsEnabled()){
	fCheckConsol   ->SetEnabled(kTRUE);
	if (checkconsolmemory)  fCheckConsol->SetState(kButtonDown);
	fActualOutput =2;
      }
      fFileNameEntry ->SetEnabled(kFALSE);
      fBrowse        ->SetEnabled(kFALSE);

      Status = gSystem->RedirectOutput(fLogFile,"a");
      fActualOutput =1;

      if (Status <0){
	fCheckLogFile->SetState(kButtonUp);
	fError.TreatError(1 ,Status ,"Redirection to file is impossible!");
	RedirectOutput();
      }
    }
  else{
    if(fCheckConsol->IsOn()) {
      checkconsolmemory = true;
      fCheckConsol->SetState(kButtonUp);
    } else checkconsolmemory = false;
    //fConsolTextView->Activate(kFALSE);
    fFileNameEntry ->SetEnabled(kTRUE);
    fBrowse        ->SetEnabled(kTRUE);
    fCheckConsol   ->SetEnabled(kFALSE);
    Status =1;
    if (fActualOutput!=0)
      Status = gSystem->RedirectOutput(0);
    fActualOutput=0;
    if (Status <0)    fError.TreatError(1 ,Status ,"Redirection  is impossible!");
  }
}

/*------------------------------------------------------*/
void GVConsol::ConsolValidation()
{// initialisation of Consol/logfile


  if (fInitalStateConsol){
    fCheckConsol->SetState(kButtonDown);
  }

  if (fInitalStateLogFile)
    fCheckLogFile->SetState(kButtonDown);
  else{
    fCheckLogFile->SetState(kButtonUp);
    fCheckConsol ->SetState(kButtonUp);
  }

  RedirectOutput();
  ClearConsol();
  if(fCheckConsol->IsOn()){
       cout <<fBarre;
       Update();
      }
}
/*------------------------------------------------------*/
void GVConsol::CreateXML(TXMLEngine* xml, XMLNodePointer_t node){
	// Creat XML congufiration output
	TString tempo;
	tempo = fLogFile;
		xml->NewChild(node, 0, "LogFile", tempo.Data());
}
/*------------------------------------------------------*/
void GVConsol::ReadXML(TXMLEngine* xml, XMLNodePointer_t node){
	//Read XML conguration output
	TString tempo;
	tempo =xml->GetNodeName(node);
	if (tempo== "LogFile") {
		tempo = xml->GetNodeContent(node);
		fFileNameEntry->SetText(tempo.Data());
		SetLogFile((char*)tempo.Data());
		tempo.Form("Log Associeted file %s", tempo.Data());
		fError.TreatError(0, 0, tempo.Data());
		Update();
	}
}

