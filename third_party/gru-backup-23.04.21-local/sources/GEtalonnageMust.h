// File :  GEtalonnageMust.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GEtalonnageMust
//
// This class do specific analisys using calibration tools for Muvi  cards
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

// definition oof constant muvi cards


#ifndef __GEtalonnageMust__
#define __GEtalonnageMust__

#include <sstream>
using std::ostream;

#include <TObject.h>
#include "General.h"
#include "GAcq.h"
#include "GParaCaliXml.h"
// definition of card constant
#include "GMuvi.h"


//_________________________________________________________________________________________


class GEtalonnageMust : public GAcq{

 private:

  // Muvi
  int  fGeneValues[NB_TELESCOPES*ENERGIE_AND_TIME];    // for each  telescop we have 2 diff�rents values of Gene
  int  fNoChannels[NB_MATES*NB_TELESCOPES]; // no of selected channel for each run ( only one channel can be selected /mate)
  int  fcountevent;
  Float_t fMemoGeneValues[NB_TELESCOPES*ENERGIE_AND_TIME][NB_MAX_OF_PICS];


  Int_t  fnRun; // nb of run in case of multirun ( rampe)
  int    fModeCalib;// 1 =test   2= control    3= rampe  4=rampe p�rodkique
  char   *fNameFirstParameter;//!

  int fIncrement1;
  int fIncrement2;
  int fIndex2DE[NB_TELESCOPES];// user parameter , to memorize index of 2D histogram
  int fIndex2DT[NB_TELESCOPES];// user parameter , to memorize index of 2D histogram

 TH1S *fStartedHisto;

 int fTelein2D; // no of telescope we want to see in 2b histogram (0->4)

 int fIndexStartedHisto;
 
 TH2S *fH2DE[NB_TELESCOPES];
 TH2S *fH2DT[NB_TELESCOPES];
 UShort_t      *fMM_X_E[NB_TELESCOPES];//!
 UShort_t      *fMM_X_T[NB_TELESCOPES];//!
 UShort_t      *fMM_Y_E[NB_TELESCOPES];//!
 UShort_t      *fMM_Y_T[NB_TELESCOPES];//!
 UShort_t      *fMM_Sili_E[NB_TELESCOPES];//!
 UShort_t      *fMM_Sili_T[NB_TELESCOPES];//!
 UShort_t      *fMM_Csi_E[NB_TELESCOPES];//!
 UShort_t      *fMM_Csi_T[NB_TELESCOPES];//!

TString fMuviEnergy2D, fMuviTime2D ;


 protected:

  int fStatus; // Internal status ( to have a satus whithout a returned status which noise the standard output of Cint )
  int fFirstIndex; //  first raw Muvi parameter
  GParaCaliXml  fXmlFile;


 public:

  GEtalonnageMust(GDevice* _fDevIn= NULL, GDevice* _fDevOut= NULL) ;   // default constructor of GEtalonnageMust object
  ~GEtalonnageMust() ;

  int  GetStatus(){return fStatus;}
  void SetStatus(int _Status){fStatus =_Status;}

  void SetModeCalib(int mode){fModeCalib= mode;}
  void SetFirstMuviName(char* name);

  virtual void InitTTreeUser();
  virtual void SetUp2DHisto();
  virtual void InitCalim(char* exp_name, char* cardname, char*host_name,
		char* para_name, int nbpara, int calimode, char *eventinitmode);
  virtual void InitUserRun();
  virtual void InitUser();
  virtual void User();
  virtual void EndUserRun();
  virtual void EndUser();
  virtual void SetVectors(int* gene_value_of_mat, int* selected_channel);
  virtual void SetVectorsStandart();
  virtual int  ConvertInIndex(int EnergieOrTime, int NoMat);
  virtual int  ConvertInIndexinDB(int EnergieOrTime, int NoMat);
  virtual void ConvertInverse(int index,int* tele,int* mate,int *channel,int *energy_or_time);
  virtual void ConvertInverseFromDB(int indexDB,int* tele,int* mate,int *channel,int *energy_or_time);

  ClassDef (GEtalonnageMust ,1); // Treatement of Data

};

#endif


