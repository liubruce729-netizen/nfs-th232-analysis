/*
 MFMReaTraceFrame.cc
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

#include "MFMReaTraceFrame.h"

//_______________________________________________________________________________
MFMReaTraceFrame::MFMReaTraceFrame(int unitBlock_size, int dataSource, int frameType,
		int revision, int frameSize, int headerSize, int itemSize, int nItems) {
	/// Constructor of frame with a memory space\n
	/// fill header information : unitBlock_size,dataSource,....
	SetPointers();
	fCountNbEventCard = NULL;
	fChangedFrameDefinedSize =0;
	fChangedFrameItemNumber =-1;
	
}

//_______________________________________________________________________________
MFMReaTraceFrame::MFMReaTraceFrame() {
	/// Constructor of a empty frame object
	fCountNbEventCard = NULL;
	fChangedFrameDefinedSize=0;
	fChangedFrameItemNumber=-1;
	
}

//_______________________________________________________________________________
MFMReaTraceFrame::~MFMReaTraceFrame() {
	///Destructor
	if (fCountNbEventCard) {
		delete[] fCountNbEventCard;
		fCountNbEventCard = NULL;
	}
}
//_______________________________________________________________________________
void MFMReaTraceFrame::SetUserDataPointer()  {	
	pUserData_char=(char*)&(((MFM_ReaTrace_Frame*) pHeader)->MFMReaTraceItem);

}
/*
//_______________________________________________________________________________
void MFMReaTraceFrame::SetAttributs(void * pt) {
	SetPointers(pt);
	MFMBasicFrame::SetAttributs(pt);
	//SetTimeStampFromFrameData();
	//SetEventNumberFromFrameData();
}*/
//_______________________________________________________________________________
string MFMReaTraceFrame::GetHeaderDisplay(char* infotext) const{
	stringstream ss;
	string display("");
	display = ss.str();

	ss << MFMBasicFrame::GetHeaderDisplay(infotext);
	ss << endl;
	ss << "  | Channel = " << GetChannelId()<< " | SetupTrace = "<< GetSetupTrace()  <<endl; 
	display = ss.str();
	
	return display;
}
//_______________________________________________________________________________

void MFMReaTraceFrame::SetTimeStampFromReaTraceFrameData() {
	/// Compute time stamp and fill fTimeStamp attribut.
	fTimeStamp = 0;
	uint64_t* timeStamp = &(fTimeStamp);
	if (GetRevision() == 0) {
		int byte_numexo[6] = { 1, 0, 5, 4, 3, 2 };
		for (int j = 0; j < 6; j++)
			memcpy(
					((char*) (&fTimeStamp)) + j,
					((char*) ((MFM_ReaTrace_Header*) pHeader)->ReaTraceEvtInfo.EventTime)
							+ byte_numexo[j], 1);
	} else {
		memcpy(((char*) (&fTimeStamp)),
				((MFM_ReaTrace_Header*) pHeader)->ReaTraceEvtInfo.EventTime, 6);

	}
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt64((timeStamp), 6);
}
//_______________________________________________________________________________
void MFMReaTraceFrame::SetEventNumberFromReaTraceFrameData() {
	/// Compute and return envent number
	fEventNumber = 0;
	char * eventNumber = (char*) &(fEventNumber);

	fEventNumber = ((MFM_ReaTrace_Header*) pHeader)->ReaTraceEvtInfo.EventIdx;
	if (GetRevision() != 0) {
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt32((uint32_t *) (eventNumber), 4);
		}
}
//_______________________________________________________________________________
void MFMReaTraceFrame::SetTimeStamp(uint64_t timestamp) {
	// Set frame timestamp
	char* pts = (char*) &timestamp;
	timestamp = timestamp & 0x0000ffffffffffff;
	memcpy(((MFM_ReaTrace_Header*) pHeader)->ReaTraceEvtInfo.EventTime, pts, 6);
}
//_______________________________________________________________________________
void MFMReaTraceFrame::SetEventNumber(uint32_t eventnumber) {
	/// set frame event number
	((MFM_ReaTrace_Header*) pHeader)->ReaTraceEvtInfo.EventIdx = eventnumber;
}
//_______________________________________________________________________________
void MFMReaTraceFrame::SetLocationId(uint16_t Id) {
	/// Set 16 bits of LocationId
	((MFM_ReaTrace_Header*) pHeader)->ReaTraceEvtInfo.LocationId = Id;
}
//_______________________________________________________________________________
void MFMReaTraceFrame::SetLocationId(uint16_t ChannelId, uint16_t BoardId) {
	uint16_t ui, ui2;
	ui2 = 0;
	//trig request form 0 to 4
	ui2 = ChannelId & NUMEXO_CHANNEL_ID_MASK;

	//id board from 5 to 15
	ui = BoardId & NUMEXO_BOARD_ID_MASK;
	ui = ui << NUMEXO_SLIP_BITS;
	ui2 = ui2 | ui;
	SetLocationId(ui2);
}
//_______________________________________________________________________________

