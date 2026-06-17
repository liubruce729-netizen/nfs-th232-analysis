#include "TDetector.h"


long long TDetector::fTimestamp=-1;
long long TDetector::fTimestampT0=-1;

ClassImp(TDetector)

TDetector::TDetector()
{
   // Default constructor
   fLabelMap.clear();
   fTypeMap.clear();
   fParameterMap.clear();	 
}



TDetector::~TDetector()
{
}
