#include <iostream>
using namespace std;

#include "TEbyEData.h"


ClassImp(TEbyEData)

TEbyEData::TEbyEData()
{
   // Default constructor

   
   fEbyE_Label.clear();
   fEbyE_Data.clear();
   fTimeStampsEbyE=0;
  
}



TEbyEData::~TEbyEData()
{
}



void TEbyEData::Clear()
{

   fEbyE_Label.clear();
   fEbyE_Data.clear();
   fTimeStampsEbyE=0;

    
}



void TEbyEData::Dump()
{
   
}
