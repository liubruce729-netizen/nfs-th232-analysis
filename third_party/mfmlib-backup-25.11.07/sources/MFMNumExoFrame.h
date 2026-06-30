#ifndef _MFMNumExoFrame_
#define _MFMNumExoFrame_
/*
  MFMNumExoFrame.h

	Copyright Acquisition group, GANIL Caen, France
	Class for  frames based on Numexo2 cards.
    Author : Luc Legeard
*/
#include "MFMBlobFrame.h"

#define NUMEXO_FRAMESIZE 32
#define NUMEXO_HEADERFRAMESIZE 18 // 8 from Common header + MFM_numexo_eventInfo (10)

#define NUMEXO_NUMBER_CRISTAL_ID 2
#define NUMEXO_STD_UNIT_BLOCK_SIZE 4
#define NUMEXO_NB_CHANNELS 16
#define NUMEXO_MAX_NUMB_BOARDS 0x07ff
#define NUMEXO_MAX_CHAN_AND_BOARDS 0xffff
#define NUMEXO_MAX_CHAN_AND_BOARDS 0xffff


#define MFM_NUMEXO_TYPE_TXT "MFM_NUMEXO_GENERIC_TYPE"
#define MFM_NUMEXO_TYPE_SHORT_TXT "Numex"
#define MFM_NUMEXO_TYPE_LONG_TXT "Numexo2"
#pragma pack(push, 1) // force alignment

struct MFM_numexo_eventInfo {
  unsigned EventIdx  : 32;
  char EventTime[6];
};

struct MFM_numexo_data{
	 unsigned CristalId : 16;
	 unsigned Data1 : 16;
	 unsigned Data2 : 16;
	 unsigned Data3 : 16;
	 unsigned Data4 : 16;
	 unsigned Data5 : 16;
	 unsigned Checksum : 16;
};

struct MFM_numexo_frame{
	 MFM_common_header     Header;
	 MFM_numexo_eventInfo  EventInfo;
	 MFM_numexo_data       Data;
};

//____________MFMNumExoFrame___________________________________________________________

class MFMNumExoFrame : public MFMBlobFrame
{
public :
long long * fCountNbEventCard;

public :

MFMNumExoFrame();
MFMNumExoFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
virtual ~MFMNumExoFrame();
char*     MyClassName()const{ return (char*)"MFMNumExoFrame";};
//virtual void SetAttributs(void * pt =NULL);
void SetUserDataPointer();
void SetTimeStampFromNumexoFrameData();
void SetEventNumberFromNumexoFrameData();
virtual void SetTimeStamp(uint64_t timestamp);
virtual void SetEventNumber(uint32_t eventnumber);
virtual string GetHeaderDisplay(char* infotext=NULL)const;
bool HasBoardId() const{return true;};

virtual const char * GetTypeText()const {return MFM_NUMEXO_TYPE_TXT;}
virtual const char * GetTypeShortText()const {return MFM_NUMEXO_TYPE_SHORT_TXT;}
virtual const char * GetTypeLongText()const {return MFM_NUMEXO_TYPE_LONG_TXT;}
virtual int GetDefinedUnitBlockSize()const {return NUMEXO_STD_UNIT_BLOCK_SIZE;};
virtual int GetDefinedHeaderSize()const {return NUMEXO_HEADERFRAMESIZE;};
virtual int GetDefinedFrameSize()const {return NUMEXO_FRAMESIZE;};

virtual void FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber) ;
void      SetCristalId(uint16_t cristalId);
void      SetCristalId(uint16_t tgRequest, uint16_t idBoard);
void      SetLocationId(uint16_t channel, uint16_t idBoard);
uint16_t  GetCristalId()const;
uint16_t  GetTGCristalId()const;
uint16_t  GetChannelId()const;
uint16_t  GetBoardId()const;
virtual void      SetChecksum(uint16_t cristalId);;
virtual uint16_t  GetChecksum()const;
virtual uint16_t  ComputeChecksum() const;
virtual bool      VerifyChecksum()const;
virtual void 	  InitStat();
virtual void 	  FillStat();
virtual string    GetStat(string info)const;
virtual bool IsSame(MFMNumExoFrame* testframe,int verbose);

};
#pragma pack(pop) // free aligment
#endif
