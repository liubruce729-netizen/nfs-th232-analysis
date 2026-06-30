// File : GNetClientSoap.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GNetClientSoap
//
// Network Client for GNetServerSoap Class
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


#ifndef __GNetClientSoap__
#define __GNetClientSoap__

#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <iomanip>
//#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#include <sstream>
using std::ostream;

#include <TObject.h>
#include <TFile.h>
#include <TString.h>
#include <TNtuple.h>
#include <TROOT.h>

#include <TSocket.h>
#include <TMessage.h>

#include "General.h"
#include "GDevice.h"
#include "GNetClient.h"
#include "GBuffer.h"
#include "GSpectrumIdentity.h"
#include "GSpectraDB.h"
#include "MFMOscilloFrame.h"
#include "GBufferMFM.h"

extern "C"
{
#include "GTtape_erreur.h"
#include "acq_tcp_struct.h"
#include "acq_codes_erreur.h"
}

//_________________________________________________________________________________________
class GNetClientSoap : public GNetClient {

 private:

 char str[1024];
 int fPortDef ;   // Default Port number for tcpip communication
 Int_t fOld_port;
 TString fOld_server;
 struct timeval fMt_permitclose;
 struct timeval fMt_reference;
 struct timezone fTz;
 Int_t           fTempo;// duration  in second during the device stay open.

 int   fSizeZoneData;
 char* fZoneData_char ; //! pointer on the begin SC buffer
 Int_t fTaille_Retour;
 Int_t fComptNotAvailable;
 Int_t fComptbuffOK;
 bool  fSCBufferEndReached;
 char* fZoneData_cur_char;//!
 int*  fZoneData_cur_int;//!
 Int_t fTimestampsbuffersize;
 Int_t fSize_SC_buffer;
 Int_t fNb_Buffers_Local;
 Int_t fPremier_Buffer;
 Int_t fNb_Buffers_Recus;
 Int_t fNBuffers_Envoyes;
 Int_t fLastNumBuff;
 GBufferMFM   * fBufferMFM;

 public:
  GNetClientSoap(char * host =(char*)"localhost");
 ~GNetClientSoap();

 virtual void SpeSave(char* filename);


 // These following methodes are  abstract in GDevice

 virtual   TNamed* GetSpectrum(const char *spectrumname,TNamed* old_spectrum=NULL);

 virtual   void  GetSpectrum(GSpectrumIdentity* id);
 virtual   void  Open(char mod = 'r') ;
 virtual   void  Open(char* mod);
 virtual   Int_t TestPortFree(Int_t port,char* name);
 virtual   void  Close ();

 virtual   void	ReadBuffer();// recuperation d'un buffer de donnees brutes

 virtual   void GetBuffer(bool afficher);
 virtual   bool GetBufferSC(bool afficher);

 virtual   void  Inquire(char* Exp_Name ) {cout<<" This Inquire do nothing\n";}
 virtual   void  Rewind(bool quiet=false)                 {cout<<" This Rewind do nothing\n"; }
 virtual   void  WriteBuffer(GBuffer* buffer ) {cout<<" This WriteBuffer do nothing\n";}

 virtual   bool  StartSpectraServer(int port=9092);

 virtual   bool  SendCommand0( char* command);
 virtual   TString* SendCommand1( char* command);

 virtual   GSpectraDB* GetSpectraDB() ;
 virtual   TNamed* GetSpectrum(const char *spectrumname, const char *family,TNamed *old_sp);
 //virtual   bool	 EndOfPage();

private:
 void InitClient();

 ClassDef ( GNetClientSoap,1); // Nerwork ROOT Client to Receive spectra
};

#endif
