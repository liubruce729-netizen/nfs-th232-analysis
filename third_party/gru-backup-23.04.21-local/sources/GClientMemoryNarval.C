// Author: $Author: legeard $
// Part of GRU
//

// GClientMemoryNarval : Client of a Ganil Acquisition It can ask for a Spectra list and
// histograms.
//
//
//

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

//#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <unistd.h>
//#include <string.h>    /* for memset() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#include <errno.h>
#include <TObject.h>
#include <TH1.h>
#include <TH2.h>
#include <TFile.h>
#include <TROOT.h>

//include des fichiers header existants


extern "C" {
#include "GEN_TYPE.H"
#include "gan_acq_buf.h"
#include "GTtape_erreur.h"
#include "GEN_TYPE.H"
#include "STR_EVT.H"
#include "ERR_GAN.H"
}

#include "General.h"
#include "GEvent.h"
#include "GBuffer.h"
#include "GDevice.h"
#include "GClientMemoryNarval.h"

//differents DEFINE

#define typeBufferSize 	8
#define DUREE_ACQ	60

ClassImp( GClientMemoryNarval);

//_________________________________________________________________________________
//---------------------------   CONSTRUCTION ~ DESTRUCTION   ---------------------------
//_________________________________________________________________________________

GClientMemoryNarval::GClientMemoryNarval(void) {
	// Default constructor of GClientMemoryNarval
	// it just initializes several variables from the current hostname

	char tmp[MAX_CARACTERES];
	fStatus = ACQ_OK;
	fTempo = 60;
	fType = GA_MEMORY_NARVAL;
	if (gethostname(tmp, MAX_CARACTERES) == 0) {
		GClientMemoryNarvalInit(tmp);
	} else {
		fStatus = -1;
		fError.TreatError(2, fStatus,
				"occured during GClientMemoryNarval creation");
	}
}

//_________________________________________________________________________________

GClientMemoryNarval::GClientMemoryNarval(const char *host) :
	GDevice(host) {
	// Constructor of GClientMemoryNarval
	// it just initializes several variables from the parameter hostname
	fTempo = 60;
	GClientMemoryNarvalInit(host);

}

//_________________________________________________________________________________

GClientMemoryNarval::~GClientMemoryNarval() {
	//Destroyer of GClientMemoryNarval

	//ferme la connection avec le serveur si celle-ci est toujours valable
	Close();

	//if ( fPile_Buffer )    delete fPile_Buffer;  // PB to solve
	if (fBuffer_Envoye) {
		delete[] fBuffer_Envoye;
		fBuffer_Envoye = NULL;
	}
	if (fBuffer_Retour) {
		delete[] fBuffer_Retour;
		fBuffer_Retour = NULL;
	}
	if (fZoneData) {
		delete[] fZoneData;
		fZoneData = NULL;
	}

	if (fVerbose > 5)
		fError.TreatError(0, 0, "Object 'GClientMemoryNarval' deleted ");
}

//_________________________________________________________________________________

void GClientMemoryNarval::GClientMemoryNarvalInit(const char* host) {
	// Initialization of several variables
	// Method called only in construtors
	fStatus = ACQ_OK;
	fBuffer_Envoye = NULL;
	fNb_Buffers_Recus = 0;
	fNb_Buffers_Local = 0;
	fNb_Buffers_Envoyes = 0;
	fBuffer_Retour = NULL;
	fZoneData = NULL;
	fZoneData_cur = NULL;
	fSizeZoneData = 0;
	SetDevice(host);
    fBuffer = new GBufferIn2p3();
	fStatus = ACQ_OK;
	fType = GA_MEMORY_NARVAL;
	fBusy = false;
	fTempo = 10;

	fStoreOn = false;
	fTimestampsbuffersize = 0;
	fCurrent_diff_time_buff = 2;
	fPremier_Buffer = 0;
	fLastNumBuff = 0;
	fVerbose = 0;
	fNewOpen = false;

	fError.Infos("Device Client Narval is created to ", host);
	fComptbuffOK =0;
	fComptNotAvailable= 0;
}

//_________________________________________________________________________________
void GClientMemoryNarval::Open(char* mod) {
	//The enter parameter 'mod' is inefficient GDopen("anything") is equivalent to  GDopen ()
	Open();
}
//_________________________________________________________________________________
void GClientMemoryNarval::Open(char mod) {
	//have not effect in GClientMemoryNarval

	if (fIsOpen)
		return;

	fStatus = ACQ_OK;
	fIsOpen = true;
}

//______________________________________________________________________________

void GClientMemoryNarval::Close() {
	//have not effect in GClientMemoryNarval
	fStatus = ACQ_OK;
fIsOpen = false;

}

//_________________________________________________________________________________

void  GClientMemoryNarval::SetBufNarval(char* zone, int size ){
	fZoneData	= zone;
	fSize_buffer_Narval = size;
}

//_________________________________________________________________________________

