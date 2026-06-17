#include <iostream>
using namespace std;

#include "TExogam2Data.h"


ClassImp(TExogam2Data)

TExogam2Data::TExogam2Data()
{
   // Default constructor

   // ECC / E
   fEXO_ECC_E_Clover.clear();
   fEXO_ECC_E_Cristal.clear();
   fEXO_ECC_E_Energy.clear();
   fEXO_ECC_E_DetNbr.clear();
   //PSA
   fEXO_ECC_E_T30.clear();
   fEXO_ECC_E_T60.clear();
   fEXO_ECC_E_T90.clear();

   // ECC / T
   fEXO_ECC_T_Clover.clear();
   fEXO_ECC_T_Cristal.clear();
   fEXO_ECC_T_Time.clear();
   fEXO_ECC_T_TSRequest.clear();

   // GOCCE / E
   fEXO_GOCCE_E_Clover.clear();
   fEXO_GOCCE_E_Cristal.clear();
   fEXO_GOCCE_E_Segment.clear();
   fEXO_GOCCE_E_Energy.clear();
   fEXO_GOCCE_E_Status.clear();
   // GOCCE / T
   fEXO_GOCCE_T_Clover.clear();
   fEXO_GOCCE_T_Cristal.clear();
   fEXO_GOCCE_T_Segment.clear();
   fEXO_GOCCE_T_Time.clear();
   
   //AC 
   fEXO_ESS_Clover.clear();
   fEXO_ESS_Cristal.clear();
   fEXO_ESS_BGO.clear();
   fEXO_ESS_CSI.clear();
   
   //Analyzed
   //addback
   aEXO_GammaEnergy.clear();
   aEXO_GammaTheta.clear();
   aEXO_GammaPhi.clear();
   aEXO_GammaCoreId.clear();
   aEXO_GammaX.clear();
   aEXO_GammaY.clear();
   aEXO_GammaZ.clear();
   aEXO_GammaX_Core.clear();
   aEXO_GammaY_Core.clear();
   aEXO_GammaZ_Core.clear();
   
   
      //no addback
   sEXO_GammaEnergy.clear();
   sEXO_GammaTheta.clear();
   sEXO_GammaPhi.clear();
   sEXO_GammaCoreId.clear();
   sEXO_GammaX.clear();
   sEXO_GammaY.clear();
   sEXO_GammaZ.clear();
   sEXO_GammaX_Core.clear();
   sEXO_GammaY_Core.clear();
   sEXO_GammaZ_Core.clear();
   
   fEXO_Neutron_NRJ.clear();
   fDeltaT.clear();
   fNeutronTOF.clear();
   fTime.clear();

   f_E877_Clover.clear();
   f_E877_Clover_E.clear();
   f_E877_Clover_T.clear();
   f_E877_Clover_BGO.clear();
   f_E877_Clover_CSI.clear();
   

   for(Int_t i=0;i<100;i++)fTimeStampsExogam2[i]= 0; //
}



TExogam2Data::~TExogam2Data()
{
}



