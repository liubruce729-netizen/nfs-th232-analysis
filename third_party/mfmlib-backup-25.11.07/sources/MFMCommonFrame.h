///
/// MFM library manage event MFM format
///
/// MFMCommonFrame Class
/// \file MFMCommonFrame.h
/// \author Legeard Luc
/// \version 1.0
/// \date 2014 01 01
///


// META-TYPE  Byte
// [7-Endian][6-Blobness][5][4][3-2-1-0-BlockSize]

#ifndef _MFM_FRAME_COMMON_
#define _MFM_FRAME_COMMON_

#include <string.h>
#include <stdint.h>
#include "MFMTypes.h"
#include "MError.h"
#include "stdlib.h"
#include "MFMFewDefines.h"
#include "CUtilities.h"
#include <typeinfo>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <byteswap.h>

#pragma pack(push, 1)// allow strict alignment
#define MFM_ENDIANNESS_MSK 0x80
#define MFM_BLOBNESS_MSK   0x40
#define MFM_UNIT_BLOCK_SIZE_MSK 0x0F
#define MFM_UNIT_BLOCK_DEFAULT_SIZE 1
#define MFM_BLOCK_SIZE     0x0C
#define MFM_BIG_ENDIAN     0
#define MFM_LITTLE_ENDIAN  0x80
#define MFM_FRAME_SIZE_MASK 0x00FFFFFF
#define MFM_BLOB_HEADER_SIZE 8
#define set_user_data_pointer(A,B) pUserData_char = (char*) &(((A*)pHeader)->B)
#define MFM_COMMON_TYPE_TXT "MFM_UNKNONW_COMMON_FRAME" 
#define MFM_COMMON_TYPE_SHORT_TXT "UnKno" 
#define MFM_COMMON_TYPE_LONG_TXT "Unknown Frame, so considerated as a Common Frame" 
#define MAXKOWNFRAMESIZE 557184 +128 // size of cobo + 128 of secure. Be carefule , xml frame can be bigger but generaly in from file.

#define MAXINDICETYPE 65536

#define NUMEXO_CRYS_MASK 0x001f
#define NUMEXO_CHANNEL_ID_MASK NUMEXO_CRYS_MASK
#define NUMEXO_BOARD_ID_MASK 0x07ff
#define NUMEXO_SLIP_BITS     5


enum MFM_ERR { MFM_ERR_UNIT_BLOCK_SIZE = -100, MFM_ERR_FRAME_SIZE };

struct MFM_common_header {
  unsigned metaType : 8;
  unsigned frameSize : 24;
  unsigned dataSource : 8;
  unsigned frameType : 16;
  unsigned revision : 8;
};

struct MFM_ext_layered_header {
  unsigned headerSize : 16;
  unsigned itemSize : 16;
  unsigned nItems : 32;
};

struct MFM_ext_basic_header {
  unsigned headerSize : 16;
  unsigned itemSize : 16;
  unsigned nItems : 32;
};

struct MFM_ext_blob_header {
};
struct MFM_ext_common_header {
};

struct MFM_layered_header {
  MFM_common_header hd;
  MFM_ext_layered_header ext;
};

struct MFM_topcommon_header {
  MFM_common_header hd;
  MFM_ext_common_header ext;
};
struct MFM_basic_header {
  MFM_common_header hd;
  MFM_ext_basic_header ext;
};

struct MFM_blob_header {
  MFM_common_header hd;
  MFM_ext_layered_header ext;
};

//____________MFMCommonFrame___________________________________________________________

