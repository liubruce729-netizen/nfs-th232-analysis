#ifndef MFMFAZIAFRAME_H
#define MFMFAZIAFRAME_H

#include "MFMBlobFrame.h"

#define FAZIA_HEADERFRAMESIZE 24
#define MFM_FAZIA_FRAME_TYPE_TXT "MFM_FAZIA_FRAME_TYPE"
#define MFM_FAZIA_FRAME_TYPE_SHORT_TXT "Fazia"
#define MFM_FAZIA_FRAME_TYPE_LONG_TXT  "Fazia"
#define FAZIA_USERSIZE   24   // fully ramdom value, just for write test
#define FAZIA_FRAMESIZE  FAZIA_HEADERFRAMESIZE + FAZIA_USERSIZE
#define  MFM_FAZIA_UNIT_BLOCK_SIZE 1

#pragma pack(push, 1) // force alignment

struct MFM_Fazia_EventInfo {
   unsigned char timestamp[6];
   unsigned eventnumber  : 32;
   unsigned reserved : 16;
   unsigned eventsize : 32;
};

struct MFM_Fazia_Data {
	char test[FAZIA_USERSIZE];
};

struct MFM_Fazia_Header {
   MFM_common_header Header;
   MFM_Fazia_EventInfo EventInfo;
   MFM_Fazia_Data Data;
};

//! \class MFMFaziaFrame
//!
//! Used for reading FAZIA frames
//!
//! In order to parse the data in the frame, use the MFMFaziaFrame::GetEventSize()
//! method in order to know the size of the event. E.g. if reading a file with
//! MFMFileReader:
//!
//!    DAQ::FzEventSet evset;
//!    MFMFileReader rdr([filename]);
//!    while( rdr.ReadNextFrame() ){
//!     if( rdr.GetReadFrameClass() == "MFMFaziaFrame" ){
//!       if( evset.ParseFromArray( rdr.GetReadFrame().GetPointUserData(), rdr.GetFrameRead<MFMFaziaFrame>().GetEventSize() ) ){
//!              // OK

class MFMFaziaFrame : public MFMBlobFrame {
   u_int32_t eventsize; // value of DAQ::FzEvent(Set):ByteSize()

public:
   MFMFaziaFrame() ;
   virtual ~MFMFaziaFrame() ;

  // void SetAttributs(void* pt = NULL);
   void SetUserDataPointer();
   void SetTimeStampFromFaziaFrameData();
   void SetTimeStamp(uint64_t t);
   void SetEventNumberFromFaziaFrameData();
   void SetEventNumber(uint32_t e);
   
   int GetDefinedUnitBlockSize()const {return MFM_FAZIA_UNIT_BLOCK_SIZE;};
   int GetDefinedHeaderSize()const {return FAZIA_HEADERFRAMESIZE;};
   int GetDefinedFrameSize()const {return FAZIA_FRAMESIZE;};
   
   void      FillDataWithRamdomValue(uint64_t timestamp,uint32_t eventnumber);
   const char * GetTypeText()const {return MFM_FAZIA_FRAME_TYPE_TXT;}
   const char * GetTypeShortText()const {return MFM_FAZIA_FRAME_TYPE_SHORT_TXT;}
   const char * GetTypeLongLong()const {return MFM_FAZIA_FRAME_TYPE_LONG_TXT;}
   bool HasTimeStamp() const
   {
      return true;
   }
   bool HasEventNumber() const
   {
      return true;
   }
 
   void SetEventSizeFromFrameData();
   u_int32_t GetEventSize() const
   {
      return eventsize;
   }
};
#pragma pack(pop) // free aligment

#endif // MFMFAZIAFRAME_H
