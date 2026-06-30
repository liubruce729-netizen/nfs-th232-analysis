// File : GVSpectraInfo.C
// Author: Jerome Chauveau
//////////////////////////////////////////////////////////////////////////////
//class GVSpectraInfo
//Graphic row displaying informations about a spectrum
//
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

#include "GVSpectraInfo.h"
#include <Riostream.h>


#include <TGLabel.h>
ClassImp(GVSpectraInfo)

  GVSpectraInfo::GVSpectraInfo(const TGWindow *p, TString name, TString source,TString sourceName):TGCompositeFrame(p)
{

  gClient->GetColorByName("white", white);
  gClient->GetColorByName("orange",orange);
  gClient->GetColorByName("green", green);
  gClient->GetColorByName("#e0e0e0", lightgrey);

  SetBackgroundColor(lightgrey);

  textName       = new TGTextView(this,150,22,name);
  textSourceType = new TGTextView(this,150,22,source);
  textSource     = new TGTextView(this,150,22,sourceName);

  fCheck = new TGCheckButton(this);
  SetLayoutManager(new TGHorizontalLayout(this));
  
  AddFrame(fCheck,new TGLayoutHints(kLHintsCenterY));
  AddFrame(textName);
  AddFrame(textSourceType);
  AddFrame(textSource);

  fCheck->SetBackgroundColor(lightgrey);
  fCheck->Connect("Clicked()","GVSpectraInfo",this,"ClickCheck()");
  
  AddInput(kButtonPressMask | kButtonReleaseMask);
  
  Connect("ProcessedEvent(Event_t)","GVSpectraInfo",this,"HandleButtonClick(Event_t)");
}


GVSpectraInfo::~GVSpectraInfo(){}

void GVSpectraInfo::Select(Bool_t down)
{
  if(down)
    fCheck->SetState(kButtonDown);
  else
    fCheck->SetState(kButtonUp);
  ClickCheck();
}



void GVSpectraInfo::ClickCheck()
{
  if(fCheck->IsDown())
    {
      //textName->SetBackgroundColor(green);
      textName->ChangeBackground(green);
      textName->Update();      
      //textSourceType->SetBackgroundColor(green);
      textSourceType->ChangeBackground(green);
      textSourceType->Update();
      //textSource->SetBackgroundColor(green);
      textSource->ChangeBackground(green);
      textSource->Update();
      textName->Update();
      textSourceType->Update();
      textSource->Update();
	
    }
  else
    {
      textName->SetBackgroundColor(white);
      textName->ChangeBackground(white);
      textName->Update();
      textSourceType->SetBackgroundColor(white);
      textSourceType->ChangeBackground(white);
      textSourceType->Update();
      textSource->SetBackgroundColor(white);
      textSource->ChangeBackground(white);
      textSource->Update();
      textName->Update();
      textSourceType->Update();
      textSource->Update();
    }
}

Bool_t GVSpectraInfo::IsSelected()
{
  return fCheck->IsDown();
}

TString GVSpectraInfo::GetSpectraName()
{
  return textName->GetText()->GetCurrentLine()->GetText();
}

TString GVSpectraInfo::GetSourceType()
{
  return textSourceType->GetText()->GetCurrentLine()->GetText();
}

Bool_t GVSpectraInfo::HandleButton(Event_t *event)
{
  if(event->fType == kButtonRelease)
    {
      if(fCheck->IsDown())
	{
	  fCheck->SetState(kButtonUp);
	}
      else
	{
	  fCheck->SetState(kButtonDown);
	}
      ClickCheck();
    }
  return kTRUE;
}
