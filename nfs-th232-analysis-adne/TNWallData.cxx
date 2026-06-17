#include <iostream>
using namespace std;

#include "TNWallData.h"


ClassImp(TNWallData)

TNWallData::TNWallData()
{
   // Default constructor

   
   fNWall_EnergyDetNbr.clear();
   fNWall_Energy.clear();

   fNWall_TOFDetNbr.clear();
   fNWall_TOF.clear();

   fNWall_TRFDetNbr.clear();
   fNWall_TRF.clear();

   fNWall_ZCODetNbr.clear();
   fNWall_ZCO.clear();

   fNWall_ZCOCalDetNbr.clear();
   fNWall_ZCOCal.clear();
   
   fNWall_EnergyFrame.clear();
   fNWall_TimeFrame.clear();    
   fNWall_SlowIntegral.clear(); 
   fNWall_FastIntegral.clear(); 
   fNWall_IntRaiseTime.clear(); 
   fNWall_NeuralNetWork.clear();
   fNWall_NbZero.clear();	    
   fNWall_NeutronFlag.clear();  
   
   
   
   
   
   for(Int_t i=0;i<200;i++)fTimeStampsNWall[i]= 0; //
  
}



TNWallData::~TNWallData()
{
}



void TNWallData::Clear()
{
   fNWall_EnergyFrame.clear();
   fNWall_TimeFrame.clear();    
   fNWall_SlowIntegral.clear(); 
   fNWall_FastIntegral.clear(); 
   fNWall_IntRaiseTime.clear(); 
   fNWall_NeuralNetWork.clear();
   fNWall_NbZero.clear();	    
   fNWall_NeutronFlag.clear();  
   
   
   fNWall_EnergyDetNbr.clear();
   fNWall_Energy.clear();

   fNWall_TOFDetNbr.clear();
   fNWall_TOF.clear();

   fNWall_TRFDetNbr.clear();
   fNWall_TRF.clear();


   fNWall_ZCODetNbr.clear();
   fNWall_ZCO.clear();

   fNWall_ZCOCalDetNbr.clear();
   fNWall_ZCOCal.clear();
   for(Int_t i=0;i<200;i++)fTimeStampsNWall[i]= 0; //
}



void TNWallData::Dump()
{
   cout << "XXXXXXXXXXXXXXXXXXXXXXXX New Event XXXXXXXXXXXXXXXXX" << endl;

}

