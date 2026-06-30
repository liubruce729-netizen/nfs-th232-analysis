/*
 MFMNedaFrame.cc
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

#include "MFMNedaFrame.h"

//_______________________________________________________________________________
MFMNedaFrame::MFMNedaFrame(int unitBlock_size, int dataSource, int frameType,
		int revision, int frameSize, int headerSize, int itemSize, int nItems) {
	/// Constructor of frame with a memory space\n
	/// fill header information : unitBlock_size,dataSource,....
	pNedaFrame =NULL;
	SetPointers();
	fCountNbEventCard = NULL;
	fEndFrame =0xF0F0F0F0;
	fNbofGoodF0F0 = 0;
        fNbofBadF0F0  = 0;

}

//_______________________________________________________________________________
MFMNedaFrame::MFMNedaFrame() {
	/// Constructor of a empty frame object
	pNedaFrame =NULL;
	fCountNbEventCard = NULL;
	fEndFrame =0xF0F0F0F0;
	fNbofGoodF0F0 = 0;
        fNbofBadF0F0  = 0;
}
//_______________________________________________________________________________
MFMNedaFrame::~MFMNedaFrame() {
	///Destructor
	if (fCountNbEventCard) {
		delete[] fCountNbEventCard;
		fCountNbEventCard = NULL;
	}

}
/*
//_______________________________________________________________________________
void MFMNedaFrame::SetAttributs(void * pt) {
	SetPointers(pt);
	MFMBasicFrame::SetAttributs(pt);
	//SetTimeStampFromFrameData();
	//SetEventNumberFromFrameData();
}*/
//_______________________________________________________________________________
string MFMNedaFrame::GetHeaderDisplay(char* infotext) const {
	stringstream ss;
	string display("");
	ss << MFMBasicFrame::GetHeaderDisplay(infotext);
	ss << endl << "   | LeInter = " << (int) GetLeInterval();
	ss << " | ZcoInterval = " << (int) GetZcoInterval() << "|  Tdc =  " << GetTdcValue() << " | Slow Integ = "
			<< GetSlowIntegral() << " | FastIntegral = ";
	ss << GetFastIntegral() << " | Bitfield = " << (int) GetBitfield()
			<< "| AbsMax = " << (int) GetAbsMax() << endl;
	ss << "   Event is neutron = " << IsNeutron() << " | Valid CFD = " << IsCFDValid() << " | Parity = " << IsCFDParity() ; 
	display = ss.str();
	return display;
}
//_______________________________________________________________________________

void MFMNedaFrame::SetTimeStampFromNedaFrameData() {
	/// Compute time stamp and fill fTimeStamp attribut. return value of TimeStamp
	fTimeStamp = 0;
	uint64_t* timeStamp = &(fTimeStamp);

	if (GetRevision() == 0) {
		int byte_numexo[6] = { 1, 0, 5, 4, 3, 2 };
		for (int j = 0; j < 6; j++)
			memcpy(
					((char*) (&fTimeStamp)) + j,
					((char*) ((MFM_Neda_Header*) pHeader)->EvtInfo.EventTime)
							+ byte_numexo[j], 1);
	} else {
		memcpy(((char*) (&fTimeStamp)),
				((MFM_Neda_Header*) pHeader)->EvtInfo.EventTime, 6);

	}
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt64((timeStamp), 6);
}

//_______________________________________________________________________________

void MFMNedaFrame::SetEventNumberFromNedaFrameData() {
	/// Compute and return envent number
	fEventNumber = 0;
	char * eventNumber = (char*) &(fEventNumber);

	fEventNumber = ((MFM_Neda_Header*) pHeader)->EvtInfo.EventIdx;
	if (GetRevision() != 0) {
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt32((uint32_t *) (eventNumber), 4);
		}
}
//_______________________________________________________________________________
void MFMNedaFrame::SetTimeStamp(uint64_t timestamp) {
	// Set frame timestamp
	char* pts = (char*) &timestamp;
	timestamp = timestamp & 0x0000ffffffffffff;
	memcpy(((MFM_Neda_Header*) pHeader)->EvtInfo.EventTime, pts, 6);
}

//_______________________________________________________________________________
void MFMNedaFrame::SetEventNumber(uint32_t eventnumber) {
	/// set frame event number
	((MFM_Neda_Header*) pHeader)->EvtInfo.EventIdx = eventnumber;
}

