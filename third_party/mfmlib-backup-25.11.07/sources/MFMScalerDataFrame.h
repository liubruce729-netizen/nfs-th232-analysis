#ifndef _MFMScalerDataFrame_
#define _MFMScalerDataFrame_
#include "GEN_TYPE.H"
#include "gan_acq_buf.h" 
#include <stdlib.h>
using namespace std;
/*
  MFMScalerDataFrame.h

	Copyright Acquisition group, GANIL Caen, France
*/

#include "MFMBasicFrame.h"
#include "GEN_TYPE.H"
#include "STR_EVT.H"
#define SCALER_DATA_HEADERSIZE 28;
#define SCALER_STD_UNIT_BLOCK_SIZE 4;
#define	MFM_SCALER_DATA_FRAME_TYPE_TXT	"MFM_SCALER_DATA_FRAME_TYPE"
#define	MFM_SCALER_DATA_FRAME_TYPE_SHORT_TXT	"Scale"
#define	MFM_SCALER_DATA_FRAME_TYPE_LONG_TXT	"Scaler"
#define MFM_SCALER_NB_ITEMS 20;
#pragma pack(push, 1) // force alignment

struct MFM_ScalerData_Info {
  char eventTime[6];
  unsigned eventIdx  : 32;
};

struct MFM_ScalerData_Item {
	  uint32_t Label;
	  uint64_t Count;
	  uint64_t Frequency;
	  int32_t  Status;
	  uint64_t Tics;
	  int32_t  AcqStatus;
	};

struct MFM_ScalerData_header{
	 MFM_basic_header ScalerDataBasicheader;
	 MFM_ScalerData_Info ScalerData_Info;
	 uint16_t Unused;
     };

//____________MFMScalerDataFrame___________________________________________________________

class MFMScalerDataFrame : public MFMBasicFrame
{

public :

MFMScalerDataFrame();
MFMScalerDataFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize,
		       int itemSize, int nItems);
virtual ~MFMScalerDataFrame();

 void SetAttributs(void * pt =NULL);
uint32_t  GetEventNumber() const;
//uint32_t  GetEventFrom() const;
 uint64_t GetTimeStamp();
void SetTimeStampFromScalerFrameData();
void SetEventNumberFromScalerFrameData();
void SetTimeStamp(uint64_t timestamp);
void SetEventNumber(uint32_t eventnumber);
void GetValuesByItem(MFM_ScalerData_Item *item,uint32_t * label,
		uint64_t *count, uint64_t *frequency,int32_t * status,uint64_t * tics, int32_t* acqstatus)const;
void SetValuesByItem(MFM_ScalerData_Item *item,uint32_t  label,
		uint64_t count, uint64_t frequency,int32_t status,uint64_t tics, int32_t acqstatus);
void GetValues(int i,uint32_t * label, uint64_t *count, uint64_t *frequency,
		int32_t * status,uint64_t * tics, int32_t* acqstatus) const;
void SetValues(int i,uint32_t  label, uint64_t count, uint64_t frequency,
		int32_t status, uint64_t tics, int32_t acqstatus);
void FillDataWithRamdomValue( uint64_t timestamp,uint32_t enventnumber,int nbitem );
void FillScalerWithVector(uint64_t timestamp,uint32_t EventCounter,int64_t * fVector, int sizeofvector);
void FillScalerWithIn2p3Buffer(char* buf,uint64_t timestamp);

const char * GetTypeText()const {return MFM_SCALER_DATA_FRAME_TYPE_TXT;}
const char * GetTypeShortText()const {return MFM_SCALER_DATA_FRAME_TYPE_SHORT_TXT;}
const char * GetTypeLongText()const {return MFM_SCALER_DATA_FRAME_TYPE_LONG_TXT;}
int GetDefinedUnitBlockSize()const {return SCALER_STD_UNIT_BLOCK_SIZE;};
int GetDefinedHeaderSize()const {return SCALER_DATA_HEADERSIZE;};
int GetItemSizeFromStructure(int type=0)const{ return sizeof (MFM_ScalerData_Item);};

int GetDefinedFrameSize()const {return 0;};

string GetDumpData(char mode='d', bool nozero=false) const;
string GetDumpTextData() const;
void DumpTextData() const;
};
#pragma pack(pop) // free alignment
#endif
