#ifndef _MFMS3RuthFrame_
#define _MFMS3RuthFrame_
/*
  MFMS3RuthFrame.h

	Copyright Acquisition group, GANIL Caen, France
	Class Rutherford frame based on Numexo2 card frame.
    Author : Luc Legeard
*/

#include "MFMS3AlphaFrame.h"

# define MFM_S3_RUTH_FRAME_TYPE_TXT "MFM_S3_RUTH_FRAME_TYPE"
# define MFM_S3_RUTH_FRAME_TYPE_SHORT_TXT "S3Rut"
# define MFM_S3_RUTH_FRAME_TYPE_LONG_TXT "S3 Rutherford"
#pragma pack(push, 1) // force alignment


//____________MFMS3RuthFrame___________________________________________________________

class MFMS3RuthFrame:public MFMS3AlphaFrame
{
public:

MFMS3RuthFrame();
MFMS3RuthFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
virtual ~MFMS3RuthFrame();

 void      FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);
 const char * GetTypeText()const {return MFM_S3_RUTH_FRAME_TYPE_TXT;} 
 const char * GetTypeShortText()const {return MFM_S3_RUTH_FRAME_TYPE_SHORT_TXT;}
 const char * GetTypeLongText()const {return MFM_S3_RUTH_FRAME_TYPE_LONG_TXT;} 
};
#pragma pack(pop) // free aligment
#endif
