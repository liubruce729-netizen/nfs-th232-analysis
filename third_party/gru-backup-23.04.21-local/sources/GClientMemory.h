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


#ifndef __GClientMemory__
#define __GClientMemory__
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <iomanip>
//#include <string.h>

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
class GClientMemory : public GDevice {

 private:


GSpectraDB *pSpectraDBServer;

 struct timeval fMt_permitclose;
 struct timeval fMt_reference;
 struct timezone fTz;
 Int_t           fTempo;// duration  in second during the device stay open.

 public:
  GClientMemory(char * host =(char*)"localhost");
 ~GClientMemory();

 virtual void SpeSave(char* filename);
 virtual bool TestServer();
 virtual void ResetSpectrumOnServer(GSpectrumIdentity* id);

 // These following methodes are  abstract in GDevice
 virtual   TString* GetListSpectra();// get a Liste name of histogram

// virtual   TH1* GetSpectrum(const char *histoname,TH1* old_histo=NULL);
 virtual   TNamed* GetSpectrum(const char *spectrumname,TNamed* old_spectrum=NULL);
 virtual   TNamed* GetSpectrum(const char *spectrumname, const char *family,TNamed *old_sp);
 virtual   void GetSpectrum(GSpectrumIdentity* id);
 virtual   void Open(char mod = 'r') ;
 virtual   void Open(char* mod);

 virtual   void Close ();
 virtual   void Close (Int_t tempo);
 virtual   void MyClose (Int_t tempo=0);
 virtual   void ReadBuffer()                  {cout<<" This Read do nothing\n";   }; // recuperation d'un buffer de donnees brutes
 virtual   void WriteBuffer(GBuffer* buffer ) {cout<<" This WriteBuffer do nothing\n";}
 virtual   void Inquire(char* Exp_Name ) {cout<<" This Inquire do nothing on "<<Exp_Name<<"\n";}
 virtual   void Rewind(bool quiet=false)  {(quiet=true);cout<<" This Rewind do nothing"<< quiet<<"\n";; }

 GSpectraDB* GetSpectraDB();// get a Liste name of spectra
 void InitClient(GSpectraDB * pspectra);

private:


 ClassDef (GClientMemory,1); // Nerwork ROOT Client to Receive spectra
};

#endif
