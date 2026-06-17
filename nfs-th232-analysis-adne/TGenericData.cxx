#include <iostream>
using namespace std;

#include "TGenericData.h"


ClassImp(TGenericData)

TGenericData::TGenericData()
{
   // Default constructor

   
   fGeneric_DetNbr.clear();
   fGeneric_Energy.clear();
   fGeneric_Type.clear();
   fGeneric_Time.clear();
   for(Int_t i=0;i<1000;i++)fTimeStampsGeneric[i]= 0; //
   
  
}



TGenericData::~TGenericData()
{
}



void TGenericData::Clear()
{

   fGeneric_DetNbr.clear();
   fGeneric_Energy.clear();
   fGeneric_Time.clear();
   fGeneric_Type.clear();
   for(Int_t i=0;i<1000;i++)fTimeStampsGeneric[i]= 0; //
}



void TGenericData::Dump()
{
   cout << "XXXXXXXXXXXXXXXXXXXXXXXX New Event XXXXXXXXXXXXXXXXX" << endl;
   for(Int_t i =0 ; i<GetGenericMult(); i++){
   		cout<<"GetGenericDet "<< GetGenericDet(i)<< "::  GetGenericQShort  "<<GetGenericEnergy(i)<<endl;;
  }
  //gets(stop);

}
