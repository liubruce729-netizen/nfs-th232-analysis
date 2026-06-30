// Author: $Author: legeard $
// Part of GRU
/////////////////////////////////////////////////////////////////////////////
// GNetClientNarval : Client of a Ganil Acquisition fo  Narval data flow.
//                    This client get buffer Narval from network by a standard TCPIP connection
// 					  The 4 first bytes of Narval buffer contain size Narval buffer
//					  sommetime the 12 following containing the timestamps (and only with IN2P3 buffer)
//					  after you can have either few IN2P3 buffer ( with fixed size) or few MFM frame (with variable size)
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

#include <errno.h>
#include <TObject.h>
#include <TH1.h>
#include <TH2.h>
#include <TFile.h>
#include <TROOT.h>
#include <TString.h>
//include des fichiers header existants

extern "C" {
#include "GTtape_erreur.h"
#include "GEN_TYPE.H"
#include "STR_EVT.H"
#include "ERR_GAN.H"
#include "gan_acq_buf.h"
}

#include "General.h"
#include "GEvent.h"
#include "GBuffer.h"
#include "GDevice.h"
#include "GNetClientNarval.h"

//differents DEFINE
#define defaultPortn 	10201
#define typeBufferSize 	8
#define DUREE_ACQ	60

ClassImp( GNetClientNarval);

//_________________________________________________________________________________

GNetClientNarval::GNetClientNarval(void) {
	// Default constructor of GNetClientNarval
	// it just initializes several variables from the current hostname
	fType = GA_NET_NARVAL;
	char tmp[MAX_CARACTERES];
	fStatus = ACQ_OK;
	fTempo = 60;
	fVerbose =10;
	TString HostName = gSystem->Getenv("ACQ_SERV_NAME");

	if (HostName.CompareTo("") == 0) {
		HostName = gSystem->HostName();
	}
	if (gethostname(tmp, MAX_CARACTERES) == 0) {
		GNetClientNarvalInit(HostName.Data());
	} else {
		fStatus = -1;
		fError.TreatError(2, fStatus,
				"occured during GNetClientNarval creation");
	}
	

}

//_________________________________________________________________________________

GNetClientNarval::GNetClientNarval(const char *host) :
	GDevice(host) {
	// Constructor of GNetClientNarval
	// it just initializes several variables from the parameter hostname
	fTempo = 60;
	fType = GA_NET_NARVAL;
	GNetClientNarvalInit(host);

}

//_________________________________________________________________________________

GNetClientNarval::~GNetClientNarval() {
	//Destroyer of GNetClientNarval

	//ferme la connection avec le serveur si celle-ci est toujours valable
	Close();
	
	if (fZoneData_char) {
		delete[] fZoneData_char;
		fZoneData_char = NULL;
	}

	if (fVerbose > 5)
		fError.TreatError(0, 0, "Object 'GNetClientNarval' deleted ");
}

//_________________________________________________________________________________


void GNetClientNarval::GNetClientNarvalInit(const char* host) {
	// Initialization of several variables
	// Method called only in construtors
	TString tempos;
	fStatus = ACQ_OK;

	fNb_Buffers_Recus = 0;
	fNb_Buffers_Local = 0;
	fNb_Buffers_Envoyes = 0;

	fSize_narval_buffer = 0;

	fSizeZoneData = 16;// initiale taille de demarrage pour pouvoir lire au moins un type
	fZoneData_char = new char[fSizeZoneData];// buffer local dans lequel est recopié les narval buffers
	RazZone(fZoneData_char, fSizeZoneData);
	fZoneData_cur_int = (int*) fZoneData_char;

	fSocket = 0;
	fStatus = ACQ_OK;
	fType = GA_NET_NARVAL;
	fBusy = false;
	fTempo = 10;

	fStoreOn = false;
	fTimestampsbuffersize = 0;
	fCurrent_diff_time_buff = 2;
	fPremier_Buffer = 0;
	fLastNumBuff = 0;
	fVerbose = 0;
	fNewOpen = false;
	fNarvalBufferEndReached = true;
	fComptbuffOK = 0;
	fComptNotAvailable = 0;
	fBufferIn2p3 = new GBufferIn2p3();
	fBufferMFM = new GBufferMFM();
	fBuffer = fBufferIn2p3;
	TInetAddress Tip = gSystem->GetHostByName(host);
	TString IpNumber = Tip.GetHostAddress();


	SetDevice(IpNumber.Data());
	SetPort(defaultPortn);
	
	TString BufSizeName = gSystem->Getenv("ACQ_BUFFER_SIZE");
	if (BufSizeName.CompareTo("") != 0) {
		Int_t size;
		size = BufSizeName.Atoi();
		if ((size > 0) and (size < 1000000000)) {
			SetBufferSize(size);
		}
	}
	tempos.Form(
			"Device narval client is created with server %s and with buffer size %d",
			GetDeviceName(), GetBufferSize());
	fError.Infos(tempos);
}

