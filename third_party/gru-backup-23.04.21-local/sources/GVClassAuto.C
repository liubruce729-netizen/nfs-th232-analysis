
// File : GVClassAuto.C
// Author: J�r�me Chauveau
//////////////////////////////////////////////////////////////////////////////
//class GVClassAUto
//Frame to automatic classification
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

#include "GVClassAuto.h"
#include <TGMsgBox.h>
#include <TBox.h>
#include <TGNumberEntry.h>


#include <Riostream.h>


ClassImp(GVClassAuto)


/*------------------------------------------------------------------------------------------------------------------*/ 
  GVClassAuto::GVClassAuto(const TGWindow *p,UInt_t w, UInt_t h, GVClassManual* manual, GVBrowser *browser,GSpectraDB *DB):TGCompositeFrame(p,w,h)
{//constructor  

  this->manual = manual;
  this->fBrowser = browser;
  this->spectraDB = DB;

  //colors
  gClient->GetColorByName("white", white);
  gClient->GetColorByName("#e0e0e0", lightgrey);


  SetLayoutManager(new TGHorizontalLayout(this));
  tree = fBrowser->GetTree();
  fCenter = new TGGroupFrame(this,"Configuration :");
  fOptions = new TGCompositeFrame(fCenter);      
  fFrameParent = new TGCompositeFrame(fCenter);
  fFrameParent->SetLayoutManager(new TGHorizontalLayout(fFrameParent));
  labParent = new TGLabel(fFrameParent,"Parent folder : ");
  fEntryParent = new TGTextEntry(fFrameParent,"Classified");
  fFrameParent->AddFrame(labParent);
  fFrameParent->AddFrame(fEntryParent);
  fEntryParent->Resize(200,20);
  fCenter->AddFrame(fFrameParent);
  fCenter->AddFrame(fOptions);
  fFrame1 = new TGCompositeFrame(fOptions);
  fFrame1->SetLayoutManager(new TGHorizontalLayout(fFrame1));
  fOption1 = new TGGroupFrame(fFrame1,"Option 1:");
  fOption1->SetLayoutManager(new TGHorizontalLayout(fOption1));
  fOptions->AddFrame(fFrame1,new TGLayoutHints(kLHintsExpandX |kLHintsExpandY));
  fFrame2 = new TGCompositeFrame(fOptions);
  fFrame2->SetLayoutManager(new TGHorizontalLayout(fFrame2));
  fOption2 = new TGGroupFrame(fFrame2,"Option 2:");
  fOption2->SetLayoutManager(new TGHorizontalLayout(fOption2));
  fOptions->AddFrame(fFrame2,new TGLayoutHints(kLHintsExpandX |kLHintsExpandY));
  fFrame3 = new TGCompositeFrame(fOptions);
  fFrame3->SetLayoutManager(new TGHorizontalLayout(fFrame3));
  fOption3 = new TGGroupFrame(fFrame3,"Option 3:");
  fOption3->SetLayoutManager(new TGHorizontalLayout(fOption3));
  fOptions->AddFrame(fFrame3,new TGLayoutHints(kLHintsExpandX |kLHintsExpandY));
  
  
  fRadio1 = new TGRadioButton(fFrame1);
  lab1 = new TGLabel(fOption1,"From letter   ");
  fEntry1 = new TGNumberEntry(fOption1);
  lab4 = new TGLabel(fOption1,"   to   ");
  fEntry2 = new TGNumberEntry(fOption1);
  lab9 = new TGLabel(fOption1,"  Text:  ");
  fEntryA = new TGTextEntry(fOption1);
  fFrame1->AddFrame(fRadio1,new TGLayoutHints(kLHintsCenterY));
  fFrame1->AddFrame(fOption1,new TGLayoutHints(kLHintsExpandX |kLHintsExpandY)); 
  fOption1->AddFrame(lab1);
  fOption1->AddFrame(fEntry1);
  fOption1->AddFrame(lab4);
  fOption1->AddFrame(fEntry2);
  fOption1->AddFrame(lab9);
  fOption1->AddFrame(fEntryA);
  fRadio2 = new TGRadioButton(fFrame2);
  lab2 = new TGLabel(fOption2,"From letter   ");
  fEntry3 = new TGNumberEntry(fOption2);
  lab5 = new TGLabel(fOption2,"   to   ");
  fEntry4 = new TGNumberEntry(fOption2);
  lab6 = new TGLabel(fOption2,"   &   ");
  fEntry5 = new TGNumberEntry(fOption2);
  lab7 = new TGLabel(fOption2,"   to   ");
  fEntry6 = new TGNumberEntry(fOption2);
  lab11 = new TGLabel(fOption2,"  Text 1:");
  fEntry9 = new TGTextEntry(fOption2);
  fEntry9->Resize(50,20);
  lab12 = new TGLabel(fOption2,"Text 2:");
  fEntry10 = new TGTextEntry(fOption2);
  fEntry10->Resize(50,20);
  fFrame2->AddFrame(fRadio2,new TGLayoutHints(kLHintsCenterY));
  fFrame2->AddFrame(fOption2,new TGLayoutHints(kLHintsExpandX |kLHintsExpandY));  
  fOption2->AddFrame(lab2);
  fOption2->AddFrame(fEntry3);
  fOption2->AddFrame(lab5);
  fOption2->AddFrame(fEntry4);
  fOption2->AddFrame(lab6);
  fOption2->AddFrame(fEntry5);
  fOption2->AddFrame(lab7);
  fOption2->AddFrame(fEntry6);
  fOption2->AddFrame(lab11);
  fOption2->AddFrame(fEntry9);
  fOption2->AddFrame(lab12);
  fOption2->AddFrame(fEntry10);
  fRadio3 = new TGRadioButton(fFrame3);
  lab3 = new TGLabel(fOption3,"Separator character   ");
  fEntry7 = new TGTextEntry(fOption3,"_");
  lab8 = new TGLabel(fOption3,"     Fragment :  ");
  fEntry8 = new TGNumberEntry(fOption3);
  lab10 = new TGLabel(fOption3,"  Text:  ");
  fEntryB = new TGTextEntry(fOption3);
  fFrame3->AddFrame(fRadio3,new TGLayoutHints(kLHintsCenterY));
  fFrame3->AddFrame(fOption3,new TGLayoutHints(kLHintsExpandX |kLHintsExpandY)); 
  fOption3->AddFrame(lab3);
  fOption3->AddFrame(fEntry7);
  fOption3->AddFrame(lab8);
  fOption3->AddFrame(fEntry8);
  fOption3->AddFrame(lab10);
  fOption3->AddFrame(fEntryB);
  fButtons = new TGCompositeFrame(fCenter);
  fButtons->SetLayoutManager(new TGHorizontalLayout(fButtons));
  fButtonOk= new TGTextButton(fButtons,"OK");
  fButtonReset = new TGTextButton(fButtons,"Reset");
  fCenter->AddFrame(fButtons,new TGLayoutHints(kLHintsRight));
  fButtons->AddFrame(fButtonOk);
  fButtons->AddFrame(fButtonReset);
  AddFrame(fCenter);
  MapSubwindows();
  MapWindow();

  //slots
  fRadio1->Connect("Clicked()","GVClassAuto",this,"ClickOpt1()");
  fRadio2->Connect("Clicked()","GVClassAuto",this,"ClickOpt2()");
  fRadio3->Connect("Clicked()","GVClassAuto",this,"ClickOpt3()");
  fButtonOk->Connect("Clicked()","GVClassAuto",this,"DoOk()");

  option = 0;
  families = new TObjArray(1,0);

  //set the tree textEntry
  browser->GetTree()->SetTextEntry(fEntryParent);

  //set colors
  SetBackgroundColor(lightgrey);
  fButtons->SetBackgroundColor(lightgrey);
  fCenter->SetBackgroundColor(lightgrey);
  fOptions->SetBackgroundColor(lightgrey);
  fOption1->SetBackgroundColor(lightgrey);
  fOption2->SetBackgroundColor(lightgrey);
  fOption3->SetBackgroundColor(lightgrey);
  fFrame1->SetBackgroundColor(lightgrey);
  fFrame2->SetBackgroundColor(lightgrey);
  fFrame3->SetBackgroundColor(lightgrey);
  fFrameParent->SetBackgroundColor(lightgrey);
  lab1->SetBackgroundColor(lightgrey);
  lab2->SetBackgroundColor(lightgrey);
  lab3->SetBackgroundColor(lightgrey);
  lab4->SetBackgroundColor(lightgrey);
  lab5->SetBackgroundColor(lightgrey);
  lab6->SetBackgroundColor(lightgrey);
  lab7->SetBackgroundColor(lightgrey);
  lab8->SetBackgroundColor(lightgrey);
  lab9->SetBackgroundColor(lightgrey);
  lab10->SetBackgroundColor(lightgrey);
  lab11->SetBackgroundColor(lightgrey);
  lab12->SetBackgroundColor(lightgrey); 
  labParent->SetBackgroundColor(lightgrey);
  fRadio1->SetBackgroundColor(lightgrey);
  fRadio2->SetBackgroundColor(lightgrey);
  fRadio3->SetBackgroundColor(lightgrey);
  GetClient()->WaitFor(this);
}
/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
GVClassAuto::~GVClassAuto()

