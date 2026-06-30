// File : GTape.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GTape
//
// This class manage tapes and files in Ganil format
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

#ifndef __GTape__
#define __GTape__


#define D2000 0x19 // DLT 2000
#define D2001 0x1a // DLT 2000 compressed
#define D2010 0x80 // DLT 2000 15G
#define D2011 0x81 // DLT 2000 15G compressed
#define D4000 0x82 // DLT 4000
#define D4001 0x83 // DLT 4000 compressed
#define D7000 0x84 // DLT 7000
#define D7001 0x85 // DLT 7000 compressed
#define D8000 0x88 // DLT 8000
#define D8001 0x89 // DLT 8000 compressed



#include <TObject.h>
#include <TFile.h>
#include <TString.h>
#include <TTree.h>
#include <TNtuple.h>
#include <TROOT.h>

#include "General.h"
#include "GDevice.h"
#include "GFile.h"
#include "GBufferIn2p3.h"
#include "TH1.h"
#include "TH2.h"
#include<zlib.h>

#include "GTtape_erreur.h"

//_____________________________________________________________
class GTape : public GFile{

 protected:


  // bool Is_protected;    // Flag if the tape or file is protected again write.
  int fLun;             // Logical Unit Number. Decriptor of the opened device.(taoe)
  gzFile fLungz;//! Logical Unit Number. Decriptor of the opened device.( tape) with handle of compression
  int fDensity; // Density of the tape
  int fCompressionLevel;// compression level (0...9)

  GBufferIn2p3 fBufferHeader[32]; //array to store the first infomations buffers of run
  int     fBufferHeadersize; // number of buffer
  int     fNewRunNumber;   // new run number in cas we want to change it during a copy ( file to file)

public:

  GTape ()  ;   // default constructor of GTtape object
  GTape (const char* _name  ,const int _density = DEFAULT_DENSITY );// constructor
  ~GTape() ;

 virtual  void GTapeInit(const char *_Name, int _Density);
 virtual  void Infos();
 virtual  void SetDevice(const char* Name1);
 virtual  void SetCompressionLevel(int level=4){fCompressionLevel=level;};
 virtual  void Skip (bool quiet=false);
 virtual  void Skip(char b,int nbr=1,bool quiet =false);
 virtual  void Skip(char* b,int nbr =1,bool quiet=false);
 virtual  void ReadBuffer ();
 virtual  void ReadBuffer(GBuffer& _Buffer);
 virtual  void ReadBuffer(GBuffer* _Buffer);
 virtual  void WriteBuffer(GBuffer& _Buffer);
 virtual  void WriteBuffer();
 virtual  void WriteBuffer(GBuffer* _Buffer);

 virtual  int  GetDensity();
;

 virtual  void Convert2DtoTxt();
 virtual  void Convert1DtoTxt();
 virtual  void GetRun(GTape* Origin_device,int FirstFile=0,int NbrFiles=0);
 virtual  void ExtractHeaders(char* fileoutname);
 virtual  void Dir();
 virtual  void WhatType();
 virtual  void Eject();

 virtual  void SetDensity(int density);
 virtual  void SetNewRunNumber(int newrunnumber){ fNewRunNumber = newrunnumber ;};
 virtual  bool IsARun(bool quiet = false);
 virtual  bool IsARootFile(bool quiet= false);
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
 virtual  void GetPos(long* position);
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

  ClassDef (GTape ,1); // Tape or file device (inherits from GDevice)

};

#endif
