#ifndef MFMMESYTECMDPPFRAME_H
#define MFMMESYTECMDPPFRAME_H

#define MFM_MESYTEC_FRAME_TYPE_TXT "MFM_MESYTEC_FRAME_TYPE"
#define MFM_MESYTEC_FRAME_TYPE_SHORT_TXT "Mesyt"
#define MFM_MESYTEC_FRAME_TYPE_LONG_TXT "MESYTEC"
#define MFM_MESYTEC_UNIT_BLOCK_SIZE 2
#define MFM_MESYTEC_HEADER_SIZE 24
#define MESYTEC_USERSIZE 20  // just for test in case of random values
#define MESYTEC_FRAMESIZE  MESYTEC_USERSIZE + MFM_MESYTEC_HEADER_SIZE

#include "MFMBlobFrame.h"

#pragma pack(push, 1) // force alignment


struct MFM_Mesytec_EventInfo {
   uint64_t timestamp : 48;
   uint32_t eventnumber;
   uint16_t reserved;
   uint32_t eventsize;
};

struct MFM_Mesytec_Data {
	char test[MESYTEC_USERSIZE];
};

struct MFM_Mesytec_frame{
   MFM_common_header Header;
   MFM_Mesytec_EventInfo EventInfo;
   MFM_Mesytec_Data Data;
};

//! \class MFMMesytecFrame
//!
//! Used for reading data from Mesytec DAQ
//!

class MFMMesytecFrame : public MFMBlobFrame
{    
    uint32_t eventsize; // size of Mesytec data buffer in bytes

public:
   MFMMesytecFrame();
   virtual ~MFMMesytecFrame();

   void SetAttributs(void *pt = NULL);
   void SetUserDataPointer();
   void SetTimeStampFromMesytecFrameData();
   void SetTimeStamp(uint64_t t);
   void SetEventNumberFromMesytecFrameData();
   void SetEventNumber(uint32_t e);
   bool HasTimeStamp() const {return true; }
   bool HasEventNumber() const {return true; }
   


   void SetEventSizeFromFrameData();
   uint32_t GetEventSize() const { return eventsize; }
   void FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);

   const char * GetTypeText()const {return MFM_MESYTEC_FRAME_TYPE_TXT;}
   const char * GetTypeShortText()const {return MFM_MESYTEC_FRAME_TYPE_SHORT_TXT;}
   const char * GetTypeLongText()const {return MFM_MESYTEC_FRAME_TYPE_LONG_TXT;}
   int GetDefinedUnitBlockSize()const {return MFM_MESYTEC_UNIT_BLOCK_SIZE;};
   int GetDefinedHeaderSize()const {return MFM_MESYTEC_HEADER_SIZE;};
   int GetDefinedFrameSize()const {return MESYTEC_FRAMESIZE;};
};
#pragma pack(pop) // free aligment

// backwards compatibility
using MFMMesytecMDPPFrame = MFMMesytecFrame;

#endif // MFMMESYTECMDPPFRAME_H
