// File : GBufferIn2p3.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GBufferIn2p3
//
// This class manage buffer in Ganil format
// The associated methods do dump....
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

#include <stdio.h>
#include <string>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "GBufferIn2p3.h"
#include "acq_codes_erreur.h"

using namespace std;

ClassImp ( GBufferIn2p3);

GBufferIn2p3::GBufferIn2p3(int _bufsize) {
	// constructor of GBufferIn2p3
	fGBuf_data = NULL;
	fGBuf_index = 0;//
	fGBuf_increment = 0; // use in screen dump to memory where we are in dump
	fVerbose = 0; // use to  debug ( =0 no debug information, =10 all)
	if (_bufsize == 0)
		fGBuf_size = BUFSIZE;
	else
		fGBuf_size = _bufsize; // Buffer size

	fGBuf_data = new char[fGBuf_size];// raw data
	fIsAEventType = false; // flag  to indicate if the buffer contain events
	fGBuf_type = UNKNOWN_Idn;
	fGBuf_header[0] = '\0';
	if (fVerbose > 1)
		cout << " buffer construction \n";
	strcpy(fDate, "");
	fRunNumber = 0;
	fIsALocalAlloc = true;
	//fBuffer_map_in2p3 = (in2p3_buffer_struct*) fGBuf_data;// pointer on differents structure in in2p3 format

	fUsedEventsSize = 0;

}
//_______________________________________________________________________________
GBufferIn2p3::~GBufferIn2p3() {
	//destructor of GBufferIn2p3

	if ((fGBuf_data) && fIsALocalAlloc) {
		delete[] (fGBuf_data);
		fGBuf_data = NULL;
	}
	if (fVerbose > 0)
		cout << "Delete buffer." << "endl";
}

//___________________________________________________________________

int GBufferIn2p3::GetNumRun_() {
	// get number of run from the second buffer of a run
	int NumRun = -1;
	char tempo[MAX_CARACTERES];
	memcpy(tempo, GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii + 61, 8);
	//  memcpy(tempo,GetBuffer_map_in2p3()->cas.Buf_eventh.NumeroRun,8); // a tester
	tempo[8] = '\0';
	NumRun = atoi(tempo);
	fRunNumber = NumRun;
	return NumRun;
}
//___________________________________________________________________
int GBufferIn2p3::GetNumBuf() { //get number of current buffer
	return (GetBuffer_map_in2p3()->les_donnees.Count);
}

