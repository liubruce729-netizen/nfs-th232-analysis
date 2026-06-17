#ifndef __NWallDATA__
#define __NWallDATA__

#include <vector>

#include "TObject.h"
#include "TObjString.h"



class TNWallData : public TObject {
 private:

   std::vector<UShort_t>	fNWall_EnergyDetNbr;
   std::vector<UShort_t>	fNWall_Energy;

   std::vector<UShort_t>	fNWall_TOFDetNbr;
   std::vector<UShort_t>	fNWall_TOF;

   std::vector<UShort_t>	fNWall_TOFCalDetNbr;
   std::vector<UShort_t>	fNWall_TOFCal;

   std::vector<UShort_t>	fNWall_ZCODetNbr;
   std::vector<UShort_t>	fNWall_ZCO;

   std::vector<UShort_t>	fNWall_ZCOCalDetNbr;
   std::vector<Float_t>	fNWall_ZCOCal;

   std::vector<UShort_t>     fNWall_TRFDetNbr;
   std::vector<Float_t>      fNWall_TRF;
   
   
   std::vector<UShort_t>	fNWall_EnergyFrame      ;     
   std::vector<UShort_t>	fNWall_TimeFrame        ;	 
   std::vector<UShort_t>	fNWall_SlowIntegral  	; 
   std::vector<UShort_t>	fNWall_FastIntegral 	;
   std::vector<UShort_t>	fNWall_IntRaiseTime 	;
   std::vector<UShort_t>	fNWall_NeuralNetWork 	;
   std::vector<UShort_t>	fNWall_NbZero	     	;
   std::vector<UShort_t>	fNWall_NeutronFlag  	;
   
   
   
   

   UShort_t	fTAC_BaF2_HF;
   UShort_t	fTAC_FT_HF;
   UShort_t	fTAC_FT_CFD;
   UShort_t	fTAC_CFD_HF;

   Float_t	fTAC_BaF2_HF_C;
   Float_t	fTAC_FT_HF_C;
   Float_t	fTAC_FT_CFD_C;
   Float_t	fTAC_CFD_HF_C;
    //Time Stamp
   long long   fTimeStampsNWall[200] ;

 public:
   TNWallData();
   virtual ~TNWallData();

   void	Clear();
   void	Dump();



   /////////////////////           GETTERS           ////////////////////////
   UShort_t	GetNWallEMult()		  {return fNWall_EnergyDetNbr.size();}
   UShort_t	GetNWallEDetNbr(Int_t i)  {return fNWall_EnergyDetNbr.at(i);}
   UShort_t	GetNWallEnergy(Int_t i)	  {return fNWall_Energy.at(i);}
   
   UShort_t	GetNWallTOFMult()	    {return fNWall_TOFDetNbr.size();}
   UShort_t	GetNWallTOFDetNbr(Int_t i)  {return fNWall_TOFDetNbr.at(i);}
   UShort_t	GetNWallTOF(Int_t i)	    {return fNWall_TOF.at(i);}

   UShort_t	GetNWallTOFCalMult()	   {return fNWall_TOFCalDetNbr.size();}
   UShort_t	GetNWallTOFCalDetNbr(Int_t i) {return fNWall_TOFCalDetNbr.at(i);}
   Float_t	GetNWallTOFCal(Int_t i)	      {return fNWall_TOFCal.at(i);}


   UShort_t	GetNWallTRFMult()	    {return fNWall_TRFDetNbr.size();}
   UShort_t	GetNWallTRFDetNbr(Int_t i) {return fNWall_TRFDetNbr.at(i);}
   Float_t      GetNWallTRF(Int_t i)	   {return fNWall_TRF.at(i);}
   
   UShort_t	GetNWallZCOMult()	   {return fNWall_ZCODetNbr.size();}
   UShort_t	GetNWallZCODetNbr(Int_t i) {return fNWall_ZCODetNbr.at(i);}
   UShort_t	GetNWallZCO(Int_t i)	   {return fNWall_ZCO.at(i);}


   UShort_t	GetNWallZCOCalMult()	   {return fNWall_ZCOCalDetNbr.size();}
   UShort_t	GetNWallZCOCalDetNbr(Int_t i) {return fNWall_ZCOCalDetNbr.at(i);}
   Float_t	GetNWallZCOCal(Int_t i)	      {return fNWall_ZCOCal.at(i);}

