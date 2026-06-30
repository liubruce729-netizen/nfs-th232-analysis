// File : GDevice.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GDevice
//
// This class manger tape and file in Ganil format
// The associated methods can do copies,duplications, verifications, dumps...
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


#ifndef __GDevice__
#define __GDevice__

#include <sstream>
using std::ostream;
#include <TThread.h>
#include <TH1.h>
#include "General.h"
#include "GBuffer.h"
#include "GBase.h"

//_________________________________________________________________________________________


class GDevice : public GBase{

private:

 protected:
 GBuffer* fBuffer ;    // current internal buffer
 Int_t   fPort; // Port Number in case of Net device , else not used

  char   fName[MAX_CARACTERES]; // Name of the device ( ex : /dev/nstO , /home/user/run.dat or ganp500.in2p3.fr)

  int    fType;                  // = GT_TYPE_TAPE , GT_TYPE_FILE , GT_TYPE_DIR, GA_CLIENT_NET
  int    fBufferSize;     // Size of buffer
  bool   fIsOpen;        // flap open=1  or not =0
  bool   fWriteOrRead ;  // Flag  giving the open context ( true = write mode , false = read mode)
  int    fStatus; // Internal status ( to have a satus whithout a returned status which noise the standard output of Cint )
  bool   fBusy;  //  indicate if the device is busy or not.t is used in methode witgh are

  Int_t   fRunNumber;     // current file run number
  char    fDate[MAX_CARACTERES];  // current date of run
  Bool_t  fStoreOn ;  // bool to know if Ganil Acquisiton are storing


 public:

  GDevice() ;   // default constructor of GDevice object
  GDevice (const char* _name);
  ~GDevice() ;


  virtual void     Init();
  virtual void     Infos();

  virtual int      GetType()         {return fType;}
  virtual int      GetStatus()       {return fStatus;}
  virtual char*    GetDeviceName()   {return fName;}
  virtual Int_t    GetPort()         {return fPort;}
  virtual  bool    GetIsOpen()       {return fIsOpen;}
  virtual  bool    GetStoreOn()      {return fStoreOn;}

  virtual  bool    GetBusy()         {return fBusy;}
  virtual Int_t    GetRunNumber()    const {return fRunNumber; };
  virtual Int_t    GetBufferSize()   {return fBufferSize; };
  virtual GBuffer* GetBuffer()       {return fBuffer; };


  virtual  void    SetDevice( const char *Name1);
  virtual  void    SetStatus(int _Status){fStatus =_Status;}
  virtual  void    SetPort(int port);
  virtual  void    SetBusy  (bool bus){fBusy = bus;}
  virtual  void    SetBufferSize  (Int_t size);

  virtual  void    DumpBuffer(char c = 'b');
  virtual  void    DumpBuffer(char* c);
  virtual  void    ReadBuffer(GBuffer& _Buffer);

// All these methods are abstract
 virtual   void    Rewind(bool quiet=false )    =0;
 virtual   void    Inquire(char* Exp_Name =(char*)"")=0;
 virtual   void    Close()=0;
 virtual   void    Open(char mod ='r') =0;
 virtual   void    Open(char* mod)=0;
 virtual   void    ReadBuffer()  =0;
 virtual   void    WriteBuffer(GBuffer* _Buffer)  =0;

 virtual   TString *GetListSpectra()=0;// get a Liste name of histogram
 virtual   TNamed*  GetSpectrum(const char* histoname=NULL,TNamed* old_spectrum=NULL)=0; // get a histogram


 protected:
  bool        GetRunStatus();
  static void ThreadMethod (void* arg); // methode pour une acquisition a un seul thread: appel de 'Read' et traitement du buffer recupere
 private:
  virtual void ToDoInCaseOfInterrupt(){};
 public:

  //***AJOUTS JOHN***********************************//
  void SetRunNumber(Int_t _run) { fRunNumber = _run; };
  GBuffer* GetCurrentBuffer() { return fBuffer; };
  //*************************************************//

  ClassDef (GDevice ,1); // Device to get GBuffer

};

#endif
