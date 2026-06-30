/*
 MFMVamosPDFrame.cc
 Copyright Acquisition group, GANIL Caen, France
 */

#include <iostream>
#include <cmath>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
using namespace std;

#include "MFMVamosPDFrame.h"

//_______________________________________________________________________________
MFMVamosPDFrame::MFMVamosPDFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, int headerSize) {
	/// Constructor for a Vamos Ionization Chamber frame . the header is filled with unitblock_size, data source , frame type , revision , frame, size and header size value
	SetPointers();
	fLabelIndice = NULL;
	fNbLabels = NULL;
	fIndiceLabel = NULL;
	fCountNbEventCard = NULL;
	MaxTrace = 2048;
}

//_______________________________________________________________________________
MFMVamosPDFrame::MFMVamosPDFrame() {
	/// Constructor for a empty Vamos IC frame
	fLabelIndice = NULL;
	fNbLabels = NULL;
	fIndiceLabel = NULL;
	fCountNbEventCard = NULL;
	MaxTrace = 2048;
}
//_______________________________________________________________________________
MFMVamosPDFrame::~MFMVamosPDFrame() {
	/// destructor of VamosPD frame$
	if (fLabelIndice) {
		delete[] fLabelIndice;
		fLabelIndice = NULL;
	};
	if (fIndiceLabel) {
		delete[] fIndiceLabel;
		fIndiceLabel = NULL;
	};
	if (fNbLabels) {
		delete[] fNbLabels;
		fNbLabels = NULL;
	};
	if (fCountNbEventCard){	
		delete[] fCountNbEventCard;
		fCountNbEventCard=NULL;
	}
}
/*
//_______________________________________________________________________________
void MFMVamosPDFrame::SetAttributs(void * pt) {
	SetPointers(pt);
	MFMBlobFrame::SetAttributs(pt);
	SetEventNumberFromFrameData();
}*/
//_______________________________________________________________________________
void  MFMVamosPDFrame::SetUserDataPointer() { 
	pUserData_char = (char*) &(((MFM_vamosPD_frame*) pHeader)->VamosPDData);
}
//_______________________________________________________________________________

void MFMVamosPDFrame::SetTimeStampFromVamosPDFrameData() {
	/// Computer, set attibut and return value of time stamp from  frame
	fTimeStamp = 0;
	uint64_t * timeStamp = &(fTimeStamp);
	memcpy(((char*) (&fTimeStamp)),
			((MFM_vamosPD_frame*) pHeader)->VamosPDEventInfo.EventTime, 6);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt64((timeStamp), 6);
}
//_______________________________________________________________________________

void MFMVamosPDFrame::SetEventNumberFromVamosPDFrameData() {
	/// Computer, set attibut and return value of event number from  frame
	fEventNumber = 0;
	char * eventNumber = (char*) &(fEventNumber);

	fEventNumber = ((MFM_vamosPD_frame*) pHeader)->VamosPDEventInfo.EventIdx;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt32((uint32_t *) (eventNumber), 4);
}
//_______________________________________________________________________________
void MFMVamosPDFrame::SetTimeStamp(uint64_t timestamp) {
	/// Set value of Time Stamp in frame
	char* pts = (char*) &timestamp;
	timestamp = timestamp & 0x0000ffffffffffff;
	memcpy(((MFM_vamosPD_frame*) pHeader)->VamosPDEventInfo.EventTime, pts, 6);
}
//_______________________________________________________________________________
void MFMVamosPDFrame::SetEventNumber(uint32_t eventnumber) {
	/// Set Event Number of frame
	((MFM_vamosPD_frame*) pHeader)->VamosPDEventInfo.EventIdx = eventnumber;
}

//_______________________________________________________________________________
void MFMVamosPDFrame::SetCristalId(uint16_t cristalId) {
	/// Set 16 bits of CristalIdx
	((MFM_vamosPD_frame*) pHeader)->VamosPDData.CristalId = cristalId;
}
//_______________________________________________________________________________
void MFMVamosPDFrame::SetCristalId(uint16_t tgRequest, uint16_t idBoard) {

	// Methode to fill item in case of MFM_COBO_FRAME_TYPE frame
	uint16_t ui, ui2;
	ui2 = 0;

	//trig request form 0 to 4
	ui2 = tgRequest & VAMOSPD_TRIG_REQ_CRYS_ID_MASK;

	//id board from 5 to 15
	ui = idBoard & VAMOSPD_BOARD_ID_MASK;
	ui = ui << 5;
	ui2 = ui2 | ui;
	SetCristalId(ui2);
}
//_______________________________________________________________________________

uint16_t MFMVamosPDFrame::GetCristalId() const{
	uint16_t cristal = 0;
	/// Compute and return the 2 bytes of CristalId()
	cristal = ((MFM_vamosPD_frame*) pHeader)->VamosPDData.CristalId;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16((&cristal));
	return ((cristal));
}

