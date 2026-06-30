#ifndef _MFMSiriusFrame_
#define _MFMSiriusFrame_
/*
  MFMSiriusFrame.h

	Copyright Acquisition group, GANIL Caen, France

*/

#include "MFMBasicFrame.h"
#include "MFMNumExoFrame.h"
#include<map>
#include <vector>

#define SIRIUS_HEADERSIZE 64
#define SIRIUS_FRAME_SIZE      2048
#define SIRIUS_STD_UNIT_BLOCK_SIZE 2
#define SIRIUS_ITEMS_NUMBER  992

#define MFM_SIRIUS_FRAME_TYPE_TXT "MFM_SIRIUS_FRAME_TYPE"
#define MFM_SIRIUS_FRAME_TYPE_SHORT_TXT "Siriu"
#define MFM_SIRIUS_FRAME_TYPE_LONG_TXT "Sirius"

#pragma pack(push, 1) // force alignment


struct MFM_Sirius_eventInfo {
	  unsigned eventIdx  : 32;
	  char eventTime[6];
	};

// Sirius
struct MFM_SiriusItem {
	  unsigned Value :  16;
	};
struct MFM_SiriusData{
};

struct MFM_SiriusHeader{
	 MFM_basic_header  SiriusBasicHeader;
	 unsigned LocationId  :  16;
	 MFM_Sirius_eventInfo SiriusEvtInfo;
	 unsigned DSSD_gain :16;
	 uint16_t Feedback[16];  
	 unsigned Free :   16;	 
};

struct MFM_SiriusFrame{
	 MFM_SiriusHeader      Header;
	 MFM_SiriusData        Data;
};

//____________MFMSiriusFrame___________________________________________________________

class MFMSiriusFrame : public MFMBasicFrame
{

private :
  long long * fCountNbEventCard;
protected:
   virtual void SetTimeStampFromSiriusFrameData();
   virtual void SetEventNumberFromSiriusFrameData();
   
public :
MFMSiriusFrame();
MFMSiriusFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize,
		       int itemSize, int nItems);
virtual ~MFMSiriusFrame();

bool IsParameterPresent(const string & name) const;

//void SetAttributs(void * pt =NULL);

const char * GetTypeText()const {return  MFM_SIRIUS_FRAME_TYPE_TXT;}
const char * GetTypeShortText()const {return  MFM_SIRIUS_FRAME_TYPE_SHORT_TXT;}
const char * GetTypeLongText()const {return  MFM_SIRIUS_FRAME_TYPE_LONG_TXT;}

int GetItemSizeFromStructure(int type=0) const{return sizeof (MFM_SiriusItem);};
int GetDefinedUnitBlockSize()const {return SIRIUS_STD_UNIT_BLOCK_SIZE;};
int GetDefinedHeaderSize() const {return SIRIUS_HEADERSIZE;};
int GetDefinedFrameSize() const {return SIRIUS_FRAME_SIZE;};
int GetDefinedNbItems(){return SIRIUS_ITEMS_NUMBER;};

 bool HasBoardId() const {return true; }
 void SetLocationId(uint16_t Id);
 uint16_t GetLocationId() const;
 void SetLocationId(uint16_t ChannelId, uint16_t BoardId) ;
 uint16_t GetChannelId() const;
 uint16_t GetBoardId()const ;

 void SetTimeStamp(uint64_t timestamp);
 void SetEventNumber(uint32_t eventnumber);

 uint16_t GetGain()const;
 void SetGain(uint32_t gain);
 uint16_t GetFeedBack(int i) const ;
 void SetFeedBack(uint32_t feed, int i) ;

 void GetParametersByItem(MFM_SiriusItem *item,uint16_t *value)const;
 void SetParametersByItem(MFM_SiriusItem *item,uint16_t value);
 void GetParameters(int i, uint16_t *value)const;
 void SetParameters(int i, uint16_t value);
 void FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber,int nbitem );
 string GetHeaderDisplay(char* infotext)const ;
 string GetDumpData(char mode='d', bool nozero=false)const;
 void InitStat() ;
 void FillStat();
 string  GetStat(string info)const;

};
#pragma pack(pop) // free alignment
#endif
