#ifndef _MFMS3DeflectorFrame_
#define _MFMS3DeflectorFrame_
/*
  MFMS3DeflectorFrame.h

	Copyright Acquisition group, GANIL Caen, France
	Class MFMS3DeflectorFrame  based on Numexo2 card frame.
    Author : Luc Legeard
*/

#include "MFMNumExoFrame.h"

#define MFM_S3_DEFLECTOR_FRAME_TYPE_TXT "MFM_S3_DEFLECTOR_FRAME_TYPE"
#define MFM_S3_DEFLECTOR_FRAME_TYPE_SHORT_TXT "S3Def"
#define MFM_S3_DEFLECTOR_FRAME_TYPE_LONG_TXT "S3 Deflector"
#pragma pack(push, 1) // force alignment


//____________MFMS3DeflectorFrame___________________________________________________________

class MFMS3DeflectorFrame:public MFMNumExoFrame
{
public:


MFMS3DeflectorFrame();
MFMS3DeflectorFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
virtual ~MFMS3DeflectorFrame();

void      SetDeflector(uint16_t energy);
uint16_t  GetDeflector();
void      FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);
const char * GetTypeText()const {return MFM_S3_DEFLECTOR_FRAME_TYPE_TXT;} 
const char * GetTypeShortText()const {return MFM_S3_DEFLECTOR_FRAME_TYPE_SHORT_TXT;}
const char * GetTypeLongText()const {return MFM_S3_DEFLECTOR_FRAME_TYPE_LONG_TXT;} 
};
#pragma pack(pop) // free aligment
#endif
