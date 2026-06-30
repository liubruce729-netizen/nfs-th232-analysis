/*
 MFMXmlFileHeaderFrame.cc
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

#include "MFMXmlFileHeaderFrame.h"

//_______________________________________________________________________________
MFMXmlFileHeaderFrame::MFMXmlFileHeaderFrame(int unitBlock_size,
		int dataSource, int frameType, int revision, int frameSize,
		int headerSize) {
	/// Constructor for a exogam frame . the header is filled with unitblock_size, data source , frame type , revision , frame, size and header size value
	SetPointers();
}

//_______________________________________________________________________________
MFMXmlFileHeaderFrame::MFMXmlFileHeaderFrame() {
	/// Constructor for a empty exogam frame

}
//_______________________________________________________________________________
MFMXmlFileHeaderFrame::~MFMXmlFileHeaderFrame() {
	/// destructor of Exogam frame
}
//_______________________________________________________________________________
void MFMXmlFileHeaderFrame::SetUserDataPointer(){
pUserData_char = (char*) &(((MFM_XmlFile_header_frame*) pHeader)->Data);
}
//_______________________________________________________________________________
void MFMXmlFileHeaderFrame::SetAttributs(void * pt) {
	SetPointers(pt);
	MFMBlobFrame::SetAttributs(pt);
}

//_______________________________________________________________________________
char*  MFMXmlFileHeaderFrame::GetText()const{
	return GetPointUserData();
}
//_______________________________________________________________________________
void MFMXmlFileHeaderFrame::FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber) {
	// fill frame with example given in FillExampleOfText()
	string text;
	text =  FillExampleOfText() ;
	int size = text.size();
	int framesize = size +1+MFM_BLOB_HEADER_SIZE ;
	SetBufferSize(framesize );
	SetUserDataPointer();
	strcpy(GetText(),(const char*)(text.data()));
	SetFrameSize(framesize); 
}
//_______________________________________________________________________________
string MFMXmlFileHeaderFrame::FillExampleOfText() {

	/// Fill text fied wiht a example
	string text = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	text += "<File_Header>";
	text += "<data_format>mfm</data_format>";
	text += "<experiment>";
	text += "<name>test_laser</name>";
	text += "<laboratory>Ganil</laboratory>";
	text += "</experiment>";
	text += "<context>";
	text += "<run_number>0042</run_number>";
	text += "<run_start_time>16-09-14_17h41m54s</run_start_time>";
	text += "<index>1</index>";
	text += "<file_name>run_0042.dat.16-09-14_17h41m54s.1</file_name>";
	text += "<file_creation_time>16-09-14_17h42m56s</file_creation_time>";
	text += "</context>";
	text += "<software>";
	text += "<type>Narval</type>";
	text += "<application>gnarval_mfm_storer</application>";
	text += "<version>v14.10-03</version>";
	text += "</software>";
	text += "<user_comment>comment</user_comment>";
	text += "</File_Header>";
return text;
}

//_______________________________________________________________________________
