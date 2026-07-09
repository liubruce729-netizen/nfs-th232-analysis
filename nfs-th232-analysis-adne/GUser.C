// File : GUser.C
// Author:Emmanuel Cl�ment
//////////////////////////////////////////////////////////////////////////////
//
// Class GUser
//
// Class for User treatment
//
////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/



#include "TROOT.h"
#include <TProfile.h> 
#include <TRandom.h>
#include <GParaCaliXml.h>
#include <TRint.h>
#include <TObject.h>
#include <TString.h>
#include <TH1.h>
#include "TMath.h"
#include "GEventMFM.h"
#include "fstream"
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <string>
#include "GUser.h"
//_________________________________global_variables______________________________

namespace {
const UShort_t kNfsVetoCloverMultiplicityMin = 2;

void AppendNfsCrystalLabels(std::vector<TString> &labels, const TString &rawList)
{
  // EN: Accept either YAML lists or one comma-separated scalar string.
  // CN: 支持 YAML 列表，也支持一个逗号分隔的字符串。
  std::stringstream stream(rawList.Data());
  std::string item;
  while(std::getline(stream,item,',')){
    TString label(item.c_str());
    label.ReplaceAll(" ","");
    label.ReplaceAll("	","");
    if(label.Length()>0)labels.push_back(label);
  }
}

bool ParseNfsCrystalLabel(const TString &label, Int_t &clover, Int_t &crystal)
{
  TString normalized(label);
  normalized.ReplaceAll(" ","");
  normalized.ReplaceAll("	","");
  normalized.ReplaceAll(":","-");
  normalized.ReplaceAll("_","-");
  normalized.ReplaceAll("/","-");
  clover=-1;
  crystal=-1;
  if(std::sscanf(normalized.Data(),"%d-%d",&clover,&crystal)!=2)return false;
  return clover>=0 && clover<16 && crystal>=0 && crystal<4;
}
}



//______________________________________________________________________________

ClassImp (GUser);

GUser::GUser (GDevice* _fDevIn, GDevice* _fDevOut)
{ 
  // Constructor/initialisator of Acquisition object Cal
  //
  // entry:
  // - Input Device
  // - Output Device  
  
  
  
  fDevIn         = _fDevIn;
  fDevOut        = _fDevOut;

  pMergeFrame=   new MFMMergeFrame();
  pExogamFrame=  new MFMExogamFrame();
  pCommonFrame=  new MFMCommonFrame();
  
  fInsideframe=  new MFMCommonFrame();
  for(Int_t i = 0 ; i < 100; i++) pInceptionMergeFrame[i] = new MFMMergeFrame();
  for(Int_t i = 0 ; i < 100; i++) fInceptionInsideframe[i] = new MFMCommonFrame();
  InceptionLayer=0;
  
  pEbyedatFrame= new MFMEbyedatFrame();
  pNedaFrame=    new MFMNedaFrame();
  pNedaCompFrame=new MFMNedaCompFrame();
  pDiamantFrame= new MFMDiamantFrame();
  pParisFrame=   new MFMParisFrame();
  pGenericFrame= new MFMReaGenericFrame();
  pVamosICFrame= new MFMVamosICFrame();
  
  YAML::Node config = YAML::LoadFile("Yaml_config_files/config.yaml");
  fNfsExoAnaTree = false;
  fNfsExoAnaSpec = false;
  fNfsExoAnaRawTree = false;
  fNfsExoAnaCrystalTimeCorrection = false;
  fNfsExoAnaCorrectionPath = "";
  fNfsExoAnaDisabledCrystals.clear();
  YAML::Node nfsExoAna = config["nfs_exo_ana"];
  if (nfsExoAna) {
    if (nfsExoAna["tree"]) fNfsExoAnaTree = nfsExoAna["tree"].as<bool>();
    if (nfsExoAna["spec"]) fNfsExoAnaSpec = nfsExoAna["spec"].as<bool>();
    if (nfsExoAna["raw_tree"]) fNfsExoAnaRawTree = nfsExoAna["raw_tree"].as<bool>();
    if (nfsExoAna["crystal_time_correction"]) fNfsExoAnaCrystalTimeCorrection = nfsExoAna["crystal_time_correction"].as<bool>();
    if (nfsExoAna["correction_path"]) fNfsExoAnaCorrectionPath = nfsExoAna["correction_path"].as<std::string>().c_str();
    if (nfsExoAna["disabled_crystals"]) {
      YAML::Node disabledCrystals = nfsExoAna["disabled_crystals"];
      if(disabledCrystals.IsSequence()){
        for(std::size_t i=0;i<disabledCrystals.size();i++){
          if(disabledCrystals[i].IsSequence() && disabledCrystals[i].size()==2){
            fNfsExoAnaDisabledCrystals.push_back(TString::Format("%d-%d",disabledCrystals[i][0].as<int>(),disabledCrystals[i][1].as<int>()));
          }
          else if(disabledCrystals[i].IsScalar()){
            AppendNfsCrystalLabels(fNfsExoAnaDisabledCrystals,disabledCrystals[i].as<std::string>().c_str());
          }
        }
      }
      else if(disabledCrystals.IsScalar()){
        AppendNfsCrystalLabels(fNfsExoAnaDisabledCrystals,disabledCrystals.as<std::string>().c_str());
      }
    }
  }
  cout << "NFS Exogam analysis switches: tree=" << (fNfsExoAnaTree ? "true" : "false")
       << " spec=" << (fNfsExoAnaSpec ? "true" : "false")
       << " raw_tree=" << (fNfsExoAnaRawTree ? "true" : "false")
       << " crystal_time_correction=" << (fNfsExoAnaCrystalTimeCorrection ? "true" : "false")
       << " correction_path=" << fNfsExoAnaCorrectionPath
       << " disabled_crystals=" << fNfsExoAnaDisabledCrystals.size() << endl;
  bool spec_exogam2	=	config["Guser"]["spec_exogam2"].as<bool>();
  bool spec_trigger	=	config["Guser"]["spec_trigger"].as<bool>();
  bool spec_MW		=	config["Guser"]["spec_MW"].as<bool>();
  bool spec_CsI		=	config["Guser"]["spec_CsI"].as<bool>();
  bool spec_Neda	=	config["Guser"]["spec_Neda"].as<bool>();
  bool spec_Paris	=	config["Guser"]["spec_Paris"].as<bool>();
  bool spec_Generic	=	config["Guser"]["spec_Generic"].as<bool>();
  bool spec_EbyE	=	config["Guser"]["spec_EbyE"].as<bool>();
  bool spec_exogam2REA	=	config["Guser"]["spec_exogam2REA"].as<bool>();
  bool spec_VamosIC	=	config["Guser"]["spec_VamosIC"].as<bool>();
  debug			= 	config["Guser"]["debug_flag"].as<bool>();
  
  
  printf("\033[33mInfo:: Start constructors for each Class \033[m \n");
  fExogam2	  = new TExogam2(spec_exogam2);
  cout<<"fExogam2 const. done"<<endl;
  fExogam2REA	  = new TExogam2REA(spec_exogam2REA);
  cout<<"fExogam2 by **REA const. done"<<endl;
  fTrigger	  = new TTrigger(spec_trigger);
  cout<<"fTrigger const. done"<<endl;
  //fMW	  	  = new TMW(true);
  fCsI	  	  = new TCsI(spec_CsI);
  cout<<"fCsI const. done"<<endl;
  fNeda	  	  = new TNWall(spec_Neda);
  cout<<"fNeda const. done"<<endl;
  fParis	  = new TParis(spec_Paris);
  cout<<"fParis const. done"<<endl;
  fGeneric        = new TGeneric(spec_Generic,0); 
  cout<<"fGeneric const. done"<<endl;
  fEbyE	  	  = new TEbyE(spec_EbyE);
  cout<<"fEbyE const. done "<<endl;
  //fGeneric[0]        = new TGeneric(true,0);
  fVamosIC	 = new TVamosIC(spec_VamosIC);
  cout<<"fVamosIC const. done "<<endl;
  printf("\033[33mInfo:: All Done \033[m \n");
  cout << "" << endl;
  
  LocalTree = NULL;
  RawTree = NULL;
  NfsTree = NULL;
  fileNfsTree = NULL;
  NfsMult3Tree = NULL;
  fileNfsMult3Tree = NULL;
  TreeFillBool=false;
  RawTreeFillBool=false;
  NfsTreeFillBool=false;
  NfsMult3TreeFillBool=false;
  MakeTreeOnly=false;
  SpyOnly=false;
  fRawEventCounter=0;
  bool controlYalm;
   
  if(controlYalm=config["Guser"]["exogam2"]["clover0"].as<bool>()) fExogam2->ActivateClover(0); //ecc#// from 2022 and numexo2 eccId==flangeId
  if(controlYalm=config["Guser"]["exogam2"]["clover1"].as<bool>()) fExogam2->ActivateClover(1);
  if(controlYalm=config["Guser"]["exogam2"]["clover2"].as<bool>()) fExogam2->ActivateClover(2);
  if(controlYalm=config["Guser"]["exogam2"]["clover3"].as<bool>()) fExogam2->ActivateClover(3);
  if(controlYalm=config["Guser"]["exogam2"]["clover4"].as<bool>()) fExogam2->ActivateClover(4);
  if(controlYalm=config["Guser"]["exogam2"]["clover5"].as<bool>()) fExogam2->ActivateClover(5);
  if(controlYalm=config["Guser"]["exogam2"]["clover6"].as<bool>()) fExogam2->ActivateClover(6);
  if(controlYalm=config["Guser"]["exogam2"]["clover7"].as<bool>()) fExogam2->ActivateClover(7);
  if(controlYalm=config["Guser"]["exogam2"]["clover8"].as<bool>()) fExogam2->ActivateClover(8);
  if(controlYalm=config["Guser"]["exogam2"]["clover9"].as<bool>()) fExogam2->ActivateClover(9);
  if(controlYalm=config["Guser"]["exogam2"]["clover10"].as<bool>())fExogam2->ActivateClover(10); 
  if(controlYalm=config["Guser"]["exogam2"]["clover11"].as<bool>())fExogam2->ActivateClover(11);
  if(controlYalm=config["Guser"]["exogam2"]["clover12"].as<bool>())fExogam2->ActivateClover(12);
  if(controlYalm=config["Guser"]["exogam2"]["clover13"].as<bool>())fExogam2->ActivateClover(13);
  if(controlYalm=config["Guser"]["exogam2"]["clover14"].as<bool>())fExogam2->ActivateClover(14);
  if(controlYalm=config["Guser"]["exogam2"]["clover15"].as<bool>())fExogam2->ActivateClover(15);
  
  auto xyzNode = config["Guser"]["exogam2"]["SetTargetPosition"];
  std::array<double, 3> xyz;
  
  if (!xyzNode || !xyzNode.IsSequence() || xyzNode.size() != 3) {
        std::cerr << "Invalid xyz field (expected 3 values)" << std::endl;
	xyz[0]=xyz[1]=xyz[2]=0.;// (x,y,z) mm
   }
    
   for (std::size_t i = 0; i < 3; ++i) {
        xyz[i] = xyzNode[i].as<double>();
   }
  if(config["Guser"]["exogam2"]["broken_seg"].as<bool>()){cout<<"Broken Segment correction activated"<<endl;}
  fExogam2->SetTargetPosition(xyz[0],xyz[1],xyz[2]); //mm
  fExogam2->SetCloverPosition(0,0,config["Guser"]["exogam2"]["clover0_Distance"].as<float>()); //ecc# flange# dis[mm] // from 2022 and numexo2 eccId==flangeId
  fExogam2->SetCloverPosition(1,1,config["Guser"]["exogam2"]["clover1_Distance"].as<float>());  //was 161 mm5
  fExogam2->SetCloverPosition(2,2,config["Guser"]["exogam2"]["clover2_Distance"].as<float>());
  fExogam2->SetCloverPosition(3,3,config["Guser"]["exogam2"]["clover3_Distance"].as<float>());
  fExogam2->SetCloverPosition(4,4,config["Guser"]["exogam2"]["clover4_Distance"].as<float>());
  fExogam2->SetCloverPosition(5,5,config["Guser"]["exogam2"]["clover5_Distance"].as<float>());
  fExogam2->SetCloverPosition(6,6,config["Guser"]["exogam2"]["clover6_Distance"].as<float>());
  fExogam2->SetCloverPosition(7,7,config["Guser"]["exogam2"]["clover7_Distance"].as<float>());
  fExogam2->SetCloverPosition(8,8,config["Guser"]["exogam2"]["clover8_Distance"].as<float>());
  fExogam2->SetCloverPosition(9,9,config["Guser"]["exogam2"]["clover9_Distance"].as<float>());
  fExogam2->SetCloverPosition(10,10,config["Guser"]["exogam2"]["clover10_Distance"].as<float>());
  fExogam2->SetCloverPosition(11,11,config["Guser"]["exogam2"]["clover11_Distance"].as<float>());
  fExogam2->SetCloverPosition(12,12,config["Guser"]["exogam2"]["clover12_Distance"].as<float>());
  fExogam2->SetCloverPosition(13,13,config["Guser"]["exogam2"]["clover13_Distance"].as<float>());
  fExogam2->SetCloverPosition(14,14,config["Guser"]["exogam2"]["clover14_Distance"].as<float>());
  fExogam2->SetCloverPosition(15,15,config["Guser"]["exogam2"]["clover15_Distance"].as<float>());
  
  
  //if more than 1 segment broken in a given crystal, the algo put the interaction on the last declared
   if(config["Guser"]["exogam2"]["broken_seg"].as<bool>()){
   	for (const auto& ms : config["exogam"]["missing_segments"]) {
		fExogam2->SetCloverBrokenSeg(ms[0].as<int>(),ms[1].as<int>(),ms[2].as<int>());
	}
   }

  fExogam2->ActivateGOCCE(config["Guser"]["exogam2"]["activate_gocce"].as<bool>()); //activate the data checking on GOCCE and analysis
  fExogam2->ActivateESS(config["Guser"]["exogam2"]["activate_ess"].as<bool>(),config["Guser"]["exogam2"]["ess_threshold"].as<int>()); //activate the  ESS and set Threshold
  fExogam2->ActivateGOCCETrack(config["Guser"]["exogam2"]["activate_gocce_track2"].as<bool>()); //activate a second level of checking for GOCCE
  fExogam2->SetPSA(config["Guser"]["exogam2"]["activate_psa"].as<bool>());

  auto promptgate_TDC = config["Guser"]["exogam2"]["SetPromptGate"];
  std::array<double, 2> pg;
  
  if (!promptgate_TDC || !promptgate_TDC.IsSequence() || promptgate_TDC.size() != 2) {
        std::cerr << "Invalid prompt field (expected 2 values)" << std::endl;
	pg[0]=pg[1]=0.;// 
   }
  else{
   	for (std::size_t i = 0; i < 2; ++i) {
        	pg[i] = promptgate_TDC[i].as<int>();
   	}
   }
  fExogam2->SetPromptGate(pg[0],pg[1]);
  fExogam2->SetMaxClovMul(config["Guser"]["exogam2"]["SetMaxClovMul"].as<int>());
  fExogam2->SetBeta(config["Guser"]["exogam2"]["SetBeta"].as<float>());
  fExogam2->SetNFS_Parameter(config["Guser"]["exogam2"]["nfs_distance"].as<float>(), config["Guser"]["exogam2"]["nfs_gammaFlashOffset"].as<float>(), config["Guser"]["exogam2"]["nfs_gammaG"].as<float>(),config["Guser"]["exogam2"]["nfs_neutron"].as<bool>());
  fExogam2->SetNfsCrystalTimeCorrection(fNfsExoAnaCrystalTimeCorrection, fNfsExoAnaCorrectionPath);
  for(std::size_t i=0;i<fNfsExoAnaDisabledCrystals.size();i++){
    Int_t disabledClover=-1;
    Int_t disabledCrystal=-1;
    if(ParseNfsCrystalLabel(fNfsExoAnaDisabledCrystals[i],disabledClover,disabledCrystal)){
      fExogam2->SetNfsCrystalEnabled(disabledClover,disabledCrystal,false);
    }
    else{
      cerr << "NFS Exogam Warning :: ignored invalid disabled_crystals entry " << fNfsExoAnaDisabledCrystals[i] << "; expected clover-crystal, e.g. 7-1" << endl;
    }
  }
  fExogam2->SetNFSAnaSpec(fNfsExoAnaSpec);
 
//------------------------REA -------------------------------------
  if(controlYalm=config["Guser"]["exogam2REA"]["clover0"].as<bool>()) fExogam2REA->ActivateClover(0); //ecc#// from 2022 and numexo2 eccId==flangeId
  if(controlYalm=config["Guser"]["exogam2REA"]["clover1"].as<bool>()) fExogam2REA->ActivateClover(1);
  if(controlYalm=config["Guser"]["exogam2REA"]["clover2"].as<bool>()) fExogam2REA->ActivateClover(2);
  if(controlYalm=config["Guser"]["exogam2REA"]["clover3"].as<bool>()) fExogam2REA->ActivateClover(3);
  if(controlYalm=config["Guser"]["exogam2REA"]["clover4"].as<bool>()) fExogam2REA->ActivateClover(4);
  if(controlYalm=config["Guser"]["exogam2REA"]["clover5"].as<bool>()) fExogam2REA->ActivateClover(5);
  if(controlYalm=config["Guser"]["exogam2REA"]["clover6"].as<bool>()) fExogam2REA->ActivateClover(6);
  if(controlYalm=config["Guser"]["exogam2REA"]["clover7"].as<bool>()) fExogam2REA->ActivateClover(7);
  if(controlYalm=config["Guser"]["exogam2REA"]["clover8"].as<bool>()) fExogam2REA->ActivateClover(8);
  if(controlYalm=config["Guser"]["exogam2REA"]["clover9"].as<bool>()) fExogam2REA->ActivateClover(9);
  if(controlYalm=config["Guser"]["exogam2REA"]["clover10"].as<bool>())fExogam2REA->ActivateClover(10); 
  if(controlYalm=config["Guser"]["exogam2REA"]["clover11"].as<bool>())fExogam2REA->ActivateClover(11);
  if(controlYalm=config["Guser"]["exogam2REA"]["clover12"].as<bool>())fExogam2REA->ActivateClover(12);
  if(controlYalm=config["Guser"]["exogam2REA"]["clover13"].as<bool>())fExogam2REA->ActivateClover(13);
  if(controlYalm=config["Guser"]["exogam2REA"]["clover14"].as<bool>())fExogam2REA->ActivateClover(14);
  if(controlYalm=config["Guser"]["exogam2REA"]["clover15"].as<bool>())fExogam2REA->ActivateClover(15);
  
  fExogam2REA->SetCloverPosition(0,0,config["Guser"]["exogam2REA"]["clover0_Distance"].as<float>()); //ecc# flange# dis[mm] // from 2022 and numexo2 eccId==flangeId
  fExogam2REA->SetCloverPosition(1,1,config["Guser"]["exogam2REA"]["clover1_Distance"].as<float>());  //was 161 mm5
  fExogam2REA->SetCloverPosition(2,2,config["Guser"]["exogam2REA"]["clover2_Distance"].as<float>());
  fExogam2REA->SetCloverPosition(3,3,config["Guser"]["exogam2REA"]["clover3_Distance"].as<float>());
  fExogam2REA->SetCloverPosition(4,4,config["Guser"]["exogam2REA"]["clover4_Distance"].as<float>());
  fExogam2REA->SetCloverPosition(5,5,config["Guser"]["exogam2REA"]["clover5_Distance"].as<float>());
  fExogam2REA->SetCloverPosition(6,6,config["Guser"]["exogam2REA"]["clover6_Distance"].as<float>());
  fExogam2REA->SetCloverPosition(7,7,config["Guser"]["exogam2REA"]["clover7_Distance"].as<float>());
  fExogam2REA->SetCloverPosition(8,8,config["Guser"]["exogam2REA"]["clover8_Distance"].as<float>());
  fExogam2REA->SetCloverPosition(9,9,config["Guser"]["exogam2REA"]["clover9_Distance"].as<float>());
  fExogam2REA->SetCloverPosition(10,10,config["Guser"]["exogam2REA"]["clover10_Distance"].as<float>());
  fExogam2REA->SetCloverPosition(11,11,config["Guser"]["exogam2REA"]["clover11_Distance"].as<float>());
  fExogam2REA->SetCloverPosition(12,12,config["Guser"]["exogam2REA"]["clover11_Distance"].as<float>());
  fExogam2REA->SetCloverPosition(13,13,config["Guser"]["exogam2REA"]["clover12_Distance"].as<float>());
  
 
  fExogam2REA->ActivateGOCCE(config["Guser"]["exogam2REA"]["activate_gocce"].as<bool>()); //activate the data checking on GOCCE and analysis
  fExogam2REA->ActivateESS(config["Guser"]["exogam2REA"]["activate_ess"].as<bool>(),config["Guser"]["exogam2REA"]["ess_threshold"].as<int>()); //activate the data checking on ESS and set Threshold
  fExogam2REA->ActivateGOCCETrack(config["Guser"]["exogam2REA"]["activate_gocce_track2"].as<bool>()); //activate a second level of checking for GOCCE
//source data
 auto promptgate_TDCREA = config["Guser"]["exogam2REA"]["SetPromptGate"];
  std::array<double, 2> pgREA;
  
  if (!promptgate_TDCREA || !promptgate_TDCREA.IsSequence() || promptgate_TDCREA.size() != 2) {
        std::cerr << "Invalid REA prompt field (expected 2 values)" << std::endl;
	pgREA[0]=pgREA[1]=0.;//
  }
  else{  
   	for (std::size_t i = 0; i < 2; ++i) {
        	pgREA[i] = promptgate_TDCREA[i].as<int>();
   	}
  }

  fExogam2REA->SetPromptGate(pgREA[0],pgREA[1]); //applied for both Segment and core Time infos from a REA modules
  fExogam2REA->SetMaxClovMul(config["Guser"]["exogam2REA"]["SetMaxClovMul"].as<int>());
  fExogam2REA->SetBeta(config["Guser"]["exogam2REA"]["SetBeta"].as<float>());
    
  fParis->SetBeta(config["Guser"]["paris"]["SetBeta"].as<float>());
  
  fTrigger->SetMK2(false);
  fTrigger->SetAGAVA(false);
  
  PrevTS=0;
  TStart=0;
  /*lparams = new DataParameters();
  lparams->FillFromActionFile("Conf/ACTIONS_vamos_agata.CHC_PAR");
  */
  
  
  cout << "" << endl;
  cout << "" << endl;
 
  
printf("\033[33m--------------------------------\033[m\n");
printf("\033[33m   By E. Clement | Jan 2026    \033[m\n");
printf("\033[33m     MFM Compatible            \033[m\n");
printf("\033[32m      *           *             \033[m\n");
printf("\033[32m     ***       ***             \033[m\n");
printf("\033[32m      *           *             \033[m\n");
printf("\033[33m   EXOGAM2, NEDA, DIAMANT, PARIS \033[m\n");
printf("\033[33m   GENERIC REA, EBYEDAT, EXO2-REA\033[m\n");
printf("\033[33m--------------------------------\033[m\n");
printf("\033[33m      ___   _   _  _ ___ _      \033[m\n");
printf("\033[33m     / __| /_\\ | \\| |_ _| |     \033[m\n");
printf("\033[33m    | (_ |/ _ \\| .` || || |__   \033[m\n");
printf("\033[33m     \\___/_/ \\_\\_|\\_|___|____|  \033[m\n");

  
  
  cout << "" << endl;
  cout << "" << endl;
  
  
  MySpectraList= GetSpectra();
  cout << "Get Spectra Data Base Address done" << endl;

  
 
 
  cout<< "Class Constructor done"<<endl;

}
 
