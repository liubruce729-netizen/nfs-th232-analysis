#ifndef _MFMS3BaF2Frame_
#define _MFMS3BaF2Frame_
/*
  MFMS3BaF2Frame.h

	Copyright Acquisition group, GANIL Caen, France
	Class S3BaF2 frame based on Numexo2 card frame.
    Author : Luc Legeard
*/

#include "MFMNumExoFrame.h"

#define MFM_S3_BAF2_FRAME_TYPE_TXT "MFM_S3_BAF2_FRAME_TYPE"
#define MFM_S3_BAF2_FRAME_TYPE_SHORT_TXT "S3Baf"
#define MFM_S3_BAF2_FRAME_TYPE_LONG_TXT "S3 Baf2"
#pragma pack(push, 1) // force alignment


//____________MFMS3BaF2Frame___________________________________________________________

class MFMS3BaF2Frame:public MFMNumExoFrame
{
public:

MFMS3BaF2Frame();
MFMS3BaF2Frame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
virtual ~MFMS3BaF2Frame();

 void      SetSetup(uint16_t setup);
 uint16_t  GetSetup();
 void      SetStatus(uint16_t status);
 uint16_t  GetStatus();
 void      SetEnergy(uint16_t energy);
 uint16_t  GetEnergy();
void      FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);
const char * GetTypeText()const {return MFM_S3_BAF2_FRAME_TYPE_TXT;} 
const char * GetTypeShortText()const {return MFM_S3_BAF2_FRAME_TYPE_SHORT_TXT;} 
const char * GetTypeLongText()const {return MFM_S3_BAF2_FRAME_TYPE_LONG_TXT;}
};
#pragma pack(pop) // free aligment
#endif
