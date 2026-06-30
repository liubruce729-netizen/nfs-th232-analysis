// File : GSubEvent.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GSubEvent
//
// This class manage sub-event in Ganil format				
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
#include <stdlib.h>
#include "General.h"
#include "GSubEvent.h"

extern "C" {
#include "ERR_GAN.H"
#include "acq_codes_erreur.h"

#include "math.h"
}

ClassImp( GSubEvent);

GSubEvent::GSubEvent(DataParameters* _Parameter) {
	//Constructor 

	fParameter = _Parameter;
	fSystemId = -1;
	fClockWords = -1;
	fStatusWords = -1;
	fSubEventNumberWords = -1;
	fFormatType = -1;
	fClock = -1;
	fStatus = -1;
	fSubEventNumber = -1;
	fSubEvent_size = 0;
	fHeader_size = 0;
	fLaw = 0;
	fProba = 0.6;
	fRand = NULL;
	randSeed = 0;
	fNbDatas = 0;
	fSubEventBrut = new UShort_t[STRUCTEVENTSIZE / 2];
	fSubEventBrutData = NULL;

}
//_______________________________________________________________________________
GSubEvent::~GSubEvent() {
	//destructor 

	if (fSubEventBrut) {
		delete[] fSubEventBrut;
		fSubEventBrutData = NULL;
	}
	if (fRand) {
		delete fRand;
		fRand = NULL;
	}
}

//______________________________________________________________________________
void GSubEvent::ClearData(Int_t value = 0) {
	//Clear Physical Data  Array or give a fixe value ( default =0);

	for (int i = 0; i < fSubEvent_size; ++i) {
		fSubEventBrut[i] = value;
	}
}
//______________________________________________________________________________
void GSubEvent::RazSubEvent() {
	fSubEventNumber = -1;
}
//______________________________________________________________________________
Int_t GSubEvent::GetNbofDatas() {
	return (fNbDatas);
}

//______________________________________________________________________________
void GSubEvent::SetSystem_id(Char_t system_id) {
	fSystemId = system_id;
}

//______________________________________________________________________________
void GSubEvent::SetNumberWords(Char_t numberWords) {
	fSubEventNumberWords = numberWords;
}

//______________________________________________________________________________
void GSubEvent::SetClockWords(Char_t clockWords) {
	fClockWords = clockWords;
}

//______________________________________________________________________________
void GSubEvent::SetStatusWords(Char_t statusWords) {
	fStatusWords = statusWords;
	;
}

//______________________________________________________________________________
void GSubEvent::SetFormatType(Char_t formatType) {
	fFormatType = formatType;
}

//______________________________________________________________________________

