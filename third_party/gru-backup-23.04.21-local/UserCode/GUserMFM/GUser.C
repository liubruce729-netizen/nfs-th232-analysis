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
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "GEventMFM.h"

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

fVerbose=1;
}

//_____________________________________________________________________________

GUser::~GUser()  {
  //Destructor of class GUser  
  

if (fCoboframe)   delete (fCoboframe);
if (fExoframe)    delete (fExoframe);
if (fEbyframe)    delete (fEbyframe);
if (fInsideframe) delete (fInsideframe);
if (fMergeframe)  delete (fMergeframe);  


gROOT->cd();


}

//______________________________________________________________

void GUser::InitUser()
{
  // Initialisation for global  user treatement
  // choice: allow  to switch on different treatements (1 to 6)
  // called with command SetUserMode( choice )
  // In this method , we can make histograms (prevously declared in GUser.h)
  // Ex : in  GUser.h we have declared           TH1I *fMyHisto ;
  //      in this methode we make it             fHisto = new TH1I ("MyHisto","MyHisto",1024,0,1024);
  //      we can include it in database          GetSpectra()->AddSpectrum(fMyHisto,"MyFamily");
  

fCoboframe   = new MFMCoboFrame();
fExoframe    = new MFMExogamFrame();
fEbyframe    = new MFMEbyedatFrame();
fInsideframe = new MFMCommonFrame();
fMergeframe  = new MFMMergeFrame();  


  
 // Specific init for Ebyedat 
  
  // exemple of full a 2D histo 
  // UserEbyFrame method for user treatement for each events

  // The event is a frame of a serie of couples  UShor_t/Short_t Label/Value 
  //      Exemple of use of manage index,label,name
  //      GetEvent()->GetDataParameters()->GetLabel("NAME")  return label of parameter with name "NAME"
  //      GetEvent()->GetDataParameters()->GetLabel(index)   return label of parameter with index i 
  //      GetEvent()->GetDataParameters()->GetParNameFromLabel(label) return name of parameter
  //      GetEvent()->GetDataParameters()->GetIndex(label)   return index of parameter with label 'label
  //      GetEvent()->GetDataParameters()->GetIndex("NAME")  return index of parameter with name "NAME"
  // Initialisation for global  user treatement
  // In this method , we can make histograms (prevously declared in GUser.h)
  // Ex : in  GUser.h we have declared           TH1I *fMyHisto ;
  //      in this methode we make it             fHisto = new TH1I ("MyHisto","MyHisto",1024,0,1024);
  //      we can include it in database          GetSpectra()->AddSpectrum(fMyHisto,"MyFamily")
  
 // exemple of a memorization of 2 labels
 fLabel1 = GetEvent()->GetDataParameters()->GetLabel("NAME1");
 fLabel2 = GetEvent()->GetDataParameters()->GetLabel("NAME2");
 
 // exemple of a 2D histo
 fMy2DHisto = new TH2I ( "My2DHisto","My2DHisto",4048,0,65536,4048,0,65536);
 GetSpectra()->AddSpectrum( fMy2DHisto,"MyFamily");
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
MFMCommonFrame * cf;

cf = (MFMCommonFrame*)(((GEventMFM*)GetEvent())->GetFrame());

if (cf == NULL) {
	fError.TreatError(1,0,"in GUser::User() return null frame in GetEvent())->GetFrame()");
	}else {
  UserFrame(cf);
  }
}
//______________________________________________________________
void GUser::UserMergeFrame(MFMCommonFrame* commonframe){
  int i_insframe=0;
  int nbinsideframe =0; 
  int dumpsize = 16;
  int framesize =0;
  int maxdump =128;

  uint32_t eventnumber ;

   fMergeframe->SetAttributs(commonframe->GetPointHeader());
   eventnumber = fMergeframe->GetEventNumber();

   	nbinsideframe = fMergeframe->GetNbItems();
   	framesize= fMergeframe->GetFrameSize();
   	if (fVerbose>3){
   		fMergeframe->HeaderDisplay();
   	}
   	if (fVerbose>5){
   		if (framesize < maxdump) dumpsize = framesize;
   		else dumpsize = maxdump;
   		fMergeframe->DumpRaw(dumpsize, 0);
   		}
   	fMergeframe->ResetReadInMem();
   	
   	for (i_insframe = 0; i_insframe < nbinsideframe; i_insframe++) {
		fMergeframe->ReadInFrame(fInsideframe);
		 UserFrame(fInsideframe);
	  } 	
	  
// At this point you can do treatement inter frames 

	  	  	
} 

