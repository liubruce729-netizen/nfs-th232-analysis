#ifndef __CsIDATA__
#define __CsIDATA__

#include <vector>

#include "TObject.h"
#include "TObjString.h"



class TCsIData : public TObject {
 private:

   std::vector<UShort_t>	fCsI_EnergyDetNbr;
   std::vector<UShort_t>	fCsI_Energy;

   std::vector<UShort_t>	fCsI_TimeDetNbr;
   std::vector<UShort_t>	fCsI_Time;

   std::vector<UShort_t>	fCsI_PIDetNbr;
   std::vector<UShort_t>	fCsI_PI;
   
   
   
//Time Stamp
   long long   fTimeStampsCsI[200] ;


 public:
   TCsIData();
   virtual ~TCsIData();

   void	Clear();
   void	Dump();



   /////////////////////           GETTERS           ////////////////////////
   UShort_t	GetCsIEMult()		  {return fCsI_EnergyDetNbr.size();}
   UShort_t	GetCsIEDetectorNbr(Int_t i) {return fCsI_EnergyDetNbr.at(i);}
   UShort_t	GetCsIEnergy(Int_t i)	  {return fCsI_Energy.at(i);}
   
   UShort_t	GetCsITMult()		  {return fCsI_TimeDetNbr.size();}
   UShort_t	GetCsITDetectorNbr(Int_t i) {return fCsI_TimeDetNbr.at(i);}
   UShort_t	GetCsITime(Int_t i)	  {return fCsI_Time.at(i);}
   
   UShort_t	GetCsIPIMult()		  {return fCsI_PIDetNbr.size();}
   UShort_t	GetCsIPIDetectorNbr(Int_t i) {return fCsI_PIDetNbr.at(i);}
   UShort_t	GetCsIPI(Int_t i)	  {return fCsI_PI.at(i);}
   
   unsigned long long    GetfTimeStamps(int i)   {return fTimeStampsCsI[i];}
   
   /////////////////////           SETTERS           ////////////////////////
   void	SetCsIEDetectorNbr(UShort_t det)    {fCsI_EnergyDetNbr.push_back(det);}
   void	SetCsIEnergy(UShort_t E) 	  {fCsI_Energy.push_back(E);}
  
  
   void	SetCsITDetectorNbr(UShort_t det)    {fCsI_TimeDetNbr.push_back(det);}
   void	SetCsITime(UShort_t E) 	  {fCsI_Time.push_back(E);}
  
   void	SetCsIPIDetectorNbr(UShort_t det)    {fCsI_PIDetNbr.push_back(det);}
   void	SetCsIPI(UShort_t E) 	  {fCsI_PI.push_back(E);}
  
   void SetfTimeStamps(unsigned long long T, int i){fTimeStampsCsI[i] = T; }
   ClassDef(TCsIData,1)  // CsIData structure
};

#endif
