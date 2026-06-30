// File : GVClass.C
// Author: J�r�me Chauveau
//////////////////////////////////////////////////////////////////////////////
//class GVClass
//Frame to class spectra. Contains a tree list and 2     
//          tabs: 1 to a manual classification                    
//                   and an other to automatic classification  
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

#include <TApplication.h>


#include "GVClass.h"

#include "Riostream.h"


ClassImp(GVClass)


/*------------------------------------------------------------------------------------------------------------------*/
//constructor
  GVClass::GVClass(GSpectraDB *DB,UInt_t w, UInt_t h):TGTransientFrame(gClient->GetRoot(),gClient->GetRoot(),w,h)
{
  spectraDB = DB;
  
  gClient->GetColorByName("#e0e0e0", lightgrey);//bg color
  SetLayoutManager(new TGHorizontalLayout(this));
  fGroupBrowser = new TGGroupFrame(this,"Classification");
  fGroupBrowser->SetBackgroundColor(lightgrey);
  fBrowser = new GVBrowser(fGroupBrowser,100,200,spectraDB);
  fGroupBrowser->AddFrame(fBrowser);
  AddFrame(fGroupBrowser);
  tab = new TGTab(this);
  AddFrame(tab,new TGLayoutHints(kLHintsExpandX |kLHintsExpandY));
  tabManual = tab->AddTab("Manual");
  tabAuto = tab->AddTab("Auto");
  fManual = new GVClassManual(tabManual,800,600,fBrowser,spectraDB);
  fAuto = new GVClassAuto(tabAuto,800,600,fManual,fBrowser,spectraDB);
  tabManual->AddFrame(fManual);
  tabAuto->AddFrame(fAuto);
  
  SetBackgroundColor(lightgrey);
  tabAuto->SetBackgroundColor(lightgrey);
  tabManual->SetBackgroundColor(lightgrey);


  SetWindowName("Classification");
  //displaying
  Layout();
  MapSubwindows();
  CenterOnParent();
  MapWindow();
  GetClient()->WaitFor(this);
  
}
/*------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------*/
//destructor
GVClass::~GVClass()
{
  /*cout<<"1"<<endl;
  //delete fGroupBrowser;
  cout<<"2"<<endl;
  delete fBrowser;
  cout<<"3"<<endl;
  delete tab;
  delete tabManual;
  delete tabAuto;
  cout<<"4"<<endl;
  if(fBrowser)
    delete fBrowser;
  if(fAuto)
    delete fAuto;
  if(fManual)
    delete fManual;
  cout<<"5"<<endl;*/
	DeleteWindow();
}
/*------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------*/
void GVClass::CloseWindow()
{
  gApplication->Terminate();
  delete this;
}
/*------------------------------------------------------------------------------------------------------------------*/
