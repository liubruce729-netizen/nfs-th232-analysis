// File : GGeneBufferActor.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GGeneBufferActor
//
// This class is a class to read device (tape,file direcory, network...
//  in Ganil format or generate datas and send it to a NARVAL actor
//
////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#include <sys/time.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mtio.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <TObject.h>
#include <TH1.h>

#include "General.h"
#include "GBuffer.h"
#include "GTape.h"
#include "GMFMFile.h"
#include "DataParameters.h"

#include "GGeneBufferActor.h"
#include "GEvent.h"

extern "C" {
#include "GEN_TYPE.H"
#include "STR_EVT.H"
#include "ERR_GAN.H"

#include "acq_codes_erreur.h"
}

#define DEFAULT_ACTIONFILE "./ACTION_local.CHC_PAR"

ClassImp( GGeneBufferActor);
//______________________________________________________________________________
GGeneBufferActor::GGeneBufferActor() {
	// Default constructor of object;

	fDevice = NULL;
	fError.TreatDebug(0, 0, "Constructor of GGeneBufferActor");
	fDevice = NULL;
	fEvent = NULL;
	fBufferType = EBYEDAT_Idn;
	fParameter = NULL;
	fActionFileUse = true;
	fGenerator = true;
	fEndrun = false;
	fId = -1;
	fVerbose = 0;
	fSleep = 0;
	fBufferCounter = 0;
	fInfiniteRead = false;
	fDot = 0;
	fDot2 = 0;
	fPaquetDot =200;
	fTotalReadSize =0;
	fAlreadyread =false;
	fTotalCopied =0;
}
//______________________________________________________________________________
GGeneBufferActor::~GGeneBufferActor(void) {
	// Default destructor of object;
	fError.Infos("Destructor of GGeneBufferActor");
	if (fDevice) {
		delete (fDevice);
		fDevice = NULL;
	}
	
}

//______________________________________________________________________________
void GGeneBufferActor::SetFGenerator(bool mybool) {
	fGenerator = mybool;
}

//______________________________________________________________________________
void GGeneBufferActor::SetBufferType(Int_t type) {
	TString tempos;
	fBufferType = type;
	tempos.Form("GGeneBufferActor::SetBufferType = %d", fBufferType);
	fError.Infos(tempos);
}
//______________________________________________________________________________
void GGeneBufferActor::SetBufferSize(Int_t bufferSize) {
	if (fDevice)
		fDevice->SetBufferSize(bufferSize);
	else
		fError.TreatError(1, 0, "GGeneBufferActor:SetBufferSize Device Null");
}

//______________________________________________________________________________
void GGeneBufferActor::SetActionFileUse(bool mybool) {
	fActionFileUse = mybool;
}

//______________________________________________________________________________
void GGeneBufferActor::SetUSleep(int sleep) {
	fSleep = sleep;
}
//______________________________________________________________________________
int GGeneBufferActor::GetfSleep() {
	return fSleep;
}
//______________________________________________________________________________
void GGeneBufferActor::SetInfiniteRead(bool infinite){fInfiniteRead= infinite;};
//______________________________________________________________________________
void GGeneBufferActor::SetInputFile(char * inputfile) {
if (fGenerator == true and fActionFileUse == false)
		strcpy(fRunOrActionFile, DEFAULT_ACTIONFILE);
	else
		strcpy(fRunOrActionFile, inputfile);	
}
//______________________________________________________________________________
GGeneBufferActor* GGeneBufferActor::process_register(unsigned int *error_code) {
	//Must return a object
	
	return this;
}
//______________________________________________________________________________
void GGeneBufferActor::process_config(char* directory_path,
		unsigned int *error_code) {

	TString tempos;
	tempos.Form("Config of GGeneBufferActor %s", directory_path);
	strcpy(fRunOrActionFile, (char*)directory_path);

}
//______________________________________________________________________________