//_____________________________________________________________________________

GUser::~GUser()  {
  //Destructor of class GUser   
  
   

  delete   fExogam2;
  delete   fExogam2REA;
  delete   fTrigger;
  delete   fMW;
  delete   fCsI;
  delete   fNeda;
  delete   fParis;
  delete   fGeneric;
  delete   fEbyE;
  delete   fVamosIC;
 // delete   fGeneric[0];
  
  gROOT->cd();
}

//______________________________________________________________

void GUser::InitUser()
{
  // Initialisation for global  user treatement
 


	
  printf("\033[31mInfo::Start Spectra Loading \033[m \n");

  //-----------------------------------     Exogam Raw

  if(fExogam2->SpectraConstructor()){
	
    MySpectraList->AddSpectrum(fExogam2->fMyHistoNCriERaw,"Exogam/Exogam_2D");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoNCriTRaw,"Exogam/Exogam_2D");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoNCriECal,"Exogam/Exogam_2D");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoNCriTCal,"Exogam/Exogam_2D");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoNSegERaw,"Exogam/Exogam_2D");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPatternESSTQ,"Exogam/Exogam_Pattern");
    MySpectraList->AddSpectrum(fExogam2->BoardIdHist,"Exogam/Exogam_Pattern");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoSegSeg,"Exogam/Exogam_2D");
    
    
    for(int k=0 ; k<16 ; k++){
      if(fExogam2->IsCloverActive(k)){
	MySpectraList->AddSpectrum(fExogam2->fMyHistoCloverECal[k],"Exogam/Clover_ECal"); // No process
	MySpectraList->AddSpectrum(fExogam2->fMyHistoCloverECalAdd[k],"Exogam/Clover_ECalAdd");//Prompt AddBack /clover
	MySpectraList->AddSpectrum(fExogam2->fMyHistoCloverECalACAdd_TC[k],"Exogam/Clover_ECalACAdd_TC"); //AddBack + AC + prompt /clover
	MySpectraList->AddSpectrum(fExogam2->fMyHistoCloverECalACAdd_DC[k],"Exogam/Clover_ECalACAddDC_DC");//AddBack + AC  /clover + DopplerCorrected
	MySpectraList->AddSpectrum(fExogam2->fMyHistoCloverECalACAddRejectF[k],"Exogam/Clover_ECalACAddRejF");
			
	MySpectraList->AddSpectrum(fExogam2->fMyHistoCloverTCal[k],"Exogam/Clover_TCal");
			
	for(int l=1;l<=4;l++){
	  MySpectraList->AddSpectrum(fExogam2->fMyHistoECCECal[k*100+(l-1)*10],"Exogam/ECC_ECal");
	  MySpectraList->AddSpectrum(fExogam2->fMyHistoECCTCal[k*100+(l-1)*10],"Exogam/ECC_TCal");
	  MySpectraList->AddSpectrum(fExogam2->fMyHistoECCTRaw[k*100+(l-1)*10],"Exogam/ECC_TRaw");
	  MySpectraList->AddSpectrum(fExogam2->fMyHistoESS_BGO[k*100+(l-1)*10],"Exogam/ESS");
	  MySpectraList->AddSpectrum(fExogam2->fMyHistoESS_CSI[k*100+(l-1)*10],"Exogam/ESS");
	  MySpectraList->AddSpectrum(fExogam2->fMyHistoECCEESSE[k*100+(l-1)*10],"Exogam/ESS");
	  MySpectraList->AddSpectrum(fExogam2->fMyHistoECCEVetoed[k*100+(l-1)*10],"Exogam/CoreRejected");
	  MySpectraList->AddSpectrum(fExogam2->fMyHistoECCEAccepted[k*100+(l-1)*10],"Exogam/CoreAccepted");
	  MySpectraList->AddSpectrum(fExogam2->fMyHistoCrysECalACRejectF[k*100+(l-1)*10],"Exogam/Core_ECalAddRejF");
	 /* MySpectraList->AddSpectrum(fExogam2->fMyHistoECCET30[k][l-1],"Exogam/PSA");
	  MySpectraList->AddSpectrum(fExogam2->fMyHistoECCET60[k][l-1],"Exogam/PSA");
	  MySpectraList->AddSpectrum(fExogam2->fMyHistoECCET90[k][l-1],"Exogam/PSA");
	  MySpectraList->AddSpectrum(fExogam2->fMyHistoECCET30_60[k][l-1],"Exogam/PSA");
	  MySpectraList->AddSpectrum(fExogam2->fMyHistoECCET30_90[k][l-1],"Exogam/PSA");
	  MySpectraList->AddSpectrum(fExogam2->fMyHistoECCET60_90[k][l-1],"Exogam/PSA");
	*/		
			
	  for(int m=1; m<=4; m++){
	    MySpectraList->AddSpectrum(fExogam2->fMyHistoGOCCEE[k*100+10*(l-1)+(m-1)],"Exogam/GOCCE_E");
	    MySpectraList->AddSpectrum(fExogam2->fMyHistoGOCCET[k*100+10*(l-1)+(m-1)],"Exogam/GOCCE_T");
	  }
	}
      }
    }
    
     for(int k2=0 ; k2<16*4 ; k2++){
	for(int k3=0 ; k3<16*4 ; k3++){
		if(fExogam2->IsCloverActive(k2/4)&&fExogam2->IsCloverActive(k3/4)){
			//MySpectraList->AddSpectrum(fExogam2->fMyHistoTT[k2][k3],"Exogam/TT");
		}
	}
     }   
    MySpectraList->AddSpectrum(fExogam2->fMyHistoTTAll,"Exogam/TT");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoGOCCENet,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoGOCCEMirror,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoRMirror,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPhiMirror,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPSASurface,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPSASurfaceCarte,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->ShortTrace,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPSAChi2,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPSAChi2GRID,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPSAChi2_Pattern,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPSAChi2GRID_Pattern,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPSAChi2_Radius,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPSACore,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPSA_CFD,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPSA_CFD_E,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPSA_CFD_Pattern,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPSARadius_Radius,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPhiMirrorPattern,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPSAWorld,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoRPattern,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPSARejectedTraces,"Exogam/PSA") ;
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPSAAcceptedTraces,"Exogam/PSA") ;
   MySpectraList->AddSpectrum(fExogam2->fMyHistoPSAK,"Exogam/PSA") ;
   MySpectraList->AddSpectrum(fExogam2->fMyHistoPSAK_Pattern,"Exogam/PSA") ;
   MySpectraList->AddSpectrum(fExogam2->fMyHistoPSAL_Pattern,"Exogam/PSA") ;
   MySpectraList->AddSpectrum(fExogam2->fMyHistoPSAKL,"Exogam/PSA") ;
   MySpectraList->AddSpectrum(fExogam2->fMyHistoPSAKL_postGrid,"Exogam/PSA") ;
   MySpectraList->AddSpectrum(fExogam2->fMyHistoPSAKEgamma,"Exogam/PSA") ;
   
   
    MySpectraList->AddSpectrum(fExogam2->fMyHistoSumECCE,"Exogam/Sum"); //sumCore
    MySpectraList->AddSpectrum(fExogam2->fMyHistoSumECCT,"Exogam/Sum");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoSumECCE_ECCT,"Exogam/Sum");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoSumGOCCEE,"Exogam/Sum");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoSumExogam2,"Exogam/Sum"); //AddBack + AC rejected + prompt
    MySpectraList->AddSpectrum(fExogam2->fMyHistoSumExogam2DC,"Exogam/Sum");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoSumExogam2BeforeAfterDC,"Exogam/Sum");
    
     MySpectraList->AddSpectrum(fExogam2->fMyHistoSumExogam2DC_VTS,"Exogam/Sum");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoSumExogam2FoldCond,"Exogam/Sum");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoSumExogam2DCFoldCond,"Exogam/Sum");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoSumExogam22D,"Exogam/Sum");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoSumExogam2DC2D,"Exogam/Sum");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoGammaGamma,"Exogam/Sum");//AddBack + AC rejected
    MySpectraList->AddSpectrum(fExogam2->fMyHistoTTinGammaGamma,"Exogam/Sum");
    
    MySpectraList->AddSpectrum(fExogam2->fMyHistoGammaGammaCore,"Exogam/Sum");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoGammaGammaCoreDiag,"Exogam/Sum");
    MySpectraList->AddSpectrum(fExogam2->ECCTcorrelCheck,"Exogam/Sum");
    
    MySpectraList->AddSpectrum(fExogam2->fMyHistoSumExogam2Calorimeter,"Exogam/Sum");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoSumGOCCET,"Exogam/Sum");
    MySpectraList->AddSpectrum(fExogam2->TransverseMomentumGamma,"Exogam/Sum");
    
    
    for(int l=0;l<10;l++){
    	MySpectraList->AddSpectrum(fExogam2->fMyHistoSumExogam2DCNucId[l],"Exogam/IdSum");
    }
    
    MySpectraList->AddSpectrum(fExogam2->fMyHistoSumExogam2DCClover,"Exogam/Sum");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoSumExogam2Clover,"Exogam/Sum");
    
    
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPatternECCE,"Exogam/Exogam_Pattern");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPatternECCT,"Exogam/Exogam_Pattern");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPatternGOCCEE,"Exogam/Exogam_Pattern");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoPatternGOCCET,"Exogam/Exogam_Pattern");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoAngletheta,"Exogam/Exogam_Pattern");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoAnglephi,"Exogam/Exogam_Pattern");
	
    MySpectraList->AddSpectrum(fExogam2->fMyHistoMultiCrystal,"Exogam/Exogam_Multiplicity");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoMultiCrystalperClover,"Exogam/Exogam_Multiplicity");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoMultiClover,"Exogam/Exogam_Multiplicity");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoMultiAntiCompt,"Exogam/Exogam_Multiplicity");

    MySpectraList->AddSpectrum(fExogam2->TimeStampDiff,"Exogam/TimeStamp");
    
    ExogamAlignement = new TH2F("ExogamAlignement","Exogam Indiv - VXI [tic]",100,0,100,200,-100,100); 
    MySpectraList->AddSpectrum(ExogamAlignement,"Exogam/TimeStamp");
    
    MySpectraList->AddSpectrum(fExogam2->fMyHistoNFS_ToF,"Exogam/NFS");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoNFS_En,"Exogam/NFS");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoNFS_EnEstar,"Exogam/NFS");
    MySpectraList->AddSpectrum(fExogam2->fMyHistoNFS_EnGmul,"Exogam/NFS");

  }



  if(fNfsExoAnaSpec && fExogam2->NfsSpectraConstructor()){
    // EN: NFS quick spectra for gamma response versus neutron Time.
    // CN: NFS 快速谱，用于检查 gamma 响应随中子 Time 的变化。
    MySpectraList->AddSpectrum(fExogam2->fNfsCrystalDeltaTEnergy,"NFS/Crystal_Fire");
    MySpectraList->AddSpectrum(fExogam2->fNfsAllCrystalTime,"NFS/Crystal_Time");
    for(int clo=0; clo<16; clo++){
      if(fExogam2->IsCloverActive(clo)==false)continue;
      MySpectraList->AddSpectrum(fExogam2->fNfsCloverAddbackGamma[clo],"NFS/Clover_Addback");
      MySpectraList->AddSpectrum(fExogam2->fNfsCloverAddbackGammaVeto[clo],"NFS/Clover_Addback_Veto");
      for(int cri=0; cri<4; cri++){
        int id=clo*4+cri;
        MySpectraList->AddSpectrum(fExogam2->fNfsCrystalDeltaT[id],"NFS/Crystal_Time");
        MySpectraList->AddSpectrum(fExogam2->fNfsCrystalEnergy[id],"NFS/Crystal_Energy");
      }
    }
  }



