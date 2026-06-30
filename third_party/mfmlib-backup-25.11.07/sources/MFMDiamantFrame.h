#ifndef _MFMDiamantFrame_
#define _MFMDiamantFrame_
/*
  MFMDiamantFrame.h

	Copyright Acquisition Group, GANIL Caen, France

*/


#include "MFMNumExoFrame.h"

# define DIA_NB_STATUS 2
# define DIA_NB_CHANNELS 16
#pragma pack(push, 1) // force alignment
# define MFM_DIAMANT_FRAME_TYPE_TXT "MFM_DIAMANT_FRAME_TYPE"
# define MFM_DIAMANT_FRAME_TYPE_SHORT_TXT "Diama"
# define MFM_DIAMANT_FRAME_TYPE_LONG_TXT  "Diamant"

struct MFM_dia_data{
  unsigned CristalId : 16;
  unsigned Status1   : 8;
  unsigned Status2   : 8;
  unsigned Energy    : 32;
  unsigned Top       : 32;
  unsigned Checksum  : 16;
};


struct MFM_dia_frame{
	 MFM_common_header  Header;
	 MFM_numexo_eventInfo  EventInfo;
	 MFM_dia_data       Data;
};




//____________MFMDiamantFrame___________________________________________________________

class MFMDiamantFrame:public MFMNumExoFrame
{
public:
MFMDiamantFrame();
MFMDiamantFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
~MFMDiamantFrame();


// DIAMANT

 void      SetStatus(int i, uint16_t status);
 uint16_t  GetStatus(int i)const;
 void      SetEnergy(uint32_t energy) ;
 uint32_t  GetEnergy()const;
 void      SetTop(uint32_t energy);
 uint32_t  GetTop()const;
 void      SetUserDataPointer();
 string    GetDumpData(char mode='d', bool nozero=false) const;
 void      FillDataWithRamdomValue(uint64_t timestamp,uint32_t eventnumber);
 const char * GetTypeText()const {return MFM_DIAMANT_FRAME_TYPE_TXT;}
 const char * GetTypeShortText()const {return MFM_DIAMANT_FRAME_TYPE_SHORT_TXT;}
 const char * GetTypeLongLong()const {return MFM_DIAMANT_FRAME_TYPE_LONG_TXT;}
};
#pragma pack(pop) // free aligment
#endif
