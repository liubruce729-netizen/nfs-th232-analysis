// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GEvent
//
// This class manage event in Ganil format
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

#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <dirent.h>
#include "math.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <TROOT.h>
#include <TSystem.h>
#include <unistd.h>
#include "General.h"
#include "GEvent.h"
#include "GBufferIn2p3.h"

extern "C" {
#include "GEN_TYPE.H"
#include "STR_EVT.H"
#include "ERR_GAN.H"
#include "EQUIPDES.H"
#include "acq_swap_buf.h"
#include "gan_acq_buf.h"
#include "acq_codes_erreur.h"
#include "math.h"

}

#include <memory> // auto_ptr
#include <iostream>
#include <fstream>
#include <cstdlib>

int rd_evstr(char *sev_filnam, char *sev_memadd, int sev_memlen);
int s_evctrl(INT16 *evtbuf, INT16 *evtctrl, char *sev_memadd, int *pCtrlForm); // recontruction from brut event to ctrl event

int get_next_event(in2p3_buffer_struct *Buff, int Size, short int *Event,
		int SizeEvent, int *NumeroEvent, int *ReadEvtSize);

#include "acq_ebyedat_get_next_event.c"
#include "acq_mt_fct_ganil.c"
#include "s_evctrl.c"
#include "rd_evstr.c"

//_______________________________________________________________________________
ClassImp( GEvent);

GEvent::GEvent(DataParameters* parameter) :
	GEventBase(parameter) {
	//Constructor
	pNextEvent = NULL;
	fStatus = ACQ_OK;
	fEventNumber = -1;// important de le positionner à -1
	fEventCount = 0;

	fEvbsize = EVBSIZE;
	fEvcsize = EVCSIZE;
	pEventBrutData = NULL;
	fCtrlForm = EVCT_VAR;
	pEventBrut = new uint16_t[fEvbsize];
	pEventCtrl_0 = new uint16_t[fEvcsize];
	pEventCtrl = pEventCtrl_0;
	pStructEvent = new char[STRUCTEVENTSIZE];
	pTimeStampSubEvts = new long long[(int) (EVBSIZE / 4)];
	fNbSubEvt = 0;
	fLaw = 0;
	fEventCtrlCurrentSize = 0;
	fProba = 0.6;
	fSubEvent = new GSubEvent(GetDataParameters());
	SetRandomLaw(fLaw);
	SetRandomProba(fProba);
	fLipflop = true;
	fHeader_size = 0;
	fEvent_size = 0;

}

//_______________________________________________________________________________

GEvent::~GEvent() {
	//destructor
	//nt_t i;
	if (pEventBrut) {
		delete[] pEventBrut;
		pEventBrut = NULL;
	}

	if (pEventCtrl_0){
		delete []pEventCtrl_0;
		pEventCtrl_0   = NULL;
		pEventCtrl =NULL;
		}
	
	if (pStructEvent) {
		delete[] pStructEvent;
		pStructEvent = NULL;
	}
	if (pTimeStampSubEvts) {
		delete[] pTimeStampSubEvts;
		pTimeStampSubEvts = NULL;
	}

	if (fSubEvent) {
		delete fSubEvent;
		fSubEvent = NULL;
	}
}
//______________________________________________________________________________

void GEvent::SetRandomLaw(int law) {
	// set random law in case random generation ( 0 =  gauss, 1= white noise , 2 binomial);
	fLaw = law;
	if (fSubEvent) {
		fSubEvent->SetRandomLaw(fLaw);
	}
}
//______________________________________________________________________________