//-----------------------------------     Exogam Raw REA

  if(fExogam2REA->SpectraConstructor()){
	
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoNCriERaw,"ExogamREA/Exogam_2D");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoNCriTRaw,"ExogamREA/Exogam_2D");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoNCriECal,"ExogamREA/Exogam_2D");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoNCriTCal,"ExogamREA/Exogam_2D");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoNSegERaw,"ExogamREA/Exogam_2D");

    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoPatternESSTQ,"ExogamREA/Exogam_Pattern");
    MySpectraList->AddSpectrum(fExogam2REA->BoardIdHist,"ExogamREA/Exogam_Pattern");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSegSeg,"ExogamREA/Exogam_2D");
    
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoTTAll,"ExogamREA/TT");
    for(int k=0 ; k<16 ; k++){
      if(fExogam2REA->IsCloverActive(k)){
	MySpectraList->AddSpectrum(fExogam2REA->fMyHistoCloverECal[k],"ExogamREA/Clover_ECal"); // No process
	MySpectraList->AddSpectrum(fExogam2REA->fMyHistoCloverECalAdd[k],"ExogamREA/Clover_ECalAdd");//Prompt AddBack /clover
	MySpectraList->AddSpectrum(fExogam2REA->fMyHistoCloverECalACAdd_TC[k],"ExogamREA/Clover_ECalACAdd_TC"); //AddBack + AC + prompt /clover
	MySpectraList->AddSpectrum(fExogam2REA->fMyHistoCloverECalACAdd_DC[k],"ExogamREA/Clover_ECalACAddDC_DC");//AddBack + AC  /clover + DopplerCorrected
	MySpectraList->AddSpectrum(fExogam2REA->fMyHistoCloverECalACAddRejectF[k],"ExogamREA/Clover_ECalACAddRejF");
			
	MySpectraList->AddSpectrum(fExogam2REA->fMyHistoCloverTCal[k],"ExogamREA/Clover_TCal");
			
	for(int l=1;l<=4;l++){
	  MySpectraList->AddSpectrum(fExogam2REA->fMyHistoECCECal[k*100+(l-1)*10],"ExogamREA/ECC_ECal");
	  MySpectraList->AddSpectrum(fExogam2REA->fMyHistoECCTCal[k*100+(l-1)*10],"ExogamREA/ECC_TCal");
	  MySpectraList->AddSpectrum(fExogam2REA->fMyHistoECCTRaw[k*100+(l-1)*10],"ExogamREA/ECC_TRaw");
	  MySpectraList->AddSpectrum(fExogam2REA->fMyHistoESS_BGO[k*100+(l-1)*10],"ExogamREA/ESS");
	  MySpectraList->AddSpectrum(fExogam2REA->fMyHistoESS_CSI[k*100+(l-1)*10],"ExogamREA/ESS");
	  
	  MySpectraList->AddSpectrum(fExogam2REA->fMyHistoESS_BGOT[k*100+(l-1)*10],"ExogamREA/ESS");
	  MySpectraList->AddSpectrum(fExogam2REA->fMyHistoESS_CSIT[k*100+(l-1)*10],"ExogamREA/ESS");
	  
	  
	  MySpectraList->AddSpectrum(fExogam2REA->fMyHistoECCEESSE[k*100+(l-1)*10],"ExogamREA/ESS");
	  MySpectraList->AddSpectrum(fExogam2REA->fMyHistoECCEVetoed[k*100+(l-1)*10],"ExogamREA/CoreRejected");
	  MySpectraList->AddSpectrum(fExogam2REA->fMyHistoECCEAccepted[k*100+(l-1)*10],"ExogamREA/CoreAccepted");
	  MySpectraList->AddSpectrum(fExogam2REA->fMyHistoCrysECalACRejectF[k*100+(l-1)*10],"ExogamREA/Core_ECalAddRejF");
	 /* MySpectraList->AddSpectrum(fExogam2REA->fMyHistoECCET30[k][l-1],"Exogam/PSA");
	  MySpectraList->AddSpectrum(fExogam2REA->fMyHistoECCET60[k][l-1],"Exogam/PSA");
	  MySpectraList->AddSpectrum(fExogam2REA->fMyHistoECCET90[k][l-1],"Exogam/PSA");
	  MySpectraList->AddSpectrum(fExogam2REA->fMyHistoECCET30_60[k][l-1],"Exogam/PSA");
	  MySpectraList->AddSpectrum(fExogam2REA->fMyHistoECCET30_90[k][l-1],"Exogam/PSA");
	  MySpectraList->AddSpectrum(fExogam2REA->fMyHistoECCET60_90[k][l-1],"Exogam/PSA");
	*/		
			
	  for(int m=1; m<=4; m++){
	    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoGOCCEE[k*100+10*(l-1)+(m-1)],"ExogamREA/GOCCE_E");
	    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoGOCCET[k*100+10*(l-1)+(m-1)],"ExogamREA/GOCCE_T");
	  }
	}
      }
    }
    
     for(int k2=0 ; k2<16*4 ; k2++){
	for(int k3=0 ; k3<16*4 ; k3++){
		if(fExogam2REA->IsCloverActive(k2/4)&&fExogam2REA->IsCloverActive(k3/4)){
			//MySpectraList->AddSpectrum(fExogam2REA->fMyHistoTT[k2][k3],"ExogamREA/TT");
		}
	}
     }   

   
   
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSumECCE,"ExogamREA/Sum"); //sumCore
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSumECCT,"ExogamREA/Sum");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSumGOCCET,"ExogamREA/Sum");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSumECCE_ECCT,"ExogamREA/Sum");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSumGOCCEE,"ExogamREA/Sum");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSumExogam2,"ExogamREA/Sum"); //AddBack + AC rejected + prompt
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSumExogam2DC,"ExogamREA/Sum");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSumExogam2FoldCond,"ExogamREA/Sum");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSumExogam2DCFoldCond,"ExogamREA/Sum");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSumExogam22D,"ExogamREA/Sum");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSumExogam2DC2D,"ExogamREA/Sum");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoGammaGamma,"ExogamREA/Sum");//AddBack + AC rejected
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoGammaGammaCore,"ExogamREA/Sum");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoGammaGammaCoreDiag,"ExogamREA/Sum");
    MySpectraList->AddSpectrum(fExogam2REA->ECCTcorrelCheck,"ExogamREA/Sum");
    
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSumExogam2Calorimeter,"ExogamREA/Sum");
    MySpectraList->AddSpectrum(fExogam2REA->TransverseMomentumGamma,"ExogamREA/Sum");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSumSeg_Core,"ExogamREA/Sum");
    
  
    
    
    for(int l=0;l<10;l++){
    	MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSumExogam2DCNucId[l],"ExogamREA/IdSum");
    }
    
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSumExogam2DCClover,"ExogamREA/Sum");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoSumExogam2Clover,"ExogamREA/Sum");
    
    
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoPatternECCE,"ExogamREA/Exogam_Pattern");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoPatternECCT,"ExogamREA/Exogam_Pattern");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoPatternGOCCEE,"ExogamREA/Exogam_Pattern");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoPatternGOCCET,"ExogamREA/Exogam_Pattern");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoAngletheta,"ExogamREA/Exogam_Pattern");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoAnglephi,"ExogamREA/Exogam_Pattern");
	
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoMultiCrystal,"ExogamREA/Exogam_Multiplicity");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoMultiCrystalperClover,"ExogamREA/Exogam_Multiplicity");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoMultiClover,"ExogamREA/Exogam_Multiplicity");
    MySpectraList->AddSpectrum(fExogam2REA->fMyHistoMultiAntiCompt,"ExogamREA/Exogam_Multiplicity");

    MySpectraList->AddSpectrum(fExogam2REA->TimeStampDiff,"ExogamREA/TimeStamp");

  }


  //--------------------Trigger -----------------------------------

  if(fTrigger->SpectraConstructor()){
    //MySpectraList->AddSpectrum(fTrigger->centEXO,"centrum");
    //MySpectraList->AddSpectrum(fTrigger->fMyHistoTriggerCorr,"centrum");
    //MySpectraList->AddSpectrum(fTrigger->AGAVA_Status,"AGAVA");
    MySpectraList->AddSpectrum(fTrigger->TimeStampDiff,"TriggerGlobal/TS");
		
  }
  //--------------------Diamant -----------------------------------

  if(fCsI->SpectraConstructor()){
    for(int k=0 ; k<16 ; k++){
      MySpectraList->AddSpectrum(fCsI->fMyHistoCsIE[k],"Diamant/MonoDimE");
      MySpectraList->AddSpectrum(fCsI->fMyHistoCsIT[k],"Diamant/MonoDimShortTop");
      MySpectraList->AddSpectrum(fCsI->fMyHistoCsIPI[k],"Diamant/MonoDimPID");
      MySpectraList->AddSpectrum(fCsI->fMyHistoCsIE_PI[k],"Diamant/BiDimEPID");
      //MySpectraList->AddSpectrum(fCsI->fMyHistoCsIT_PI[k],"Diamant/BiDimTPI");
      //MySpectraList->AddSpectrum(fCsI->fMyHistoCsIE_T[k],"Diamant/BiDimET");
    }
		
    MySpectraList->AddSpectrum(fCsI->fMyHistoCsIPattE2d,"Diamant/Pattern");
    MySpectraList->AddSpectrum(fCsI->fMyHistoCsIPattT2d,"Diamant/Pattern");
    MySpectraList->AddSpectrum(fCsI->fMyHistoCsIPattPI2d,"Diamant/Pattern");
    MySpectraList->AddSpectrum(fCsI->fMyHistoCsIPattE,"Diamant/Pattern");
    MySpectraList->AddSpectrum(fCsI->fMyHistoCsIPattT,"Diamant/Pattern");
    MySpectraList->AddSpectrum(fCsI->fMyHistoCsIPattPI,"Diamant/Pattern");
		
    MySpectraList->AddSpectrum(fCsI->fMyHistoCsIEmult,"Diamant/Multiplicity");
    MySpectraList->AddSpectrum(fCsI->fMyHistoCsITmult,"Diamant/Multiplicity");
    MySpectraList->AddSpectrum(fCsI->fMyHistoCsIPImult,"Diamant/Multiplicity");
		
    MySpectraList->AddSpectrum(fCsI->fMyHistoCsIDataCheckET,"Diamant/DataMonitoring");
    MySpectraList->AddSpectrum(fCsI->fMyHistoCsIDataCheckEPI,"Diamant/DataMonitoring");
    MySpectraList->AddSpectrum(fCsI->fMyHistoCsIDataCheckTPI,"Diamant/DataMonitoring");
    MySpectraList->AddSpectrum(fCsI->TimeStampDiff,"Diamant/DataMonitoring");
  }
	
	
  //--------------------Neutron Wall -----------------------------------

    if(fNeda->SpectraConstructor()){
		
    for(int k=0 ; k<96 ; k++){
      MySpectraList->AddSpectrum(fNeda->fMyHistoNWallE[k],"NEDA/E");
      MySpectraList->AddSpectrum(fNeda->fMyHistoNWallTOF[k],"NEDA/TOF");
      MySpectraList->AddSpectrum(fNeda->fMyHistoNWallZCO[k],"NEDA/ZCO");
      MySpectraList->AddSpectrum(fNeda->fMyHistoNWallZCO_E[k],"NEDA/E_ZCO");
      MySpectraList->AddSpectrum(fNeda->fMyHistoNWallZCO_TOF[k],"NEDA/ZCO_TOF");
    }
		
    for (int i_hist=0; i_hist<9; i_hist++) {
      MySpectraList->AddSpectrum(fNeda-> fMyHistoNWallNbetween_dt[i_hist],
				 "NEDA/Nbetween_dt");
    }
		  

    MySpectraList->AddSpectrum(fNeda->fMyHistoNWallPattE2d,"NEDA/Pattern");
    MySpectraList->AddSpectrum(fNeda->fMyHistoNWallPattT2d,"NEDA/Pattern");
    MySpectraList->AddSpectrum(fNeda->fMyHistoNWallPattZCO2d,"NEDA/Pattern");
    MySpectraList->AddSpectrum(fNeda->fMyHistoNWallPattE,"NEDA/Pattern");
    MySpectraList->AddSpectrum(fNeda->fMyHistoNWallPattT,"NEDA/Pattern");
    MySpectraList->AddSpectrum(fNeda->fMyHistoNWallPattZCO,"NEDA/Pattern");

    MySpectraList->AddSpectrum(fNeda->fMyHistoNWallEmult,"NEDA/Multiplicity");
    MySpectraList->AddSpectrum(fNeda->fMyHistoNWallTmult,"NEDA/Multiplicity");
    MySpectraList->AddSpectrum(fNeda->fMyHistoNWallZCOmult,"NEDA/Multiplicity");
    MySpectraList->AddSpectrum(fNeda->fMyHistoNWallDataCheckQT,"NEDA/DataMonitoring");
    MySpectraList->AddSpectrum(fNeda->fMyHistoNWallDataCheckQZCO,"NEDA/DataMonitoring");
    MySpectraList->AddSpectrum(fNeda->fMyHistoNWallDataCheckTZCO,"NEDA/DataMonitoring");
    MySpectraList->AddSpectrum(fNeda->TimeStampDiff,"NEDA/DataMonitoring");
    MySpectraList->AddSpectrum(fNeda->BoardDiff,"NEDA/DataMonitoring");
    MySpectraList->AddSpectrum(fNeda->ChannelDiff,"NEDA/DataMonitoring");
    MySpectraList->AddSpectrum(fNeda->ChannelDiff2D,"NEDA/DataMonitoring");
    MySpectraList->AddSpectrum(fNeda->TimeStampDiffInterEvent,"NEDA/DataMonitoring");
    MySpectraList->AddSpectrum(fNeda->CrossTalk2D,"NEDA/DataMonitoring");	
  }
	
  	
	
  // --------------------- for velocity ---------------------------
  /*	
	if(fMW->SpectraConstructor()){
	MySpectraList->AddSpectrum(fMW->fMyHistoMW,"MW");
	MySpectraList->AddSpectrum(fMW->fMyHistoVelocity,"MW");
	MySpectraList->AddSpectrum(fMW->fMyHistoBeta,"MW");
	MySpectraList->AddSpectrum(fMW->fMyHistoMWMul,"MW");
	MySpectraList->AddSpectrum(fMW->fMyHistoTacCorr,"MW");
	}
	
  */


	if(fParis->SpectraConstructor()){
		for(int k=0 ; k<80 ; k++){
			MySpectraList->AddSpectrum(fParis->fMyHistoParisQShort[k],"Paris/QShort");
			MySpectraList->AddSpectrum(fParis->fMyHistoParisQLong[k],"Paris/QLong");
			MySpectraList->AddSpectrum(fParis->fMyHistoParisQShortQLong[k],"Paris/QShortQLong2D");
			MySpectraList->AddSpectrum(fParis->fMyHistoParisQLaBr3[k],"Paris/QShortLaBr3");
			MySpectraList->AddSpectrum(fParis->fMyHistoParisQNaI[k],"Paris/QLongNaI");
			MySpectraList->AddSpectrum(fParis->fMyHistoParisCfd[k],"Paris/Cfd");
			MySpectraList->AddSpectrum(fParis->fMyHistoParisQShortQLongCond[k],"Paris/QShortQLongLaBr3");
			MySpectraList->AddSpectrum(fParis->fMyHistoParisCompton[k],"Paris/Compton2D");
			MySpectraList->AddSpectrum(fParis->fMyHistoParisQLongCompton[k],"Paris/Compton");
			MySpectraList->AddSpectrum(fParis->IndvTimeStampDiffTref[k],"Paris/Time");
		}
		MySpectraList->AddSpectrum(fParis->fMyHistoParisComptonRec,"Paris/Compton");
		MySpectraList->AddSpectrum(fParis->fMyHistoParisAddBack,"Paris/Pattern");
		MySpectraList->AddSpectrum(fParis->fMyHistoParisCalo,"Paris/Pattern");
		MySpectraList->AddSpectrum(fParis->fMyHistoParisMerit,"Paris/Pattern");
		MySpectraList->AddSpectrum(fParis->fMyHistoParisMul,"Paris/Pattern");
		MySpectraList->AddSpectrum(fParis->fMyHistoParisPattQShort2d,"Paris/Pattern");
		MySpectraList->AddSpectrum(fParis->fMyHistoParisPattQLong2d,"Paris/Pattern");
		MySpectraList->AddSpectrum(fParis->fMyHistoParisPattCfd2d,"Paris/Pattern");
		MySpectraList->AddSpectrum(fParis->TimeStampDiffQLaBr3,"Paris/Pattern");
		MySpectraList->AddSpectrum(fParis->TimeStampDiffQNaI,"Paris/Pattern");
		MySpectraList->AddSpectrum(fParis->TimeStampDiffAll,"Paris/Pattern");
		MySpectraList->AddSpectrum(fParis->TimeStampDiffTref,"Paris/Pattern");
		MySpectraList->AddSpectrum(fParis->TimeStampDiffTrefPattern,"Paris/Pattern");
		MySpectraList->AddSpectrum(fParis->TimeStampDiffTrefEg,"Paris/Pattern");
		MySpectraList->AddSpectrum(fParis->TimeStampDiffQLaBr3Eg,"Paris/Pattern");
		MySpectraList->AddSpectrum(fParis->fMyHistoParisQLaBr3SumDC,"Paris/LaBr3Adv");
		MySpectraList->AddSpectrum(fParis->fMyHistoParisQShortGG,"Paris/LaBr3Adv");
		MySpectraList->AddSpectrum(fParis->fMyHistoParisQShortGGTimeGated,"Paris/LaBr3Adv");
		MySpectraList->AddSpectrum(fParis->fMyHistoParisCaloFold,"Paris/LaBr3Adv");
		MySpectraList->AddSpectrum(fParis->fMyHistoParisQLaBr3Sum,"Paris/QShort");
		MySpectraList->AddSpectrum(fParis->fMyHistoParisThetaPhi,"Paris/Pattern");
		MySpectraList->AddSpectrum(fParis->NeutronPattern,"Paris/Pattern");
		
	}
	
	if(fGeneric->SpectraConstructor()){
		for(int k=0 ; k<100 ; k++){
			MySpectraList->AddSpectrum(fGeneric->fMyHistoGenericE[k],"Generic/Single_nrj");
			MySpectraList->AddSpectrum(fGeneric->fMyHistoGenericE[k],"Generic/Single_tdc");

		}
		MySpectraList->AddSpectrum(fGeneric->fMyHistoGenericMul,"Generic");
		MySpectraList->AddSpectrum(fGeneric->TimeStampDiff,"Generic");
		MySpectraList->AddSpectrum(fGeneric->fMyHistoGenericPattE,"Generic");
		MySpectraList->AddSpectrum(fGeneric->fMyHistoGenericPattT,"Generic");
		MySpectraList->AddSpectrum(fGeneric->fMyHistoGenericDEE,"Generic");
		MySpectraList->AddSpectrum(fGeneric->fMyHistoGenericDE,"Generic");
		MySpectraList->AddSpectrum(fGeneric->fMyHistoGenericDEECond,"Generic");
		MySpectraList->AddSpectrum(fGeneric->fMyHistoGenericDETOF,"Generic");
		MySpectraList->AddSpectrum(fGeneric->fMyHistoGenericDETOF_LISE,"Generic");
		MySpectraList->AddSpectrum(fGeneric->fMyHistoGenericDETOF_LISEcond,"Generic");
		MySpectraList->AddSpectrum(fGeneric->TransverseMomentum,"Generic");
		MySpectraList->AddSpectrum(fGeneric->fMyHistoGenericTT ,"Generic");
		MySpectraList->AddSpectrum(fGeneric->fMyHistoGenericEE ,"Generic");
		
		
		for(int l=0;l<100;l++){
			MySpectraList->AddSpectrum(fGeneric->trackI[l],"Generic/Rate");
		}

		
	}

	if(fEbyE->SpectraConstructor()){
		MySpectraList->AddSpectrum(fEbyE->fMyHistoEbyE2d,"EbyE");
		MySpectraList->AddSpectrum(fEbyE->fMyHistoEbySum,"EbyE");
		MySpectraList->AddSpectrum(fEbyE->fMyHistoEbylabel,"EbyE");
	}
	
	
	MergeFrameDeltaTS = new TH1F("MergeFrameDeltaTS","MergeFrameDeltaTS",10010,-10,10000);
	MergeDetectorFrameDeltaTS= new TH1F("MergeDetectorFrameDeltaTS","MergeDetectorFrameDeltaTS",2000,-1000,1000);
  	MFMKey= new TH1F("MFMKey","MFMKey",70000,0,70000);
  	MySpectraList->AddSpectrum(MergeFrameDeltaTS,"Global");
	MySpectraList->AddSpectrum(MergeDetectorFrameDeltaTS,"Global");
	MySpectraList->AddSpectrum(MFMKey,"Global");
	
	DTS_test= new TH1F("DTS_test","Exogam - VXI[tic]",20000,-10000,10000);
	MySpectraList->AddSpectrum(DTS_test,"Global");
	DTS_test2= new TH1F("DTS_test2","ZDD {VXI - Plastic} [tic]",2000,-1000,1000);
	MySpectraList->AddSpectrum(DTS_test2,"Global");
	DTS_test3= new TH1F("DTS_test3","ZDD Plastic - VXI [tic]",2000,-1000,1000);
	MySpectraList->AddSpectrum(DTS_test3,"Global");
	rate= new TH1F("rate","MFM Rate 2h",7200,0,7200); //2h = 7200 sec
	MySpectraList->AddSpectrum(rate,"Global");
	
	
	
	if(fVamosIC->SpectraConstructor()){
		for(int k=0 ; k<1000 ; k++){
			MySpectraList->AddSpectrum(fVamosIC->fMyHistoVamosICE[k],"VamosIC/Single");
		}
		MySpectraList->AddSpectrum(fVamosIC->fMyHistoVamosICE2D,"VamosIC/2D");
		MySpectraList->AddSpectrum(fVamosIC->fMyHistoVamosICECorrel,"VamosIC/2D");
	}
	
	
  printf("\033[31mInfo:: End Spectra Loading \033[m \n");
  cerr<<""<<endl;
}
//______________________________________________________________

