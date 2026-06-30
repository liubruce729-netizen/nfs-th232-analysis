/*
 MFMReaGenericFrame.cc
 Copyright Acquisition group, GANIL Caen, France
 */

#include <iostream>
#include <cmath>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
using namespace std;

#include "MFMReaGenericFrame.h"

//_______________________________________________________________________________

MFMReaGenericFrame::MFMReaGenericFrame(){

	fStatus1 = NULL;
	fStatus2 = NULL;
	fTypeTNS = NULL;
	fEnergy  = NULL;
	fTime    = NULL;
	fNumberOfBoards =0;
	fInitTabValuesDone = false;
 }
//_______________________________________________________________________________ 
MFMReaGenericFrame::~MFMReaGenericFrame(){
   
   int card=0;
   if (fInitTabValuesDone==false)return;
   for (card=0;card<fNumberOfBoards;card++){
	if (fStatus1[card]!=NULL)  delete[] fStatus1[card]; 
	if (fStatus2[card]!=NULL)  delete[] fStatus2[card]; 
	if (fTypeTNS[card]!=NULL)  delete[] fTypeTNS[card]; 
	if (fEnergy[card] !=NULL)  delete[] fEnergy[card]; 
	if (fTime[card]   !=NULL)  delete[] fTime[card]; 
   	}
   if (fStatus1!=NULL)  delete[] fStatus1; 
   if (fStatus2!=NULL)  delete[] fStatus2; 
   if (fTypeTNS!=NULL)  delete[] fTypeTNS; 
   if (fEnergy !=NULL)  delete[] fEnergy; 
   if (fTime   !=NULL)  delete[] fTime; 
 }
//_______________________________________________________________________________

void MFMReaGenericFrame::SetStatus(int i, uint16_t status) {
	if (i < 0 and i > REA_GENERIC_NB_STATUS)
		cout << "MFMReaGenericFrame::ReaGenericSetStatus Error of status index\n";
	if (i == 0)
		((MFM_ReaGeneric_frame*) pHeader)->Data.Status1 = status;
	if (i == 1)
		((MFM_ReaGeneric_frame*) pHeader)->Data.Status2 = status;
}
//_______________________________________________________________________________

uint16_t MFMReaGenericFrame::GetStatus(int i)const {
	/// Set Status (0,1 or 2)
	uint16_t status;
	if (i < 0 and i > REA_GENERIC_NB_STATUS) {
		cout << "MFMReaGenericFrame::ExGetStatus Error of status index\n";
		return 0;
	} else {
		if (i == 0)
			status = (((MFM_ReaGeneric_frame*) pHeader)->Data.Status1);
		if (i == 1)
			status = (((MFM_ReaGeneric_frame*) pHeader)->Data.Status2);
	}
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&status);
	return status;
}
//_______________________________________________________________________________

void MFMReaGenericFrame::SetTypeTns(enum ReaTnsType type) {
	/// Set TypeTns
	((MFM_ReaGeneric_frame*) pHeader)->Data.Type_Tns= type;
}
//_______________________________________________________________________________

enum ReaTnsType MFMReaGenericFrame::GetTypeTns()const {
	/// Get TypeTns
	uint16_t  type;
	type = (((MFM_ReaGeneric_frame*) pHeader)->Data.Type_Tns);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&type);
	return (enum ReaTnsType)type;
}
//_______________________________________________________________________________

void MFMReaGenericFrame::SetEnergy(uint16_t energy) {
	/// Set Energy
	((MFM_ReaGeneric_frame*) pHeader)->Data.Energy = energy;
}
//_______________________________________________________________________________

uint16_t MFMReaGenericFrame::GetEnergy() const{
	/// Get Energy
	uint16_t energy;
	energy = (((MFM_ReaGeneric_frame*) pHeader)->Data.Energy);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&energy);
	return energy;
}
//_______________________________________________________________________________

void MFMReaGenericFrame::SetTime(uint16_t time) {
	/// Set Time
	((MFM_ReaGeneric_frame*) pHeader)->Data.Time= time;
}
//_______________________________________________________________________________

