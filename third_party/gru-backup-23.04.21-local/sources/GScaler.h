// $Id: GScaler.h,v 1.2 2001/07/07 19:19:25 patois Exp $
// Author: $Author: patois /legeard$
//*************************************************************************
//                             -------------------
//    begin                : Thu Jun 14 2001
//    copyright            : (C) 2001 by Garp
//    email                : patois@ganil.fr legeard@ganil.fr
//////////////////////////////////////////////////////////////////////////
//
// GScaler
//
// Scaler class for scaler .
//
//////////////////////////////////////////////////////////////////////////

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __GScaler_H
#define __GScaler_H

#include <TObject.h>
#include <TFile.h>
#include <TTree.h>
#include "GScalerEvent.h"
#include "General.h"
#include "GBase.h"
#include "GBufferIn2p3.h"
//#ifndef __GanAcqBuf_H
extern "C"
{
#include "gan_acq_buf.h"
  //typedef struct SCALE {int bidon;} scale;
}
//#endif

class GScaler: public GBase
{

 protected:

  Int_t fNbChannel;           // Number of individual scales
  GScalerEvent *fScalerArray; // [fNbChannel] Array of scalers
  TTree   *theScalerTree;        //! Root tree  where the Ganil Scalers data will be converted and strored
  TFile   *theScalerTreeFile;    //! File for Scalers TTree theTree

 private:

  bool fReinitTree;   // Reinitialisation of run for each run
  bool fInitTreeNeverDone; // true if init tree is not done
  bool fTTreeWithRunNumber;   // TRee will contain run number in its file name
  int fScalerMode;  // =1 enable Scaler treatment
 char fNameScalerTreeFile[MAX_CARACTERES];   //! name of Scalers TTree file

 public:



  GScaler       (void);
  virtual ~GScaler      (void);

  virtual  char* GetNameScalerTreeFile(){ return fNameScalerTreeFile;}
  virtual  int  GetScalerMode (){return fScalerMode;}
  virtual  void GetScalerEvents(GBufferIn2p3 *_buffer, UInt_t nbevent =0,int runnum =0);
  virtual  void ScalerTreatement(GBufferIn2p3 *_buffer,  UInt_t nbevent =0,int runnum=0);
  virtual  void Stop();
  virtual  void GScalerInit(GBufferIn2p3* _buffer,int runnum=0);
  virtual  void SetScalerMode(int mode=0,const char* filename="MyScalerTTree",bool reinit =false, bool withrun=false);
  virtual  void DumpScaler(void);
  virtual  void DumpScalerSpe(void);
  virtual  const GScalerEvent* GetScalerPtr (Int_t index) const;
  virtual  Int_t GetNbChannel (void ) const {return fNbChannel;}
 private:
  virtual void ToDoInCaseOfInterrupt(){};

  ClassDef(GScaler,1);         // Scaler  class

};


#endif
