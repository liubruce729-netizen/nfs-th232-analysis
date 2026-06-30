/*
 MFMSiriusFrame.cc
 Copyright Acquisition group, GANIL Caen, France
 */

#include <iostream>
#include <cmath>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <time.h>
using namespace std;

#include "MFMSiriusFrame.h"

//_______________________________________________________________________________
MFMSiriusFrame::MFMSiriusFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, int headerSize,
		int itemSize, int nItems) {
	/// Constructor of frame with a memory space\n
	/// fill header information : unitBlock_size,dataSource,....
	SetPointers();	
	fCountNbEventCard = NULL;
}
//_______________________________________________________________________________
MFMSiriusFrame::MFMSiriusFrame() {
	/// Constructor of a empty frame object
	fCountNbEventCard = NULL;
}
//_______________________________________________________________________________
MFMSiriusFrame::~MFMSiriusFrame() {
	///Destructor
	if (fCountNbEventCard) {
		delete[] fCountNbEventCard;
		fCountNbEventCard = NULL;
	}
}
/*
//_______________________________________________________________________________
void MFMSiriusFrame::SetAttributs(void * pt) {
	SetPointers(pt);
	MFMBasicFrame::SetAttributs(pt);

}*/
//_______________________________________________________________________________
string MFMSiriusFrame::GetHeaderDisplay(char* infotext) const {
	stringstream ss;
	string display("");
	display = ss.str();
	ss << MFMBasicFrame::GetHeaderDisplay(infotext);
	ss<< " Channel = " << GetChannelId(); 
	display = ss.str();
	return display;
}
//_______________________________________________________________________________

void MFMSiriusFrame::SetTimeStampFromSiriusFrameData() {
	/// Compute time stamp and fill fTimeStamp attribut. return value of TimeStamp
	fTimeStamp = 0;
	uint64_t* timeStamp = &(fTimeStamp);

		memcpy(((char*) (&fTimeStamp)),
				((MFM_SiriusHeader*) pHeader)->SiriusEvtInfo.eventTime,
				6);
		if (fLocalIsBigEndian != fFrameIsBigEndian)
			SwapInt64((timeStamp), 6);
	
}

//_______________________________________________________________________________

void  MFMSiriusFrame::SetEventNumberFromSiriusFrameData() {
	/// Compute and return envent number
	fEventNumber = 0;
	char * eventNumber = (char*) &(fEventNumber);
	fEventNumber= ((MFM_SiriusHeader*) pHeader)->SiriusEvtInfo.eventIdx;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt32((uint32_t *) (eventNumber), 4);


}

//_______________________________________________________________________________
void MFMSiriusFrame::SetTimeStamp(uint64_t timestamp) {
	// Set frame timestamp
	char* pts = (char*) &timestamp;
	timestamp = timestamp & 0x0000ffffffffffff;
	memcpy(((MFM_SiriusHeader*) pHeader)->SiriusEvtInfo.eventTime,
				pts, 6);
}

//_______________________________________________________________________________
void MFMSiriusFrame::SetEventNumber(uint32_t eventnumber) {
	/// set frame event number

	((MFM_SiriusHeader*) pHeader)->SiriusEvtInfo.eventIdx = eventnumber;
}

//_______________________________________________________________________________
void MFMSiriusFrame::SetLocationId(uint16_t Id) {
	/// Set 16 bits of LocationId
	((MFM_SiriusHeader*) pHeader)->LocationId = Id;
}
//_______________________________________________________________________________
void MFMSiriusFrame::SetLocationId(uint16_t ChannelId, uint16_t BoardId) {
	uint16_t ui, ui2;
	ui2 = 0;
	//trig request form 0 to 4
	ui2 = ChannelId & NUMEXO_CHANNEL_ID_MASK;

	//id board from 5 to 15
	ui = BoardId & NUMEXO_BOARD_ID_MASK;
	ui = ui << 4;
	ui2 = ui2 | ui;
	SetLocationId(ui2);
}
//_______________________________________________________________________________