void GSubEvent::MakeSubEventHeader() {
	//Method to make a subEvent header

	//Creation of the start token (16bits)
	Short_t startToken = fSystemId << 2;
	startToken = (startToken + fClockWords) << 2;
	startToken = (startToken + fStatusWords) << 2;
	startToken = (startToken + fSubEventNumberWords) << 4;
	startToken = (startToken + fFormatType);

	memcpy(&fSubEventBrut[0], &startToken, 2);

	//Decomposition of clock, status and sub-event number in 3 blocks of 16 bits
	UShort_t clockList[3];
	clockList[0] = fClock >> 32;
	clockList[1] = fClock >> 16;
	clockList[2] = fClock;

	UShort_t statusList[3];
	statusList[0] = fStatus >> 32;
	statusList[1] = fStatus >> 16;
	statusList[2] = fStatus;

	UShort_t numberList[3];
	numberList[0] = fSubEventNumber >> 32;
	numberList[1] = fSubEventNumber >> 16;
	numberList[2] = fSubEventNumber;

	memcpy(&fSubEventBrut[1], &fSubEvent_size, 2);

	//Determination of the block(s) to write according to clockWords, statusWords and numberWords
	switch (fClockWords) {
	case 1:
		memcpy(&fSubEventBrut[2], &clockList[2], 2);
		break;

	case 2:
		memcpy(&fSubEventBrut[2], &clockList[1], 2);
		memcpy(&fSubEventBrut[3], &clockList[2], 2);
		break;

	case 3:
		memcpy(&fSubEventBrut[2], &clockList[0], 2);
		memcpy(&fSubEventBrut[3], &clockList[1], 2);
		memcpy(&fSubEventBrut[4], &clockList[2], 2);
		break;
	}

	switch (fStatusWords) {
	case 1:
		memcpy(&fSubEventBrut[fClockWords + 2], &statusList[2], 2);
		break;

	case 2:
		memcpy(&fSubEventBrut[fClockWords + 2], &statusList[1], 2);
		memcpy(&fSubEventBrut[fClockWords + 3], &statusList[2], 2);
		break;

	case 3:
		memcpy(&fSubEventBrut[fClockWords + 2], &statusList[0], 2);
		memcpy(&fSubEventBrut[fClockWords + 3], &statusList[1], 2);
		memcpy(&fSubEventBrut[fClockWords + 4], &statusList[2], 2);
		break;
	}

	switch (fSubEventNumberWords) {
	case 1:
		memcpy(&fSubEventBrut[fClockWords + fStatusWords + 2], &numberList[2],
				2);
		break;

	case 2:
		memcpy(&fSubEventBrut[fClockWords + fStatusWords + 2], &numberList[1],
				2);
		memcpy(&fSubEventBrut[fClockWords + fStatusWords + 3], &numberList[2],
				2);
		break;

	case 3:
		memcpy(&fSubEventBrut[fClockWords + fStatusWords + 2], &numberList[0],
				2);
		memcpy(&fSubEventBrut[fClockWords + fStatusWords + 3], &numberList[1],
				2);
		memcpy(&fSubEventBrut[fClockWords + fStatusWords + 4], &numberList[2],
				2);
		break;
	}

}
//______________________________________________________________________________
void GSubEvent::MakeSubEvent(Long64_t clock, Long64_t status, Long64_t number,
		Int_t* subTabOfIndex, Int_t sizeOfSubTab) {
	//Method to make a subEvent


	ClearData();
	if (fRand == NULL)
		fRand = new TRandom2();
	//Change seed of function rand()
	srand(++randSeed);

	fClock = clock;
	fStatus = status;
	fSubEventNumber = number;

	bool labelsAparition[sizeOfSubTab];

	fNbDatas = fParameter->GetDepthFromIndex(subTabOfIndex[0]);

	//Determination of appearance or not for each label.
	//First label is always selected to be sure that the sub-event will be not empty
	labelsAparition[0] = true;

	for (int i = 1; i < sizeOfSubTab; ++i) {

		if (rand() < RAND_MAX * fProba and subTabOfIndex[i] != -1) //Probability of 60% for apperance of parameter
		{
			labelsAparition[i] = true;
			fNbDatas += fParameter->GetDepthFromIndex(subTabOfIndex[i]);
		} else {
			labelsAparition[i] = false;
		}
	}

	fHeader_size = fClockWords + fStatusWords + fSubEventNumberWords + 2;
	fSubEvent_size = fHeader_size + fNbDatas * 2;//Size of sub-event in 16bits words, including header.


	fSubEventBrutData = fSubEventBrut + fHeader_size;

	//Write of label/data couples
	int currAdress = 0;
	int depth = 0;
	;
	int nbits = 0;
	Int_t currentLabel = 0;
	int value = 0;
	Int_t max = 0;
	Int_t mean =0;
	Int_t sigma=0;

	for (int i = 0; i < sizeOfSubTab; ++i) {

		if (labelsAparition[i] == true) {

			if (subTabOfIndex[i]<0) break;
			currentLabel = fParameter->GetLabel(subTabOfIndex[i]);
			depth = fParameter->GetDepthFromIndex(subTabOfIndex[i]);
			nbits = fParameter->GetNbitsFromIndex(subTabOfIndex[i]);
			value = 0;
			max = (Int_t)(pow(2., nbits));
			mean = max/2;
			sigma = max/16;
			for (int j = 0; j < depth; j++) {
				//Gaus(Double_t max/2 = 0, Double_t sigma = max)
				switch (fLaw) {
					case 1 : value = rand()% max;break;
					case 2 : value = fRand->Binomial(max,0.5 );;break;
					default: value = (int)(fRand->Gaus(mean,sigma));break;
				}
				//Label is written in peer blocs of 2 bytes, value in unpeer blocs
				memcpy(&fSubEventBrutData[currAdress], &currentLabel, 2);
				memcpy(&fSubEventBrutData[currAdress + 1], &value, 2);
				currAdress += 2;

			}

			//cout <<" depth " <<depth <<" currentLabel " <<currentLabel  <<" value " << value<< "\n";
		}

	}

	MakeSubEventHeader();

	if (fVerbose == 10) {
		DumpSubEvent();
	}
}