void GUser::InitUserRun()
{
  // Initialisation for user treatemeant for each  run  
  // For specific user treatement
  
  	
  //check the ParamList contains
  //lparams->DumpListPara();

  
	
  //fTrigger->Init(lparams);
  //fMW->Init(lparams);
  cerr<<""<<endl;
  printf("\033[31mInfo:: Start Init of each Class \033[m \n");
	
  YAML::Node config = YAML::LoadFile("Yaml_config_files/config.yaml");
  fExogam2->InitCal();
  if(config["Guser"]["exogam2"]["use_default_cal"].as<bool>())fExogam2->DefaultCal();
  if(config["Guser"]["exogam2"]["readcal_file"].as<bool>())fExogam2->ReadCal();
  fExogam2->CounterReset();
  cerr<<"End Init fExogam2"<<endl;
  		
		
  fExogam2REA->InitCal();
  if(config["Guser"]["exogam2REA"]["use_default_cal"].as<bool>())fExogam2REA->DefaultCal();
  if(config["Guser"]["exogam2REA"]["readcal_file"].as<bool>()) fExogam2REA->ReadCal();
  fExogam2REA->CounterReset();
  cerr<<"End Init fExogam2REA"<<endl;
  		
		
		
		
		
  // fMW->InitCal();
  //fMW->ReadCal();
  //cerr<<"End Init fMW "<<endl;
  fNeda->InitCal();
  //fNeda->ReadCal();
  //if(SpyOnly==false)fNeda->ReadCut();
  //fNeda->Init_n2();
  cerr<<"End Init fNeda"<<endl;
  fCsI->InitCal();
  //fCsI->ReadCal();
  //fCsI->ReadCut();
  cerr<<"End Init fCsI "<<endl;
  
  fParis->InitCal();
  if(config["Guser"]["paris"]["readcal_file"].as<bool>()) fParis->ReadCal();
  fParis->ClearCounter();
  cerr<<"End Init fParis "<<endl;
  
  
  fGeneric->InitCal();
   //fGeneric->ReadCal();
  if(SpyOnly==true)fGeneric->ReadCut();
  fGeneric->ClearCounter();
  
  cerr<<"End Init fGeneric "<<endl;
  cerr<<"End Init fEbyE "<<endl;
  cerr<<""<<endl;
  
  printf("\033[33mInfo:: Reading look up tables  \033[m \n");
  
  fExogam2->InitNumexo2(const_cast<Char_t*>("Conf/Exogam2_in_Tree.cfg"));
  fExogam2REA->InitNumexo2(const_cast<Char_t*>("Conf/Exogam2_in_Tree.cfg"));
  fNeda->InitNumexo2(const_cast<Char_t*>("Conf/Neda_in_Tree.cfg"));
  fCsI->InitNumexo2(const_cast<Char_t*>("Conf/Diamant_in_Tree.cfg"));
  fParis->InitNumexo2(const_cast<Char_t*>("Conf/Paris_in_Tree.cfg"));	
  fGeneric->InitNumexo2(const_cast<Char_t*>("Conf/Generic_in_Tree.cfg"));	
  fVamosIC->InitNumexo2(const_cast<Char_t*>("Conf/VamosIC_in_Tree.cfg"));
  fEbyE->InitNumexo2(const_cast<Char_t*>("ACTIONS_experiment.CHC_PAR"));
  	
  printf("\033[33m----> Completed \033[m \n");
  eventcounter=tvb=0;
  cerr<<""<<endl;		
  PreviousTime=date.GetMinute();
  CMFM_PARIS_FRAME_TYPE=CMFM_MERGE_TS_FRAME_TYPE=CMFM_EXO2_FRAME_TYPE=CMFM_EBY_EN_FRAME_TYPE=CMFM_EBY_TS_FRAME_TYPE=CMFM_EBY_EN_TS_FRAME_TYPE=CMFM_NEDA_FRAME_TYPE=CMFM_DIAMANT_FRAME_TYPE=CMFM_NEDACOMPRESS_FRAME_TYPE=CMFM_GENERIC_FRAME_TYPE=CMFM_EXO2REA_FRAME_TYPE=CMFM_VAMOSIC_FRAME_TYPE=00;
  TSZero=0;
  printf("\033[31mInfo:: End Init of each Class \033[m \n");	
		
  

  
  
}
//______________________________________________________________
void GUser::User()
{
  
  
  if(debug)  printf("\033[31mInfo:: --------------< User Clear >------------------\033[m \n");
  fExogam2->Clear();
  fExogam2REA->Clear();
  fTrigger->Clear();
  //fMW->Clear();
  fCsI->Clear();
  fNeda->Clear();
  fParis->Clear();
  fGeneric->Clear();
  fVamosIC->Clear();
  fEbyE->Clear();
  char name[100];
  
  if(debug)  printf("\033[31mInfo:: --------------< User Treatement >------------------\033[m \n");
  int size;
  Exogam2b=Exogam2REAb=  Exogamb=Triggerb=MWb=Tacb=eventOK=nowcheck=CsIb=NWallb=Parisb=Genericb=EbyEb=VamosICb=false;
  uint16_t label,value;
   
  GEventMFM* pMFMevent = (GEventMFM*)GetEvent(); // acces � l'evenement
  eventcounter++;
  pCommonFrame->SetAttributs(pMFMevent->GetFrame()->GetPointHeader());

  if(RawTreeFillBool){
    CaptureRawEvent(pCommonFrame);
    RawTree->Fill();
  }
   
   if(debug) cerr<<"Frame Type:: "<<pCommonFrame->GetFrameType()<<endl;
   
   InceptionLayer=0;
   for(Int_t jj = 0; jj<100 ; jj++){Inceptionnbinsideframe[jj]=Inceptionnoevent[jj]=PrevTSInception[jj]=0;}
   
   
   Unpack(pCommonFrame,debug);
  
   if(debug) cerr<<"Entering in pre-Treat() section "<<endl;
  //########################" Treat Section ######################### 
  
   
   if(MakeTreeOnly==false){
   	if(debug) cerr<<"Entering in Treat() section "<<endl;
   	if(CsIb) 	fCsI->Treat();
   	if(NWallb) 	fNeda->Treat();
   	if(Genericb)	fGeneric->Treat();
   	if(Exogam2b)	fExogam2->Treat();
	if(Exogam2REAb)	fExogam2REA->Treat();
   	if(Parisb)      fParis->Treat();
   	if(Triggerb)	fTrigger->Treat();
   	if(MWb)		fMW->Treat();  	 
  	if(EbyEb)	fEbyE->Treat();
	if(VamosICb)	fVamosIC->Treat();
  	if(debug) cerr<<"End Treat() section "<<endl;
	
	//if(Exogam2b&&EbyEb)fExogam2->Treat();
	//if(Exogam2b&&EbyEb)Analysis();
   	//Analysis();
	if(debug) cerr<<"End Analysis() section "<<endl;
   
   }
  //########################" Diagnostic Section #########################

  if(eventcounter>1E7 ){
    eventcounter=0;
    if(debug)cout<< "   "<<endl;
    if(debug)cout<< "-----------------------------   "<<endl;
    if(debug)cout<< "Counter Print after 10 MEvents"<<endl;
    if(fExogam2->BoolSpec){
    	fExogam2->Counter();
    	fExogam2->CounterReset();
    }
    if(fExogam2REA->BoolSpec){
    	fExogam2REA->Counter();
    	fExogam2REA->CounterReset();
    }
    
    
    
    
    //fExogam2->CheckCoreResolution(1408.);
    if(debug)cout<< "-----------------------------   "<<endl;
   // fNeda->Counter();
    //fNeda->ClearCounter();
	
    if(debug)cout<< "-----------------------------   "<<endl;
	
    //fCsI->Counter();
   // fCsI->ClearCounter();
    
	
    if(debug)cout<< "-----------------------------   "<<endl; 
    //fParis->Counter();
    //fParis->ClearCounter();
    
    //fGeneric->Counter();
    //fGeneric->ClearCounter();
    
    if(TreeFillBool)cout<< "Current Tree :: is "<< fileTree2->GetName()<<" / "<<LocalTree->GetEntries()<<" evt  (" <<LocalTree->GetTotBytes()/8e10<<" Go)"<<endl;
    if(RawTreeFillBool)cout<< "Current RawTree :: is "<< fileTree2->GetName()<<" / "<<RawTree->GetEntries()<<" evt  (" <<RawTree->GetTotBytes()/8e10<<" Go)"<<endl;
    if(NfsTreeFillBool)cout<< "Current NFS Tree :: is "<< fileNfsTree->GetName()<<" / "<<NfsTree->GetEntries()<<" evt  (" <<NfsTree->GetTotBytes()/8e10<<" Go)"<<endl;
    if(NfsMult3TreeFillBool)cout<< "Current NFS mult>=2 Tree :: is "<< fileNfsMult3Tree->GetName()<<" / "<<NfsMult3Tree->GetEntries()<<" evt  (" <<NfsMult3Tree->GetTotBytes()/8e10<<" Go)"<<endl;
    gSystem->GetCpuInfo(&cpuinfos);
    gSystem->GetMemInfo(&meminfos);
    //printf("\033[32mInfo SYS:: RAM:: %2.1f\%, SWAP:: %2.1f\%, CPU Load:: %2.1f\%  \033[m \n",100.*meminfos.fMemUsed/meminfos.fMemTotal,100.*meminfos.fSwapUsed/meminfos.fSwapTotal,cpuinfos.fIdle);
    
    printf("\033[32mInfo SYS:: RAM:: %2.1f%%, SWAP:: %2.1f%%, CPU Load:: %2.1f%%  \033[m\n",
       100.*meminfos.fMemUsed/meminfos.fMemTotal,
       100.*meminfos.fSwapUsed/meminfos.fSwapTotal,
       cpuinfos.fIdle);

    
   // if(100.*meminfos.fSwapUsed/meminfos.fSwapTotal >10)printf("\033[31m Alarm:: Swap is greater than 10\% \033[m \n");
    if(100.*meminfos.fSwapUsed/meminfos.fSwapTotal >10)
    fputs("\033[31mAlarm:: Swap is greater than 10%\033[m\n", stdout);

    
    
  cout<<"-------------Counter ----------------"<<endl;
  cout<<"Frame with TS Zero  " <<TSZero<<endl;
  cout<<"CMFM_MERGE_FRAME_TYPE "<<CMFM_MERGE_TS_FRAME_TYPE<<endl;
  cout<<"CMFM_EXO2_FRAME_TYPE "<<CMFM_EXO2_FRAME_TYPE<<endl;
  cout<<"CMFM_EXO2REA_FRAME_TYPE "<<CMFM_EXO2REA_FRAME_TYPE<<endl;
  cout<<"CMFM_NEDA_FRAME_TYPE "<<CMFM_NEDA_FRAME_TYPE<<endl;
  cout<<"CMFM_PARIS_FRAME_TYPE "<<CMFM_PARIS_FRAME_TYPE<<endl;
  cout<<"CMFM_GENERIC_FRAME_TYPE "<<CMFM_GENERIC_FRAME_TYPE<<endl;
  cout<<"CMFM_VAMOSIC_FRAME_TYPE "<<CMFM_VAMOSIC_FRAME_TYPE<<endl;
  cout<<"CMFM_EBY_EN_FRAME_TYPE "<<CMFM_EBY_EN_FRAME_TYPE<<endl;
  if(CMFM_NEDA_FRAME_TYPE>0)cout<<"Duplicated Neda   "<<fNeda->duplicatedEventC<<"  "<<100.*fNeda->duplicatedEventC/CMFM_NEDA_FRAME_TYPE <<"%"<<endl;
  cout<<"CMFM_NEDACOMPRESS_FRAME_TYPE "<<CMFM_NEDACOMPRESS_FRAME_TYPE<<endl;
  if(CMFM_NEDACOMPRESS_FRAME_TYPE>0)cout<<"Duplicated Neda Compressed  "<<fNeda->duplicatedEventC<<"  "<<100.*fNeda->duplicatedEventC/CMFM_NEDACOMPRESS_FRAME_TYPE <<"%"<<endl;
  cout<<"CMFM_DIAMANT_FRAME_TYPE "<<CMFM_DIAMANT_FRAME_TYPE<<endl;
  if(CMFM_DIAMANT_FRAME_TYPE>0)cout<<"Duplicated Diamant   "<<fCsI->duplicatedEventC<<"  "<<100.*fCsI->duplicatedEventC/CMFM_DIAMANT_FRAME_TYPE <<"%"<<endl;
  cout<<"------------- ----------------"<<endl;
  if(CMFM_PARIS_FRAME_TYPE>0)cout<<"Duplicated Paris   "<<fParis->duplicatedEventC<<"  "<<100.*fParis->duplicatedEventC/CMFM_PARIS_FRAME_TYPE <<"%"<<endl;
  if(CMFM_PARIS_FRAME_TYPE>0)cout<<"Event with no Tref "<<fParis->EventNoTref   <<  "  "<<100.*fParis->EventNoTref/fParis->MergerCounter<<"%"<<endl;
  if(CMFM_PARIS_FRAME_TYPE>0)cout<<"Event with  Tref "<<fParis->EventWithTref   <<  "  "<<100.*fParis->EventWithTref/fParis->MergerCounter<<"%"<<endl;
  if(CMFM_PARIS_FRAME_TYPE>0)cout<<"Event with duplicated Tref "<<fParis->TrefError<<  "  "<<100.*fParis->TrefError/fParis->MergerCounter<<"%"<<endl;
  cout<<"------------- ----------------"<<endl;
  if(CMFM_GENERIC_FRAME_TYPE>0)cout<<"Duplicated Generic   "<<fGeneric->duplicatedEventC<<"  "<<100.*fGeneric->duplicatedEventC/CMFM_GENERIC_FRAME_TYPE <<"%"<<endl;
  cout<<"------------- ----------------"<<endl;
  if(CMFM_EXO2_FRAME_TYPE>0)cout<<"No Segment Net Charge Event in Exogam  "<<fExogam2->NoNetCharge<<"  "<<100.*fExogam2->NoNetCharge/CMFM_EXO2_FRAME_TYPE<<"%"<<endl;
  cout<<"------------- ----------------"<<endl;
    
    
    
  }
 
  if(TreeFillBool)LocalTree->Fill();
  if(NfsTreeFillBool)NfsTree->Fill();
  if(NfsMult3TreeFillBool && fExogam2->GetExogam2Data()->GetE877CloverVetoMult()>=kNfsVetoCloverMultiplicityMin)NfsMult3Tree->Fill();
  
  
  
  if(debug)  printf("\033[32mInfo:: --------------< End User Treatement >------------------\033[m \n");
}
//______________________________________________________________

