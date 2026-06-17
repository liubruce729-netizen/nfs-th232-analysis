#include <iostream>
using namespace std;

#include "TVamosICData.h"


ClassImp(TVamosICData)

TVamosICData::TVamosICData()
{
   // Default constructor

   
   fVamosIC_DetNbr.clear();
   fVamosIC_Energy.clear();
   fVamosIC_Type.clear();
   for(Int_t i=0;i<1000;i++)fTimeStampsVamosIC[i]= 0; //
   
  
}



TVamosICData::~TVamosICData()
{
}



void TVamosICData::Clear()
{

   fVamosIC_DetNbr.clear();
   fVamosIC_Energy.clear();
   fVamosIC_Type.clear();
   for(Int_t i=0;i<1000;i++)fTimeStampsVamosIC[i]= 0; //
}



void TVamosICData::Dump()
{
   cout << "XXXXXXXXXXXXXXXXXXXXXXXX New Event XXXXXXXXXXXXXXXXX" << endl;
   for(Int_t i =0 ; i<GetVamosICMult(); i++){
   		cout<<"GetVamosICDet "<< GetVamosICDet(i)<< "::  GetVamosICQShort  "<<GetVamosICEnergy(i)<<endl;;
  }
  //gets(stop);

}