void GEvent::SetRandomProba(Float_t proba) {
	// set random probability in case random generation
	fProba = proba;
	if (fSubEvent) {
		fSubEvent->SetRandomProba(proba);
	}

}
//______________________________________________________________________________
int GEvent::NextEvent(GBuffer* _buffer) {
	// Read an event from  buffer _buffer
	// return status

	fEvt_increment = 0;
	fStatus = ACQ_OK;
	TString tempos;
	//cout << " debug "<< _buffer->fGBuf_type<<"\n";
	switch (_buffer->fGBuf_type) {  
	case EVENTDB_Idn:
		fEventCount++;
		ReadNextEvent_EVENTDB(((GBufferIn2p3*) _buffer));
		return (fStatus);
	case EVENTCT_Idn:
		fEventCount++;
		ReadNextEvent_EVENTCT(_buffer);
		return (fStatus);
	case EBYEDAT_Idn:
		fEventCount++;
		ReadNextEvent_EBEYEDAT(((GBufferIn2p3*) _buffer));
		return (fStatus);
	case ENDRUN_Idn:
		fError.TreatError(0, 0, " GEvent::NextEvent --ENDRUN--");
		return (fStatus);
	default:
		tempos.Form("Unknown header in GDevice::Next:%d", _buffer->fGBuf_type);
		fError.TreatError(1, 0, tempos);
		fStatus = ACQ_BUFTYPUNKNOWN;
		return (fStatus);
	}
}
//______________________________________________________________________________
int GEvent::ReadNextEvent_EVENTDB(GBuffer* _buffer) {
	// Utility routine to read events from EVENTDB buffers.
	fStatus = ACQ_OK;
	fEventReadSize = 0;
	pEventBrut_char = NULL;
	fTimeStamp = 0;

	if (!fIsStrFilePresent) {
		fError.TreatError(
				4,
				0,
				"ReadNextEvent_EVENTDB : Init event not done probably cause of no file ACTION_exp.CHC_STR");
	}
	if (!_buffer) {
			fError.TreatError(
					4,
					0,
					"ReadNextEvent_EVENTDB : Buffer Null");
		}
	//cout <<" debug _buffer size"<< _buffer->GetBufSize()<<"\n";
	fStatus = get_next_event(((GBufferIn2p3*) _buffer)->GetBuffer_map_in2p3(),
			((GBufferIn2p3*) _buffer)->GetBufSize(), (short*) pEventBrut,
			fEvbsize, &(fEventNumber), &fEventReadSize, &pEventBrut_char);
	fEventReadSize = fEventReadSize * 2; // in char size
	if (fStatus == ACQ_OK) {
		fStatus = s_evctrl((short*) pEventBrut, (short*) pEventCtrl,
				pStructEvent, &fCtrlForm);

		if (fStatus == GR_OK) {
			if (fCtrlForm != -1) {
				return (EventUnravelling());
			} else {
				fError.TreatError(2, fStatus, "Reconstruction with s_evctrl");
				return (false);
			}
		}
	} else if (fStatus == ACQ_ENDOFBUFFER) {
		fEventNumber = -1;
		return (fStatus);
	} else {

		fError.TreatError(2, fStatus, "in ReadNextEvent ");
		return (fStatus);
	}

	return (fStatus);
}
//______________________________________________________________________________
int GEvent::ReadNextEvent_EVENTCT(GBuffer* _buffer) {
	// PRIVATE
	// Utility routine to read events from buffers.;
	fEventReadSize = 0;
	pEventBrut_char = NULL;
	fStatus = ACQ_OK;
	fTimeStamp = 0;
	fStatus = get_next_event(((GBufferIn2p3*) _buffer)->GetBuffer_map_in2p3(),
			((GBufferIn2p3*) _buffer)->GetBufSize(), (short*) pEventCtrl,
			fEvcsize, &fEventNumber, &fEventReadSize, &pEventBrut_char);
	fEventReadSize = fEventReadSize * 2; // in char size
	if (fStatus == ACQ_OK) {
		if (fCtrlForm != -1)
			return (EventUnravelling());
	} else if (fStatus == ACQ_ENDOFBUFFER) {
		fEventNumber = -1;
		return (fStatus);
	} else {
		fError.TreatError(2, fStatus, "in ReadNextEvent (control event)");
		return (fStatus);
	}
	return (fStatus);
}

