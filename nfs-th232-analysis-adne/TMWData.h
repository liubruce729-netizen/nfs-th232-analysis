#ifndef __MWDATA__
#define __MWDATA__

#include <vector>

#include "TObject.h"
#include "TObjString.h"



class TMWData : public TObject {
 private:

   std::vector<Float_t>	fMW_Energy;
   std::vector<UShort_t>	fMW_DetNbr;
   Short_t 		TAC_Val;
   Short_t 		BARREL;
   Short_t 		HYBALL;
   Short_t 		MUST2;
   std::vector<Short_t >	MUST2NRJX;
   std::vector<Short_t >	MUST2NRJY;

 public:
   TMWData();
   virtual ~TMWData();

   void	Clear();
   void	Dump();



   /////////////////////           GETTERS           ////////////////////////
   // (E)
   UShort_t	GetMultE()        {return fMW_Energy.size();}   
   UShort_t	GetMWNbr(Int_t i) {return fMW_DetNbr.at(i);}
   Float_t	GetEnergy(Int_t i)  {return fMW_Energy.at(i);}
   Short_t	GetTac()	  {return TAC_Val; }
   Short_t	GetBARREL()	  {return BARREL; }
   Short_t	GetHYBALL()	  {return HYBALL; }
   Short_t	GetMUST2()	  {return MUST2; }
   Short_t	GetMUST2NRJX(Int_t i)	  {return MUST2NRJX.at(i); }
   Short_t	GetMUST2NRJY(Int_t i)	  {return MUST2NRJY.at(i); }
   UShort_t	GetMUST2NRJX_Mult()	   {return MUST2NRJX.size(); }
   UShort_t	GetMUST2NRJY_Mult()	   {return MUST2NRJY.size(); }
   
   

   /////////////////////           SETTERS           ////////////////////////
   // (E)
   void	SetEnergy(Float_t E)     {fMW_Energy.push_back(E);}
   void	SetMWDetectorNbr(UShort_t det)    {fMW_DetNbr.push_back(det);}
   void SetTac(Short_t tt)	{TAC_Val=tt;}
   void SetBARREL(Short_t tc)	{BARREL=tc;}
   void SetHYBALL(Short_t td)	{HYBALL=td;}
   void SetMUST2(Short_t te)	{MUST2=te;}
   void SetMUST2NRJX(Short_t tf)	{MUST2NRJX.push_back(tf);}
   void SetMUST2NRJY(Short_t tf)	{MUST2NRJY.push_back(tf);}
   
   

   ClassDef(TMWData,1)  // MWData structure
};

#endif
