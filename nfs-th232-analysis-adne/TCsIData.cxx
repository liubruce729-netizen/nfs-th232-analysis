#include <iostream>
using namespace std;

#include "TCsIData.h"


ClassImp(TCsIData)

TCsIData::TCsIData()
{
   // Default constructor

   
   fCsI_EnergyDetNbr.clear();
   fCsI_Energy.clear();

   fCsI_TimeDetNbr.clear();
   fCsI_Time.clear();

   fCsI_PIDetNbr.clear();
   fCsI_PI.clear();
   for(Int_t i=0;i<200;i++)fTimeStampsCsI[i]= 0; //
   
  
}



TCsIData::~TCsIData()
{
}



void TCsIData::Clear()
{

   fCsI_EnergyDetNbr.clear();
   fCsI_Energy.clear();

   fCsI_TimeDetNbr.clear();
   fCsI_Time.clear();

   fCsI_PIDetNbr.clear();
   fCsI_PI.clear();
   for(Int_t i=0;i<200;i++)fTimeStampsCsI[i]= 0; //
}



void TCsIData::Dump()
{
   cout << "XXXXXXXXXXXXXXXXXXXXXXXX New Event XXXXXXXXXXXXXXXXX" << endl;

}
