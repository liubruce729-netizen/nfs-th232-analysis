#ifndef _MFMChimeraFrame_
#define _MFMChimeraFrame_
/*
  MFMChimeraFrame.h
	Copyright Acquisition group, GANIL Caen, France
	e.d.f.  2016  (test class for chimera daq) 
*/

#define CHI_HEADERFRAMESIZE 20
#define CHI_USERSIZE   28   // fully ramdom value, just for write test
#define CHI_FRAMESIZE  CHI_HEADERFRAMESIZE + CHI_USERSIZE



#define MFM_CHIMERA_DATA_FRAME_TYPE_TXT "MFM_CHIMERA_DATA_FRAME_TYPE"
#define MFM_CHIMERA_DATA_FRAME_TYPE_SHORT_TXT "Chime"
#define MFM_CHIMERA_DATA_FRAME_TYPE_LONG_TXT "Chimera"
#define MFM_CHIMERA_DATA_STD_UNIT_BLOCK_SIZE 1
#include "MFMBlobFrame.h"

#pragma pack(push, 1) // force alignment
struct MFM_CHI_eventInfo {
  char EventTime[6];
  unsigned EventIdx  : 32;
  unsigned reserved : 16; 
};

struct MFM_CHI_data{
	char test[CHI_USERSIZE];
};

struct MFM_CHI_frame{
	 MFM_common_header  CHIBlobcHeader;
	 MFM_CHI_eventInfo  CHIEventInfo;
 	 MFM_CHI_data       CHIData;
};

//____________MFMChimeraFrame___________________________________________________________

class MFMChimeraFrame : public MFMBlobFrame
{

public :

MFMChimeraFrame();
MFMChimeraFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
virtual ~MFMChimeraFrame();

 void SetAttributs(void * pt =NULL);
 void SetTimeStampFromChimeraFrameData();
 void SetEventNumberFromChimeraFrameData();
 void SetUserDataPointer();
 
 const char * GetTypeText()const {return MFM_CHIMERA_DATA_FRAME_TYPE_TXT;} 
 const char * GetTypeShortText()const {return MFM_CHIMERA_DATA_FRAME_TYPE_SHORT_TXT;}
  const char * GetTypeLongText()const {return MFM_CHIMERA_DATA_FRAME_TYPE_LONG_TXT;}
 int GetDefinedUnitBlockSize()const {return MFM_CHIMERA_DATA_STD_UNIT_BLOCK_SIZE;};
  int GetDefinedHeaderSize()const{return CHI_HEADERFRAMESIZE;};
 
 //int GetDefinedFrameSize()const {return CHI_FRAMESIZE;};
 int GetDefinedFrameSize()const {return sizeof (MFM_CHI_frame);}; 
 void SetTimeStamp(uint64_t timestamp);
 void SetEventNumber(uint32_t eventnumber);

 void FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);

};
#pragma pack(pop) // free aligment
#endif
