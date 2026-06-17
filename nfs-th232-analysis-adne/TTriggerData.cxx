#include <iostream>
using namespace std;

#include "TTriggerData.h"


ClassImp(TTriggerData)

TTriggerData::TTriggerData()
{
   // Default constructor

   // (T)
   fTRIG_1 = 0;
   fTRIG_2 = 0;
   fTRIG_3 = 0;
   fTRIG_4 = 0;
   for(Int_t i=0;i<30;i++)fAGAVADATA[i]= 0; //
   for(Int_t i=0;i<10;i++)fTimeStampsGlobal[i]= 0; //
}



TTriggerData::~TTriggerData()
{
}



void TTriggerData::Clear()
{
   // (T)
   fTRIG_1 = 0;
   fTRIG_2 = 0;
   fTRIG_3 = 0;
   fTRIG_4 = 0;
   for(Int_t i=0;i<30;i++)fAGAVADATA[i]= 0; //
   for(Int_t i=0;i<10;i++)fTimeStampsGlobal[i]= 0; //
}



void TTriggerData::Dump()
{
   cout << "XXXXXXXXXXXXXXXXXXXXXXXX New Event XXXXXXXXXXXXXXXXX" << endl;

   cout << "TRIG_1 : " << fTRIG_1 << endl;
   cout << "TRIG_2 : " << fTRIG_2 << endl;
   cout << "TRIG_3 : " << fTRIG_3 << endl;
   cout << "TRIG_4 : " << fTRIG_4 << endl;
   cout << "Time Stamper : " << fTimeStampsGlobal[0]  <<endl;
  
}