{
	DeleteWindow();
}
/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassAuto::ClickOpt1()
{//first option: class on a word fragment
  if(fRadio1->IsDown())
    {
      fRadio2->SetState(kButtonUp);
      fRadio3->SetState(kButtonUp);
      option = 1;
    }  
}
/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassAuto::ClickOpt2()
{//second option: class on 2 word fragments
  if(fRadio2->IsDown())
    {
      fRadio1->SetState(kButtonUp);
      fRadio3->SetState(kButtonUp);
      option = 2;
    } 
} 
/*------------------------------------------------------------------------------------------------------------------*/ 
 
/*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassAuto::ClickOpt3()
{//third option
  if(fRadio3->IsDown())
    {
      fRadio2->SetState(kButtonUp);
      fRadio1->SetState(kButtonUp);
      option = 3;
    }  
}
/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassAuto::AutoClassOpt1()
{
  if(tree->FindItemByPathname(fEntryParent->GetText())!=NULL)
    {
      TObjArray *list = manual->GetSpectraNames();
      Int_t first = (Int_t)fEntry1->GetNumber();
      Int_t last = (Int_t)fEntry2->GetNumber();
      //TString currenti, currentj;
      TString expr = fEntryA->GetText();
      TString current;
      TGListTreeItem *item = NULL;
      const TGPicture *pict = gClient->GetPicture("marker29.xpm");
      
      //item = tree->AddFamily(tree->FindItemByPathname(fEntryParent->GetText()),expr);
      item = tree->FindItemByPathname(fEntryParent->GetText());
      
      for(Int_t i = 0;i<=list->GetLast();i++)
	{
	  current = ((TObjString*)list->At(i))->GetName();
	  if(current(first,last-first)==expr)
	    {
	      tree->AddItem(item,current,pict,pict);
	      fBrowser->SeClassified(current,kTRUE);
	    }
	  
	}
    }
     else
       {
	 int retval;
	 EMsgBoxIcon icontype = kMBIconQuestion; 
	 new TGMsgBox(gClient->GetRoot(), this,
		      "Warning", "This family do not exists, please create it!",
		      icontype, 1, &retval); 
       }
  
}
  
  /*------------------------------------------------------------------------------------------------------------------*/ 
  
  /*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassAuto::AutoClassOpt2()
{



  if(tree->FindItemByPathname(fEntryParent->GetText())!=NULL)
    {
      TObjArray *list = manual->GetSpectraNames();
      Int_t first1 = (Int_t)fEntry3->GetNumber();
      Int_t last1 = (Int_t)fEntry4->GetNumber();
      Int_t first2 = (Int_t)fEntry5->GetNumber();
      Int_t last2 = (Int_t)fEntry6->GetNumber();
      //TString currenti, currentj;
      TString expr = fEntryA->GetText();
      TString expr2 = fEntryB->GetText();
      TString current;
      TGListTreeItem *item = NULL;
      const TGPicture *pict = gClient->GetPicture("marker29.xpm");
      
      //item = tree->AddFamily(tree->FindItemByPathname(fEntryParent->GetText()),expr);
      item = tree->FindItemByPathname(fEntryParent->GetText());
      
      for(Int_t i = 0;i<=list->GetLast();i++)
	{
	  current = ((TObjString*)list->At(i))->GetName();
	  if(current(first1,last1-first1)==expr && current(first2,last2-first2)==expr2)
	    {
	      tree->AddItem(item,current,pict,pict);
	      fBrowser->SeClassified(current,kTRUE);
	    }
	}
    }
     else
       {
	 int retval;
	 EMsgBoxIcon icontype = kMBIconQuestion; 
	 new TGMsgBox(gClient->GetRoot(), this,
		      "Warning", "This family do not exists, please create it!",
		      icontype, 1, &retval); 
       }

  
}
/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassAuto::AutoClassOpt3()
{
  if(tree->FindItemByPathname(fEntryParent->GetText())!=NULL)
    { 
      TObjArray *list = manual->GetSpectraNames();
      TString separator = fEntry7->GetText();
      Int_t index = (Int_t)fEntry8->GetNumber();
      TString current;
      TString expr =  fEntryB->GetText();
      TGListTreeItem *item = NULL;
      const TGPicture *pict = gClient->GetPicture("marker29.xpm");
      //item = tree->AddFamily(tree->FindItemByPathname(fEntryParent->GetText()),expr);
      item = tree->FindItemByPathname(fEntryParent->GetText());
      for(Int_t i = 0;i<=list->GetLast();i++)
	{
	  current = ((TObjString*)list->At(i))->GetName();
	  TObjArray *tab = current.Tokenize(separator);
	  TString s = ((TObjString*)tab->At(index))->GetName();
	  if(expr == s)
	    {
	      tree->AddItem(item,current,pict,pict);
	      fBrowser->SeClassified(current,kTRUE);
	    }
	}
    }
  else
    {
      int retval;
      EMsgBoxIcon icontype = kMBIconQuestion; 
      new TGMsgBox(gClient->GetRoot(), this,
		   "Warning", "This family do not exists, please create it!",
		   icontype, 1, &retval); 
    }
}
 

/*------------------------------------------------------------------------------------------------------------------*/ 

/*------------------------------------------------------------------------------------------------------------------*/ 
void GVClassAuto::DoOk()
{
  switch(option)
    {
    case 1:
      AutoClassOpt1();
      break;
    case 2:
      AutoClassOpt2();
      break;
    case 3:
      AutoClassOpt3();
      break;

    }
}
/*------------------------------------------------------------------------------------------------------------------*/ 
