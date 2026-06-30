// File : GMFMFile.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class
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

#ifndef ____
#define ____

#include <TObject.h>
#include <TFile.h>
#include <TString.h>
#include <TTree.h>
#include <TNtuple.h>
#include <TROOT.h>
#include "General.h"
#include "GDevice.h"
#include "GBufferMFM.h"
#include "GFile.h"
#include "TH1.h"
#include "TH2.h"
#include<zlib.h>



//_____________________________________________________________
class  GMFMFile : public GFile{

 protected:


  // bool Is_protected;    // Flag if the tape or file is protected again write.


public:

  GMFMFile ()  ;   // default constructor of GTtape object
  GMFMFile (const char* _name );// constructor
  ~GMFMFile() ;

 virtual  void Init(const char *_Name);
 virtual  void SetDevice(const char* Name1);
 virtual  void SetCompressionLevel(int level=6){fCompressionLevel=level;};
 virtual  void Skip (bool quiet=false);
 virtual  void Skip(char b,int nbr=1,bool quiet =false);
 virtual  void Skip(char* b,int nbr =1,bool quiet=false);
 virtual  void ReadBuffer ();
 virtual  void ReadBuffer(GBuffer& _Buffer);
 virtual  void ReadBuffer(GBuffer* _Buffer);
 virtual  void WriteBuffer(GBuffer& _Buffer);
 virtual  void WriteBuffer(GBuffer* _Buffer);
 virtual  void WriteBuffer();
 //virtual  void GetPos(long* position);
 virtual  void WhatType();
 virtual  void IsATape();
 virtual  bool IsARun(bool quiet);


 //virtual  int  TestMFM();
 virtual GBuffer* GetBuffer()       {return fBuffer; };

// These following functions are  abstract in GDevice
 virtual  void Open(char mod = 'r');
 virtual  void Open(char* mod);

 virtual  void Rewind(bool quiet=false);
private:
 //------------------------
 int  MyRead(char * ptchar, int size);
 int  MyWrite(char * ptchar, int size);
  long long int MyOpen(char* name , int mode);
  int MyClose();

  void SkipBlock (int  NombreSkip);
  void SkipFile  (int  NombreSkip);
  void SkipEO    (int *NombreSkip);
  void SkipRewind();

  ClassDef ( GMFMFile,1); // Tape or file device (inherits from GDevice)

};

#endif