//____________________________________________________________________
int GBufferIn2p3::GetBufSizeFromBuffer() {
	// method to get bufsize  form the first buffer of a run

	int size = -1;
	char tempo[MAX_CARACTERES];
	memcpy(tempo, GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii,
			MAX_CARACTERES);
	memcpy(tempo, GetBuffer_map_in2p3()->les_donnees.cas.Buf_fileh.TailleBlocs,
			8);
	tempo[8] = '\0';
	size = atoi(tempo);
	SetReadSize(size);
	//if ((size!=512)&&(size!=1024)&&(size!=2048)&&(size!=4096)&&(size!=8192)&&(size!=16384)&&(size!=32768)&&(size!=65536)) size =-1;
	return size;
}
//__________________________________________________________________
char* GBufferIn2p3::GetDate_() {
	// method to get date form the second buffer of a run
	char tempo[MAX_CARACTERES + 8] = "            ";
	char *p_deb = NULL, *p_fin, *p_sep;
	tempo[MAX_CARACTERES + 7] = '\0';
	memcpy(tempo, GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii,
			MAX_CARACTERES);
	// find first '-' in date (dd-mm-yy) */
	p_deb = strchr(tempo, '-');

	if (p_deb)
		p_deb -= 2;

	// find first ':' or 'anti/' in time (hh:mm:ss)
	p_fin = strtok((char*) tempo, ":\\");
	if (p_fin) {
		p_fin += 6;
		*p_fin = '\0';
	}

	// change separator between date and time */
	p_sep = strchr(p_deb, ' ');
	if (p_sep)
		*p_sep = '_';

	if (fVerbose > 0)
		cout << "\n\t Date = " << p_deb << " len = " << strlen(p_deb) << "  \n";

	strcpy(fDate, p_deb);
	return fDate;
}
//____________________________________________________________________
bool GBufferIn2p3::IsAIn2p3Buffer() {
	// return true if is a IN2P3 buffer
	bool test;
	test =    ((fGBuf_type == EVENTDB_Idn)   || (fGBuf_type == EVENTDB_SWAP_Idn)
			|| (fGBuf_type == EVENTCT_Idn)   || (fGBuf_type == EVENTCT_SWAP_Idn)
			|| (fGBuf_type == EBYEDAT_Idn)   || (fGBuf_type == RAWDT32_Idn)
			|| (fGBuf_type == JBUS_SWAP_Idn) || (fGBuf_type == JBUS_Idn)
			|| (fGBuf_type == FILEH_Idn)     || (fGBuf_type == EVENTH_Idn)
			|| (fGBuf_type == COMMENT_Idn)   || (fGBuf_type == PARAM_Idn)
			|| (fGBuf_type == ENDRUN_Idn)    || (fGBuf_type == SCALER_Idn));
	return test;
}
//____________________________________________________________________
Int_t GBufferIn2p3::TestType(char * pt) {
	//test string on 8 char to determine buffer type
	TString tempos;
	tempos.Form( "GBufferIn2p3::TestType Buffer pointer is null %ld  ",(long int )pt);
	if ( pt == NULL ) fError.TreatError(5,0, tempos );
	tempos.Form( "GBufferIn2p3::TestType,  Size buffer is not enough long to do correct test ");
    if (fGBuf_size< IN2P3_HEADER_SIZE)  fError.TreatError(1,0, tempos );

	int type;
	if (strncmp(pt, EBYEDAT_Id, IN2P3_HEADER_SIZE) == 0) {
		type = EBYEDAT_Idn;
	} else if (strncmp(pt, EVENTDB_Id, IN2P3_HEADER_SIZE) == 0) {
		type = EVENTDB_Idn;
	} else if (strncmp(pt, EVENTDB_SWAP_Id, IN2P3_HEADER_SIZE) == 0) {
		type = EVENTDB_Idn;
	} else if (strncmp(pt, EVENTCT_Id, IN2P3_HEADER_SIZE) == 0) {
		type = EVENTCT_Idn;
	} else if (strncmp(pt, EVENTCT_SWAP_Id, IN2P3_HEADER_SIZE) == 0) {
		type = EVENTCT_Idn;
	} else if (strncmp(pt, SCALER_Id, IN2P3_HEADER_SIZE) == 0) {
		type = SCALER_Idn;
	} else if (strncmp(pt, SCALER_SWAP_Id, IN2P3_HEADER_SIZE) == 0) {
		type = SCALER_Idn;
	} else if (strncmp(pt, STATUS_Id, IN2P3_HEADER_SIZE) == 0) {
		type = STATUS_Idn;
	} else if (strncmp(pt, STATUS_SWAP_Id, IN2P3_HEADER_SIZE) == 0) {
		type = STATUS_Idn;
	} else if (strncmp(pt, JBUS_Id, IN2P3_HEADER_SIZE) == 0) {
		type = JBUS_Idn;
	} else if (strncmp(pt, JBUS_SWAP_Id, IN2P3_HEADER_SIZE) == 0) {
		type = JBUS_Idn;
	} else if (strncmp(pt, RAWDT32_Id, IN2P3_HEADER_SIZE) == 0) {
		type = RAWDT32_Idn;
	} else if (strncmp(pt, CONFIG_Id, IN2P3_HEADER_SIZE) == 0) {
		type = CONFIG_Idn;
	} else if (strncmp(pt, INFODAT_Id, IN2P3_HEADER_SIZE) == 0) {
		type = INFODAT_Idn;
	} else if (strncmp(pt, COMMENT_Id, IN2P3_HEADER_SIZE) == 0) {
		type = COMMENT_Idn;
	} else if (strncmp(pt, FILEH_Id, IN2P3_HEADER_SIZE) == 0) {
		type = FILEH_Idn;
	} else if (strncmp(pt, PARAM_Id, IN2P3_HEADER_SIZE) == 0) {
		type = PARAM_Idn;
	} else if (strncmp(pt, EVENTH_Id, IN2P3_HEADER_SIZE) == 0) {
		type = EVENTH_Idn;
	} else if (strncmp(pt, ENDRUN_Id, IN2P3_HEADER_SIZE) == 0) {
		type = ENDRUN_Idn;
	} else {
		type = UNKNOWN_Idn;
	}
	return type;
}
//____________________________________________________________________
void GBufferIn2p3::SetAttributs(bool quiet) {
	// used only in ReadBuffer() method
	// set few attributs after a buffer is read on device

	fGBuf_type = TestType(fGBuf_data);
	fIsAIn2p3Type = IsAIn2p3Buffer();
	fIsAEventType = IsAEventBuffer();
	if (fIsAIn2p3Type == false){
		if (!quiet)
		fError.TreatError(1, 0, " Not a In2p3 Buffer !");
		return;
	}
	if (fGBuf_type ==FILEH_Idn){
		GetBufSizeFromBuffer();
	}
	if (fGBuf_type ==EVENTH_Idn){
			GetDate_();
			GetNumRun_();
	}
	strncpy(fGBuf_header, GetBuffer_map_in2p3()->les_donnees.Ident,
			IN2P3_HEADER_SIZE);
	fGBuf_header[IN2P3_HEADER_SIZE] = '\0';
	fGBuf_index = (int) (GetBuffer_map_in2p3()->les_donnees.Count);
}
//____________________________________________________________________
void GBufferIn2p3::MyDelete() {
	delete[] fGBuf_data;
	fGBuf_data = NULL;
}
//____________________________________________________________________
int GBufferIn2p3::GetBufSizeFromFILEH() {
	// method to get bufsize  form the first buffer of a run

	int size = -1;
	char tempo[MAX_CARACTERES];
	memcpy(tempo, GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii,
			MAX_CARACTERES);
	memcpy(tempo, GetBuffer_map_in2p3()->les_donnees.cas.Buf_fileh.TailleBlocs,
			8);
	tempo[8] = '\0';
	size = atoi(tempo);

	//if ((size!=512)&&(size!=1024)&&(size!=2048)&&(size!=4096)&&(size!=8192)&&(size!=16384)&&(size!=32768)&&(size!=65536)) size =-1;
	return size;
}

