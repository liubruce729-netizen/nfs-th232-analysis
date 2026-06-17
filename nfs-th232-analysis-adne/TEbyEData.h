#ifndef __EbyEDATA__
#define __EbyEDATA__

#include <vector>

#include "TObject.h"
#include "TObjString.h"



class TEbyEData : public TObject {
 private:

   std::vector<UShort_t>	fEbyE_Label;
   std::vector<UShort_t>	fEbyE_Data;
//Time Stamp
 unsigned  long long   fTimeStampsEbyE ;


 public:
   TEbyEData();
   virtual ~TEbyEData();

   void	Clear();
   void	Dump();
   /////////////////////           GETTERS           ////////////////////////
   UShort_t	GetEbyEMult()			{return fEbyE_Label.size();}
   UShort_t	GetEbyEDet(Int_t i)		{return fEbyE_Label.at(i);}
   Float_t	GetEbyEEnergy(Int_t i) 		{return fEbyE_Data.at(i);}
   unsigned  long long GetEbyETimeStamps()	{return fTimeStampsEbyE;}
   
   /////////////////////           SETTERS           ////////////////////////
   void	SetEbyELabel(UShort_t det)    {fEbyE_Label.push_back(det);}
   void	SetEbyEData(UShort_t det)    {fEbyE_Data.push_back(det);}
   void SetEbyETimeStamps(unsigned long long T ){fTimeStampsEbyE = T; }
   
   ClassDef(TEbyEData,1)  // EbyEData structure
};

#endif
