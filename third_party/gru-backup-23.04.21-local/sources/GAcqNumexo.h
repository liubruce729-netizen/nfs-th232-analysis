// File :  GAcqNumexo.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
// Class  GAcqNumexo specific analyse for Numexo2 cards.
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


#ifndef __GAcqNumexo__
#define __GAcqNumexo__

#include <iostream>
#include <stdint.h>
#define NBTRACE 4
#define SIZETRACE 16384
#define NUMBER_BOARD 8
//using namespace std;
#include <TObject.h>
#include "General.h"
#include "GAcq.h"
#include "GDevice.h"
#include "TH1.h"
#include "MFMOscilloFrame.h"
#include "MFMExogamFrame.h"
//_________________________________________________________________________________________

class GAcqNumexo : public  GAcq{

 protected:

TH1I * mytrace[NBTRACE];
TH1I * mytraceSum;

MFMOscilloFrame  * fOscilloFrame;
MFMCommonFrame * fCommonframe;

int fBoardList[NUMBER_BOARD];
TH1I * DeltaT[EXO_NUMBER_CRISTAL_ID];
TH1I * BGO[EXO_NUMBER_CRISTAL_ID];
TH1I * Csi[EXO_NUMBER_CRISTAL_ID];
TH1I ** Outer[EXO_NUMBER_CRISTAL_ID];
TH1I ** InnerM[EXO_NUMBER_CRISTAL_ID];
TH1I ** InnerT[EXO_NUMBER_CRISTAL_ID];
TH2I * InnerT30vT90[EXO_NUMBER_CRISTAL_ID];
TH2I * InnerT30vT60[EXO_NUMBER_CRISTAL_ID];

TH1F *  NbEvent;
TH1I *  NbEventH;
TH1I *  NbEventL;
Int_t countpoint;
UInt_t fNoEvent,fNoEventOld;
 bool fOscilloFlag;
 bool fAcquiFlag;
 bool fTimeView;  // to show time in x axes rather nb of samples for Oscillo
 int fSizehisto ;//nb of samples
 int fMaxhisto ;//nb of samples
 int fUnittime;//unit base time in nano second per sample

 public:

  GAcqNumexo(GDevice* _fDevIn= NULL, GDevice* _fDevOut= NULL) ;   // default constructor of GUser object
  ~GAcqNumexo() ;

  virtual void InitUser();
  virtual void InitUserRun();
  virtual void User();
  virtual void EndUserRun();
  virtual void EndUser();


 ClassDef (GAcqNumexo ,1); // Controle of Data Flow
};

#endif