//______________________________________________________________________________
int GEvent::ReadNextEvent_EBEYEDAT(GBufferIn2p3* _buffer) {
	// PRIVATE
	// Utility routine to read events from buffers.
	fStatus = ACQ_OK;

	unsigned short len;

	fEventReadSize = 0;
	pEventBrut_char = NULL;
	fTimeStamp = 0;
    int nb=0;
	//fCtrlForm = EVCT_VAR or -1 in cas of raw event

	UNSINT16 * pCurrentEvent = NULL;
//printf( " pt event %lx \n",pEventCtrl);
	fStatus = acq_ebyedat_get_next_event_r(
			(UNSINT16*) _buffer->GetBuffer_map_in2p3(), &pEventCtrl,
			&fEventNumber, fCtrlForm, &pCurrentEvent, &pNextEvent, &fNbSubEvt,
			&pTimeStampSubEvts);

	// we get last time stamp of all subevent
	// usualy only one is good
	for (int nb = 0; nb < fNbSubEvt; nb++) {
		fTimeStamp = pTimeStampSubEvts[nb];
	}
	if (fStatus == ACQ_OK) {
		fEventHd = (EBYEDAT_EVENT_HD*) pCurrentEvent;
		len = (fEventHd->length);
		fEventReadSize = (int) len;
		fEventReadSize = fEventReadSize * 2; // in char size
		pEventBrut_char = (char*) pCurrentEvent;

		if (fCtrlForm != -1){
			nb = EventUnravelling();
			return (nb);
		}
	} else {
		fEventNumber = -1;
		if (fStatus == ACQ_ENDOFBUFFER) {
			return (fStatus);
		} else {
			fError.TreatError(2, fStatus, "in ReadNextEvent__EBEYEDAT ");
			return (fStatus);
		}
	}

	return (fStatus);
}
//______________________________________________________________________________
int GEvent::EventUnravelling(void) {
	// PRIVATE
	// If mode is variable length event, we have to reconstruct the Data buffer
	// from the given event.
	// WARNING: temporary the default: we dont check that it's really the case
	TString tempos;
	Int_t i;
	uint16_t index;
	fStatus = ACQ_OK;
	CTRL_EVENT *pCtrlEvent = (CTRL_EVENT *) pEventCtrl;
	uint16_t *brutData = (uint16_t*) (&(pCtrlEvent->ct_par));
	uint16_t current_label = 0;
	uint16_t parameter_value = 0;
	Int_t eventLength = pCtrlEvent->ct_len;
	eventLength = eventLength * 2; // in char size
	Int_t header_size = (Int_t)((char*) brutData - (char*) pCtrlEvent);
	Int_t scanLength = eventLength - header_size; // we remove fix size of control event header in char size

	fEventCtrlCurrentSize = scanLength / (sizeof(uint16_t));
	pEventBrutData = brutData;


	ClearData();

	for (i = 0; i < (Int_t)(scanLength / (sizeof(uint16_t))); i += 2) {
		current_label = 0;
		current_label = brutData[i];
		parameter_value = brutData[i + 1];

		//if(current_label > 0 && parameter_value >0 ) {
		if (current_label > 0) {
			index = GetDataParameters()->GetIndex(current_label);

			if (index <= fDataArraySize) {

				pDataArray[index] = parameter_value;// in all case we fill pDataArray[index] even in case of multiplicity to have information in case of dump for example
			} else // More on error handling would be cool
			{
				tempos.Form(
						"EventUnravelling : increment %d Label=%d Value=%d index overflow [%d] >=  SizeofArray = %d",
						i, ((int) current_label), ((int) parameter_value),
						((int) index), ((int) fDataArraySize));
				fError.TreatError(2, fStatus, tempos);
			}
		}//end if current_label
	}//end for

	return (fStatus);
}

//______________________________________________________________________________
void GEvent::EventInitWithFileName(char* actionFilePAR, char* actionFileSTR) {

	int structEvent_size = STRUCTEVENTSIZE;//bidon
	int local_status;

	if (fIsStrFilePresent) {
		local_status = rd_evstr(actionFileSTR, pStructEvent, structEvent_size);
		if (local_status != GR_OK) {
			TString tempo;
			tempo.Form(
					"Initialization of event structure: Event Struct :size = %lld Struc = %lld",
					(long long) structEvent_size, (long long) pStructEvent);
			fError.TreatError(2, fStatus, tempo);
			return;
		}
	}
	GEventBase::EventInitWithFileNameBase(actionFilePAR, actionFileSTR);
	return;
}


//______________________________________________________________________________
TString GEvent::GetDumpHeader() {
	return ((TString) "TODO");
}

