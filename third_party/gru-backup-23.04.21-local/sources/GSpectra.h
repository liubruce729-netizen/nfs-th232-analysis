// File : GSpectra.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GSpectra
//
// This class manage spectra from Ganil format to root histogram .
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

#ifndef __GSpectra__
#define __GSpectra__

#include <sstream>
using std::ostream;

#include <TObject.h>
#include <TFile.h>
#include <TString.h>
#include <TTree.h>
#include <TNtuple.h>
#include <TROOT.h>
#include <TDirectory.h>
#include <TFolder.h>
#include <TXMLEngine.h>
#include "TH1.h"
#include "TH2.h"


#include "GVClassManual.h"
#include "GVBrowser.h"
#include "GVListTree.h"
#include "GVSpectraInfo.h"
#include "GVClass.h"
#include "GVClassAuto.h"

#include "General.h"
#include "GBase.h"
#include "DataParameters.h"
#include "GEvent.h"
#include "GSpectraDB.h"
#include "GSpectrumIdentity.h"

#include "GDevice.h"
#include "GListDevice.h"
#include "GTape.h"
#include "GNetClientRoot.h"
#include "GClientMemory.h"

#ifdef NET_LIB
#include "GNetClientSoap.h"
#include "GNetClientNarval.h"
# define MAX_SIZE_GANIL_SPECTRUM  2097500  // 1024*1024*2 + header
#endif

extern "C" {
#include "acq_tcp_struct.h"
}

//_________________________________________________________________________________________

class GSpectra: public GBase {

private:


	TString fDefaultHostName; // Default Host Name writen in DB
	TString fDefaultSourceType; // Default source type writen in DB
	TString fDefaultSource; // Default source  writen in DB
	Int_t fDefaultHostPort; // Default Host portwriten in DB
	TString fRootConfigFile;

	GSpectraDB *fListServerDB;
	GSpectraDB *fSpectraDB;


	GListDevice *fListDevice;

	/*GDevice *fNetClient;
	GTape *fTape;
	GNetClientRoot *fNetClientRoot;
	GClientMemory *fClientMemory;
*/
#ifdef NET_LIB
	GNetClientSoap *fNetClientSoap;
	GNetClientNarval *fNetClientNarval;


	char* fZoneData; //! vector to receive a Ganil spectrum
	tete_spec * fSpectrumHeader; // buffer of spectrum header ( in Ganil Format)char*
	char* fSpectrumData; //! pointer on spectra in buffer of spectrum data ( in Ganil format)
	Int_t fSpectrumHeaderSize; // size of  Ganil spectrum header.
#endif
	int fStatus; // Status
	Int_t fNb_Raw_spectra; // nb raw spectra 

public:

	GSpectra(); // default constructor
	~GSpectra();

	virtual GListDevice* GetListDevice() {
		return fListDevice;
	}

	//virtual void RestoreConfig(TString afileXML);
	virtual void ReadXML(TXMLEngine *xml, XMLNodePointer_t node, bool server);
	virtual void CreateXML(TXMLEngine *xml, XMLNodePointer_t node, bool server);

	virtual Int_t UpdateSpectraList();
	virtual Int_t UpdateSpectraListGanilFile(GSpectrumIdentity *idserv);
	virtual Int_t UpdateSpectraListGruFile(GSpectrumIdentity *idserv);
	virtual Int_t UpdateSpectraListGanilNet(GSpectrumIdentity *idserv);
	virtual Int_t UpdateSpectraListGruNet(GSpectrumIdentity *idserv);
	virtual Int_t UpdateSpectraListSoapNet(GSpectrumIdentity *idserv);
	virtual Int_t UpdateSpectraListMemory(GSpectrumIdentity *idserv);
	virtual Int_t AddSpectrumFromFile(TFile *file, TNamed *obj,TString path);
	virtual Int_t ScanDirectoryFile(TFile * file, TDirectory * dir);
	virtual Int_t ScanDirectoryMemory(TFolder * folder,char * path =NULL);
	virtual Int_t ScanDirectoryMemory(char * dir);
	virtual Int_t AddSpectrumFromMemory( TNamed * sp, char* path);

