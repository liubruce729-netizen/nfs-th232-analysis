#ifndef __EXOGAMDATA__
#define __EXOGAMDATA__

#include <vector>

#include "TObject.h"
#include "TObjString.h"



class TExogamData : public TObject {
 private:
   // ECC / Energy
   std::vector<UShort_t>	fEXO_ECC_E_Clover;
   std::vector<UShort_t>	fEXO_ECC_E_Cristal;
   std::vector<Float_t>	fEXO_ECC_E_Energy;
   // ECC / Time
   std::vector<UShort_t>	fEXO_ECC_T_Clover;
   std::vector<UShort_t>	fEXO_ECC_T_Cristal;
   std::vector<UShort_t>	fEXO_ECC_T_Time;
   // GOCCE / Energy
   std::vector<UShort_t>	fEXO_GOCCE_E_Clover;
   std::vector<UShort_t>	fEXO_GOCCE_E_Cristal;
   std::vector<UShort_t>	fEXO_GOCCE_E_Segment;
   std::vector<UShort_t>	fEXO_GOCCE_E_Energy;
   std::vector<UShort_t>	fEXO_GOCCE_E_Status;
   // GOCCE / Time
   std::vector<UShort_t>	fEXO_GOCCE_T_Clover;
   std::vector<UShort_t>	fEXO_GOCCE_T_Cristal;
   std::vector<UShort_t>	fEXO_GOCCE_T_Segment;
   std::vector<UShort_t>	fEXO_GOCCE_T_Time;
  
  
  // ESS / Energy
   std::vector<UShort_t>	fEXO_ESS_E_Clover;
   std::vector<UShort_t>	fEXO_ESS_E_Energy;
  // ESS / T
   std::vector<UShort_t>	fEXO_ESS_T_Clover;
   std::vector<UShort_t>	fEXO_ESS_T_Time;
   // ESS / TQ
   std::vector<UShort_t>	fEXO_ESS_TQ_Clover;
   std::vector<UShort_t>	fEXO_ESS_TQ_Cristal;
   std::vector<UShort_t>	fEXO_ESS_TQ_Time;
   


 public:
   TExogamData();
   virtual ~TExogamData();

   void	Clear();
   void	Dump();


   /////////////////////           SETTERS           ////////////////////////
   // ECC / Energy
   void	SetECCEClover(UShort_t clov)	{ fEXO_ECC_E_Clover.push_back(clov);}
   void	SetECCECristal(UShort_t cris)	{ fEXO_ECC_E_Cristal.push_back(cris);}
   void	SetECCEEnergy(Float_t ener)	{ fEXO_ECC_E_Energy.push_back(ener);}
   // ECC / Time
   void	SetECCTClover(UShort_t clov)	{ fEXO_ECC_T_Clover.push_back(clov);}
   void	SetECCTCristal(UShort_t cris)	{ fEXO_ECC_T_Cristal.push_back(cris);}
   void	SetECCTTime(UShort_t time)	{ fEXO_ECC_T_Time.push_back(time);}
   // GOCCE / Energy
   void	SetGOCCEEClover(UShort_t clov)	{ fEXO_GOCCE_E_Clover.push_back(clov);}
   void	SetGOCCEECristal(UShort_t cris)	{ fEXO_GOCCE_E_Cristal.push_back(cris);}
   void	SetGOCCEESegment(UShort_t seg)	{ fEXO_GOCCE_E_Segment.push_back(seg);}
   void	SetGOCCEEEnergy(UShort_t ener)	{ fEXO_GOCCE_E_Energy.push_back(ener);}
   void	SetGOCCEEStatus(UShort_t ener)	{ fEXO_GOCCE_E_Status.push_back(ener);}
   
   // GOCCE / Time
   void	SetGOCCETClover(UShort_t clov)	{ fEXO_GOCCE_T_Clover.push_back(clov);}
   void	SetGOCCETCristal(UShort_t cris)	{ fEXO_GOCCE_T_Cristal.push_back(cris);}
   void	SetGOCCETSegment(UShort_t seg)	{ fEXO_GOCCE_T_Segment.push_back(seg);}
   void	SetGOCCETTime(UShort_t time)	{ fEXO_GOCCE_T_Time.push_back(time);}
 
   // ESS / Energy
   void	SetESSEClover(UShort_t clov)	{ fEXO_ESS_E_Clover.push_back(clov);}
   void	SetESSEEnergy(UShort_t ener)	{ fEXO_ESS_E_Energy.push_back(ener);}

