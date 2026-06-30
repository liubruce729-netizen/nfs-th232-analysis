// File : GGeneBuffer.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GGeneBufferActor
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


#ifndef __GGeneBufferActor__
#define __GGeneBufferActor__

#include <sstream>
using std::ostream;

#include <TThread.h>
#include <TH1.h>
#include "General.h"
#include "GBase.h"
#include "GBuffer.h"

#include "GTape.h"
#include "GGeneBufferActor.h"
#include "GGeneBuffer.h"


//_________________________________________________________________________________________


class GGeneBufferActor : public GBase{  // public producer


	GEvent		*fEvent;	//! Event to Generate event datas
	DataParameters	*fParameter;	//! Needed by Events

    int fDot, fDot2;
	GDevice    *fDevice;//!
	Bool_t	fGenerator;	// true mean generation mode and false read mode
	Bool_t	fEndrun;	// true if ENDRUN buffer is detected
	Bool_t	fActionFileUse;	// true if a action file is used
	long long int fTotalReadSize;
	long long int fTotalCopied;
	int fPaquetDot;
	bool fAlreadyread;
//	GTape		fReader2();
	unsigned int fId;
	char	fRunOrActionFile[MAX_CARACTERES];//!

	int	 fSleep; // Sleep time between two calls of process_block in milliseconds

    int  fBufferType;

	int	 fBufferCounter; // Counter of Buffers

     bool fInfiniteRead ;// if we want a continuous flow from 1 file (run) ( no end)


 public:


	GGeneBufferActor ();// default constructor of GGeneBufferActor object
	~GGeneBufferActor() ;

//	base_class *process_register (unsigned int *);
	GGeneBufferActor *process_register (unsigned int *);
	virtual   void SetFGenerator(bool mybool);
        virtual   void SetBufferType(Int_t type);
        virtual   void SetUSleep(int sleep);
        virtual   int GetfSleep();

  //  GDevice* GetReader(){return (GDevice*)fReader;};
        virtual         void SetInputFile(char* inputfile);
        virtual 	void SetActionFileUse(bool mybool);
	virtual 	void SetBufferSize(Int_t bufferSize);
	virtual 	void SetInfiniteRead(bool infinite);
	virtual 	void process_config (char *directory_path, unsigned int *error_code);
	virtual void process_initialize (unsigned int *error_code);

	virtual void process_start (unsigned int *error_code);
	virtual void process_block (void *output_buffer, unsigned int size_of_output_buffer, unsigned int *used_size_of_output_buffer, unsigned int *error_code);
	virtual void process_stop (unsigned int *error_code);
	virtual void process_pause (unsigned int *error_code);
	virtual void process_resume (unsigned int *error_code);
	virtual void process_unload (unsigned int *error_code);
	virtual void process_reset (unsigned int *error_code);
	virtual void set_id (unsigned int id);
	virtual void ToDoInCaseOfInterrupt(){};
	ClassDef (GGeneBufferActor ,1); // Device to generate GBuffer for Narval Actors

};

#endif
