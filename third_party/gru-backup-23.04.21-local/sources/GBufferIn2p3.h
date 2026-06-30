// File : GBufferIn2p3.h
// Author: Luc Legeard  (winter 2004)
//////////////////////////////////////////////////////////////////////////////
//
// Class GBufferIn2p3
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

#ifndef __GBufferIn2p3__
#define __GBufferIn2p3__

// Size of different element lot of buffer
#define IN2P3_HEADER_SIZE     8
#define COUNT_SIZE      4
#define ASCII_INFO_SIZE 80

#include "GEN_TYPE.H"
#include "GBuffer.h"


extern "C"
{
#include "gan_acq_buf.h"
}

//______________________________________________________________________________________



class GBufferIn2p3 : public GBuffer{


 public :
   char   fGBuf_header[IN2P3_HEADER_SIZE+1];// header of buffer copy of 8 first char of buffer

  GBufferIn2p3(int _bufsize = BUFSIZE);
  ~GBufferIn2p3();
virtual  in2p3_buffer_struct  *GetBuffer_map_in2p3(){return((in2p3_buffer_struct*) fGBuf_data);};
virtual  int    GetBufSizeFromFILEH();
virtual  int    GetHeaderSize();
virtual  int    GetNumBuf(); // get last number of current buffer
virtual  char*  GetBufHeader() {return fGBuf_header;};
virtual TString GetDumpBufferHeader(); // Do a dump of header of a EBYEADT Buffer
virtual void    Equal(GBuffer& buf1);
virtual void    Equal(GBuffer* buf1);
virtual void    MyDelete() ;
virtual Int_t   TestType(char * pt);

// no verified
virtual void  SetAttributs(bool quiet = false);
virtual char* GetDate_();
virtual int   GetNumRun_();
virtual void  SetToCommentBuffer();
virtual void  ChangeRunNumber(int newnumber);
virtual int   GetBufSizeFromBuffer();
virtual bool  IsAIn2p3Buffer();
virtual void  MakeEBYEDATHeader(UInt_t blockNumber, Short_t sourceId, Short_t destinationId, Short_t dataStreamNumber, Short_t numOfEvents);
virtual void  MakeEndRunHeader(UInt_t blockNumber);
  ClassDef (GBufferIn2p3 ,1) // In2P3 Buffer

};
#endif
