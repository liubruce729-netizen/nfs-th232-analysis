// File : GNetClient.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GNetClient
//
// Network Client abstract class
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


#ifndef __GNetClient__
#define __GNetClient__
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <iomanip>
//#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sstream>
using std::ostream;

#include <TObject.h>
#include <TFile.h>
#include <TString.h>
#include <TNtuple.h>
#include <TROOT.h>



#include "General.h"
#include "GDevice.h"
#include "GBuffer.h"
#include "GSpectrumIdentity.h"
#include "GSpectraDB.h"

extern "C"
{
#include "acq_tcp_struct.h"
#include "acq_codes_erreur.h"
}

//_________________________________________________________________________________________
class GNetClient : public GDevice {

 private:

  char fStr[1024];
  int fPortDef ;   // Default Port number for tcpip communication

 public:
  GNetClient(char * host =(char*)"localhost");
 ~GNetClient();

 void InitClient();

 virtual   bool SendCommand(const char* command,TString* stretour=NULL);
 virtual   TObject * SendCommand(const char* command,TObject * obj);

 virtual  bool SendCommand0(char* command) =0;
 virtual  TString* SendCommand1(char* command)=0;

 virtual   bool	EndOfPage();

 virtual   void ResetSpectrumOnServer(GSpectrumIdentity* id);
 virtual   void ResetAllSpectraOnServer(GSpectrumIdentity* id);
 virtual   void Open(char mod = 'r') =0;
 virtual   void Open(char* mod)=0;
 virtual   void Close ()=0;
 virtual   bool TestServer(Int_t time =0) ;
 virtual   TString* RequestInfo();
 virtual   TString* GetPid();
 virtual   TString* GetListSpectra() ;
 virtual   TString* GetInfo() ;
 virtual   bool InitInput(char* ip, int port);
 virtual   bool InitInputFile(char* name) ;
 virtual   bool InitGUser(int mode) ;
 virtual   bool InitEvent(char* name) ;
 virtual   bool SetBuffer(int size) ;
 virtual   bool StartRawSpectra() ;
 virtual   bool SpectraSave(char *name) ;
 virtual   bool Start() ;
 virtual   bool Pause(int time=-1) ;
 virtual   bool Stop();
 virtual   bool Finish() ;
 virtual   TNamed* GetSpectrum(const char *spectrumname, const char *family,TNamed *old_sp)=0;
 virtual   TNamed* GetSpectrum(const char *spectrumname,TNamed *old_sp)=0;
 virtual   GSpectraDB* GetSpectraDB()=0;
private:

 ClassDef ( GNetClient,1); // Nerwork ROOT Client to Receive spectra
};

#endif