void TExogam2Data::Clear()
{
   // ECC / E
   fEXO_ECC_E_Clover.clear();
   fEXO_ECC_E_Cristal.clear();
   fEXO_ECC_E_Energy.clear();   
   fEXO_ECC_E_DetNbr.clear();

   // ECC / T
   fEXO_ECC_T_Clover.clear();
   fEXO_ECC_T_Cristal.clear();
   fEXO_ECC_T_Time.clear(); 
   fEXO_ECC_T_TSRequest.clear(); 
   fEXO_ECC_E_T30.clear();
   fEXO_ECC_E_T60.clear();
   fEXO_ECC_E_T90.clear();

   // GOCCE / E
   fEXO_GOCCE_E_Clover.clear();
   fEXO_GOCCE_E_Cristal.clear();
   fEXO_GOCCE_E_Segment.clear();
   fEXO_GOCCE_E_Energy.clear();
   fEXO_GOCCE_E_Status.clear();
   // GOCCE / T
   fEXO_GOCCE_T_Clover.clear();
   fEXO_GOCCE_T_Cristal.clear();
   fEXO_GOCCE_T_Segment.clear();
   fEXO_GOCCE_T_Time.clear();

   fEXO_ESS_Clover.clear();
   fEXO_ESS_Cristal.clear();
   fEXO_ESS_BGO.clear();
   fEXO_ESS_CSI.clear();
   
   //Analyzed
   aEXO_GammaEnergy.clear();
   aEXO_GammaTheta.clear();
   aEXO_GammaPhi.clear();
   aEXO_GammaCoreId.clear();
   aEXO_GammaX.clear();
   aEXO_GammaY.clear();
   aEXO_GammaZ.clear();
   aEXO_GammaX_Core.clear();
   aEXO_GammaY_Core.clear();
   aEXO_GammaZ_Core.clear(); 
   
   //Anaalyzed no addback
   sEXO_GammaEnergy.clear();
   sEXO_GammaTheta.clear();
   sEXO_GammaPhi.clear();
   sEXO_GammaCoreId.clear();
   sEXO_GammaX.clear();
   sEXO_GammaY.clear();
   sEXO_GammaZ.clear();
   sEXO_GammaX_Core.clear();
   sEXO_GammaY_Core.clear();
   sEXO_GammaZ_Core.clear();
   
   fEXO_Neutron_NRJ.clear();
   fDeltaT.clear();
   fNeutronTOF.clear();
   fTime.clear();

   f_E877_Clover.clear();
   f_E877_Clover_E.clear();
   f_E877_Clover_T.clear();
   f_E877_Clover_BGO.clear();
   f_E877_Clover_CSI.clear();
   
   for(Int_t i=0;i<100;i++)fTimeStampsExogam2[i]= 0; //

}



void TExogam2Data::Dump()
{
   cout << "XXXXXXXXXXXXXXXXXXXXXXXX New Event XXXXXXXXXXXXXXXXX" << endl;

   // ECC
   // Energy
   cout << "EXO_ECC_MultE = " << fEXO_ECC_E_Clover.size() << endl;
   for (UShort_t i = 0; i < fEXO_ECC_E_Clover.size(); i++) {
      cout << "CloverE: " << fEXO_ECC_E_Clover[i] << " CristalE: " << fEXO_ECC_E_Cristal[i] << " Energy: " << fEXO_ECC_E_Energy[i] << endl;
   }
   // Time
   cout << "EXO_ECC_MultT = " << fEXO_ECC_T_Clover.size() << endl;
   for (UShort_t i = 0; i < fEXO_ECC_T_Clover.size(); i++) {
      cout << "CloverT: " << fEXO_ECC_T_Clover[i] << " CristalT: " << fEXO_ECC_T_Cristal[i] << " Time: " << fEXO_ECC_T_Time[i] << endl;
   }
   cout << "DeltaT(ns) mult = " << fDeltaT.size() << " NeutronTOF(ns) mult = " << fNeutronTOF.size() << " Time(ns) mult = " << fTime.size() << endl;
   cout << "E877 Clover addback mult = " << f_E877_Clover.size() << endl;
   for (UShort_t i = 0; i < f_E877_Clover.size(); i++) {
      cout << "E877 Clover: " << f_E877_Clover[i] << " Energy: " << f_E877_Clover_E[i] << " Time: " << f_E877_Clover_T[i] << " BGO: " << f_E877_Clover_BGO[i] << " CSI: " << f_E877_Clover_CSI[i] << endl;
   }

   // GOCCE
   // Energy
   cout << "EXO_GOCCE_MultE = " << fEXO_GOCCE_E_Clover.size() << endl;
   for (UShort_t i = 0; i < fEXO_GOCCE_E_Clover.size(); i++) {
      cout << "CloverE: " << fEXO_GOCCE_E_Clover[i] << " CristalE: " << fEXO_GOCCE_E_Cristal[i] << " SegmentE: " << fEXO_GOCCE_E_Segment[i] << " Energy: " << fEXO_GOCCE_E_Energy[i] << endl;
   }
   // Time
   cout << "EXO_GOCCE_MultT = " << fEXO_GOCCE_T_Clover.size() << endl;
   for (UShort_t i = 0; i < fEXO_GOCCE_T_Clover.size(); i++) {
      cout << "CloverT: " << fEXO_GOCCE_T_Clover[i] << " CristalT: " << fEXO_GOCCE_T_Cristal[i] << " SegmentT: " << fEXO_GOCCE_T_Segment[i] << " Time: " << fEXO_GOCCE_T_Time[i] << endl;
   }

  
}