	//virtual void SaveSpectra(GSpectraDB* DB, char* & file);
	virtual TNamed* GetSpectrumNetOrFileOrMem(GSpectrumIdentity* id);
	virtual TNamed* GetSpectrumNet(GSpectrumIdentity *id);
	virtual TNamed* GetSpectrumFile(GSpectrumIdentity *id);
	virtual TNamed* GetSpectrumMem(GSpectrumIdentity* id);
	virtual void ResetSpectrumOnServer(GSpectrumIdentity* id);

	virtual GSpectraDB* GetServerDB();

	virtual GSpectraDB* GetDB() {
		return fSpectraDB;
	}
	virtual void SetDB(GSpectraDB* newDB) {
		 fSpectraDB=newDB;
	};

	virtual void DumpSpectrumHeader();
	virtual TH1I* Convert1D();
	virtual TH2S* Convert2D();

	virtual void StartAllRawParameters();
	virtual void ConstructAllRawSpectra();
	virtual void AddAllParameterInDB(DataParameters *data_para,
			TString family = "",Int_t auto_level=0);
	virtual void SumParameterSpectrum(DataParameters *data_para, TString family);
	virtual int  RawParameter(DataParameters *data_para, int index,
			TString family = "",Int_t auto_level=0);
	virtual void RemoveSpectrum(TNamed *sp);
	virtual void RemoveSpectrum(int no);
	virtual void CleanAllDB();
	virtual Int_t AddSpectrum(TNamed *spectrum,const char* family = NULL,
			Int_t port = 0,const char* source_name = NULL, const char* sourcenetorfile =
					NULL);
	virtual Int_t AddCut(TNamed *spectrum,const char* family = NULL, Int_t port = 0,
			const char* source_name = NULL,const char* sourcenetorfile = NULL);
	virtual void GetCutFromFile(TNamed *spectrum,const char* family, const char* filename);
	virtual void RazDB();
	virtual void RemoveDB();
	virtual void SpeSave(const char* filename);
	virtual void FillRawSpectra(GEventBase* _event);
	virtual void ReferenceInMemory() ;

	virtual int GetLast();

	virtual void ReplaceDB(GSpectraDB* newDB);

	virtual void SetStarted(Int_t id, Int_t index2 = -1);
	virtual void SetStarted(const char* histo);
	virtual void SetStarted(Int_t id, bool start);
	virtual void SetStopped(Int_t index, Int_t index2 = -1);
	virtual void SetStopped(const char* histo);
	virtual void SetfDefaultHostName(const char* hostname) {
		fDefaultHostName = hostname;
	}
	;
	virtual void SetfDefaultHostPort(Int_t port) {
		fDefaultHostPort = port;
	}
	;
	virtual bool IsStarted(Int_t index);
	virtual bool IsStarted(const char* histo);
	virtual TString GetfDefaultHostName() {
		return fDefaultHostName;
	}
	;
	virtual TString GetfDefaultSourceType() {
		return fDefaultSourceType;
	}
	;
	virtual Int_t GetfDefaultHostPort() {
		return fDefaultHostPort;
	}
	;
	virtual GSpectrumIdentity* GetIdWithIndex(Int_t index);
	virtual GSpectrumIdentity* GetIdentity(const char* sp,const char* familly);
	virtual GSpectrumIdentity* GetIdentity(const char* sp);
	virtual TNamed* GetSpectrum(const char* sp);
	virtual TNamed* GetSpectrum(const char* sp,const  char* familly);
	virtual void SetSynchroOnEvt(const char* sp,bool value=true) ;
	virtual void SetSynchroOnEvt(const char* sp, const char* familly,bool value=true) ;

	virtual TNamed* GetSpectrum(Int_t index);
	virtual TH1* GetHisto(const char* histo);
	virtual TH1* GetHisto(Int_t index);
private:
	  virtual void ToDoInCaseOfInterrupt(){};
ClassDef(GSpectra ,1)
; // Manager of Data base of spectra (histograms) : Recover spectra from files and from net. Fill the DB when sources are chosen
};

#endif