//____________________________________________________
int GBufferIn2p3::GetHeaderSize() {
	return (IN2P3_HEADER_SIZE);
}
//____________________________________________________
void GBufferIn2p3::Equal(GBuffer& buf1) {
	Equal(buf1);
}
//____________________________________________________
void GBufferIn2p3::Equal(GBuffer* buf1) {
	// buffer copy method
	//Exemple
	//>> Buf1.GBequal(Buf2)
	// all data from buffer Buf1 are copied in buffer Buf2

	memcpy(((GBufferIn2p3*) buf1)->fGBuf_data, fGBuf_data, fGBuf_size);
	strcpy(((GBufferIn2p3*) buf1)->fGBuf_header, fGBuf_header);
	buf1->fGBuf_increment = fGBuf_increment;
	buf1->fGBuf_index = fGBuf_index;
	buf1->fGBuf_type = fGBuf_type;
	buf1->SetVerbose(fVerbose);
	buf1->fRunNumber = fRunNumber;
	strcpy(buf1->fDate, fDate);
	//  strcpy (buf1.fGBuf_ascii,fGBuf_ascii);
}
//____________________________________________________________________
int GBuffer::GetHeaderSize() {
	return (IN2P3_HEADER_SIZE);
}

//____________________________________________________________________
void GBufferIn2p3::SetToCommentBuffer() {
	// return true if it is a header buffer
	fGBuf_type = COMMENT_Idn;
	fIsAEventType = false;
	if (fGBuf_data){
	for (int i = 0; i > fGBuf_size; i++) {
		fGBuf_data[i] = ' ';
	}

	//fGBuf_index =0;
	//fBuffer_map_in2p3->les_donnees.Count = UNSINT32 (0);

	// fGBuf_index  = (int)(fBuffer_map_in2p3->les_donnees.Count);
	strncpy(fGBuf_data, " COMMENT", IN2P3_HEADER_SIZE);
}

	return;
}
;
//__________________________________________________________________
void GBufferIn2p3::ChangeRunNumber(int newnumber) {
	if ((newnumber != 0) && (fGBuf_type == EVENTH_Idn)) {
		char tempo0[17];
		char tempo1[17];
		char tempo2[17];
		char NomRun[17]; /* Nom du run                                */
		char NumeroRun[9]; /* Numero du run                             */

		sprintf(tempo1, " RUN =%5d     ", newnumber);
		sprintf(tempo0, "%d", newnumber);
		int lnnb = strlen(tempo0);

		memcpy(tempo2, " ", 1);
		memcpy(tempo2 + 1, "0000000", 7 - lnnb);
		memcpy(tempo2 + 7 - lnnb + 1, tempo0, lnnb);
		memcpy(NomRun, tempo1, 16);
		memcpy(NomRun + 16, "\0", 1);
		memcpy(NumeroRun, tempo2, 8);
		memcpy(NumeroRun + 8, "\0", 1);
		fError.TreatError(0, 1, "Change of Run Number :", NomRun);
		// cout << "-"<<NomRun<<"-\n";
		// cout << "-"<<NumeroRun<<"-\n";
		memcpy(GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii + 44, NomRun,
				16);
		memcpy(GetBuffer_map_in2p3()->les_donnees.cas.Buf_ascii + 60,
				NumeroRun, 8);

	}
}

//_______________________________________________________________________________