void GUser::EndUserRun()
{
  //  end of run ,  executed a end of each run
  /*cout <<"--------------< End User Run Called >---"<<endl; 	  
    */
   if(TreeFillBool || RawTreeFillBool){
    fileTree2->cd();
    fileTree2->ls();
    if(TreeFillBool && LocalTree){
      cout<<"Final Writing Tree :: "<< LocalTree->GetTotBytes()/8e9<<" Go " <<endl;
      LocalTree->Show();
      LocalTree->Write();
    }
    if(RawTreeFillBool && RawTree){
      cout<<"Final Writing RawTree :: "<< RawTree->GetTotBytes()/8e9<<" Go " <<endl;
      RawTree->Show();
      RawTree->Write();
    }
    fileTree2->ls();
    cout<<"Done !"<<endl;
    fileTree2->Close();
    cout<<"Tree File closed"<<endl;
    TreeFillBool=false;
    RawTreeFillBool=false;
    LocalTree=NULL;
    RawTree=NULL;
    fileTree2=NULL;
    }

   if(NfsTreeFillBool){
    fileNfsTree->cd();
    fileNfsTree->ls();
    if(NfsTree){
      cout<<"Final Writing NFS Tree :: "<< NfsTree->GetTotBytes()/8e9<<" Go " <<endl;
      NfsTree->Show();
      NfsTree->Write();
    }
    fileNfsTree->ls();
    cout<<"Done NFS Tree !"<<endl;
    fileNfsTree->Close();
    cout<<"NFS Tree File closed"<<endl;
    NfsTreeFillBool=false;
    NfsTree=NULL;
    fileNfsTree=NULL;
    }

   if(NfsMult3TreeFillBool){
    fileNfsMult3Tree->cd();
    fileNfsMult3Tree->ls();
    if(NfsMult3Tree){
      cout<<"Final Writing NFS mult>=2 Tree :: "<< NfsMult3Tree->GetTotBytes()/8e9<<" Go " <<endl;
      NfsMult3Tree->Show();
      NfsMult3Tree->Write();
    }
    fileNfsMult3Tree->ls();
    cout<<"Done NFS mult>=2 Tree !"<<endl;
    fileNfsMult3Tree->Close();
    cout<<"NFS mult>=2 Tree File closed"<<endl;
    NfsMult3TreeFillBool=false;
    NfsMult3Tree=NULL;
    fileNfsMult3Tree=NULL;
    }

}

//______________________________________________________________
void GUser::EndUser()
{
  // globlal final end executed a end of runs
  // must be explicitly called! 
  bool debug = true;
  char name[100];
  TDatime date;
  if(debug)cout <<"--------------< End User Called> ------------------\n";
   
  if(debug)cout<<"-------------Counter ----------------"<<endl;
  
  if(debug)cout<<"Frame with TS Zero  " <<TSZero<<endl;
  if(debug)cout<<"CMFM_MERGE_FRAME_TYPE "<<CMFM_MERGE_TS_FRAME_TYPE<<endl;
  if(debug)cout<<"CMFM_EXO2_FRAME_TYPE "<<CMFM_EXO2_FRAME_TYPE<<endl;  
  if(debug)cout<<"CMFM_EXO2REA_FRAME_TYPE "<<CMFM_EXO2REA_FRAME_TYPE<<endl;
  if(debug)cout<<"CMFM_VAMOSIC_FRAME_TYPE "<<CMFM_VAMOSIC_FRAME_TYPE<<endl;

  if(debug)cout<<"CMFM_NEDA_FRAME_TYPE "<<CMFM_NEDA_FRAME_TYPE<<endl;
  if(debug)cout<<"CMFM_PARIS_FRAME_TYPE "<<CMFM_PARIS_FRAME_TYPE<<endl;
  if(debug)cout<<"CMFM_GENERIC_FRAME_TYPE "<<CMFM_GENERIC_FRAME_TYPE<<endl;
  if(debug)cout<<"CMFM_EBY_EN_FRAME_TYPE "<<CMFM_EBY_EN_FRAME_TYPE<<endl;
  if(CMFM_NEDA_FRAME_TYPE>0)cout<<"Duplicated Neda   "<<fNeda->duplicatedEventC<<"  "<<100.*fNeda->duplicatedEventC/CMFM_NEDA_FRAME_TYPE <<"%"<<endl;
  if(debug)cout<<"CMFM_NEDACOMPRESS_FRAME_TYPE "<<CMFM_NEDACOMPRESS_FRAME_TYPE<<endl;
  if(CMFM_NEDACOMPRESS_FRAME_TYPE>0)cout<<"Duplicated Neda Compressed  "<<fNeda->duplicatedEventC<<"  "<<100.*fNeda->duplicatedEventC/CMFM_NEDACOMPRESS_FRAME_TYPE <<"%"<<endl;
  if(debug)cout<<"CMFM_DIAMANT_FRAME_TYPE "<<CMFM_DIAMANT_FRAME_TYPE<<endl;
  if(CMFM_DIAMANT_FRAME_TYPE>0)cout<<"Duplicated Diamant   "<<fCsI->duplicatedEventC<<"  "<<100.*fCsI->duplicatedEventC/CMFM_DIAMANT_FRAME_TYPE <<"%"<<endl;
  if(debug)cout<<"------------- ----------------"<<endl;
  if(CMFM_PARIS_FRAME_TYPE>0)cout<<"Duplicated Paris   "<<fParis->duplicatedEventC<<"  "<<100.*fParis->duplicatedEventC/CMFM_PARIS_FRAME_TYPE <<"%"<<endl;
  if(CMFM_PARIS_FRAME_TYPE>0)cout<<"Event with no Tref "<<fParis->EventNoTref   <<  "  "<<100.*fParis->EventNoTref/fParis->MergerCounter<<"%"<<endl;
  if(CMFM_PARIS_FRAME_TYPE>0)cout<<"Event with  Tref "<<fParis->EventWithTref   <<  "  "<<100.*fParis->EventWithTref/fParis->MergerCounter<<"%"<<endl;
  if(CMFM_PARIS_FRAME_TYPE>0)cout<<"Event with duplicated Tref "<<fParis->TrefError<<  "  "<<100.*fParis->TrefError/fParis->MergerCounter<<"%"<<endl;
  if(debug)cout<<"------------- ----------------"<<endl;
  if(CMFM_GENERIC_FRAME_TYPE>0)cout<<"Duplicated Generic   "<<fGeneric->duplicatedEventC<<"  "<<100.*fGeneric->duplicatedEventC/CMFM_GENERIC_FRAME_TYPE <<"%"<<endl;
  if(debug)cout<<"------------- ----------------"<<endl;
  if(CMFM_EXO2_FRAME_TYPE>0)cout<<"No Segment Net Charge Event in Exogam  "<<fExogam2->NoNetCharge<<"  "<<100.*fExogam2->NoNetCharge/CMFM_EXO2_FRAME_TYPE<<"%"<<endl;
  if(debug)cout<<"------------- ----------------"<<endl;
  if(CMFM_EXO2REA_FRAME_TYPE>0)cout<<"No Segment Net Charge Events in Exogam_REA : "<<fExogam2REA->NoNetCharge<<" eq. "<<100.*fExogam2REA->NoNetCharge/CMFM_EXO2REA_FRAME_TYPE<<"%"<<endl;
  if(CMFM_EXO2REA_FRAME_TYPE>0)cout<<"No Core Net Charge Events in Exogam_REA : "<<fExogam2REA->NoCoreCharge<<" eq. "<<100.*fExogam2REA->NoCoreCharge/CMFM_EXO2REA_FRAME_TYPE<<"%"<<endl;
  if(debug)cout<<"------------- ----------------"<<endl;
  
  	if(debug)cout << "Start saving spectra " << endl;	
  	
	
	
	if(fExogam2->HListExogam2.GetLast()>=0 || fExogam2REA->HLisTExogam2REA.GetLast()>=0){
		sprintf(name,"out/histoExogam2_%d.root",1);
  		if(debug)cout << "Saving Exogam spectra : " << name << endl;
  		TFile *fileAfiiterE = new TFile(name,"RECREATE");
  		for( int i=0;i<=fExogam2->HListExogam2.GetLast(); i++ ){
    			fExogam2->HListExogam2[i]->Write();
  		}
  		for( int i=0;i<=fExogam2REA->HLisTExogam2REA.GetLast(); i++ ){
    			fExogam2REA->HLisTExogam2REA[i]->Write();
  		}
  		fileAfiiterE->Close();
	}

	if(fNfsExoAnaSpec && fExogam2->HListNfsExogam2.GetLast()>=0){
		sprintf(name,"out/nfs_histoExogam2_%d.root",1);
  		if(debug)cout << "Saving NFS Exogam spectra : " << name << endl;
  		TFile *fileNfsSpec = new TFile(name,"RECREATE");
  		for( int i=0;i<=fExogam2->HListNfsExogam2.GetLast(); i++ ){
    			fExogam2->HListNfsExogam2[i]->Write();
  		}
  		fileNfsSpec->Close();
	}

	if(fCsI->HListCsI.GetLast()>=0){
  		sprintf(name,"out/histoDiamant_%d.root",1);
  		if(debug)cout << "Saving Diamant spectra : " << name << endl;
  		TFile *fileAfiiterD = new TFile(name,"RECREATE");
  		for( int i=0;i<=fCsI->HListCsI.GetLast(); i++ ){
    			fCsI->HListCsI[i]->Write();
  		}
  		fileAfiiterD->Close();
	}

	if(fNeda->HListNWall.GetLast()>=0){
  		sprintf(name,"out/histoNWall_%d.root",1);
  		if(debug)cout << "Saving NWall spectra : " << name << endl;
  		TFile *fileAfiiterN = new TFile(name,"RECREATE");
  		for( int i=0;i<=fNeda->HListNWall.GetLast(); i++ ){
     			fNeda->HListNWall[i]->Write();
   		}
  		fileAfiiterN->Close();
	}

	if(fParis->HListParis.GetLast()>=0){
   		sprintf(name,"out/histoParis_%d.root",1);
  		if(debug)cout << "Saving Paris spectra : " << name << endl;
  		TFile *fileAfiiterP = new TFile(name,"RECREATE");
  		for( int i=0;i<=fParis->HListParis.GetLast(); i++ ){
     			fParis->HListParis[i]->Write();
   		}
  		fileAfiiterP->Close();
	}

	if(fGeneric->HListGeneric.GetLast()>=0){
  		sprintf(name,"out/histoGeneric_%d.root",1);
  		if(debug)cout << "Saving Generic spectra : " << name << endl;
  		TFile *fileAfiiterG = new TFile(name,"RECREATE");
  		for( int i=0;i<=fGeneric->HListGeneric.GetLast(); i++ ){
  			fGeneric->HListGeneric[i]->Write();
  		}
  		fileAfiiterG->Close();
	}

	if(fEbyE->HListEbyE.GetLast()>=0){
  		sprintf(name,"out/histoEbyE_%d.root",1);
  		if(debug)cout << "Saving EbyE spectra : " << name << endl;
  		TFile *fileAfiiterEBE = new TFile(name,"RECREATE");
  		for( int i=0;i<=fEbyE->HListEbyE.GetLast(); i++ ){
  			fEbyE->HListEbyE[i]->Write();
  		}
  		fileAfiiterEBE->Close();
	}

  	if(debug)cout << "End save spectra " << endl;    
    

}
//______________________________________________________________________________
void GUser::OnlyTreeConversion(bool TreeOnly)
{
	MakeTreeOnly=TreeOnly;
	if(MakeTreeOnly){
		cout<<"GUser::Only Tree Making"<<endl;
	}
	else{
		cout<<"GUser::Performing Full Analysis"<<endl;
	}
}
//______________________________________________________________________________

void GUser::LoadCut(bool Spy)
{
	SpyOnly=Spy;
	if(SpyOnly){
		cout<<"GUser::TCut loaded Spy "<<endl;
	}
	else{
		cout<<"GUser::No TCut loaded"<<endl;
	}


}
//______________________________________________________________________________

void GUser::InitTTreeUser(Char_t *nameF, bool Exogam, bool Trigger, bool MW, bool CsI, bool Neda, bool Paris, bool Generic, bool EbyE, bool exogamREA, bool VamosICb, bool RawTreeb)
{
  // EN: Initialise detector trees and the optional raw MFM event tree.
  // CN: 初始化探测器树，以及可选的 raw MFM event tree。
  cout<<"GUser::InitTTreeUser()"<<endl;

  bool StandardTree = Exogam|| Trigger|| MW || CsI || Neda || Paris ||Generic || EbyE || exogamREA||VamosICb;

  if(StandardTree || RawTreeb){
    fileTree2= new TFile(nameF,"RECREATE");
    fileTree2->ls();
  }

  if(RawTreeb){
    InitRawTreeBranches();
    cout<< "Raw MFM Tree ok"<<endl;
  }

  if(fNfsExoAnaTree){
    TString nfsName = BuildNfsTreeFileName(nameF);
    fileNfsTree= new TFile(nfsName.Data(),"RECREATE");
    fileNfsTree->ls();
    NfsTree=new TTree("TreeMaster","NFS TreeMaster");
    NfsTree->ls();
    NfsTreeFillBool=true;
    fExogam2->InitBranch(NfsTree);
    cout<< "NFS Exogam2 Tree ok: " << nfsName << endl;

    TString mult3Name = BuildNfsMult3TreeFileName(nameF);
    fileNfsMult3Tree= new TFile(mult3Name.Data(),"RECREATE");
    fileNfsMult3Tree->ls();
    NfsMult3Tree=new TTree("TreeMaster","NFS veto clover multiplicity >= 2 TreeMaster");
    NfsMult3Tree->ls();
    NfsMult3TreeFillBool=true;
    fExogam2->InitBranch(NfsMult3Tree);
    cout<< "NFS Exogam2 mult>=2 Tree ok: " << mult3Name << " (file prefix kept as mult3_ for script compatibility)" << endl;
  }

  if(StandardTree){
    fileTree2->cd();
    LocalTree=new TTree("TreeMaster","TreeMaster");
    LocalTree->ls();
    TreeFillBool=true;

    if(Exogam){
      fExogam2->InitBranch(LocalTree);
      cout<< "Exogam2 Tree ok"<<endl;
    }
    if(exogamREA){
      fExogam2REA->InitBranch(LocalTree);
      cout<< "Exogam2 by REA Tree ok"<<endl;
    }
    if(Trigger){
      fTrigger->InitBranch(LocalTree);
      cout<< "Trigger Tree ok"<<endl;
    }
    if(MW){
      fMW->InitBranch(LocalTree);
      cout<< "MW Tree ok"<<endl;
    }
    if(CsI){
      fCsI->InitBranch(LocalTree);
      cout<< "Diamant Tree ok"<<endl;
    }
    if(Neda){
      fNeda->InitBranch(LocalTree);
      cout<< "NWall Tree ok"<<endl;
    }
    if(Paris){
      fParis->InitBranch(LocalTree);
      cout<< "Paris Tree ok"<<endl;
    }
    if(Generic){
      fGeneric->InitBranch(LocalTree);
      cout<< "Generic Tree ok"<<endl;
    }
    if(EbyE){
      fEbyE->InitBranch(LocalTree);
      cout<<"EBYEDAT Tree ok"<<endl;
    }
    if(VamosICb){
      fVamosIC->InitBranch(LocalTree);
      cout<<" VamosIC Tree ok"<<endl;
    }
    printf("\033[31mInfo::  Root Tree Init done\033[m \n");
  }
  else if(!RawTreeb && !fNfsExoAnaTree) {
    printf("\033[31mInfo::  No Branch attached  \033[m \n");
  }

  if(LocalTree) LocalTree->Show();
  if(RawTree) RawTree->Show();
  if(NfsTree) NfsTree->Show();
  if(NfsMult3Tree) NfsMult3Tree->Show();

}

