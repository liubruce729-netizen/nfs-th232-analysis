#ifndef _MFMVamosICFrame_
#define _MFMVamosICFrame_
/*
  MFMVamosICFrame.h

	Copyright Acquisition group, GANIL Caen, France

*/
#define VAMOSIC_NB_STATUS 2
#define VAMOSIC_STD_UNIT_BLOCK_SIZE 4
#define MFM_VAMOSIC_FRAME_TYPE_TXT "MFM_VAMOSIC_FRAME_TYPE"
#define MFM_VAMOSIC_FRAME_TYPE_SHORT_TXT "VamIC"
#define MFM_VAMOSIC_FRAME_TYPE_LONG_TXT "Vamos Ionisation Chamber"
#include "MFMBlobFrame.h"
#include "MFMNumExoFrame.h"
#pragma pack(push, 1) // force alignment
struct MFM_ICvamos_eventInfo {
  unsigned EventIdx  : 32;
  char     EventTime[6];
};

struct MFM_ICvamos_data{
  unsigned CristalId : 16;
  unsigned Status1   : 16;
  unsigned Status2   : 16;
  unsigned NotUsed1  : 16;
  unsigned Energy    : 16;
  unsigned NotUsed2  : 16;
  unsigned Checksum  : 16;
};


struct MFM_ICvamos_frame{
	 MFM_common_header      Header;
	 MFM_ICvamos_eventInfo  Info;
	 MFM_ICvamos_data       Data;
};

//____________MFMVamosICFrame___________________________________________________________

class MFMVamosICFrame : public MFMNumExoFrame
{
public :

  const char * GetTypeText()const    {return MFM_VAMOSIC_FRAME_TYPE_TXT;} 
  const char * GetTypeShortText()const    {return MFM_VAMOSIC_FRAME_TYPE_SHORT_TXT;}
  const char * GetTypeLongText()const    {return MFM_VAMOSIC_FRAME_TYPE_LONG_TXT;} 
  
  int GetDefinedUnitBlockSize()const {return VAMOSIC_STD_UNIT_BLOCK_SIZE;};
 
 void      SetStatus(int i, uint16_t status);
 uint16_t  GetStatus(int i) const;

 void      SetEnergy(uint16_t energy);
 uint16_t  GetEnergy()const;

 void      FillEventRandomConst(uint64_t timestamp=0,uint32_t enventnumber=0);

 void 	  InitStat();
 void 	  FillStat();
 string   GetStat(string info) const;

};
#pragma pack(pop) // free aligment
#endif
