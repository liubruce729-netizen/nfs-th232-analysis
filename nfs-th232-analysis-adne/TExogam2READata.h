#ifndef __Exogam2READATA__
#define __Exogam2READATA__

#include <vector>

#include "TObject.h"
#include "TObjString.h"



class TExogam2READata : public TObject {
 private:
   // ECC / Energy
   std::vector<UShort_t>	fEXOrea_ECC_E_Clover;
   std::vector<UShort_t>	fEXOrea_ECC_E_Cristal;
   std::vector<UShort_t>	fEXOrea_ECC_E_DetNbr;
   std::vector<Float_t>	        fEXOrea_ECC_E_Energy;
   //PSA
   std::vector<UShort_t> 	fEXOrea_ECC_E_T30;
   std::vector<UShort_t> 	fEXOrea_ECC_E_T60;
   std::vector<UShort_t> 	fEXOrea_ECC_E_T90;
   // ECC / Time
   std::vector<UShort_t>	fEXOrea_ECC_T_Clover;
   std::vector<UShort_t>	fEXOrea_ECC_T_Cristal;
   std::vector<UShort_t>	fEXOrea_ECC_T_Time;
   // GOCCE / Energy
   std::vector<UShort_t>	fEXOrea_GOCCE_E_Clover;
   std::vector<UShort_t>	fEXOrea_GOCCE_E_Cristal;
   std::vector<UShort_t>	fEXOrea_GOCCE_E_Segment;
   std::vector<Float_t>	        fEXOrea_GOCCE_E_Energy;
   std::vector<UShort_t>	fEXOrea_GOCCE_E_Status;
   // GOCCE / Time
   std::vector<UShort_t>	fEXOrea_GOCCE_T_Clover;
   std::vector<UShort_t>	fEXOrea_GOCCE_T_Cristal;
   std::vector<UShort_t>	fEXOrea_GOCCE_T_Segment;
   std::vector<UShort_t>	fEXOrea_GOCCE_T_Time;
  
  // ESS / BGO 
   std::vector<UShort_t>	fEXOrea_ESSBGO_Clover;
   std::vector<UShort_t>	fEXOrea_ESSBGO_Cristal;
   std::vector<UShort_t>	fEXOrea_ESS_BGOE;
   std::vector<UShort_t>	fEXOrea_ESS_BGOT;

// ESS / CsI
   std::vector<UShort_t>	fEXOrea_ESSCSI_Clover;
   std::vector<UShort_t>	fEXOrea_ESSCSI_Cristal;
   std::vector<UShort_t>	fEXOrea_ESS_CSIE;
   std::vector<UShort_t>	fEXOrea_ESS_CSIT;
   
   
//Time Stamp
   long long   fTimeStampsExogam2rea[100] ; //for Core cristal 
   std::vector<ULong_t>   fTimeStampsExogam2rea_Segment; //GOCCE 
   std::vector<ULong_t>   fTimeStampsExogam2rea_BGO; //BGO 
   std::vector<ULong_t>   fTimeStampsExogam2rea_CSI; //CSI 
   

 //Analyzed  
   std::vector<Float_t>		aEXOrea_GammaEnergy;
   std::vector<Float_t>		aEXOrea_GammaTheta;
   std::vector<Float_t>		aEXOrea_GammaPhi;
   std::vector<UShort_t>	aEXOrea_GammaCoreId; 
   
   
 public:
   TExogam2READata();
   virtual ~TExogam2READata();

   void	Clear();
   void	Dump();


   /////////////////////           SETTERS           ////////////////////////
   // ECC / Energy
   void	SetECCEClover(UShort_t clov)	{ fEXOrea_ECC_E_Clover.push_back(clov);}
   void	SetECCECristal(UShort_t cris)	{ fEXOrea_ECC_E_Cristal.push_back(cris);}
   void	SetECCEEnergy(Float_t ener)	{ fEXOrea_ECC_E_Energy.push_back(ener);}
   void	SetECCEDetNbr(UShort_t cris)    { fEXOrea_ECC_E_DetNbr.push_back(cris);}
   // ECC PSA
   void	SetECCET30(UShort_t psa)	{ fEXOrea_ECC_E_T30.push_back(psa);}
   void	SetECCET60(UShort_t psa)	{ fEXOrea_ECC_E_T60.push_back(psa);}
   void	SetECCET90(UShort_t psa)	{ fEXOrea_ECC_E_T90.push_back(psa);}
   
