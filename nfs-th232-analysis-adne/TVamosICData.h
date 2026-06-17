#ifndef __VamosICDATA__
#define __VamosICDATA__

#include <vector>

#include "TObject.h"
#include "TObjString.h"



class TVamosICData : public TObject {
 private:

   std::vector<UShort_t>	fVamosIC_DetNbr;
   std::vector<Float_t>	fVamosIC_Energy;
   std::vector<Int_t>	fVamosIC_Type;
   
//Time Stamp
 unsigned  long long   fTimeStampsVamosIC[1000] ;


 public:
   TVamosICData();
   virtual ~TVamosICData();

   void	Clear();
   void	Dump();



   /////////////////////           GETTERS           ////////////////////////
   UShort_t	GetVamosICMult()		{return fVamosIC_DetNbr.size();}
   UShort_t	GetVamosICDet(Int_t i)		{return fVamosIC_DetNbr.at(i);}
   Float_t	GetVamosICEnergy(Int_t i) 	{return fVamosIC_Energy.at(i);}
   Int_t	GetVamosICType(Int_t i) 	{return fVamosIC_Type.at(i);}
  
   unsigned long long    GetfTimeStamps(int i)   {return fTimeStampsVamosIC[i];}
   
   /////////////////////           SETTERS           ////////////////////////
   void	SetVamosICDetectorNbr(UShort_t det)    	{fVamosIC_DetNbr.push_back(det);}
   void	SetVamosICEnergy(Float_t E) 	  	{fVamosIC_Energy.push_back(E);}
   void	SetVamosICType(Int_t E) 	  	{fVamosIC_Type.push_back(E);}


   void SetfTimeStamps(unsigned long long T, int i){fTimeStampsVamosIC[i] = T; }
   ClassDef(TVamosICData,1)  // VamosICData structure
};

#endif
