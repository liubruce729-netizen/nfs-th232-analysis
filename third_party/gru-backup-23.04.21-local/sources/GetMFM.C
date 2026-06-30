// Author: $Author: legeard $
// Part of GRU
/////////////////////////////////////////////////////////////////////////////
// GetMFM : Client to Get MFM frame for different source ( file, network ...).
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

#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "GetMFM.h"

ClassImp( GetMFM);

//_________________________________________________________________________________

GetMFM::GetMFM(void) {
	// Default constructor of GetMFM
	// it just initializes several variables from the current hostname
	GetMFMInit(NULL);
}

//_________________________________________________________________________________

GetMFM::GetMFM(const char *host) {
	// Constructor of GetMFM
	// it just initializes several variables from the parameter hostname
	GetMFMInit(host);
}

//_________________________________________________________________________________

GetMFM::~GetMFM() {
	//Destroyer of GetMFM

	//ferme la connection avec le serveur si celle-ci est toujours valable
	Close();
	if (fGetMFMDeviceRand) delete fGetMFMDeviceRand;
	if (fGetMFMDeviceRF)   delete fGetMFMDeviceRF;
    if (fGetMFMDeviceTCP)  delete fGetMFMDeviceTCP;
    if (fGetMFMDeviceUDP)  delete fGetMFMDeviceUDP;
    if (fGetMFMDeviceFDT)  delete fGetMFMDeviceFDT;
    if (fGetMFMDeviceZMQ)  delete fGetMFMDeviceZMQ;
    if (fGetMFMDeviceFile) delete fGetMFMDeviceFile;
    if (fGetMFMDeviceEbyBlockFile) delete fGetMFMDeviceEbyBlockFile;
#if defined(__CLING__) && !defined(__ROOTCLING__)
    if (fGetMFMDeviceEbyBlockTcp)  delete fGetMFMDeviceEbyBlockTcp;

#endif
	if (fVerbose > 5)
		fError.TreatError(0, 0, "Object 'GetMFM' deleted ");
}
//_________________________________________________________________________________
void GetMFM::GetMFMInit(const char *host){
   fBufferSize = MFM_BLOB_HEADER_SIZE;
   MyVector_c   = new UtilVector_c(fBufferSize);
   fType = GA_GETMFM ;
   fStatus = ACQ_OK;
   if (fBuffer ){
		if (!(fBuffer->IsAMFMBuffer())) {
		delete fBuffer;
		fBuffer = NULL;
	}
     }
    if (fBuffer == NULL)
	fBuffer = (GBuffer*) (new GBufferMFM(fBufferSize));;

    fGetMFMDeviceRand  = new DeviceRandMFM();
    fGetMFMDeviceRF    = new DeviceRFSoc();
    fGetMFMDeviceTCP   = new DevicePureTCP();
    fGetMFMDeviceUDP   = new DevicePureUDP();
    fGetMFMDeviceFDT   = new DeviceFDT();
    fGetMFMDeviceZMQ   = new DeviceZMQ();
    fGetMFMDeviceFile  = new DeviceFile();
    fGetMFMDeviceNarval= new DeviceNarval();
    fGetMFMDeviceXdaq  = new DeviceXdaq();
    fGetMFMDeviceEbyBlockFile = new DeviceEbyBlockFile();
#if defined(__CLING__) && !defined(__ROOTCLING__)
    fGetMFMDevice = new DeviceGeneric(); // TODO
    fGetMFMDeviceEbyBlockTcp = new DeviceEbyBlockTcp();
#endif
}	

//_________________________________________________________________________________
void GetMFM::InitReceiver (const char* host,int port,char* type) {

	// Initialization of several variables
	// Method called only in construtors
	TString tempos;
	fStatus = ACQ_OK;
	DEVICE_TYPE devtype;
	devtype = fGetMFMDeviceRF->GetDeviceType(type);
	

switch(devtype){
    /*case GENERIC:
	fGetMFMDevice = (Device*)fGetMFMDeviceGeneric;
	break;*/
    case SIMUL:
	fGetMFMDevice = (Device*)fGetMFMDeviceRand; 
	break;
    case RFSOC:
 	fGetMFMDevice = (Device*)fGetMFMDeviceRF; 
	break;
    case TCP_PURE:
 	fGetMFMDevice = (Device*)fGetMFMDeviceTCP; 
	break;
    case UDP_PURE:
	fGetMFMDevice = (Device*)fGetMFMDeviceUDP; 
	break;
    case FDT:   
	fGetMFMDevice = (Device*)fGetMFMDeviceFDT;
	break;
   case NARVAL:   
	fGetMFMDevice = (Device*)fGetMFMDeviceNarval;
	break;
   case XDAQ:   
	fGetMFMDevice = (Device*)fGetMFMDeviceXdaq;
	break;
    case ZMQ:
	fGetMFMDevice = (Device*)fGetMFMDeviceZMQ;
	break;
    case FILE_RUN:
	fGetMFMDevice = (Device*)fGetMFMDeviceFile;
	break;
#if defined(__CLING__) && !defined(__ROOTCLING__)
    case EBY_TCP_BLOCK:
 	fGetMFMDevice = (Device*)fGetMFMDeviceEbyBlockTcp;
 	break;
#endif
    case EBY_FILE_BLOCK:
    fGetMFMDevice = (Device*)fGetMFMDeviceEbyBlockFile;

   default:
    	cout << "GetMFM::InitReceiver : No Device choosen "<<endl;
	break;	
   }	
	
        if (host!=NULL) fGetMFMDevice->SetHostName ((char*)host) ;
        fGetMFMDevice->SetPort(port);
        SetDevice(fGetMFMDevice->GetHostName_char()) ;
        SetPort(port);
        //fGetMFMDevice->SetDeviceType(type);
        fGetMFMDevice->SetVerbose (GetVerbose());
	tempos.Form("Device GetMFM %s is created with host %s:%d",
			fGetMFMDevice->GetTextDeviceType(devtype),GetDeviceName(), GetPort());
	fError.Infos(tempos);
}

//_________________________________________________________________________________
void GetMFM::Open(char* mod) {	
	Open();
}
//_________________________________________________________________________________
void GetMFM::Open(char mod) {
	fIsOpen = true;
	fGetMFMDevice->InitReceiver (GetDeviceName(),GetPort());
}
//______________________________________________________________________________
void GetMFM::Close() {
	fStatus = ACQ_OK;
	
	if (fIsOpen) {
                fGetMFMDevice->Close();
		fIsOpen = false;
	}
}
//_________________________________________________________________________________

void GetMFM::ReadBuffer() {
	// Read a buffer
	fError.TreatDebug(10,0, "GetMFM::ReadBuffer()");

	int returnsize;
	fStatus = ACQ_OK;
	if (fIsOpen == true) {	
        	returnsize=fGetMFMDevice->Read(MyVector_c);
        	if (fBuffer->GetBufSize() < returnsize) {
        		((GBufferMFM*)fBuffer)->GBufferMFM::SetBufSize(returnsize);
        	}
        	((GBufferMFM*)fBuffer)->GBufferMFM::SetReadSize(returnsize);
        	memcpy(fBuffer->fGBuf_data,MyVector_c->GetPointer(),returnsize);
        	fBuffer->SetType(fBuffer->TestType(NULL));
        }
}
//_________________________________________________________________________________
//_________________________________________________________________________________

