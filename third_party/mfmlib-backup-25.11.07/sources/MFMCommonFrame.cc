/*
 MFMCommonFrame.cc

 Copyright Acquisition group, GANIL Caen, France

 */

#include <iostream>
#include <cmath>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <cstdlib>
#include <sys/time.h>
using namespace std;
#include "MFMCommonFrame.h"

string MFMCommonFrame::indentation = "";
//_______________________________________________________________________________
MFMCommonFrame::MFMCommonFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize) {
	/// Constructor of frame with a memory space\n
	/// fill header information : unitBlock_size,dataSource,....
	Init();
	int minsize = MFM_BLOB_HEADER_SIZE;
	if (minsize > frameSize) {
		cout << "Error of frame size (" << frameSize << ") > size of header ("
				<< minsize << ")\n";
		throw(MFM_BLOCK_SIZE);
	}
	SetBufferSize(frameSize);
	MFM_make_header(unitBlock_size, dataSource, frameType, revision, frameSize);
	fFrameSize = frameSize;
	fBoardId  =-1;
}

//_______________________________________________________________________________
MFMCommonFrame::MFMCommonFrame(MFMCommonFrame *frame) {
/// Constructor \n
	Init();
	int i = 0;
	char *pttest;
	char *thispttest;
	frame->SetAttributs();
	int size = frame->GetFrameSize();
	SetBufferSize(size);
	thispttest = (char*) (GetPointHeader());
	pttest = (char*) (frame->GetPointHeader());
	for (i = 0; i < size; i++) {
		(thispttest[i] = (char) ((pttest[i])));
	}
	SetAttributs();
}

//_______________________________________________________________________________
MFMCommonFrame::MFMCommonFrame() {
	/// Constructor of a empty frame object
	Init();

}
;
//_______________________________________________________________________________
MFMCommonFrame::~MFMCommonFrame() {
	/// Destructor
	if (pDataNew) {
		free (pDataNew);
		pDataNew = NULL;
		pData = NULL;
		pData_char = NULL;
		pUserData_char = NULL;
	}
}

//_______________________________________________________________________________

void MFMCommonFrame::Init() {
	///
	/// Initialization of MFMCommonFrame object\n
	///
	fFrameSize = 0;
	fTimeDiff = 0;
	SetTimeDiffUs();
	pData = NULL;
	pDataNew = NULL;
	pHeader = NULL;
	fHeaderSize = MFM_BLOB_HEADER_SIZE;
	pReserveHeader = NULL;
	pData_char = NULL;
	fIncrement = 0;
	fBufferSize = 0;
	fSizeOfUnitBlock = 0;
	fLocalIsBigEndian = (Endianness() == MFM_BIG_ENDIAN);
	fFrameIsBigEndian = false;
	fCountFrame = 0;
	fTimeStamp = 0;
	fFrameType = 0;
	fWantedFrameType = 0;
	fInitStatDone = false;
}
//_______________________________________________________________________________
void MFMCommonFrame::SetUserDataPointer() {
	SetHeaderSizeFromFrameData();
	pUserData_char = pData_char + GetHeaderSize() + 6 * (int) (HasTimeStamp())
			+ 4 * (int) (HasEventNumber());
}

//_______________________________________________________________________________
void MFMCommonFrame::SetHeader(MFM_topcommon_header *header) {
	/// Set pointer of on header of frame\n
	pHeader = header;
}
;
//_______________________________________________________________________________
void MFMCommonFrame::DumpRaw(int dumpsize, int increment) const {
	///
	/// Display dump of frame realised by GetDumpRaw(...\n
	/// dumpsize  : size of dump\n
	/// increment : begin of dump
	///
	cout << (GetDumpRaw(dumpsize, increment)).data();
	return;
}

//_______________________________________________________________________________

string MFMCommonFrame::GetDumpRaw(int dumpsize, int increment) const {
	///
	/// Creat a string of  dump of frame\n
	/// dumpsize  : size of dump if dumpsize =0 , dumpsize = standard = 256\n
	/// increment : begin of dump
	///

	string mydump;

	int framesize = GetFrameSize();
	if (dumpsize == 0)
		dumpsize = framesize;

	if ((increment > 0) && (increment > dumpsize))
		fIncrement = increment;

	if (increment < 0) {
		increment = 0;
		fIncrement = 0;
	}

	if ((increment > 0) && (increment <= framesize)) {
		if (dumpsize + fIncrement > framesize)
			dumpsize = framesize - fIncrement;

		if (dumpsize == 0) {
			dumpsize = 256;
			fIncrement = 0;
		}
	}

	GetDumpRaw((void*) pData, dumpsize, fIncrement, &mydump);

	//} else {
	//mydump += "\n\t end of bloc \n";
	//}
	return mydump;
}


//_______________________________________________________________________________

void MFMCommonFrame::GetDumpRaw(void *point, int dumpsize, int increment,
		string *mydump) const {

	///  Creat a string of dump of memory space\n
	///  point : pointer bo dump begin\n
	///  if dumpsize =0 , dumpsize = standard = 256 and fGBbuf_increment raz\n
	///  if mydump==NULL dump is displayed else dump is retuen in  string * mydump

	string *mydumploc;

	if (mydump == NULL) {
		string st;
		mydumploc = &st;
	} else {
		mydumploc = mydump;
	}

	int i, k;
	int nbrcol = 16; // nb de colonnes affich�es

	int asciimin = 32; // range min of a char to be ascii character
	char tempo[128] = "";
	char chartmp = 0;

	int asciimax = 127; // idem but max
	int nbrperline = 16; // nbr of bytes per line
	int nbrline = 0; // nbr lines
	int dumpsize2 = dumpsize;
	int dumpsize3 = dumpsize;
	unsigned char *pChar = NULL;
	unsigned char *pChar2 = NULL;

	nbrline = (int) (dumpsize / nbrperline)
			+ (int) ((dumpsize % nbrperline) != 0);

	pChar = (unsigned char*) ((char*) point + fIncrement);
	pChar2 = pChar;

	if (nbrline < 1)
		nbrline = 1;
	if (nbrcol > dumpsize)
		nbrcol = dumpsize;
	if (increment == dumpsize)
		nbrline = 1;
	for (i = 0; i < nbrline; i++) {
		sprintf(tempo, "\n%5d %s ", increment, ": ");
		*mydumploc += tempo;
		for (k = 0; k < nbrcol; k++) {
			sprintf(tempo, "%02hX ", (unsigned short) ((*(pChar2++))));
			*mydumploc += tempo;
			dumpsize3--;
			if (dumpsize3 == 0)
				break;
		}
		*mydumploc += "  ";

		for (k = 0; k < nbrcol; k++) {
			chartmp = pChar[0];
			//memcpy (&chartmp, pChar ,1);
			if ((chartmp >= asciimin) && (chartmp <= asciimax)) {
				sprintf(tempo, "%c", *pChar);
				*mydumploc += tempo;
			} else
				*mydumploc += ".";
			pChar++;
			dumpsize2--;
			if (dumpsize2 == 0)
				break;
		}
		increment += nbrperline;
	}
	*mydumploc += "\n";

	if (mydump == NULL)
		cout << (*mydumploc).data();
}

//_______________________________________________________________________________

bool MFMCommonFrame::isBigEndian() const {
	/// return attribut containing bigendian information
	return fLocalIsBigEndian;
}

//_______________________________________________________________________________

uint8_t MFMCommonFrame::GetMetaType() const {
	/// return MetaType byte of frame
	return pHeader->hd.metaType;
}
//_______________________________________________________________________________
void MFMCommonFrame::SetUnitBlockSizeFromFrameData() {
	/// Compute Unit BlockSize of frame
	int tmp = pHeader->hd.metaType & MFM_UNIT_BLOCK_SIZE_MSK;
	fSizeOfUnitBlock = pow((double) 2, tmp);

}

//_______________________________________________________________________________
uint16_t MFMCommonFrame::GetDataSource() const {
	/// Return Data Source information of Frame
	return (uint16_t) pHeader->hd.dataSource;
}

