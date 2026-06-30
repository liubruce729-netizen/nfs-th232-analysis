// File : GFile.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GFile
//
// This class manage   files
// The associated methods can do copies,duplications, verifications, dumps
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

#ifndef __GFile__
#define __GFile__


#include <TObject.h>
#include <TFile.h>
#include <TString.h>
#include <TTree.h>
#include <TNtuple.h>
#include <TROOT.h>

#include "General.h"
#include "GDevice.h"
#include "GBufferIn2p3.h"
#include "TH1.h"
#include "TH2.h"
#include<zlib.h>

#include "GTtape_erreur.h"

//_____________________________________________________________
class GFile : public GDevice{

 protected:


  // bool Is_protected;    // Flag if the tape or file is protected again write.
  int fLun;             // Logical Unit Number. Decriptor of the opened device.(taoe)
  gzFile fLungz;//! Logical Unit Number. Decriptor of the opened device.( tape) with handle of compression
  int fCompressionLevel;// compression level (0...9)

  GBufferIn2p3 fBufferHeader[32]; //array to store the first infomations buffers of run
  int     fBufferHeadersize; // number of buffer
  int     fNewRunNumber;   // new run number in cas we want to change it during a copy ( file to file)

public:

  GFile ()  ;   // default constructor of GTtape object
  GFile (const char* _name);// constructor
  ~GFile() ;

 virtual  void GFileInit(const char *_Name);
 virtual  void Infos();
 virtual  void SetDevice(const char* Name1);
 virtual  void SetCompressionLevel(int level=4){fCompressionLevel=level;};
 virtual  void Skip (bool quiet=false);
 virtual  void Skip (char b,int nbr=1,bool quiet =false);
 virtual  void Skip (char* b,int nbr =1,bool quiet=false);
 virtual  void ReadBuffer ();
 virtual  void ReadBuffer(GBuffer& _Buffer);
 virtual  void ReadBuffer(GBuffer* _Buffer);
 virtual  void WriteBuffer(GBuffer& _Buffer);
 virtual  void WriteBuffer();
 virtual  void WriteBuffer(GBuffer* _Buffer);

 virtual  void GetPos(long* position);


 virtual  void Convert2DtoTxt();
 virtual  void Convert1DtoTxt();
 virtual  void ExtractHeaders(char* fileoutname);
 virtual  void Dir();
 virtual  void WhatType();
 virtual  void Eject();

 virtual  void SetNewRunNumber(int newrunnumber){ fNewRunNumber = newrunnumber ;};
 virtual  bool IsARun(bool quiet = false);
 virtual  bool IsARootFile(bool quiet= false);
 virtual  bool IsExiste();
 virtual  int  SizeRuns();
 virtual  int ReadRunNumDate(Int_t placeofbuffer,bool rewind=true);
 virtual  Int_t GetBufSizeFromFILEH(bool rewind=true);
// These following functions are  abstract in GDevice
 virtual  void Open(char mod = 'r');
 virtual  void Open(char* mod);
//virtual  TH1* GetSpectrum(const char *histoname=NULL,TH1* old_histo=NULL);
 virtual  TNamed* GetSpectrum(const char *spectrumname=NULL,TNamed* old_spectrum=NULL);
 virtual  TString* GetListSpectra();
 virtual  void Close();
 virtual  void Inquire(char* Exp_Nam = (char*)"");
 virtual  void Rewind(bool quiet=false);
protected:
 //------------------------
 int  MyRead(char * ptchar, int size);
 int  MyWrite(char * ptchar, int size);
 long long MyOpen(char* name , int mode);
 long long MyClose();

  void SkipBlock (int  NombreSkip);
  void SkipFile  (int  NombreSkip);
  void SkipEO    (int *NombreSkip);
  void SkipRewind();
  void IsTape    ();

  ClassDef (GFile ,1); // Tape or file device (inherits from GDevice)

};

#endif
