#include <iostream>
using namespace std;

#include "TMWData.h"


ClassImp(TMWData)

TMWData::TMWData()
{
   // Default constructor

   // (E)
   fMW_Energy.clear();
   fMW_DetNbr.clear();
   TAC_Val=BARREL=HYBALL=MUST2=0;
   MUST2NRJX.clear();
   MUST2NRJY.clear();
   
   
}



TMWData::~TMWData()
{
}



void TMWData::Clear()
{
   // (E)
   fMW_Energy.clear();
   fMW_DetNbr.clear();
   TAC_Val=BARREL=HYBALL=MUST2=0;
   MUST2NRJX.clear();
   MUST2NRJY.clear();
}



void TMWData::Dump()
{
   cout << "XXXXXXXXXXXXXXXXXXXXXXXX New Event XXXXXXXXXXXXXXXXX" << endl;

   // MW
   // (E)
   cout << "MW_MultE = " << fMW_Energy.size() << endl;
   for (UShort_t i = 0; i < fMW_Energy.size(); i++)
      cout <<  " Energy: " << fMW_Energy[i] << endl;

  
}
