#include <iostream>
using namespace std;

#include "TParisData.h"


ClassImp(TParisData)

TParisData::TParisData()
{
   // Default constructor

   
   fParis_DetNbr.clear();
   fParis_QShort.clear();
   fParis_QLaBr3Cond.clear();
   fParis_QNaICond.clear();
   fParis_QCompton.clear();
   fParis_QLong.clear();
   fParis_Cfd.clear();
   fParis_Theta.clear();
   fParis_Phi.clear();
   for(Int_t i=0;i<150;i++)fTimeStampsParis[i]=0; //
   
  
}



TParisData::~TParisData()
{
}



void TParisData::Clear()
{

   fParis_DetNbr.clear();
   fParis_QShort.clear();
   fParis_QLaBr3Cond.clear();
   fParis_QNaICond.clear();
   fParis_QCompton.clear();
   fParis_QLong.clear();
   fParis_Cfd.clear();
   fParis_Theta.clear();
   fParis_Phi.clear();

   for(Int_t i=0;i<150;i++)fTimeStampsParis[i]=  0; //
}



void TParisData::Dump()
{
   cout << "XXXXXXXXXXXXXXXXXXXXXXXX New Event XXXXXXXXXXXXXXXXX" << endl;
   for(Int_t i =0 ; i<GetParisMult(); i++){
   		cout<<"GetParisDet "<< GetParisDet(i)<< "::  GetParisQShort  "<<GetParisQShort(i)<<endl;;
  }
 // gets(stop);

}