uint16_t MFMReaGenericFrame::GetTime() const{
	/// Get Time
	uint16_t time;
	time = (((MFM_ReaGeneric_frame*) pHeader)->Data.Time);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&time);
	return time;
}

//_______________________________________________________________________________
void MFMReaGenericFrame::FillDataWithRamdomValue(uint64_t timestamp,
		uint32_t eventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	MFMNumExoFrame::FillDataWithRamdomValue(timestamp,eventnumber);
	int value = random();
	uint16_t uivalue = (uint16_t) (4294967296 * value / RAND_MAX);
	SetTypeTns(REA_GENERIC_TIME_TYPE);
	SetEnergy(uivalue);
	SetTime(uivalue+1);
}
//_______________________________________________________________________________
void MFMReaGenericFrame::InitStat() {
        MFMNumExoFrame::InitStat();
	if (fInitStatDone == false )return ;	
	int channel;
	for ( channel=0; channel< NUMEXO_NB_CHANNELS ; channel ++){
		fSumEnergy[channel]=0;
		fSumStatus[channel][0]=0;
		fSumStatus[channel][1]=0;
		fSumTypeTNS[channel]=0;
		fSumTime[channel]=0;
		fSumTimeCount[channel]=0;
		fSumEnergyCount[channel]=0;
		fSumStatusCount[channel][0]=0;
		fSumStatusCount[channel][1]=0;
		fSumTypeTNSCount[channel]=0;
		}

}
//_______________________________________________________________________________
void MFMReaGenericFrame::FillStat() {
	if (fInitStatDone == false )return ;
	MFMNumExoFrame::FillStat();
	int channel,board;
	uint32_t eventnumber = GetEventNumber();
	static uint32_t eventnumberold = 0;
	if (eventnumber != 0) {
		if (eventnumber - eventnumberold < 0)
			IncrementNegativJump();
		if (eventnumber - eventnumberold != 1)
			IncrementNoContiJump();
		eventnumberold=eventnumber;
	}
	channel = GetTGCristalId();
  	board   = GetBoardId();
	fSumTime[channel]   += GetTime();
	fSumEnergy[channel] += GetEnergy();
	fSumStatus[channel][0] += GetStatus(0);
	fSumStatus[channel][1] += GetStatus(1);
	fSumTypeTNS[channel]   += GetTypeTns();
	if (fSumTime[channel]!=0) fSumTimeCount[channel] ++;
	if (fSumEnergy[channel]!=0) fSumEnergyCount[channel] ++;
	if (fSumStatus[channel][0]!=0) fSumStatusCount[channel][0]++;
	if (fSumStatus[channel][1]!=0)fSumStatusCount[channel][1]++;
	if (fSumTypeTNS[channel]!=0) fSumTypeTNSCount[channel] ++;
	if (fEnergy)  SetTabValues();

}
//_______________________________________________________________________________
string MFMReaGenericFrame::GetStat(string info)const {
	if (fInitStatDone == false )return(string)"" ;
	string display("");
	stringstream ss("");
	ss << MFMNumExoFrame::GetStat(info);
	int i, j;
	int total = 0;
	int channel;
	
	ss << "Sum of all values of events for each of "<< NUMEXO_NB_CHANNELS <<" channels (if no null): \n";
	for ( channel=0; channel< NUMEXO_NB_CHANNELS ; channel ++){
		if (fSumEnergyCount [channel]>0)  ss << "Count of Energy>0  ["<<channel<<"] = "<< fSumEnergyCount [channel]<<"\n"; 
		if (fSumTimeCount [channel]>0)    ss << "Count of Time>0    ["<<channel<<"] = "<< fSumTimeCount[channel]<<"\n"; 
		if (fSumTypeTNSCount [channel]>0) ss << "Count of TypeTNS>0 ["<<channel<<"] = "<< fSumTypeTNSCount[channel]<<"\n"; 
		if (fSumStatusCount[channel][0]>0)ss << "Count of Status0>0 ["<<channel<<"] = "<< fSumStatusCount[channel][0]<<"\n"; 
		if (fSumStatusCount[channel][1])  ss << "Count of Status1>0 ["<<channel<<"] = "<< fSumStatusCount[channel][1]<<"\n"; 
		}
	display = ss.str();
	return display;
}