//_______________________________________________________________________________
uint16_t MFMCommonFrame::GetFrameType() const {
	/// Return attribut containing frame type
	return fFrameType;
}
//_______________________________________________________________________________
void MFMCommonFrame::SetFrameTypeFromFrameData() {
	/// Compute and set attribut of frame type\n
	/// the indianess conversion is realised if necessary

	uint16_t tmp;
	tmp = pHeader->hd.frameType;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt16(&tmp);
	fFrameType = tmp;
}

//_______________________________________________________________________________
uint8_t MFMCommonFrame::GetRevision() const {
	/// Return revision frame informationif

	return ((uint32_t) pHeader->hd.revision);
}
//_______________________________________________________________________________
int MFMCommonFrame::GetBlobNess() const {
	/// Return BlobNess frame information
	return (uint32_t)((pHeader->hd.metaType & MFM_BLOBNESS_MSK) >> 6);
}
//_______________________________________________________________________________
void MFMCommonFrame::SetFrameSizeFromFrameData() {
	/// Compute and set attributi of frame size\n
	/// the indianess conversion is realised if necessary

	uint32_t tmp;
	tmp = pHeader->hd.frameSize;
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt32(&tmp, 3);
	if (fSizeOfUnitBlock == 0)
		SetUnitBlockSizeFromFrameData();
	tmp = tmp * (uint32_t) fSizeOfUnitBlock;
	fFrameSize = tmp;
}
//_______________________________________________________________________________

void SetHeaderSizeFromFrameData() {
// Nothing to do??
}
//_______________________________________________________________________________

void MFMCommonFrame::SetEventNumberFromFrameData()  {
// return time stamps of frame , if no TS => 0 
	fEventNumber = 0;
	int Shift = GetShiftEN(GetFrameType());
	if (Shift != 0) {
		char *ptEvt = NULL;
		ptEvt = (char*) (GetPointHeader()) + Shift;

		/// Computer, set attibut and return value of event number from  frame
		fEventNumber = 0;
		char *peventNumber = (char*) &(fEventNumber);
		fEventNumber = *((uint32_t*) ptEvt);
		if (fLocalIsBigEndian != fFrameIsBigEndian)
			SwapInt32((uint32_t*) (peventNumber), 4);
	}

}

//_______________________________________________________________________________


/*
void MFMNumExoFrame::SetTimeStampFromNumexoFrameData() {
	/// Computer, set attibut and return value of time stamp from  frame
	fTimeStamp = 0;
	uint64_t * timeStamp = &(fTimeStamp);
	memcpy(((char*) (&fTimeStamp)),
			((MFM_numexo_frame*) pHeader)->EventInfo.EventTime, 6);
	if (fLocalIsBigEndian != fFrameIsBigEndian)
		SwapInt64((timeStamp), 6);
}*/


//_______________________________________________________________________________

//uint64_t MFMCommonFrame::GetTimeStampFromCommonFrameData() const {

void MFMCommonFrame::SetTimeStampFromFrameData(){
/// Computer, set attibut and return value of time stamp from  frame
	fTimeStamp = 0;
	int Shift = GetShiftTS(GetFrameType());
	if (Shift != 0) {
		char *ptTS = NULL;
		ptTS = (char*) (GetPointHeader()) + Shift;
		uint64_t *ptimeStamp = &(fTimeStamp);
		memcpy(((char*) (&fTimeStamp)), ptTS, 6);
		if (fLocalIsBigEndian != fFrameIsBigEndian)
			SwapInt64((ptimeStamp), 6);
	}
}

//_______________________________________________________________________________
uint64_t MFMCommonFrame::GetTimeStamp() const {
	/// Return value of TimeStamp
	/// This value is computer by daugther class 
	return ((uint64_t) fTimeStamp);
}


//_______________________________________________________________________________
void  MFMCommonFrame::SetBoardAndChannelFromFrame() {
	 /// Computer, set attibut and return value of Boad IF  from  frame 
	 /// for Numexo , it is board id, for GET card it is Cobo Card 
	 /// for Numexo , it is channel id, for GET  card it is Adad Car
	 /// fo other card , should be -1
	 
	 fBoardId   = -1;
	 fChannelId = -1;
	 int Shift= GetShiftLocation(GetFrameType());
	 char *ptLoc = NULL;
	 uint16_t location = 0;	
	 ptLoc   = (char*)((long long int) GetPointHeader()  + Shift);
	 memcpy(((char*) (&location)), ptLoc, 2);
	 if (Shift!=0){
		if ((GetFrameType()==MFM_COBO_FRAME_TYPE) or (GetFrameType()==MFM_COBOF_FRAME_TYPE)){
			char tmp;
			tmp = *(ptLoc);
			fBoardId = (uint16_t)tmp;
			tmp = *(ptLoc+1); 
			fChannelId = (uint16_t)tmp;
		}else if (GetFrameType()==MFM_FASTER_FRAME_TYPE){
			// 1000 est la separation des types de  données faster 
			fChannelId = location%1000;
			
			fChannelId = fChannelId %20; // 20 est un valeur arbitraire ,il sera nécessaire que les labels des voies des cartes faster fasse ce saut ( ex carte1 : label 3001, 3002, 3003, 3004    carte2 :label  3021, 3022,...)
			fBoardId   = fChannelId/20;
		}else {	
			if (fLocalIsBigEndian != fFrameIsBigEndian)
				SwapInt16((&location));
			fChannelId = location & NUMEXO_CRYS_MASK ;
			fBoardId   = ((location >> NUMEXO_SLIP_BITS) & NUMEXO_BOARD_ID_MASK);
			
		}
	
	}
 };

 

//_______________________________________________________________________________

void MFMCommonFrame::SetMetaType(int unitBlock_size, bool isablob) {
	/// Set MetaType byte of frame\n
	/// realize all coding computation : unitBlock_size , blog and endianness
	unsigned char endianness;
	unsigned char blobness;
	fSizeOfUnitBlock = unitBlock_size;
	double j = log10(unitBlock_size) / log10(2);
	double l = pow((double) 2, (int) j);

	if (fLocalIsBigEndian)
		endianness = MFM_BIG_ENDIAN;
	else
		endianness = MFM_LITTLE_ENDIAN;

	fFrameIsBigEndian = fLocalIsBigEndian;

	if (isablob)
		blobness = MFM_BLOBNESS_MSK;
	else
		blobness = 0;
	if (l != unitBlock_size)
		throw MFM_ERR_UNIT_BLOCK_SIZE; // unitBlock_size is not a power**2

	uint32_t block_size_power_2 = (int) j;

	if (block_size_power_2 > 16)
		throw MFM_ERR_UNIT_BLOCK_SIZE; // unitBlock_size out of range

	pHeader->hd.metaType = (endianness | block_size_power_2 | blobness);

}
//_______________________________________________________________________________
void MFMCommonFrame::SetUnitBlockSize(int bsize) {
	/// Set UnitBlockSize inside metatype byte
	pHeader->hd.metaType = bsize & MFM_UNIT_BLOCK_SIZE_MSK;
}

//_______________________________________________________________________________
void MFMCommonFrame::SetDataSource(uint8_t source) {
	/// Set Data source of frame
	pHeader->hd.dataSource = source;
}

//_______________________________________________________________________________
void MFMCommonFrame::SetFrameType(uint16_t frametype) {
	/// Set type of frame
	fFrameType = frametype;
	if (pHeader != NULL)
		pHeader->hd.frameType = frametype;
}

//_______________________________________________________________________________
void MFMCommonFrame::SetRevision(uint8_t revision) {
	/// Set revision of frame
	pHeader->hd.revision = revision;
}
//_______________________________________________________________________________
void MFMCommonFrame::SetFrameSize(uint32_t size) {
	/// Set Frame Size of frame ( only part on 3 bytes) 
	/// UnitBlock size is not affected by this method
	uint32_t test = size & MFM_FRAME_SIZE_MASK;
	pHeader->hd.frameSize = test;
}

