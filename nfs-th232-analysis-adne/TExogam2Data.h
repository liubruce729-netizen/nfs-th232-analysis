#ifndef __Exogam2DATA__
#define __Exogam2DATA__

#include <vector>

#include "TObject.h"
#include "TObjString.h"



class TExogam2Data : public TObject {
 private:
   // ECC / Energy
   std::vector<UShort_t>	fEXO_ECC_E_Clover;
   std::vector<UShort_t>	fEXO_ECC_E_Cristal;
   std::vector<UShort_t>	fEXO_ECC_E_DetNbr;
   std::vector<Float_t>	        fEXO_ECC_E_Energy;
   //PSA
   std::vector<UShort_t> 	fEXO_ECC_E_T30;
   std::vector<UShort_t> 	fEXO_ECC_E_T60;
   std::vector<UShort_t> 	fEXO_ECC_E_T90;
   // ECC / Time
   std::vector<UShort_t>	fEXO_ECC_T_Clover;
   std::vector<UShort_t>	fEXO_ECC_T_Cristal;
   std::vector<UShort_t>	fEXO_ECC_T_Time;
   std::vector<UShort_t>	fEXO_ECC_T_TSRequest;
   // GOCCE / Energy
   std::vector<UShort_t>	fEXO_GOCCE_E_Clover;
   std::vector<UShort_t>	fEXO_GOCCE_E_Cristal;
   std::vector<UShort_t>	fEXO_GOCCE_E_Segment;
   std::vector<Float_t>		fEXO_GOCCE_E_Energy;
   std::vector<UShort_t>	fEXO_GOCCE_E_Status;
   // GOCCE / Time
   std::vector<UShort_t>	fEXO_GOCCE_T_Clover;
   std::vector<UShort_t>	fEXO_GOCCE_T_Cristal;
   std::vector<UShort_t>	fEXO_GOCCE_T_Segment;
   std::vector<UShort_t>	fEXO_GOCCE_T_Time;
  
   // ESS / Energy
   std::vector<UShort_t>	fEXO_ESS_Clover;
   std::vector<UShort_t>	fEXO_ESS_Cristal;
   std::vector<UShort_t>	fEXO_ESS_BGO;
   std::vector<UShort_t>	fEXO_ESS_CSI;
   
   
   
   //Analyzed Data  //Addbacked
   std::vector<Float_t>		aEXO_GammaEnergy;
   std::vector<Float_t>		aEXO_GammaTheta;
   std::vector<Float_t>		aEXO_GammaPhi;
   std::vector<UShort_t>	aEXO_GammaCoreId;
   std::vector<Float_t>		aEXO_GammaX; //segment level
   std::vector<Float_t>		aEXO_GammaY;
   std::vector<Float_t>		aEXO_GammaZ;
   std::vector<Float_t>		aEXO_GammaX_Core;
   std::vector<Float_t>		aEXO_GammaY_Core;
   std::vector<Float_t>		aEXO_GammaZ_Core;
   
  
  //Analyzed Data  //not Addbacked
   std::vector<Float_t>		sEXO_GammaEnergy;
   std::vector<Float_t>		sEXO_GammaTheta;
   std::vector<Float_t>		sEXO_GammaPhi;
   std::vector<UShort_t>	sEXO_GammaCoreId;
   std::vector<Float_t>		sEXO_GammaX; //segment level
   std::vector<Float_t>		sEXO_GammaY;
   std::vector<Float_t>		sEXO_GammaZ;
   std::vector<Float_t>		sEXO_GammaX_Core;
   std::vector<Float_t>		sEXO_GammaY_Core;
   std::vector<Float_t>		sEXO_GammaZ_Core;
   
   std::vector<Float_t>		fEXO_Neutron_NRJ;
   std::vector<Float_t>		fDeltaT;       // reversed EXOGAM DeltaT converted to ns
   std::vector<Float_t>		fNeutronTOF;   // reversed DeltaT plus gamma-flash offset in ns
   std::vector<Float_t>		fTime;         // EN: per-crystal analysis Time in ns; CN: 每个 crystal 的分析 Time，单位 ns