//_________________________________________________________________________________
void GNetClientNarval::Open(char* mod) {
	//The enter parameter 'mod' is inefficient GDopen("anything") is equivalent to  GDopen ()
	Open();
}

//_________________________________________________________________________________
void GNetClientNarval::Open(char mod) {
	//Log on to the distant host which name is a parameter

	if (fIsOpen)
		return;
	gettimeofday(&fMt_permitclose, &fTz);

	TString name = GDevice::GetDeviceName();

	struct hostent *host_addr;
	in_addr_t tmp;

	memset(&fServAddr, 0, sizeof(fServAddr)); //reset

	if (atoi(name.Data()) > 0) /* le 1er caractere de ip_adrs est numerique */
	{
		fServAddr.sin_family = AF_INET;
		fServAddr.sin_addr.s_addr = inet_addr(name.Data());
	} else {
		host_addr = gethostbyname(name.Data());
		fServAddr.sin_family = host_addr->h_addrtype;
		memcpy((char *) &tmp, host_addr->h_addr, host_addr->h_length);
		fServAddr.sin_addr.s_addr = tmp;
	}

	fStatus = ACQ_OK;

	fSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (fSocket <= 0) {
		fError.TreatError(2, 0, "Socket call failed in GNetClientNarval");
		return;
	}

	fServAddr.sin_port = htons(GDevice::GetPort()); /* Server port */

	fIsOpen = true;
	fNewOpen = true;
}

//______________________________________________________________________________

void GNetClientNarval::Close() {
	MyClose();
}
//______________________________________________________________________________

void GNetClientNarval::Close(Int_t tempo) {
	MyClose(tempo);
}
//______________________________________________________________________________
void GNetClientNarval::MyClose(Int_t tempo) {
	//disconnection thanks to the following C function:
	//	int  acq_tcp_remote_action  (int *socket, int *code_env, int *len_env, char *tab_env, int *code_ret, int *lmax_ret, int *len_ret, char *tab_ret)
	//on ferme la connection avec le serveur si celle-ci est ouverte, car close peut tres bien avoir ete appelee par une methode precedemment

	fStatus = ACQ_OK;

	if (fIsOpen) {
		gettimeofday(&fMt_reference, &fTz);
		Int_t current_diff_time = (fMt_reference.tv_sec
				- fMt_permitclose.tv_sec);

		if (current_diff_time < tempo)
			return;
		close(fSocket);
		fSocket = 0;
		if (fStatus != ACQ_OK) {
			char msg[MAX_CARACTERES];

			fError.TreatError(
					2,
					0,
					"Occured during the connection attempt  (Automatically closing socket) ",
					msg);
		} else {
			if (fVerbose)
				fError.Infos("Disconnection :OK ");
		}
		fIsOpen = false;
		fNewOpen = false;
	}
}

//_________________________________________________________________________________