//_______________________________________________________________________________
void MFMNedaFrame::SetLocationId(uint16_t Id) {
	/// Set 16 bits of LocationId
	((MFM_Neda_Header*) pHeader)->EvtInfo.LocationId = Id;
}
//_______________________________________________________________________________
void MFMNedaFrame::SetLocationId(uint16_t ChannelId, uint16_t BoardId) {
	uint16_t ui, ui2;
	ui2 = 0;
	//trig request form 0 to 4
	ui2 = ChannelId & NEDA_CHANNEL_ID_MASK;

	//id board from 5 to 15
	ui = BoardId & NEDA_BOARD_ID_MASK;
	ui = ui << 5;
	ui2 = ui2 | ui;
	SetLocationId(ui2);
}

//_______________________________________________________________________________

uint16_t MFMNedaFrame::GetLocationId()const {
	uint16_t Id = 0;
	/// Compute and return the 2 bytes of LocationId()
	Id = ((MFM_Neda_Header*) pHeader)->EvtInfo.LocationId;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16((&Id));
	return ((Id));
}
//_______________________________________________________________________________
uint16_t MFMNedaFrame::GetChannelId() const{
	/// Compute and return extracted ChannelId
	return (GetLocationId() & NEDA_CHANNEL_ID_MASK);
}

//_______________________________________________________________________________
uint16_t MFMNedaFrame::GetBoardId() const{
	/// Compute and return id value of Board
	return (((GetLocationId() >> 5) & NEDA_BOARD_ID_MASK));
}
//_______________________________________________________________________________