TString GUser::BuildNfsTreeFileName(Char_t *nameF)
{
  TString nfsName(nameF);
  Ssiz_t slash = nfsName.Last('/');
  if(slash == kNPOS) nfsName.Prepend("nfs_");
  else nfsName.Insert(slash+1,"nfs_");
  return nfsName;
}

TString GUser::BuildNfsMult3TreeFileName(Char_t *nameF)
{
  // EN: File prefix is kept as mult3_ so existing ADNE/fission helper scripts remain compatible.
  // CN: 文件前缀暂时保留 mult3_，以兼容已有 ADNE/fission 辅助脚本。
  TString mult3Name = BuildNfsTreeFileName(nameF);
  Ssiz_t slash = mult3Name.Last('/');
  if(slash == kNPOS) mult3Name.Prepend("mult3_");
  else mult3Name.Insert(slash+1,"mult3_");
  return mult3Name;
}

void GUser::InitRawTreeBranches()
{
  // EN: One RawTree entry is one top-level MFM event as delivered by the input file.
  // CN: RawTree 的一个 entry 对应输入文件已经组好的一个顶层 MFM event。
  RawTreeFillBool=true;
  fRawEventCounter=0;
  RawTree=new TTree("RawTree","Raw MFM event tree");
  RawTree->Branch("raw_event_index",&fRawEventIndex,"raw_event_index/l");
  RawTree->Branch("raw_top_frame_type",&fRawTopFrameType,"raw_top_frame_type/I");
  RawTree->Branch("raw_top_data_source",&fRawTopDataSource,"raw_top_data_source/I");
  RawTree->Branch("raw_top_frame_size",&fRawTopFrameSize,"raw_top_frame_size/I");
  RawTree->Branch("raw_top_nb_items",&fRawTopNbItems,"raw_top_nb_items/I");
  RawTree->Branch("raw_top_event_number",&fRawTopEventNumber,"raw_top_event_number/i");
  RawTree->Branch("raw_top_timestamp",&fRawTopTimeStamp,"raw_top_timestamp/l");
  RawTree->Branch("raw_event_bytes",&fRawEventBytes);
  RawTree->Branch("raw_frame_depth",&fRawFrameDepth);
  RawTree->Branch("raw_frame_index_in_parent",&fRawFrameIndexInParent);
  RawTree->Branch("raw_frame_type",&fRawFrameType);
  RawTree->Branch("raw_frame_data_source",&fRawFrameDataSource);
  RawTree->Branch("raw_frame_revision",&fRawFrameRevision);
  RawTree->Branch("raw_frame_size",&fRawFrameSize);
  RawTree->Branch("raw_frame_header_size",&fRawFrameHeaderSize);
  RawTree->Branch("raw_frame_unit_block_size",&fRawFrameUnitBlockSize);
  RawTree->Branch("raw_frame_nb_items",&fRawFrameNbItems);
  RawTree->Branch("raw_frame_board_id",&fRawFrameBoardId);
  RawTree->Branch("raw_frame_channel_id",&fRawFrameChannelId);
  RawTree->Branch("raw_frame_event_number",&fRawFrameEventNumber);
  RawTree->Branch("raw_frame_timestamp",&fRawFrameTimeStamp);
  RawTree->Branch("raw_exo_cristal_id",&fRawExoCristalId);
  RawTree->Branch("raw_exo_tg_cristal_id",&fRawExoTGCristalId);
  RawTree->Branch("raw_exo_board_id",&fRawExoBoardId);
  RawTree->Branch("raw_exo_status1",&fRawExoStatus1);
  RawTree->Branch("raw_exo_status2",&fRawExoStatus2);
  RawTree->Branch("raw_exo_status3",&fRawExoStatus3);
  RawTree->Branch("raw_exo_delta_t",&fRawExoDeltaT);
  RawTree->Branch("raw_exo_inner_m6",&fRawExoInnerM6);
  RawTree->Branch("raw_exo_inner_m20",&fRawExoInnerM20);
  RawTree->Branch("raw_exo_outer1",&fRawExoOuter1);
  RawTree->Branch("raw_exo_outer2",&fRawExoOuter2);
  RawTree->Branch("raw_exo_outer3",&fRawExoOuter3);
  RawTree->Branch("raw_exo_outer4",&fRawExoOuter4);
  RawTree->Branch("raw_exo_bgo",&fRawExoBGO);
  RawTree->Branch("raw_exo_csi",&fRawExoCSI);
  RawTree->Branch("raw_exo_inner_t30",&fRawExoInnerT30);
  RawTree->Branch("raw_exo_inner_t60",&fRawExoInnerT60);
  RawTree->Branch("raw_exo_inner_t90",&fRawExoInnerT90);
  RawTree->Branch("raw_exo_padding",&fRawExoPadding);
  RawTree->Branch("raw_diamant_cristal_id",&fRawDiamantCristalId);
  RawTree->Branch("raw_diamant_board_id",&fRawDiamantBoardId);
  RawTree->Branch("raw_diamant_channel_id",&fRawDiamantChannelId);
  RawTree->Branch("raw_diamant_status1",&fRawDiamantStatus1);
  RawTree->Branch("raw_diamant_status2",&fRawDiamantStatus2);
  RawTree->Branch("raw_diamant_energy",&fRawDiamantEnergy);
  RawTree->Branch("raw_diamant_top",&fRawDiamantTop);
  RawTree->Branch("raw_neda_board_id",&fRawNedaBoardId);
  RawTree->Branch("raw_neda_channel_id",&fRawNedaChannelId);
  RawTree->Branch("raw_neda_location_id",&fRawNedaLocationId);
  RawTree->Branch("raw_neda_le_interval",&fRawNedaLeInterval);
  RawTree->Branch("raw_neda_zco_interval",&fRawNedaZcoInterval);
  RawTree->Branch("raw_neda_tdc_value",&fRawNedaTdcValue);
  RawTree->Branch("raw_neda_slow_integral",&fRawNedaSlowIntegral);
  RawTree->Branch("raw_neda_fast_integral",&fRawNedaFastIntegral);
  RawTree->Branch("raw_neda_bitfield",&fRawNedaBitfield);
  RawTree->Branch("raw_neda_abs_max",&fRawNedaAbsMax);
  RawTree->Branch("raw_neda_interpol_cfd",&fRawNedaInterpolCFD);
  RawTree->Branch("raw_neda_comp_energy",&fRawNedaCompEnergy);
  RawTree->Branch("raw_neda_comp_time",&fRawNedaCompTime);
  RawTree->Branch("raw_neda_comp_tdc_cor_value",&fRawNedaCompTdcCorValue);
  RawTree->Branch("raw_neda_comp_slow_integral",&fRawNedaCompSlowIntegral);
  RawTree->Branch("raw_neda_comp_fast_integral",&fRawNedaCompFastIntegral);
  RawTree->Branch("raw_neda_comp_int_raise_time",&fRawNedaCompIntRaiseTime);
  RawTree->Branch("raw_neda_comp_neural_network",&fRawNedaCompNeuralNetWork);
  RawTree->Branch("raw_neda_comp_nb_zero",&fRawNedaCompNbZero);
  RawTree->Branch("raw_neda_comp_neutron_flag",&fRawNedaCompNeutronFlag);
  RawTree->Branch("raw_paris_cristal_id",&fRawParisCristalId);
  RawTree->Branch("raw_paris_board_id",&fRawParisBoardId);
  RawTree->Branch("raw_paris_channel_id",&fRawParisChannelId);
  RawTree->Branch("raw_paris_qshort",&fRawParisQShort);
  RawTree->Branch("raw_paris_qlong",&fRawParisQLong);
  RawTree->Branch("raw_paris_cfd",&fRawParisCfd);
  RawTree->Branch("raw_paris_pll",&fRawParisPLL);
  RawTree->Branch("raw_paris_pur",&fRawParisPUR);
  RawTree->Branch("raw_paris_ovr",&fRawParisOVR);
  RawTree->Branch("raw_generic_cristal_id",&fRawGenericCristalId);
  RawTree->Branch("raw_generic_board_id",&fRawGenericBoardId);
  RawTree->Branch("raw_generic_channel_id",&fRawGenericChannelId);
  RawTree->Branch("raw_generic_status1",&fRawGenericStatus1);
  RawTree->Branch("raw_generic_status2",&fRawGenericStatus2);
  RawTree->Branch("raw_generic_type_tns",&fRawGenericTypeTns);
  RawTree->Branch("raw_generic_energy",&fRawGenericEnergy);
  RawTree->Branch("raw_generic_time",&fRawGenericTime);
  RawTree->Branch("raw_vamos_cristal_id",&fRawVamosCristalId);
  RawTree->Branch("raw_vamos_board_id",&fRawVamosBoardId);
  RawTree->Branch("raw_vamos_channel_id",&fRawVamosChannelId);
  RawTree->Branch("raw_vamos_status1",&fRawVamosStatus1);
  RawTree->Branch("raw_vamos_status2",&fRawVamosStatus2);
  RawTree->Branch("raw_vamos_energy",&fRawVamosEnergy);
  RawTree->Branch("raw_ebye_frame_index",&fRawEbyEFrameIndex);
  RawTree->Branch("raw_ebye_label",&fRawEbyELabel);
  RawTree->Branch("raw_ebye_value",&fRawEbyEValue);
}

void GUser::ClearRawTreeEvent()
{
  fRawEventBytes.clear();
  fRawFrameDepth.clear();
  fRawFrameIndexInParent.clear();
  fRawFrameType.clear();
  fRawFrameDataSource.clear();
  fRawFrameRevision.clear();
  fRawFrameSize.clear();
  fRawFrameHeaderSize.clear();
  fRawFrameUnitBlockSize.clear();
  fRawFrameNbItems.clear();
  fRawFrameBoardId.clear();
  fRawFrameChannelId.clear();
  fRawFrameEventNumber.clear();
  fRawFrameTimeStamp.clear();
  fRawExoCristalId.clear();
  fRawExoTGCristalId.clear();
  fRawExoBoardId.clear();
  fRawExoStatus1.clear();
  fRawExoStatus2.clear();
  fRawExoStatus3.clear();
  fRawExoDeltaT.clear();
  fRawExoInnerM6.clear();
  fRawExoInnerM20.clear();
  fRawExoOuter1.clear();
  fRawExoOuter2.clear();
  fRawExoOuter3.clear();
  fRawExoOuter4.clear();
  fRawExoBGO.clear();
  fRawExoCSI.clear();
  fRawExoInnerT30.clear();
  fRawExoInnerT60.clear();
  fRawExoInnerT90.clear();
  fRawExoPadding.clear();
  fRawDiamantCristalId.clear();
  fRawDiamantBoardId.clear();
  fRawDiamantChannelId.clear();
  fRawDiamantStatus1.clear();
  fRawDiamantStatus2.clear();
  fRawDiamantEnergy.clear();
  fRawDiamantTop.clear();
  fRawNedaBoardId.clear();
  fRawNedaChannelId.clear();
  fRawNedaLocationId.clear();
  fRawNedaLeInterval.clear();
  fRawNedaZcoInterval.clear();
  fRawNedaTdcValue.clear();
  fRawNedaSlowIntegral.clear();
  fRawNedaFastIntegral.clear();
  fRawNedaBitfield.clear();
  fRawNedaAbsMax.clear();
  fRawNedaInterpolCFD.clear();
  fRawNedaCompEnergy.clear();
  fRawNedaCompTime.clear();
  fRawNedaCompTdcCorValue.clear();
  fRawNedaCompSlowIntegral.clear();
  fRawNedaCompFastIntegral.clear();
  fRawNedaCompIntRaiseTime.clear();
  fRawNedaCompNeuralNetWork.clear();
  fRawNedaCompNbZero.clear();
  fRawNedaCompNeutronFlag.clear();
  fRawParisCristalId.clear();
  fRawParisBoardId.clear();
  fRawParisChannelId.clear();
  fRawParisQShort.clear();
  fRawParisQLong.clear();
  fRawParisCfd.clear();
  fRawParisPLL.clear();
  fRawParisPUR.clear();
  fRawParisOVR.clear();
  fRawGenericCristalId.clear();
  fRawGenericBoardId.clear();
  fRawGenericChannelId.clear();
  fRawGenericStatus1.clear();
  fRawGenericStatus2.clear();
  fRawGenericTypeTns.clear();
  fRawGenericEnergy.clear();
  fRawGenericTime.clear();
  fRawVamosCristalId.clear();
  fRawVamosBoardId.clear();
  fRawVamosChannelId.clear();
  fRawVamosStatus1.clear();
  fRawVamosStatus2.clear();
  fRawVamosEnergy.clear();
  fRawEbyEFrameIndex.clear();
  fRawEbyELabel.clear();
  fRawEbyEValue.clear();
}

void GUser::PushRawDetectorDefaults()
{
  fRawExoCristalId.push_back(-1);
  fRawExoTGCristalId.push_back(-1);
  fRawExoBoardId.push_back(-1);
  fRawExoStatus1.push_back(-1);
  fRawExoStatus2.push_back(-1);
  fRawExoStatus3.push_back(-1);
  fRawExoDeltaT.push_back(-1);
  fRawExoInnerM6.push_back(-1);
  fRawExoInnerM20.push_back(-1);
  fRawExoOuter1.push_back(-1);
  fRawExoOuter2.push_back(-1);
  fRawExoOuter3.push_back(-1);
  fRawExoOuter4.push_back(-1);
  fRawExoBGO.push_back(-1);
  fRawExoCSI.push_back(-1);
  fRawExoInnerT30.push_back(-1);
  fRawExoInnerT60.push_back(-1);
  fRawExoInnerT90.push_back(-1);
  fRawExoPadding.push_back(-1);
  fRawDiamantCristalId.push_back(-1);
  fRawDiamantBoardId.push_back(-1);
  fRawDiamantChannelId.push_back(-1);
  fRawDiamantStatus1.push_back(-1);
  fRawDiamantStatus2.push_back(-1);
  fRawDiamantEnergy.push_back(-1);
  fRawDiamantTop.push_back(-1);
  fRawNedaBoardId.push_back(-1);
  fRawNedaChannelId.push_back(-1);
  fRawNedaLocationId.push_back(-1);
  fRawNedaLeInterval.push_back(-1);
  fRawNedaZcoInterval.push_back(-1);
  fRawNedaTdcValue.push_back(-1);
  fRawNedaSlowIntegral.push_back(-1);
  fRawNedaFastIntegral.push_back(-1);
  fRawNedaBitfield.push_back(-1);
  fRawNedaAbsMax.push_back(-1);
  fRawNedaInterpolCFD.push_back(-1);
  fRawNedaCompEnergy.push_back(-1);
  fRawNedaCompTime.push_back(-1);
  fRawNedaCompTdcCorValue.push_back(-1);
  fRawNedaCompSlowIntegral.push_back(-1);
  fRawNedaCompFastIntegral.push_back(-1);
  fRawNedaCompIntRaiseTime.push_back(-1);
  fRawNedaCompNeuralNetWork.push_back(-1);
  fRawNedaCompNbZero.push_back(-1);
  fRawNedaCompNeutronFlag.push_back(-1);
  fRawParisCristalId.push_back(-1);
  fRawParisBoardId.push_back(-1);
  fRawParisChannelId.push_back(-1);
  fRawParisQShort.push_back(-1);
  fRawParisQLong.push_back(-1);
  fRawParisCfd.push_back(-1.0);
  fRawParisPLL.push_back(-1);
  fRawParisPUR.push_back(-1);
  fRawParisOVR.push_back(-1);
  fRawGenericCristalId.push_back(-1);
  fRawGenericBoardId.push_back(-1);
  fRawGenericChannelId.push_back(-1);
  fRawGenericStatus1.push_back(-1);
  fRawGenericStatus2.push_back(-1);
  fRawGenericTypeTns.push_back(-1);
  fRawGenericEnergy.push_back(-1);
  fRawGenericTime.push_back(-1);
  fRawVamosCristalId.push_back(-1);
  fRawVamosBoardId.push_back(-1);
  fRawVamosChannelId.push_back(-1);
  fRawVamosStatus1.push_back(-1);
  fRawVamosStatus2.push_back(-1);
  fRawVamosEnergy.push_back(-1);
}

void GUser::CaptureRawEvent(MFMCommonFrame *frame)
{
  ClearRawTreeEvent();
  fRawEventIndex=fRawEventCounter++;
  frame->SetAttributs();
  fRawTopFrameType=frame->GetFrameType();
  fRawTopDataSource=frame->GetDataSource();
  fRawTopFrameSize=frame->GetFrameSize();
  fRawTopEventNumber=frame->GetEventNumber();
  fRawTopTimeStamp=frame->GetTimeStamp();
  fRawTopNbItems=-1;
  if(fRawTopFrameType == MFM_MERGE_EN_FRAME_TYPE || fRawTopFrameType == MFM_MERGE_TS_FRAME_TYPE){
    MFMMergeFrame mergeFrame;
    mergeFrame.SetAttributs(frame->GetPointHeader());
    fRawTopNbItems=mergeFrame.GetNbItems();
  }

  if(fRawTopFrameSize>0 && frame->GetPointHeader()!=NULL){
    UChar_t *begin = reinterpret_cast<UChar_t*>(frame->GetPointHeader());
    fRawEventBytes.assign(begin, begin + fRawTopFrameSize);
  }

  CaptureRawFrame(frame,0,0);
}