uint16_t MFMSiriusFrame::GetLocationId() const{
	uint16_t Id = 0;
	/// Compute and return the 2 bytes of LocationId()
	Id = ((MFM_SiriusHeader*) pHeader)->LocationId;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16((&Id));
	return ((Id));
}
//_______________________________________________________________________________
uint16_t MFMSiriusFrame::GetChannelId() const{
	/// Compute and return extracted ChannelId
	return (GetLocationId() & NUMEXO_CHANNEL_ID_MASK);
}

//_______________________________________________________________________________
uint16_t MFMSiriusFrame::GetBoardId()const {
	/// Compute and return id value of Board
	return (((GetLocationId() >> 5) & NUMEXO_BOARD_ID_MASK));
}

//_______________________________________________________________________________
uint16_t MFMSiriusFrame::GetGain() const {
    /// return DSSD gain 
    uint16_t tmp=((MFM_SiriusHeader*) pHeader)->DSSD_gain;
	if (fLocalIsBigEndian!=fFrameIsBigEndian)
		SwapInt16(&tmp);
	return  tmp;
}
//_______________________________________________________________________________
void MFMSiriusFrame::SetGain(uint32_t gain) {
	/// set frame event number
	((MFM_SiriusHeader*) pHeader)->DSSD_gain = gain;
}
//_______________________________________________________________________________
uint16_t MFMSiriusFrame::GetFeedBack(int i) const {
    /// return one of 16 feedbacks available 
    uint16_t tmp=((MFM_SiriusHeader*) pHeader)->Feedback[i];
	if (fLocalIsBigEndian!=fFrameIsBigEndian)
		SwapInt16(&tmp);
	return  tmp;
}
//_______________________________________________________________________________
void MFMSiriusFrame::SetFeedBack(uint32_t feed, int i) {
	/// set one of 16 feedbacks available 
	((MFM_SiriusHeader*) pHeader)->Feedback[i] = feed;
}
//_______________________________________________________________________________
void MFMSiriusFrame::GetParameters(int i,uint16_t *value) const {
	/// Compute and return the couple information of label /value of the i-th item
	GetParametersByItem((MFM_SiriusItem *) GetItem(i),value);
}
//_______________________________________________________________________________
void MFMSiriusFrame::GetParametersByItem(MFM_SiriusItem *item, uint16_t *value)const {
	/// Compute and return the couple information of label /value of  item

	if (fLocalIsBigEndian != fFrameIsBigEndian) {
		uint16_t tmp = item->Value;
		SwapInt16(&tmp);
		*value = tmp;
	} else {
		*value = item->Value;
	}
}
//_______________________________________________________________________________
void MFMSiriusFrame::SetParameters(int i, uint16_t value) {
	/// set i-th item 
	SetParametersByItem((MFM_SiriusItem *) GetItem(i), value);
}
//_______________________________________________________________________________
void MFMSiriusFrame::SetParametersByItem(MFM_SiriusItem *item, uint16_t value) {
	/// set "item" with the couple information of label /value
	item->Value = value;
}
//_______________________________________________________________________________
void MFMSiriusFrame::FillDataWithRamdomValue(uint64_t timestamp,
		uint32_t enventnumber,int nbitem ) {
	/// Fill frame items with random values
	//  in this case the nume
	
    	int max_value = 16384; // nous nous basons sur 14 bits comme beaucoup de cartes electoniques
	float randval;
	uint16_t i = 0;
	int nb_bits = 16;
	float nbofperiodes = 1;
        int16_t board=112;
	float P;
	static unsigned int seed = 15;
	unsigned short uivalue;
	float rando, tempof;
	int h;
	static int16_t channel = 0;

	for (i = 0; i < 16; i++) { 
		SetFeedBack(i,i);
	}
	SetGain(1966);
	SetLocationId( channel, board );
	nbitem = GetDefinedNbItems();
	
	P = 1.570796327 * 2;
	rando = seed;

	h = pow(2, nb_bits) - 1;

	
	if (nbitem > 0)
		SetParameters(0,  1);
	for (i = 1; i < nbitem; i++) {
		rando = (float) ((rand_r(&seed) / (RAND_MAX + 1.0)));
		seed++;
		nbofperiodes = 1 + rando * 4;
		tempof = (float) i / (nbitem / (nbofperiodes));
		uivalue = (unsigned short) (h / 2 + h / 2 * sin((float) 2 * P * tempof));
		SetParameters(i, uivalue);
	}
	SetEventNumber(enventnumber);
	SetTimeStamp(timestamp);
	channel++;
	channel = channel%NUMEXO_NB_CHANNELS;
}

