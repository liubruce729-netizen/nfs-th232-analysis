#ifndef _MFMBoxDiagFrame_
#define _MFMBoxDiagFrame_

/*
  MFMBoxDiagFrame.h

	Copyright Acquisition group, GANIL Caen, France

*/

#include "MFMNumExoFrame.h"

#define BOX_DIAG_NB_STATUS 2
#define BOX_DIAG_NB_CHANNELS 16
#define MFM_BOX_DIAG_FRAME_TYPE_TXT "MFM_BOX_DIAG_FRAME_TYPE"
#define MFM_BOX_DIAG_FRAME_TYPE_SHORT_TXT "DiagB"
#define MFM_BOX_DIAG_FRAME_TYPE_LONG_TXT "Box Diagnostic"
#include "MFMBlobFrame.h"

#pragma pack(push, 1) // force alignment

struct MFM_BoxDiag_data{
  unsigned CristalId : 16;
  unsigned Status1   : 16;
  unsigned Status2   : 16;
  unsigned TimeStop  : 16;
  unsigned Energy    : 16;
  unsigned Time      : 16;
  unsigned Checksum  : 16;
};

struct MFM_BoxDiag_frame{
	 MFM_common_header      Header;
	 MFM_numexo_eventInfo   EventInfo;
	 MFM_BoxDiag_data       Data;
};

//____________MFMBoxDiagFrame___________________________________________________________

class MFMBoxDiagFrame : public MFMNumExoFrame
{

public :
 void      SetStatus(int i, uint16_t status);
 uint16_t  GetStatus(int i)const ;
 void      SetTimeStop(uint16_t timestop);
 uint16_t  GetTimeStop()const;
 void      SetEnergy(uint16_t energy);
 uint16_t  GetEnergy()const;
 void      SetTime(uint16_t time);
 uint16_t  GetTime()const;
 void      SetUserDataPointer();
 string    GetHeaderDisplay(char* infotext) const;
 void      FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);
 const char * GetTypeText()const {return MFM_BOX_DIAG_FRAME_TYPE_TXT;} 
 const char * GetTypeShortText()const {return MFM_BOX_DIAG_FRAME_TYPE_SHORT_TXT;} 
 const char * GetTypeLongText()const {return MFM_BOX_DIAG_FRAME_TYPE_LONG_TXT;} 
};
#pragma pack(pop) // free aligment
#endif