class MFMCommonFrame
{
protected:

uint32_t fHeaderSize;   		///< Header size in Bytes
void *   pReserveHeader; 		///< Pointer on frame reserve header
char *   pDataNew;  			///< Pointer of beginning frame  if local allocation.
char *   pData_char; 			///< Pointer of beginning frame
char *   pUserData_char; 		///< Pointer of User Data ( after header and timestamp,enventnumber..)
int      fSizeOfUnitBlock; 		///< Size of Unit block in Bytes
bool     fLocalIsBigEndian; 	///< Endianness or running computer  
bool     fFrameIsBigEndian ; 	///< Endianness or Frame
uint16_t fFrameType; 			///< Frame type
int      fWantedFrameType; 		///< Wanted Frame type in case of generation of frame ( simulation ). No usage in read frame
int      fFrameSize; 			///< Frame size
uint64_t fTimeStamp;			///< Time Stamp
uint32_t fEventNumber;			///< EventNumber
int16_t  fBoardId;				///< Board id or Cobo card if exists, if not = -1
int16_t  fChannelId;			///< Channel id or Asad ,  if exists, if not = -1
bool     fInitStatDone;         ///< tag for init of statistics


private :
mutable int    fIncrement;    			///< memory of incrementation in case of dump.
int      fBufferSize;   			///< available buffer size, it can be higher than frame size.
uint64_t fTimeDiff;  		///< memorise time reference in microsecond
protected :
char *   pCurrentPointerForAdd; ///< Pointer  to keep memory where we can add data or other frames in current frame
char *   pCurrentPointerForRead; ///<Pointer  to keep memory where we can read data or other frames in current frame
unsigned long long   fCountFrame        ; /// counter to do statistic ( in case of same frame object is reused to read a file for example)
double fMeanFrameSize ; /// statistic : MeanFrameSize
unsigned long long fNegatifJump   ; /// counter negativ jump in event number
unsigned long long fNoContiJump   ; //  counter of non contigus +1 jump in event number

MError   fError;
//public :
void * pData;     				///< Pointer of beginning frame
MFM_topcommon_header * pHeader; ///<  Pointer on frame header

public :
 static string indentation;

MFMCommonFrame();
MFMCommonFrame(int unitBlock_size, int dataSource,
	 		 int frameType, int revision, int frameSize);
MFMCommonFrame(MFMCommonFrame* frame);
 virtual ~MFMCommonFrame();
 char* MyClassName()const{ return (char*)"MFMCommonFrame";};
 virtual void Init();
 virtual string WhichFrame(uint16_t type =0) const;
 void SetMetaType(int unitBlock_size,bool isablob=false);
 void SetUnitBlockSize(int size);
 void SetDataSource(uint8_t datasource);
 void SetRevision(uint8_t  revision);
 void SetFrameSize(uint32_t  size); 
 void SetFrameType(uint16_t frametype);
 void SetHeader(MFM_topcommon_header* header);
 void SetBufferSize(int size,bool ifinferior=true);
 void SetPointers(void * pt =NULL);
 virtual void SetAttributs(void * pt =NULL);
 virtual void SetAttributsOn4Bytes(void * pt =NULL);
 virtual void SetAttributsOn8Bytes(void * pt =NULL);
 

 virtual unsigned char Endianness(void)const;
 bool isBigEndian()const;
 bool is_power_2(int i)const;
 int GetBlobNess()const;
 uint8_t GetMetaType()const;
 
 
 virtual const char * GetTypeText()const    {return MFM_COMMON_TYPE_TXT;} 
 virtual const char * GetTypeShortText()const {return MFM_COMMON_TYPE_SHORT_TXT;}
 virtual const char * GetTypeLongText()const {return MFM_COMMON_TYPE_LONG_TXT;}
 virtual int GetDefinedUnitBlockSize()const {return MFM_UNIT_BLOCK_DEFAULT_SIZE;};
 virtual int GetDefinedHeaderSize()const    {return MFM_BLOB_HEADER_SIZE;};
 virtual int GetDefinedFrameSize()const     {return MFM_BLOB_HEADER_SIZE;};
 
 void * GetDataPointer()const { return pData;};

 unsigned char GetFrameEndianness(void)const;
 uint16_t  GetDataSource()const; 
 uint16_t  GetFrameType()const;
 uint8_t  GetRevision()const;
 bool TestType(uint16_t type=0) const;
 int  TestFramesInBuffer(char* ptvector , int usedsize);
 int  TestFramesInBufferv(const UtilVector_c* vector);
 int  GetHeaderSize()const;
 int  GetBufferSize()const;
 int  GetFrameSize()const;
 int  GetUnitBlockSize() const {return fSizeOfUnitBlock;}
 void SetFrameTypeFromFrameData();
 void SetFrameSizeFromFrameData();
 void SetTimeStampFromFrameData();
 void SetEventNumberFromFrameData();
 void SetBoardAndChannelFromFrame();
 //uint32_t GetEventNumberFromCommonFrameData()const;
 //uint64_t GetTimeStampFromCommonFrameData()const;
 
 virtual void  SetUnitBlockSizeFromFrameData();
 virtual void  SetHeaderSizeFromFrameData(){};
 virtual void SetUserDataPointer();
 virtual void *GetPointHeader()const;
 virtual char *GetPointUserData()const;
 virtual uint32_t GetEventNumber()const;
 virtual uint64_t GetTimeStamp()const;


