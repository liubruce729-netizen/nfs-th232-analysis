
// File : GNetServerRoot.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GNetServerRoot
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


#ifndef __GNetServerRoot__
#define __GNetServerRoot__

#include "General.h"
#include "GNetServer.h"
#include "GAcq.h"
#include <sstream>
using std::ostream;

#include <TObject.h>
#include <TFile.h>
#include <TString.h>
#include <TNtuple.h>
#include <TROOT.h>
#include <TThread.h>
#include <TMessage.h>
#include <TSocket.h>
#include <TServerSocket.h>
#include <TMonitor.h>
#include <TGListTree.h>
#include <TList.h>
#include <TH1.h>
#include <TH2.h>
#include <TThread.h>

#include "TMayaHisto.h"
#include "GBuffer.h"
#include "GSpectra.h"
#include "GNetServer.h"

//_________________________________________________________________________________________
class GNetServerRoot : public GNetServer {

 private:

  TServerSocket *fServSock ;  //! Server
  char           fStr[1024];//
  TList         *fSockList;   //! List of open socket with client
  TMonitor      *fMon;        //! socket monitor (if we want to drive few client connection)
  TThread       *fThreadNet; //!
  TString        fServerName; //
  char           fNameThread[64]; //  Name of thread
  bool           fStarting;  // informe about starting of server
  bool           fQuiet; // inform if in few method message have to been screened
  bool           fForMetaserver; // Flag to indicate if the server is for a metaserver or not

  int fDebug_cont;
  int fDebug_cont2;


 public:

  GNetServerRoot(int port,GSpectra* spectra =NULL);
  GNetServerRoot(int port,GAcq* acq);
  ~GNetServerRoot();
  virtual void SetAcq(GAcq *acq) ;
  virtual void InitCommand();
  virtual Int_t StartServer(bool testport=false,bool autoportincrement=false);
  virtual void StartThreadServer(bool quiet= false);
  virtual void StopServer(bool quiet= false);
  virtual void InitServer();

  virtual void WaitCommandsFromClient();
  virtual void TreatmentCommands(TSocket *localsock);
  virtual void SetForMetaServer(bool meta){fForMetaserver =  meta;}
 private:

  static void  ThreadWaitCommandsFromSeveralClients(void* arg);
  static void  ThreadWaitCommandsFromOneClient(void* arg);

  virtual void GiveObject(TSocket *localsock,TObject * obj);
  virtual void GiveWords(TSocket *localsock,TString * words);
  virtual int ReceiveWords(TSocket *localsock,TString *recp);
  virtual bool ReceiveConnections(TSocket *localsock,TString *recp);
  virtual TObject* ReceiveObject(TSocket *localsock);


  void GiveListSpectra(TSocket *localsock);
  void GiveSpectraDB(TSocket *localsock);
  void GiveSpectrumOnMetaServer(char* namespectra,char* family,TSocket *localsock);
  void GiveSpectrum(char* namespectra,char* family,TSocket *localsock);
  void RefreshHisto();
  bool ReceiveSpectrum(char* namespectra, char* family,TSocket *localsock);
  bool ResetSpectrumOnMetaServer(char* namespectra, char* family);
  bool AnalyseSpecificCommandAndDo(char* command, TSocket*localsock) ;

  virtual bool GetForMetaServer(){return fForMetaserver;}

  ClassDef (GNetServerRoot ,1); // Nerwork ROOT Server to send spectra

};

#endif