uint16_t MFMReaTraceFrame::GetLocationId() const{
	uint16_t Id = 0;
	/// Compute and return the 2 bytes of LocationId()
	Id = ((MFM_ReaTrace_Header*) pHeader)->ReaTraceEvtInfo.LocationId;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16((&Id));
	return ((Id));
}
//_______________________________________________________________________________
uint16_t MFMReaTraceFrame::GetChannelId() const{
	/// Compute and return extracted ChannelId
	return (GetLocationId() & NUMEXO_CHANNEL_ID_MASK);
}

//_______________________________________________________________________________
uint16_t MFMReaTraceFrame::GetBoardId()const {
	/// Compute and return id value of Board
	return (((GetLocationId() >> NUMEXO_SLIP_BITS) & NUMEXO_BOARD_ID_MASK));
}
//_______________________________________________________________________________
uint16_t MFMReaTraceFrame::GetSetupTrace()const {

	uint16_t integral = 0;
	char * pintegral = (char*) &(integral);

	integral = ((MFM_ReaTrace_Header*) pHeader)->ReaTraceEvtInfo.SetupTrace;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		if (GetRevision() > 0)
			SwapInt16((uint16_t *) (pintegral));
	return integral;
}
//_______________________________________________________________________________
void MFMReaTraceFrame::SetSetupTrace(uint16_t setup) {
	((MFM_ReaTrace_Header*) pHeader)->ReaTraceEvtInfo.SetupTrace = setup;
}
//_______________________________________________________________________________
void MFMReaTraceFrame::ReaTraceGetParameters(int i, uint16_t *value) const {
	/// Compute and return the couple information of  value of the i-th item
	ReaTraceGetParametersByItem((MFM_ReaTrace_Item *) GetItem(i), value);
}
//_______________________________________________________________________________
void MFMReaTraceFrame::ReaTraceGetParametersByItem(MFM_ReaTrace_Item *item, uint16_t *value)const {
	/// Compute and return the couple information of value of  item

	if (fLocalIsBigEndian != fFrameIsBigEndian) {
		uint16_t tmp = item->Value;
		SwapInt16(&tmp);
		*value = tmp;
	} else {
		*value = item->Value;
	}
	
}
//_______________________________________________________________________________
void MFMReaTraceFrame::ReaTraceSetParameters(int i, uint16_t value) {
	/// set i-th item if frame with the couple information of value
	ReaTraceSetParametersByItem((MFM_ReaTrace_Item *) GetItem(i), value);

}
//_______________________________________________________________________________
void MFMReaTraceFrame::ReaTraceSetParametersByItem(MFM_ReaTrace_Item *item, uint16_t value) {
	/// set "item" with the couple information of value
	static int toto;
	item->Value = value;
}
//_______________________________________________________________________________
void MFMReaTraceFrame::SetCheckSum(uint16_t checksum) {
	uint16_t * pchecksum ;
	pchecksum = (uint16_t * )(((char*) pHeader) + REA_TRACE_HEADERSIZE + GetNbItems()*sizeof(MFM_ReaTrace_Item));
	*pchecksum=checksum;
	//((MFM_ReaTrace_Frame*) pHeader)->CheckSum.CheckSum = checksum;
}
//_______________________________________________________________________________
uint16_t MFMReaTraceFrame::GetCheckSum()const {

	uint16_t checksum = 0;
	char * pchecksum = (char*) &(checksum);
        uint16_t * pt=  (uint16_t * )(((char*) pHeader) + REA_TRACE_HEADERSIZE + GetNbItems()*sizeof(MFM_ReaTrace_Item));
        checksum =*pt;
	//checksum = ((MFM_ReaTrace_Frame*) pHeader)->CheckSum.CheckSum;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		if (GetRevision() > 0)
			SwapInt16((uint16_t *) (pchecksum));
	return checksum;
}
//_______________________________________________________________________________
void MFMReaTraceFrame::FillDataWithRamdomValue(  uint64_t timestamp, uint32_t enventnumber,int nbitem) {
	/// Fill frame items  of sinus values with random perios,

	int nb_bits        = 16;
	float nbofperiodes = 1;
    int16_t board      = 112;
	int i;
	static int16_t channel = 0;
	float P;
	static unsigned int seed = 15;
	unsigned short uivalue;
	float rando, tempof;
	P = 1.570796327 * 2;
	rando = seed;
        channel ++;
        channel = channel % NUMEXO_NB_CHANNELS;
	int h;
	h = pow(2, nb_bits) - 1;
        SetLocationId(channel,board);
	if (nbitem > 0)
		ReaTraceSetParameters(0, 1);
	rando = (float) ((rand_r(&seed) / (RAND_MAX + 1.0)));
	seed++;
	
        SetSetupTrace(1966);
	for (i = 1; i < nbitem; i++) {
		seed++;
		nbofperiodes = 1 + rando * 4;
		tempof = (float) i / (nbitem / (nbofperiodes));
		uivalue = (unsigned short) (h / 2 + h / 2 * sin((float) 2 * P * tempof));
		ReaTraceSetParameters(i, uivalue);
	}
	SetEventNumber(enventnumber);
	SetTimeStamp(timestamp);
	SetLocationId(1, board);
	SetCheckSum(66);

}