//_______________________________________________________________________________
void MFMCommonFrame::HeaderDisplay(char *infotext) const {
	/// Display  Header information realised by GetHeaderDisplay()\n
	/// if infotext is not NULL replace the standart "MFM header" title
	cout << (GetHeaderDisplay(infotext));
}
//_______________________________________________________________________________
string MFMCommonFrame::GetHeaderDisplay(char *infotext) const {
	/// Return a string containing infomation of MFM Header\n
	/// if infotext is not NULL replace the standart "MFM header" title
	string display("");
	bool blob;
	stringstream ss("");

	//ss << endl;
	int type = GetFrameType();
	if ((pHeader->hd.metaType & MFM_BLOBNESS_MSK) == 0)
		blob = false;
	else
		blob = true;
	if (infotext == NULL)
		ss << "Frame Type : " << GetTypeText() << " ";
	else
		ss << MFMCommonFrame::indentation << infotext;

	ss << MFMCommonFrame::indentation << " " << dec << type << hex << "(0x"
			<< type << dec << ")   FrameSize=" << GetFrameSize() << hex << "(0x"
			<< GetFrameSize() << ")" << dec << "  MetaType="
			<< (int) GetMetaType() << hex << "(0x" << (int) GetMetaType() << ")"
			<< dec << "  B=" << blob << "  UBS=" << fSizeOfUnitBlock << endl;
	if ((GetShiftEN(type) > 0) || (GetShiftTS(type) > 0))
		ss << MFMCommonFrame::indentation;
	if ((GetShiftEN(type) > 0))
		ss << "  EN = " << dec << GetEventNumber();
	if ((GetShiftTS(type) > 0))
		ss << "  TS = " << dec << GetTimeStamp() << " (0x"
				<< hex << GetTimeStamp() << ")" << dec;

	//if(HasEventNumber()||HasTimeStamp()) ss << MFMCommonFrame::indentation;
	//if(HasEventNumber()) ss << "  EN2 = "  << GetEventNumber();
	//if(HasTimeStamp())   ss << "  TS2 = "  << GetTimeStamp() << " (0x" << hex << GetTimeStamp() << ")"<< dec ;

	if (HasBoardId())
		ss << "  Board =" << GetBoardId();
	ss << MFMCommonFrame::indentation << "  Source = " << GetDataSource() << hex
			<< "(0x" << GetDataSource() << ")" << dec << "  Rev = "
			<< (int) GetRevision() << hex << "(0x" << (int) GetRevision() << ")"
			<< dec;
	//ss << "  pointer = "<< (long long*) GetPointHeader(); 
	ss << endl;

	display = ss.str();
	return display;

} //_______________________________________________________________________________
unsigned char MFMCommonFrame::GetFrameEndianness() const {
	/// Return  BIG_ENDIAN or LITTLE_ENDIAN value of current computer for metaType format

	unsigned char tmp = pHeader->hd.metaType & MFM_ENDIANNESS_MSK;
	return (tmp);

}
//_______________________________________________________________________________
unsigned char MFMCommonFrame::Endianness(void) const {
	/// Return  endianness value of current computer 
	// return value is LITTLE_ENDIAN(=0x80)  or BIG_ENDIAN (=0) 
	// example of little endian 
	//  for decimal value of 3  on a 4 bytes
	// LITTLE_ENDIAN  1024 -> a[0]=3  ;a[1] = 0 ; a[2]=0  ;a[3] = 0 
	// BIG   _ENDIAN  1024 -> a[0]=0  ;a[1] = 0 ; a[2]=0  ;a[3] = 3 
	unsigned char LsbFlag;
	union {
		uint16_t s;
		uint64_t l;
	} b;

	b.l = 0;
	b.s = 1;
	if (b.l == b.s)
		LsbFlag = MFM_LITTLE_ENDIAN; /* LSB (little endian) car short = long  ( case of my x86-64,)*/
	else
		LsbFlag = MFM_BIG_ENDIAN; /* MSB (big endian) car byte inverses */
	return LsbFlag;
}

//_______________________________________________________________________________

bool MFMCommonFrame::is_power_2(int i) const {
	/// Test if i is a power of 2\n

	double j = log10(i) / log10(2);
	int k = (int) j;
	cout << i << " - " << i % 2 << " - " << j << " - " << k;

	double l = pow((double) 2, (int) j);
	if (l != i)
		return false;
	else
		return true;
}

//_______________________________________________________________________________

void MFMCommonFrame::MFM_fill_header(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, bool blob) {
	/// Fill a Header with a list of parameters:\n
	/// unitBlock_size, dataSource, frameType, revision, frameSize, blob\n
	if (pHeader) {
		SetMetaType(unitBlock_size, blob);
		SetFrameSize((uint32_t) frameSize);
		SetDataSource((uint8_t) dataSource);
		SetFrameType((uint16_t) frameType);
		SetRevision((uint8_t) revision);

	} else {
		cout << "MFMCommonFrame::MFM_fill_header , Error of header null\n";
	}
}
//_______________________________________________________________________________
void MFMCommonFrame::MFM_make_header(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, bool blob) {
	/// Do memory allocation for frame and\n
	/// fill its Header with a list of parameters:\n
	/// unitBlock_size, dataSource, frameType, revision, frameSize, blob\n
	SetBufferSize(frameSize * unitBlock_size);

	if (!pHeader)
		cout << "MFMCommonFrame::MFM_make_header , Error of header null\n";
	MFM_fill_header(unitBlock_size, dataSource, frameType, revision, frameSize,
			blob);
	SetPointers();
}

//_______________________________________________________________________________
void MFMCommonFrame::SetBufferSize(int size, bool ifinferior) {
	/// Do memory allocation or a reallacation for frame\n
	/// if ifinferior==true the allocaton is forced to size even if the acutal size is bigger\n
	if (size == fBufferSize)
		return;
	if (!ifinferior or (size > fBufferSize)) {
		if (pData == NULL) {
			pDataNew = (char*) (malloc(size));
		} else
			pDataNew = (char*) (realloc((void*) pData, size));
		for (int i = fBufferSize; i < size; i++)
			((char*) pDataNew)[i] = 0;
		pData = pDataNew;
		SetPointers();
		pReserveHeader = NULL;
		fBufferSize = size;
	}
}
//_______________________________________________________________________________
void MFMCommonFrame::SetPointers(void *pt) {
	/// Initialize pointers of frame\n
	/// if pt==NULL initialization is with current value of main pointer of frame (pData)\n
	/// else initialization is done with pData = pt\n
	/// pData must be the reference;
	if (pt != NULL) {
		pData = pt;
	}
	pHeader = (MFM_topcommon_header*) pData;
	fFrameIsBigEndian = (GetFrameEndianness() == MFM_BIG_ENDIAN);
	pData_char = (char*) pData;
	SetUnitBlockSizeFromFrameData();
	//SetUserDataPointer();
}
//_______________________________________________________________________________
void MFMCommonFrame::SetAttributs(void *pt) {
	/// Initialize a set of attributs (frame size, endianess, type ...) and pointers of frame\n
	/// reading and computing data comming from header of frame\n
	/// if pt==NULL initialization is done with current value of main pointer of frame (pData)\n
	/// else initialization is done with pData = pt
	fLocalIsBigEndian = (Endianness() == MFM_BIG_ENDIAN);
	SetPointers(pt);
	SetFrameTypeFromFrameData(); //Attention, must be done before SetTimeStampFromFrameData and SetTimeStampFromFrameData !	
	SetEventNumberFromFrameData();
	SetFrameSizeFromFrameData();
	SetTimeStampFromFrameData();
	SetBoardAndChannelFromFrame();
	SetUserDataPointer();
}
//_______________________________________________________________________________
void MFMCommonFrame::CopyFrameAndResizeFrameIfNecessary(MFMCommonFrame *frame) {
// resize Frame if necessary and copy vector in frame supposed to contain a MFM frame 
	frame->SetAttributs();
	if (frame->GetFrameSize() > GetBufferSize()) {
		SetBufferSize(frame->GetFrameSize());
	}
	memcpy(GetDataPointer(), frame->GetDataPointer(), frame->GetFrameSize());
	SetAttributs();
}
//_______________________________________________________________________________
void MFMCommonFrame::SetAttributsOn4Bytes(void *pt) {
	/// Initialize a set of attributs (frame size, endianess, type ...) and pointers of frame\n
	/// reading and computing data comming from header of frame\n
	/// if pt==NULL initialization is done with current value of main pointer of frame (pData)\n
	/// else initialization is done with pData = pt
	// this is done only on the first 4th bytes so type is not read
	fLocalIsBigEndian = (Endianness() == MFM_BIG_ENDIAN);
	SetPointers(pt);
	SetUnitBlockSizeFromFrameData();
	SetFrameSizeFromFrameData();

}
//_______________________________________________________________________________
void MFMCommonFrame::SetAttributsOn8Bytes(void *pt) {
	/// Initialize a set of attributs (frame size, endianess, type ...) and pointers of frame\n
	/// reading and computing data comming from header of frame\n
	/// if pt==NULL initialization is done with current value of main pointer of frame (pData)\n
	/// else initialization is done with pData = pt
	// this is done only on the first 4th bytes so type is not read
	fLocalIsBigEndian = (Endianness() == MFM_BIG_ENDIAN);
	SetPointers(pt);
	SetUnitBlockSizeFromFrameData();
	SetFrameSizeFromFrameData();
	SetFrameTypeFromFrameData();
}	
//_______________________________________________________________________________
void MFMCommonFrame::ReadAttributsExtractFrame(int verbose, int dumpsize,
		bool display, int noframe, void *pt) {
// set attributs, fill stat and diplay information if display == true
	SetAttributs(pt);
	if (display)
		ExtractInfoFrame(verbose, dumpsize, noframe);
	FillStat();
	TestUserPointer(noframe, verbose);

}
//_______________________________________________________________________________

