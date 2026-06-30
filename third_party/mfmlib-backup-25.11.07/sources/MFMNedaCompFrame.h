#ifndef _MFMNedaCompFrame_
#define _MFMNedaCompFrame_
/*
  MFMNedaCompFrame.h

	Copyright Acquisition group, GANIL Caen, France

*/

#include "MFMBlobFrame.h"


#define NEDACOMP_HEADERSIZE 20
#define NEDACOMP_FRAMESIZE  42
#define NEDACOMP_STD_UNIT_BLOCK_SIZE 2
#define NEDACOMP_BOARD_ID_MASK 0x0fff
#define NEDACOMP_CHANNEL_ID_MASK 0x000f
#pragma pack(push, 1) // force alignment
#define	MFM_NEDACOMP_FRAME_TYPE_TXT	"MFM_NEDACOMP_FRAME_TYPE"
#define	MFM_NEDACOMP_FRAME_TYPE_SHORT_TXT	"NedaC"
#define	MFM_NEDACOMP_FRAME_TYPE_LONG_TXT	"Neda Compressed"
struct  MFM_NedaComp_EventInfo {
	  unsigned EventIdx     : 32;
	  char     EventTime[6];
	  unsigned LocationId     : 16;
	};

// NedaComp data
struct MFM_NedaComp_Data {
	  unsigned Energy         : 16;
	  unsigned Time           : 16;
	  unsigned TdcCorValue    : 16;
	  unsigned SlowIntegral   : 32;
	  unsigned FastIntegral   : 32;
	  unsigned IntRaiseTime : 32;
	  unsigned NeuralNetWork  : 16;
	  unsigned NbZero         : 8;
	  unsigned NeutronFlag    : 8;
	  };


struct MFM_NedaComp_Header{
	 MFM_common_header      NedaCompHeader;
	 MFM_NedaComp_EventInfo 	NedaCompEvtInfo;
};
struct MFM_NedaComp_Frame{
	 MFM_NedaComp_Header       NedaCompHeader;
	 MFM_NedaComp_Data         NedaCompData;
};


//____________MFMNedaCompFrame___________________________________________________________

class MFMNedaCompFrame : public MFMBlobFrame
{

private:

long long * fCountNbEventCard;

public :

MFMNedaCompFrame();
MFMNedaCompFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize,
		       int itemSize, int nItems);
virtual ~MFMNedaCompFrame();


//virtual void SetHeaderBasic(MFM_basic_header* header) ;

// void SetAttributs(void * pt =NULL);
 void SetUserDataPointer() ;
void SetTimeStampFromNedaCompFrameData();
void SetEventNumberFromNedaCompFrameData();

const char * GetTypeText()     const{return MFM_NEDACOMP_FRAME_TYPE_TXT;}
const char * GetTypeShortText()const{return MFM_NEDACOMP_FRAME_TYPE_SHORT_TXT;}
const char * GetTypeLongText() const{return MFM_NEDACOMP_FRAME_TYPE_LONG_TXT;}
int GetDefinedUnitBlockSize()  const{return NEDACOMP_STD_UNIT_BLOCK_SIZE;};
int GetDefinedHeaderSize()     const{return NEDACOMP_HEADERSIZE;};
int GetDefinedFrameSize()      const{return NEDACOMP_FRAMESIZE;} ;


 void SetTimeStamp(uint64_t timestamp);
 void SetEventNumber(uint32_t eventnumber);

 uint16_t GetBoardId()const;
bool HasBoardId() const { return true; }

 uint16_t GetChannelId() const;
 void SetLocationId(uint16_t Id);
 void SetLocationId(uint16_t ChannelId, uint16_t BoardId);
 uint16_t GetLocationId()const ;
 uint16_t GetEnergy() const;
 void SetEnergy(uint16_t energy) ;
 uint16_t GetTime() const;
 void SetTime(uint16_t time) ;
 uint16_t GetTdcCorValue() const;
 void SetTdcCorValue(uint16_t val);
 uint32_t GetSlowIntegral() const;
 void SetSlowIntegral(uint32_t integral);
 uint32_t GetFastIntegral() const;
 void SetFastIntegral(uint32_t integral) ;
 int32_t GetIntRaiseTime() const;
 void SetIntRaiseTime(int32_t time) ;
 uint16_t GetNeuralNetWork() const;
 void SetNeuralNetWork(uint16_t neural) ;
 uint8_t GetNbZero() const;
 void SetNbZero(uint8_t nb) ;
 bool GetNeutronFlag()const ;
 void SetNeutronFlag(bool neutron);
void FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);
 string GetHeaderDisplay(char* infotext)const ;
 void InitStat() ;
 void FillStat();
 string  GetStat(string info)const;
};
#pragma pack(pop) // free alignment
#endif
