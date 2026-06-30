#ifndef  __GSpectraDB__
#define  __GSpectraDB__

#include <TObjArray.h>
#include <TXMLEngine.h>
#include <TH1.h>
#include <TGWindow.h>
#include <TString.h>
#include "GSpectrumIdentity.h"
#include "GListDevice.h"
#include "GError.h"

class GSpectraDB : public TObjArray {

private:

	Int_t fSpeIndex; // unique index which is incremeted each time we have a AddIdent
	GError fError;
	int fVerbose;
public:

	GSpectraDB(Int_t s = 1, Int_t lowerBound = 0);
	virtual ~GSpectraDB();

	void       MakeDUMP(const char* fileName, int nb =0);
	void       MakeDUMP(Int_t nb );
	void       MakeDUMP();
	TNamed*    GetSpectrumBySpeIndex(Int_t spe_index);
	TNamed*    GetSpectrumByIndex(Int_t index);
	TString    GetNameByIndex(Int_t index);
	TString    GetNameBySpeIndex(Int_t spe_index);
	Int_t      GetIndexByName(const char* namehisto);
	Int_t      GetIndexByName(const char* namehisto,const char* family);
	Int_t      GetSpeIndexByName(const char* namehisto);
	TNamed*    GetSpectrumByName(const char* namehisto);
	TNamed*    GetSpectrumByName(const char* namehisto,const char* family) ;
	TObjArray* GetSpectraByID(TObjArray* ids);
	TObjArray* GetSpectraByNumTab(Int_t numTab);
	Int_t      GetIndex(TNamed* sp);
	Int_t      GetSpeIndex(TNamed* sp);
	void       SetVerbose(int verb){fVerbose = verb;}
	GSpectrumIdentity* GetIdentity(const char* namehisto,const char* family);
	GSpectrumIdentity* GetIdentity(const char* namehisto);
	GSpectrumIdentity* GetIdentity(Int_t tab, Int_t pad);
	GSpectrumIdentity* GetIdentity(Int_t index);
	GSpectrumIdentity* GetIdentity(TNamed* sp);
	TCutG*     GetCut();
	Bool_t     IsFromNet(Int_t tab, Int_t pad);
	//Int_t      IsHistoExiste2(char* name,   char* sourcename, char* sourcetype,
	//		                 char* source, int port);
	Int_t      IsHistoExiste(const char* name,  const  char* family,Int_t typespe=-1);
	Int_t      IsServerExiste(const char* SourceName, Int_t port);
	Int_t      IsHistoExiste(const char* name, const char* family, const char* SourceType) ;
	Color_t    IsLineColorAlreadyExiste(Int_t indice);
	Color_t    IsLineColorAlreadyExiste(GSpectrumIdentity* id,Int_t avoid_this_indice=-1) ;
//	void       DeleteIdentity(Int_t index, Bool_t delete_histo = true);
	void       RazPointSpectra();
	void       DeleteIdentity(GSpectrumIdentity* identity, Bool_t delete_histo = true);
	void       DeleteIdentity(Int_t tab, Int_t pad,Bool_t delete_histo= true);
	void       DeleteIdentity(TNamed* spectrum, Bool_t delete_histo =true);
	void       DeleteAllIdentities(bool quiet = true, bool raw = false);
	void       ChangeSpectrum(int index,TNamed* spectrum);
	void       ChangeSpectrumOnServer(Int_t index);
	void       CleanIdentity(Int_t tab, Int_t pad,Bool_t delete_histo= true);
	void       DeleteSpectra();
	void       RemoveAllCuts();
	void       IntegralsInCuts(TObjArray* listcuts);
	void       ReIndexation();
	Int_t      AddIdentity(TNamed *h,         TString n,  TString sName,   TString sType,
			               TString source, Int_t port, TString fam , Int_t nTab = -1,
			               Int_t nPad = -1,bool start = true);

	void       MyAddLast(GSpectrumIdentity* id);
	Int_t      AddLastInDB(GSpectrumIdentity* id,bool quiet = true);
	Int_t      AddIdentity(GSpectrumIdentity * id);
	Int_t      AddServerIdentity( TString sName, TString sType,TString source, Int_t port=0);
	void       SaveXML(TString fileName);
	Int_t      ReadXML(TXMLEngine *ml, XMLNodePointer_t node,bool server);
	void       SaveSpectra(TFile* file,Int_t type = -1);
	void       SaveSpectraTxt(ofstream *file);
    Bool_t     ReadSpectra(TFile *file,Int_t type =-1);
	Int_t      CreateXML(TXMLEngine *ml, XMLNodePointer_t node,bool Server);
	Int_t      ReadXMListedByTab(TXMLEngine *xml, XMLNodePointer_t node,Int_t tab,Int_t nb ) ;
	Int_t      CreateXMListedByTab(TXMLEngine *xml, XMLNodePointer_t node,Int_t tab,Int_t nb ) ;
	void       UpdateFromXML(TXMLEngine* xml, XMLNodePointer_t node);
	void       UpdateFromDB(GSpectraDB *DB);
	void       ReplaceDB(GSpectraDB *DB);
	void       ReplaceDBOnlySpectra(GSpectraDB *DB);
	void       InstanceRawHistoFromAction() ;
	GSpectraDB* CloneSpectraNull();
	void       SetPortDB(Int_t port);
	void       SetSourceDB(TString source) ;
	void       SetSourceNameDB(TString source) ;
	void       SetStartedDB(Bool_t start);
	void       ConstructAllRawInDB();
	void 	   RazDB();
	void       ResetActions();
	Bool_t     ResetSpectra(const TGWindow* main,Bool_t question );
	void       Refresh(bool getanewone,int tab, int nbpad,TString option2D,GListDevice* listdev);
	Bool_t     ResetAllSpectraOnServer(const TGWindow* main,Bool_t question );
	void       TestObject();
	void       CancelZoom();
	void       ApplyZoom(Int_t xmin_pad, Int_t xmax_pad, Int_t ymin_pad,Int_t ymax_pad);
	void       AutoZoom();
	void       ApplyZoomProportional(Int_t xmin, Int_t xmax, Int_t ymin, Int_t ymax);
	void       ApplySetRangeUser(bool inx,bool iny,bool inz, Double_t xmin, Double_t xmax,Double_t ymin,Double_t ymax,Double_t zmin, Double_t zmax);
	void       FFT();
	void       FFThalf();
	void       Scatter();
	void       ZeroLess();
	void       ProjectionX();
	void       ProjectionY();
	void       ProfileX();
	void       ProfileY();
	void       PeakFind(Int_t fNbPeaks,Float_t fResolution ,Double_t fSigma,Double_t fThreshold,bool fDisplay_polymarker);
	void ByName(int premier , int dernier);
        void Reverse();
	void ByPage(int premier , int dernier);
	TString GetSpectraList();
	void SpectraList();

ClassDef(GSpectraDB,1)
	//Spectra Data Base
};
#endif
