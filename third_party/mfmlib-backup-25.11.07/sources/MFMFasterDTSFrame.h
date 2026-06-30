#ifndef _MFMFASTERDTSFrame_
#define _MFMFASTERDTSFrame_
/*
 
  MFMFasterDTSFrame.h
  Copyright Acquisition group, GANIL Caen, France

*/

#include "MFMBlobFrame.h"

#define FASTERDTS_HEADERFRAMESIZE 18 
#define FASTERDTS_STD_UNIT_BLOCK_SIZE 1
#define MFM_FASTERDTS_FRAME_TYPE_TXT "MFM_FASTERDTS_FRAME_TYPE"
#define MFM_FASTERDTS_FRAME_TYPE_SHORT_TXT "FasDTS"
#define MFM_FASTERDTS_FRAME_TYPE_LONG_TXT  "FASTERDTS"

#define FASTERDTS_USERSIZE   30
#define FASTERDTS_FRAMESIZE  FASTERDTS_HEADERFRAMESIZE + FASTERDTS_USERSIZE

#pragma pack(push, 1)       // force alignment

struct MFM_FasterDTS_eventInfo {
  char     Timestamp[6];        // TS Faster
  unsigned EventIdx  : 32;      // n° event 
};

struct MFM_FasterDTS_data{

char     ExternTimestamp[6]; // TS fourni par SMart ds notre cas
uint16_t TimeBase;           // Tic Faster
uint16_t ExternTimeBase;     // Tic extern ( Smart dans notre cas)
int64_t  DeltaT;             // Difference entre 2 TS normalisé en nanoseconde 
uint16_t IdOfExternSystem;   // N° Id Subsystem
char     Reserved[8];        // future and padding 
uint16_t Checksum;           // checksum
};

struct MFM_FasterDTS_frame{
	 MFM_common_header   BlobcHeader;
	 MFM_FasterDTS_eventInfo  EventInfo;
	 MFM_FasterDTS_data       Data;
};


//____________MFMFasterDTSFrame___________________________________________________________

class MFMFasterDTSFrame : public MFMBlobFrame
{

public :

MFMFasterDTSFrame();
MFMFasterDTSFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
 ~MFMFasterDTSFrame();
 void SetUserDataPointer();
 //void SetAttributs(void * pt =NULL);

 const char * GetTypeText()const {return MFM_FASTERDTS_FRAME_TYPE_TXT;} 
 const char * GetTypeShortText()const {return MFM_FASTERDTS_FRAME_TYPE_SHORT_TXT;}
 const char * GetTypeLongText()const {return MFM_FASTERDTS_FRAME_TYPE_LONG_TXT;}
 int          GetDefinedUnitBlockSize()const {return FASTERDTS_STD_UNIT_BLOCK_SIZE;};
 int          GetDefinedHeaderSize()const {return FASTERDTS_HEADERFRAMESIZE;};
 int          GetDefinedFrameSize()const {return FASTERDTS_FRAMESIZE;};
 void         SetTimeStampFromFasterDTSFrameData();
 void         SetEventNumberFromFasterDTSFrameData();
	
 void         SetTimeStamp(uint64_t timestamp);
 void         SetExternTimeStamp(uint64_t timestamp);
 void         SetEventNumber(uint32_t eventnumber);
 
 string       GetHeaderDisplay(char* infotext=NULL)const ;
 void         FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);

uint16_t      GetBaseTime() const ;
uint16_t      GetExternBaseTime() const ;
uint64_t      GetExternTimestamp() const;
int64_t       GetDeltaT() const ;
uint16_t      GetIdOfExternSystem() const;
uint16_t      GetChecksum() const;
void	      SetBaseTime(uint16_t bt);
void	      SetExternBaseTime(uint16_t bt);
void	      SetDeltaT(int64_t dt);
void	      SetIdOfExternSystem (uint16_t id);
void	      SetChecksum (uint16_t ck);
};
#pragma pack(pop) // free aligment
#endif