//_______________________________________________________________________________
void  MFMReaGenericFrame::InitTabValues(uint16_t *listofboards,int numberofboards){
  // method to init tab  for analyse and use to fill TTree 
  
  int board =0, channel=0;
  fNumberOfBoards = numberofboards;
  fInitTabValuesDone = true;
  for(board =0; board< NUMEXO_MAX_NUMB_BOARDS ;board ++) {
  	fConvertNoBoardIndex[board] = 0;
  	fConvertIndexNoBoard[board] = 0;
  	}
  if (fEnergy  == NULL) fEnergy  = new uint16_t*[fNumberOfBoards];
  if (fTime    == NULL) fTime    = new uint16_t*[fNumberOfBoards]; 
  if (fTypeTNS == NULL) fTypeTNS = new uint16_t*[fNumberOfBoards];
  
  if (fStatus1 == NULL) fStatus1 = new uint16_t*[fNumberOfBoards];
  if (fStatus2 == NULL) fStatus2 = new uint16_t*[fNumberOfBoards]; 
  for (board=0 ; board< numberofboards; board++){	
  	fConvertNoBoardIndex[listofboards[board]] = board ;
  	fConvertIndexNoBoard[board] = listofboards[board] ;
  	fEnergy[board]  = new uint16_t[NUMEXO_NB_CHANNELS];
	fTime[board]    = new uint16_t[NUMEXO_NB_CHANNELS];
	fTypeTNS[board] = new uint16_t[NUMEXO_NB_CHANNELS];
	fStatus1[board] = new uint16_t[NUMEXO_NB_CHANNELS];
	fStatus2[board] = new uint16_t[NUMEXO_NB_CHANNELS]; 
		
	}
   ResetTabValues();
}
//_______________________________________________________________________________
void   MFMReaGenericFrame::ResetTabValues(){ 	

 if (fInitTabValuesDone==false)return;

int board =0, channel=0;
  fTabValueEventNumber=0;
  fTabValueTimeStamp=0;
  fChannel =0;
  fBoardNumber=0;
  fBoardIndex=0;

  for (board=0 ; board< fNumberOfBoards; board++){	
	for (channel=0 ; channel< NUMEXO_NB_CHANNELS; channel++){
		fEnergy[board][channel]   = 0;
		fTime[board][channel] 	  = 0;    
		fTypeTNS[board][channel]  = 0; 
		fStatus1[board][channel]  = 0;
		fStatus2[board][channel]  = 0;
	}
  }
}
//_______________________________________________________________________________
void   MFMReaGenericFrame::SetTabValues(){ 
	// method to set values of tabs  for analyse and use to fill TTree 
	ResetTabValues();
	fChannel       = GetTGCristalId();
  	fBoardNumber   = GetBoardId();
  	fTabValueEventNumber = GetEventNumber();
  	fTabValueTimeStamp   = GetTimeStamp();
  	fBoardIndex          = fConvertNoBoardIndex[fBoardNumber];
 if (fInitTabValuesDone==false)return;  	
  	if ((fNumberOfBoards>fBoardIndex)and ( NUMEXO_NB_CHANNELS >fChannel)){
  		fEnergy[fBoardIndex][fChannel]  = GetEnergy();
		fTime[fBoardIndex][fChannel]    = GetTime();
		fTypeTNS[fBoardIndex][fChannel] = GetTypeTns();
		fStatus1[fBoardIndex][fChannel] = GetStatus(0);
		fStatus2[fBoardIndex][fChannel] = GetStatus(1);
	} else { 
	   cout << "Warning , index of  card or No Channel to big \n";
	   cout <<dec <<" BoardIndex   = "<<fBoardIndex << "  <->   NumberOfBoards = "<<fNumberOfBoards<<"\n";
  	   cout       <<" ChannelIndex = "<<fChannel << "  <->  NUMEXO_NB_CHANNELS = "<<NUMEXO_NB_CHANNELS<<"\n";
	}		
}
//_______________________________________________________________________________	