void GNetClientNarval::SetDevice(const char* _host) {
	//close the current connection with the previous server before changing the server name

	GDevice::SetDevice(_host);
	if (fStatus != ACQ_OK) {
		fError.TreatError(2, fStatus, "Changing name not valid");
		return;
	}
}
//_________________________________________________________________________________
void GNetClientNarval::SetPort(Int_t port) {
	//close the current connection with the previous server before changing the server port

	bool context = fIsOpen;
	if (fIsOpen == true) {
		Close(); // close the opened device before change file
	}

	GDevice::SetPort(port);
	if (context) {
		Open();
	}
}
//_________________________________________________________________________________
bool GNetClientNarval::GetBufferNarval(bool afficher) {
	// Get Narval Buffer
	// return true if data ok
	//
	
	TString tempos;
	fStatus = ACQ_OK;
	int size_to_recv;
	int received_data;
	int total_received_data;
	
	if (!fIsOpen) {
		Open();
		if (!fIsOpen) {
			fError.TreatError(1, 0, " any connected server ! ");
			fStatus = ACQ_NOTOPEN;
			return false;
		}
	}

	fTaille_Retour = 0;
	RazZone(fZoneData_char, fSizeZoneData);
	fNb_Buffers_Local = 0;

		if (connect(fSocket, (struct sockaddr *) &fServAddr, sizeof(fServAddr)) < 0) {
			fError.TreatError(2, 0, "Socket connection failed");
			fNb_Buffers_Local = 0;
			sleep(1);// wait 1 second if fail
			fBufferIn2p3->SetToCommentBuffer();
			fBufferMFM->RazBuffer();
			Close();
			fStatus = ACQ_DISCONNECT;
		}
	
	// read header containing size
	fSize_narval_buffer = 0;

		if ((sizeof(int32_t)) != (recv(fSocket, (char*)(&fSize_narval_buffer), sizeof(int32_t), 0))) {
			fError.TreatError(2, 0, "Bad read of size of Narval buffer");
			Close();
			return false;
		}
	

	if (fSize_narval_buffer == 0) {
		 Close();
		fStatus = ACQ_OK;
		if (fCurrent_diff_time_buff > 1000000)
			fCurrent_diff_time_buff = 1000000;
		if (fCurrent_diff_time_buff < 10)
			fCurrent_diff_time_buff = 10;
		if (fVerbose) {
			tempos.Form("Received empty data, Waiting time = %.1f ms",
					(Float_t)(fCurrent_diff_time_buff) / 1000);
			fError.TreatError(1, 0, tempos);
			fComptNotAvailable++;
		}
		return false;
	} else {
		fComptbuffOK++;
		gettimeofday(&fMt_meanbetw2buf, &fTz);
		fCurrent_diff_time_buff = ((int) (fMt_meanbetw2buf.tv_usec)
				- fLastTimeNarvalbuf);
		fLastTimeNarvalbuf = fMt_meanbetw2buf.tv_usec;
	}
	//	if (fComptbuffOK%10==0) cout << " Ok = "<< fComptbuffOK<<"  notavail = "<<fComptNotAvailable<<"\n";

	//ReallocBufferSize(&fZoneData_char,fSize_narval_buffer,fSizeZoneData, true) ;

	if (fSize_narval_buffer > fSizeZoneData) {
		if (fZoneData_char) {
			delete[] fZoneData_char;
			fZoneData_char = NULL;
		}
		fSizeZoneData = fSize_narval_buffer;
		fZoneData_char = new char[fSizeZoneData];
		//cout <<" debug  GNetClientNarval::GetBufferNarval fZoneData_char just after alloc = "<< (long int)(fZoneData_char)<<endl;
		RazZone(fZoneData_char, fSizeZoneData);
		tempos.Form("Resize of buffer for narval buffer from size %d to %d \n",
				fSizeZoneData, fSize_narval_buffer);
		fError.Infos(tempos);
		fZoneData_cur_char = fZoneData_char;
	}

	fSizeZoneData = fSize_narval_buffer;
	fZoneData_cur_char = fZoneData_char;
	total_received_data = 0;
	size_to_recv = fSize_narval_buffer;
	
        received_data = ReadAll(fSocket,fZoneData_cur_char,size_to_recv);
        total_received_data += received_data;
        if (received_data == -1) {
        	fNb_Buffers_Local = 0;
		fError.TreatError(2, errno, "Received data");
		perror(NULL);
		fStatus = ACQ_BUFALREADYGOT;
		Close();
		usleep(100);// wait if error error in receive
		return false;
	}

/*
	while (total_received_data < fSize_narval_buffer) {
		received_data = recv(fSocket, fZoneData_cur_char, size_to_recv, 0);
		total_received_data += received_data;

		if (fVerbose) {
			tempos.Form("received data %d total %d\n", received_data,
					total_received_data);
			fError.Infos(tempos);
		}
		if (received_data == -1) {
			fNb_Buffers_Local = 0;
			fError.TreatError(2, errno, "Received data");
			perror(NULL);
			fStatus = ACQ_BUFALREADYGOT;
			Close();
			usleep(100);// wait if error error in receive
			return false;
		}

		size_to_recv -= received_data;
		fZoneData_cur_char += received_data;
	}
	*/

	fZoneData_cur_char = fZoneData_char;

	fNarvalBufferEndReached = false;
	Close();
	fStatus = ACQ_OK;

	return true;

}
//_________________________________________________________________________________
int  GNetClientNarval::ReadAll(int sockfd, char*buff, int sizeToRead){
  int nleft = sizeToRead;
  int nread = 0;
  while(nleft > 0)
    {
      nread = recv(sockfd,buff,nleft,0);
      if(nread<0){
	fError.TreatError(2, 0, "Bad read  of Narval buffer");
      }else if(nread==0){
	fError.TreatError(2, 0, "Connection closed by client");
      }
      nleft -= nread;
      buff += nread;
    }    
  return (sizeToRead - nleft);
}

