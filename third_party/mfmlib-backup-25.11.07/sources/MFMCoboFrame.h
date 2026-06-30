#ifndef _MFMCoboFrame_
#define _MFMCoboFrame_
/*
  MFMCoboFrame.h

	Copyright Acquisition group, GANIL Caen, France

*/
#define COBO_SAMPLE_MASK	0x0fff // 12 bits
#define COBO_CHANIDX_MASK	0x007f //  7 bits
#define COBO_AGETIDX_MASK	0x0003 //  2 bits
#define COBO_BUCKIDX_MASK	0x01ff //  9 bits
#define COBO_STD_UNIT_BLOCK_SIZE 128
#define COBO_HEADERSIZE 128


#define MFM_COBO_FRAME_TYPE_TXT  "MFM_COBO_FRAME_TYPE"
#define MFM_COBOF_FRAME_TYPE_TXT "MFM_COBOF_FRAME_TYPE"
#define MFM_COBO_FRAME_TYPE_SHORT_TXT  "Cobo "
#define MFM_COBOF_FRAME_TYPE_SHORT_TXT "CoboF"
#define MFM_COBO_FRAME_TYPE_LONG_TXT  "Cobo "
#define MFM_COBOF_FRAME_TYPE_LONG_TXT "Cobo Full"
#include "MFMBasicFrame.h"
#include <vector>
#include "Cobo.h"

#pragma pack(push, 1)// force alignment
struct MFM_cobo_eventInfo {
  char eventTime[6];
  unsigned eventIdx  : 32;
};

// cobo
struct MFM_cobo_board {
  unsigned coboIdx :    8;
  unsigned asaIdx  :    8;
  unsigned readOffset: 16;
  unsigned status:      8;
};

struct MFM_cobo_hits {
  unsigned char hitPat_0[9];
  unsigned char hitPat_1[9];
  unsigned char hitPat_2[9];
  unsigned char hitPat_3[9];
  unsigned char multip_0[2];
  unsigned char multip_1[2];
  unsigned char multip_2[2];
  unsigned char multip_3[2];
};

struct MFM_coboItem {
	  unsigned agetIdx :  2;
	  unsigned chanIdx :  7;
	  unsigned buckIdx :  9;
	  unsigned unusedIdx: 2;
	  unsigned sample:   12;
	};
struct MFM_cobofItem {
	  unsigned agetIdx :  2;
	  unsigned unusedIdx: 2;
	  unsigned sample:   12;
	};
struct MFM_cobo_out{
	unsigned windowOut:32;
	unsigned char lastCell_0[2];
	unsigned char lastCell_1[2];
	unsigned char lastCell_2[2];
	unsigned char lastCell_3[2];
};

struct MFM_cobo_header{
	 MFM_basic_header   coboBasicHeader;
	 MFM_cobo_eventInfo coboEvtInfo;
	 MFM_cobo_board     coboBoard;
	 MFM_cobo_hits      coboHit;
	 MFM_cobo_out       coboOut;
};


//#pragma pack(pop) // free alignment

//____________MFMCoboFrame___________________________________________________________

class MFMCoboFrame : public MFMBasicFrame
{
public:


private:

int  fNbentries[COBO_NB_AGET][COBO_NB_AGET_CHANNEL];
int  fCoef[COBO_NB_AGET][COBO_NB_AGET_CHANNEL];

int  fChannelcount[COBO_NB_AGET];
int  fBucketcount[COBO_NB_AGET];

vector<int> fCountCoboFrame;     // vector to make statistic
vector<int> fCountEmptyCoboFrame;// vector to make statistic

public :

MFMCoboFrame();
MFMCoboFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize,
		       int itemSize, int nItems);
virtual ~MFMCoboFrame();

//virtual void SetHeaderBasic(MFM_basic_header* header) ;
virtual void SetAttributs(void * pt =NULL);
void	SetTimeStampFromCoboFrameData();
void	SetEventNumberFromCoboFrameData();
int     GetItemSizeFromStructure(int type=0) const;
virtual void SetTimeStamp(uint64_t timestamp);
virtual void SetEventNumber(uint32_t eventnumber);
virtual string  GetHeaderDisplay(char* infotext=NULL) const;

virtual void FillStat();
virtual void InitStat();
string GetStat  (string info )const;

// COBO
virtual const char * GetTypeText()const {if (GetFrameType() == MFM_COBO_FRAME_TYPE) return  MFM_COBO_FRAME_TYPE_TXT;
else return  MFM_COBOF_FRAME_TYPE_TXT;}
virtual const char * GetTypeShortText()const {if (GetFrameType() == MFM_COBO_FRAME_TYPE) return  MFM_COBO_FRAME_TYPE_SHORT_TXT;
else return  MFM_COBOF_FRAME_TYPE_SHORT_TXT;}
virtual const char * GetTypeLongText()const {if (GetFrameType() == MFM_COBO_FRAME_TYPE) return  MFM_COBO_FRAME_TYPE_LONG_TXT;
else return  MFM_COBOF_FRAME_TYPE_LONG_TXT;}

int GetDefinedNbItems()const {return COBO_NB_MAX_ITEM;}
int GetDefinedFrameSize()  const{return 0;} // 0 mean that FrameSize have to be computed
int GetDefinedUnitBlockSize()const {return COBO_STD_UNIT_BLOCK_SIZE;};
int GetDefinedHeaderSize()const {return COBO_HEADERSIZE;};

 void CoboRazCounts();
 int CoboGetCoboIdx()const;
 int CoboGetAsaIdx()const;
 int CoboGetReadOffset()const;
 int CoboGetStatus()const;
 char* CoboGetHitPat(int i)const;
 char* CoboGetMultip(int i)const;
 uint32_t CoboGetWindowOut()const;
 char* CoboGetLastCell(int i)const;

 void CoboSetCoboIdx(int coboidx);
 void CoboSetAsaIdx(int asaidx);
 void CoboSetReadOffset(int offset);
 void CoboSetStatus(int status);
 void CoboSetHitPat(int i,char* hitpat);
 void CoboSetMultip(int i,char* multi);
 void CoboSetWindowOut(uint32_t  windowsout);
 void CoboSetLastCell(int i,char* cell);

 void CoboGetParametersByItem(MFM_coboItem *item,uint32_t * sample, uint32_t *buckidx,uint32_t *chanidx,uint32_t *agetidx);
 void CoboSetParametersByItem(MFM_coboItem *item,uint32_t sample,uint32_t buckidx,uint32_t chanidx,uint32_t agetidx);
 void CoboGetParametersByItem(MFM_cobofItem *item,uint32_t * sample,uint32_t *agetidx);
 void CoboSetParametersByItem(MFM_cobofItem *item,uint32_t sample,uint32_t agetidx);
 void CoboSetParameters(int i,uint32_t sample,uint32_t buckidx,uint32_t chanidx,uint32_t agetidx);
 void CoboSetParameters(uint32_t sample, uint32_t buckidx,uint32_t chanidx, uint32_t agetidx);
 void CoboGetParameters(int i,uint32_t *sample, uint32_t *buckidx,uint32_t *chanidx,uint32_t *agetidx);
 void FillDataWithRamdomValue(  uint64_t timestamp, uint32_t enventnumber, int nbitem);
//void WriteRandomFrame(int lun, int nbframe,int verbose, int dumpsize, int  type);
};
#pragma pack(pop) // free alignment
#endif
