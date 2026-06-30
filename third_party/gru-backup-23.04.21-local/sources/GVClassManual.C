// File : GVClassManual.C
// Author: J�r�me Chauveau
//////////////////////////////////////////////////////////////////////////////
//class GVClassManual
//Frame to manual classification. User can choose 
//spectra which he wants to class
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




#include "GVClassManual.h"
#include "GVSpectraInfo.h"

#include <TGTableLayout.h>
#include <TGTextEntry.h>
#include <TGTextView.h>
#include <TObjString.h>
#include <TTimer.h>
#include <TVirtualPadEditor.h>
#include <Riostream.h>
#include <TGMsgBox.h>
#include "Vigru.h"



ClassImp(GVClassManual)

/*------------------------------------------------------------------------------------------------------------------*/ 
//constructor
  GVClassManual::GVClassManual(const TGWindow *p,UInt_t w, UInt_t h,GVBrowser *browser,GSpectraDB* DB):TGCompositeFrame(p,w,h)
{
 
  fBrowser = browser;
  spectraDB = DB;

  //colors
  gClient->GetColorByName("white", white);
  gClient->GetColorByName("#e0e0e0", lightgrey);
  gClient->GetColorByName("black", black);
  //fMain = (TGMainFrame*)tg;
  SetBackgroundColor(lightgrey);
  
  //panel with ok and cancel buttons
  //fButtons = new TGCompositeFrame(this);
  //fButtons->SetLayoutManager(new TGMatrixLayout(fButtons,0,2,10));
  //fButtons->SetBackgroundColor(lightgrey);
  //fButtonOk = new TGTextButton(fButtons,"OK");
  //fButtonCancel =  new TGTextButton(fButtons,"Cancel");
  //fButtonOk->Resize(50,30);
  //fButtonCancel->Resize(50,30);
  //add buttons to the buttons frame
 // fButtons->AddFrame(fButtonOk);
  //fButtons->AddFrame(fButtonCancel);

  //fcenter will contains browser and spectras table
  fCenter = new TGCompositeFrame(this);
  fCenter->SetBackgroundColor(lightgrey);
  fCenter->SetLayoutManager(new TGHorizontalLayout(fCenter));
  
  //spectras table
  fGroupTable = new TGGroupFrame(fCenter,"Available spectras");
  fCanvas = new TGCanvas(fGroupTable,520, 600,kSunkenFrame);
  fScrollTable = new TGCompositeFrame(fCanvas->GetViewPort());
  fCanvas->SetContainer(fScrollTable);
  fTableSpectras = new TGCompositeFrame(fScrollTable);
  SetBackgroundColor(lightgrey);
  fScrollTable->SetBackgroundColor(lightgrey);
  //columns titles
  fTitles = new TGCompositeFrame(fTableSpectras);
  fTitles->SetLayoutManager(new TGHorizontalLayout(fTitles));
  fTitles->SetBackgroundColor(lightgrey);
  fCheckAll = new TGCheckButton(fTitles);
  fCheckAll->ChangeBackground(lightgrey);
  TGTextView *cola = new TGTextView(fTitles,150,30,"    Spectra name");
  cola->SetBackgroundColor(black);
  cola->ChangeBackground(black);
  TGTextView *colb = new TGTextView(fTitles,150,30,"    Source type");
  colb->SetBackgroundColor(black);
  colb->ChangeBackground(black);
  TGTextView *colc = new TGTextView(fTitles,150,30,"    Source name");
  colc->SetBackgroundColor(black);
  colc->ChangeBackground(black);
  cola->Update();
  colb->Update();
  colc->Update();
  fTitles->AddFrame(fCheckAll);
  fTitles->AddFrame(cola);
  fTitles->AddFrame(colb);
  fTitles->AddFrame(colc);
  fTableSpectras->AddFrame(fTitles);

  //panel with add and remove buttons
  fArrows = new TGCompositeFrame(fCenter);
  fArrows->SetBackgroundColor(lightgrey);
  fButtonAdd = new TGPictureButton(fArrows,"arrow_left.xpm");
  fButtonRemove = new TGPictureButton(fArrows,"arrow_right.xpm");
  fArrows->AddFrame(fButtonAdd);
  fArrows->AddFrame(fButtonRemove);
  
  fCenter->AddFrame(fArrows,new TGLayoutHints(kLHintsCenterX | kLHintsCenterY));
  fCenter->AddFrame(fGroupTable);
  fGroupTable->AddFrame(fCanvas);
  fScrollTable->AddFrame(fTableSpectras);
  fGroupTable->SetBackgroundColor(lightgrey); 

  AddFrame(fCenter,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  //AddFrame(fButtons,new TGLayoutHints(kLHintsBottom | kLHintsRight));

  //connections slots
   //fButtonCancel->Connect("Clicked()", "GVClassManual", this, "DoCancel()");
   //fButtonOk->Connect("Clicked()", "GVClassManual", this, "DoOk()");
   //fButtonOk->Connect("Clicked()", "GVMenuSpectra", this, "UpdateTree");
   fButtonAdd->Connect("Clicked()", "GVClassManual", this, "AddSpectra()");
   fButtonRemove->Connect("Clicked()", "GVClassManual", this, "RemoveSpectra()");
   fCheckAll->Connect("Clicked()", "GVClassManual", this, "SelectAll()");
   
   //spectrasInfos
   spectrasInfoList = new TObjArray(1,0);
   FillFromDB(spectraDB);
   GetClient()->WaitFor(this);
}
/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
GVClassManual::~GVClassManual()
{
	Cleanup();
  
	DeleteWindow(); 
}
/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassManual::CloseWindow()
{
  //delete this;
  UnmapWindow();
}
/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassManual::DoOk()
{
 /* Vigru *f = (Vigru*)fMain;
  f->UpdateMenuSpectraTree();*/
  //DoCancel();
  
//A VOIRRRRRRRRRRRRR
}
/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassManual::DoCancel(){

  TTimer::SingleShot(150, "GVClassManual", this, "CloseWindow()");
  // Close the Ged editor if it was activated.
  if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
    TVirtualPadEditor::Terminate();

}
/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassManual::AddSpectra()
{  
  if(fBrowser->GetTree()->GetSelected())
    {
      if(fBrowser->IsPossibleToAdd())
	{
	  GVSpectraInfo *current = NULL;
	  for(Int_t i = 0;i <spectrasInfoList->GetLast(); i++)
	    {
	      current = (GVSpectraInfo*)spectrasInfoList->At(i);
	      if(current->IsSelected())	  
		{
		  GSpectrumIdentity *identity = spectraDB->GetIdentity(i);
		  fBrowser->AddItem(identity,"");
		  fBrowser->SeClassified(identity->GetSpectrumName(),kTRUE);
		  
		}
	      
	    }
	}
      else
	{
	  Int_t retval = 0;
	  EMsgBoxIcon icontype = kMBIconStop; 
	  new TGMsgBox(gClient->GetRoot(), this,
		       "Warning", "You can only add spectra(s) in a family folder",
		       icontype, 1, &retval);
	}
    }
  else
    {
      cout<<"not selected folder"<<endl;
    }
  spectraDB->MakeDUMP("NAWAk.DUMP");
}
/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassManual::ReConstruct()
{
 /* if(fTableSpectras)
    {
      delete fTableSpectras;
    }

  //spectras table
  fTableSpectras = new TGCompositeFrame(fScrollTable);
  fTableSpectras->SetBackgroundColor(lightgrey);
  fTableSpectras->ChangeBackground(lightgrey);
  //columns titles
  fTitles = new TGCompositeFrame(fTableSpectras);
  fTitles->SetLayoutManager(new TGHorizontalLayout(fTitles));
  fTitles->SetBackgroundColor(lightgrey);
  fCheckAll = new TGCheckButton(fTitles);
  fCheckAll->ChangeBackground(lightgrey);
  TGTextView *cola = new TGTextView(fTitles,150,30,"   Spectra name");
  cola->SetBackgroundColor(black);
  cola->ChangeBackground(black);
  TGTextView *colb = new TGTextView(fTitles,150,30,"   Source type");
  colb->SetBackgroundColor(black);
  colb->ChangeBackground(black);
  TGTextView *colc = new TGTextView(fTitles,150,30,"   Source name");
  colc->SetBackgroundColor(black);
  colc->ChangeBackground(black);
  cola->Update();
  colb->Update();
  colc->Update();

  
  fTitles->AddFrame(fCheckAll,new TGLayoutHints(kLHintsCenterY));
  fTitles->AddFrame(cola);
  fTitles->AddFrame(colb);
  fTitles->AddFrame(colc);
  fTableSpectras->AddFrame(fTitles); 
  
  fCheckAll->Connect("Clicked()", "GVClassManual", this, "SelectAll()");*/
 
}
/*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassManual::Show()
{
 Layout();
  MapSubwindows();
  MapWindow(); 
  Resize(GetDefaultSize());//resize to redraw all components(without Resize(),TGTextView have a bad display) 
  Resize(820,700);
}
/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassManual::FillTableSpectras(TObjArray *list, Int_t type, Bool_t append)
{
/* TString source;
  if(type == 1)
    source = "NET";
  else
    source = "FILE";
  for(int i =0;i<=list->GetLast();i++)
    {
       TObjString *str = (TObjString*)list->At(i);
       GVSpectraInfo *info = new GVSpectraInfo(fTableSpectras,str->GetName(),source,"nawak");
       fTableSpectras->AddFrame(info);
       spectrasInfoList->Add(info);
       //also add spectras name in the Tree (in "all" folder)
       fBrowser->AddItem(str->GetName(),"Raw",source);
       
    }*/
}
/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassManual::FillFromDB(GSpectraDB *DB)
{
  GSpectrumIdentity *identity = NULL;
  //TGListTreeItem *item = NULL;
  cout<<"TAILLE "<<DB->GetLast()<<endl;
  for(Int_t i = 0;i<=DB->GetLast();i++)
    {
      identity = (GSpectrumIdentity*)DB->At(i); 
      GVSpectraInfo *info = new GVSpectraInfo(fTableSpectras,identity->GetSpectrumName(),identity->GetSource(),identity->GetSourceName());
      fTableSpectras->AddFrame(info);
      spectrasInfoList->Add(info);
      fBrowser->AddItem(identity,"Raw");
      //item->SetTipText((identity->ToString()).Data());
    }
}
/*------------------------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassManual::SelectAll()
{
  Bool_t down = fCheckAll->IsDown();
  for(Int_t i = 0;i <=spectrasInfoList->GetLast(); i++)
    {
      ((GVSpectraInfo*)spectrasInfoList->At(i))->Select(down);
    }
}

void GVClassManual::AddTreeToFill(GVListTree *tree)
{
  fBrowser->AddTreeToFill(tree);
}
/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
TObjArray * GVClassManual::TestFill(TString str)
{
  TObjArray *array = new TObjArray(1,0);
  //char toto[32];
  //strcpy (toto,str);
  //TString s;;
  for(Int_t i =0; i<10;i++)
    {
       //sprintf (s,"%d",i);
       str+=i;
       array->Add(new TObjString(str));
    }
  return array;
}
/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
TObjArray* GVClassManual::GetSpectraNames()
{
  TObjArray *names = new TObjArray(1,0);
  for(Int_t i =0; i<=spectrasInfoList->GetLast();i++)
    {
      names->Add(new TObjString(((GVSpectraInfo*)spectrasInfoList->At(i))->GetSpectraName()));
    }
  return names;
}
/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassManual::RemoveSpectra()
{
  fBrowser->SeClassified(fBrowser->GetTree()->GetSelected()->GetText(),kFALSE);
  fBrowser->GetTree()->DeleteItem(fBrowser->GetTree()->GetSelected());


}
/*------------------------------------------------------------------------------------------------------------------*/ 
