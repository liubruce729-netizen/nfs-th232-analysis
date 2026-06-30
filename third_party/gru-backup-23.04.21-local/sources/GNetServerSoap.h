
// File : GNetServerSoap.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GNetServerSoap
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


#ifndef __GNetServerSoap__
#define __GNetServerSoap__

#include <sstream>
using std::ostream;

#include <TObject.h>
#include <TFile.h>
#include <TString.h>
#include <TNtuple.h>
#include <TROOT.h>
#include "GDevice.h"
#include "GAcq.h"
#include "GEtalonnageMust.h"
#include "GEtalonnageMatacq.h"
#include "General.h"
#include "GBuffer.h"
#include "GSpectra.h"
#include "General.h"
#include "GBase.h"
#include "GNetServer.h"
#include "GNetServerRoot.h"
#include "TPluginManager.h"

#ifndef NO_GSOAP
#include "GSoapErrorCode.h"

#ifndef __GNetServerSoap_H
extern "C"
{
// This might be written better
//typedef struct SOAP {int bidon;} saop;
  //typedef struct NS_CALIMRESPONSE {int bidon;} ns__CalimResponse;
}
#endif
#endif

//_________________________________________________________________________________________
class GNetServerSoap : public GNetServer{

 private:

  bool           fRunning; //
  char           fNameThread[32];//Name of thread
  int fPort;
  TPluginHandler *fPlug;

  GNetServer     * fRootServer;
  bool 			fPrompt_wait;
  char *       fMessage;//!
 public:

  char * GetMessage(){return fMessage;}
  GNetServerSoap(int port);
  virtual ~GNetServerSoap();
  virtual  void StartServer(bool quiet= false);
  virtual void StopServer(bool quiet= false);
  void SetVerbose (int verb);
  virtual void InitCommand();

 private:

 //void InitGlobalCalim();
 public:

 void * handler(void * p_soap);
 void InitSpectraServer(int mode, int port);
 void SetRootServer(GNetServerRoot* serv);

 ClassDef (GNetServerSoap ,1); // Nerwork ROOT Server to send spectra

};

#endif