void MFMCommonFrame::WriteRandomFrame(int lun, int nbframe, int verbose,
		int dumpsize, int type) {
/// write  frames in file with ramdom data
	uint64_t ts;
	int verif;
	uint32_t framesize = 0;
	bool display = true;
	SetWantedFrameType(type);
	for (int i = 0; i < nbframe; i++) {
		ts = GetTimeStampUs();
		GenerateAFrameExample(ts, i);
		ReadAttributsExtractFrame(verbose, dumpsize, display, i);
		framesize = GetFrameSize();
		verif = write(lun, GetPointHeader(), framesize);
		if (verif != framesize)
			fError.TreatError(2, 0, "Error of write");
	}
}
//_______________________________________________________________________________

void MFMCommonFrame::GenerateAFrameExample(uint64_t timestamp,
		uint32_t eventnumber) {
	/// Generate a example of frame containing random value\n
	/// usable for tests.

	uint32_t unitBlock_size = GetDefinedUnitBlockSize();
	;
	uint16_t type = GetWantedFrameType();
	uint32_t framesize = GetDefinedFrameSize();
	uint32_t revision = 1;
	uint16_t source = 0xff; // standard value when produced by a desktop computer
	uint32_t headersize = GetDefinedHeaderSize();
	/*cout <<dec<< "MFMCommonFrame GenerateAFrameExample GetWantedFrameType       = "<< GetWantedFrameType()<<"\n";
	 cout << "MFMCommonFrame GenerateAFrameExample GetDefinedUnitBlockSize  = "<< GetDefinedUnitBlockSize()<<"\n";
	 cout << "MFMCommonFrame GenerateAFrameExample GetframeSize             = "<< framesize<<"\n";
	 cout << "MFMCommonFrame GenerateAFrameExample GetDefinedFrameSize      = "<< GetDefinedFrameSize()<<"\n";
	 cout << "MFMCommonFrame GenerateAFrameExample GetDefinedHeaderSize     = "<< headersize<<"\n";
	 */
	// generation of MFM header , in this case, MFM is same for all MFM frames
	MFM_make_header(unitBlock_size, source, type, revision,
			(int) (framesize / unitBlock_size), (headersize / unitBlock_size));
	FillDataWithRamdomValue(timestamp, eventnumber);
}

//_______________________________________________________________________________
int MFMCommonFrame::FillBigBufferFromFile(int fLun, char *vector,
		unsigned int vectorsize, unsigned int *readsize,
		unsigned int *eventcount) {
	/// Get data from a file, and fill current frame and initialize its attributs and its pointer
	/// if size of actual frame is not enough, and new size is reallocated
	///      fLun   : descriptor of file (given by a previous open)
	///      vector : pointer buffer .
	///      vectorsize : max size
	///      readsize : value of read size
	///      return status : 0 vector full , 1 end of file,   2 error of read  , 3 else error.
	int count = 0;
	int framesize = 0;
	int header4bytes = 4;
	(*readsize) = 0;
	int reste = 0, sizetoread = 0;
	static int localsizeread = 0;
	static int localsizevector = 2048;
	static char *localvector = NULL;
	char *vectornew = NULL;

	if (localvector == NULL) {
		localvector = new char[localsizevector];

	}

	while (true) {

		if (localsizeread > 0) {
			reste = vectorsize - (*readsize);
			if (localsizeread > reste)
				return 0;
			memcpy((void*) vector, (void*) localvector, localsizeread);
			*readsize += localsizeread;
			localsizeread = 0;
			reste = vectorsize - *readsize;
		}

		count = read(fLun, (void*) (localvector), header4bytes);

		if (count <= 0) {
			fError.TreatError(0, 0,
					"End of read file MFMCommonFrame::FillBigBufferFromFile");
			return 1;
		}
		if (count < header4bytes) {
			fError.TreatError(2, 0,
					"Error in read file MFMCommonFrame::FillBigBufferFromFile");
			return 2;
		}

		SetAttributsOn4Bytes(localvector);
		framesize = GetFrameSize();

		if (framesize > 1000000000) {
			fError.TreatError(2, framesize, "Crazy Frame size > 1000000000");
			return 3;
		}

		if (localsizevector < framesize) {
			vectornew = (char*) (realloc((void*) (localvector), framesize));
			if (vectornew != NULL) {
				localvector = vectornew;
				localsizevector = framesize;
				SetAttributsOn4Bytes(vector);
			} else {
				fError.TreatError(1, 0,
						"Memory allocation in MFMCommonFrame::FillBigBufferFromFile");
				return 3;
			}
		}

		sizetoread = framesize - header4bytes;
		count = read(fLun, (void*) ((localvector) + header4bytes), sizetoread);
		if (count != sizetoread) {
			fError.TreatError(2, 0,
					"Error in read file MFMCommonFrame::FillBigBufferFromFile");
			return 2;
		}
		localsizeread = framesize;
		(*eventcount)++;
	}
	return 0;
}

