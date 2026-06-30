// File : GError.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GError
//
// This class manager Errors  
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


#ifndef __GError__
#define __GError__

#include <sstream>
#include <iostream>
#include <ostream>

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <TString.h>
#include <TObject.h>

//_________________________________________________________________________________________




class GError : public TObject{
  
 private:
  
  
 mutable int fVerbose;
 mutable int fNbError; // number of critical errors
 mutable int fNbCriticalError; // number of critical errors
 int fNbCriticalErrorMax; // number of critical errors
 mutable int fNbWarning; // number of warning
 mutable int fNbInfo; // number of infon
 mutable int fNbDebug; // number of debug
 public :
  GError();   // default constructor of GError object
  ~GError();   // destructor of GError
  virtual void Infos( const char *message,const char *message2=NULL, const char* comment=NULL)const; 
  virtual void Infos( char *message,TString *message2=NULL,    TString* comment=NULL)const;
  virtual void Infos( TString *message,   TString *message2=NULL,    TString* comment=NULL)const;
  virtual void Barre();
  virtual void Test(int coups)const;
  virtual void TreatError(int level,int status , const char *message, const char *message2=NULL, const char* comment =NULL)const;
  virtual void TreatError(int level,int status , TString *message,    TString    *message2=NULL, TString*    comment =NULL)const;
  virtual void TreatError(int level,int status , char *message,       TString    *message2=NULL, TString*    comment =NULL)const; 
  
  virtual void TreatDebug (int level,int status , const char *message, const char *message2=NULL, const char* comment =NULL)const;
  virtual void TreatDebug (int level,int status , TString *message,    TString    *message2=NULL, TString*    comment =NULL)const;
  virtual void TreatDebug (int level,int status , char *message, TString    *message2=NULL, TString*    comment =NULL)const;
  
  virtual void SetDebugVerbose (int level);
  
  virtual void SetVerbose(int  i=0){fVerbose =i;};

 ClassDef (GError ,1); // Manage Errors

};

#endif
