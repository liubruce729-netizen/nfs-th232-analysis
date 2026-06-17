#include <iostream>
using namespace std;

#include "TExogamData.h"


ClassImp(TExogamData)

TExogamData::TExogamData()
{
   // Default constructor

   // ECC / E
   fEXO_ECC_E_Clover.clear();
   fEXO_ECC_E_Cristal.clear();
   fEXO_ECC_E_Energy.clear();
   // ECC / T
   fEXO_ECC_T_Clover.clear();
   fEXO_ECC_T_Cristal.clear();
   fEXO_ECC_T_Time.clear();
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
   
   
   fEXO_ESS_E_Clover.clear();
   fEXO_ESS_E_Energy.clear();

   fEXO_ESS_T_Clover.clear();
   fEXO_ESS_T_Time.clear();

   fEXO_ESS_TQ_Clover.clear();
   fEXO_ESS_TQ_Cristal.clear();
   fEXO_ESS_TQ_Time.clear();
   
   
   

}



TExogamData::~TExogamData()
{
}



void TExogamData::Clear()
{
   // ECC / E
   fEXO_ECC_E_Clover.clear();
   fEXO_ECC_E_Cristal.clear();
   fEXO_ECC_E_Energy.clear();
   // ECC / T
   fEXO_ECC_T_Clover.clear();
   fEXO_ECC_T_Cristal.clear();
   fEXO_ECC_T_Time.clear();
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
   
   fEXO_ESS_E_Clover.clear();
   fEXO_ESS_E_Energy.clear();

   fEXO_ESS_T_Clover.clear();
   fEXO_ESS_T_Time.clear();

   fEXO_ESS_TQ_Clover.clear();
   fEXO_ESS_TQ_Cristal.clear();
   fEXO_ESS_TQ_Time.clear();
   
   
  
  
  
  
   
}



void TExogamData::Dump()
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