void GBufferIn2p3::MakeEBYEDATHeader(UInt_t blockNumber, Short_t sourceId,
		Short_t destinationId, Short_t dataStreamNumber, Short_t numOfEvents) {// write header and end of EBYEDAT buffer
	fGBuf_type = EBYEDAT_Idn;
	fIsAEventType = true;

	Int_t usedSizeByWords = (fUsedEventsSize + 2) / 2;
	char* buf_data_current = NULL; // current pointer on buffer
	Int_t null = 0;
	Int_t end = 0xff00;
	Int_t magicNumber = MAGIC_NUMBER;
	buf_data_current = fGBuf_data;

	memcpy(buf_data_current, &" EBYEDAT", 8); //
	buf_data_current += 8;
	memcpy(buf_data_current, &blockNumber, 4);
	buf_data_current += 4;
	memcpy(buf_data_current, &magicNumber, 4);
	buf_data_current += 4;
	memcpy(buf_data_current, &sourceId, 2);
	buf_data_current += 2;
	memcpy(buf_data_current, &destinationId, 2);
	buf_data_current += 2;
	memcpy(buf_data_current, &dataStreamNumber, 2);
	buf_data_current += 2;
	memcpy(buf_data_current, &numOfEvents, 2);
	buf_data_current += 2;
	memcpy(buf_data_current, &null, 4);
	buf_data_current += 4;
	memcpy(buf_data_current, &usedSizeByWords, 4);
	buf_data_current += 4;

	//if (((int)buf_data_current-(int)fGBuf_data)!= GANIL_BUF_HD_SIZE ) fError.TreatError(2,0,"Write EBYEDAT header Error");
	if (((int) (buf_data_current - fGBuf_data)) != GANIL_BUF_HD_SIZE)
		fError.TreatError(2, 0, "Write EBYEDAT header Error");

	buf_data_current = fGBuf_data + fUsedEventsSize + GANIL_BUF_HD_SIZE;
	memcpy(buf_data_current, &end, 2);
	buf_data_current += 2;
	memcpy(buf_data_current, &null, 2);
	buf_data_current += 2;

}
//_______________________________________________________________________________

void GBufferIn2p3::MakeEndRunHeader(UInt_t blockNumber) {// write header and end of EBYEDAT buffer
	fGBuf_type = ENDRUN_Idn;
	fIsAEventType = false;


	Int_t magicNumber = MAGIC_NUMBER;
	char* buf_data_current = NULL; // current pointer on buffer
	buf_data_current = fGBuf_data;

	memcpy(buf_data_current, &" ENDRUN ", 8); //
	buf_data_current += 8;
	memcpy(buf_data_current, &blockNumber, 4);
	buf_data_current += 4;
	memcpy(buf_data_current, &magicNumber, 4);
	buf_data_current += 4;


}
//_______________________________________________________________________________


TString GBufferIn2p3::GetDumpBufferHeader() {
	TString mydump;
	TString tempos;
	Char_t type[9];

	memcpy(type, fGBuf_data, 8);
	type[8] = 0;

	Int_t blockNumber;
	memcpy(&blockNumber, fGBuf_data + 8, 4);

	Int_t magicNumber;
	memcpy(&magicNumber, fGBuf_data + 12, 4);

	Short_t sourceId;
	memcpy(&sourceId, fGBuf_data + 16, 2);

	Short_t destinationId;
	memcpy(&destinationId, fGBuf_data + 18, 2);

	Short_t dataStreamNumber;
	memcpy(&dataStreamNumber, fGBuf_data + 20, 2);

	Short_t numOfEvents;
	memcpy(&numOfEvents, fGBuf_data + 22, 2);

	Int_t checkSum;
	memcpy(&checkSum, fGBuf_data + 24, 4);

	Int_t lenght;
	memcpy(&lenght, fGBuf_data + 28, 4);

	tempos.Form("type : %s    ", type);
	mydump = tempos;
	tempos.Form("block number : %d\n", blockNumber);
	mydump += tempos;
	tempos.Form("magic number : %d   ", magicNumber);
	mydump += tempos;
	tempos.Form("source id : %d    ", sourceId);
	mydump += tempos;
	tempos.Form("destination id : %d\n", destinationId);
	mydump += tempos;
	tempos.Form("data stream number : %d    ", dataStreamNumber);
	mydump += tempos;
	tempos.Form("number of first events : %d\n", numOfEvents);
	mydump += tempos;
	tempos.Form("checksum : %d   ", checkSum);
		mydump += tempos;
	tempos.Form("lenght : %d\n", lenght);
	mydump += tempos;

	return mydump;
}

////////////////////////////////////////fin /////////////////////////////////////
