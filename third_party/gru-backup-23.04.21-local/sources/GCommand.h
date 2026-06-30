// File : GCommand.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GCommand
//
// This Class to manage few commands ( usually from soap server)
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
// GCommand is a Class to command GAcq often with a remote TCPIP client ( via a soap server or a root server)
// A State machine si used to respect the different steps of processus
// 

#ifndef __GCommand__
#define __GCommand__


#include <sstream>

#include <TObject.h>
#include <TFile.h>
#include <TString.h>
#include <TNtuple.h>
#include <TROOT.h>
#include <TPluginManager.h>
#include <TTimer.h>

#include "GDevice.h"
#include "GAcq.h"
#include "GBuffer.h"
#include "General.h"
#include "GBuffer.h"
#include "GSpectra.h"
#include "GSpectraDB.h"
#include "General.h"
#include "GDevice.h"
#include "GAcq.h"
#include "GTape.h"
#include "GStateMachine.h"
#include "GMFMFile.h"

#ifdef NET_LIB
#include "GNetClientSoap.h"
#endif

class GCommand: public GBase {

protected:
    GStateMachine	*fStateMachine;	  //! State machine to control, authorize and verify steps

	int fTypeTtree;
	TString fFilenameSpectra;
	TString fFilenameTtree;
	bool fIsSaveTtree;
	bool fInRun;
	int fLocalVerbose;

	GDevice *fReaderDevice;
	GAcq *fAcq;
	GSpectra* fSpectra;
	Bool_t fAcqLocalCreation;
	TObject * fNetServRoot;
	TObject * fNetServSoap;

public:

	GCommand(TObject* above,GAcq* acq =NULL);
	~GCommand();


	virtual void InitDevice(char *source, Int_t type = 1, Int_t port = 0);
	virtual void RewindDevice();
	virtual void SetBufferSize(int size);
	virtual void SetDevice(GDevice *device) {fReaderDevice = device;};
	virtual GDevice * GetDevice() {return fReaderDevice;};
	virtual void SetServ(TObject *above);
	virtual void SetAcq(GAcq *acq);
	virtual GAcq * GetAcq() const{return fAcq;} ;
	virtual GSpectra * GetSpectra();
	virtual bool AnalyseCommandAndDo(char* command,TString *streturn =NULL);
	virtual void InitGAcq(int mode);
	virtual void InitRuns();
	virtual void InitEvent(char *source,char*type =NULL);
	virtual void InitStateMachine();
	virtual GStateMachine * GetStateMachine() const {return fStateMachine;}
	virtual bool IsInit();
	virtual void DoRun(int coups);
	virtual void DoRun();
	virtual void PauseAcq(int sec) ;
	virtual void Dump(int mode) const;
	virtual TString GetDump(int mode) const ;
	virtual TString GetInformation();
	virtual void PrintInformation();
	virtual void SaveTtree(char *string, int type);
	virtual void SpectraSave(char* filename =NULL);
	virtual void SpectraReset();
	virtual void SpectrumReset(char * namespectrum,char * family=NULL);
	virtual void SetSaveName(char *string);
	virtual void SetInfiniteReadGAcq(bool mode = true);
	virtual void StopRun(bool etat);
	virtual void EndAcq(bool calim =false);
	virtual void SetRun(bool etat);
	virtual bool InRun();
	virtual void Kill();
	virtual void printtext (char* info); 
	virtual void RefreshSpectraDB();
	virtual void SetVerboseGAcq(int level);
	virtual void InitSpectraServer(int mode, int port = 0);
	virtual void InitModeSpectre(int mode);
	virtual TString GetInfo(Int_t* nb_info);
	virtual void GetPlugin(char *pluginName);
	virtual GSpectraDB *GetSpectraBD();
	virtual void SetIncludeUser(char* include);
	virtual void Compilation(char *nameuser,char* opt=NULL);
	virtual TString GetSpectraList  ()const;
	virtual void SpectraList()const ;
	//virtual bool InitGlobalCalim(char * cardname);
	virtual	void InitCalim(char* exp_name, char* cardname, char*host_name,
				char* para_name, int nbpara, int calimode,char* eventinitmode=NULL);
	virtual	void DoRun( int coups, int* p_mat, int* p_voie);
	virtual void ConfigVigru();
	virtual void DumpCommand(char** commandsliced,int nb_words)const ;
private:
 virtual void ToDoInCaseOfInterrupt();

ClassDef(GCommand ,1);
};

#endif
