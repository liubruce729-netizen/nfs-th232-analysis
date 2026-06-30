#ifndef _MFMS3eGUNFrame_
#define _MFMS3eGUNFrame_
/*
  MFMS3eGUNFrame.h

	Copyright Acquisition group, GANIL Caen, France
	Class eGUN frame based on Numexo2 card frame.
    Author : Luc Legeard
*/

#include "MFMNumExoFrame.h"

#define MFM_S3_EGUN_FRAME_TYPE_TXT "MFM_S3_EGUN_FRAME_TYPE"
#define MFM_S3_EGUN_FRAME_TYPE_SHORT_TXT "S3Egu"
#define MFM_S3_EGUN_FRAME_TYPE_LONG_TXT "S3 EGUN"

#pragma pack(push, 1) // force alignment


//____________MFMS3eGUNFrame___________________________________________________________

class MFMS3eGUNFrame:public MFMNumExoFrame
{
public:

MFMS3eGUNFrame();
MFMS3eGUNFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize,int headerSize);
virtual ~MFMS3eGUNFrame();

 void      SetSetup(uint16_t i,uint16_t setup);
 uint16_t  GetSetup(uint16_t i);
 void      SetCup(uint16_t cup);
 uint16_t  GetCup();
 void      SetGrid(uint16_t grid);
 uint16_t  GetGrid();

 void      FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);
 const char * GetTypeText()const {return MFM_S3_EGUN_FRAME_TYPE_TXT;} 
 const char * GetTypeShortText()const {return MFM_S3_EGUN_FRAME_TYPE_SHORT_TXT;}
 const char * GetTypeLongText()const {return MFM_S3_EGUN_FRAME_TYPE_LONG_TXT;} 
};
#pragma pack(pop) // free aligment
#endif