void GGeneBufferActor::process_start(unsigned int *error_code) {
	*error_code = 0;
	TString tempos;
	//strcpy(fRunOrActionFile, (char*)"/home/legeard/ganacq_manip/geneactorebyin2p3/das-save/ACTIONS_geneactorebyin2p3.CHC_PAR");
	tempos.Form("Start of GGeneBufferActor in generatoin mod (%d) or in readfile mod(%d)  whith input file %s and with type %d",fGenerator,!fGenerator, (char*)fRunOrActionFile,fBufferType);
	fError.TreatDebug(0, 0, tempos);
	
	if (!fGenerator) // Reader mode
	{
		fError.TreatDebug(0, 0,
				"GGeneBufferActor::GGeneBufferActor process_register in reader mode");
		if ((fBufferType >= IN2P3_MIN_Idn) || (fBufferType <= IN2P3_MAX_Idn)) {
			if (fDevice == NULL) {
				fDevice = new GTape(fRunOrActionFile);
			} else {
				fDevice -> SetDevice(fRunOrActionFile);
			}

			if (!((GTape*) fDevice)->IsARun()) {
				*error_code = 1;
			}
		}
		if ((fBufferType >= MFM_MIN_TYPE) and (fBufferType <= MFM_MAX_TYPE)) {
			fDevice = new GMFMFile(fRunOrActionFile);
		}

	}
	else // Generator mode
	{
		fError.TreatDebug(0, 0,
				"GGeneBufferActor::process_config in generator mode");
		if (fDevice == NULL) {
			fDevice = (GDevice*) (new GGeneBuffer(
					(const char*) fRunOrActionFile, fBufferType));
		} else {
			fDevice -> SetDevice(fRunOrActionFile);
		}
	}

	tempos.Form("Start of GGeneBufferActor in generaton mod (%d) or in readfile mod(%d)  whith input file %s and with type %d",fGenerator,!fGenerator, fRunOrActionFile,fBufferType);
	fError.TreatDebug(0, 0, tempos);
	*error_code = 0;

	fError.TreatDebug(1, 0, "GGeneBufferActor::process_start");
	if (fDevice) {
		fDevice -> Open();
	} else {
		fError.TreatDebug(0, 0, "Device Null : Start Impossible !");
		*error_code = 1;
	}
}
;

//______________________________________________________________________________

void GGeneBufferActor::process_stop(unsigned int *error_code) {
	fError.TreatDebug(0, 0, "GGeneBufferActor::process_stop ");
	fDevice->Close();
	*error_code = 0;
}

//______________________________________________________________________________