//____________________________________________________________________________

string MFMSiriusFrame::GetDumpData(char mode, bool nozero) const {
	// Dump parameter Label and parameter value of the current event.
	// if enter parameter is true (default value), all zero parameter of event aren't dumped
	// mode = 'd' for decimal, 'b' for binary, 'h' for hexa, 'o' for octal

	stringstream ss;
	string display("");

	int i, j, maxbin, presentation = 0, max_presentation = 5;
	char tempo[300];
	char Bin[255];
	char Bin2[255];
	uint16_t value;
	int reste;
	char one = '1', zero = '0';
	int DecNumber = 0;

	if (mode == 'b')
		max_presentation = 3;
	if (GetEventNumber() == 0xFFFFFFFF) {
		ss << "No Event , so no event dump. Get a new event frame";
	} else {
		for (i = 0; i < GetNbItems(); i++) {
			value = 0;
			GetParameters(i, &value);
			if ((value != 0xFFFF) || (nozero == false)) {
				switch (mode) {
				case 'd':// decimal display
					sprintf(tempo, "|%5d = %5d",i, value);
					break;
				case 'o':// octal display
					sprintf(tempo, "|%5o = %5o",i,  value);
					break;
				case 'h':// hexa display
					sprintf(tempo, "|%5x = %5x",i,  value);
					break;
				case 'b': // binary display
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
					sprintf(tempo, "|%5d = %16s",i, Bin2);
					break;
				}
				ss << tempo;
				presentation++;
			}
			if (presentation == max_presentation) {
				ss << "|\n";
				presentation = 0;
			}
		}
		if (presentation != 0)
			ss << "|\n";
	}
	display = ss.str();
	return display;
}


//_______________________________________________________________________________
void MFMSiriusFrame::InitStat() {
	int i=0;
	MFMBasicFrame::InitStat();
	fCountNbEventCard = new long long[65536];
	for (i = 0; i < 65536; i++) {
		fCountNbEventCard[i] = 0;
	}
}
//____________________________________________________________________
void MFMSiriusFrame::FillStat() {
	MFMBasicFrame::FillStat();
	uint16_t id;
	id = GetBoardId();
	fCountNbEventCard[id]++;
	
	uint32_t eventnumber = GetEventNumber();
	static uint32_t eventnumberold = 0;
	if (eventnumber != 0) {
		if (eventnumber - eventnumberold < 0)
			IncrementNegativJump();
		if (eventnumber - eventnumberold != 1)
			IncrementNoContiJump();
		eventnumberold=eventnumber;
	}
}
//____________________________________________________________________
string MFMSiriusFrame::GetStat(string info) const {
	
	string display("");
	stringstream ss("");
	ss << MFMBasicFrame::GetStat(info);
	int i, j;
	int total = 0;
	for (i = 0; i < NUMEXO_MAX_NUMB_BOARDS; i++) {
		if (fCountNbEventCard[i] != 0) {
			j = i;
			ss << "Card " << i;
			ss << " NbEvents = " << fCountNbEventCard[i] << "\n";
			total += fCountNbEventCard[i];
		}
	}
	ss << " Total                        = " << total << "\n";
	display = ss.str();
	return display;
}

//_______________________________________________________________________________


