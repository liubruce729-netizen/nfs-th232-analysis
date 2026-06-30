// GInterrupt.h
#ifndef __GInterrupt__
#define __GInterrupt__
#include <iostream>
#include "TSystem.h"
#include "TSysEvtHandler.h"
#include "GNetServer.h"
class GInterrupt: public TSignalHandler {

 public:
  Int_t NumSig;
  GBase* fO;
 public:
  GInterrupt(GBase *Obj,ESignals sig=kSigInterrupt, Bool_t sync=kFALSE);
  ~GInterrupt(){};
  Bool_t Notify();  //   Override.

  ClassDef(GInterrupt,1) // GInterrupt
};
#endif
