#ifndef _MFMS3SynchroFrame_
#define _MFMS3SynchroFrame_
/*
  MFMS3SynchroFrame.h

	Copyright Acquisition group, GANIL Caen, France
	Class Synchro frame based on Numexo2 card frame.
    Author : Luc Legeard
*/

#include "MFMNumExoFrame.h"


#pragma pack(push, 1) // force alignment

#define MFM_S3_SYNC_FRAME_TYPE_TXT "MFM_S3_SYNC_FRAME_TYPE"
#define MFM_S3_SYNC_FRAME_TYPE_SHORT_TXT "S3Syn"
#define MFM_S3_SYNC_FRAME_TYPE_LONG_TXT "S3 Synchro"
struct MFM_syncho_data{
  uint16_t CristalId ;
  uint16_t Nothing1  ;
  uint16_t Nothing2  ;
  char     Period[6];
  uint16_t Checksum ;
};

struct MFM_syncho_frame{
	 MFM_common_header  Header;
	 MFM_numexo_eventInfo  EventInfo;
	 MFM_syncho_data       Data;
};

//____________MFMS3SynchroFrame___________________________________________________________

class MFMS3SynchroFrame:public MFMNumExoFrame
{
public:

MFMS3SynchroFrame();
MFMS3SynchroFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
virtual ~MFMS3SynchroFrame();

 void      SetPeriod(uint64_t period);
 uint64_t  GetPeriod();

void      FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);
const char * GetTypeText()const {return MFM_S3_SYNC_FRAME_TYPE_TXT;};
const char * GetTypeShortText()const {return MFM_S3_SYNC_FRAME_TYPE_SHORT_TXT;};
const char * GetTypeLongText()const {return MFM_S3_SYNC_FRAME_TYPE_LONG_TXT;};
};
#pragma pack(pop) // free aligment
#endif

