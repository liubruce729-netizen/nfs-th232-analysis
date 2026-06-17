#include <cstdlib>
#include "fstream"
#include "TEbyE.h"
#include "TEbyEData.h"
#include <TCutG.h>
#include <iostream>
#include "stdio.h"
#include "string.h"
#include "TFile.h"
#include "TMath.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

ClassImp(TEbyE)

TEbyE::TEbyE(bool bspec)
{
  // Default constructor
  fEbyEData    = new TEbyEData();
  BoolSpec=bspec;
  debug = false;
  GMT_Trigger = -1.;
  if(debug)printf("\033[35m********************EbyE  Info :  debug level**********\033[m \n");
}



TEbyE::~TEbyE()
{
  delete fEbyEData;
}



bool TEbyE::Clear()
{
  fEbyEData->Clear();
  GMT_Trigger = -1.;
  return true;
}
bool TEbyE::InitCal()
{
	
	
	
	
  return true;
}

bool TEbyE::SpectraConstructor()
{
  if(BoolSpec){  
  	printf("\033[35mEbyE  Info :  Start Spectra constructors\033[m \n");
	fMyHistoEbyE2d= new TH2I("fMyHistoEbyE2d","fMyHistoEbyE2d",12000,0,12000,8192,0,16384);
        HListEbyE.Add(fMyHistoEbyE2d);
	fMyHistoEbySum= new TH1F("fMyHistoEbySum","fMyHistoEbySum",16384,0,16384);
	HListEbyE.Add(fMyHistoEbySum);
	fMyHistoEbylabel= new TH1F("fMyHistoEbylabel","fMyHistoEbylabel",9000,0,9000);
  	HListEbyE.Add(fMyHistoEbylabel);
	printf("\033[35m ----> Done \033[m \n");
	return true;
  }
  else {return false;}
 
}
bool TEbyE::ReadCal()
{
	
	
  return true;
}

double TEbyE::Cal(UShort_t en, float offset, float gain, float gain2){
  double enc; 
  enc = (double)en+gRandom->Uniform(1.0)-.5;
  enc = enc*enc*gain2+enc*gain+offset;
	   
  return enc;
}
double TEbyE::CalI(int en, float offset, float gain, float gain2){
  double enc; 
  enc = (double)en+gRandom->Uniform(1.0)-.5;
  enc = enc*enc*gain2+enc*gain+offset;
	   
  return enc;
}
float TEbyE::Doppler_Correction(float Theta_Gamma, float Phi_Gamma, float Theta_Part, float Phi_Part, float Beta_Part, float energie_Mes){  //rad, v/c
	float energievraie,cosinusPSI;
			  
		  cosinusPSI =TMath::Sin(Theta_Part)*TMath::Cos(Phi_Part)*TMath::Sin(Theta_Gamma)*TMath::Cos(Phi_Gamma)+
		  	      TMath::Sin(Theta_Part)*TMath::Sin(Phi_Part)*TMath::Sin(Theta_Gamma)*TMath::Sin(Phi_Gamma)+
			      TMath::Cos(Theta_Part)*TMath::Cos(Theta_Gamma);
			      
	energievraie = energie_Mes*(1.-Beta_Part*cosinusPSI)/sqrt(1.-Beta_Part*Beta_Part);

	return energievraie;
};
bool TEbyE::SetBeta(float beta){
	Beta=beta;
	return true;
}
bool TEbyE::Init(DataParameters *params)
{
  bool status = false;
  
		
  return status; 
   
}

bool TEbyE::InitNumexo2(Char_t *fileNumexo2){
        std::ifstream fichier(fileNumexo2);
        std::string ligne;
        std::vector<std::string> voiesMesure;
        std::vector<int> numerosVoieInfo;
	 // Lire le fichier et stocker les données
	while (std::getline(fichier, ligne)) {
            std::istringstream iss(ligne);
            std::string voieMesure;
            int numeroVoieInfo;

            if (iss >> voieMesure >> numeroVoieInfo) {
                voiesMesure.push_back(voieMesure);
                numerosVoieInfo.push_back(numeroVoieInfo);
            }
        }
	// Initialiser le tableau des indicateurs de match
        matchFlags.resize(voiesMesure.size(), 0);
	
	// Vérifier les correspondances et remplir le tableau
        for (size_t i = 0; i < voiesMesure.size(); ++i) {
            if (voiesMesure[i].find("MG") == 0 &&
                voiesMesure[i].find("STR") != std::string::npos &&
                voiesMesure[i].find("_E") != std::string::npos) {
                matchFlags[i] = 1; // Indique un match
		//cout<<i<<"  "<<voiesMesure[i]<<endl;
            }
        }
	cout<<"=>File "<<fileNumexo2<<" read"<<endl;
    

  return true;
}



bool TEbyE::IsMFMEbyE(UShort_t label,UShort_t data)
{ 
  
    //Push  Data
    if(debug)cerr<<" (*) Enter EbyE "<<endl;
    if(BoolSpec)fMyHistoEbyE2d->Fill(label,data);
    if(BoolSpec)fMyHistoEbySum->Fill(data);
    if(BoolSpec)fMyHistoEbylabel->Fill(label);
    //cerr<<"Label "<<label<<"  Data "<<data<<endl;
    fEbyEData->SetEbyELabel(label);
    fEbyEData->SetEbyEData(data);
	
    if(debug)cerr<<" (*) Push Data completed "<<endl;
      
  
   if(debug)cerr<<"(*) EbyE Exit "<<endl;
  return true;
}

bool TEbyE::Is(UShort_t lbl, Short_t val)
{

  bool status = false;
  return status;
}



bool TEbyE::Treat()
{
	MMMul=0;
	GMT_Trigger = -1;
	
	//cout<<"TEbyE::Treat() Infos : There are "<<fEbyEData->GetEbyEMult()<<" labels "<<endl;
	for(Int_t i =0 ; i<fEbyEData->GetEbyEMult(); i++){
 		if(fEbyEData->GetEbyEDet(i)==1){
			GMT_Trigger =fEbyEData->GetEbyEEnergy(i) ;
			//cout<<GMT_Trigger<<endl;
		}
		if(matchFlags[fEbyEData->GetEbyEDet(i)]== 1)MMMul++;
		//if(fEbyEData->GetEbyEEnergy(i)>6000&&fEbyEData->GetEbyEEnergy(i)<10000&&fEbyEData->GetEbyEDet(i)<8000&&(fEbyEData->GetEbyEDet(i)%2)==1)MMMul++;// MuST2 pair are energy
	}
	//cout<<MMMul++<<endl;
  return true;
}

bool TEbyE::Counter()
{
  return true;

}

bool TEbyE::ClearCounter()
{


  return true;

}

void TEbyE::InitBranch(TTree *tree)
{
  tree->Branch("EbyE", "TEbyEData", &fEbyEData,32000,99);
}


