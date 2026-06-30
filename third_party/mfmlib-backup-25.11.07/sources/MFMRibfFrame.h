#ifndef _MFMRibfFrame_
#define _MFMRibfFrame_
/*
  MFMRibfFrame.h

	Copyright Acquisition group, GANIL Caen, France

*/
#include "MFMBlobFrame.h"

#define RIBF_HEADERFRAMESIZE 18
#define RIBF_STD_UNIT_BLOCK_SIZE 1
#define MFM_RIBF_FRAME_TYPE_TXT "MFM_RIBF_FRAME_TYPE"
#define MFM_RIBF_FRAME_TYPE_SHORT_TXT "Ribf "
#define MFM_RIBF_FRAME_TYPE_LONG_TXT "Ribf "

#define RIBF_USERSIZE 20   // fully ramdom value, just for write test
#define RIBF_FRAMESIZE RIBF_HEADERFRAMESIZE + RIBF_USERSIZE

#pragma pack(push, 1) // force alignment

struct MFM_Ribf_eventInfo {
  char EventTime[6];
  unsigned EventIdx  : 32;
};

struct MFM_Ribf_data{
char test[RIBF_USERSIZE];
};

struct MFM_Ribf_header{
	 MFM_common_header   BlobcHeader;
	 MFM_Ribf_eventInfo  EventInfo;
	 MFM_Ribf_data       Data;
};


//____________MFMRibfFrame___________________________________________________________

class MFMRibfFrame : public MFMBlobFrame
{

public :

MFMRibfFrame();
MFMRibfFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
 ~MFMRibfFrame();
 void SetUserDataPointer();
 //void SetAttributs(void * pt =NULL);

 const char * GetTypeText()const {return MFM_RIBF_FRAME_TYPE_TXT;} 
 const char * GetTypeShortText()const {return MFM_RIBF_FRAME_TYPE_SHORT_TXT;}
 const char * GetTypeLongText()const {return MFM_RIBF_FRAME_TYPE_LONG_TXT;}
 int GetDefinedUnitBlockSize()const {return RIBF_STD_UNIT_BLOCK_SIZE;};
 int GetDefinedHeaderSize()const {return RIBF_HEADERFRAMESIZE;};
 int GetDefinedFrameSize()const {return RIBF_FRAMESIZE;};

void SetTimeStampFromRibfFrameData();
void SetEventNumberFromRibfFrameData();
	
 void SetTimeStamp(uint64_t timestamp);
 void SetEventNumber(uint32_t eventnumber);
 string GetHeaderDisplay(char* infotext=NULL)const ;
 void FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);

};
#pragma pack(pop) // free aligment
#endif
