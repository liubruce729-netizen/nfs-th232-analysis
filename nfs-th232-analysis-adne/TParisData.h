#ifndef __ParisDATA__
#define __ParisDATA__

#include <vector>

#include "TObject.h"
#include "TObjString.h"



class TParisData : public TObject {
 private:

   std::vector<UShort_t>	fParis_DetNbr;
   std::vector<Float_t>	fParis_QShort;
   std::vector<Float_t>	fParis_QLaBr3Cond;
   std::vector<Float_t>	fParis_QNaICond;
   std::vector<Float_t>	fParis_QCompton;
   std::vector<Float_t>	fParis_QLong;
   std::vector<Float_t>	fParis_Cfd;
   std::vector<Float_t>	fParis_Theta;
   std::vector<Float_t>	fParis_Phi;
   
//Time Stamp
 unsigned  long long   fTimeStampsParis[150] ;


 public:
   TParisData();
   virtual ~TParisData();

   void	Clear();
   void	Dump();



   /////////////////////           GETTERS           ////////////////////////
   UShort_t	GetParisMult()		  {return fParis_DetNbr.size();}
   UShort_t	GetParisDet(Int_t i)	{return fParis_DetNbr.at(i);}
   Float_t	GetParisQShort(Int_t i) {return fParis_QShort.at(i);}
   Float_t	GetParisQLaBr3Cond(Int_t i) {return fParis_QLaBr3Cond.at(i);}
   Float_t	GetParisQNaICond(Int_t i) {return fParis_QNaICond.at(i);}
   Float_t	GetParisQCompton(Int_t i) {return fParis_QCompton.at(i);}
   Float_t	GetParisQLong(Int_t i)	  {return fParis_QLong.at(i);}
   Float_t	GetParisCfd(Int_t i)	  {return fParis_Cfd.at(i);}
   Float_t	GetParisTheta(Int_t i)	  {return fParis_Theta.at(i);}
   Float_t	GetParisPhi(Int_t i)	  {return fParis_Phi.at(i);}
  
   unsigned long long    GetfTimeStamps(int i)   {return fTimeStampsParis[i];}

   /////////////////////           SETTERS           ////////////////////////
   void	SetParisDetectorNbr(UShort_t det)    {fParis_DetNbr.push_back(det);}
   void	SetParisQShort(Float_t E) 	  {fParis_QShort.push_back(E);}
   void	SetParisQLaBr3Cond(Float_t E) 	  {fParis_QLaBr3Cond.push_back(E);}
   void	SetParisQNaICond(Float_t E) 	  {fParis_QNaICond.push_back(E);}
   void	SetParisQCompton(Float_t E) 	  {fParis_QCompton.push_back(E);}
   void	SetParisQLong(Float_t E) 	  {fParis_QLong.push_back(E);}
   void	SetParisCfd(Float_t E) 	  	  {fParis_Cfd.push_back(E);}
   void	SetParisTheta(Float_t E) 	  {fParis_Theta.push_back(E);}
   void	SetParisPhi(Float_t E) 	  	  {fParis_Phi.push_back(E);}


   void SetfTimeStamps(unsigned long long T, int i){fTimeStampsParis[i] = T; }
   ClassDef(TParisData,1)  // ParisData structure
};

#endif