//_______________________________________________________________________________
uint16_t MFMVamosPDFrame::GetTGCristalId()const {
	/// Compute and return extracted Trigger Request value of CristalId
	return (GetCristalId() & VAMOSPD_TRIG_REQ_CRYS_ID_MASK);
}

//_______________________________________________________________________________
uint16_t MFMVamosPDFrame::GetBoardId() const{
	/// Compute and return id value of Board
	return ((GetCristalId() >> 5) & VAMOSPD_BOARD_ID_MASK);
}

//_______________________________________________________________________________

void MFMVamosPDFrame::SetEnergy(int i, uint16_t energy) {
	if (i < 0 and i > VAMOSPD_NB_VALUE)
		cout << "MFMVamosPDFrame::SetEnergy Error of status index\n";
	((MFM_vamosPD_frame*) pHeader)->VamosPDData.LabelValue[i].Val = energy;

}
//_______________________________________________________________________________

uint16_t MFMVamosPDFrame::GetEnergy(int i) const{
	uint16_t energy;
	if (i < 0 and i > VAMOSPD_NB_VALUE) {
		cout << "MFMVamosPDFrame::GetEnergy Error of energy index\n";
		return 0;
	} else {
		energy = ((MFM_vamosPD_frame*) pHeader)->VamosPDData.LabelValue[i].Val;
	}
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&energy);
	return energy;
}
//_______________________________________________________________________________

void MFMVamosPDFrame::SetLabel(int i, uint16_t Label) {
	if (i < 0 and i > VAMOSPD_NB_VALUE)
		cout << "MFMVamosPDFrame::SetLabel Error of status index\n";
	((MFM_vamosPD_frame*) pHeader)->VamosPDData.LabelValue[i].Lab = Label;

}
//_______________________________________________________________________________

uint16_t MFMVamosPDFrame::GetLabel(int i) const{
	uint16_t Label;
	if (i < 0 and i > VAMOSPD_NB_VALUE) {
		cout << "MFMVamosPDFrame::GetLabel Error of Label index\n";
		return 0;
	} else {
		Label = ((MFM_vamosPD_frame*) pHeader)->VamosPDData.LabelValue[i].Lab;
	}
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&Label);
	return Label;
}
//_______________________________________________________________________________
void MFMVamosPDFrame::GetParameters(int i, uint16_t* label, uint16_t* value)const {
	*label = GetLabel(i);
	*value = GetEnergy(i);
}

//_______________________________________________________________________________

void MFMVamosPDFrame::SetLocalCount(uint16_t count) {
	/// Set LocalCount
	((MFM_vamosPD_frame*) pHeader)->VamosPDData.LocalCount = count;
}

//_______________________________________________________________________________

uint16_t MFMVamosPDFrame::GetLocalCount()const {
	/// Get LocalCount
	uint16_t count;
	count = (((MFM_vamosPD_frame*) pHeader)->VamosPDData.LocalCount);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&count);
	return count;
}
//_______________________________________________________________________________

void MFMVamosPDFrame::SetChecksum(uint16_t cksum) {
	/// Set Checksum data
	(((MFM_vamosPD_frame*) pHeader)->VamosPDData.Checksum) = cksum;
}
//_______________________________________________________________________________