uint8_t MFMNedaFrame::GetLeInterval() const{
	uint8_t id = ((MFM_Neda_Header*) pHeader)->EvtInfo.LeInterval;
	return id;
}
//_______________________________________________________________________________
void MFMNedaFrame::SetLeInterval(uint8_t interval) {
	((MFM_Neda_Header*) pHeader)->EvtInfo.LeInterval = interval;
}
//_______________________________________________________________________________
uint8_t MFMNedaFrame::GetZcoInterval() const{
	return (uint8_t) ((MFM_Neda_Header*) pHeader)->EvtInfo.ZcoInterval;

}
//_______________________________________________________________________________
void MFMNedaFrame::SetZcoInterval(uint8_t interval) {
	((MFM_Neda_Header*) pHeader)->EvtInfo.ZcoInterval = interval;
}
//_______________________________________________________________________________
uint16_t MFMNedaFrame::GetTdcValue() const{
	uint16_t val = 0;
	val = ((MFM_Neda_Header*) pHeader)->EvtInfo.TdcValue;
	if (GetRevision() > 0) {
		if (fLocalIsBigEndian != fFrameIsBigEndian) {
			SwapInt16(&val);
		}
	} else {
		val = val & 0xff;
	}
	return val;
}
//_______________________________________________________________________________
void MFMNedaFrame::SetTdcValue(uint16_t val) {
	((MFM_Neda_Header*) pHeader)->EvtInfo.TdcValue = val;
}
//_______________________________________________________________________________
uint16_t MFMNedaFrame::GetSlowIntegral()const {

	uint16_t integral = 0;
	char * pintegral = (char*) &(integral);

	integral = ((MFM_Neda_Header*) pHeader)->EvtInfo.SlowIntegral;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		if (GetRevision() > 0)
			SwapInt16((uint16_t *) (pintegral));
	return integral;
}
//_______________________________________________________________________________
void MFMNedaFrame::SetSlowIntegral(uint16_t id) {
	((MFM_Neda_Header*) pHeader)->EvtInfo.SlowIntegral = id;
}
//_______________________________________________________________________________
uint16_t MFMNedaFrame::GetFastIntegral() const{
	uint16_t integral = 0;
	char * pintegral = (char*) &(integral);

	integral = ((MFM_Neda_Header*) pHeader)->EvtInfo.FastIntegral;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		if (GetRevision() > 0)
			SwapInt16((uint16_t *) (pintegral));
	return integral;
}
//_______________________________________________________________________________
void MFMNedaFrame::SetFastIntegral(uint16_t id) {
	((MFM_Neda_Header*) pHeader)->EvtInfo.FastIntegral = id;
}
//_______________________________________________________________________________
uint8_t MFMNedaFrame::GetBitfield() const{
	uint8_t id = ((MFM_Neda_Header*) pHeader)->EvtInfo.Bitfield;
	return id;
}
//_______________________________________________________________________________
bool MFMNedaFrame::GetBitfield(int id) const{
	uint8_t bitfield = ((MFM_Neda_Header*) pHeader)->EvtInfo.Bitfield;
	return ((bool) ((bitfield >> id) & 0x1));
}
//_______________________________________________________________________________
void MFMNedaFrame::SetBitfield(uint8_t bitfield) {
	((MFM_Neda_Header*) pHeader)->EvtInfo.Bitfield = bitfield;
}
//_______________________________________________________________________________
void MFMNedaFrame::SetBitfield(int id, bool bit) {
	uint8_t bitfield = GetBitfield();
	uint8_t localbit = 0x1 << id;
	if (bit) {
		bitfield = localbit | bitfield;
	} else {
		bitfield = bitfield & (~localbit);
	}
	((MFM_Neda_Header*) pHeader)->EvtInfo.Bitfield = bitfield;
}
//_______________________________________________________________________________
uint8_t MFMNedaFrame::GetAbsMax() const{
	uint8_t id = ((MFM_Neda_Header*) pHeader)->EvtInfo.AbsMax;
	return id;
}
//_______________________________________________________________________________
void MFMNedaFrame::SetAbsMax(uint8_t id) {
	((MFM_Neda_Header*) pHeader)->EvtInfo.AbsMax = id;
}
//_______________________________________________________________________________
uint8_t MFMNedaFrame::GetInterpolCFD() const{
	uint8_t id = ((MFM_Neda_Header*) pHeader)->EvtInfo.InterpolCFD;
	return id;
}
//_______________________________________________________________________________
void MFMNedaFrame::SetInterpolCFD(uint8_t id) {
	((MFM_Neda_Header*) pHeader)->EvtInfo.InterpolCFD = id;
}
//_______________________________________________________________________________
void MFMNedaFrame::NedaGetParameters(int i, uint16_t *value)const {
	/// Compute and return the couple information of  value of the i-th item
	NedaGetParametersByItem((MFM_Neda_Item *) GetItem(i), value);
	int parity = (*value & NEDA_MASK_PARITY_ITEM) >> 14;
	*value = *value & NEDA_MASK_DATA_ITEM;
	if ((i % 2) == 0 && parity != 0) {
		std::cerr << "Problem decoding even sample "<<i<<" val  "<< *value << " parity " <<parity<<   std::endl;
		*value = 0;
	} else if ((i % 2) == 1 && parity != 1) {
		std::cerr << "Problem decoding odd sample "<<i<<" val  " << *value  << " parity " <<parity << std::endl;
		*value = 0;
	}
}
//_______________________________________________________________________________
void MFMNedaFrame::NedaGetParametersByItem(MFM_Neda_Item *item, uint16_t *value) const{
	/// Compute and return the couple information of value of  item

	if (fLocalIsBigEndian != fFrameIsBigEndian) {
		uint16_t tmp = item->Value;
		if (GetRevision() > 0)
			SwapInt16(&tmp);
		*value = tmp;
	} else {
		*value = item->Value;
	}
	
}
//_______________________________________________________________________________
void MFMNedaFrame::NedaSetParameters(int i, uint16_t value) {
	/// set i-th item if frame with the couple information of value
	if ((i % 2) == 0)
		value = value | NEDA_MASK_DATA_ITEM;
	if ((i % 2) == 1)
		value = value | (NEDA_MASK_PARITY_ITEM | NEDA_MASK_DATA_ITEM);
	NedaSetParametersByItem((MFM_Neda_Item *) GetItem(i), value);

}
//_______________________________________________________________________________
void MFMNedaFrame::NedaSetParametersByItem(MFM_Neda_Item *item, uint16_t value) {
	/// set "item" with the couple information of value
        static int toto=0;
	item->Value = value;
}
//_______________________________________________________________________________
void MFMNedaFrame::SetEndFrame(uint32_t end) {
	fEndFrame =end;
}
//_______________________________________________________________________________
bool MFMNedaFrame::TestEndOfFrame()const {
	/// test last 4 butes of frame ,
		return (( (MFM_Neda_Frame*) pData)->MFMNedaEOF.EndOfFrame == 0xf0f0f0f0);
}
//_______________________________________________________________________________
void MFMNedaFrame::FillEndOfFrame() {
	/// last 4 bytes to of frame,

static unsigned int seed = 6;      
	if ((fEndFrame == 0xf0f0f0f0)|| (fEndFrame == 0)){
		((MFM_Neda_Frame*) pData)->MFMNedaEOF.EndOfFrame = fEndFrame;

	}else{
		// get fEndFrame witch give the rate of error in per cent
		uint rando = seed;
		float tempo = rand_r(&seed);
		rando = (int )((float) (tempo *100/ (RAND_MAX + 1.0)));

		if (rando < fEndFrame){

			((MFM_Neda_Frame*) pData)->MFMNedaEOF.EndOfFrame = 0xABBAABBA;

		}else{
			((MFM_Neda_Frame*)pData)->MFMNedaEOF.EndOfFrame = 0xf0f0f0f0;
		}
	}

}
//_______________________________________________________________________________
void MFMNedaFrame::FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber,int nbitem){
	/// Fill frame items  of sinus values with random perios,


	int nb_bits = 16;
	float nbofperiodes = 1;
	int h;
	int i;
	float P;
	static unsigned int seed = 15;
	unsigned short uivalue;
	float rando, tempof;
	P = 1.570796327 * 2;
	rando = seed;
	h = pow(2, nb_bits) - 1;
	
	SetLocationId(1, 116) ;
	SetLeInterval(3);
	SetZcoInterval(4);
	SetTdcValue(5);
	SetSlowIntegral(6);
	SetFastIntegral(7);
	SetBitfield(8);
	SetAbsMax(9);

	
	if (nbitem > 0)
		NedaSetParameters(0, 1);
	rando = (float) ((rand_r(&seed) / (RAND_MAX + 1.0)));
	seed++;
  
	for (i = 1; i < nbitem; i++) {

		seed++;
		nbofperiodes = 1 + rando * 4;

		tempof = (float) i / (nbitem / (nbofperiodes));
		uivalue
				= (uint16_t) (h / 2 + h / 2 * sin((float) 2 * P * tempof));

		NedaSetParameters(i, uivalue);
	}
	SetEventNumber(enventnumber);
	SetTimeStamp(timestamp);
	FillEndOfFrame();

}
/*
//_______________________________________________________________________________

void MFMNedaFrame::GenerateANedaExample(int type, int eventnumber) {
	/// Generate a example of frame containing random value\n
	/// usable for tests.
	if (type != MFM_NEDA_FRAME_TYPE) {
		cout
				<< "Error in  MFMNedaFrame::GenerateANedaExample type not understood\n";
		return;
	}

	uint32_t unitBlock_size = 0;
	uint32_t itemsize = 0;
	uint32_t nbitem = 0;
	uint32_t framesize = 0;
	uint32_t revision = 1;
	uint32_t headersize = 0;
	unitBlock_size = NEDA_STD_UNIT_BLOCK_SIZE;
	itemsize = 2;
	nbitem = NEDA_NB_OF_ITEMS;

	headersize = NEDA_HEADERSIZE;

	framesize = headersize + nbitem * itemsize + sizeof(MFM_Neda_EOF);
	revision = 1;
	// generation of MFM header
	MFM_make_header(unitBlock_size, 0, type, revision, (int) (framesize
			/ unitBlock_size), (headersize / unitBlock_size), itemsize, nbitem);


	FillEventWithRamdomConst(GetTimeStampUs(), eventnumber,nbitem);

}

*/
//_______________________________________________________________________________
void MFMNedaFrame::InitStat() {
	MFMBasicFrame::InitStat();
	int i;
	fCountNbEventCard = new long long[65536];
	for (i = 0; i < 65536; i++) {
		fCountNbEventCard[i] = 0;
	}
}
//____________________________________________________________________
void MFMNedaFrame::FillStat() {
	MFMBasicFrame::FillStat();
	uint16_t id;
	id = GetLocationId();
	fCountNbEventCard[id]++;
	if (TestEndOfFrame())
		fNbofGoodF0F0++;
	else
		fNbofBadF0F0++;
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
string MFMNedaFrame::GetStat(string info)const {

	string display("");
	stringstream ss("");
	ss << MFMBasicFrame::GetStat(info);
	int i, j;
	int total = 0;
	for (i = 0; i < 65536; i++) {
		if (fCountNbEventCard[i] != 0) {
			j = i;
			ss << "Card " << ((j >> NUMEXO_SLIP_BITS) & NEDA_BOARD_ID_MASK);
			j = i;
			ss << " Cristal  " << (j & NEDA_CHANNEL_ID_MASK);
			ss << " NbEvents = " << fCountNbEventCard[i] << "\n";
			total += fCountNbEventCard[i];
		}
	}
	ss << " Nb of Good F0F0F0F0          = "<<fNbofGoodF0F0  << "\n";
	ss << " Nb of Bad  F0F0F0F0          = "<<fNbofBadF0F0  << "\n";
	ss << " Total                        = " << total << "\n";
	display = ss.str();
	return display;
}
//____________________________________________________________________
bool MFMNedaFrame::IsNeutron()const
{
  uint8_t id = ((MFM_Neda_Header*) pHeader)->EvtInfo.Bitfield;
  return id>>7;
}
//____________________________________________________________________
bool MFMNedaFrame::IsCFDValid()const
{
  uint8_t id = ((MFM_Neda_Header*) pHeader)->EvtInfo.Bitfield;
  return (id>>5)&0x1;
}
//____________________________________________________________________
bool MFMNedaFrame::IsCFDParity()const
{
  uint8_t id = ((MFM_Neda_Header*) pHeader)->EvtInfo.Bitfield;
  return (id>>6)&0x1;
}
//____________________________________________________________________