void GClientMemoryNarval::GetBuffer(bool afficher) {
	// Retrieve the control buffer of the acquisition.
	TString tempos;
	int sizein2p3buf;
	fStatus = ACQ_OK;
//	int size_to_recv=0
	int size_buffer=0;
	//int received_data=0;
	//int total_received_data=0;
	//int countofbuffer=0;
	sizein2p3buf = GetBufferSize();

	if (fNb_Buffers_Local <= 0) {
		if (!fIsOpen) {
			Open();
		}

		fNb_Buffers_Local = 0;
		fTaille_Retour = 0;
	//	countofbuffer = 0;
		fTimestampsbuffersize = 12;

		size_buffer = fSize_buffer_Narval;

		if (size_buffer == 0) {
			fNb_Buffers_Local = 0;
			((GBufferIn2p3*)fBuffer)->SetToCommentBuffer();
			Close();
			//fStatus = ACQ_DISCONNECT;
			fStatus = ACQ_OK;
			if (fCurrent_diff_time_buff > 1000000)
				fCurrent_diff_time_buff = 1000000;
			if (fCurrent_diff_time_buff < 10)
				fCurrent_diff_time_buff = 10;
			if (fVerbose){
			tempos.Form("Received empty data, Waiting time = %.1f ms",(Float_t)(fCurrent_diff_time_buff)/1000);
			fError.TreatError(1, 0, tempos);
			fComptNotAvailable++;
			}
			//usleep((int)(((float)fCurrent_diff_time_buff)*0.2));
			return;
		} else {
			fComptbuffOK ++;
			gettimeofday(&fMt_meanbetw2buf, &fTz);
			fCurrent_diff_time_buff = ((int) (fMt_meanbetw2buf.tv_usec)
					- fLastTimeNarvalbuf);
			fLastTimeNarvalbuf = fMt_meanbetw2buf.tv_usec;
		}

		if (size_buffer > fSizeZoneData) {
			if (fZoneData) {
				delete[] fZoneData;
				fZoneData = NULL;
				fError.Infos("Delete fZoneData");
			}
			fSizeZoneData = size_buffer;
			if (fSizeZoneData < fTimestampsbuffersize + sizein2p3buf)
				fSizeZoneData = fTimestampsbuffersize + sizein2p3buf;
			fZoneData = new char[fSizeZoneData];
		}

		fZoneData_cur = (int*) fZoneData;
		//total_received_data = 0;
		//size_to_recv = size_buffer;

		fNb_Buffers_Local = (int) (size_buffer / sizein2p3buf);

		if (fNb_Buffers_Local * sizein2p3buf == size_buffer)
			fTimestampsbuffersize = 0;

		//received_data=size_buffer;
		Close();
		fStatus = ACQ_OK;
		fZoneData_cur = (int*) fZoneData;
	} //if (fNb_Buffers_Local <= 0)

	switch (fStatus) {
	case ACQ_OK:
		fNb_Buffers_Local--;
		fZoneData_cur = fZoneData_cur + fTimestampsbuffersize / (sizeof(int));
		//fBuffer->SetOutsideBuffer((char*) fZoneData_cur);
		memcpy((fBuffer->fGBuf_data), (char*) fZoneData_cur, sizein2p3buf);
		fZoneData_cur = fZoneData_cur + sizein2p3buf / (sizeof(int));

		if (fBuffer->IsEventBuffer()) {
			//fBuffer->DumpBuffer(256, 0);
			if (fPremier_Buffer == 0)
				fPremier_Buffer = fBuffer->GetNumBuf();
			fNb_Buffers_Recus++;

			fNb_Buffers_Envoyes = fBuffer->GetNumBuf() - fPremier_Buffer + 1;
			if (fLastNumBuff == fBuffer->GetNumBuf()) {
				fError.TreatError(2, 0,
						"Same Buffer Number...so it may be same buffer...");
			}
			fLastNumBuff = fBuffer->GetNumBuf();
			//cout <<"nBuffers_send="<<fLastNumBuff<<"\n";
		}
		break;
	default:
		if (afficher)
			fError.TreatError(1, 0, "Not a new buff");

	}

	return;
}

//_________________________________________________________________________________

void GClientMemoryNarval::ReadBuffer() {
	// Read a buffer
	struct timeval mt_reference;
	struct timeval mt_Timeout;
	struct timezone tz;
	Int_t time_out = 10;
	gettimeofday(&mt_Timeout, &tz);
	gettimeofday(&mt_reference, &tz);
	fStatus = ACQ_OK;

	while (true) {
		GetBuffer(false);
		gettimeofday(&mt_Timeout, &tz);

		if (((mt_reference.tv_sec - mt_Timeout.tv_sec) > time_out)) {
			fError.TreatError(1, 0, "Time out with no event");
			break;
		}
		if (fStatus != ACQ_NOCURRCTRLBUF)
			break;
	};
	fBuffer->SetAttributs();
}

//________________________________________________________________________________________

void GClientMemoryNarval::ReadBuffer(GBuffer& _Buffer) {
	// Read a buffer ; it just calls the method 'GetEventBuffer' and copies it into the given buffer
	while (true) {
		GetBuffer(false);
		if (fStatus != ACQ_NOCURRCTRLBUF)
			break;
	};
	fBuffer->Equal(_Buffer);
	fBuffer->SetAttributs();
}
