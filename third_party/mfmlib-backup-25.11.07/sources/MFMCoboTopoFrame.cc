/*
 MFMCoboTopoFrame.cc
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

#include "MFMCoboTopoFrame.h"
#include "XmlTags.h"
//_______________________________________________________________________________
MFMCoboTopoFrame::MFMCoboTopoFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, int headerSize) {
	/// Constructor for a exogam frame . the header is filled with unitblock_size, data source , frame type , revision , frame, size and header size value
	SetPointers();
}
//_______________________________________________________________________________
MFMCoboTopoFrame::MFMCoboTopoFrame() {
	/// Constructor for a empty exogam frame
}
//_______________________________________________________________________________
MFMCoboTopoFrame::~MFMCoboTopoFrame() {
/// destructor of Exogam frame
}
//_______________________________________________________________________________
void MFMCoboTopoFrame::SetUserDataPointer(){
pUserData_char = (char*) &(((MFM_CoboTopo_frame*) pHeader)->CoboTopoData);
}
//_______________________________________________________________________________
void MFMCoboTopoFrame::SetAttributs(void * pt) {
	SetPointers(pt);
	MFMBlobFrame::SetAttributs(pt);
}
//_______________________________________________________________________________
uint16_t MFMCoboTopoFrame::CoboGetIdx() const {
	return ((int16_t) (((MFM_CoboTopo_frame*) pHeader)->CoboTopoData.CoboIdx));
}
//_______________________________________________________________________________
void MFMCoboTopoFrame::CoboSetIdx(uint16_t idx){
	((MFM_CoboTopo_frame*) pHeader)->CoboTopoData.CoboIdx = idx;
}
//_______________________________________________________________________________
uint16_t MFMCoboTopoFrame::CoboGetAsadMask() const {
	return ((int16_t) (((MFM_CoboTopo_frame*) pHeader)->CoboTopoData.AsadMask));
}
//_______________________________________________________________________________
void MFMCoboTopoFrame::CoboSetAsadMask(uint16_t asadmask){
	((MFM_CoboTopo_frame*) pHeader)->CoboTopoData.AsadMask = asadmask;
}
//_______________________________________________________________________________
uint16_t MFMCoboTopoFrame::CoboGetTpMode() const {
	return ((int16_t) (((MFM_CoboTopo_frame*) pHeader)->CoboTopoData.AsadMask));
}
//_______________________________________________________________________________
void MFMCoboTopoFrame::CoboSetTpMode(uint16_t mode) {
	((MFM_CoboTopo_frame*) pHeader)->CoboTopoData.AsadMask = mode;
}
//______________________________________________________________________________
void MFMCoboTopoFrame::FillDataWithRamdomValue(  uint64_t timestamp, uint32_t enventnumber) {
// fill data  with random data
        

	int static increment =0;
	SetEventNumber(enventnumber);
	SetTimeStamp(timestamp);
	
	CoboSetIdx(increment++%16);
	CoboSetAsadMask(increment++%16);
	CoboSetTpMode(increment++%16);

}

//_______________________________________________________________________________
string MFMCoboTopoFrame::GetHeaderDisplay(char* infotext) const{
	stringstream ss;
	string display("");
	display = ss.str();
	ss << MFMCommonFrame::GetHeaderDisplay(infotext);	
	ss << "  CoboIdx = " << CoboGetIdx();
	ss << " CoboAsaIdxMask = " << CoboGetAsadMask();
	ss << " CoboTpMode = " << CoboGetTpMode();
	display = ss.str();
	return display;
}
//_______________________________________________________________________________



