// GInterrupt.cc
#include "GInterrupt.h"
using namespace std;
ClassImp(GInterrupt)

GInterrupt::GInterrupt(GBase* Obj,ESignals sig, Bool_t sync ):
TSignalHandler(sig, sync){
	fO= Obj;
	NumSig=0;
}
Bool_t  GInterrupt::Notify(){
	if (fO){
	fO->ToDoInCaseOfInterrupt();
	}
	//std::cerr << "You pressed crtl-C for the " << ++NumSig << " time !\n";
  if(NumSig > 5){
    gSystem->Exit(2);
  }

  return(1);
}