void GUser::CaptureRawFrame(MFMCommonFrame *frame, Int_t depth, Int_t indexInParent)
{
  if(frame==NULL) return;
  frame->SetAttributs();
  Int_t frameIndex=fRawFrameType.size();
  fRawFrameDepth.push_back(depth);
  fRawFrameIndexInParent.push_back(indexInParent);
  fRawFrameType.push_back(frame->GetFrameType());
  fRawFrameDataSource.push_back(frame->GetDataSource());
  fRawFrameRevision.push_back(frame->GetRevision());
  fRawFrameSize.push_back(frame->GetFrameSize());
  fRawFrameHeaderSize.push_back(frame->GetHeaderSize());
  fRawFrameUnitBlockSize.push_back(frame->GetUnitBlockSize());
  fRawFrameNbItems.push_back(-1);
  fRawFrameBoardId.push_back(-1);
  fRawFrameChannelId.push_back(-1);
  fRawFrameEventNumber.push_back(frame->GetEventNumber());
  fRawFrameTimeStamp.push_back(frame->GetTimeStamp());
  PushRawDetectorDefaults();

  if(frame->GetFrameType() == MFM_MERGE_EN_FRAME_TYPE || frame->GetFrameType() == MFM_MERGE_TS_FRAME_TYPE){
    MFMMergeFrame mergeFrame;
    MFMCommonFrame insideFrame;
    mergeFrame.SetAttributs(frame->GetPointHeader());
    fRawFrameNbItems.back()=mergeFrame.GetNbItems();
    mergeFrame.ResetReadInMem();
    for(Int_t i=0;i<mergeFrame.GetNbItems();i++){
      mergeFrame.ReadInFrame(&insideFrame);
      CaptureRawFrame(&insideFrame,depth+1,i);
    }
    return;
  }

  switch(frame->GetFrameType()){
    case MFM_EXO2_FRAME_TYPE:
      pExogamFrame->SetAttributs(frame->GetPointHeader());
      fRawFrameBoardId.back()=pExogamFrame->GetBoardId();
      fRawFrameChannelId.back()=pExogamFrame->ExoGetTGCristalId();
      fRawExoCristalId.back()=pExogamFrame->ExoGetCristalId();
      fRawExoTGCristalId.back()=pExogamFrame->ExoGetTGCristalId();
      fRawExoBoardId.back()=pExogamFrame->ExoGetBoardId();
      fRawExoStatus1.back()=pExogamFrame->ExoGetStatus(0);
      fRawExoStatus2.back()=pExogamFrame->ExoGetStatus(1);
      fRawExoStatus3.back()=pExogamFrame->ExoGetStatus(2);
      fRawExoDeltaT.back()=pExogamFrame->ExoGetDeltaT();
      fRawExoInnerM6.back()=pExogamFrame->ExoGetInnerM(0);
      fRawExoInnerM20.back()=pExogamFrame->ExoGetInnerM(1);
      fRawExoOuter1.back()=pExogamFrame->ExoGetOuter(0);
      fRawExoOuter2.back()=pExogamFrame->ExoGetOuter(1);
      fRawExoOuter3.back()=pExogamFrame->ExoGetOuter(2);
      fRawExoOuter4.back()=pExogamFrame->ExoGetOuter(3);
      fRawExoBGO.back()=pExogamFrame->ExoGetBGO();
      fRawExoCSI.back()=pExogamFrame->ExoGetCsi();
      fRawExoInnerT30.back()=pExogamFrame->ExoGetInnerT(0);
      fRawExoInnerT60.back()=pExogamFrame->ExoGetInnerT(1);
      fRawExoInnerT90.back()=pExogamFrame->ExoGetInnerT(2);
      fRawExoPadding.back()=pExogamFrame->ExoGetPadding();
      break;
    case MFM_DIAMANT_FRAME_TYPE:
      pDiamantFrame->SetAttributs(frame->GetPointHeader());
      fRawFrameBoardId.back()=pDiamantFrame->GetBoardId();
      fRawFrameChannelId.back()=pDiamantFrame->GetChannelId();
      fRawDiamantCristalId.back()=pDiamantFrame->GetCristalId();
      fRawDiamantBoardId.back()=pDiamantFrame->GetBoardId();
      fRawDiamantChannelId.back()=pDiamantFrame->GetChannelId();
      fRawDiamantStatus1.back()=pDiamantFrame->GetStatus(0);
      fRawDiamantStatus2.back()=pDiamantFrame->GetStatus(1);
      fRawDiamantEnergy.back()=pDiamantFrame->GetEnergy();
      fRawDiamantTop.back()=pDiamantFrame->GetTop();
      break;
    case MFM_NEDA_FRAME_TYPE:
      pNedaFrame->SetAttributs(frame->GetPointHeader());
      fRawFrameBoardId.back()=pNedaFrame->GetBoardId();
      fRawFrameChannelId.back()=pNedaFrame->GetChannelId();
      fRawNedaBoardId.back()=pNedaFrame->GetBoardId();
      fRawNedaChannelId.back()=pNedaFrame->GetChannelId();
      fRawNedaLocationId.back()=pNedaFrame->GetLocationId();
      fRawNedaLeInterval.back()=pNedaFrame->GetLeInterval();
      fRawNedaZcoInterval.back()=pNedaFrame->GetZcoInterval();
      fRawNedaTdcValue.back()=pNedaFrame->GetTdcValue();
      fRawNedaSlowIntegral.back()=pNedaFrame->GetSlowIntegral();
      fRawNedaFastIntegral.back()=pNedaFrame->GetFastIntegral();
      fRawNedaBitfield.back()=pNedaFrame->GetBitfield();
      fRawNedaAbsMax.back()=pNedaFrame->GetAbsMax();
      fRawNedaInterpolCFD.back()=pNedaFrame->GetInterpolCFD();
      break;
    case MFM_NEDACOMP_FRAME_TYPE:
      pNedaCompFrame->SetAttributs(frame->GetPointHeader());
      fRawFrameBoardId.back()=pNedaCompFrame->GetBoardId();
      fRawFrameChannelId.back()=pNedaCompFrame->GetChannelId();
      fRawNedaBoardId.back()=pNedaCompFrame->GetBoardId();
      fRawNedaChannelId.back()=pNedaCompFrame->GetChannelId();
      fRawNedaLocationId.back()=pNedaCompFrame->GetLocationId();
      fRawNedaCompEnergy.back()=pNedaCompFrame->GetEnergy();
      fRawNedaCompTime.back()=pNedaCompFrame->GetTime();
      fRawNedaCompTdcCorValue.back()=pNedaCompFrame->GetTdcCorValue();
      fRawNedaCompSlowIntegral.back()=pNedaCompFrame->GetSlowIntegral();
      fRawNedaCompFastIntegral.back()=pNedaCompFrame->GetFastIntegral();
      fRawNedaCompIntRaiseTime.back()=pNedaCompFrame->GetIntRaiseTime();
      fRawNedaCompNeuralNetWork.back()=pNedaCompFrame->GetNeuralNetWork();
      fRawNedaCompNbZero.back()=pNedaCompFrame->GetNbZero();
      fRawNedaCompNeutronFlag.back()=pNedaCompFrame->GetNeutronFlag();
      break;
    case MFM_PARIS_FRAME_TYPE:
      pParisFrame->SetAttributs(frame->GetPointHeader());
      fRawFrameBoardId.back()=pParisFrame->GetBoardId();
      fRawFrameChannelId.back()=pParisFrame->GetChannelId();
      fRawParisCristalId.back()=pParisFrame->GetCristalId();
      fRawParisBoardId.back()=pParisFrame->GetBoardId();
      fRawParisChannelId.back()=pParisFrame->GetChannelId();
      fRawParisQShort.back()=pParisFrame->GetQShort();
      fRawParisQLong.back()=pParisFrame->GetQLong();
      fRawParisCfd.back()=pParisFrame->GetCfd();
      fRawParisPLL.back()=pParisFrame->GetPLL();
      fRawParisPUR.back()=pParisFrame->GetPUR();
      fRawParisOVR.back()=pParisFrame->GetOVR();
      break;
    case MFM_REA_GENE_FRAME_TYPE:
      pGenericFrame->SetAttributs(frame->GetPointHeader());
      fRawFrameBoardId.back()=pGenericFrame->GetBoardId();
      fRawFrameChannelId.back()=pGenericFrame->GetChannelId();
      fRawGenericCristalId.back()=pGenericFrame->GetCristalId();
      fRawGenericBoardId.back()=pGenericFrame->GetBoardId();
      fRawGenericChannelId.back()=pGenericFrame->GetChannelId();
      fRawGenericStatus1.back()=pGenericFrame->GetStatus(0);
      fRawGenericStatus2.back()=pGenericFrame->GetStatus(1);
      fRawGenericTypeTns.back()=pGenericFrame->GetTypeTns();
      fRawGenericEnergy.back()=pGenericFrame->GetEnergy();
      fRawGenericTime.back()=pGenericFrame->GetTime();
      break;
    case MFM_VAMOSIC_FRAME_TYPE:
      pVamosICFrame->SetAttributs(frame->GetPointHeader());
      fRawFrameBoardId.back()=pVamosICFrame->GetBoardId();
      fRawFrameChannelId.back()=pVamosICFrame->GetChannelId();
      fRawVamosCristalId.back()=pVamosICFrame->GetCristalId();
      fRawVamosBoardId.back()=pVamosICFrame->GetBoardId();
      fRawVamosChannelId.back()=pVamosICFrame->GetChannelId();
      fRawVamosStatus1.back()=pVamosICFrame->GetStatus(0);
      fRawVamosStatus2.back()=pVamosICFrame->GetStatus(1);
      fRawVamosEnergy.back()=pVamosICFrame->GetEnergy();
      break;
    case MFM_EBY_EN_FRAME_TYPE:
    case MFM_EBY_TS_FRAME_TYPE:
    case MFM_EBY_EN_TS_FRAME_TYPE:
      pEbyedatFrame->SetAttributs(frame->GetPointHeader());
      fRawFrameNbItems.back()=pEbyedatFrame->GetNbItems();
      for(Int_t i=0;i<pEbyedatFrame->GetNbItems();i++){
        uint16_t label=0;
        uint16_t value=0;
        pEbyedatFrame->EbyedatGetParameters(i,&label,&value);
        fRawEbyEFrameIndex.push_back(frameIndex);
        fRawEbyELabel.push_back(label);
        fRawEbyEValue.push_back(value);
      }
      break;
    default:
      break;
  }
}
void GUser::Unpack(MFMCommonFrame *frame, bool debug)
{ 
char STOP[2];	
int size;
uint16_t label,value; 


	  frame->SetAttributs(); // fix_lsy: refresh common-frame attributes before timestamp access for recent MFMlib API
	  MFMKey->Fill(frame->GetFrameType());
	  if(frame->GetTimeStamp()==0){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
	  	printf("\033[31mError:: %d Frame Type has TS zero for board %d\033[m \n",frame->GetFrameType(),frame->GetBoardId());
		WhichMFM(frame->GetFrameType());
		TSZero++;
		return ;
	  }
	 //cerr<<"MFMKey "<<frame->GetFrameType()<<endl;
	  if(frame->GetFrameType() ==MFM_MERGE_EN_FRAME_TYPE || frame->GetFrameType() ==MFM_MERGE_TS_FRAME_TYPE){
	  	  frame->SetAttributs(); // fix_lsy: merge frame reads timestamp before counter; refresh attributes at branch entry
	          if(PrevTS>0){MergeFrameDeltaTS->Fill(frame->GetTimeStamp()-PrevTS);} // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
	  	  PrevTS=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		 
		 
		  if(debug)cerr<<"  "<<endl;
	  	  if(debug)cerr<<"GetTimeStamp  "<<frame->GetTimeStamp()<<endl; // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
	        PrevTSInception[InceptionLayer]=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		InceptionLayer++;
		if(InceptionLayer>99){
				printf("\033[31mInfo:: You have reach the level 100 of Inception Layers -> STOP (Please Adjust) \033[m \n");
				//gets(STOP);
		
		}
    		//----INCEPTION   pMergeFrame->SetAttributs(frame->GetPointHeader()); // it is a merged frame, so allocate to MergeFrame Class
    		pInceptionMergeFrame[InceptionLayer]->SetAttributs(frame->GetPointHeader());
		
		
		//----INCEPTION  if(debug) cerr<<pMergeFrame<<"  " <<pMergeFrame->GetFrameSize()<<endl;

		//----INCEPTION    noevent = pMergeFrame->GetEventNumber();
		Inceptionnoevent[InceptionLayer] =pInceptionMergeFrame[InceptionLayer]->GetEventNumber();
		
    		//----INCEPTION    nbinsideframe= pMergeFrame->GetNbItems(); //how many items within this merged frame ??
		Inceptionnbinsideframe[InceptionLayer]=pInceptionMergeFrame[InceptionLayer]->GetNbItems();
		
    		//----INCEPTION    pMergeFrame->ResetReadInMem(); //Place the pointer to the first item of the MultiFrame
		pInceptionMergeFrame[InceptionLayer]->ResetReadInMem();
		
    		CMFM_MERGE_TS_FRAME_TYPE++;
    		frame->SetAttributs(); // fix_lsy: refresh attributes after frame-type counter before timestamp use
    		//----INCEPTION    if(debug) cerr<<"Into composite with "<<pMergeFrame->GetNbItems()<<" sub frames "<<endl;
		if(debug) cerr<<"Inception Layer "<< InceptionLayer  <<"  Into composite with "<<pInceptionMergeFrame[InceptionLayer]->GetNbItems()<<" sub frames "<<endl;
		
		
		
    		for (int i = 0; i < Inceptionnbinsideframe[InceptionLayer]; i++) { //loop into the multiFrame merged    nbinsideframe
			//----INCEPTION    pMergeFrame->ReadInFrame(fInsideframe); //read the next frame within the multiframe
			pInceptionMergeFrame[InceptionLayer]->ReadInFrame(fInceptionInsideframe[InceptionLayer]);
			
	  		if(debug) cerr<<"nbinsideframe = : "<<i<<"  Inception Level " <<InceptionLayer <<endl;
			Unpack(fInceptionInsideframe[InceptionLayer],debug);
		 }
		 
		if(debug) cerr<<"Exit Recurcive layer  "<<InceptionLayer <<endl;
		InceptionLayer--;
	  }
	  else if(frame->GetFrameType() ==MFM_EXO2_FRAME_TYPE && frame->GetDataSource()==0){
    		pExogamFrame->SetAttributs(frame->GetPointHeader());
    		noevent = pExogamFrame->GetEventNumber();
    		CMFM_EXO2_FRAME_TYPE++;
    		frame->SetAttributs(); // fix_lsy: refresh attributes after frame-type counter before timestamp use
    		if(debug)cerr<<"pExogamFrame"<<endl;
    		Exogam2b=fExogam2->IsMFMExo(pExogamFrame);
    		if(fExogam2->BoolSpec)fExogam2->TimeStampDiff->Fill(frame->GetTimeStamp()-fExogam2->PrevTS); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
    		if(frame->GetTimeStamp()-fExogam2->PrevTS <0)printf("\033[31m *********  Time Stamp Error :: TExogam2 back in future !!! ********* \033[m \n"); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
    		fExogam2->PrevTS=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		if(debug)cerr<<"pExogamFrame TS "<<fExogam2->PrevTS<<endl;
		MergeDetectorFrameDeltaTS->Fill(fExogam2->PrevTS-PrevTSInception[0]);
		
		if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8<7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Fill((frame->GetTimeStamp()-TStart)/1e8); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		}
		else if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8>7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			TStart=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Reset();
		}
		else if (TStart==0){TStart=frame->GetTimeStamp();} // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		else {cerr<<"Alarm rate monitor :: Should be impossible"<<endl;}
  	}
   
  	else if(frame->GetFrameType() ==MFM_NEDA_FRAME_TYPE){
     		pNedaFrame->SetAttributs(frame->GetPointHeader());
     		CMFM_NEDA_FRAME_TYPE++;
     		frame->SetAttributs(); // fix_lsy: refresh attributes after frame-type counter before timestamp use
      		if(debug)cerr<<"pNedaFrame"<<endl;
     		NWallb=fNeda->IsMFMNeda(pNedaFrame);
     		if(fNeda->BoolSpec)fNeda->TimeStampDiff->Fill(frame->GetTimeStamp()-fNeda->PrevTS); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
     		if(frame->GetTimeStamp()-fNeda->PrevTS<0)printf("\033[31m *********  Time Stamp Error :: TNWall back in future !!! ********* \033[m \n"); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
     		fNeda->PrevTS=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		MergeDetectorFrameDeltaTS->Fill(fNeda->PrevTS-PrevTSInception[0]);
		if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8<7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Fill((frame->GetTimeStamp()-TStart)/1e8); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		}
		else if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8>7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			TStart=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Reset();
		}
		else if (TStart==0){TStart=frame->GetTimeStamp();} // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		else {cerr<<"Alarm rate monitor :: Should be impossible"<<endl;}
  
  	}
  	else if(frame->GetFrameType() == MFM_NEDACOMP_FRAME_TYPE){
     		pNedaCompFrame->SetAttributs(frame->GetPointHeader());
     		CMFM_NEDACOMPRESS_FRAME_TYPE++;
     		frame->SetAttributs(); // fix_lsy: refresh attributes after frame-type counter before timestamp use
      		if(debug)cerr<<"NEDA Compressed Frame"<<endl;
     		NWallb=fNeda->IsMFMNedaComp(pNedaCompFrame);
		if(debug)cerr<<"Set TS ?"<<endl;
		if(debug)cerr<<"Yes"<<endl;
     		if(fNeda->BoolSpec)fNeda->TimeStampDiff->Fill(frame->GetTimeStamp()-fNeda->PrevTS); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
     		if(debug)cerr<<"Filled"<<endl;
		if(frame->GetTimeStamp()-fNeda->PrevTS<0)printf("\033[31m *********  Time Stamp Error :: TNWall back in future !!! ********* \033[m \n"); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
     		fNeda->PrevTS=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		MergeDetectorFrameDeltaTS->Fill(fNeda->PrevTS-PrevTSInception[0]);
		if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8<7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Fill((frame->GetTimeStamp()-TStart)/1e8); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		}
		else if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8>7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			TStart=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Reset();
		}
		else if (TStart==0){TStart=frame->GetTimeStamp();} // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		else {cerr<<"Alarm rate monitor :: Should be impossible"<<endl;}
 	}	 
  	else if(frame->GetFrameType() ==MFM_DIAMANT_FRAME_TYPE){
    		pDiamantFrame->SetAttributs(frame->GetPointHeader());
    		CMFM_DIAMANT_FRAME_TYPE++;
    		frame->SetAttributs(); // fix_lsy: refresh attributes after frame-type counter before timestamp use
     		if(debug)cerr<<"pDiamantFrame"<<endl;
    		CsIb=fCsI->IsMFMDiamant(pDiamantFrame);
		
    		if(fCsI->BoolSpec)fCsI->TimeStampDiff->Fill(frame->GetTimeStamp()-fCsI->PrevTS); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
    		if(frame->GetTimeStamp()-fCsI->PrevTS<0)printf("\033[31m *********  Time Stamp Error :: TCsI back in future !!! ********* \033[m \n"); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
    		fCsI->PrevTS=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		MergeDetectorFrameDeltaTS->Fill(fCsI->PrevTS-PrevTSInception[0]);
		if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8<7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Fill((frame->GetTimeStamp()-TStart)/1e8); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		}
		else if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8>7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			TStart=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Reset();
		}
		else if (TStart==0){TStart=frame->GetTimeStamp();} // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		else {cerr<<"Alarm rate monitor :: Should be impossible"<<endl;}
  	}
	
	
 	else if(frame->GetFrameType() ==MFM_PARIS_FRAME_TYPE){          ///////////// was pCommonFrame->GetFrameType()
    		if(debug)cerr<<"PARIS Frame"<<endl;
    		if(debug)cerr<<"pCommonFrame->GetFrameSize()::  "<<frame->GetFrameSize()<<endl;
		pParisFrame->SetAttributs(frame->GetPointHeader());
		CMFM_PARIS_FRAME_TYPE++;
		frame->SetAttributs(); // fix_lsy: refresh attributes after frame-type counter before timestamp use
		Parisb=fParis->IsMFMParis(pParisFrame);
		if(debug)cerr<<"Paris Time Local "<<fParis->PrevTS<<endl;
		MergeDetectorFrameDeltaTS->Fill(fParis->PrevTS-PrevTSInception[0]);
		if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8<7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Fill((frame->GetTimeStamp()-TStart)/1e8); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		}
		else if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8>7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			TStart=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Reset();
		}
		else if (TStart==0){TStart=frame->GetTimeStamp();} // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		else {cerr<<"Alarm rate monitor :: Should be impossible"<<endl;}
  	}
	else if(frame->GetFrameType() ==MFM_REA_GENE_FRAME_TYPE && frame->GetDataSource()==0){
		if(debug)cerr<<"Generic Frame"<<endl;
		pGenericFrame->SetAttributs(frame->GetPointHeader());
		CMFM_GENERIC_FRAME_TYPE++;
		frame->SetAttributs(); // fix_lsy: refresh attributes after frame-type counter before timestamp use
		Genericb=fGeneric->IsMFMGeneric(pGenericFrame);
		fGeneric->PrevTS=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		if(debug)cerr<<"Time Local "<<fGeneric->PrevTS<<endl;
		MergeDetectorFrameDeltaTS->Fill(fGeneric->PrevTS-PrevTSInception[0]);
		if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8<7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Fill((frame->GetTimeStamp()-TStart)/1e8); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		}
		else if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8>7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			TStart=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Reset();
		}
		else if (TStart==0){TStart=frame->GetTimeStamp();} // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		else {cerr<<"Alarm rate monitor :: Should be impossible"<<endl;}
	}
	
	else if(frame->GetFrameType() ==MFM_VAMOSIC_FRAME_TYPE){
		if(debug)cerr<<"VAMOSIC Frame"<<endl;
		pVamosICFrame->SetAttributs(frame->GetPointHeader());
		CMFM_VAMOSIC_FRAME_TYPE++;
		frame->SetAttributs(); // fix_lsy: refresh attributes after frame-type counter before timestamp use
		VamosICb=fVamosIC->IsMFMVamosIC(pVamosICFrame);
		fVamosIC->PrevTS=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		if(debug)cerr<<"Time Local "<<fVamosIC->PrevTS<<endl;
		MergeDetectorFrameDeltaTS->Fill(fVamosIC->PrevTS-PrevTSInception[0]);
		if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8<7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Fill((frame->GetTimeStamp()-TStart)/1e8); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		}
		else if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8>7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			TStart=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Reset();
		}
		else if (TStart==0){TStart=frame->GetTimeStamp();} // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		else {cerr<<"Alarm rate monitor :: Should be impossible"<<endl;}
	}
	
	
	else if(frame->GetFrameType() ==MFM_REA_GENE_FRAME_TYPE && frame->GetDataSource()==1){ //EXOGAM encoded in REA
		if(debug)cerr<<"Unpacker Infos :: Generic Frame for EXOGAM"<<endl;
		
		pGenericFrame->SetAttributs(frame->GetPointHeader());
		CMFM_EXO2REA_FRAME_TYPE++;
		frame->SetAttributs(); // fix_lsy: refresh attributes after frame-type counter before timestamp use
		Exogam2REAb=fExogam2REA->IsMFMExo(pGenericFrame);
		
		
		fExogam2REA->PrevTS=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		if(debug)cerr<<"Time Local "<<fExogam2REA->PrevTS<<"  TStart "<< TStart<<" PrevTSInception[0] "<<PrevTSInception[0]<<endl;
		MergeDetectorFrameDeltaTS->Fill(fExogam2REA->PrevTS-PrevTSInception[0]);

		if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8<7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Fill((frame->GetTimeStamp()-TStart)/1e8); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		}
		else if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8>7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			TStart=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Reset();
		}
		else if (TStart==0){TStart=frame->GetTimeStamp();} // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		else {cerr<<"Alarm rate monitor :: Should be impossible"<<endl;}
				
	}
	else if(frame->GetFrameType() ==MFM_EBY_EN_FRAME_TYPE || frame->GetFrameType() == MFM_EBY_TS_FRAME_TYPE ||frame->GetFrameType() == MFM_EBY_EN_TS_FRAME_TYPE ){
	
		pEbyedatFrame->SetAttributs(frame->GetPointHeader());
		//size=pEbyedatFrame->GetNbItemsAttribut();
		size=pEbyedatFrame->GetNbItems();
		//size=pEbyedatFrame->GetItemSizeFromStructure();
		CMFM_EBY_EN_FRAME_TYPE++;
		frame->SetAttributs(); // fix_lsy: refresh attributes after frame-type counter before timestamp use
		fEbyE->GetEbyEData()->SetEbyETimeStamps(frame->GetTimeStamp()); //push the Header TS EbyEdat from AGAVA into the class // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		for (Int_t i = 0; i < size; i++) { //unpack event
	  		pEbyedatFrame->EbyedatGetParameters(i,&label,&value); 
	  		//cerr<<i<<" Label::  "<<label<<"  Data::  "<< value<<endl;
	  		EbyEb=fEbyE->IsMFMEbyE((UShort_t)label,(Short_t)value);
		}
		if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8<7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Fill((frame->GetTimeStamp()-TStart)/1e8); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		}
		else if(TStart>0&&(frame->GetTimeStamp()-TStart)/1e8>7200){ // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			TStart=frame->GetTimeStamp(); // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
			rate->Reset();
		}
		else if (TStart==0){TStart=frame->GetTimeStamp();} // fix_lsy: API update from removed GetTimeStampFromCommonFrameData()
		else {cerr<<"Alarm rate monitor :: Should be impossible"<<endl;}
	
	}
	
	else{
		//cerr<<"Unknown Frame  :: "<<frame->GetFrameType()<<endl;
	}
	
	if(debug) cerr<<"Return Recurcive layer  "<< InceptionLayer <<endl;
	return ;
}