//______________________________________________________________________________
TString GEvent::GetDumpEvent(char mode) {
	// Dump parameter index and value of  the current control event.

	Int_t i, j, maxbin, presentation = 0, max_presentation = 5;
	char tempo[MAX_CARACTERES*2] = "";
	char Bin[MAX_CARACTERES] = "";
	char Bin2[MAX_CARACTERES] = "";
	TString streturn;
	fStatus = ACQ_OK;
	CTRL_EVENT *pCtrlEvent = (CTRL_EVENT *) pEventCtrl;
	Short_t *brutData = &(pCtrlEvent->ct_par);
	Int_t eventLength = pCtrlEvent->ct_len;
	eventLength = eventLength * 2; // in char size
	Int_t header_size = (Int_t)((char*) brutData - (char*) pCtrlEvent);
	Int_t scanLength = eventLength - header_size; // in char size
	int value;
	int label;
	int reste;
	label = 0;
	value = 0;
	char one = '1', zero = '0';
	int DecNumber;

	if (mode == 'b')
		max_presentation = 3;

	if (fEventNumber == -1) {
		fError.TreatError(1, -1,
				" No Event , so no event dump. Get a new event buffer");
	} else {
		//cout << "-- Dumping Ctrl Event, nb :"<< fEventNumber<<" -nb par = "
		//	<<scanLength/2<<"-----(|parameter Label=value|)--\n";
		sprintf(
				tempo,
				"- Dumping Event, nb :%d --timestamps: %lld --Length = %d (in char size)----------\n",
				fEventNumber, fTimeStamp, scanLength / 2);
		streturn += tempo;

		for (i = 0; i < scanLength / 2; i += 2) {
			label = 0;
			value = 0;
			label = (uint16_t)(brutData[i]);
			value = (uint16_t)(brutData[i + 1]);
			sprintf(tempo, "\n");
			switch (mode) {
			case 'd':// decimal display
				sprintf(tempo, "|%5d = %5d", label, value);
				break;
			case 'o':// octal display
				sprintf(tempo, "|%5o = %5o", label, value);
				break;
			case 'h':// hexa display
				sprintf(tempo, "|%5x = %5x", label, value);
				break;
			case 'b':// binary display
				DecNumber = (int) value;
				Bin[0] = '\0';
				Bin[1] = zero;
				Bin2[1] = '\0';
				Bin2[0] = zero;
				j = 1;
				if (DecNumber != 0) {
					while (DecNumber >= 1) {
						reste = DecNumber % 2;
						DecNumber -= reste;
						DecNumber /= 2;
						if (reste)
							Bin[j] = one;
						else
							Bin[j] = zero;
						j++;
					}
					j--;
					maxbin = j;
					while (j >= 0) {
						Bin2[j] = Bin[maxbin - j];
						j--;
					}
				}
				sprintf(tempo, "|%5d = %16s", label, Bin2);
				break;
			}
			streturn += tempo;
			presentation++;
			if (presentation == max_presentation) {
				streturn += "|\n";
				presentation = 0;
			}
		}
		if (presentation != 0)
			streturn += "|\n";
	}
	return streturn;
}

//_________________________________________________________________________
//////////////// AJOUTS JOHN ////////////////////////////////////////////////
void GEvent::Connect(const Int_t index, uint16_t **p) const {
	// Connect a pointer to a data to the defined index in the Data Array

	if ((index < 0) || (index > fDataArraySize)) {
		cout << "Invalid connection:" << index << ". Valid only in 1<=index<="
				<< fDataArraySize << endl;
		return;
	}
	*p = &(pDataArray[index]);
}

//______________________________________________________________________________
Bool_t GEvent::Connect(const TString parName, uint16_t **p) const {
	// Connect a pointer to a data to a given parameter name in the Data Array

	string name = parName.Data();
	Int_t index = GetDataParameters()->GetIndex(name);
	if (index < 0) {
		//parameter not found
		return kFALSE;
	}
	Connect(index, p);
	return kTRUE;
}
//______________________________________________________________________________
uint16_t GEvent::GetArrayLabelValue_Label(uint16_t position) {
	// return label of control event at position "position"
	return ((GetArrayLabelValue())[position * 2]);
}
//______________________________________________________________________________
uint16_t GEvent::GetArrayLabelValue_Value(uint16_t position) {
	// return value of control event at position "position"
	return ((GetArrayLabelValue())[position * 2 + 1]);
}
//______________________________________________________________________________
void GEvent::RazEvent() {
	fEventNumber = -1;

	for (int i = 0; i < fEvent_size; i++) {
		pEventBrut[i] = 0;
	}
	//Clear();
}
//______________________________________________________________________________
int GEvent::GetNbofSubEvt() {
	return (fNbSubEvt);
}