   // ECC / Time
   void	SetECCTClover(UShort_t clov)	{ fEXOrea_ECC_T_Clover.push_back(clov);}
   void	SetECCTCristal(UShort_t cris)	{ fEXOrea_ECC_T_Cristal.push_back(cris);}
   void	SetECCTTime(UShort_t time)	{ fEXOrea_ECC_T_Time.push_back(time);}
   
   // GOCCE / Energy
   void	SetGOCCEEClover(UShort_t clov)	{ fEXOrea_GOCCE_E_Clover.push_back(clov);}
   void	SetGOCCEECristal(UShort_t cris)	{ fEXOrea_GOCCE_E_Cristal.push_back(cris);}
   void	SetGOCCEESegment(UShort_t seg)	{ fEXOrea_GOCCE_E_Segment.push_back(seg);}
   void	SetGOCCEEEnergy(Float_t ener)	{ fEXOrea_GOCCE_E_Energy.push_back(ener);}
   void	SetGOCCEEStatus(UShort_t ener)	{ fEXOrea_GOCCE_E_Status.push_back(ener);}
   
   // GOCCE / Time
   void	SetGOCCETClover(UShort_t clov)	{ fEXOrea_GOCCE_T_Clover.push_back(clov);}
   void	SetGOCCETCristal(UShort_t cris)	{ fEXOrea_GOCCE_T_Cristal.push_back(cris);}
   void	SetGOCCETSegment(UShort_t seg)	{ fEXOrea_GOCCE_T_Segment.push_back(seg);}
   void	SetGOCCETTime(UShort_t time)	{ fEXOrea_GOCCE_T_Time.push_back(time);}
 
 
   // ESS / BGO
   void	SetESSBGOClover(UShort_t clov)	{ fEXOrea_ESSBGO_Clover.push_back(clov);}
   void	SetESSBGOCristal(UShort_t cris)	{ fEXOrea_ESSBGO_Cristal.push_back(cris);}
   void	SetESSBGOE(UShort_t val)	{ fEXOrea_ESS_BGOE.push_back(val);	     }
   void	SetESSBGOT(UShort_t val)	{ fEXOrea_ESS_BGOT.push_back(val);      }
   
   // ESS / CSI
   void	SetESSCSIClover(UShort_t clov)	{ fEXOrea_ESSCSI_Clover.push_back(clov);}
   void	SetESSCSICristal(UShort_t cris)	{ fEXOrea_ESSCSI_Cristal.push_back(cris);}
   void	SetESSCSIE(UShort_t val)	{ fEXOrea_ESS_CSIE.push_back(val);      }
   void	SetESSCSIT(UShort_t val)	{ fEXOrea_ESS_CSIT.push_back(val);      }
   
//Analyzed Data addback
   void	SetGammaEnergy(Float_t val)	{aEXOrea_GammaEnergy.push_back(val);}
   void	SetGammaTheta(Float_t val)	{aEXOrea_GammaTheta.push_back(val);}
   void	SetGammaPhi(Float_t val)	{aEXOrea_GammaPhi.push_back(val);}
   void	SetGammaCoreId(UShort_t cris)	{aEXOrea_GammaCoreId.push_back(cris);}





  unsigned long long    GetfTimeStamps(int i)   {return fTimeStampsExogam2rea[i];}
  unsigned long long    GetfTimeStampsSegment(int i)   {return fTimeStampsExogam2rea_Segment.at(i);}
  unsigned long long    GetfTimeStampsBGO(int i)   {return fTimeStampsExogam2rea_BGO.at(i);}
  unsigned long long    GetfTimeStampsCSI(int i)   {return fTimeStampsExogam2rea_CSI.at(i);}


