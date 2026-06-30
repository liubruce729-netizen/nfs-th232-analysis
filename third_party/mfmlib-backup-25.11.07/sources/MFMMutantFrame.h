#ifndef _MFMMutantFrame_
#define _MFMMutantFrame_
/*
  MFMMutantFrame.h
	Copyright Acquisition group, GANIL Caen, France
*/

#define MUT_HEADERFRAMESIZE 18
#define MUT_FRAMESIZE 64
#define MUT_NB_EVTCOUNT 4
#define MUT_NB_SCALER 5
#define MUT_UNIT_BLOCK_SIZE 2
#define MUT_NB_MULTIPLICITY 2
#define	MFM_MUTANT_FRAME_TYPE_TXT	"MFM_MUTANT_FRAME_TYPE"
#define	MFM_MUTANT_FRAME_TYPE_SHORT_TXT	"Mutan"
#define	MFM_MUTANT_FRAME_TYPE_LONG_TXT	"Mutant"
#include "MFMBlobFrame.h"

#pragma pack(push, 1) // force alignment
struct MFM_mut_eventInfo {
  char EventTime[6];
  unsigned EventIdx  : 32;
};

struct MFM_mut_data{
  uint16_t TriggerInfo ;
  uint16_t Mutiplicity[2];/// 0->A; 1->B;
  uint32_t EvtCount[4]; /// 0->L0 , 1->L1A , 2-> L1B  3->3L2_
  uint32_t Scaler[5];
  uint32_t D2pTime ;
};


struct MFM_mut_frame{
	 MFM_common_header  MutBlobHeader;
	 MFM_mut_eventInfo  MutEventInfo;
	 MFM_mut_data       MutData;
};


//____________MFMMutantFrame___________________________________________________________

class MFMMutantFrame : public MFMBlobFrame
{

public :

MFMMutantFrame();
MFMMutantFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
 ~MFMMutantFrame();

// void SetAttributs(void * pt =NULL);

void SetTimeStampFromMutantFrameData();
void SetEventNumberFromMutantFrameData();

 void SetTimeStamp(uint64_t timestamp);
 void SetEventNumber(uint32_t eventnumber);

 const char * GetTypeText()const {return MFM_MUTANT_FRAME_TYPE_TXT;}
 const char * GetTypeShortText()const {return MFM_MUTANT_FRAME_TYPE_SHORT_TXT;}
 const char * GetTypeLongText()const {return MFM_MUTANT_FRAME_TYPE_LONG_TXT;} 
 int GetDefinedUnitBlockSize()const {return MUT_UNIT_BLOCK_SIZE;};
 int GetDefinedHeaderSize()const {return MUT_HEADERFRAMESIZE;};
 int GetDefinedFrameSize()const {return MUT_FRAMESIZE;};
 string GetHeaderDisplay(char* infotext)const;
// MUTANT
 void      SetTriggerInfo(uint16_t trig);
 uint16_t  GetTriggerInfo() const;

 void      SetMultiplicity(int i, uint16_t mult);
 uint16_t  GetMultiplicity(int i)const;

 void      SetEvtCount(int i, uint32_t count);
 uint32_t  GetEvtCount(int i)const;

 void      SetScaler(int i, uint32_t scaler);
 uint32_t  GetScaler(int i)const;

 void      SetD2pTime(uint32_t trig);
 uint32_t  GetD2pTime()const;
 void      FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);
 string    DumpData()const;
};
#pragma pack(pop) // free aligment
#endif
