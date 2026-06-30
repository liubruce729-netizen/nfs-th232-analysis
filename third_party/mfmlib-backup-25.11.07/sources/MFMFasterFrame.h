#ifndef _MFMFasterFrame_
#define _MFMFasterFrame_
/*
 
  MFMFasterFrame.h
  Copyright Acquisition group, GANIL Caen, France

*/

#include "MFMBlobFrame.h"

#define FASTER_HEADERFRAMESIZE 18 
#define FASTER_STD_UNIT_BLOCK_SIZE 1
#define MFM_FASTER_FRAME_TYPE_TXT "MFM_FASTER_FRAME_TYPE"
#define MFM_FASTER_FRAME_TYPE_SHORT_TXT "Faste"
#define MFM_FASTER_FRAME_TYPE_LONG_TXT  "FASTER"

#define FASTER_USERSIZE   20   // fully ramdom value, just for write test
#define FASTER_FRAMESIZE  FASTER_HEADERFRAMESIZE + FASTER_USERSIZE

#pragma pack(push, 1)       // force alignment

struct MFM_Faster_eventInfo {
  char EventTime[6];
  unsigned EventIdx  : 32;
};

struct MFM_Faster_data{
char test[FASTER_USERSIZE];
};

struct MFM_Faster_frame{
	 MFM_common_header   BlobcHeader;
	 MFM_Faster_eventInfo  EventInfo;
	 MFM_Faster_data       Data;
};


//____________MFMFasterFrame___________________________________________________________

class MFMFasterFrame : public MFMBlobFrame
{

public :

MFMFasterFrame();
MFMFasterFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
 ~MFMFasterFrame();
 void SetUserDataPointer();
 //void SetAttributs(void * pt =NULL);

 const char * GetTypeText()const {return MFM_FASTER_FRAME_TYPE_TXT;} 
 const char * GetTypeShortText()const {return MFM_FASTER_FRAME_TYPE_SHORT_TXT;}
 const char * GetTypeLongText()const {return MFM_FASTER_FRAME_TYPE_LONG_TXT;}
 int          GetDefinedUnitBlockSize()const {return FASTER_STD_UNIT_BLOCK_SIZE;};
 int          GetDefinedHeaderSize()const {return FASTER_HEADERFRAMESIZE;};
 int          GetDefinedFrameSize()const {return FASTER_FRAMESIZE;};
 void         SetTimeStampFromFasterFrameData();
 void         SetEventNumberFromFasterFrameData();
	
 void         SetTimeStamp(uint64_t timestamp);
 void         SetEventNumber(uint32_t eventnumber);
 string       GetHeaderDisplay(char* infotext=NULL)const ;
 void         FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);

};
#pragma pack(pop) // free aligment
#endif