//______________________________________________________________________________
void GEvent::MakeEventHeader(Char_t SystemId, Char_t statusWords,
		Char_t numberWords, Char_t formatType, Long64_t status,
		Long64_t number, Short_t size) {

	fEventNumber = number;

	Short_t startToken = (0xff) << 2;
	startToken = (startToken + statusWords) << 2;
	startToken = (startToken + numberWords) << 4;
	startToken += formatType;

	memcpy(&pEventBrut[0], &startToken, 2);
	memcpy(&pEventBrut[1], &fEvent_size, 2);

	Short_t statusList[3];
	statusList[0] = status >> 32;
	statusList[1] = status >> 16;
	statusList[2] = status;

	Short_t numberList[3];
	numberList[0] = number >> 32;
	numberList[1] = number >> 16;
	numberList[2] = number;

	switch (statusWords) {
	case 1:
		memcpy(&pEventBrut[2], &statusList[2], 2);
		break;

	case 2:
		memcpy(&pEventBrut[2], &statusList[1], 2);
		memcpy(&pEventBrut[3], &statusList[2], 2);
		break;

	case 3:
		memcpy(&pEventBrut[2], &statusList[0], 2);
		memcpy(&pEventBrut[3], &statusList[1], 2);
		memcpy(&pEventBrut[4], &statusList[2], 2);
		break;
	}

	switch (numberWords) {
	case 1:
		memcpy(&pEventBrut[statusWords + 2], &numberList[2], 2);
		break;

	case 2:
		memcpy(&pEventBrut[statusWords + 2], &numberList[1], 2);
		memcpy(&pEventBrut[statusWords + 3], &numberList[2], 2);
		break;

	case 3:
		memcpy(&pEventBrut[statusWords + 2], &numberList[0], 2);
		memcpy(&pEventBrut[statusWords + 3], &numberList[1], 2);
		memcpy(&pEventBrut[statusWords + 4], &numberList[2], 2);
		break;
	}
}
//______________________________________________________________________________
void GEvent::FillEvent(Char_t systemId, Char_t statusWords, Char_t numberWords,
		Char_t clockWords, Char_t format, Long64_t status, Long64_t number,
		Int_t** subTabsOfIndex, Int_t nbOfSubEvents) {

	Int_t sizeOfSubTabs = (GetDataParameters()->GetNbParameters()
			/ nbOfSubEvents) + 1;
          
	fNbSubEvt = nbOfSubEvents;
	fStatus = status;
	fEventNumber = number;

	fHeader_size = statusWords + numberWords + 2;

	pEventBrutData = pEventBrut + fHeader_size;

	fEvent_size = fHeader_size;

	fSubEvent->SetSystem_id(0);
	fSubEvent->SetClockWords(0);
	fSubEvent->SetStatusWords(0);
	fSubEvent->SetNumberWords(0
	);
	fSubEvent->SetFormatType(1);

	for (short i = 0; i < fNbSubEvt; ++i) {
		fSubEvent->MakeSubEvent(46748, 0, 4654, subTabsOfIndex[i],
				sizeOfSubTabs);
		memcpy(&pEventBrut[fEvent_size], fSubEvent->GetSubEventBrut(),
				fSubEvent->GetSize() * 2);
		fEvent_size += fSubEvent->GetSize();
	}
	// finish header of event with computer informations
	MakeEventHeader(systemId, statusWords, numberWords, format, status, number,
			fEvent_size);

}

