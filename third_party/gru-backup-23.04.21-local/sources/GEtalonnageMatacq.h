// File :  GEtalonnageMatacq.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GEtalonnageMatacq
//
// This class do specific analisys using calibration tools for Mataq cards
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


#ifndef __GEtalonnageMatacq__
#define __GEtalonnageMatacq__

#include <sstream>
using std::ostream;

#include <TObject.h>
#include "General.h"
#include "GAcq.h"
#include "GParaCaliXml.h"
// definition of card constant

#include "GMatacq.h"

//_________________________________________________________________________________________


class GEtalonnageMatacq : public GAcq{

 private:

  // Muvi

  Int_t fnRun; // nb of run in case of multirun ( rampe)
  int  fModeCalib;// 1 =test   2= control    3= rampe  4=rampe p�rodkique

  int fIncrement1;
  int fIncrement2;


 //Matacq

  TString fMatacqPostrig,fMatacqTrigRec ,fMatacqNbCol ,fMatacqMask,fMatacqVernierbase;
  TH1I *fCurtrace[NB_CHANNEL],*fTrace[5][NB_CHANNEL];//!
  TH1F *fEtalonnage[NB_CHANNEL];//!
  TH1F *fHistoVernier[NB_CHANNEL];//!
  Int_t  fIncrement[NB_CHANNEL];//!
  Float_t  fVernierMin[NB_CHANNEL];//!
  Float_t  fVernierMax[NB_CHANNEL];//!
  Float_t fDeltaT[NB_CHANNEL];//!
  Int_t  fLabelVernier[NB_CHANNEL];//!int
  int  fNoChannels[NB_CHANNEL];// no of selected channel for each run
  TH2I *fTrace2D;//!
  Int_t fHistory ;
  Int_t fCurrent2D;
  Int_t fHistoryTrace;
  Int_t fCurrentTrace;
  Int_t fCurrentNumberChannel;
  UShort_t fLongTrace;
  UShort_t fLabelTrace;
  UShort_t fLabelPostTrig;
  UShort_t fLabelTrigRec;
  UShort_t fLabelNbCol;
  UShort_t fLabelMask;
  Int_t fNbOfWarning;
  char *fNameFirstParameter;//!

 protected:

  int fStatus; // Internal status ( to have a satus whithout a returned status which noise the standard output of Cint )
  int fFirstIndex; //  first raw Muvi parameter
   GParaCaliXml  fXmlFile;


 public:

  GEtalonnageMatacq(GDevice* _fDevIn= NULL, GDevice* _fDevOut= NULL) ;   // default constructor of GEtalonnageMatacq object
  ~GEtalonnageMatacq() ;

  int  GetStatus(){return fStatus;}
  void SetStatus(int _Status){fStatus =_Status;}

  void SetModeCalib(int mode){fModeCalib= mode;}

  virtual void InitCalim(char* exp_name, char* cardname, char*host_name,
		char* para_name, int nbpara, int calimode, char *eventinitmode);
  virtual void InitTTreeUser();
  virtual void InitEtalonnage(char* para_name =NULL);
  virtual void InitUserRun();
  virtual void InitUser();
  virtual void User();
  virtual void EndUserRun();
  virtual void EndUser();




  ClassDef (GEtalonnageMatacq ,1); // Treatement of Data

};

#endif