void GUser::WhichMFM(uint16_t type)
{

	switch(type){
		
	case MFM_COBO_FRAME_TYPE     	:	cerr<<"---  Cobo card frame	"<<endl;break;	   
	case MFM_COBOF_FRAME_TYPE    	:	cerr<<"---  Cobo card frame full signals "<<endl; break;   
	case MFM_COBOT_FRAME_TYPE    	:	cerr<<"---  Cobo topology frame	"<<endl;break;	   
	case MFM_MUTANT_FRAME_TYPE   	:	cerr<<"---  Mutant Data Frame"<<endl;break;		   
	case MFM_MUTANT1_FRAME_TYPE  	:	cerr<<"---  Mutant Data Frame Reserved"<<endl;break;	   
	case MFM_MUTANT2_FRAME_TYPE  	:	cerr<<"---  Mutant Data Frame Reserved"<<endl;break;	   
	case MFM_MUTANT3_FRAME_TYPE  	:	cerr<<"---  Mutant Data Frame Reserved"<<endl;break;	   
	case MFM_EXO2_FRAME_TYPE     	:	cerr<<"---  numexo2 card frame	"<<endl;break;	   
	case MFM_OSCI_FRAME_TYPE     	:	cerr<<"---  Oscillo data frame use in Numexo2"<<endl;break;
	case MFM_NEDA_FRAME_TYPE     	:	cerr<<"---  Raw data frame use in NEDA	"<<endl;  break; 
	case MFM_NEDACOMP_FRAME_TYPE 	:	cerr<<"---  Compressed data frame used in NED"<<endl;break;
	case MFM_VAMOSIC_FRAME_TYPE  	:	cerr<<"---  Vamos Ionization Chamber Frame  "<<endl; break;
	case MFM_VAMOSPD_FRAME_TYPE  	:	cerr<<"---  Vamos Position Detector Frame "<<endl;  break; 
	case MFM_DIAMANT_FRAME_TYPE  	:	cerr<<"---  Diamant Frame "<<endl;		   break;
	case MFM_S3_BAF2_FRAME_TYPE  	:	cerr<<"---  for detector S3 Baf2 Frame"<<endl;break;	   
	case MFM_S3_ALPHA_FRAME_TYPE 	:	cerr<<"---  for detector S3 Alpha Frame"<<endl;break;	   
	case MFM_S3_RUTH_FRAME_TYPE  	:	cerr<<"---  for detector S3 Rutherford Frame "<<endl;break;
	case MFM_S3_EGUN_FRAME_TYPE  	:	cerr<<"---  for detector S3 eGUN  	"<<endl;   break;
	case MFM_S3_SYNC_FRAME_TYPE  	:	cerr<<"---  for detector S3 Synchro	 "<<endl;  break;
	case MFM_REA_GENE_FRAME_TYPE 	:	cerr<<"---  Generic Rea Frame"<<endl;break;		   
	case MFM_VAMOSTAC_FRAME_TYPE 	:	cerr<<"---  Vamos Time  Frame"<<endl;break;		   
	case MFM_BOX_DIAG_FRAME_TYPE 	:	cerr<<"---  Box Diagnostic	"<<endl;break;	   
	case MFM_EBY_EN_FRAME_TYPE   	:	cerr<<"---  Ganil data frame with event number"<<endl;break;
	case MFM_EBY_TS_FRAME_TYPE   	:	cerr<<"---  Ganil data frame with time stamp"<<endl; break;
	case MFM_EBY_EN_TS_FRAME_TYPE   : 	cerr<<"---  Ganil data frame with TS & EN"<<endl;break;
	case MFM_MATACQ_FRAME_TYPE      : 	cerr<<"---  Mataq card frame"<<endl;	     break;  
	case MFM_SCALER_DATA_FRAME_TYPE : 	cerr<<"---  Ganil Scaler data frame  "<<endl;    break;
	case MFM_RIBF_DATA_FRAME_TYPE    :	cerr<<"---  RIBF data frame	"<<endl;       break;
	case MFM_FAZIA_DATA_FRAME_TYPE  : 	cerr<<"---  FAZIA data frame"<<endl;	break;       
	case MFM_CHIMERA_DATA_FRAME_TYPE :	cerr<<"---  CHIMERA data frame"<<endl;	break;       
	case MFM_SIRIUS_FRAME_TYPE       :	cerr<<"---  Sirius data frame"<<endl;	break;       
	case MFM_REA_TRACE_FRAME_TYPE    :	cerr<<"---  Trace  Rea Frame"<<endl;	break;       
	case MFM_S3_DEFLECTOR_FRAME_TYPE :	cerr<<"---  Deflector frame	 "<<endl;      break;
	case MFM_PARIS_FRAME_TYPE        :	cerr<<"---  PARIS     frame	"<<endl;       break;
	case MFM_HELLO_FRAME_TYPE     	:	cerr<<"---  Hello Frame "<<endl; 		break;
	case MFM_MERGE_EN_FRAME_TYPE 	:	cerr<<"---  Merge frame in envent number "<<endl;break;
	case MFM_MERGE_TS_FRAME_TYPE  	:	cerr<<"---  Merge frame in time stamp"<<endl;break;	
		default : cerr<<" Unknown Frame "<<endl; 
		break;
	}
}
void GUser::Analysis()
{
int detect1,detect2;
int DTS,DTS2, DTS3, DTSAlign;
bool coinc=false;
unsigned long long G_TS, G_TS2, G_TS3;
// A third level analysis can be done here involving all the TDetector Class
 	
	
	//cout<<" ************************* In MM "<<fExogam2->GetExogam2Data()->GetECCEMult() <<endl;
	G_TS=G_TS2=G_TS3=0;
	fExogam2->DTSAlign=0;
	if(
	//fGeneric->GetGenericData()->GetGenericMult()>0
	fExogam2->GetExogam2Data()->GetECCEMult()>0

	//&&fVamosIC->GetVamosICData()->GetVamosICMult()==2
	&&fGeneric->MultiplicityPlastic==2 // plastic
	//&&fVamosIC->Muliplicity==2
	//&&fGeneric->MuliplicitySec==1
	//&& fEbyE->MMMul==2
	&& fGeneric->PrompIons
	&& fEbyE->GMT_Trigger==8
	/*	&&
		fGeneric->G_TS2>0
		&&
		fGeneric->G_TS>0*/
		
	) //MUST2 fired
	
        {
	//cout<<" ************************* In MM "<<fExogam2->GetExogam2Data()->GetECCEMult() <<endl;	 
	
	/// G_TS=fGeneric->G_TS; //OPSA
	
	 //G_TS=fVamosIC->G_TS;
	 
	 
	 G_TS2=fGeneric->G_TS; //plastic ZDD
	 G_TS=fEbyE->GetEbyEData()->GetEbyETimeStamps(); //AGAVA for VXI = MUST2
	 //cout<<G_TS<<"  "<<G_TS2<<"  "<<G_TS3<<endl;
	 
	 DTS2=G_TS-G_TS2+321-120; //VXI ZDD Plastic 
	 if(G_TS>0&&G_TS2>0) DTS_test2->Fill(DTS2);
	 
	 /*DTS3=G_TS-G_TS3+27; //ZDD Plastic - AGAVA for VXI
	 if(G_TS>0&&G_TS3>0)DTS_test3->Fill(DTS3);
*/
	 /*fExogam2->noyau=fGeneric->NucleusId;
	 fExogam2->FreeParam1=fGeneric->FreeParam1;*/
	 
	/* fExogam2->Theta_P=0;
	 fExogam2->Phi_P=0;
	 
	 for(Int_t a = 0; a<16;a++){
	 	for(Int_t b = 0; b<16;b++){fExogam2->IsPromptPublic[a][b]=false;}
	 }
*/
	 if(fExogam2->GetExogam2Data()->GetECCEMult()>0){
		for(int u=0;u<fExogam2->GetExogam2Data()->GetECCEMult();u++){
		
			if(fExogam2->GetExogam2Data()->GetECCEEnergy(u)>100){
	 			detect1=fExogam2->GetExogam2Data()->GetECCEDetNbr(u);
				//DTS=fExogam2->GetExogam2Data()->GetfTimeStamps(detect1)-G_TS3+102; //Exo - AGAVA for VXI
	 			
				//if(fExogam2->GetExogam2Data()->GetfTimeStamps(detect1)>0&&G_TS3>0)DTS_test->Fill(DTS);
			
				DTSAlign=G_TS-fExogam2->GetExogam2Data()->GetfTimeStamps(detect1)+2;//VXI(MUST2) - EXOGAM indi
				if(fExogam2->GetExogam2Data()->GetfTimeStamps(detect1)>0&&G_TS>0){
				
				
					ExogamAlignement->Fill(detect1,DTSAlign);
					DTS_test->Fill(DTSAlign);
					/*coinc=true;
					fExogam2->IsPromptPublic[fExogam2->GetExogam2Data()->GetECCEClover(u)][fExogam2->GetExogam2Data()->GetECCECristal(u)]=true;
					fExogam2->DTSAlign=DTSAlign;*/
				}
				if(abs(DTSAlign)<5&&abs(DTS2<30)){
				//if(abs(DTS2<5)){
				//if(abs(DTSAlign)<10){
					coinc=true;
					//fExogam2->IsPromptPublic[fExogam2->GetExogam2Data()->GetECCEClover(u)][fExogam2->GetExogam2Data()->GetECCECristal(u)]=true;
					fExogam2->DTSAlign=DTSAlign;
				}
				
				
				
			}
		}
	 }
	 
	
	if(coinc){
		fExogam2->Treat();
		
		if(TreeFillBool)LocalTree->Fill(); 
	}
	 
 }


}
