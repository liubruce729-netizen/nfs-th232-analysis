#ifndef _MFMReaTraceFrame_
#define _MFMReaTraceFrame_
/*
  MFMReaTraceFrame.h

  Copyright Acquisition group, GANIL Caen, France

*/

#include "MFMBasicFrame.h"
#include<map>
#include <vector>
#include "MFMNumExoFrame.h"
#define REA_TRACE_HEADERSIZE 30
#define REA_TRACE_STD_UNIT_BLOCK_SIZE 1
#define REA_TRACE_FRAMESIZE 8192
#define REA_TRACE_NB_OF_ITEMS 4080

#define MFM_REA_TRACE_FRAME_TYPE_TXT "MFM_REA_TRACE_FRAME_TYPE"
#define MFM_REA_TRACE_FRAME_TYPE_SHORT_TXT "Trace"
#define MFM_REA_TRACE_FRAME_TYPE_LONG_TXT "REA Trace"
#pragma pack(push, 1) // force alignment

struct MFM_ReaTrace_EventInfo {
  unsigned LocationId   : 16;
  unsigned EventIdx     : 32;
  char     EventTime[6] ;
  unsigned SetupTrace   : 16;
};

// ReaTrace
struct MFM_ReaTrace_Item {
  unsigned Value :  16;
};

struct MFM_ReaTraceCheckSum {
  unsigned CheckSum :  16;
};

struct MFM_ReaTrace_Header{
  MFM_basic_header              ReaTraceBasicHeader;
  MFM_ReaTrace_EventInfo 	ReaTraceEvtInfo;
};
struct MFM_ReaTrace_Frame{
  MFM_ReaTrace_Header       ReaTraceReaTraceHeader;
  MFM_ReaTrace_Item         MFMReaTraceItem;
  MFM_ReaTraceCheckSum 	    CheckSum;
};

//____________MFMReaTraceFrame___________________________________________________________

class MFMReaTraceFrame : public MFMBasicFrame
{


 private:
  long long * fCountNbEventCard;
  int fChangedFrameDefinedSize; 
  int fChangedFrameItemNumber; 
 public :

  MFMReaTraceFrame();
  MFMReaTraceFrame(int unitBlock_size, int dataSource,
	       int frameType, int revision, int frameSize,int headerSize,
	       int itemSize, int nItems);
  virtual ~MFMReaTraceFrame();
  char* MyClassName()const{ return (char*)"MFMReaTraceFrame";};
  //void SetAttributs(void * pt =NULL);
  void SetUserDataPointer();
  int GetDefinedNbItems()const ;

  int GetDefinedUnitBlockSize()const {return REA_TRACE_STD_UNIT_BLOCK_SIZE;};
  int GetDefinedHeaderSize()const {return REA_TRACE_HEADERSIZE;};
  int GetItemSizeFromStructure(int type=0)const{ return sizeof (MFM_ReaTrace_Item);};
  int GetDefinedFrameSize()const;
  const char * GetTypeText()const {return MFM_REA_TRACE_FRAME_TYPE_TXT;}
  const char * GetTypeShortText()const {return MFM_REA_TRACE_FRAME_TYPE_SHORT_TXT;}
  const char * GetTypeLongText()const {return MFM_REA_TRACE_FRAME_TYPE_LONG_TXT;}
  void ChangeDefinedFrameSize(int size);
  
  void SetTimeStampFromReaTraceFrameData();
  void SetEventNumberFromReaTraceFrameData();
  virtual void SetTimeStamp(uint64_t timestamp);
  virtual void SetEventNumber(uint32_t eventnumber);

   uint16_t GetBoardId()const;
   bool HasBoardId () const{ return true;};

   uint16_t GetChannelId()const ;

   void SetLocationId(uint16_t Id);
   void SetLocationId(uint16_t ChannelId, uint16_t BoardId);
   uint16_t GetLocationId()const;

   uint16_t GetSetupTrace()const;
   void SetSetupTrace(uint16_t setup);
  
   void ReaTraceGetParametersByItem(MFM_ReaTrace_Item *item,uint16_t *value)const;
   void ReaTraceSetParametersByItem(MFM_ReaTrace_Item *item,uint16_t value);
   void ReaTraceGetParameters(int i, uint16_t *value)const;
   void ReaTraceSetParameters(int i, uint16_t value);
   uint16_t GetCheckSum()const;
   void SetCheckSum(uint16_t checksum); 
  
  
   void FillDataWithRamdomValue(  uint64_t timestamp, uint32_t enventnumber,int nbitem);
   void FillScalerWithVector(uint64_t timestamp,uint32_t EventCounter,int64_t *fVector,int sizeofvector);
   string GetHeaderDisplay(char* infotext)const ;
   string DumpData(char mode='d', bool nozero=false)const;
   void InitStat() ;
   void FillStat();
   string  GetStat(string info)const;
   bool IsSame(MFMReaTraceFrame* testframe,int verbose);
   bool UnitTest(int verbose);

};
#pragma pack(pop) // free alignment
#endif