//_______________________________________________________________________________
int MFMCommonFrame::ReadInFile(int *lun, char **vector, int *vectorsize) {

	/// Get data from a file, and fill current frame and initialize its attributs and its pointer
	/// if size of actual frame is not enough, a new size is reallocated
	///      lun    : descriptor of file (given by a previous open)
	///      vector : pointer on pointer will contain frame . if size isn't big, a new value of pointer
	///      vectorsize of this pointer
	///      return size of read frame.

	static UtilVector_c utilvector(MFM_BLOB_HEADER_SIZE);
	utilvector.SetExternalPointers(vector, vectorsize);
	int size = ReadInFile(lun, &utilvector);
	return (size);
}
//_______________________________________________________________________________
int MFMCommonFrame::ReadInFile(int *lun) {

	/// Get data from a file, and fill current frame and initialize its attributs and its pointer
	/// if size of actual frame is not enough, a new size is reallocated
	///      lun    : descriptor of file (given by a previous open)
	///      vector : pointer on pointer will contain frame . if size isn't big, a new value of pointer
	///      vectorsize of this pointer
	///      return size of read frame.

	static UtilVector_c utilvector(MFM_BLOB_HEADER_SIZE);
	int size = ReadInFile(lun, &utilvector);
	return (size);
}
//_______________________________________________________________________________
int MFMCommonFrame::ReadInFile(int *lun, UtilVector_c *utilvector) {

	/// Get data from a file, and fill current frame and initialize its attributs and its pointer
	/// if size of actual frame is not enough, a new size is reallocated
	///      lun    : descriptor of file (given by a previous open)
	///      utilvector : vector will contain frame . if size isn't big enough, a resize is done
	///      return size of read frame.
	int count = 0;
	int framesize = 0;
	static long long int sumread = 0;
	int sizetoread = MFM_BLOB_HEADER_SIZE; // =8	

	count = read(*lun, (void*) (utilvector->GetPointer()), sizetoread);
	if (count <= 0) {
		cout << endl << " ** End of read file **\n";
		return count;
	}
	if (count < sizetoread) {
		cout << " Error in read file\n";
		return count;
	}
	MFMCommonFrame::SetAttributs(utilvector->GetPointer());
	framesize = GetFrameSize();
	if (framesize > 1000000000)
		fError.TreatError(2, framesize, "Crazy Frame size > 1000000000");
	if (utilvector->GetSize() < framesize) {
		utilvector->ReSize(framesize);
		SetAttributs(utilvector->GetPointer());
	}

	sizetoread = framesize - sizetoread;
	count = read(*lun,
			(void*) ((utilvector->GetPointer()) + MFM_BLOB_HEADER_SIZE),
			sizetoread);
	if (count != sizetoread) {
		cout << " Error in read file\n";
	}
	sumread += framesize;
	//if (sumread> 3000000000)
	//cout << " debug read size "<<sumread<<" \n";
	utilvector->SetUsedSize(framesize);
	return framesize;
}

//_______________________________________________________________________________
int MFMCommonFrame::ReadInFileold(int *lun, char **vector, int *vectorsize) {
	/// Get data from a file, and fill current frame and initialize its attributs and its pointer
	/// if size of actual frame is not enough, a new size is reallocated
	///      lun    : descriptor of file (given by a previous open)
	///      vector : pointer on pointer will contain frame . if size isn't big, a new value of pointer
	///      vectorsize of this pointer
	///      return size of read frame.
	int count = 0;
	int framesize = 0;
	char *vectornew = NULL;
	static long long int sumread = 0;
	int sizetoread = MFM_BLOB_HEADER_SIZE; // =8	
	count = read(*lun, (void*) (*vector), sizetoread);
	if (count <= 0) {
		cout << endl << " ** End of read file **\n";
		return count;
	}
	if (count < sizetoread) {
		cout << " Error in read file\n";
		return count;
	}
	MFMCommonFrame::SetAttributs((*vector));
	framesize = GetFrameSize();
	if (framesize > 1000000000)
		fError.TreatError(2, framesize, "Crazy Frame size > 1000000000");
	if (*vectorsize < framesize) {
		vectornew = (char*) (realloc((void*) (*vector), framesize));
		if (vectornew != NULL) {
			(*vector) = vectornew;
			*vectorsize = framesize;
			SetAttributs((*vector));
		} else {
			fError.TreatError(1, 0,
					"Memory allocation in MFMCommonFrame:: ReadInFile");
			return 0;
		}
	}
	sizetoread = framesize - sizetoread;
	count = read(*lun, (void*) ((*vector) + MFM_BLOB_HEADER_SIZE), sizetoread);
	if (count != sizetoread) {
		cout << " Error in read file\n";
	}
	sumread += framesize;
	//if (sumread> 3000000000)
	//cout << " debug read size "<<sumread<<" \n";
	return framesize;
}
//_______________________________________________________________________________________________________________________
void MFMCommonFrame::ExtractInfoFrame(int verbose, int dumpsize, int noframe) {
// extract informations Frames  and print informations

	if ((verbose > 1)) {

		cout << endl << "--------------------- nb = " << dec << noframe
				<< "----------------------------------" << endl;
		HeaderDisplay();
		if (verbose > 3 and dumpsize > 0) {
			int framesize = GetFrameSize();
			int dump = dumpsize;
			if (framesize < dump)
				dump = framesize;
			DumpRaw(dump, 0);
		}
	}
	if (verbose >= 9)
		DumpData();
}
//_______________________________________________________________________________
int MFMCommonFrame::ReadInMem(char **pt) {
	/// Get data from a memory, and fill current frame and initialize its attributs and its pointer
	//  return size of read frame.
	int framesize = 0;
	SetAttributs((*pt));
	framesize = GetFrameSize();
	(*pt) = (*pt) + framesize;
	return (framesize);
}
//_______________________________________________________________________________
int MFMCommonFrame::WriteInMem(char **pt) {
	/// Get data from a memory, and fill current frame and initialize its attributs and its pointer
	//  return size of read frame.
	int framesize = GetFrameSize();
	memcpy(*pt, GetPointHeader(), framesize);
	(*pt) = (*pt) + framesize;
	return (framesize);
}
//_______________________________________________________________________________
uint64_t MFMCommonFrame::GetTimeStampUs(uint64_t diff) const {
	/// give time in useconde from last SetTimeDiffUs() or since diff if diff >=0
	/// if diff =0 => since 1970
	/// GetTimeStampUs() is used  for simulation
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t tstime;
	tstime = tv.tv_sec * (uint64_t) 1000000 + tv.tv_usec;
	if (diff == 0)
		tstime -= diff;
	else
		tstime -= fTimeDiff;
	return tstime;
}
//_______________________________________________________________________________
uint64_t MFMCommonFrame::GenerateATimeStamp() const {
	///
	///generate a time stamp with computer clock for simulation
	///
	uint64_t ts = 0;
	clock_t t = clock();
	if (t >= 0)
		ts = (uint64_t) t;
	return ts;
}
//_______________________________________________________________________________
uint64_t MFMCommonFrame::SetTimeDiffUs() {
	/// give time in useconde from 1970
	/// SetTimeDiffUs() is used for simulation
	struct timeval tv;
	gettimeofday(&tv, NULL);
	fTimeDiff = (tv.tv_sec * (uint64_t) 1000000 + tv.tv_usec);
	return fTimeDiff;
}
//_______________________________________________________________________________
int MFMCommonFrame::GetHeaderSize() const {
	/// Return header size without computing it.
	return fHeaderSize;
}
//_______________________________________________________________________________
int MFMCommonFrame::GetBufferSize() const {
	/// Return buffer size without computing it.
	return fBufferSize;
}
//_______________________________________________________________________________
int MFMCommonFrame::GetFrameSize() const {
	/// Return frame size without computing it.
	return fFrameSize;
}
//_______________________________________________________________________________
void* MFMCommonFrame::GetPointHeader() const {
	/// Return pointer of begining of frame.
	return pData;
}
//_______________________________________________________________________________
char* MFMCommonFrame::GetPointUserData() const {
	/// Get pointer after header of frame
	return pUserData_char;
}
//_______________________________________________________________________________
uint32_t MFMCommonFrame::GetEventNumber() const {
	/// Return value of EventNumber.
	/// This value is computer by daugther class 
	return fEventNumber;
}

