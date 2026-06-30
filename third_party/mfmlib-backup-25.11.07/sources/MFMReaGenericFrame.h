#ifndef _MFMReaGenericFrame_
#define _MFMReaGenericFrame_
/*
  MFMReaGenericFrame.h

	Copyright Acquisition group, GANIL Caen, France

*/

#include "MFMNumExoFrame.h"

#define REA_GENERIC_NB_STATUS 2
#define MFM_REA_GENE_TYPE_TXT "MFM_REA_GENE_FRAME_TYPE"
#define MFM_REA_GENE_TYPE_SHORT_TXT "ReaGe"
#define MFM_REA_GENE_TYPE_LONG_TXT "Rea Generic"

#pragma pack(push, 1) // force alignment

struct MFM_ReaGeneric_data{
  unsigned CristalId : 16;
  unsigned Status1   : 16;
  unsigned Status2   : 16;
  unsigned Type_Tns  : 16;
  unsigned Energy    : 16;
  unsigned Time      : 16;
  unsigned Checksum  : 16;
};

struct MFM_ReaGeneric_frame{
	 MFM_common_header         Header;
	 MFM_numexo_eventInfo      EventInfo;
	 MFM_ReaGeneric_data       Data;
};
enum ReaTnsType {REA_GENERIC_ENERGY_TYPE =0x00,REA_GENERIC_ENERGY_TIME_TYPE=0x01,REA_GENERIC_TIME_TYPE=0x02,REA_GENERIC_CHARGE_TYPE=0x04};

//____________MFMReaGenericFrame___________________________________________________________

class MFMReaGenericFrame : public MFMNumExoFrame
{
 private:
  // attributes to do statistic on channel to know if there are " something inside"
  long long fSumTime[NUMEXO_NB_CHANNELS];
  long long fSumEnergy[NUMEXO_NB_CHANNELS];
  long long fSumStatus[NUMEXO_NB_CHANNELS][2];
  long long fSumTypeTNS[NUMEXO_NB_CHANNELS];
  long long fSumTimeCount[NUMEXO_NB_CHANNELS];
  long long fSumEnergyCount[NUMEXO_NB_CHANNELS];
  long long fSumStatusCount[NUMEXO_NB_CHANNELS][2];
  long long fSumTypeTNSCount[NUMEXO_NB_CHANNELS];

 public :
 
 // attibutes to creat fix memory values for root Tree convertion
 uint16_t  **fStatus1, **fStatus2, **fTypeTNS,**fEnergy,**fTime;
 uint32_t fTabValueEventNumber;
 uint64_t fTabValueTimeStamp;
 bool fInitTabValuesDone ; // init  of Tab/Values   for root Tree convertion
 int fNumberOfBoards; //sum of board
 int fBoardNumber;    //current number of board
 int fBoardIndex;     //current index  of board
 int fChannel;        //current number of channel
 // attributes to do convertion between index and number of board
 int fConvertNoBoardIndex[NUMEXO_MAX_NUMB_BOARDS];  
 int fConvertIndexNoBoard[NUMEXO_MAX_NUMB_BOARDS];      


 MFMReaGenericFrame();
 ~MFMReaGenericFrame();
 char*     MyClassName()const{ return (char*)"MFMReaGenericFrame";};
 void      SetStatus(int i, uint16_t status);
 uint16_t  GetStatus(int i)const;
 void      SetTypeTns(enum ReaTnsType type);
 enum ReaTnsType  GetTypeTns()const;
 void      SetEnergy(uint16_t energie);
 uint16_t  GetEnergy()const;
 void      SetTime(uint16_t time);
 uint16_t  GetTime()const;
 void      FillDataWithRamdomValue(uint64_t timestamp,uint32_t enventnumber);
 const char * GetTypeText()const {return MFM_REA_GENE_TYPE_TXT;} 
 const char * GetTypeShortText()const {return MFM_REA_GENE_TYPE_SHORT_TXT;}
 const char * GetTypeLongText()const {return MFM_REA_GENE_TYPE_LONG_TXT;}
 string GetStat(string info)const ;
  void  InitStat() ;
  void  FillStat();
  void  InitTabValues(uint16_t *listofboard,int numberofboard);
  string GetDumpData(char mode, bool nozero) const;
  void  SetTabValues();
  void  ResetTabValues();
  bool IsSame(MFMReaGenericFrame* testframe,int verbose);
  bool UnitTest(int verbose);
};
#pragma pack(pop) // free aligment
#endif