void GGeneBufferActor::process_block(void* narval_output_buffer,
		unsigned int size_of_narval_output_buffer,
		unsigned int *used_size_of_narval_output_buffer,
		unsigned int *error_code) {

	if (fVerbose >6){
		fError.TreatDebug(0, 0, "GGeneBufferActor:: process_block ");
		
		}
	unsigned int usedSize = 0;
	static unsigned int currentBufferSize = 0;
	char *narval_output_buffer_to_use;
	unsigned int offset = 0;
	int dump_size=256;
	*error_code = 0;
	int status=0;
	TString tempos;
        static long long int count =0; 
 
	usleep(10);// necessaire sinon les buffers ne sont pas tous copiés.
	usleep(14000);
	narval_output_buffer_to_use = (char*) narval_output_buffer + offset;

	if (offset > 0) {
		memcpy((char*) (narval_output_buffer),
				"bidon bidon bidon bidon bidon ", offset);
	}
	
	if (fEndrun){
		fError.Infos("End of File "); 
		sleep(1);
		return;
		}
	


	while (true) {
		if (fAlreadyread == false ){
			fDevice->GetBuffer()->RazBuffer();
			fDevice ->ReadBuffer();
			status =fDevice ->GetStatus();
			fAlreadyread = true;
			currentBufferSize = fDevice -> GetCurrentBuffer() -> GetBufSize();
			fTotalReadSize +=currentBufferSize;
		}

		if (fVerbose >6){
			cout << " debug fDevice -> GetCurrentBuffer()->DumpBuffer("<<dump_size<<",0);\n";
			fDevice -> GetCurrentBuffer()->DumpBuffer(dump_size, 0);
		}
		if (fDevice ->GetType() == GT_TYPE_FILE) {
	//cout << "debug  on ne rentre jamais sauf en read  run "<< endl;
			if (fInfiniteRead) {
				if (fDevice->GetCurrentBuffer()->fGBuf_type == ENDRUN_Idn) {
					
					fDevice->Rewind();
					while (true) {
						fDevice->ReadBuffer();
						currentBufferSize = fDevice -> GetCurrentBuffer() -> GetBufSize();
						fTotalReadSize +=currentBufferSize;
						
						cout << "-------------read-rejected buffer-----------------------\n";
						if (fDevice->GetBuffer()->IsAEventBuffer())
							break; // break on  while (true)
					}
				fAlreadyread = false;
				}
			}
			//	cout<<" debug fDevice -> GetCurrentBuffer()->DumpBuffer(256,0);\n";

			if ((fDevice -> GetCurrentBuffer() -> IsENDRUN())) //Detection of ENDRUN buffer
			{// use in case of reader mode
				tempos.Form("ENRUN or empty buffer detected , Total read = %lld",fTotalReadSize);
				fError.Infos(tempos);
				fEndrun = true;
				fAlreadyread = false;
				break;
			}
		}
		if ((size_of_narval_output_buffer < currentBufferSize)) {
				fError.TreatError(1,0,"Narval Buffer < Read Buffer ");	
		}
		if (((size_of_narval_output_buffer - (usedSize + offset))
				< currentBufferSize) and !fEndrun) {
				
			break;
		}

		if (!(fDevice -> GetCurrentBuffer() -> IsAHeaderBuffer())) {
			memcpy((char*) (narval_output_buffer_to_use) + usedSize,
					(char*) (fDevice->GetCurrentBuffer() -> fGBuf_data),
					currentBufferSize);
			usedSize += currentBufferSize;
			fTotalCopied +=currentBufferSize;	
		}
		fAlreadyread = false;
	}

	if (fDot2++ > fPaquetDot) {
		cout << "." << flush;
		fDot2 = 0;
		fDot++;
	}
	if (fDot >= 40) {
		cout << "\n";
		fDot = 0;
	}

	if (fVerbose == 10) {
		tempos.Form("%d Bytes transferred ", usedSize);
		fError.Infos(tempos);
	}
	*used_size_of_narval_output_buffer = usedSize + offset;
	cout <<"  --- nb narval buff : "<<++count <<" *used_size_of_narval_output_buffer " << usedSize<<" Total read " <<fTotalReadSize<< " Total copied  " <<fTotalCopied<< "  Status : "<< status<<"\n";
	*error_code = fEndrun ? 66 : 0;
	usleep(fSleep);
}
//______________________________________________________________________________

void GGeneBufferActor::set_id(unsigned int new_id) {
	//	fError.TreatDebug(0,0,"SetId of GGeneBufferActor");
	fId = new_id;
}

//______________________________________________________________________________

void GGeneBufferActor::process_reset(unsigned int *error_code) {

	fError.TreatDebug(0, 0, "Reset of GGeneBufferActor");
	unsigned int errorCode;
	process_unload(&errorCode);

	if (errorCode != 0) {
		*error_code = errorCode;
	} else {
		process_initialize(&errorCode);

		if (errorCode != 0) {
			*error_code = errorCode;
		} else {
			*error_code = 0;
		}
	}
}

//______________________________________________________________________________

void GGeneBufferActor::process_initialize(unsigned int *error_code) {
	fError.TreatDebug(0, 0, "Initialize of GGeneBufferActor");
	*error_code = 0;
}

//______________________________________________________________________________

void GGeneBufferActor::process_unload(unsigned int *error_code) {
	fError.TreatDebug(0, 0, "Unload of GGeneBufferActor");
	*error_code = 0;
}

//______________________________________________________________________________

void GGeneBufferActor::process_pause(unsigned int *error_code) {
	fError.TreatDebug(0, 0, "Pause of GGeneBufferActor");
	*error_code = 0;
}

//______________________________________________________________________________

void GGeneBufferActor::process_resume(unsigned int *error_code) {
	fError.TreatDebug(0, 0, "Resume of GGeneBufferActor");
	*error_code = 0;
}

//______________________________________________________________________________
