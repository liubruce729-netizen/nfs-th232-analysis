#ifndef _MFMParisFrame_
#define _MFMParisFrame_
/*
  MFMParisFrame.h

  Copyright Acquisition Group, GANIL Caen, France

*/


#include "MFMNumExoFrame.h"

#define MFM_PARIS_FRAME_TYPE_TXT "MFM_PARIS_FRAME_TYPE"
#define MFM_PARIS_FRAME_TYPE_SHORT_TXT "Paris"
#define MFM_PARIS_FRAME_TYPE_LONG_TXT "Paris"
#define PARIS_UNIT_BLOCK_SIZE 1
#define PARIS_FRAMESIZE 28
#pragma pack(push, 1) // force alignment

struct MFM_paris_data{
  unsigned LocationID: 16;
  unsigned QShort    : 16;
  unsigned QLong     : 16;
  unsigned Cfd       : 24;
  unsigned Flags     :  8;
};

struct MFM_paris_frame{
  MFM_common_header     Header;
  MFM_numexo_eventInfo  EventInfo;
  MFM_paris_data        Data;
};

//____________MFMParisFrame___________________________________________________________

class MFMParisFrame:public MFMNumExoFrame
{


 public :

  MFMParisFrame();
  MFMParisFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize,int headerSize);
  virtual ~MFMParisFrame();
  char*   MyClassName()const{ return (char*)"MFMParisFrame";};

  // PARIS
   void      SetQShort(uint16_t energy);
   uint16_t  GetQShort() const;
   void      SetQLong(uint16_t energy);
   uint16_t  GetQLong()const ;
   void      SetCfd(float cfd);
   float     GetCfd()const;
   void      FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);
   string    GetHeaderDisplay(char* infotext) const; 

  virtual bool      GetPLL() const;
  virtual bool      GetPUR() const;
  virtual bool      GetOVR() const;

  const char * GetTypeText()const             {return MFM_PARIS_FRAME_TYPE_TXT;} 
  const char * GetTypeShortText()const        {return MFM_PARIS_FRAME_TYPE_SHORT_TXT;} 
  const char * GetTypeLongText()const        {return MFM_PARIS_FRAME_TYPE_LONG_TXT;}
  virtual int  GetDefinedUnitBlockSize()const {return PARIS_UNIT_BLOCK_SIZE ;};
  virtual int  GetDefinedHeaderSize()const    {return NUMEXO_HEADERFRAMESIZE;};
  int  GetDefinedFrameSize()const     {return PARIS_FRAMESIZE;}; 
  uint16_t GetChecksum()const{ return 0;}
  bool IsSame(MFMParisFrame* testframe,int verbose);
  bool UnitTest(int verbose);
};
#pragma pack(pop) // free aligment
#endif