   // NFS E877-style clover addback / NFS E877 格式 clover 合并
   std::vector<UShort_t>	f_E877_Clover;
   std::vector<Float_t>		f_E877_Clover_E;
   std::vector<Float_t>		f_E877_Clover_T;
   std::vector<Float_t>		f_E877_Clover_BGO;
   std::vector<Float_t>		f_E877_Clover_CSI;
  
//Time Stamp
   long long   fTimeStampsExogam2[100] ;
 
 public:
   TExogam2Data();
   virtual ~TExogam2Data();

   void	Clear();
   void	Dump();


   /////////////////////           SETTERS           ////////////////////////
   // ECC / Energy
   void	SetECCEClover(UShort_t clov)	{ fEXO_ECC_E_Clover.push_back(clov);}
   void	SetECCECristal(UShort_t cris)	{ fEXO_ECC_E_Cristal.push_back(cris);}
   void	SetECCEEnergy(Float_t ener)	{ fEXO_ECC_E_Energy.push_back(ener);}
   void	SetECCEDetNbr(UShort_t cris)    { fEXO_ECC_E_DetNbr.push_back(cris);}
   // ECC PSA
   void	SetECCET30(UShort_t psa)	{ fEXO_ECC_E_T30.push_back(psa);}
   void	SetECCET60(UShort_t psa)	{ fEXO_ECC_E_T60.push_back(psa);}
   void	SetECCET90(UShort_t psa)	{ fEXO_ECC_E_T90.push_back(psa);}
   
   // ECC / Time
   void	SetECCTClover(UShort_t clov)	{ fEXO_ECC_T_Clover.push_back(clov);}
   void	SetECCTCristal(UShort_t cris)	{ fEXO_ECC_T_Cristal.push_back(cris);}
   void	SetECCTTime(UShort_t time)	{ fEXO_ECC_T_Time.push_back(time);}
   void	SetECCTTSRequest(UShort_t time) { fEXO_ECC_T_TSRequest.push_back(time);}
   
   // GOCCE / Energy
   void	SetGOCCEEClover(UShort_t clov)	{ fEXO_GOCCE_E_Clover.push_back(clov);}
   void	SetGOCCEECristal(UShort_t cris)	{ fEXO_GOCCE_E_Cristal.push_back(cris);}
   void	SetGOCCEESegment(UShort_t seg)	{ fEXO_GOCCE_E_Segment.push_back(seg);}
   void	SetGOCCEEEnergy(Float_t ener)	{ fEXO_GOCCE_E_Energy.push_back(ener);}
   void	SetGOCCEEStatus(UShort_t ener)	{ fEXO_GOCCE_E_Status.push_back(ener);}
   
   // GOCCE / Time
   void	SetGOCCETClover(UShort_t clov)	{ fEXO_GOCCE_T_Clover.push_back(clov);}
   void	SetGOCCETCristal(UShort_t cris)	{ fEXO_GOCCE_T_Cristal.push_back(cris);}
   void	SetGOCCETSegment(UShort_t seg)	{ fEXO_GOCCE_T_Segment.push_back(seg);}
   void	SetGOCCETTime(UShort_t time)	{ fEXO_GOCCE_T_Time.push_back(time);}
 
 
   // ESS / TimeQ
   void	SetESSTQClover(UShort_t clov)	{ fEXO_ESS_Clover.push_back(clov);}
   void	SetESSTQCristal(UShort_t cris)	{ fEXO_ESS_Cristal.push_back(cris);}
   void	SetESSTQBGO(UShort_t val)	{ fEXO_ESS_BGO.push_back(val);}
   void	SetESSTQCSI(UShort_t val)	{ fEXO_ESS_CSI.push_back(val);}

  unsigned long long    GetfTimeStamps(int i)   {return fTimeStampsExogam2[i];}

