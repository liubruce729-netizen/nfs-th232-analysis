#ifndef _MFMVamosPDFrame_
#define _MFMVamosPDFrame_
/*
  MFMVamosPDFrame.h

	Copyright Acquisition group, GANIL Caen, France

*/


#define VAMOSPD_FRAMESIZE 280
#define VAMOSPD_HEADERFRAMESIZE 18
#define VAMOSPD_TRIG_REQ_CRYS_ID_MASK 0x001f
#define VAMOSPD_BOARD_ID_MASK 0x07ff
#define VAMOSPD_NUMBER_CRISTAL_ID 2
#define VAMOSPD_NB_VALUE 64
#define VAMOSPD_STD_UNIT_BLOCK_SIZE 4
#define VAMOSPD_NB_CHANNEL 2048
#define MFM_VAMOSPD_FRAME_TYPE_TXT "MFM_VAMOSPD_FRAME_TYPE"
#define MFM_VAMOSPD_FRAME_TYPE_SHORT_TXT "VamPD"
#define MFM_VAMOSPD_FRAME_TYPE_LONG_TXT "Vamos Position Detector"  
#include "MFMBlobFrame.h"

#pragma pack(push, 1) // force alignment
struct MFM_vamosPD_eventInfo {
  unsigned EventIdx  : 32;
  char     EventTime[6];
};
struct MFM_LabelValue {
  unsigned Lab : 16;
  unsigned Val : 16;
};

struct MFM_vamosPD_data{
  unsigned CristalId : 16;
  MFM_LabelValue   LabelValue[64];
  unsigned LocalCount: 16;
  unsigned Checksum  : 16;
};


struct MFM_vamosPD_frame{
	 MFM_common_header      VamosPDBlobcHeader;
	 MFM_vamosPD_eventInfo  VamosPDEventInfo;
	 MFM_vamosPD_data       VamosPDData;
};



//____________MFMVamosPDFrame___________________________________________________________

class MFMVamosPDFrame : public MFMBlobFrame
{
long long * fCountNbEventCard;
int * fLabelIndice; // vector to make statistic
int * fIndiceLabel; // vector to make statistic
int * fNbLabels;// vector to make statistic
uint16_t fNbPara;
int MaxTrace;

public :

MFMVamosPDFrame();
MFMVamosPDFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
virtual ~MFMVamosPDFrame();

//virtual void SetAttributs(void * pt =NULL);
void SetUserDataPointer() ;
 
void SetTimeStampFromVamosPDFrameData();
void SetEventNumberFromVamosPDFrameData();
void SetTimeStamp(uint64_t timestamp);
void SetEventNumber(uint32_t eventnumber);
  
 string GetHeaderDisplay(char* infotext=NULL)const ;
  const char * GetTypeText()const    {return MFM_VAMOSPD_FRAME_TYPE_TXT;}
  const char * GetTypeShortText()const    {return MFM_VAMOSPD_FRAME_TYPE_SHORT_TXT;}
  const char * GetTypeLongText()const    {return MFM_VAMOSPD_FRAME_TYPE_LONG_TXT;}
  int GetDefinedUnitBlockSize()const {return VAMOSPD_STD_UNIT_BLOCK_SIZE;};
  int GetDefinedHeaderSize()const    {return VAMOSPD_HEADERFRAMESIZE;};
  int GetDefinedFrameSize()const     {return VAMOSPD_FRAMESIZE;};

void     SetCristalId(uint16_t cristalId) ;
void     SetCristalId(uint16_t tgRequest, uint16_t idBoard);
uint16_t GetCristalId() const;
uint16_t GetTGCristalId()const;
uint16_t GetBoardId()const;
bool     HasBoardId(){return true;};

 void      SetLabel(int i, uint16_t Label);
 uint16_t  GetLabel(int i)const;
 void      SetEnergy(int i, uint16_t energy);
 uint16_t  GetEnergy(int i)const;
 void      GetParameters(int i, uint16_t *label, uint16_t* value)const;
 void      SetLocalCount(uint16_t count);
 uint16_t  GetLocalCount()const;
 void      SetChecksum(uint16_t cksum);
 uint16_t  GetChecksum()const;
 void      FillDataWithRamdomValue(uint64_t timestamp,uint32_t eventnumber);

 string DumpData(char mode, bool nozero)const;
 string GetStat(string info)  const;
 void   FillStat();
 void   InitStat();

};
#pragma pack(pop) // free aligment
#endif
