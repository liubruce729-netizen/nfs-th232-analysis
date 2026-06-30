// File :  GParaCaliXml.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GParaCaliXml
//
// This class store coefficients of calibration in xml file
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


#ifndef __GParaCaliXml__
#define __GParaCaliXml__

#include <sstream>
using std::ostream;

#include <TObject.h>
#include "General.h"
#include "GBase.h"
#include <TXMLFile.h>
#include <TH1.h>

//_________________________________________________________________________________________


class GParaCaliXml :public GBase{

 private:

  TDatime *my_time;//!
  TXMLEngine* fXmlFile; //!

  XMLNodePointer_t fXmlMain; //!
  XMLDocPointer_t  fXmlDoc;//!

 protected:

  Int_t fStatus; // Internal status ( to have a satus whithout a returned status which noise the standard output of Cint )
  Float_t fVersion;// version of this class


 public:

  TString fFileName; // Xml File Name

  //char* fWorkingDirectory; //!

  GParaCaliXml() ;   // default constructor of GParaCaliXml object
  ~GParaCaliXml() ;

  virtual void GetParameter (int* no,int* Piedestal, float* Etalon, float* Gene_a1, float* Gene_a2, float* Gene_a3, int* Mate, int* voie,int* telescopes);
  virtual void SaveParameter(int* no,const char* ParaName, int* nb_values, float* Values_Gene, float* Etalon, float* Gene_a1, float* Gene_a2, float* Gene_a3, int* Mate, int* voie,int* telescopes,float* pics, int* nbpics);
  virtual void SaveParameter(int  no,const char* ParaName, int  nb_values, float* Values_Gene, float  Etalon, float  Gene_a1, float  Gene_a2, float  Gene_a3, int  Mate, int  voie,int  telescopes,float* pics, int  nbpics);

  virtual void SaveCoefMatacq(TH1F** Etalonnage,Float_t* DeltaT,Float_t* VernierMin,Float_t* VernierMax,Int_t nb,Int_t longchannel);

  virtual void UpdateFromXML(TXMLEngine* xml,XMLNodePointer_t node);
  virtual int  GetStatus(){return fStatus;}
  virtual void SetStatus(int _Status){fStatus =_Status;}

  virtual void InitXml(bool write_or_read,TString commentaire);
  virtual void CloseXml();

  virtual void GetCoefMatacq(TH1F** Etalonnage ,Float_t* DeltaT,Float_t* VernierMin,Float_t* VernierMax,Int_t nb,Int_t longchannel);

 private:
  virtual void ToDoInCaseOfInterrupt(){};

  ClassDef (GParaCaliXml ,1); // Treatement of Data

};

#endif

