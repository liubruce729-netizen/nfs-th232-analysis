#ifndef _MFMVamosTACFrame_
#define _MFMVamosTACFrame_
/*
  MFMVamosTACFrame.h

	Copyright Acquisition group, GANIL Caen, France

*/

#define VAMOSTAC_HEADERFRAMESIZE 18

#define VAMOSTAC_NB_STATUS 2
#define VAMOSTAC_STD_UNIT_BLOCK_SIZE 4
#define MFM_VAMOSTAC_FRAME_TYPE_TXT "MFM_VAMOSTAC_FRAME_TYPE"
#define MFM_VAMOSTAC_FRAME_TYPE_SHORT_TXT "VamTa"
#define MFM_VAMOSTAC_FRAME_TYPE_LONG_TXT "Vamos TAC"

#include "MFMNumExoFrame.h"
#pragma pack(push, 1) // force alignment
struct MFM_TACvamos_eventInfo {
  unsigned EventIdx  : 32;
  char     EventTime[6];
};


struct MFM_TACvamos_data{
  unsigned CristalId : 16;
  unsigned Status1   : 16;
  unsigned Status2   : 16;
  unsigned NotUsed1  : 16;
  unsigned Time      : 16;
  unsigned NotUsed2  : 16;
  unsigned Checksum  : 16;
};


struct MFM_TACvamos_frame{
	 MFM_common_header       Header;
	 MFM_TACvamos_eventInfo  Info;
	 MFM_TACvamos_data       Data;
};



//____________MFMVamosTACFrame___________________________________________________________

class MFMVamosTACFrame : public MFMNumExoFrame
{
public :



 const char * GetTypeText()const {return MFM_VAMOSTAC_FRAME_TYPE_TXT;}
 const char * GetTypeShortText()const {return MFM_VAMOSTAC_FRAME_TYPE_SHORT_TXT;}
 const char * GetTypeLongText()const {return MFM_VAMOSTAC_FRAME_TYPE_LONG_TXT;} 
 
 int GetDefinedUnitBlockSize()const {return MFM_UNIT_BLOCK_DEFAULT_SIZE;};


 void      SetStatus(int i, uint16_t status);
 uint16_t  GetStatus(int i)const;

 void      SetTime(uint16_t time);
 uint16_t  GetTime()const;
 void      SetChecksum(uint16_t cksum);
 uint16_t  GetChecksum()const;
 void      FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);

 void 	  InitStat();
 void 	  FillStat();
 string   GetStat(string info)const;

};
#pragma pack(pop) // free aligment
#endif