//_______________________________________________________________________________
int MFMCommonFrame::GetShiftEN(int type) const {
	// return shift of EN in frame
	// if 0 => no EN

	int Shift = NUMEXO_EN_SHI; //default value
	if ((type == MFM_COBOT_FRAME_TYPE) or (type == MFM_OSCI_FRAME_TYPE)
			or (type == MFM_EBY_TS_FRAME_TYPE)
			or (type == MFM_MERGE_TS_FRAME_TYPE)
			or (type == MFM_XML_DATA_DESCRIPTION_FRAME_TYPE)
			or (type == MFM_XML_FILE_HEADER_FRAME_TYPE)) {
		Shift = NO_EN;
	} else if ((type == MFM_COBO_FRAME_TYPE)
			or (type == MFM_SCALER_DATA_FRAME_TYPE)
			or (type == MFM_COBOF_FRAME_TYPE)) {
		Shift = BAS_EN_SHI_INV;
	} else if ((type == MFM_EBY_EN_FRAME_TYPE)
			or (type == MFM_EBY_EN_TS_FRAME_TYPE)
			or (type == MFM_MERGE_EN_FRAME_TYPE)) {
		Shift = BAS_EN_SHI;
	} else if ((type == MFM_NEDA_FRAME_TYPE) or (type == MFM_SIRIUS_FRAME_TYPE)
			or (type == MFM_REA_TRACE_FRAME_TYPE)) {
		Shift = BAS_EN_SHI2;
	} else if ((type == MFM_MUTANT_FRAME_TYPE)
			or (type == MFM_MUTANT1_FRAME_TYPE)
			or (type == MFM_MUTANT2_FRAME_TYPE)
			or (type == MFM_MUTANT3_FRAME_TYPE)
			or (type == MFM_HELLO_FRAME_TYPE)
			or (type == MFM_RIBF_DATA_FRAME_TYPE)
			or (type == MFM_MESYTEC_FRAME_TYPE)
			or (type == MFM_FAZIA_DATA_FRAME_TYPE)
			or (type == MFM_FASTER_FRAME_TYPE)
			or (type == MFM_FASTERDTS_FRAME_TYPE)
			or (type == MFM_CHIMERA_DATA_FRAME_TYPE)) {
		Shift = NUMEXO_EN_SHI_INV;
	}

	return Shift;
}

//_______________________________________________________________________________
int MFMCommonFrame::GetShiftTS(int type) const {
// return shift of TS in frame
	// if 0 => no TS
	int Shift = NUMEXO_TS_SHI; //default value
	if ((type == MFM_COBOT_FRAME_TYPE) or (type == MFM_OSCI_FRAME_TYPE)
			or (type == MFM_EBY_EN_FRAME_TYPE)
			or (type == MFM_MERGE_EN_FRAME_TYPE)
			or (type == MFM_XML_DATA_DESCRIPTION_FRAME_TYPE)
			or (type == MFM_XML_FILE_HEADER_FRAME_TYPE))
		Shift = NO_TS;
	else if ((type == MFM_COBO_FRAME_TYPE) or (type == MFM_COBOF_FRAME_TYPE)
			or (type == MFM_EBY_TS_FRAME_TYPE)
			or (type == MFM_SCALER_DATA_FRAME_TYPE)
			or (type == MFM_MERGE_TS_FRAME_TYPE))
		Shift = BAS_TS_SHI_INV;
	else if ((type == MFM_EBY_EN_TS_FRAME_TYPE))
		Shift = BAS_TS_SHI;
	else if ((type == MFM_NEDA_FRAME_TYPE) or (type == MFM_SIRIUS_FRAME_TYPE)
			or (type == MFM_REA_TRACE_FRAME_TYPE))
		Shift = BAS_TS_SHI2;
	else if ((type == MFM_MUTANT_FRAME_TYPE) or (type == MFM_MUTANT1_FRAME_TYPE)
			or (type == MFM_MUTANT2_FRAME_TYPE)
			or (type == MFM_MUTANT3_FRAME_TYPE)
			or (type == MFM_HELLO_FRAME_TYPE)
			or (type == MFM_RIBF_DATA_FRAME_TYPE)
			or (type == MFM_MESYTEC_FRAME_TYPE)
			or (type == MFM_FAZIA_DATA_FRAME_TYPE)
			or (type == MFM_FASTER_FRAME_TYPE)
			or (type == MFM_FASTERDTS_FRAME_TYPE)
			or (type == MFM_CHIMERA_DATA_FRAME_TYPE)) {
		Shift = NUMEXO_TS_SHI_INV;
	}
	return Shift;
}

//_______________________________________________________________________________
int MFMCommonFrame::GetShiftLocation(int type) const {
// return shift of location ( board number and channel id)  in frame
	// if 0 => no location
	int Shift = NO_LO; //default value

	if ((type == MFM_EXO2_FRAME_TYPE) or (type == MFM_VAMOSIC_FRAME_TYPE)
			or (type == MFM_VAMOSPD_FRAME_TYPE)
			or (type == MFM_DIAMANT_FRAME_TYPE)
			or (type == MFM_S3_BAF2_FRAME_TYPE)
			or (type == MFM_S3_ALPHA_FRAME_TYPE)
			or (type == MFM_S3_RUTH_FRAME_TYPE)
			or (type == MFM_S3_EGUN_FRAME_TYPE)
			or (type == MFM_S3_SYNC_FRAME_TYPE)
			or (type == MFM_REA_GENE_FRAME_TYPE)
			or (type == MFM_VAMOSTAC_FRAME_TYPE)
			or (type == MFM_BOX_DIAG_FRAME_TYPE)
			or (type == MFM_NEDACOMP_FRAME_TYPE)
			or (type == MFM_S3_DEFLECTOR_FRAME_TYPE))
		Shift = NUMEXO_LO_SHI;
	else if ((type == MFM_COBO_FRAME_TYPE) or (type == MFM_COBOF_FRAME_TYPE)
			or (type == MFM_REA_TRACE_FRAME_TYPE))
		Shift = BAS_LO_SHI;
	else if ((type == MFM_NEDA_FRAME_TYPE) or (type == MFM_OSCI_FRAME_TYPE)
			or (type == MFM_SIRIUS_FRAME_TYPE)
			or (type == MFM_REA_TRACE_FRAME_TYPE))
		Shift = BAS_LO_SHI2;
	else if (type == MFM_FASTER_FRAME_TYPE)
		Shift =FAS_LO_SHI;
	return Shift;
}