   // ESS / Time
   void	SetESSTClover(UShort_t clov)	{ fEXO_ESS_T_Clover.push_back(clov);}
   void	SetESSTTime(UShort_t time)	{ fEXO_ESS_T_Time.push_back(time);}

   // ESS / TimeQ
   void	SetESSTQClover(UShort_t clov)	{ fEXO_ESS_TQ_Clover.push_back(clov);}
   void	SetESSTQCristal(UShort_t cris)	{ fEXO_ESS_TQ_Cristal.push_back(cris);}
   void	SetESSTQTime(UShort_t time)	{ fEXO_ESS_TQ_Time.push_back(time);}

 
 


   /////////////////////           GETTERS           ////////////////////////
   // ECC / Energy
   UShort_t	GetECCEMult()		{return fEXO_ECC_E_Clover.size();}
   UShort_t	GetECCEClover(Int_t i)	{return fEXO_ECC_E_Clover.at(i);}
   UShort_t	GetECCECristal(Int_t i)	{return fEXO_ECC_E_Cristal.at(i);}
   Float_t	GetECCEEnergy(Int_t i)	{return fEXO_ECC_E_Energy.at(i);}
   // ECC / Time
   UShort_t	GetECCTMult()		{return fEXO_ECC_T_Clover.size();}
   UShort_t	GetECCTClover(Int_t i)	{return fEXO_ECC_T_Clover.at(i);}
   UShort_t	GetECCTCristal(Int_t i)	{return fEXO_ECC_T_Cristal.at(i);}
   UShort_t	GetECCTTime(Int_t i)	{return fEXO_ECC_T_Time.at(i);}
   // GOCCE / Energy
   UShort_t	GetGOCCEEMult()			{return fEXO_GOCCE_E_Clover.size();}
   UShort_t	GetGOCCEEClover(Int_t i)	{return fEXO_GOCCE_E_Clover.at(i);}
   UShort_t	GetGOCCEECristal(Int_t i)	{return fEXO_GOCCE_E_Cristal.at(i);}
   UShort_t	GetGOCCEESegment(Int_t i)	{return fEXO_GOCCE_E_Segment.at(i);}
   UShort_t	GetGOCCEEEnergy(Int_t i)	{return fEXO_GOCCE_E_Energy.at(i);}
   UShort_t	GetGOCCEEStatus(Int_t i)	{return fEXO_GOCCE_E_Status.at(i);}
   // GOCCE / Time
   UShort_t	GetGOCCETMult()			{return fEXO_GOCCE_T_Clover.size();}
   UShort_t	GetGOCCETClover(Int_t i)	{return fEXO_GOCCE_T_Clover.at(i);}
   UShort_t	GetGOCCETCristal(Int_t i)	{return fEXO_GOCCE_T_Cristal.at(i);}
   UShort_t	GetGOCCETSegment(Int_t i)	{return fEXO_GOCCE_T_Segment.at(i);}
   UShort_t	GetGOCCETTime(Int_t i)		{return fEXO_GOCCE_T_Time.at(i);}
   
   
    
   // ESS / Energy
   UShort_t     GetESSEMult()		  {return fEXO_ESS_E_Clover.size();}
   UShort_t  	GetESSEClover(Int_t i)    {return fEXO_ESS_E_Clover.at(i);}
   UShort_t  	GetESSEEnergy(Int_t i)    {return fEXO_ESS_E_Energy.at(i);}

   // ESS / Time
   UShort_t	GetESSTMult()		  {return fEXO_ESS_T_Clover.size();}
   UShort_t  	GetESSTClover(Int_t i)    {return fEXO_ESS_T_Clover.at(i);}
   UShort_t 	GetESSTTime(Int_t i)      {return fEXO_ESS_T_Time.at(i);}

   // ESS / TimeQ
   UShort_t	GetESSTQMult()		  {return fEXO_ESS_TQ_Clover.size();}
   UShort_t	GetESSTQClover(Int_t i)   {return fEXO_ESS_TQ_Clover.at(i);}
   UShort_t	GetESSTQCristal(Int_t i)  {return fEXO_ESS_TQ_Cristal.at(i);}
   UShort_t	GetESSTQTime(Int_t i)     {return fEXO_ESS_TQ_Time.at(i);}


   ClassDef(TExogamData,1)  // ExogamData structure
};

#endif
