#ifndef _MFMXmlFileHeaderFrame_
#define _MFMXmlFileHeaderFrame_
/*
  MFMXmlFileHeaderFrame.h
	Copyright Acquisition group, GANIL Caen, France
*/

#include "MFMBlobFrame.h"
#ifndef NO_MFMXML
#include <tinyxml.h>
#endif
#pragma pack(push, 1) // force alignment

struct MFM_XmlFile_data {
};
struct MFM_XmlFile_header_frame{
	 MFM_common_header Header;
	 MFM_XmlFile_data  Data;
};

#define MFM_XML_FILE_HEADER_FRAME_TYPE_TEXT "MFM_XML_FILE_HEADER_FRAME_TYPE"
#define MFM_XML_FILE_HEADER_FRAME_TYPE_SHORT_TEXT "XmlHe"
#define MFM_XML_FILE_HEADER_FRAME_TYPE_LONG_TEXT "File Header XML"
#define MFM_XML_FILE_HEADER_STD_UNIT_BLOCK_SIZE 1

//____________MFMXmlFileHeaderFrame___________________________________________________________

class MFMXmlFileHeaderFrame : public MFMBlobFrame
{

public :

MFMXmlFileHeaderFrame();
MFMXmlFileHeaderFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
virtual ~MFMXmlFileHeaderFrame();
void SetAttributs(void * pt =NULL);
void SetUserDataPointer(); 
const char * GetTypeText()const {return MFM_XML_FILE_HEADER_FRAME_TYPE_TEXT;}
const char * GetTypeShortText()const {return MFM_XML_FILE_HEADER_FRAME_TYPE_SHORT_TEXT;}
const char * GetTypeLongText()const {return MFM_XML_FILE_HEADER_FRAME_TYPE_LONG_TEXT;} 
int GetDefinedUnitBlockSize()const {return MFM_XML_FILE_HEADER_STD_UNIT_BLOCK_SIZE;};
 
 char * GetText()const; 
 bool HasTimeStamp()   const { return false; }
 bool HasEventNumber() const { return false; }

void FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);
string FillExampleOfText();

};
#pragma pack(pop) // free aligment
#endif
