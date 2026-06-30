#ifndef _MFMHelloFrame_
#define _MFMHelloFrame_
/*
  MFMHelloFrame.h
	Copyright Acquisition group, GANIL Caen, France
*/
#define HEL_USER_SIZE 21
#define HEL_HEADERFRAMESIZE 18
#define HEL_FRAMESIZE   HEL_HEADERFRAMESIZE+HEL_USER_SIZE// fully random value , just for test
#define HEL_STD_UNIT_BLOCK_SIZE 1
#define MFM_HELLO_FRAME_TYPE_TXT "MFM_HELLO_FRAME_TYPE"
#define MFM_HELLO_FRAME_TYPE_SHORT_TXT "Hello"
#define MFM_HELLO_FRAME_TYPE_LONG_TXT "Hello"
#define NB_CHAR_IN_MFM_HELLO_FRAME_TYPE_TEXT 20

#include "MFMBlobFrame.h"

#pragma pack(push, 1) // force alignment
struct MFM_hel_eventInfo {
  char EventTime[6];
  unsigned EventIdx  : 32;
};

struct MFM_hel_data{
};

struct MFM_hel_header{
	 MFM_common_header  HelBlobcHeader;
	 MFM_hel_eventInfo  HelEventInfo;
	 MFM_hel_data       HelData;
};

//____________MFMHelloFrame___________________________________________________________

class MFMHelloFrame : public MFMBlobFrame
{
long long fCountTest;
public :

MFMHelloFrame();
MFMHelloFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
virtual ~MFMHelloFrame();
void SetUserDataPointer() ;
void SetPointers(void * pt =NULL);
//void SetAttributs(void * pt =NULL);
void SetTimeStampFromHelloFrameData();
void SetEventNumberFromHelloFrameData();
virtual void SetTimeStamp(uint64_t timestamp);
virtual void SetEventNumber(uint32_t eventnumber);
virtual string GetHeaderDisplay(char* infotext=NULL) const;
void FillDataWithRamdomValue(  uint64_t timestamp, uint32_t enventnumber);
 const char * GetTypeText()const {return MFM_HELLO_FRAME_TYPE_TXT;} 
 const char * GetTypeShortText()const {return MFM_HELLO_FRAME_TYPE_SHORT_TXT;}
 const char * GetTypeLongText()const {return MFM_HELLO_FRAME_TYPE_LONG_TXT;} 
 int GetDefinedUnitBlockSize()const {return HEL_STD_UNIT_BLOCK_SIZE;};
 int GetDefinedHeaderSize()const {return HEL_HEADERFRAMESIZE;};
 int GetDefinedFrameSize()const {return HEL_FRAMESIZE;};
 void PutData(void* pt , int size);
 void InitStat();
 void FillStat();
 string GetStat(string info)const;
};
#pragma pack(pop) // free aligment
#endif
