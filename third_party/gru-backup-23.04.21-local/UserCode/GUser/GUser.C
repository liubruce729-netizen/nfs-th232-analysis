// File : GUser.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GUser
//
// Class for User treatment
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


#include "./GUser.h"
#include "TROOT.h"
#include <TProfile.h>
#include <TRandom.h>
//_________________________________global_variables______________________________


//______________________________________________________________________________

ClassImp (GUser);

GUser::GUser (GDevice* _fDevIn, GDevice* _fDevOut):GAcq(_fDevIn,_fDevOut)
{
  // Constructor/initialisator of Acquisition object
  //
  // entry:
  // - Input Device
  // - Output Device
  fDevIn         = _fDevIn;
  fDevOut        = _fDevOut;

}

//_____________________________________________________________________________

GUser::~GUser()  {
  //Destructor of class GUser
   gROOT->cd();

}

//______________________________________________________________

void GUser::InitUser()
{
  // Initialisation for global  user treatement
  // In this method , we can make histograms (prevously declared in GUser.h)
  // Ex : in  GUser.h we have declared           TH1I *fMyHisto ;
  //      in this methode we make it             fHisto = new TH1I ("MyHisto","MyHisto",1024,0,1024);
  //      we can include it in database          GetSpectra()->AddSpectrum(fMyHisto,"MyFamily");


  
}
//______________________________________________________________

void GUser::InitUserRun()
{
  // Initialisation for user treatemeant for each  run
  // For specific user treatement

 
}
//______________________________________________________________
void GUser::User()
{
  // User method for user treatement for each events
  // the event is presented on two ways ( user can use one or other )
  // 1 - event is a vector of a serie of couples  UShor_t/Short_t Label/Value ( GetEventArrayLabelValue())
  //      of variable size   GetEventArrayLabelValueSize() and  with a max size of GetEventArrayLabelValueSizeMax()
  //      The numbers of couple  Label/Value is GetEventArrayLabelValueSize()/2
  //      GetEventArrayLabelValue_Label(i)  return  Label number i in  vector GetEventArrayLabelValue()
  //      GetEventArrayLabelValue_Value(i)  return  Value number  i in  vector GetEventArrayLabelValue()
  //
  //      Exemple of use of manage index,label,name
  //      GetEvent()->GetDataParameters()->GetLabel("NAME")  return label of parameter with name "NAME"
  //      GetEvent()->GetDataParameters()->GetLabel(index)   return label of parameter with index i (in GetEventArray() vector)
  //      GetEvent()->GetDataParameters()->GetParNameFromLabel(label) return name of parameter with label 'label'
  //      GetEvent()->GetDataParameters()->GetIndex(label)   return index(in GetEventArray() vector) of parameter with label 'label
  //      GetEvent()->GetDataParameters()->GetIndex("NAME")  return index(in GetEventArray() vector) of parameter with name "NAME"
  // 2  - event is a vector of UShor_t* GetEventArray() of fixed size  (GetEventArraySize())
  //      '0's have been included in vector where parameter were not present
  //      GetEventArray_Value(i) return value index i in vector GetEventArray()
  //      In case of multi value for same label, this way must not be used


}


//______________________________________________________________
void GUser::EndUserRun()
{

 //  end of run ,  executed a end of each run

}

//______________________________________________________________
void GUser::EndUser()
{
  // globlal final end executed a end of runs
  // must be explicitly called!


}
//______________________________________________________________________________

void GUser::InitTTreeUser()
{
  // User method for specfic initialisation of TTree
  // It can be usefull for example multi-hit detections
  // or to have a TTree with only few parameters ( for low compute)
  // to run this method , you have to slect mode 3 in  SetTTreeMode
  // ex : a->SetTTreeMode(3,"/space/MyTTree.root");
  // GetTree() return TTree pointer

  // ex : GetTree()->Branch("mybranche",&GetEventArrayLabelValue(),"mybranche/s");

}
