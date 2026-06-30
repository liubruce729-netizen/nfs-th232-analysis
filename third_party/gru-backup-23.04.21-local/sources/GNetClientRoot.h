// File : GNetClientRoot.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GNetClientRoot
//
// Network Client for GNetServerRoot Class
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


#ifndef __GNetClientRoot__
#define __GNetClientRoot__
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <iomanip>
//#include <string.h>

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
#include <TSocket.h>
#include <TMessage.h>


#include "GNetClient.h"


//_________________________________________________________________________________________
class GNetClientRoot : public GNetClient {

 private:

  TSocket *fSock;
  char fStr[1024];
  int fPortDef ;   // Default Port number for tcpip communication
 Int_t fOld_port;
 TString fOld_server;
 struct timeval fMt_permitclose;
 struct timeval fMt_reference;
 struct timezone fTz;
 Int_t           fTempo;// duration  in second during the device stay open.

 public:
  GNetClientRoot(char * host =(char*)"localhost");
 ~GNetClientRoot();


 virtual void SpeSave(char* filename);


 // These following methodes are  abstract in GDevice
 virtual   TString* GetListSpectra();// get a Liste name of histogram
 virtual   TNamed* GetSpectrum(const char *spectrumname,TNamed* old_spectrum=NULL);
 virtual   TNamed* GetSpectrum(const char *spectrumname, const char *family,TNamed *old_sp=NULL);
 virtual   void  SendSpectrum(GSpectrumIdentity* id);
 virtual   void GetSpectrum(GSpectrumIdentity* id);
 virtual   void Open(char mod = 'r') ;
 virtual   void Open(char* mod);
 virtual   Int_t TestPortFree(Int_t port,char* name);
 virtual   void Close ();
 virtual   void Close (Int_t tempo);
 virtual   void MyClose (Int_t tempo=0);
 virtual   void ReadBuffer()  {cout<<" This Read do nothing\n";   }; // recuperation d'un buffer de donnees brutes
 virtual   void Inquire(char* Exp_Name ) {cout<<" This Inquire do nothing on "<<Exp_Name<<"\n";}
 virtual   void Rewind(bool quiet=false)  {cout<<" This Rewind do nothing"<< quiet<<"\n";; }
 virtual   void WriteBuffer(GBuffer* buffer ) {cout<<" This WriteBuffer do nothing\n";cout << buffer<<endl;}
 virtual   bool SendCommand0( char* command);
 virtual   TString* SendCommand1( char* command);

 GSpectraDB* GetSpectraDB();// get a Liste name of spectra

private:

virtual void GiveObject(TSocket *localsock,TObject * obj);
virtual void GiveWords (TSocket *localsock,TString * words);
virtual int  ReceiveWords(TSocket *localsock,TString *recp);
virtual TObject* ReceiveObject(TSocket *localsock);
void InitClient();
 ClassDef ( GNetClientRoot,1); // Nerwork ROOT Client to Receive spectra
};

#endif
