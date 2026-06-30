#ifndef _MFMExogamFrame_
#define _MFMExogamFrame_
/*
  MFMExogamFrame.h

	Copyright Acquisition group, GANIL Caen, France
    Author : Luc Legeard
*/
#define EXO_NB_OUTER 4
#define EXO_NB_INNER_M 2
#define EXO_NB_INNER_T 3
#define EXO_NB_STATUS 3
#define EXO_NB_MAX_PARA 17
#define EXO_FRAMESIZE 52
#define EXO_NUMBER_CRISTAL_ID 2
#define EXO_STD_UNIT_BLOCK_SIZE 4
#define MFM_EXO2_FRAME_TYPE_TXT "MFM_EXO2_FRAME_TYPE"
#define MFM_EXO2_FRAME_TYPE_SHORT_TXT "Exoga"
#define MFM_EXO2_FRAME_TYPE_LONG_TXT  "Exogam "
#include "MFMBlobFrame.h"
#include "MFMNumExoFrame.h"

#pragma pack(push, 1) // force alignment
struct MFM_exo_eventInfo {
  unsigned EventIdx  : 32;
  char EventTime[6];
};


struct MFM_exo_data{
  unsigned CristalId : 16;
  unsigned Status1   : 16;
  unsigned Status2   : 16;
  unsigned Status3   : 16;
  unsigned DeltaT    : 16;
  unsigned Inner6M   : 16;
  unsigned Inner20M  : 16;
  unsigned Outer1    : 16;
  unsigned Outer2    : 16;
  unsigned Outer3    : 16;
  unsigned Outer4    : 16;
  unsigned BGO       : 16;
  unsigned Csi       : 16;
  unsigned InnerT30  : 16;
  unsigned InnerT60  : 16;
  unsigned InnerT90  : 16;
  unsigned Padding   : 16;
};


struct MFM_exo_header{
	 MFM_common_header  ExoBlobcHeader;
	 MFM_exo_eventInfo  ExoEventInfo;
	 MFM_exo_data       ExoData;
};



//____________MFMExogamFrame___________________________________________________________

class MFMExogamFrame : public MFMBlobFrame
{
long long * fCountNbEventCard;


public :

MFMExogamFrame();
MFMExogamFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
virtual ~MFMExogamFrame();

 void SetUserDataPointer();
 //void SetAttributs(void * pt =NULL);
 void SetTimeStampFromExogamFrameData();
 void SetEventNumberFromExogamFrameData();
 void SetTimeStamp(uint64_t timestamp);
 void SetEventNumber(uint32_t eventnumber);
 string GetHeaderDisplay(char* infotext=NULL)const;

 const char * GetTypeText()const {return MFM_EXO2_FRAME_TYPE_TXT;} 
 const char * GetTypeShortText()const {return MFM_EXO2_FRAME_TYPE_SHORT_TXT;}
 const char * GetTypeLongText()const {return MFM_EXO2_FRAME_TYPE_LONG_TXT;} 
 int GetDefinedUnitBlockSize()const {return EXO_STD_UNIT_BLOCK_SIZE;};
 int GetDefinedHeaderSize()const {return NUMEXO_HEADERFRAMESIZE;};
 int GetDefinedFrameSize()const {return EXO_FRAMESIZE;};


 void      ExoSetCristalId(uint16_t cristalId);
 void      ExoSetCristalId(uint16_t tgRequest, uint16_t idBoard);
 uint16_t  ExoGetCristalId() const;
 uint16_t  ExoGetTGCristalId()const;
 uint16_t  ExoGetBoardId()const;
 uint16_t  GetBoardId()const ;
 bool      HasBoardId () const{return true;};

 void      ExoSetStatus(int i, uint16_t status);
 uint16_t  ExoGetStatus(int i)const;
 void      ExoSetDetaT(uint16_t detaT);
 uint16_t  ExoGetDeltaT()const;
 void      ExoSetInnerM(int i, uint16_t innner);
 uint16_t  ExoGetInnerM(int i)const;
 void      ExoSetOuter(int i, uint16_t outer);
 uint16_t  ExoGetOuter(int i)const;
 void      ExoSetBGO(uint16_t bgo);
 uint16_t  ExoGetBGO()const;
 void      ExoSetCsi(uint16_t Csi);
 uint16_t  ExoGetCsi()const;
 void      ExoSetInnerT(int i, uint16_t inner);
 uint16_t  ExoGetInnerT(int i)const;
 void      ExoSetPara(int i, uint16_t value);
 uint16_t  ExoGetPara(int i)const;
 void      ExoSetPadding(uint16_t padding);
 uint16_t  ExoGetPadding()const;
 void      FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);
 void 	   InitStat();
 void 	   FillStat();
 string    GetStat(string info)const;


};
#pragma pack(pop) // free aligment
#endif
