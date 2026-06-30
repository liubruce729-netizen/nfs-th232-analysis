// File :  GDemo.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GDemo
//
// This class do demonstration of alive spectra.
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


#ifndef __GDemo__
#define __GDemo__

#include <sys/time.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<iostream>
#include<string>
#include<pthread.h>
#include "General.h"
#include "GBase.h"
#include "DataParameters.h"
#include "GSpectra.h"
#include "GNetServerRoot.h"
#include "TCutG.h"
#include "TMayaHisto.h"
#include "TH2.h"
#include "TGraph.h"
#include "TNtuple.h"
#include "TProfile.h"
#include "TThread.h"

#define SIZEOFHISTO 512
#define NBOFHISTO   8
//_________________________________________________________________________________________


class GDemo : public GBase{

	private:
	Int_t fSizeofhisto;
	Int_t fNbofhisto;

	Int_t fNb_peak;
	Int_t fPort;
	TString fServername;

	GSpectra* fSpe;
	GNetServerRoot* fServ;

	public:
	TMayaHisto* fTMaya1, *fTMaya2;
	TCutG * Mycut1;
	TCutG * Mycut2;
	TCutG * Mycut3;
	TH1F *fHpx[NBOFHISTO];
	TH2F *fHpxpy[8];
	TProfile*fHprof[8];
	TGraph *fGraph[8];
	TH1S* fHisto5[8];
	TH2S* fHisto6[8];
	TH1F* fHisto7[8];
	TThread * fThreadAlive;

public:
	GDemo();
	~GDemo();

	virtual	void Init();
	virtual	void InitSpectra();
	virtual void SetPort(int port){fPort =port;};
	virtual	void InGSpectra();
	virtual	void InitRootServer(int port = 9090);
	virtual void GSpectraInMem() ;
	virtual GSpectra * GetGSpectra();
	virtual void SpectreAlive() ;
	static void SpectreAlive(void * arg);
	virtual void SpectreAliveT();
	private:
	virtual void ToDoInCaseOfInterrupt();

ClassDef (GDemo,1); //Demonstration of alive spectra

};

#endif

