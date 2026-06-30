/*
  MFMParisFrame.cc
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

#include "MFMParisFrame.h"

//_______________________________________________________________________________
MFMParisFrame::MFMParisFrame(int unitBlock_size, int dataSource,
			     int frameType, int revision, int frameSize, int headerSize) {
  /// Constructor for a PARIS frame . the header is filled with unitblock_size, data source , frame type , revision , frame, size and header size value
  SetPointers();
}

//_______________________________________________________________________________
MFMParisFrame::MFMParisFrame() {
  /// Constructor for a empty PARIS frame

}
//_______________________________________________________________________________
MFMParisFrame::~MFMParisFrame() {
  /// destructor of PARIS frame
}

//_______________________________________________________________________________
void MFMParisFrame::SetQShort(uint16_t energy) {
  /// Set Energy
  (((MFM_paris_frame*) pHeader)->Data.QShort) = energy;
}

//_______________________________________________________________________________
uint16_t MFMParisFrame::GetQShort()const {
  /// GetEnergy
  uint16_t energy;
  energy=(((MFM_paris_frame*) pHeader)->Data.QShort);
  if (fLocalIsBigEndian != fFrameIsBigEndian)
    SwapInt16(&energy);
  return energy;
}

//_______________________________________________________________________________
void MFMParisFrame::SetQLong(uint16_t energy) {
  /// Set Energy
  (((MFM_paris_frame*) pHeader)->Data.QLong) = energy;
}

//_______________________________________________________________________________
uint16_t MFMParisFrame::GetQLong() const{
  /// GetEnergy
  uint16_t energy;
  energy=(((MFM_paris_frame*) pHeader)->Data.QLong);
  if (fLocalIsBigEndian != fFrameIsBigEndian)
    SwapInt16(&energy);
  return energy;
}

//_______________________________________________________________________________

void MFMParisFrame::SetCfd(float cfd) {
  /// Set Top in frame
  (((MFM_paris_frame*) pHeader)->Data.Cfd) = cfd;
}
//_______________________________________________________________________________

float MFMParisFrame::GetCfd()const {
  /// computer and return  Top value from frame
  float cfd;
  cfd = ((((MFM_paris_frame*) pHeader)->Data.Cfd) + random()/RAND_MAX -0.5 )/ 1000.;

  return cfd;
}


//_______________________________________________________________________________
void MFMParisFrame::FillDataWithRamdomValue(uint64_t timestamp,
					 uint32_t eventnumber) {

  /// Fill all data of frame with random values to do test
  /// And report time stamp and event number
  float maxuint16 = pow(2,16);
  float maxuint32 = pow(2,32);
  float value = random();

  uint16_t uivalue16 = (uint16_t) (maxuint16 * (float)(value / RAND_MAX));
  uint32_t uivalue32 = (uint32_t) (maxuint32 * (float)(value / RAND_MAX));


  SetQShort(uivalue16);
  SetQLong (uivalue16);
  SetCfd   (uivalue32 +0.5);

  SetEventNumber(eventnumber);
  SetTimeStamp(timestamp);
  MFMNumExoFrame::FillDataWithRamdomValue(timestamp,eventnumber);
  SetFrameType (MFM_PARIS_FRAME_TYPE);
}

//_______________________________________________________________________________

string MFMParisFrame::GetHeaderDisplay(char* infotext)const{

   stringstream ss;
   string display("");
   display = ss.str(); 
   ss << MFMCommonFrame::GetHeaderDisplay(infotext) ;
   ss << "   Channel = " << GetTGCristalId(); 
   ss << std::endl;
   ss << " qShort = " << GetQShort() ;
   ss << " qLong = " << GetQLong() << " CFD = " << GetCfd() << " [ns]";
   ss << std::endl;
   ss << " Flags = " << std::hex << ((MFM_paris_frame*) pHeader)->Data.Flags << std::dec ; 
   ss << "   PLL unlock " << ((GetPLL()) ? "true" : "false");
   ss << "   PUR " << ((GetPUR()) ? "true" : "false");
   ss << "   OVR " << ((GetOVR()) ? "true" : "false");
  
   display = ss.str();
   return display; 
}

//_______________________________________________________________________________
bool MFMParisFrame::GetPLL()const {
  return ((((MFM_paris_frame*) pHeader)->Data.Flags)&0x20)>>5;
}
//_______________________________________________________________________________
bool MFMParisFrame::GetPUR()const{
  return ((((MFM_paris_frame*) pHeader)->Data.Flags)&0x10)>>4;
}
//_______________________________________________________________________________
bool MFMParisFrame::GetOVR()const{
  return ((((MFM_paris_frame*) pHeader)->Data.Flags)&0x8)>>3;
}
//_______________________________________________________________________________
bool MFMParisFrame::IsSame(MFMParisFrame* testframe,int verbose){
// test if testframe have same attibut values

string display("");
stringstream ss("");
bool test= true;
bool testloc= true;
int i=0;  

ss << "   Test for "<< MyClassName()<<" : " ;
testframe->MFMParisFrame::SetAttributs();
MFMParisFrame::SetAttributs();

test= MFMNumExoFrame::IsSame((MFMNumExoFrame*)testframe,verbose);

testloc =  ( GetQShort()== testframe->GetQShort());
ss << testloc ; test = test and testloc;
testloc =  ( GetQLong() == testframe->GetQLong());
ss << testloc ; test =  test and testloc;
testloc =  ( GetCfd()   == testframe->GetCfd());
ss << testloc ; test =  test and testloc;
testloc =  ( GetPLL()   == testframe->GetPLL());
ss << testloc ; test = test and testloc;
testloc =  ( GetPUR()   == testframe->GetPUR());
ss << testloc ; test = test and testloc;
testloc =  ( GetOVR()  == testframe->GetOVR());
ss << testloc ; test = test and testloc;
display = ss.str();
if ((verbose>0))
   cout << display <<"  (QS QL CF PLL PUR OVR)" <<endl;
return test;
}
//_______________________________________________________________________________
bool MFMParisFrame::UnitTest(int verbose){
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
     MFMParisFrame * framewrite = new MFMParisFrame();    
     lun = open(tmpfilename, (O_RDWR | O_CREAT | O_TRUNC), 0644);
     framewrite->WriteRandomFrame(lun,1, 0, 0,MFM_REA_GENE_FRAME_TYPE);
     close(lun);  

//read
     lun = open(tmpfilename, (O_RDONLY));  
     MFMParisFrame * frameread = new MFMParisFrame();    
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

