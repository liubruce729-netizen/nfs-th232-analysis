#include <iostream>
using namespace std;

#include "TExogam2READata.h"


ClassImp(TExogam2READata)

TExogam2READata::TExogam2READata()
{
   // Default constructor

   // ECC / E
   fEXOrea_ECC_E_Clover.clear();
   fEXOrea_ECC_E_Cristal.clear();
   fEXOrea_ECC_E_Energy.clear();
   fEXOrea_ECC_E_DetNbr.clear();
   //PSA
   fEXOrea_ECC_E_T30.clear();
   fEXOrea_ECC_E_T60.clear();
   fEXOrea_ECC_E_T90.clear();

   // ECC / T
   fEXOrea_ECC_T_Clover.clear();
   fEXOrea_ECC_T_Cristal.clear();
   fEXOrea_ECC_T_Time.clear();

   // GOCCE / E
   fEXOrea_GOCCE_E_Clover.clear();
   fEXOrea_GOCCE_E_Cristal.clear();
   fEXOrea_GOCCE_E_Segment.clear();
   fEXOrea_GOCCE_E_Energy.clear();
   fEXOrea_GOCCE_E_Status.clear();
   // GOCCE / T
   fEXOrea_GOCCE_T_Clover.clear();
   fEXOrea_GOCCE_T_Cristal.clear();
   fEXOrea_GOCCE_T_Segment.clear();
   fEXOrea_GOCCE_T_Time.clear();
   
   fEXOrea_ESSBGO_Clover.clear();
   fEXOrea_ESSBGO_Cristal.clear();
   fEXOrea_ESS_BGOE.clear();
   fEXOrea_ESS_BGOT.clear();


   fEXOrea_ESSCSI_Clover.clear();
   fEXOrea_ESSCSI_Cristal.clear();
   fEXOrea_ESS_CSIE.clear();
   fEXOrea_ESS_CSIT.clear();
   //Analyzed
   //addback

aEXOrea_GammaEnergy.clear();
aEXOrea_GammaTheta.clear();
aEXOrea_GammaPhi.clear();
aEXOrea_GammaCoreId.clear(); 


   fTimeStampsExogam2rea_Segment.clear();
   fTimeStampsExogam2rea_BGO.clear();
   fTimeStampsExogam2rea_CSI.clear(); 
   
   for(Int_t i=0;i<100;i++)fTimeStampsExogam2rea[i]= 0; //
}



TExogam2READata::~TExogam2READata()
{
}



void TExogam2READata::Clear()
{
   // ECC / E
   fEXOrea_ECC_E_Clover.clear();
   fEXOrea_ECC_E_Cristal.clear();
   fEXOrea_ECC_E_Energy.clear();   
   fEXOrea_ECC_E_DetNbr.clear();

   // ECC / T
   fEXOrea_ECC_T_Clover.clear();
   fEXOrea_ECC_T_Cristal.clear();
   fEXOrea_ECC_T_Time.clear(); 
   fEXOrea_ECC_E_T30.clear();
   fEXOrea_ECC_E_T60.clear();
   fEXOrea_ECC_E_T90.clear();

   // GOCCE / E
   fEXOrea_GOCCE_E_Clover.clear();
   fEXOrea_GOCCE_E_Cristal.clear();
   fEXOrea_GOCCE_E_Segment.clear();
   fEXOrea_GOCCE_E_Energy.clear();
   fEXOrea_GOCCE_E_Status.clear();
   // GOCCE / T
   fEXOrea_GOCCE_T_Clover.clear();
   fEXOrea_GOCCE_T_Cristal.clear();
   fEXOrea_GOCCE_T_Segment.clear();
   fEXOrea_GOCCE_T_Time.clear();


   
   fEXOrea_ESSBGO_Clover.clear();
   fEXOrea_ESSBGO_Cristal.clear();
   fEXOrea_ESS_BGOE.clear();
   fEXOrea_ESS_BGOT.clear();


   fEXOrea_ESSCSI_Clover.clear();
   fEXOrea_ESSCSI_Cristal.clear();
   fEXOrea_ESS_CSIE.clear();
   fEXOrea_ESS_CSIT.clear();
   for(Int_t i=0;i<100;i++)fTimeStampsExogam2rea[i]= 0; //
   fTimeStampsExogam2rea_Segment.clear();
   fTimeStampsExogam2rea_BGO.clear();
   fTimeStampsExogam2rea_CSI.clear(); 
   
   
   aEXOrea_GammaEnergy.clear();
   aEXOrea_GammaTheta.clear();
   aEXOrea_GammaPhi.clear();
   aEXOrea_GammaCoreId.clear(); 

}
	

void TExogam2READata::Dump()
{
   cout << "XXXXXXXXXXXXXXXXXXXXXXXX New Event XXXXXXXXXXXXXXXXX" << endl;

   // ECC
   // Energy
   cout << "EXO_ECC_MultE = " << fEXOrea_ECC_E_Clover.size() << endl;
   for (UShort_t i = 0; i < fEXOrea_ECC_E_Clover.size(); i++) {
      cout << "CloverE: " << fEXOrea_ECC_E_Clover[i] << " CristalE: " << fEXOrea_ECC_E_Cristal[i] << " Energy: " << fEXOrea_ECC_E_Energy[i] << endl;
   }
   // Time
   cout << "EXO_ECC_MultT = " << fEXOrea_ECC_T_Clover.size() << endl;
   for (UShort_t i = 0; i < fEXOrea_ECC_T_Clover.size(); i++) {
      cout << "CloverT: " << fEXOrea_ECC_T_Clover[i] << " CristalT: " << fEXOrea_ECC_T_Cristal[i] << " Time: " << fEXOrea_ECC_T_Time[i] << endl;
   }
   // GOCCE
   // Energy
   cout << "EXO_GOCCE_MultE = " << fEXOrea_GOCCE_E_Clover.size() << endl;
   for (UShort_t i = 0; i < fEXOrea_GOCCE_E_Clover.size(); i++) {
      cout << "CloverE: " << fEXOrea_GOCCE_E_Clover[i] << " CristalE: " << fEXOrea_GOCCE_E_Cristal[i] << " SegmentE: " << fEXOrea_GOCCE_E_Segment[i] << " Energy: " << fEXOrea_GOCCE_E_Energy[i] << endl;
   }
   // Time
   cout << "EXO_GOCCE_MultT = " << fEXOrea_GOCCE_T_Clover.size() << endl;
   for (UShort_t i = 0; i < fEXOrea_GOCCE_T_Clover.size(); i++) {
      cout << "CloverT: " << fEXOrea_GOCCE_T_Clover[i] << " CristalT: " << fEXOrea_GOCCE_T_Cristal[i] << " SegmentT: " << fEXOrea_GOCCE_T_Segment[i] << " Time: " << fEXOrea_GOCCE_T_Time[i] << endl;
   }

  
}
