// File : GBufferMFM.h
// Author: Luc Legeard  (winter 2004)
//////////////////////////////////////////////////////////////////////////////
//
// Class GBufferMFM
//
// This class manger Ganil Buffer
//
/////////////////////////////////////////////////////////////////////////////


// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#ifndef __GBufferMFM__
#define __GBufferMFM__

// Size of different element lot of buffer


#include "GEN_TYPE.H"
#include "General.h"
#include "GBuffer.h"
#include "MFMCommonFrame.h"

extern "C"
{
#include "gan_acq_buf.h"
}

//______________________________________________________________________________________

class GBufferMFM : public GBuffer{

 public :

  char   fGBuf_header[MFM_BLOB_HEADER_SIZE];// header of buffer

private:

  MFMCommonFrame * pFrame;
 public :

  GBufferMFM(int _bufsize =0);
  ~GBufferMFM();

  virtual  int   GetNumRun_(){return 0;};
  virtual  char *GetDate_(){ return ((char*) "nodate");};

virtual  int  GetBufSizeFromBuffer();
virtual  Int_t TestType(char * pt =NULL);
virtual  int   GetHeaderSize();
virtual  void  Equal(GBuffer& buf1);
virtual  void  Equal(GBuffer* buf1);
virtual  void  MyDelete();
virtual  void  SetAttributs(bool quiet = false);

virtual  bool  IsAMFMBuffer();
virtual   MFMCommonFrame* GetFrame(){return pFrame;};
unsigned int  fUsedEventsSize; //Used size by Event in bytes of the buffer during filling
virtual void SetExternalDataZone(void* pt,int size) ;
virtual unsigned int GetUsedEventsSize(){return fUsedEventsSize;};

virtual void MakeMFMHeader(UInt_t blockNumber, Short_t sourceId, Short_t destinationId, Short_t dataStreamNumber, Short_t numOfEvents);
virtual TString GetDumpBufferHeader(); // Do a dump of header of a MFM Buffer
virtual void DumpEventBufferData();// Do a dump of header of a MFM Buffer
virtual void SetBufSize(int size);
virtual TString GetDumpBuffer(int dumpsize=256, int increment=-1);

ClassDef (GBufferMFM,1) // In2P3 Buffer

};
#endif