//_______________________________________________________________________________
string MFMCommonFrame::WhichFrame(uint16_t type) const {

	string tempos = "UNKNOWN_TYPE";
	if (type == 0) {
		type = fFrameType;
	}
	
	
	if (type == MFM_COBO_FRAME_TYPE)
		tempos = "MFM_COBO_FRAME_TYPE";
	else if (type == MFM_COBOF_FRAME_TYPE)
		tempos = "MFM_COBOF_FRAME_TYPE";
	else if (type == MFM_COBOT_FRAME_TYPE)
		tempos = "MFM_COBOT_FRAME_TYPE";
	else if (type == MFM_EXO2_FRAME_TYPE)
		tempos = "MFM_EXO2_FRAME_TYPE";
	else if (type == MFM_OSCI_FRAME_TYPE)
		tempos = "MFM_OSCI_FRAME_TYPE";
	else if (type == MFM_NEDA_FRAME_TYPE)
		tempos = "MFM_NEDA_FRAME_TYPE";
	else if (type == MFM_NEDACOMP_FRAME_TYPE)
		tempos = "MFM_NEDACOMP_FRAME_TYPE";
	else if (type == MFM_EBY_EN_FRAME_TYPE)
		tempos = "MFM_EBY_EN_FRAME_TYPE";
	else if (type == MFM_EBY_TS_FRAME_TYPE)
		tempos = "MFM_EBY_TS_FRAME_TYPE";
	else if (type == MFM_EBY_EN_TS_FRAME_TYPE)
		tempos = "MFM_EBY_EN_TS_FRAME_TYPE";
	else if (type == MFM_SCALER_DATA_FRAME_TYPE)
		tempos = "MFM_SCALER_DATA_FRAME_TYPE";
	else if (type == MFM_MERGE_EN_FRAME_TYPE)
		tempos = "MFM_MERGE_EN_FRAME_TYPE";
	else if (type == MFM_MERGE_TS_FRAME_TYPE)
		tempos = "MFM_MERGE_TS_FRAME_TYPE";
	else if (type == MFM_XML_DATA_DESCRIPTION_FRAME_TYPE)
		tempos = "MFM_XML_DATA_DESCRIPTION_FRAME_TYPE";
	else if (type == MFM_RIBF_DATA_FRAME_TYPE)
		tempos = "MFM_RIBF_DATA_FRAME_TYPE";
	else if (type == MFM_MUTANT_FRAME_TYPE)
		tempos = "MFM_MUTANT_FRAME_TYPE";
	else if (type == MFM_CHIMERA_DATA_FRAME_TYPE)
		tempos = "MFM_CHIMERA_DATA_FRAME_TYPE";
	else if (type == MFM_HELLO_FRAME_TYPE)
		tempos = "MFM_HELLO_FRAME_TYPE";
	else if (type == MFM_XML_FILE_HEADER_FRAME_TYPE)
		tempos = "MFM_XML_FILE_HEADER_FRAME_TYPE";
	else if (type == MFM_VAMOSIC_FRAME_TYPE)
		tempos = "MFM_VAMOSIC_FRAME_TYPE";
	else if (type == MFM_VAMOSPD_FRAME_TYPE)
		tempos = "MFM_VAMOSPD_FRAME_TYPE";
	else if (type == MFM_DIAMANT_FRAME_TYPE)
		tempos = "MFM_DIAMANT_FRAME_TYPE";
	else if (type == MFM_S3_BAF2_FRAME_TYPE)
		tempos = "MFM_S3BaF2_FRAME_TYPE";
	else if (type == MFM_S3_ALPHA_FRAME_TYPE)
		tempos = "MFM_S3_ALPHA_FRAME_TYPE";
	else if (type == MFM_S3_RUTH_FRAME_TYPE)
		tempos = "MFM_S3_RUTH_FRAME_TYPE";
	else if (type == MFM_S3_EGUN_FRAME_TYPE)
		tempos = "MFM_S3_EGUN_FRAME_TYPE";
	else if (type == MFM_S3_SYNC_FRAME_TYPE)
		tempos = "MFM_S3_SYNC_FRAME_TYPE";
	else if (type == MFM_REA_GENE_FRAME_TYPE)
		tempos = "MFM_REA_GENE_FRAME_TYPE";
	else if (type == MFM_REA_TRACE_FRAME_TYPE)
		tempos = "MFM_REA_TRACE_FRAME_TYPE";
	else if (type == MFM_SIRIUS_FRAME_TYPE)
		tempos = "MFM_SIRIUS_FRAME_TYPE";
	else if (type == MFM_PARIS_FRAME_TYPE)
		tempos = "MFM_PARIS_FRAME_TYPE";
	else if (type == MFM_MESYTEC_FRAME_TYPE)
		tempos = "MFM_MESYTEC_FRAME_TYPE";
	else if (type == MFM_FAZIA_DATA_FRAME_TYPE)
		tempos = "MFM_FAZIA_DATA_FRAME_TYPE";
	else if (type == MFM_FASTER_FRAME_TYPE)
		tempos = "MFM_FASTER_FRAME_TYPE";
	else if (type == MFM_FASTERDTS_FRAME_TYPE)
		tempos = "MFM_FASTERDTS_FRAME_TYPE";
	return tempos;
}
//_______________________________________________________________________________
bool MFMCommonFrame::TestType(uint16_t type) const {

	if (type == 0) {
		type = fFrameType;
	}
	if (type == MFM_COBO_FRAME_TYPE)
		return true;
	else if (type == MFM_COBOF_FRAME_TYPE)
		return true;
	else if (type == MFM_COBOT_FRAME_TYPE)
		return true;
	else if (type == MFM_EXO2_FRAME_TYPE)
		return true;
	else if (type == MFM_OSCI_FRAME_TYPE)
		return true;
	else if (type == MFM_NEDA_FRAME_TYPE)
		return true;
	else if (type == MFM_NEDACOMP_FRAME_TYPE)
		return true;
	else if (type == MFM_EBY_EN_FRAME_TYPE)
		return true;
	else if (type == MFM_EBY_TS_FRAME_TYPE)
		return true;
	else if (type == MFM_EBY_EN_TS_FRAME_TYPE)
		return true;
	else if (type == MFM_SCALER_DATA_FRAME_TYPE)
		return true;
	else if (type == MFM_MERGE_EN_FRAME_TYPE)
		return true;
	else if (type == MFM_MERGE_TS_FRAME_TYPE)
		return true;
	else if (type == MFM_XML_DATA_DESCRIPTION_FRAME_TYPE)
		return true;
	else if (type == MFM_RIBF_DATA_FRAME_TYPE)
		return true;
	else if (type == MFM_MUTANT_FRAME_TYPE)
		return true;
	else if (type == MFM_CHIMERA_DATA_FRAME_TYPE)
		return true;
	else if (type == MFM_HELLO_FRAME_TYPE)
		return true;
	else if (type == MFM_XML_FILE_HEADER_FRAME_TYPE)
		return true;
	else if (type == MFM_VAMOSIC_FRAME_TYPE)
		return true;
	else if (type == MFM_VAMOSPD_FRAME_TYPE)
		return true;
	else if (type == MFM_DIAMANT_FRAME_TYPE)
		return true;
	else if (type == MFM_S3_BAF2_FRAME_TYPE)
		return true;
	else if (type == MFM_S3_ALPHA_FRAME_TYPE)
		return true;
	else if (type == MFM_S3_RUTH_FRAME_TYPE)
		return true;
	else if (type == MFM_S3_EGUN_FRAME_TYPE)
		return true;
	else if (type == MFM_S3_SYNC_FRAME_TYPE)
		return true;
	else if (type == MFM_REA_GENE_FRAME_TYPE)
		return true;
	else if (type == MFM_REA_TRACE_FRAME_TYPE)
		return true;
	else if (type == MFM_SIRIUS_FRAME_TYPE)
		return true;
	else if (type == MFM_PARIS_FRAME_TYPE)
		return true;
	else if (type == MFM_MESYTEC_FRAME_TYPE)
		return true;
	else if (type == MFM_FAZIA_DATA_FRAME_TYPE)
		return true;
	else if (type == MFM_FASTER_FRAME_TYPE)
		return true;	
	else if (type == MFM_FASTERDTS_FRAME_TYPE)
		return true;	
	
	return false;
}
//_______________________________________________________________________________
bool MFMCommonFrame::IsABlobType(int type) const {

	bool retour = false;
	int blobness = GetBlobNess();
	if (blobness > 0)
		retour = true;
	return retour;
}
//_______________________________________________________________________________
bool MFMCommonFrame::IsAEbyedat(int type) const {
	if (type == 0)
		type = fFrameType;
	if ((type == MFM_EBY_EN_FRAME_TYPE) || (type == MFM_EBY_TS_FRAME_TYPE)
			|| (type == MFM_EBY_EN_TS_FRAME_TYPE)) {
		return true;
	}
	return false;
}
//_______________________________________________________________________________
bool MFMCommonFrame::IsAScaler(int type) const {
	if (type == 0)
		type = fFrameType;
	if (type == MFM_SCALER_DATA_FRAME_TYPE) {
		return true;
	}
	return false;
}
//_______________________________________________________________________________
bool MFMCommonFrame::IsACobo(int type) const {
	if (type == 0)
		type = fFrameType;
	if ((type == MFM_COBO_FRAME_TYPE) || (type == MFM_COBOF_FRAME_TYPE)
			|| (type == MFM_COBOT_FRAME_TYPE)) {
		return true;
	}
	return false;
}
//_______________________________________________________________________________
bool MFMCommonFrame::IsAMutant(int type) const {
	if (type == 0)
		type = fFrameType;
	if ((type == MFM_MUTANT_FRAME_TYPE) || (type == MFM_MUTANT1_FRAME_TYPE)
			|| (type == MFM_MUTANT2_FRAME_TYPE)
			|| (type == MFM_MUTANT3_FRAME_TYPE)) {
		return true;
	}
	return false;
}
//_______________________________________________________________________________
void MFMCommonFrame::InitStat() {

	fCountFrame = 0;
	fMeanFrameSize = 0;
	fNegatifJump = 0;
	fNoContiJump = 0;
	fInitStatDone = true;
}
//_______________________________________________________________________________
void MFMCommonFrame::IncrementNegativJump() {
	fNegatifJump++;
}
//_______________________________________________________________________________
void MFMCommonFrame::IncrementNoContiJump() {
	fNoContiJump++;
}
//_______________________________________________________________________________
void MFMCommonFrame::FillStat() {
	if (fInitStatDone == false) {
		//cout << " Init of statistic was not done\n";
		return;
	}
	fCountFrame++;
	uint32_t framesize = GetFrameSize();
	fMeanFrameSize += framesize;
}
//_______________________________________________________________________________
string MFMCommonFrame::GetStat(string info) const {
	
	if (fInitStatDone == false)
		return (string) "";
	string display("");
	stringstream ss("");
	ss << "Number of " << info << " Frames : " << fCountFrame << endl;
	if (fCountFrame != 0)
		ss << "Mean Frame Size " << (double) (fMeanFrameSize / fCountFrame)
				<< endl;
	ss << "Negativ Jumps " << fNegatifJump << endl;
	ss << "No Contigus Jumps " << fNoContiJump << endl;
	display = ss.str();
	return display;
}
//_______________________________________________________________________________
void MFMCommonFrame::PrintStat(string info) const {
	if (fInitStatDone == false)
		return;
	cout << (GetStat(info));
}
//_______________________________________________________________________________
unsigned long long int MFMCommonFrame::GetCountFrame() const {
	return fCountFrame;
}