   //Analyzed Data addback
   void	SetGammaEnergy(Float_t val)	{aEXO_GammaEnergy.push_back(val);}
   void	SetGammaTheta(Float_t val)	{aEXO_GammaTheta.push_back(val);}
   void	SetGammaPhi(Float_t val)	{aEXO_GammaPhi.push_back(val);}
   void	SetGammaCoreId(UShort_t cris)	{aEXO_GammaCoreId.push_back(cris);}
   void	SetGammaX(Float_t val)		{aEXO_GammaX.push_back(val);}
   void	SetGammaY(Float_t val)		{aEXO_GammaY.push_back(val);}
   void	SetGammaZ(Float_t val)		{aEXO_GammaZ.push_back(val);}
   void SetGammaX_Core(Float_t val)	{aEXO_GammaX_Core.push_back(val);}
   void	SetGammaY_Core(Float_t val)	{aEXO_GammaY_Core.push_back(val);}
   void	SetGammaZ_Core(Float_t val)	{aEXO_GammaZ_Core.push_back(val);}
   
  //Analyzed Data not addback:: single
   void	SingleSetGammaEnergy(Float_t val)	{sEXO_GammaEnergy.push_back(val);}
   void	SingleSetGammaTheta(Float_t val)	{sEXO_GammaTheta.push_back(val);}
   void	SingleSetGammaPhi(Float_t val)		{sEXO_GammaPhi.push_back(val);}
   void	SingleSetGammaCoreId(UShort_t cris)	{sEXO_GammaCoreId.push_back(cris);}
   void	SingleSetGammaX(Float_t val)		{sEXO_GammaX.push_back(val);}
   void	SingleSetGammaY(Float_t val)		{sEXO_GammaY.push_back(val);}
   void	SingleSetGammaZ(Float_t val)		{sEXO_GammaZ.push_back(val);}
   void SingleSetGammaX_Core(Float_t val)	{sEXO_GammaX_Core.push_back(val);}
   void	SingleSetGammaY_Core(Float_t val)	{sEXO_GammaY_Core.push_back(val);}
   void	SingleSetGammaZ_Core(Float_t val)	{sEXO_GammaZ_Core.push_back(val);}
   void	SetNeutronNRJ(Float_t val)	        {fEXO_Neutron_NRJ.push_back(val);}
   void	SetDeltaT(Float_t val)	        {fDeltaT.push_back(val);}
   void	SetNeutronTOF(Float_t val)	        {fNeutronTOF.push_back(val);}
   void	SetTime(Float_t val)	        {fTime.push_back(val);}
   void SetE877Clover(UShort_t val)        {f_E877_Clover.push_back(val);}
   void SetE877CloverEnergy(Float_t val)   {f_E877_Clover_E.push_back(val);}
   void SetE877CloverTime(Float_t val)     {f_E877_Clover_T.push_back(val);}
   void SetE877CloverBGO(Float_t val)      {f_E877_Clover_BGO.push_back(val);}
   void SetE877CloverCSI(Float_t val)      {f_E877_Clover_CSI.push_back(val);}
   
   
   /////////////////////           GETTERS           ////////////////////////
   // ECC / Energy
   UShort_t	GetECCEMult()		{return fEXO_ECC_E_Clover.size();}
   UShort_t	GetECCEClover(Int_t i)	{return fEXO_ECC_E_Clover.at(i);}
   UShort_t	GetECCECristal(Int_t i)	{return fEXO_ECC_E_Cristal.at(i);}
   Float_t	GetECCEEnergy(Int_t i)	{return fEXO_ECC_E_Energy.at(i);}
   UShort_t	GetECCEDetNbr(Int_t i)	{return fEXO_ECC_E_DetNbr.at(i);}
   // ECC PSA
   UShort_t	GetECCET30()	  { return fEXO_ECC_E_T30.at(0);}
   UShort_t	GetECCET60()	  { return fEXO_ECC_E_T60.at(0);}
   UShort_t	GetECCET90()	  { return fEXO_ECC_E_T90.at(0);}
   
   
   // ECC / Time
   UShort_t	GetECCTMult()		{return fEXO_ECC_T_Clover.size();}
   UShort_t	GetECCTClover(Int_t i)	{return fEXO_ECC_T_Clover.at(i);}
   UShort_t	GetECCTCristal(Int_t i)	{return fEXO_ECC_T_Cristal.at(i);}
   UShort_t	GetECCTTime(Int_t i)	{return fEXO_ECC_T_Time.at(i);}   
   UShort_t	GetECCTTSRequest(Int_t i) 	{return  fEXO_ECC_T_TSRequest.at(i);}
   UShort_t	GetDeltaTMult()		{return fDeltaT.size();}
   Float_t	GetDeltaT(Int_t i)	{return fDeltaT.at(i);}
   UShort_t	GetNeutronTOFMult()	{return fNeutronTOF.size();}
   Float_t	GetNeutronTOF(Int_t i)	{return fNeutronTOF.at(i);}
   UShort_t	GetTimeMult()		{return fTime.size();}
   Float_t	GetTime(Int_t i)	{return fTime.at(i);}
   UShort_t	GetE877CloverMult()	      {return f_E877_Clover.size();}
   UShort_t	GetE877Clover(Int_t i)	      {return f_E877_Clover.at(i);}
   Float_t	GetE877CloverEnergy(Int_t i) {return f_E877_Clover_E.at(i);}
   Float_t	GetE877CloverTime(Int_t i)   {return f_E877_Clover_T.at(i);}
   Float_t	GetE877CloverBGO(Int_t i)    {return f_E877_Clover_BGO.at(i);}
   Float_t	GetE877CloverCSI(Int_t i)    {return f_E877_Clover_CSI.at(i);}
   UShort_t	GetE877CloverVetoMult() {
      UShort_t mult=0;
      for(UShort_t i=0;i<f_E877_Clover.size();i++){
         if(f_E877_Clover_BGO.at(i)<=0 && f_E877_Clover_CSI.at(i)<=0)mult++;
      }
      return mult;
   }