   /////////////////////           GETTERS           ////////////////////////
   // ECC / Energy
   UShort_t	GetECCEMult()		{return fEXOrea_ECC_E_Clover.size();}
   UShort_t	GetECCEClover(Int_t i)	{return fEXOrea_ECC_E_Clover.at(i);}
   UShort_t	GetECCECristal(Int_t i)	{return fEXOrea_ECC_E_Cristal.at(i);}
   Float_t	GetECCEEnergy(Int_t i)	{return fEXOrea_ECC_E_Energy.at(i);}
   UShort_t	GetECCEDetNbr(Int_t i)	{return fEXOrea_ECC_E_DetNbr.at(i);}
   // ECC PSA
   UShort_t	GetECCET30()	  { return fEXOrea_ECC_E_T30.at(0);}
   UShort_t	GetECCET60()	  { return fEXOrea_ECC_E_T60.at(0);}
   UShort_t	GetECCET90()	  { return fEXOrea_ECC_E_T90.at(0);}
   
   
   // ECC / Time
   UShort_t	GetECCTMult()		{return fEXOrea_ECC_T_Clover.size();}
   UShort_t	GetECCTClover(Int_t i)	{return fEXOrea_ECC_T_Clover.at(i);}
   UShort_t	GetECCTCristal(Int_t i)	{return fEXOrea_ECC_T_Cristal.at(i);}
   UShort_t	GetECCTTime(Int_t i)	{return fEXOrea_ECC_T_Time.at(i);}   

   // GOCCE / Energy
   UShort_t	GetGOCCEEMult()			{return fEXOrea_GOCCE_E_Clover.size();}
   UShort_t	GetGOCCEEClover(Int_t i)	{return fEXOrea_GOCCE_E_Clover.at(i);}
   UShort_t	GetGOCCEECristal(Int_t i)	{return fEXOrea_GOCCE_E_Cristal.at(i);}
   UShort_t	GetGOCCEESegment(Int_t i)	{return fEXOrea_GOCCE_E_Segment.at(i);}
   Float_t	GetGOCCEEEnergy(Int_t i)	{return fEXOrea_GOCCE_E_Energy.at(i);}
   UShort_t	GetGOCCEEStatus(Int_t i)	{return fEXOrea_GOCCE_E_Status.at(i);}
   
   // GOCCE / Time
   UShort_t	GetGOCCETMult()			{return fEXOrea_GOCCE_T_Clover.size();}
   UShort_t	GetGOCCETClover(Int_t i)	{return fEXOrea_GOCCE_T_Clover.at(i);}
   UShort_t	GetGOCCETCristal(Int_t i)	{return fEXOrea_GOCCE_T_Cristal.at(i);}
   UShort_t	GetGOCCETSegment(Int_t i)	{return fEXOrea_GOCCE_T_Segment.at(i);}
   UShort_t	GetGOCCETTime(Int_t i)		{return fEXOrea_GOCCE_T_Time.at(i);}
   
    
   // ESS / BGO
   UShort_t	GetESSBGOMult()		  {return fEXOrea_ESSBGO_Clover.size();}
   UShort_t	GetESSBGOClover(Int_t i)   {return fEXOrea_ESSBGO_Clover.at(i);}
   UShort_t	GetESSBGOCristal(Int_t i)  {return fEXOrea_ESSBGO_Cristal.at(i);}
   UShort_t	GetESSBGOE(Int_t i)     {return fEXOrea_ESS_BGOE.at(i);}
   UShort_t	GetESSBGOT(Int_t i)     {return fEXOrea_ESS_BGOT.at(i);}
   
   // ESS / CSI
   UShort_t	GetESSCSIMult()		  {return fEXOrea_ESSCSI_Clover.size();}
   UShort_t	GetESSCSIClover(Int_t i)   {return fEXOrea_ESSCSI_Clover.at(i);}
   UShort_t	GetESSCSICristal(Int_t i)  {return fEXOrea_ESSCSI_Cristal.at(i);}
   UShort_t	GetESSCSIE(Int_t i)     {return fEXOrea_ESS_CSIE.at(i);}
   UShort_t	GetESSCSIT(Int_t i)     {return fEXOrea_ESS_CSIT.at(i);}
   
   void SetfTimeStampsSegment(unsigned long long T){ fTimeStampsExogam2rea_Segment.push_back(T);}
   void SetfTimeStampsBGO(unsigned long long T){ fTimeStampsExogam2rea_BGO.push_back(T);}
   void SetfTimeStampsCSI(unsigned long long T){ fTimeStampsExogam2rea_CSI.push_back(T);}

  void SetfTimeStamps(unsigned long long T, int i){
 	if(i<100){ 	
		fTimeStampsExogam2rea[i] = T; 
	}
	else printf("TS Error push in EXogamData rea \n");
 }

   ClassDef(TExogam2READata,1)  // Exogam2Data structure
};

#endif