 virtual void SetTimeStamp(uint64_t t) { fTimeStamp=t; }
 virtual void SetEventNumber(uint32_t e) { fEventNumber=e; }
 virtual void DumpRaw(int dumpsize=0, int increment=256) const;
 virtual string GetDumpRaw(int dumpsize=0, int increment=256)const;
 virtual void   GetDumpRaw(void *point,int dumpsize, int increment,string * mydump = NULL)const;
 void  DumpData(char mode='d', bool nozero=false) const {cout << GetDumpData( mode,  nozero);};
 virtual string GetDumpData(char mode='d', bool nozero=false) const {string test ="";return test;}; ///  Dump decoded data in if frame class avec this method
 virtual string GetHeaderDisplay(char* infotext=NULL)const;
 void   HeaderDisplay(char* infotext=NULL) const ;
 virtual bool   RawEqual(MFMCommonFrame* testframe)const;
 virtual bool   IsSame(MFMCommonFrame* testframe,int verbose); 
 
 virtual void   MFM_make_header(int unitBlock_size, int dataSource,
			    int frameType, int revision, int frameSize,bool blob=false);
 virtual void   MFM_fill_header( int unitBlock_size,
				   int dataSource, int frameType, int revision,
				   int frameSize,bool blob=false);
virtual void FillDataWithRamdomValue(uint64_t timestamp,uint32_t eventnumber,int  nbitem){fError.TreatError(2,0,"MFMCommonFrame:FillDataWithRamdomValue should never be here----------\n");};
virtual void FillDataWithRamdomValue(uint64_t timestamp,uint32_t eventnumber){fError.TreatError(2,0,"MFMCommonFrame:FillDataWithRamdomValue should never be here----------\n");}

virtual void GenerateAFrameExample(uint64_t timestamp,uint32_t eventnumber);
virtual void WriteRandomFrame(int lun,int  nbframe,int verbose,int dumsize,int type);

 int  ReadInFile(int *lun, char** vector, int * vectosize);
 int  ReadInFileold(int *lun, char** vector, int * vectosize);
 int  ReadInFile(int *lun, UtilVector_c* vector);
 int  ReadInFile(int *lun);
 int  FillBigBufferFromFile(int fLun,char* vector, unsigned int  vectosize,unsigned int *readsize,unsigned int *eventcount);
 int  ReadInMem(char** vector);
 int  WriteInMem(char **vector);
virtual void ExtractInfoFrame(int verbose,int dumpsize,int noframe);
virtual void ReadAttributsExtractFrame(int verbose,int dumpsize,bool display,int noframe,void * pt=NULL);
 uint64_t GetTimeStampUs(uint64_t diff= 0)const;
 uint64_t SetTimeDiffUs();
 uint64_t GenerateATimeStamp()const;
 bool IsAEbyedat(int type =0)const;
 bool IsACobo(int type =0)const;
 bool IsAMutant(int type =0)const;
 bool IsAScaler(int type=0)const;
 bool IsABlobType(int type =0)const;
virtual void FillStat();
virtual void InitStat();
 void IncrementNegativJump();
 void IncrementNoContiJump();
virtual string GetStat(string info = "UnknowFrame" ) const;
virtual void PrintStat(string ="UnknowFrame" )const;
virtual bool HasTimeStamp() const {return true; }
virtual bool HasEventNumber() const {return true; } 
virtual bool HasBoardId() const {return false; }
virtual uint16_t GetBoardId() const { return fBoardId; }
virtual uint16_t GetChannelId() const { return fChannelId; }

int GetShiftEN(int type)const;
int GetShiftTS(int type)const;
int GetShiftLocation(int type)const;


//uint32_t GetEventNumberFromCommonFrameData()const;
//uint64_t GetTimeStampFromCommonFrameData()const;
/*
uint16_t GetCristalIdFromCommonFrameData() const;
uint16_t GetTGCristalIdFromCommonFrameData() const;
uint16_tGetChannelIdFromCommonFrameData() const;
uint16_t MFMNumExoFrame::GetBoardIdFromCommonFrameData() const ;
*/


void SetWantedFrameType(uint16_t type){ fWantedFrameType = type;}
int  GetWantedFrameType() const { return fWantedFrameType;}
unsigned long long int GetCountFrame() const;
virtual void  TestUserPointer(int noframe,int  verbose) const;

virtual void Print(int /*verbose*/=0, int /*dumpsize*/=0) const {
        HeaderDisplay();
        DumpRaw();
        cout << endl;
    }

 bool IsType(uint16_t t) const{
  return fFrameType==t;
}
void CopyBufferAndResizeFrameIfNecessary(UtilVector_c* Vector_c);
void CopyFrameAndResizeFrameIfNecessary(MFMCommonFrame* frame);
virtual void debug_frame() const;
virtual void FrameSpecificInitialisations(const string&) {}

};


#pragma pack(pop) // free aligment



