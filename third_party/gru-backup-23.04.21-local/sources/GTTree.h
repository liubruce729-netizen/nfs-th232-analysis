// File : GTTree.h
// Author: Luc Legeard 
//////////////////////////////////////////////////////////////////////////////
//
// Class GTTree
//
// This class manger TTree for GRU
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

#ifndef __GTTree__
#define __GTTree__
#include <TObject.h>
#include <TFile.h>
#include <TTree.h>
#include "General.h"
#include "GBase.h"
#include "GEventBase.h"
#include "DataParameters.h"
#include <time.h>
#include <sys/time.h>
//______________________________________________________________________________________

class GTTree : public GBase {

 public:
  int            fStatus;

  int  fTTreeMode;    // creation and configuration mode of TTree
  //TREE_NO : no TTree
  //TREE_STANDARD : Standard initialization is a sheet by parameter
  //  this conversion can be  slow .
  //TREE_ONE_VECTOR : TTree is initialized  with only on sheet with "my branch" as name. In this sheet we store
  // a fix size vector which contain a values of event (all values of parameter, even a parameter is null)
  // b_spectra = true , authorize the creation of standard histogram
  //TREE_USER : user TTree but need to use GUser class

  bool fReinitTree;   // Reinitialization of run for each run
  bool fInitTreeNeverDone; // true if init tree is not done
  bool fTTreeWithRunNumber;   // TRee will contain run number in its file name
  bool fSkipFillTtree; // Flag to significate that we do not fill the tree with current TTree
  bool fIsUserInit; // Flag to determine if we are in case of ttree user init 
  Int_t fTTreeCompressionLevel; // Compression level of TTree file 1 to 9 ( 1 low level but fast 9 high level but slow)
  TTree *fTree ;       //! Root tree  where the Ganil data will be converted and strored
  TFile *fTreeFile ;   //! File for TTree theTree
  char  fNameTreeFile[MAX_CARACTERES]; //! name of TTree file
  struct timeval  fMt_reference;
  struct timeval  fMt_autosave; // to compute time between 2 save of TTree
  struct timezone fTz;
  int	 fDiff_time_save ;

 public :

  GTTree();
  virtual ~GTTree();
  virtual  int GetTTreeMode(){ return fTTreeMode ;}
  virtual  bool IsUserInit(){ return fIsUserInit;};
  virtual  char* GetNameTreeFile(){ return fNameTreeFile;}
  virtual  void SetTTreeMode   (TTreeMode mode, const char* filename,bool reinit, bool withrun);
  virtual  void SetCompressionLevel(int level =1)   {fTTreeCompressionLevel =level;};
  virtual  void SetSkipFillTree(bool skiptree=false){ fSkipFillTtree = skiptree;};
  virtual  void AutoSave();
  virtual TTree*    GetTree()    {return fTree;}
  virtual TFile*    GetTreeFile(){return fTreeFile;}
  void ToDoInCaseOfInterrupt(){};
  virtual void FillTTreeOnce();
  virtual void StopTree();
  virtual void InitTTreeUser(){};
  virtual void InitTTree(int runnumber,GEventBase* event);
  ClassDef (GTTree ,1); // TTree management for GRU
};

#endif