string MFMReaGenericFrame::GetDumpData(char mode, bool nozero) const {
	// Dump parameter Label and parameter value of the current event.
	// if enter parameter is true (default value), all zero parameter of event aren't dumped
	// mode = 'd' for decimal, 'b' for binary, 'h' for hexa, 'o' for octal

	stringstream ss;
	string display("");

	int i, j, maxbin, presentation = 0, max_presentation = 5;
	char tempo[255];
	char Bin[255];
	char Bin2[255];
	int reste;
	char one = '1', zero = '0';
	int DecNumber = 0;

	if (mode == 'b')
		max_presentation = 3;
	if (GetEventNumber() == 0xFFFFFFFF) {
		ss << "No Event , so no event dump. Get a new event frame";
	} else {
		ss << "  E = "<<GetEnergy()<<"  T = "<< GetTime() ;
	}
	ss << endl;
	display = ss.str();
	return display;
}
//_______________________________________________________________________________
bool MFMReaGenericFrame::IsSame(MFMReaGenericFrame* testframe,int verbose){
// test if testframe have same attibut values

string display("");
stringstream ss("");
bool test= true;
bool testloc= true;
int i=0;  

ss << "   Test for "<< MyClassName()<<" : " ;
testframe->MFMReaGenericFrame::SetAttributs();
MFMReaGenericFrame::SetAttributs();

test= MFMNumExoFrame::IsSame((MFMNumExoFrame*)testframe,verbose);

testloc =  ( GetStatus(0)       == testframe->GetStatus(0));
ss << testloc ; test =  test and testloc;
testloc =  ( GetStatus(1)       == testframe->GetStatus(1));
ss << testloc ; test =  test and testloc;
testloc =  ( GetTypeTns()      == testframe->GetTypeTns());
ss << testloc ; test =  test and testloc;
testloc =  ( GetEnergy()      == testframe->GetEnergy());
ss << testloc ; test =  test and testloc;
testloc =  ( GetTime()        == testframe->GetTime());
ss << testloc ; test =  test and testloc;
testloc =  ( GetChecksum()== testframe->GetChecksum());
ss << testloc ; test =  test and testloc;
display = ss.str();
if ((verbose>0))
   cout << display <<"  (St1 St2 Typ Ener Time Check)" <<endl;
return test;
}
//_______________________________________________________________________________
bool MFMReaGenericFrame::UnitTest(int verbose){
// Test this class
// this methode generate a frame write it in a tmpfile 
// read this frame with a other object frame and after comare the both frame
// return true if OK , false if noOK		

int lun; // Logical Unit Number
UtilVector_c* vector = new UtilVector_c(MFM_BLOB_HEADER_SIZE); // min size =8 

bool test =true;

char  tmpfilename[256];
sprintf ( tmpfilename,"test%s.tmp",MyClassName());
int type = MFM_REA_GENE_FRAME_TYPE;
int dump =0;
if (verbose>9)dump =64;
//generation and write 
     MFMReaGenericFrame * framewrite = new MFMReaGenericFrame();    
     lun = open(tmpfilename, (O_RDWR | O_CREAT | O_TRUNC), 0644);
     framewrite->WriteRandomFrame(lun,1, 0, 0,MFM_REA_GENE_FRAME_TYPE);
     close(lun);  

//read
     lun = open(tmpfilename, (O_RDONLY));  
     MFMReaGenericFrame * frameread = new MFMReaGenericFrame();    
     frameread->ReadInFile(&lun, vector);
//compare
     cout << ">--Test begin for "<<frameread->MyClassName()<<"--------"<<endl;
     test=framewrite->IsSame(frameread,verbose);
     cout << ">--Result for "<<frameread->MyClassName()<<" = "<< test<<endl;
     if (verbose >5) {
    	framewrite->ReadAttributsExtractFrame(verbose,dump, true,0,framewrite->GetPointHeader());
    	frameread->ReadAttributsExtractFrame(verbose,dump, true,0,frameread->GetPointHeader());
     	}

//end
     delete (framewrite);
     delete (frameread);
     if(vector) delete(vector);
     close(lun);
     return test;
}
//_______________________________________________________________________________
//_______________________________________________________________________________