//______________________________________________________________________________

Int_t GSubEvent::GetSize() {
	return fSubEvent_size;
}

//______________________________________________________________________________

UShort_t* GSubEvent::GetSubEventBrut() {
	return fSubEventBrut;
}
//______________________________________________________________________________

void GSubEvent::DumpSubEvent() {
	//Read written datas to verify values

	if (fSubEventBrut == NULL) {
		fError.TreatError(2, 0,
				"No sub-event , so no dump. You would do a MakeSubEvent");
	}

	else {

		//Value of StartToken	
		UShort_t startToken = fSubEventBrut[0];
		cout << "start token : " << startToken << endl;

		short detectorSysId, clockWords, statusWords, numberWords, formatType,
				header_size;
		int numberOfData;
		formatType = startToken & 0x000F;
		numberWords = (startToken >> 4) & 0x0003;
		statusWords = (startToken >> 6) & 0x0003;
		clockWords = (startToken >> 8) & 0x0003;
		detectorSysId = (startToken >> 10) & 0x003F;
		header_size = clockWords + statusWords + numberWords + 2;
		numberOfData = (fSubEventBrut[1 + clockWords + statusWords
				+ numberWords] - header_size) / 2;

		cout << "detector system Id : " << detectorSysId << endl;
		cout << "clock words :  : " << clockWords << endl;
		cout << "status words : " << statusWords << endl;
		cout << "sub-event number words : " << numberWords << endl;
		cout << "sub-event format type : " << formatType << "\n" << endl;

		//Size of sub-event
		cout << "sub-event size : " << fSubEventBrut[clockWords + statusWords
				+ numberWords + 1] << endl;
		cout << "header size : " << header_size << endl;
		cout << "number of couples Label/Data : " << numberOfData << "\n"
				<< endl;

		//Values of each word of clock
		switch (clockWords) {
		case 1:
			cout << "1st clock word : " << fSubEventBrut[1] << endl;
			break;

		case 2:
			cout << "1st clock word : " << fSubEventBrut[1] << endl;
			cout << "2nd clock word : " << fSubEventBrut[2] << endl;
			break;

		case 3:
			cout << "1st clock word : " << fSubEventBrut[1] << endl;
			cout << "2nd clock word : " << fSubEventBrut[2] << endl;
			cout << "3rd clock word : " << fSubEventBrut[3] << endl;
		}
		cout << endl;

		//Values of each word of status
		switch (statusWords) {
		case 1:
			cout << "1st status word : " << fSubEventBrut[clockWords + 1]
					<< endl;
			break;

		case 2:
			cout << "1st status word : " << fSubEventBrut[clockWords + 1]
					<< endl;
			cout << "2nd status word : " << fSubEventBrut[clockWords + 2]
					<< endl;
			break;

		case 3:
			cout << "1st status word : " << fSubEventBrut[clockWords + 1]
					<< endl;
			cout << "2nd status word : " << fSubEventBrut[clockWords + 2]
					<< endl;
			cout << "3rd status word : " << fSubEventBrut[clockWords + 3]
					<< endl;
		}
		cout << endl;

		//Values of each word of number
		switch (numberWords) {
		case 1:
			cout << "1st number word : " << fSubEventBrut[clockWords
					+ statusWords + 1] << endl;
			break;

		case 2:
			cout << "1st number word : " << fSubEventBrut[clockWords
					+ statusWords + 1] << endl;
			cout << "2nd number word : " << fSubEventBrut[clockWords
					+ statusWords + 2] << endl;
			break;

		case 3:
			cout << "1st number word : " << fSubEventBrut[clockWords
					+ statusWords + 1] << endl;
			cout << "2nd number word : " << fSubEventBrut[clockWords
					+ statusWords + 2] << endl;
			cout << "3rd number word : " << fSubEventBrut[clockWords
					+ statusWords + 3] << endl;
		}

		cout << endl;

		//All couples of label/data
		for (int i = 0; i < numberOfData; ++i) {
			cout << "label : " << fSubEventBrut[header_size + i * 2]
					<< " value : " << fSubEventBrut[header_size + i * 2 + 1]
					<< endl;
		}
	}
}

//______________________________________________________________________________


////////////////////////////////////////fin /////////////////////////////////////