//_________________________________________________________________________________

void GNetClientNarval::GetBuffer(bool afficher) {
	//Get data buffer
	TString tempos;
	int countinsidebuffer = 0;
	fStatus = ACQ_OK;

	int buffer_size = 0;
	int type = 0;

	if (fNarvalBufferEndReached) {// if end is reached get a new buffer
		bool getbufferbool = GetBufferNarval(afficher);
		if (!getbufferbool) {
			//cout << " debug  GNetClientNarval::GetBuffer no buffer available -> return\n";
			return;
		}
		countinsidebuffer = 0;
		fZoneData_cur_char = fZoneData_char;
		//cout << " debug  GNetClientNarval::GetBuffer fZoneData_char= "
			//	<< (long int) (fZoneData_cur_char) << " with size "
				//<< fSizeZoneData << endl;
		//GetDumpRaw2((void*) fZoneData_cur_char, 32, fSize_narval_buffer);

		uint32_t * fZoneData_uint32 = (uint32_t *) fZoneData_char;
		if (((fZoneData_uint32[0] == 0) or (fZoneData_uint32[0] == 65535))
				and fSize_narval_buffer == 4) {
			// old Hello buffer =0000;
			fError.TreatError(0, 0, "Hello in old way Buffer in Narval!");
			fNarvalBufferEndReached = true;
			type = UNKNOWN_Idn;
			fBufferIn2p3->SetType(UNKNOWN_Idn);
			fStatus = UNKNOWN_Idn;

		} else {
			if (fSize_narval_buffer < 8) {
				// old Hello buffer =0000;
				fError.TreatError(0, 0,
						"Narval buffer with < Min size buffer (8) !");
				fNarvalBufferEndReached = true;
				fStatus = UNKNOWN_Idn;
			} else {
				fBufferIn2p3 ->SetExternalDataZone(fZoneData_cur_char, 8);
				type = fBufferIn2p3-> TestType(fZoneData_cur_char);
				fBufferIn2p3->SetType(type);

				if (fBufferIn2p3->IsAIn2p3Buffer()) {
					fTimestampsbuffersize = 0;
					fBuffer = fBufferIn2p3;
				} else {
					fBufferIn2p3 ->SetExternalDataZone(fZoneData_cur_char, 8);
					fBufferIn2p3-> TestType(fZoneData_cur_char + 12);
					fBufferIn2p3->SetType(fBufferIn2p3->TestType(
							fZoneData_cur_char + 12));
					if (fBufferIn2p3->IsAIn2p3Buffer()) {
						fTimestampsbuffersize = 12;
						fZoneData_cur_char = fZoneData_cur_char
								+ fTimestampsbuffersize;
						fBuffer = fBufferIn2p3;
					} else {
						fBufferMFM ->SetExternalDataZone(fZoneData_cur_char,
								8);
						fBufferMFM->SetType(fBufferMFM->TestType(
								fZoneData_cur_char));
						//cout <<" debug test type buffer = "<<fBufferMFM->TestType(fZoneData_cur_char)<<"n";
						if (fBufferMFM->IsAMFMBuffer()) {
							fTimestampsbuffersize = 0;
							fBuffer = fBufferMFM;
						} else {

							fStatus = ACQ_UNKBUF;
							TString tempos;
							tempos = Form("Buffer in narval buffer unknown, with type = %d!",
											type);
							fError.TreatError(2, 0, tempos.Data());
							fBufferMFM->DumpBuffer(64, 0);
							fNarvalBufferEndReached = true;
						}
					}
				}
			}
		}
	}//end of NarvalBufferEndReached

	fZoneData_cur_int = (int*) fZoneData_cur_char;
	countinsidebuffer++;
	switch (fStatus) {
	case ACQ_OK:
		if ((fBufferIn2p3)->IsAIn2p3Buffer()) {
			fBuffer = fBufferIn2p3;
			fBufferIn2p3->SetExternalDataZone(fZoneData_cur_char, fBufferSize);
			fBufferIn2p3->SetAttributs(true);
			buffer_size = fBuffer->GetBufSize();
			fZoneData_cur_char += buffer_size;
			fZoneData_cur_int = (int*) fZoneData_cur_char;
			//cout << " debug  GNetClientNarval::GetBuffer  In2p3 buffer no "<<fNb_Buffers_Local<< "/"<< "(buffer_size "<<buffer_size<<" / "<<"fSize_narval_buffer "<<fSize_narval_buffer<<") =" <<(int)(fSize_narval_buffer/buffer_size)<<"\n";
			if (buffer_size <= 0) {
				//cout <<"debug fNarvalBufferEndReached = true cause Buffer_size <= 0 \n";
				fNarvalBufferEndReached = true;
			}
			if (fZoneData_cur_char >= fZoneData_char + fSize_narval_buffer) {
				//	cout <<"debug fNarvalBufferEndReached = true cause  fZoneData_cur_char >= fZoneData_char + fSize_narval_buffer\n";
				fNarvalBufferEndReached = true;
			}
			fNb_Buffers_Local++;
			if (fBuffer->IsEventBuffer()) {
				if (fPremier_Buffer == 0)
					fPremier_Buffer = fBuffer->GetNumBuf();
				fNb_Buffers_Recus++;
				fNb_Buffers_Envoyes = fBuffer->GetNumBuf() - fPremier_Buffer
						+ 1;
				if (fLastNumBuff == fBuffer->GetNumBuf()) {
					//fError.TreatError(1, 0,
					//	"Same Buffer Number...so it may be same buffer...");
				}
				fLastNumBuff = fBuffer->GetNumBuf();
			}
		}

		if (((GBufferMFM*) fBufferMFM)->IsAMFMBuffer()) {
			fBuffer = fBufferMFM;
			fBufferMFM ->SetExternalDataZone(fZoneData_cur_char,
					MFM_BLOB_HEADER_SIZE);
			//buffer_size = fBufferMFM->GetBufSizeFromBuffer();
			//fBufferMFM->SetAttributs();
			buffer_size = fBufferMFM->GetBufSize();
			//cout <<" debug -------GetBuffer from Narval buffer with size = "<<buffer_size<<"\n";
			fBufferMFM->SetAttributs(true);
			fZoneData_cur_char += buffer_size;
			fZoneData_cur_int = (int*) fZoneData_cur_char;
			//	cout <<"debug  fZoneData_cur_data "<<(int*)fZoneData_cur_char<< "buffer_size"<<buffer_size<<"\n";
			if ((fZoneData_cur_char >= fZoneData_char + fSize_narval_buffer)
					or (buffer_size <= 0)) {
				fNarvalBufferEndReached = true;
				//cout <<" Debug fNarvalBufferEndReached buffer_size = "<<buffer_size<<" countinsidebuffer "<<countinsidebuffer <<" narval buff=" <<fSize_narval_buffer<<"\n";
				//	cout <<" Debug fSize_narval_buffer ="<<fSize_narval_buffer<<"  \n";
			}

			fNb_Buffers_Local++;
			if (fBufferMFM->IsEventBuffer()) {
				//fBufferMFM->DumpBuffer(256, 0);
				//cout <<" GNetClientNarval::GetBuffer fBufferMFM->GetNumBuf() ="<<fBufferMFM->GetNumBuf()<<"\n";
				if (fPremier_Buffer == 0)
					fPremier_Buffer = fBufferMFM->GetNumBuf();
				fNb_Buffers_Recus++;
				fNb_Buffers_Envoyes = fBufferMFM->GetNumBuf() - fPremier_Buffer
						+ 1;
				if (fLastNumBuff == fBufferMFM->GetNumBuf()) {
					//fError.TreatError(1, 0,
					//	"Same Buffer Number...so it may be same buffer...");
				}
				fLastNumBuff = fBufferMFM->GetNumBuf();
			}
		}
		break;
	case UNKNOWN_Idn:
		fStatus = ACQ_OK;
		break;

	default:
		if (afficher)
			fError.TreatError(1, 0, "Not a new buff");
	}
	return;
}

//_________________________________________________________________________________

void GNetClientNarval::ReadBuffer() {
	// Read a buffer
	fError.TreatDebug(10,0, "GNetClientNarval::ReadBuffer()");
	struct timeval mt_reference;
	struct timeval mt_Timeout;
	struct timezone tz;
	Int_t time_out = 10;
	gettimeofday(&mt_Timeout, &tz);
	gettimeofday(&mt_reference, &tz);
	fStatus = ACQ_OK;
	char tempos[128];
	while (true) {
		GetBuffer(false);
		gettimeofday(&mt_Timeout, &tz);

		if (((mt_reference.tv_sec - mt_Timeout.tv_sec) > time_out)) {
			sprintf(tempos , " Time out (%d seconds) with no event ",time_out);
			fError.TreatError(1, 0, tempos);
			break;
		}
		if (fStatus != ACQ_NOCURRCTRLBUF)
			break;
	};
	if (fBuffer) {
		if (fBuffer->GetType() != UNKNOWN_Idn)
			fBuffer->SetAttributs(true);
	}
}
//_________________________________________________________________________________
