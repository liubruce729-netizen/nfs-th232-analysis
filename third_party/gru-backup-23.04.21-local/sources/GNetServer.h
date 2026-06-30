
// File : GNetServer.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GNetServer
//
// This Class serve spectra from a GSpectra class
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

#ifndef __GNetServer__
#define __GNetServer__

#include <sstream>
using std::ostream;

#include <TObject.h>
#include <TFile.h>
#include <TString.h>
#include <TNtuple.h>
#include <TROOT.h>
#include "GDevice.h"
#include "GCommand.h"
#include "General.h"
#include "GBuffer.h"
#include "GSpectra.h"
#include "General.h"
#include "GBase.h"
#include "GAcq.h"
#include "TPluginManager.h"

//_________________________________________________________________________________________
class GNetServer : public GDevice{

 private:
  bool           fRunning; //
  char           fNameThread[32];//Name of thread

  TPluginHandler *fPlug;
  GSpectra       *fSpectra;//!

  GCommand *      fCommand ;//! control of command
  bool            fServerReady;
 public:
  GAcq           *fAcq;//!
  GNetServer(int port);
  ~GNetServer();

 virtual void SetSpectra(GSpectra *spectra){fSpectra = spectra;}
 virtual GSpectra * GetSpectra(){return fSpectra ;} ;
 virtual void InitCommand() =0;
 virtual GCommand * GetCommand(){ return fCommand;};
 virtual void  SetCommand(GCommand *  command );
 virtual bool * GetFlagServerReady(){ return &fServerReady;};
 virtual bool GetRunningFlag(){ return fRunning;};
 virtual void SetRunningFlag(bool run){ fRunning= run;};
 virtual void InitCalimero();
 virtual void PauseAcq(int time);


 virtual Int_t Testport(Int_t port,bool autoportincrement=false);

 // methode coming from GDevice.h
 virtual   void    Rewind(bool quiet=false )   { cout <<" no function "<<quiet<<"\n";}
 virtual   void    Inquire(char* Exp_Name =(char*)""){ cout <<" no function "<<Exp_Name<<"\n";}
 virtual   void    Close() { cout <<" no function\n";}
 virtual   void    Open(char mod ='r') { cout <<" no function "<< mod<<" \n";}
 virtual   void    Open(char* mod){ cout <<" no function"<< mod<<"\n";}
 virtual   void    ReadBuffer()  { cout <<" no function\n";}
 virtual   void    WriteBuffer(GBuffer* _Buffer)  { cout <<" no function"<<_Buffer<<"\n";}

 virtual   TString *GetListSpectra(){ cout <<" no function\n"; return NULL;}
 virtual   TNamed*  GetSpectrum(const char* histoname=NULL,TNamed* old_spectrum=NULL) { cout <<" no function\n"; return NULL;}
 virtual void StopServer(bool quiet= false) =0;
 virtual void ToDoInCaseOfInterrupt();
 ClassDef (GNetServer ,1); // Nerwork  Server virtual Class

};

#endif
