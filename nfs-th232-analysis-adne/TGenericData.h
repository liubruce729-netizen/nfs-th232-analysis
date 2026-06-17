#ifndef __GenericDATA__
#define __GenericDATA__

#include <vector>

#include "TObject.h"
#include "TObjString.h"



class TGenericData : public TObject {
 private:

   std::vector<UShort_t>	fGeneric_DetNbr;
   std::vector<Float_t>		fGeneric_Energy;
   std::vector<Float_t>		fGeneric_Time;
   std::vector<Int_t>		fGeneric_Type;
   
//Time Stamp
 unsigned  long long   fTimeStampsGeneric[1000] ;


 public:
   TGenericData();
   virtual ~TGenericData();

   void	Clear();
   void	Dump();



   /////////////////////           GETTERS           ////////////////////////
   UShort_t	GetGenericMult()		{return fGeneric_DetNbr.size();}
   UShort_t	GetGenericDet(Int_t i)		{return fGeneric_DetNbr.at(i);}
   Float_t	GetGenericEnergy(Int_t i) 	{return fGeneric_Energy.at(i);}
   Float_t	GetGenericTime(Int_t i) 	{return fGeneric_Time.at(i);}
   Int_t	GetGenericType(Int_t i) 	{return fGeneric_Type.at(i);}
  
   unsigned long long    GetfTimeStamps(int i)   {return fTimeStampsGeneric[i];}
   
   /////////////////////           SETTERS           ////////////////////////
   void	SetGenericDetectorNbr(UShort_t det)    	{fGeneric_DetNbr.push_back(det);}
   void	SetGenericEnergy(Float_t E) 	  	{fGeneric_Energy.push_back(E);}
   void	SetGenericTime(Float_t E) 	  	{fGeneric_Time.push_back(E);}
   void	SetGenericType(Int_t E) 	  	{fGeneric_Type.push_back(E);}


   void SetfTimeStamps(unsigned long long T, int i){fTimeStampsGeneric[i] = T; }
   ClassDef(TGenericData,1)  // GenericData structure
};

#endif