//_______________________________________________________________________________
void MFMCommonFrame::TestUserPointer(int noframe, int verbose) const {
	static long long int sumsize = 0;
	static long long int sumsizebefore = 0;
	sumsizebefore = sumsize;
	if ((long long int) GetPointHeader() + GetFrameSize()
			<= (long long int) GetPointUserData()) {
		if (verbose > 1)
			cout << " Frame with no place for data " << endl;
		return;
	}
	if (GetFrameType() == MFM_MERGE_EN_FRAME_TYPE) {
		sumsize += 20;
	}	// header size
	else if (GetFrameType() == MFM_MERGE_TS_FRAME_TYPE) {
		sumsize += 26;
	} // header size
	else {
		sumsize += GetFrameSize();
	}

	if (GetPointUserData() != pData_char + GetDefinedHeaderSize()) {
		char tempos[64];
		sprintf(tempos, "On frame no = %d , sum size = %lld", noframe,
				sumsizebefore);

		fError.TreatError(0, 2, " GetPointHeader() no good!", tempos);
		printf(
				"    If is a file, try a \"od -tx1 -Ad -w16 -v -j %lld filename.dat | less\" )\n",
				sumsizebefore - 32);
		debug_frame();
	}

}
//_______________________________________________________________________________
void MFMCommonFrame::CopyBufferAndResizeFrameIfNecessary(
		UtilVector_c *Vector_c) {
// resize Frame if necessary and copy vector in frame supposed to contain a MFM frame 
	if (Vector_c->GetUsedSize() > GetBufferSize()) {
		SetBufferSize(Vector_c->GetUsedSize());
	}
	memcpy(GetDataPointer(), Vector_c->GetPointer(), Vector_c->GetUsedSize());
	SetAttributs();
}

//_______________________________________________________________________________
void MFMCommonFrame::debug_frame() const {
    int count =0;
	cout << "Frame begins at " << pData << " (also beginning of header "
			<< pHeader << ")" << endl;
	cout << "   -- frame size  = " << GetFrameSize() << " bytes" << endl;
	cout << "   -- header size = " << GetHeaderSize() << " bytes" << endl;
	cout << "   -- user data at " << (void*) GetPointUserData();
	cout << "   -- (diff = ) "
			<< (void*) ((long long*) GetPointUserData()
					- (long long*) GetPointHeader()) << endl;

	cout << "Memory layout:" << endl;
	unsigned char *p = (unsigned char*) GetPointHeader();
	unsigned char *end_frame = p + GetFrameSize();
	while (p < end_frame) {
		cout << count ++ <<"  "<<(void*) p << " : " << hex << (unsigned int) (*p) << dec;
		if (p == (unsigned char*) GetPointUserData()) {
			cout << " <== first user data"<<endl;
			cout << "....................";
			if (end_frame - p > 10)
				p = end_frame - 10;
		}
		cout << dec << endl;
		++p;
	}	cout << " last user data"<<endl;
}
//_______________________________________________________________________________
bool MFMCommonFrame::RawEqual(MFMCommonFrame *testframe) const {
// test if testframe and this:frame is equal
	bool test = true;
	int i = 0;
	char *pttest;
	char *thispttest;
	testframe->SetAttributs();
	int size = testframe->GetFrameSize();
	thispttest = (char*) (GetPointHeader());
	pttest = (char*) (testframe->GetPointHeader());
	for (i = 0; i < size; i++) {
		test = test and ((char) (pttest[i]) == (char) ((thispttest[i])));
		if (not test)
			break;
	}

	return test;
}
//_______________________________________________________________________________
bool MFMCommonFrame::IsSame(MFMCommonFrame *testframe, int verbose) {
// test if testframe have same attibuts

	string display("");
	stringstream ss("");
	bool test = true;
	bool testloc = true;
	int i = 0;

	ss << "   Test for " << MyClassName() << " :";
	testframe->MFMCommonFrame::SetAttributs();
	MFMCommonFrame::SetAttributs();

	testloc = (GetFrameSize() == testframe->GetFrameSize());
	ss << testloc;
	test = test and testloc;
	testloc = (GetHeaderSize() == testframe->GetHeaderSize());
	ss << testloc;
	test = test and testloc;
	testloc = (GetBlobNess() == testframe->GetBlobNess());
	ss << testloc;
	test = test and testloc;
	testloc = (GetMetaType() == testframe->GetMetaType());
	ss << testloc;
	test = test and testloc;
	testloc = (GetFrameEndianness() == testframe->GetFrameEndianness());
	ss << testloc;
	test = test and testloc;
	testloc = (GetDataSource() == testframe->GetDataSource());
	ss << testloc;
	test = test and testloc;
	testloc = (GetUnitBlockSize() == testframe->GetUnitBlockSize());
	ss << testloc;
	test = test and testloc;
	testloc = (GetRevision() == testframe->GetRevision());
	ss << testloc;
	test = test and testloc;
	testloc = (GetEventNumber()
			== testframe->GetEventNumber());
	ss << testloc;
	test = test and testloc;
	testloc = (GetTimeStamp()
			== testframe->GetTimeStamp());
	ss << testloc;
	test = test and testloc;
	display = ss.str();
	if ((verbose > 0))
		cout << display << "  (Si Hs Bl Me En So Un Re En Ts)" << endl;
	return test;

}

//_______________________________________________________________________________
int MFMCommonFrame::TestFramesInBufferv(const UtilVector_c *vector) {
	return TestFramesInBuffer((char*) (vector->GetPointer()),
			(int) (vector->GetUsedSize()));
}
//_______________________________________________________________________________
int MFMCommonFrame::TestFramesInBuffer(char *ptvector, int usedsize) {
// test if the buffer contains good frames 
// return nb of detected errors 
	int maxsize = 2500000;
	char *pt = ptvector;
	int framesize = 0;
	int readsize = 0;
	int nberror = 0;
	int type = 0;
	long long int count = 0;
	if (usedsize > 0) {
		while (true) {
			framesize = ReadInMem(&pt);
			SetAttributs();
			type = GetFrameType();
			if (!TestType())
				nberror++;
			if ((framesize > maxsize)or (framesize < MFM_BLOB_HEADER_SIZE))
				nberror++;
			readsize += framesize;
			//cout << "Scan vector "<<readsize <<"/"<<usedsize<< "  Type ="<< type  <<endl;
			if (usedsize <= readsize)
				break;
		}
	}
	return nberror;
}
//_______________________________________________________________________________

