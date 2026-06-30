// File : GBase.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GBase
// GBase : base Class  of all GRU class
// 		contains utilities, common functions, etc
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

#ifndef __GBase__
#define __GBase__



#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <TString.h>
#include <TObject.h>
#include "General.h"
#include "GError.h"
#include "GEN_TYPE.H"
#include <TVirtualX.h>
#include <TSystem.h>
extern "C"
{
#include "gan_acq_buf.h"
}

using std::ostream;
//______________________________________________________________________________________

class GBase : public TObject{

 public :

  GError fError;

protected:
  int    fVerbose;               // level (0 to 10 ) of verbose default = 0  ( level 10 is  for debug)

 public :

  GBase();
  virtual ~GBase();
  virtual void SetVerbose(int  i=0){fVerbose =i;}
  virtual int  GetVerbose()const {return fVerbose;};
  virtual GError* GetError() {return &fError;};
  virtual bool CompareWordsIgnoreCase(char* word1,const char* word2);
  virtual bool CompareWordsIgnoreCase(TString* word1,const char* word2);
  virtual bool TestGruWord(char* word);
  virtual int  Slicer(char* inputwords,char*** commandsliced,int** commandsliced_int);
  virtual bool  ReplaceChar(char* chaine, char char1, char char2);
  virtual bool  ReplaceChar(TString* chaine, char char1, char char2);
  virtual char* RemoveWhite(char * input);
  virtual char* RemoveChar(char * input, char* toremove, bool frombegin);
  virtual int   WordsCount (char* words);
  virtual void  Test(int coups);
  virtual void GetDumpRaw  (void *point, int dumpsize, int increment=0,
  		string * mydump=NULL)const;
  virtual void GetDumpRaw2 (void *point, int dumpsize, int sizeframe, int increment=0,
  		string * mydump=NULL)const;
  virtual void GetDumpRawReal(void *point, int dumpsize, int increment,
  		  		string * mydump=NULL)const ;
  virtual void SwapInt32(UNSINT32 *Buf, int NbOctets);
  virtual void SwapInt16(UNSINT16 *Buf, int NbOctets);
  virtual void SwapInt64(long long *Buf,int NbOctets);
  virtual void WaitWithPoint (int nsec)const;
  virtual void WaitAChar()const ;
  virtual void RazZone(char* pt,int size,int start_size=0);
  virtual void ReallocBufferSize(char**pt,int newsize,int oldsize, bool ifinferior=true);
  virtual void ToDoInCaseOfInterrupt()=0;

  ClassDef (GBase ,1) // Base Class of GRU

};
#endif
