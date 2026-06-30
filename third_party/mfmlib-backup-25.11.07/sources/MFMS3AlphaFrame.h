#ifndef _MFMS3AlphaFrame_
#define _MFMS3AlphaFrame_
/*
  MFMS3AlphaFrame.h

	Copyright Acquisition group, GANIL Caen, France
	Author : Luc Legeard
	Class Alpha frame based on Numexo2 card frame.
*/

#include "MFMNumExoFrame.h"


#pragma pack(push, 1) // force alignment
#define MFM_S3_ALPHA_FRAME_TYPE_TXT "MFM_S3_ALPHA_FRAME_TYPE"
#define MFM_S3_ALPHA_FRAME_TYPE_SHORT_TXT "S3alp"
#define MFM_S3_ALPHA_FRAME_TYPE_LONG_TXT "S3 Alpha"

//____________MFMS3AlphaFrame___________________________________________________________

class MFMS3AlphaFrame:public MFMNumExoFrame
{
public:

MFMS3AlphaFrame();
MFMS3AlphaFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
virtual ~MFMS3AlphaFrame();

 void      SetSetup(uint16_t i,uint16_t setup);
 uint16_t  GetSetup(uint16_t i);
 void      SetStatus(uint16_t status);
 uint16_t  GetStatus();
 void      SetEnergy(uint16_t energy);
 uint16_t  GetEnergy();
 void      FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);
 const char * GetTypeText()const {return MFM_S3_ALPHA_FRAME_TYPE_TXT;} 
 const char * GetTypeShortText()const {return MFM_S3_ALPHA_FRAME_TYPE_SHORT_TXT;}
 const char * GetTypeLongText()const  {return MFM_S3_ALPHA_FRAME_TYPE_LONG_TXT;}
};
#pragma pack(pop) // free aligment
#endif