   UShort_t     GetNWallEnergyFrame(Int_t i)		  {return     fNWall_EnergyFrame.at(i);}
   UShort_t     GetNWallTimeFrame(Int_t i)		  {return     fNWall_TimeFrame.at(i);}	
   UShort_t	GetNWallSlowIntegral(Int_t i)		  {return     fNWall_SlowIntegral.at(i);}  
   UShort_t	GetNWallFastIntegral(Int_t i)		  {return     fNWall_FastIntegral.at(i);}
   UShort_t	GetNWallIntRaiseTime(Int_t i)		  {return     fNWall_IntRaiseTime.at(i);}
   UShort_t	GetNWallNeuralNetWork(Int_t i)		  {return     fNWall_NeuralNetWork.at(i);}
   UShort_t	GetNWallNbZero(Int_t i)		  	  {return     fNWall_NbZero.at(i);}	
   UShort_t	GetNWallNeutronFlag(Int_t i)		  {return     fNWall_NeutronFlag.at(i);}  




   
   UShort_t	GetTAC_BaF2_HF()  {return fTAC_BaF2_HF;}
   UShort_t	GetTAC_FT_HF()  {return fTAC_FT_HF;}
   UShort_t	GetTAC_FT_CFD()  {return fTAC_FT_CFD;}
   UShort_t	GetTAC_CFD_HF()  {return fTAC_CFD_HF;}

   Float_t	GetTAC_BaF2_HF_C()  {return fTAC_BaF2_HF_C;}
   Float_t	GetTAC_FT_HF_C()    {return fTAC_FT_HF_C;}
   Float_t	GetTAC_FT_CFD_C()   {return fTAC_FT_CFD_C;}
   Float_t	GetTAC_CFD_HF_C()   {return fTAC_CFD_HF_C;}
   unsigned long long    GetfTimeStamps(int i)   {return fTimeStampsNWall[i];}
   
   /////////////////////           SETTERS           ////////////////////////
   void	SetNWallEDetNbr(UShort_t det)    {fNWall_EnergyDetNbr.push_back(det);}
   void	SetNWallEnergy(UShort_t E) 	  {fNWall_Energy.push_back(E);}
    
   void	SetNWallTOFDetNbr(UShort_t det)    {fNWall_TOFDetNbr.push_back(det);}
   void	SetNWallTOF(UShort_t E) 	  {fNWall_TOF.push_back(E);}

   void	SetNWallTOFCalDetNbr(UShort_t det)    {fNWall_TOFCalDetNbr.push_back(det);}
   void	SetNWallTOFCal(Float_t E) 	  {fNWall_TOFCal.push_back(E);}

   void	SetNWallTRFDetNbr(UShort_t det)    {fNWall_TRFDetNbr.push_back(det);}
   void	SetNWallTRF(Float_t E) 	           {fNWall_TRF.push_back(E);}

  
   void	SetNWallZCODetNbr(UShort_t det)    {fNWall_ZCODetNbr.push_back(det);}
   void	SetNWallZCO(UShort_t E) 	   {fNWall_ZCO.push_back(E);}

   void	SetNWallZCOCalDetNbr(UShort_t det)    {fNWall_ZCOCalDetNbr.push_back(det);}
   void	SetNWallZCOCal(Float_t E) 	      {fNWall_ZCOCal.push_back(E);}

   void  SetNWallEnergyFrame(UShort_t E)	      	{fNWall_EnergyFrame.push_back(E);}
   void  SetNWallTimeFrame(UShort_t E)	      		{fNWall_TimeFrame.push_back(E);}  
   void  SetNWallSlowIntegral(UShort_t E)	      	{fNWall_SlowIntegral.push_back(E);}  
   void  SetNWallFastIntegral(UShort_t E)	      	{fNWall_FastIntegral.push_back(E);}
   void  SetNWallIntRaiseTime(UShort_t E)	      	{fNWall_IntRaiseTime.push_back(E);}
   void  SetNWallNeuralNetWork(UShort_t E)        	{fNWall_NeuralNetWork.push_back(E);}
   void  SetNWallNbZero(UShort_t E) 		      	{fNWall_NbZero.push_back(E);}     
   void  SetNWallNeutronFlag(UShort_t E)	      	{fNWall_NeutronFlag.push_back(E);}  
  
   UShort_t     SetTAC_BaF2_HF(UShort_t val)  {return fTAC_BaF2_HF=val;}
   UShort_t     SetTAC_FT_HF(UShort_t val)    {return fTAC_FT_HF=val;}
   UShort_t     SetTAC_FT_CFD(UShort_t val)   {return fTAC_FT_CFD=val;}
   UShort_t     SetTAC_CFD_HF(UShort_t val) {return fTAC_CFD_HF=val;}

   void     SetTAC_BaF2_HF_C(Float_t val)  {fTAC_BaF2_HF_C=val;}
   void     SetTAC_FT_HF_C(Float_t val)    {fTAC_FT_HF_C=val;}
   void     SetTAC_FT_CFD_C(Float_t val)   {fTAC_FT_CFD_C=val;}
   void     SetTAC_CFD_HF_C(Float_t val)   {fTAC_CFD_HF_C=val;}

   void SetfTimeStamps(unsigned long long T, int i){fTimeStampsNWall[i] = T; }
   ClassDef(TNWallData,1)  // NWallData structure
};

#endif