//______________________________________________________________________________
void GEvent::DumpMadeEvent() {
	Short_t statusWords = (pEventBrut[0] & 0xC0) >> 6;
	Short_t numberWords = (pEventBrut[0] & 0x30) >> 4;
	Short_t formatType = (pEventBrut[0] & 0x0F);
	Short_t event_size = pEventBrut[1];

	cout << "Event Status Words : " << statusWords << endl;
	cout << "Event Number Words : " << numberWords << endl;
	cout << "Event Format Type : " << formatType << endl;
	cout << "Event Size of Event : " << event_size << endl;

	cout << endl;

	switch (statusWords) {
	case 1:
		cout << "1st event status word : " << pEventBrut[2] << endl;
		break;

	case 2:
		cout << "1st event status word : " << pEventBrut[2] << endl;
		cout << "2nd event status word : " << pEventBrut[3] << endl;
		break;

	case 3:
		cout << "1st event status word : " << pEventBrut[2] << endl;
		cout << "2nd event status word : " << pEventBrut[3] << endl;
		cout << "3rd event status word : " << pEventBrut[4] << endl;
	}

	cout << endl;

	switch (numberWords) {
	case 1:
		cout << "1st event number word : " << pEventBrut[statusWords + 2]
				<< endl;
		break;

	case 2:
		cout << "1st event number word : " << pEventBrut[statusWords + 2]
				<< endl;
		cout << "2nd event number word : " << pEventBrut[statusWords + 3]
				<< endl;
		break;

	case 3:
		cout << "1st event number word : " << pEventBrut[statusWords + 2]
				<< endl;
		cout << "2nd event number word : " << pEventBrut[statusWords + 3]
				<< endl;
		cout << "3rd event number word : " << pEventBrut[statusWords + 4]
				<< endl;
	}

	cout << endl;
	uint16_t startToken;
	int currentPosition = 0, subEventNumberWords, subEventStatusWords,
			subEventClockWords;
	for (int i = 0; i < fNbSubEvt; ++i) {
		DumpSubEvent(currentPosition);
		startToken = pEventBrutData[currentPosition + 0];

		subEventNumberWords = (startToken >> 4) & 0x0003;
		subEventStatusWords = (startToken >> 6) & 0x0003;
		subEventClockWords = (startToken >> 8) & 0x0003;

		currentPosition += pEventBrutData[currentPosition + subEventClockWords
				+ subEventStatusWords + subEventNumberWords + 1];
	}

}

//______________________________________________________________________________

void GEvent::DumpSubEvent(int currentPosition) {
	//Read written datas to verify values

	if (pEventBrut == NULL) {
		fError.TreatError(2, 0,
				"No sub-event , so no dump. You would do a MakeSubEvent");
	}

	else {

		//Value of StartToken
		uint16_t startToken = pEventBrutData[currentPosition + 0];
		cout << "sub-event start token : " << startToken << endl;

		short detectorSysId, clockWords, statusWords, numberWords, formatType,
				header_size;
		int numberOfData;
		formatType = startToken & 0x000F;
		numberWords = (startToken >> 4) & 0x0003;
		statusWords = (startToken >> 6) & 0x0003;
		clockWords = (startToken >> 8) & 0x0003;
		detectorSysId = (startToken >> 10) & 0x003F;
		header_size = clockWords + statusWords + numberWords + 2;
		numberOfData = (pEventBrutData[currentPosition + 1 + clockWords
				+ statusWords + numberWords] - header_size) / 2;

		cout << "sub-event detector system Id : " << detectorSysId << endl;
		cout << "sub-event clock words :  : " << clockWords << endl;
		cout << "sub-event status words : " << statusWords << endl;
		cout << "sub-event number words : " << numberWords << endl;
		cout << "sub-event format type : " << formatType << "\n" << endl;

		//Size of sub-event
		cout << "sub-event size : " << pEventBrutData[currentPosition
				+ clockWords + statusWords + numberWords + 1] << endl;
		cout << "sub-event header size : " << header_size << endl;
		cout << "sub-event number of couples Label/Data : " << numberOfData
				<< "\n" << endl;

		//Values of each word of clock
		switch (clockWords) {
		case 1:
			cout << "1st sub-event clock word : "
					<< pEventBrutData[currentPosition + 1] << endl;
			break;

		case 2:
			cout << "1st sub-event clock word : "
					<< pEventBrutData[currentPosition + 1] << endl;
			cout << "2nd sub-event clock word : "
					<< pEventBrutData[currentPosition + 2] << endl;
			break;

		case 3:
			cout << "1st sub-event clock word : "
					<< pEventBrutData[currentPosition + 1] << endl;
			cout << "2nd sub-event clock word : "
					<< pEventBrutData[currentPosition + 2] << endl;
			cout << "3rd sub-event clock word : "
					<< pEventBrutData[currentPosition + 3] << endl;
		}
		cout << endl;

		//Values of each word of status
		switch (statusWords) {
		case 1:
			cout << "1st sub-event status word : "
					<< pEventBrutData[currentPosition + clockWords + 1] << endl;
			break;

		case 2:
			cout << "1st sub-event status word : "
					<< pEventBrutData[currentPosition + clockWords + 1] << endl;
			cout << "2nd sub-event status word : "
					<< pEventBrutData[currentPosition + clockWords + 2] << endl;
			break;

		case 3:
			cout << "1st sub-event status word : "
					<< pEventBrutData[currentPosition + clockWords + 1] << endl;
			cout << "2nd sub-event status word : "
					<< pEventBrutData[currentPosition + clockWords + 2] << endl;
			cout << "3rd sub-event status word : "
					<< pEventBrutData[currentPosition + clockWords + 3] << endl;
		}
		cout << endl;

		//Values of each word of number
		switch (numberWords) {
		case 1:
			cout << "1st sub-event number word : "
					<< pEventBrutData[currentPosition + clockWords
							+ statusWords + 1] << endl;
			break;

		case 2:
			cout << "1st sub-event number word : "
					<< pEventBrutData[currentPosition + clockWords
							+ statusWords + 1] << endl;
			cout << "2nd sub-event number word : "
					<< pEventBrutData[currentPosition + clockWords
							+ statusWords + 2] << endl;
			break;

		case 3:
			cout << "1st sub-event number word : "
					<< pEventBrutData[currentPosition + clockWords
							+ statusWords + 1] << endl;
			cout << "2nd sub-event number word : "
					<< pEventBrutData[currentPosition + clockWords
							+ statusWords + 2] << endl;
			cout << "3rd sub-event number word : "
					<< pEventBrutData[currentPosition + clockWords
							+ statusWords + 3] << endl;
		}

		cout << endl;

		//All couples of label/data
		for (int i = 0; i < numberOfData; ++i) {
			cout << "label : " << pEventBrutData[currentPosition + header_size
					+ i * 2] << " value : " << pEventBrutData[currentPosition
					+ header_size + i * 2 + 1] << endl;
		}
	}
	cout << endl;
}

