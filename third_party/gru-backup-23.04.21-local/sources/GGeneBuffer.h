// File : GGeneBuffer.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GGeneBuffer
//
// This class manger tape and file in Ganil format
// The associated methods can do copies,duplications, verifications, dumps...
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


#ifndef __GGeneBuffer__
#define __GGeneBuffer__

#include <sstream>
using std::ostream;


#include <TThread.h>
#include <TH1.h>
#include "TTimeStamp.h"
#include "General.h"
#include "GBuffer.h"
#include "GTape.h"
#include "GBufferIn2p3.h"
#include "GBufferMFM.h"
#include "GEvent.h"
#include "GEventMFM.h"

//_________________________________________________________________________________________


class GGeneBuffer : public GDevice//, public producer
{
	GEventBase		*fEvent;	//! Event to Generate event datas
	DataParameters	*fParameter;	//! Needed by Events
	Bool_t	fActionFileUse;	// true if a action file is used

	int	 **fSubTabsOfIndex; //! 2 dimensions tab, a column by sub-event
	int  fNbOfSubEvents; // Number of sub-EVents by Event
	int	 fSizeOfSubTab; // Number of Parameters by Sub-Event
	int	 fEventCounter; // Counter of Events
	UInt_t	fBufferCounter; // Counter of Buffers
	int fBufferType;//
	int fBufferInsideType;//
	TTimeStamp* fTimeStamp;
    bool fInfiniteRead ;// if we want a continuous flow from 1 file (run) ( no end)
    int fLaw; // in set  law in ramdom generation
    Float_t fProba;// Probability of parameter appearance
 public:

	GGeneBuffer ();// default constructor of GGeneBuffer object
	GGeneBuffer (const char* _name,int type = EBYEDAT_Idn);// default constructor of GGeneBuffer object
	~GGeneBuffer() ;
	void GGeneInit(const char* _name,int type = EBYEDAT_Idn,int insidetype = 0);
	void SetNbOfSubEvents(int myint);
	void SetRandomLaw(int law);
	void SetRandomProba(Float_t proba);

	 virtual   void    Rewind(bool quiet=false ) {cout << "Rewind in GGeneBuffer has no action\n";} ;
	 virtual   void    Inquire(char* Exp_Name =(char*)""){cout << "Inquire in GGeneBuffer has no action\n";};
	 virtual   void    Close();
	 virtual   void    Open(char mod ='r') ;
	 virtual   void    Open(char* mod);
	 virtual   void    ReadBuffer()  ;
	 virtual   void    ReadBufferEBYEDAT()  ;
	 virtual   void    ReadBufferMFMCOBO()  ;
	 virtual   void    ReadBufferMFMEXOGAM2()  ;
	 virtual   void    ReadBufferMFMEEBYEDAT(uint32_t type)  ;
	 virtual   void    ReadBufferMFMOSCI() ;
	 virtual   void    ReadBufferMFM(uint32_t type);
	 virtual   void    WriteBuffer(GBuffer* _Buffer) ;

	 virtual   TString *GetListSpectra(){cout << "GetListSpectra() in GGeneBuffer has no action\n";return(NULL);};
	 virtual   TNamed*  GetSpectrum(const char* histoname=NULL,TNamed* old_spectrum=NULL){cout << "GetSpectrum in GGeneBuffer has no action\n";return(NULL);};

	ClassDef (GGeneBuffer ,1); // Device to generate Ebyedat GBuffer


};

#endif