//____________________________________________________________________
// Utility functions
/*
//____________________________________________________________________

This methode does not work in UBUNTU 20 
We suppose is du to  Mot16 = (SW_MOT16 *) Buf;
 "=" is not supported by struct SW_MOT16

inline void SwapInt16(uint16_t *Buf, int repeat=1) {
    /// Swap a 16 bits(2 Bytes) integer to do endianess conversion
    ///     Buf   : pointer on integer to convert
    ///     repeat: nb of repeat in case of a vector to convert. default =1
    typedef struct mot16 {
        unsigned char Byte1;
        unsigned char Byte2;
    } SW_MOT16;
    SW_MOT16 Temp, *Mot16;

    Mot16 = (SW_MOT16 *) Buf;

    for (int i = 0; i < repeat; i++, Mot16++) {
        Temp.Byte1 = Mot16->Byte2;
        Temp.Byte2 = Mot16->Byte1;
        *Mot16 = Temp;
    }
}

//____________________________________________________________________

Etonnament si dans ce code suivant si on commente la ligne 
cout << endl<<"--> Input "<<(uint16_t)(*Buf)<< "  pt buff = "<< ( long long *) Buf <<" pt Mot16 = "<<(long long*)Mot16<< endl;
ca ne marche plus
//____________________________________________________________________
inline void SwapInt16(uint16_t *Buf, int repeat=1) {
   /// Swap a 16 bits(2 Bytes) integer to do endianess conversion
   ///     Buf   : pointer on integer to convert
   ///     repeat: nb of repeat in case of a vector to convert. default =1

typedef struct mot16 {
       unsigned char Byte1;
       unsigned char Byte2;
   } SW_MOT16;


   SW_MOT16 Temp, *Mot16;
   Mot16 = (SW_MOT16 *) Buf;

   cout << endl<<"--> Input "<<(uint16_t)(*Buf)<< "  pt buff = "<< ( long long *) Buf <<" pt Mot16 = "<<(long long*)Mot16<< endl;

   for (int i = 0; i < repeat; i++, Mot16++) {
       Temp.Byte1 = Mot16->Byte2;
       Temp.Byte2 = Mot16->Byte1;

       cout <<" in1   "<<(int) ( Mot16->Byte1)<< "  in2    " <<(int)Mot16->Byte2<<endl;
       cout <<" temp1 "<< (int) Temp.Byte1    << "  temp2  " <<(int)Temp.Byte2<<endl;
       // *Mot16 = Temp;

       Mot16->Byte1 = Temp.Byte1;
       Mot16->Byte2 = Temp.Byte2;

       //cout << endl<<"--> Input "<<(uint16_t)(*Buf)<< "  pt buff = "<< ( long long *) Buf <<" pt Mot16 = "<<(long long*)Mot16<< endl;
       cout <<endl<<" in1  "<<(int) ( Mot16->Byte1)<< "  in2 " <<(int)Mot16->Byte2<<endl;
       cout << "final "<<(uint16_t)(*Buf)<<"  temp : " << *((uint16_t*)(&Temp)) <<endl;
   }
}
*/


//____________________________________________________________________

inline void SwapInt16(uint16_t* Buf)
{
   /// Swap a 16 bits(2 Bytes) integer to do endianess conversion
   ///     Buf   : pointer on integer to convert
   ///     repeat: nb of repeat in case of a vector to convert. default =1

    *Buf = bswap_16(*Buf);
}


//____________________________________________________________________
inline void SwapInt64(uint64_t* Buf, int nbByte = 8)
{
   /// Swap a 64 bits(8 Bytes) integer to do endianess conversion \n
   ///     Buf   : pointer on integer to convert\n
   ///     nByte : number of bytes concerned by swaping (default = 8)\n

    if(nbByte<8)
       *Buf = bswap_64(*Buf) >> (8-nbByte)*8;
    else
       *Buf = bswap_64(*Buf);
}


//_______________________________________________________________________________
inline uint64_t GenerateATimeStamp() {
    ///
    ///generate a time stamp with computer clock for simulation
    ///
    uint64_t ts= 0 ;
    clock_t t = clock();
    if (t>=0 )
        ts=(uint64_t)t;
    return ts;
}

//____________________________________________________________________
inline void SwapInt32(uint32_t* Buf, int nbByte = 4)
{
   /// Swap a 32 bits(4 Bytes) integer to do endianess conversion\n
   ///     Buf   : pointer on integer to convert\n
   ///     nByte : number of bytes concerned by swaping (default =4)\n

    if(nbByte<4)
       *Buf = bswap_32(*Buf) >> (4-nbByte)*8;
    else
       *Buf = bswap_32(*Buf);

}


//____________________________________________________________________
#endif