uint16_t MFMVamosPDFrame::GetChecksum()const {
	/// Compute and return value of Checksum
	uint16_t cksum;
	cksum = (((MFM_vamosPD_frame*) pHeader)->VamosPDData.Checksum);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&cksum);
	return cksum;
}
//_______________________________________________________________________________
string MFMVamosPDFrame::DumpData(char mode, bool nozero) const{
	// Dump parameter Label and parameter value of the current event.
	// if enter parameter is true (default value), all zero parameter of event aren't dumped
	// mode = 'd' for decimal, 'b' for binary, 'h' for hexa, 'o' for octal

	stringstream ss("");
	string display("");

	int i, j, maxbin, presentation = 0, max_presentation = 5;
	char tempo[300];
	char Bin[255];
	char Bin2[255];
	uint16_t value;
	uint16_t label;
	int reste;
	char one = '1', zero = '0';
	int DecNumber = 0;

	if (mode == 'b')
		max_presentation = 3;
	if (GetEventNumber() == 0xFFFFFFFF) {
		ss << "No Event , so no event dump. Get a new event frame";
	} else {
		for (i = 0; i < VAMOSPD_NB_VALUE; i++) {
			label = 0;
			value = 0;
			GetParameters(i, &label, &value);
			if ((value != 0xFFFF) || (nozero == false)) {
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
					sprintf(tempo, "|%5d = %16s", label, Bin2);
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
void MFMVamosPDFrame::InitStat() {
	int i = 0;
	int max = MaxTrace;
	if (fLabelIndice == NULL)
		fLabelIndice = new int[max + 3];// MaxTrace + count of 0xDEF0 +count of 0xFFFF + count of overlabel.
	if (fIndiceLabel == NULL)
		fIndiceLabel = new int[max + 3];
	if (fNbLabels == NULL)
		fNbLabels = new int[max + 3];
	for (i = 0; i < max + 3; i++) {
		fNbLabels[i] = 0;
		fLabelIndice[i] = -1;
		fIndiceLabel[i] = 0;
	}
	MFMCommonFrame::InitStat();
	fNbPara = 0;

	if (fCountNbEventCard == NULL)
		fCountNbEventCard = new long long[65536];
	for (i = 0; i < 65536; i++) {
		fCountNbEventCard[i] = 0;
	}
}

//____________________________________________________________________
void MFMVamosPDFrame::FillStat() {
	MFMCommonFrame::FillStat();

	int NbItems = VAMOSPD_NB_VALUE;
	int i, indice;
	uint16_t label, value;
	for (i = 0; i < NbItems; i++) {
		GetParameters(i, &label, &value);
		if (label == 0xdef0) {
			fNbLabels[MaxTrace]++; // count of 0xDEF0
		} else {
			if (label == 0xffff) {
				fNbLabels[MaxTrace + 1]++; // count of 0xFFFF
			} else {
				if ((label >= MaxTrace)) {
					fNbLabels[MaxTrace + 2]++; // count outflow
				} else {
					if (fLabelIndice[label] == -1) {
						fLabelIndice[label] = fNbPara;
						fIndiceLabel[fNbPara] = label;

						//cout << GetCountFrame()<<" l= "<<label<<" nb para "<< fNbPara << endl;
						fNbPara++;
					}
					indice = fLabelIndice[label];
					fNbLabels[indice]++;
				}

			}
		}
	}
	uint16_t id;
	id = GetCristalId();
	fCountNbEventCard[id]++;

}
//____________________________________________________________________
string MFMVamosPDFrame::GetStat(string info)const {
	string display("");
	stringstream ss("");
	ss << MFMCommonFrame::GetStat(info);
	display = ss.str();
	//return display;
	int total1 = 0;
	uint16_t i;
	int k, j;
	int total2 = 0;

	for (k = 0; k < 65536; k++) {

		if (fCountNbEventCard[k] != 0) {
			j = k;
			ss << "Card " << ((j >> 5) & VAMOSPD_BOARD_ID_MASK);
			j = k;
			ss << " Cristal  " << (j & VAMOSPD_TRIG_REQ_CRYS_ID_MASK);
			ss << " NbEvents = " << fCountNbEventCard[k] << "\n";
			total2 += fCountNbEventCard[k];
		}
	}
	ss << "Total MFMVamosPDFrame       = " << total2 << "\n";
	ss << "      ---------------         \n";
	for (i = 0; i < fNbPara; i++) {

		ss << "Indice : " << i << "   Label :" << fIndiceLabel[i] << "  Nb : "
				<< fNbLabels[i] << endl;
		total1 += fNbLabels[i];
	}
	ss << " Total  label/data couples = " << total1 << endl;
	ss << " Count of labels  DEF0 = " << fNbLabels[MaxTrace]     << endl;
	ss << " Count of labels  FFFF = " << fNbLabels[MaxTrace + 1] << endl;
	ss << " Count of OverLabel    = " << fNbLabels[MaxTrace + 2] << endl;
	display = ss.str();
	return display;
}

//_______________________________________________________________________________
void MFMVamosPDFrame::FillDataWithRamdomValue(uint64_t timestamp,uint32_t eventnumber) {

	/// Fill all data of frame with random values to do test
	/// And report time stamp and event number
	// 4294967296 = 2^32
	float nbofv = random();
	float valtempo = VAMOSPD_NB_VALUE;
	float nbofv2 = (valtempo * nbofv / RAND_MAX);
	int i;
	SetCristalId(8, 112);
	for (i = 0; i < VAMOSPD_NB_VALUE; i++) {
		int value = random();
		uint16_t uivalue = (uint16_t) (4294967296 * value / RAND_MAX);

		if (i < nbofv2) {
			SetEnergy(i, uivalue);
			SetLabel(i, i + 1);

		} else {
			SetEnergy(i, 0);
			SetLabel(i, 0xffff);
		}
	}
	//SetCristalId(8,112);
	SetEventNumber(eventnumber);
	SetTimeStamp(timestamp);
	SetLocalCount(eventnumber);
	uint16_t cheksum = 1;

	SetChecksum(cheksum);
}
//_______________________________________________________________________________
string MFMVamosPDFrame::GetHeaderDisplay(char* infotext)const {

	stringstream ss("");
	string display("");
	display = ss.str();
	ss << MFMCommonFrame::GetHeaderDisplay(infotext);
	ss << "  BoardId =" << GetBoardId();
	ss << "  Cristal Id =" << GetTGCristalId();

	display = ss.str();
	return display;
}
//_______________________________________________________________________________