   // GOCCE / Energy
   UShort_t	GetGOCCEEMult()			{return fEXO_GOCCE_E_Clover.size();}
   UShort_t	GetGOCCEEClover(Int_t i)	{return fEXO_GOCCE_E_Clover.at(i);}
   UShort_t	GetGOCCEECristal(Int_t i)	{return fEXO_GOCCE_E_Cristal.at(i);}
   UShort_t	GetGOCCEESegment(Int_t i)	{return fEXO_GOCCE_E_Segment.at(i);}
   Float_t	GetGOCCEEEnergy(Int_t i)	{return fEXO_GOCCE_E_Energy.at(i);}
   UShort_t	GetGOCCEEStatus(Int_t i)	{return fEXO_GOCCE_E_Status.at(i);}
   
   // GOCCE / Time
   UShort_t	GetGOCCETMult()			{return fEXO_GOCCE_T_Clover.size();}
   UShort_t	GetGOCCETClover(Int_t i)	{return fEXO_GOCCE_T_Clover.at(i);}
   UShort_t	GetGOCCETCristal(Int_t i)	{return fEXO_GOCCE_T_Cristal.at(i);}
   UShort_t	GetGOCCETSegment(Int_t i)	{return fEXO_GOCCE_T_Segment.at(i);}
   UShort_t	GetGOCCETTime(Int_t i)		{return fEXO_GOCCE_T_Time.at(i);}
   
    
   // ESS / TimeQ
   UShort_t	GetESSTQMult()		  {return fEXO_ESS_Clover.size();}
   UShort_t	GetESSTQClover(Int_t i)   {return fEXO_ESS_Clover.at(i);}
   UShort_t	GetESSTQCristal(Int_t i)  {return fEXO_ESS_Cristal.at(i);}
   UShort_t	GetESSTQBGO(Int_t i)     {return fEXO_ESS_BGO.at(i);}
   UShort_t	GetESSTQCSI(Int_t i)     {return fEXO_ESS_CSI.at(i);}

  void SetfTimeStamps(unsigned long long T, int i){
 	if(i<100){ 	
		fTimeStampsExogam2[i] = T; 
	}
	else printf("TS Error push in EXOGAM Data \n");
 }

   ClassDef(TExogam2Data,3)  // Exogam2Data structure
};

#endif
