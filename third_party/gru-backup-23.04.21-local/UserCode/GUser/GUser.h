// File :  GUser.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GUser
//
// This class manage user methodes
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


#ifndef __GUser__
#define __GUser__

#include <sstream>
using std::ostream;

#include <TObject.h>
#include "General.h"
#include "GAcq.h"
#include "GDevice.h"
#include "TH1.h"



//_________________________________________________________________________________________

class GUser : public  GAcq{

 
 protected:
  int fVerbose;               // level (0 to 10 ) of verbose default = 0  ( level 10 is good for debug)        
  

 public:


  GUser(GDevice* _fDevIn= NULL, GDevice* _fDevOut= NULL) ;   // default constructor of GUser object 
  ~GUser() ; 
  
 
  virtual void InitUser();
  virtual void InitUserRun();
  virtual void User();
  virtual void EndUserRun();
  virtual void EndUser();
  virtual void InitTTreeUser(); 
  ClassDef (GUser ,1); // User Treatment of Data
  
};

#endif
 