//______________________________________________________________________________

void GEvent::FillBufferWithEvent(GBuffer* buffer) {
	//Copy a ctrl ebyedat event in buffer

	Int_t currentEventSize = pEventBrut[1] * 2; //pEventBrut[1] = size of Event in 16bits words


	memcpy(((buffer->fGBuf_data) + buffer->GetUsedEventsSize()
			+ GANIL_BUF_HD_SIZE), pEventBrut, currentEventSize); // GANIL_BUF_HD_SIZE  header is not included
	//if (count!=currentEventSize) fError.TreatError(2,0,"GEvent::FillBufferWithEvent pb with filling buffer.");
	buffer->SetUsedEventsSize(buffer->GetUsedEventsSize() + currentEventSize);
}
//______________________________________________________________________________

void GEvent::FillBufferWithRawEvent(GBuffer* buffer) {
	//Copy a raw ebyedat event in buffer


	Int_t currentEventSize = GetEventDataSize();
	//cout <<" GetUsedEventsSize="<<GetEventDataSize()<<" currentEventSize="<<currentEventSize;
	//cout <<" GetEventDataChar()="<<(int*)GetEventDataChar()<<" buffer->GetUsedEventsSize()="<< buffer->GetUsedEventsSize()<<"\n";

	memcpy(((buffer->fGBuf_data) + buffer->GetUsedEventsSize()
			+ GANIL_BUF_HD_SIZE), GetEventDataChar(), currentEventSize); // GANIL_BUF_HD_SIZE  header is not included
	buffer->SetUsedEventsSize(buffer->GetUsedEventsSize() + currentEventSize);

}
//______________________________________________________________________________

void GEvent::GetDumpRawEvent() {
	GetDumpRaw(GetEventDataChar(), GetEventDataSize(), 0);
}
//______________________________________________________________________________

bool GEvent::IsRaz() {
	if (fEventNumber == -1)
		return true;
	else
		return false;
}

//______________________________________________________________________________

int GEvent::GetUsedSize() {
	if (fEventNumber == -1)
		return 0;
	else
		return (pEventBrut[1] * 2); //pEventBrut[1] is the size of Event in 16bits words

}

////////////////////////////////////////fin /////////////////////////////////////