//______________________________________________________________
void GUser::UserFrame(MFMCommonFrame* commonframe){

if (fVerbose >2) cout <<" ------------General Event----------------\n";

 int type =  commonframe->GetFrameType();

 if ((type == MFM_COBO_FRAME_TYPE) or (type== MFM_COBOF_FRAME_TYPE)) {
  	UserCoboFrame(commonframe);
 } 

 if ((type == MFM_MERGE_EN_FRAME_TYPE)or(type == MFM_MERGE_TS_FRAME_TYPE)) {
    	UserMergeFrame(commonframe);	 	
  } 
   
   if (type == MFM_EXO2_FRAME_TYPE){ 
  	UserNumexoFrame(commonframe);
  }
 
  if ((type == MFM_EBY_EN_FRAME_TYPE)||(type == MFM_EBY_TS_FRAME_TYPE)||(type == MFM_EBY_EN_TS_FRAME_TYPE)) {
  	UserEbyFrame(commonframe);
  }
 
}

//______________________________________________________________
void GUser::UserNumexoFrame(MFMCommonFrame* commonframe)
{
// specific User methode for exogam frame
	uint32_t eventnumber;
	uint64_t timestamp;
	int  dumpsize;
	int framesize;
	int maxdump =128;
	fExoframe->SetAttributs(commonframe->GetPointHeader());
	framesize=fExoframe->GetFrameSize();
	fExoframe->HeaderDisplay();
	if (fVerbose>3){
	   		fExoframe->HeaderDisplay();
	   	}
	   	if (fVerbose>5){
	   		if (framesize < maxdump) dumpsize = framesize;
	   		else dumpsize = maxdump;
	   		fExoframe->DumpRaw(dumpsize, 0);
	   		}
	 if (fVerbose==2){
	 	eventnumber =fExoframe->GetEventNumber();
		timestamp = (uint64_t)(fExoframe->GetTimeStamp());
	  	cout << " EN = "<<eventnumber <<" TS = " << timestamp <<endl;
	  	}


}

//______________________________________________________________
void GUser::UserEbyFrame(MFMCommonFrame* commonframe){
// executed in case of Ebyedat Frame
uint32_t eventnumber; 
uint64_t timestamp;
int  dumpsize;  
int framesize;
int maxdump =128;
fEbyframe->SetAttributs(commonframe->GetPointHeader());
framesize=fEbyframe->GetFrameSize();
fEbyframe->HeaderDisplay();
if (fVerbose>3){
   		fEbyframe->HeaderDisplay();
   	}
   	if (fVerbose>5){
   		if (framesize < maxdump) dumpsize = framesize;
   		else dumpsize = maxdump;
   		fEbyframe->DumpRaw(dumpsize, 0);
   		}
 if (fVerbose==2){
 	eventnumber =fEbyframe->GetEventNumber();
	timestamp = (uint64_t)(fEbyframe->GetTimeStamp()); 
  	cout << " EN = "<<eventnumber <<" TS = " << timestamp <<endl;
  	}
	
// exemple of full a 2D histo 
  // UserEbyFrame method for user treatement for each events
  // the event is presented on two ways ( user can use one or other )
  // 1 - event is a vector of a serie of couples  UShor_t/Short_t Label/Value 
  //     
  //      The numbers of couple  Label/Value is given by fEbyframe->GetNbItems()
  //      Each coulple Label/Value is gibent by fEbyframe->EbyedatGetParameters(i, &label, &value);


	
uint32_t value1=0,value2=0;	
	
	
int NbItems;

NbItems= fEbyframe->GetNbItems();
	int i;
	uint16_t label, value;
	for (i = 0; i < NbItems; i++) {
		fEbyframe->EbyedatGetParameters(i, &label, &value);
		if (label==fLabel1) { value1 = value;}
		if (label==fLabel2) { value2 = value;}	
	}	
	
 fMy2DHisto->Fill(value1,value2);	
				
}
//______________________________________________________________

//______________________________________________________________
void GUser::UserCoboFrame(MFMCommonFrame* commonframe){
// executed in case of Ebyedat Frame
uint32_t eventnumber; 
uint64_t timestamp;
int  dumpsize;  
int framesize;
int maxdump =128;
fCoboframe->SetAttributs(commonframe->GetPointHeader());
framesize=fCoboframe->GetFrameSize();
fCoboframe->HeaderDisplay();
if (fVerbose>3){
   		fCoboframe->HeaderDisplay();
   	}
   	if (fVerbose>5){
   		if (framesize < maxdump) dumpsize = framesize;
   		else dumpsize = maxdump;
   		fCoboframe->DumpRaw(dumpsize, 0);
   		}
 if (fVerbose==2){
 	eventnumber =fCoboframe->GetEventNumber();
	timestamp = (uint64_t)(fCoboframe->GetTimeStamp()); 
  	cout << " EN = "<<eventnumber <<" TS = " << timestamp <<endl;
  	}

				
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
  // to have a TTree with only few parameters ( for low compute)
  // to run this method , you have to slect mode 3 in  SetTTreeMode
  // ex : a->SetTTreeMode(3,"/space/MyTTree.root");
   // GetTree() return TTree pointer
  // this can't work with a value created in this class	

  // ex : GetTree()->Branch("mybranche",&(GetEvent()->GetArray()[5]),"mybranche/s");     // to introduce a short in a TTree

}
