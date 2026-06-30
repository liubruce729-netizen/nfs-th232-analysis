#ifndef _MFMXmlDataDescriptionFrame_
#define _MFMXmlDataDescriptionFrame_
/*
  MFMXmlDataDescriptionFrame.h
	Copyright Acquisition group, GANIL Caen, France
*/

#include "MFMBlobFrame.h"
#ifndef NO_MFMXML
#include <tinyxml.h>
#endif 
#include "XmlTags.h"

#pragma pack(push, 1) // force alignment

#define MFM_XML_DATA_DESCRIPTION_FRAME_TYPE_TXT "MFM_XML_DATA_DESCRIPTION_FRAME_TYPE"
#define MFM_XML_DATA_DESCRIPTION_FRAME_TYPE_SHORT_TXT "XmlDa"
#define MFM_XML_DATA_DESCRIPTION_FRAME_TYPE_LONG_TXT "Xml Data Description"
#define MFM_XML_DATA_STD_UNIT_BLOCK_SIZE 1

struct MFM_xml_Data {
};

struct MFM_XmlDataDescription_frame{
	 MFM_common_header  XmlBlobHeader;
	 MFM_xml_Data  XmlData;
};



//____________MFMXmlDataDescriptionFrame___________________________________________________________

class MFMXmlDataDescriptionFrame : public MFMBlobFrame
{

#ifndef NO_MFMXML
TiXmlDocument * MyTinyDoc;
#endif 
public :

MFMXmlDataDescriptionFrame();
MFMXmlDataDescriptionFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
virtual ~MFMXmlDataDescriptionFrame();

 const char * GetTypeText()const {return MFM_XML_DATA_DESCRIPTION_FRAME_TYPE_TXT;} 
 const char * GetTypeShortText()const {return MFM_XML_DATA_DESCRIPTION_FRAME_TYPE_SHORT_TXT;}
 const char * GetTypeLongText()const {return MFM_XML_DATA_DESCRIPTION_FRAME_TYPE_LONG_TXT;} 
 int GetDefinedUnitBlockSize()const {return MFM_XML_DATA_STD_UNIT_BLOCK_SIZE;};

 void SetUserDataPointer();
 void SetAttributs(void * pt =NULL);
 char * GetText()const ;
 string FillExampleOfText();
 void   FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);
 int    InitXml(bool write_or_read, string commentaire);
bool HasTimeStamp()   const { return false; };
bool HasEventNumber() const { return false; };
void dump_to_stdout(const char* pFilename);
#ifndef NO_MFMXML 
void dump_to_stdout( TiXmlNode* pParent, unsigned int indent = 0 );
int dump_attribs_to_stdout(TiXmlElement* pElement, unsigned int indent);
#endif 
const char * getIndentAlt( unsigned int numIndents );
const char * getIndent( unsigned int numIndents );
};
#pragma pack(pop) // free aligment
#endif
