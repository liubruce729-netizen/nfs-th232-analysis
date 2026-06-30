#ifndef _MFMOscilloFrame_
#define _MFMOscilloFrame_
/*
  MFMOscilloFrame.h

	Copyright Acquisition group, GANIL Caen, France

*/

#include "MFMBasicFrame.h"

#pragma pack(push, 1) // force alignment

// OSCILLO
struct MFM_OscilloItem {
	  unsigned Value :  16;
	};

struct MFM_Oscillo_EventInfo {
	  unsigned ChannelIdx :  16;
	  unsigned Config     :  16;
	};
struct MFM_OscilloHeader{
	 MFM_basic_header       BasicHeader;
	 MFM_Oscillo_EventInfo  EvtInfo;
};
struct MFM_OscilloData{
};
struct MFM_OscilloFrame{
	 MFM_OscilloHeader      Header;
	 MFM_OscilloData        Data;
};

#define MFM_CHANNELID_BOARD_MSK 0xFFE0
#define MFM_CHANNELID_NUMBER_MSK 0x001F
#define MFM_OSCILLO_NB_ITEMS     16384 /// 16384 is usual size for numexo2  
#define MFM_CONFIG_ONOFF_MSK    0x8000
#define MFM_CONFIG_TIMEBASE_MSK 0x7800
#define MFM_CONFIG_TRIG_MSK     0x0700
#define MFM_CONFIG_SIGNAL_MSK   0x00E0
#define MFM_CONFIG_IDXCHAN_MSK  0x001F
#define MFM_OSCILLO_HEADERSIZE  20
#define OSCILLO_STD_UNIT_BLOCK_SIZE 2

#define MFM_OSCI_FRAME_TYPE_TXT "MFM_OSCI_FRAME_TYPE"
#define MFM_OSCI_FRAME_TYPE_SHORT_TXT "Oscil"
#define MFM_OSCI_FRAME_TYPE_LONG_TXT  "Oscilloscope"

//____________MFMOscilloFrame___________________________________________________________

class MFMOscilloFrame : public MFMBasicFrame
{
private:

	long long * fCountNbEventCard;

public :

MFMOscilloFrame();
MFMOscilloFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize,
		       int itemSize, int nItems);
		       
virtual ~MFMOscilloFrame();

 void SetAttributs(void * pt =NULL);
 string GetHeaderDisplay(char* infotext=NULL);
 void SetConfig(uint16_t value);
 void SetConfig(uint16_t on_off,uint16_t basetime,uint16_t trig,uint16_t signal,uint16_t channel);
 void SetConfigOnOff(uint16_t onoff);
 void SetConfigTimeBase(uint16_t time);
 void SetConfigTrig(uint16_t trig);
 void SetConfigSignal(uint16_t signal);
 void SetConfigChannelIdx(uint16_t idx);
 void SetChannelIdx(uint16_t idx);
 void SetChannelIdx(uint16_t idxch,uint16_t idxboard);

const char * GetTypeText()const {return MFM_OSCI_FRAME_TYPE_TXT;}
const char * GetTypeShortText()const {return MFM_OSCI_FRAME_TYPE_SHORT_TXT;}
const char * GetTypeLongText()const {return MFM_OSCI_FRAME_TYPE_LONG_TXT;} 
int GetDefinedNbItems()const { return MFM_OSCILLO_NB_ITEMS ; }
int GetDefinedUnitBlockSize()const {return OSCILLO_STD_UNIT_BLOCK_SIZE;};
int GetDefinedHeaderSize()const {return MFM_OSCILLO_HEADERSIZE;};
int GetItemSizeFromStructure (int type)const {return(sizeof(MFM_OscilloItem));}
int GetDefinedFrameSize() const{return  0;} // 0 mean that FrameSize have to be computed
bool HasEventNumber() const { return false; }
bool HasTimeStamp() const { return false; }


//virtual void SetChannelIdxNumber(uint16_t idx);
//virtual void SetChannelIdxBoard(uint16_t idx);
virtual void 	  InitStat();
virtual void 	  FillStat();
virtual string    GetStat(string info);

uint16_t GetConfig() const; 
uint16_t GetConfigOnOff() const;
uint16_t GetConfigTimeBase() const;
uint16_t GetConfigTrig() const;
uint16_t GetConfigSignal() const;
uint16_t GetConfigChannelIdx() const;
uint16_t GetChannelIdx() const;
uint16_t GetChannelIdxNumber() const;
uint16_t GetChannelIdxBoard() const;
uint16_t GetBoardId() const;
bool HasBoardId() const { return true; }

virtual void OscilloGetParametersByItem(MFM_OscilloItem *item,uint16_t *value);
virtual void OscilloSetParametersByItem(MFM_OscilloItem *item,uint16_t  value);
virtual void OscilloGetParameters(int i, uint16_t *value);
virtual void OscilloSetParameters(int i, uint16_t value);

void FillDataWithRamdomValue(uint64_t timestamp,uint32_t eventnumber,int nbitem );

};
#pragma pack(pop) // free alignment
#endif