//_______________________________________________________________________________
string MFMReaTraceFrame::DumpData(char mode, bool nozero) const {
	// Dump  values of the current event.
	// if enter parameter is true (default value), all zero parameter of event aren't dumped
	// mode = 'd' for decimal, 'b' for binary, 'h' for hexa, 'o' for octal

	stringstream ss;
	string display("");

	int i, j, maxbin, presentation = 0, max_presentation = 5;
	char tempo[300];
	char Bin[255];
	char Bin2[255];

	int reste;
	char one = '1', zero = '0';
	int DecNumber = 0;

	if (mode == 'b')
		max_presentation = 3;
	if (GetEventNumber() == 0xFFFFFFFF) {
		ss << "No Event , so no event dump. Get a new event frame";
	} else {
		for (i = 0; i < GetNbItems(); i++) {

			uint16_t value = 0;
			ReaTraceGetParameters(i, &value);
			if ((value != 0xFFFF) || (nozero == false)) {
				switch (mode) {
				case 'd':// decimal display
					sprintf(tempo, "|%5d ", value);
					break;
				case 'o':// octal display
					sprintf(tempo, "|%5o ", value);
					break;
				case 'h':// hexa display
					sprintf(tempo, "|%5x x", value);
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
					sprintf(tempo, "|%16s ", Bin2);
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
void MFMReaTraceFrame::InitStat() {
	MFMBasicFrame::InitStat();
	if (fInitStatDone == false )return;
	int i;
	fCountNbEventCard = new long long[NUMEXO_MAX_NUMB_BOARDS];
	for (i = 0; i < NUMEXO_MAX_NUMB_BOARDS; i++) {
		fCountNbEventCard[i] = 0;
	}
}
//_______________________________________________________________________________
void MFMReaTraceFrame::FillStat() {
	if (fInitStatDone == false )return;
	MFMBasicFrame::FillStat();
	uint16_t id;

	id = GetLocationId();
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
//_______________________________________________________________________________
string MFMReaTraceFrame::GetStat(string info)const {
	if (fInitStatDone == false )return(string)"";
	string display("");
	stringstream ss("");
	ss << MFMBasicFrame::GetStat(info);
	int i, j;
	int total = 0;
	for (i = 0; i < NUMEXO_MAX_NUMB_BOARDS; i++) {
		if (fCountNbEventCard[i] != 0) {
			j = i;
			ss << "Card " << ((j >> 4) & NUMEXO_BOARD_ID_MASK);
			j = i;
			ss << " Cristal  " << (j & NUMEXO_CHANNEL_ID_MASK);
			ss << " NbEvents = " << fCountNbEventCard[i] << "\n";
			total += fCountNbEventCard[i];
		}
	}
	
	ss << " Total                        = " << total << "\n";
	display = ss.str();
	return display;
}
//_______________________________________________________________________________
void MFMReaTraceFrame::ChangeDefinedFrameSize(int size) {
// this is used to do test on a specitfic frame, and DAQ
 if (size <REA_TRACE_HEADERSIZE+ sizeof (MFM_ReaTraceCheckSum)) size = REA_TRACE_HEADERSIZE + sizeof (MFM_ReaTraceCheckSum);
 fChangedFrameItemNumber = (size - (REA_TRACE_HEADERSIZE + sizeof (MFM_ReaTraceCheckSum)) )/sizeof(MFM_ReaTrace_Item);
 fChangedFrameDefinedSize =size;
 
}//_______________________________________________________________________________
  int MFMReaTraceFrame::GetDefinedNbItems()const {
  if (fChangedFrameItemNumber<0) return REA_TRACE_NB_OF_ITEMS;
  else return fChangedFrameItemNumber;
}
//_______________________________________________________________________________
  int MFMReaTraceFrame::GetDefinedFrameSize()const{ 
   if (fChangedFrameDefinedSize ==0) return REA_TRACE_FRAMESIZE;
   
   else return fChangedFrameDefinedSize;
   
   }

//_______________________________________________________________________________
bool MFMReaTraceFrame::IsSame(MFMReaTraceFrame* testframe,int verbose){
// test if testframe have same attibut values
// return true if OK

string display("");
stringstream ss("");
bool test= true;
bool testloc= true;
int i=0;  
uint16_t value;
uint16_t valuetest;
ss << "   Test for "<< MyClassName()<<" :" ;
testframe->MFMReaTraceFrame::SetAttributs();
MFMReaTraceFrame::SetAttributs();

test= MFMBasicFrame::IsSame((MFMBasicFrame*)testframe,verbose);

int nbitems= testframe->GetNbItems();

testloc =  ( GetSetupTrace()== testframe->GetSetupTrace());
ss << testloc ; test =  test and testloc;

testloc= true;
for (i =0; i< nbitems; i++){
	ReaTraceGetParameters(i,  &value);
	testframe ->ReaTraceGetParameters(i, &valuetest);
	testloc = testloc and (valuetest==value);
}
ss << testloc ; test =  test and testloc;

testloc =  ( GetCheckSum()== testframe->GetCheckSum());
ss << testloc ; test =  test and testloc;
display = ss.str();
if ((verbose>0))
   cout << display <<"  (Setup Data Check)" <<endl;
return test;
}
//_______________________________________________________________________________
bool MFMReaTraceFrame::UnitTest(int verbose){
// Test this class
// this methode generate a frame write it in a tmpfile 
// read this frame with a other object frame and after comare the both frame
// return true if OK , false if noOK		

int lun; // Logical Unit Number
UtilVector_c* vector = new UtilVector_c(MFM_BLOB_HEADER_SIZE); // min size =8 

bool test =true;
char  tmpfilename[256];
sprintf ( tmpfilename,"test%s.tmp",MyClassName());
int type = MFM_REA_TRACE_FRAME_TYPE;
int dump =0;
if (verbose>9)dump =64;
//generation and write 
     MFMReaTraceFrame * framewrite = new MFMReaTraceFrame();    
     lun = open(tmpfilename, (O_RDWR | O_CREAT | O_TRUNC), 0644);
     framewrite->WriteRandomFrame(lun,1, 0, 0,type);
     close(lun);  

//read
     lun = open(tmpfilename, (O_RDONLY));     
     MFMReaTraceFrame * frameread = new MFMReaTraceFrame();
     frameread->ReadInFile(&lun, vector);    
//compare
     cout << ">--Test begin for "<<frameread->MyClassName()<<"--------"<<endl;
     test=framewrite->IsSame(frameread,verbose);
     cout << ">--Result for "<<frameread->MyClassName()<<" = "<< test<<endl;
     if (verbose >5) {
    	framewrite->ReadAttributsExtractFrame(verbose,dump, true,0,framewrite->GetPointHeader());
    	frameread->ReadAttributsExtractFrame(verbose,dump, true,0,frameread->GetPointHeader());
     	}
//end
     delete (framewrite);
     delete (frameread);
     if(vector) delete(vector);
     lun = close(lun);
     return test;
}
//_______________________________________________________________________________
//_______________________________________________________________________________

