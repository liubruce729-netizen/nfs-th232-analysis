#ifndef  __GSpectrumIdentity__
#define  __GSpectrumIdentity__

#include <TString.h>
#include <TObject.h>
#include <TNamed.h>
#include <TH1.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <Riostream.h>
#include <TXMLFile.h>
#include <TBuffer.h>
#include <TBufferXML.h>
#include <TXMLEngine.h>
#include <TDOMParser.h>
#include <TXMLNode.h>
#include "General.h"
#include "TMayaHisto.h"

#include "TVirtualFFT.h"
#include <map>
#include <string>
#include "GListDevice.h"
#include "GBase.h"

enum TypeSpe{USERTYPE =0 , RAWTYPE=1 , COMPUTEDTYPE=2 , CUTTYPE=3 , SERVERTYPE=4} ;

class GSpectrumIdentity: public GBase {

private:

	const char** list_info; //! text information about parameters.
	bool* list_server_info; //!  information if is only a server information

	TNamed* fSpectrum; // Pointer on TGraph or Histogram
	TString fName; // Name of GraphObject;
	TString fSourceName; // Name of source  ( file name or net node)
	TString fSourceType; // = gru or soap.
	TString fSource; // = net or file  or mem (localmemory)
	TString fFamily; // Names of family in which GraphObject is classified
	TString fServerIdent; // Server identifiant , can be use incase of a server data base existe: rater use fSource and fPort , we report to the server identified by fServerIdent in server data base
	Int_t fPort; // = no port in case of tcpip source
	Int_t fNumTab; // No of tabulation where GraphObject is displayed
	Int_t fNumPad; // No of pad where GraphObject is displayed
	Bool_t fStarted; //  started or stopped
	Bool_t fLocSameEvt;//flag to validate the fact that a the spectra must have the same event statistic
	Int_t fSpeInd; // index number unique index
	Int_t fDBInd; // index number in Database ( given by At() method)
	Int_t fXmin, fYmin, fXmax, fYmax; // memorization of zoom zone values
	TString fDrawStyle; // Memorization of DrawStyle
	Bool_t fAction; // flag to notice that a action will be done on this spectra (visualization....)
	TString fType; // type of spectra ( TH2S, INT16, TCutG...)
	Int_t fNbBits; // Nb of bit of GraphObjectf in case of Raw GraphObject (Raw Histogram) f(else = -1)
	Int_t fLineColor; // color of line
	Int_t fNbEltInId; // number of elements in a Identity ( to make verification)
	enum TypeSpe fTypeSpe; // Type GraphObjectf (0-> no type or User) , 1->Raw ,2=>Computed ,3=>TCut, 3=> no histo  but is info on one Server..)
public:

	GSpectrumIdentity(TNamed *sp, TString n, TString sName, TString sType,
			TString source, Int_t port, TString fam, Int_t nTab, Int_t nPad,
			Int_t BD_ind, bool start = true, TString type = "");
	GSpectrumIdentity(TString sName, TString sType, TString source, Int_t port);
	GSpectrumIdentity();
	GSpectrumIdentity(GSpectrumIdentity* id, bool with_instance_ofspectrum =
			true);
	virtual ~GSpectrumIdentity();
	virtual void IdentityInit();

	virtual void CopyFrom(GSpectrumIdentity* id, bool with_instance_ofspectrum =
			true);

	TH1* Get_Histo() {
		if (fSpectrum->InheritsFrom("TH1"))
			return (TH1*) fSpectrum;
		else
			return NULL;
	}
	TGraph* GetGraph() {
		if (fSpectrum->InheritsFrom("TGraph"))
			return (TGraph*) fSpectrum;
		else
			return NULL;
	}
	TGraph2D* GetGraph2D() {
		if (fSpectrum->InheritsFrom("TGraph2D"))
			return (TGraph2D*) fSpectrum;
		else
			return NULL;
	}
	TNamed* GetSpectrum() {
		return fSpectrum;
	}
	TString GetSpectrumName() {
		return fName;
	}
	TString GetSourceName() {
		return fSourceName;
	}
	TString GetSourceType() {
		return fSourceType;
	}
	TString GetSource() {
		return fSource;
	}
	TString GetServerIdent() {
		return fServerIdent;
	}
	Int_t GetPort() {
		return fPort;
	}
	TString GetFamily() {
		return fFamily;
	}
	Int_t GetNumTab() {
		return fNumTab;
	}
	Int_t GetNumPad() {
		return fNumPad;
	}
	Int_t GetSpeIndex() {
		return fSpeInd;
	}
	Int_t GetDBIndex() {
		return fDBInd;
	}
	Bool_t GetStarted() {
		return fStarted;
	}
	Bool_t GetLocSameEvt() {
		return fLocSameEvt;
	}
	Int_t GetXmin() {
		return fXmin;
	}
	Int_t GetYmin() {
		return fYmin;
	}
	Int_t GetXmax() {
		return fXmax;
	}
	Int_t GetYmax() {
		return fYmax;
	}
	TString GetDrawStyle() {
		return fDrawStyle;
	}
	Bool_t GetAction() {
		return fAction;
	}
	Int_t GetTypeSpe() {
		return fTypeSpe;
	}
	TString GetType() {
		return fType;
	}
	Int_t GetNbBits() {
		return fNbBits;
	}
	Float_t GetRatio();
	Float_t GetXaxisLast();
	Float_t GetXaxisFirst();
	Float_t GetXminAxis();
	Float_t GetXmaxAxis();
	Float_t GetMaximumBin();
	Color_t GetLineColor(bool with_save = true);
	TAxis*  GetXaxis ();
	TAxis*  GetYaxis ();
	TAxis*  GetZaxis ();
	/*
	 void    Set_MayaHisto (TMayaHisto* sp)   { fSpectrum =(TNamed*) sp;}
	 void    Set_Histo (TH1* sp  )			 { fSpectrum =(TNamed*) sp;}
	 void    SetGraph (TGraph* sp )           { fSpectrum =(TNamed*) sp;}
	 void    SetGraph2D (TGraph2D* sp )       { fSpectrum =(TNamed*) sp;}*/
	void SetSpectrum(TNamed* sp);
	void SetSpectrumName(TString name) {
		fName = name;
	}
	void SetSourceName(TString sourcename) {
		fSourceName = sourcename;
	}
	void SetSourceType(TString type) {
		fSourceType = type;
	}
	void SetSource(TString source) {
		fSource = source;
	}
	void SetServerIdent(TString si) {
		fServerIdent = si;
	}
	void SetPort(Int_t port) {
		fPort = port;
	}
	void SetFamily(TString family);
	void SetNumTab(Int_t n) {
		fNumTab = n;
	}
	void SetNumPad(Int_t n) {
		fNumPad = n;
	}
	void SetSpeIndex(Int_t n) {
		fSpeInd = n;
	}
	void SetDBIndex(Int_t n) {
		fDBInd = n;
	}
	void SetStarted(Bool_t start);
	void SetLocSameEvt(Bool_t loc){
		fLocSameEvt=loc;
	}
	void SetXmin(Int_t x) {
		fXmin = x;
	}
	void SetYmin(Int_t x) {
		fYmin = x;
	}
	void SetXmax(Int_t x) {
		fXmax = x;
	}
	void SetYmax(Int_t x) {
		fYmax = x;
	}
	void SetDrawStyle(TString style) {
		fDrawStyle = style;
	}
	void SetAction(Bool_t action) {
		fAction = action;
	}
	void SetTypeSpe(enum TypeSpe typespectrum) {
		fTypeSpe = typespectrum;
	}
	void SetType(TString type);
	void SetNbBits(Int_t NbBits) {
		fNbBits = NbBits;
	}
	void SetLineColor(Color_t color);
	void Reset();
	Int_t GetDimension();
	TString DumpHeader(Int_t nb = 0);
	TString DumpId(Int_t nb = 0);
	void ConstructRawHisto();
	void DeleteSpectrumInstance();
	void DeleteRawSpectrumInstance();
	void ChangeCutSpectrum(TNamed* spectrum);
	void ChangeSpectrum(TNamed* spectrum);
	void ChangeSpectrumOnServer();
	void SaveDims();
	Color_t SaveLineColor();
	void RestaureOption();
	void SaveOption();
	TNamed* GetSpectrumNetOrFileOrMem(GListDevice* listdev);
	TNamed* GetSpectrumNet(GListDevice* listdev);
	TNamed* GetSpectrumFile(GListDevice* listdev);
	TNamed* GetSpectrumMem(GListDevice* listdev);

	void FillHisto(UShort_t value);
	void SaveSpectrumTxt(ofstream *file);
	bool ReadSpectrumTxt(ifstream *file, Int_t textlines);
	void SaveSpectrum(TFile *file, Int_t type = -1);
	Bool_t ReadSpectrum(TFile *file, Int_t type = -1);
	Bool_t HasProperties(TString name, TString source, TString sourceName,
			TString sourceType, Int_t port, TString family);
	void CreatXML(TXMLEngine* xml, XMLNodePointer_t node);
	void ReadXML(TXMLEngine* xml, XMLNodePointer_t node);
	TString ToString();
	//char* RemoveChar(char * input, char* toremove, bool frombegin);
	void FitPanel();
	void DrawPanel();
	void GaussBg2(TF1 *fitfunc, Double_t gLowX, Double_t gUpX);
	void GaussBg2_h(TF1 *fitfunc, Double_t gLowX, Double_t gUpX);
	void FitBG();
	void Fit(TF1 *f1, Option_t *option = "", Option_t *goption = "",
			Axis_t xmin = 0, Axis_t xmax = 0);
	void Fit(const char *formula, Option_t *option = "",
			Option_t *goption = "", Axis_t xmin = 0, Axis_t xmax = 0);
	void PeakFind(Int_t nb_expected_peaks = 10, Float_t resolution = 1,
			Double_t sigma = 2, Double_t threshold = 0.05,
			bool display_polymarker = true);
	void MyDraw(bool same, TString option2D);
	void ReportXYMinMax();
	void ResetSpectrumOnServer();
	void ResetAllSpectraOnServer();
	void CancelZoom();
	void FFT();
	void FFThalf();
	void Scatter();
	void ZeroLess();
	void ProjectionX();
	void ProjectionY();
	void ProfileX();
	void ProfileY();
	void IntegralsInCuts(TCutG* cut);
	Double_t MeanHist(TCutG* cut, Int_t xory = 1);
	TH1F* ScatterSpectrum(TH1* sp);
	void ApplyZoom(Int_t xmin_pad, Int_t xmax_pad, Int_t ymin_pad,
			Int_t ymax_pad);
	void AutoZoom(Int_t *xmin, Int_t *xmax, Int_t *ymin, Int_t *ymax);
	void ApplyZoomProportional(Int_t xmin, Int_t xmax, Int_t ymin, Int_t ymax);
	void ApplySetRangeUser(bool inx,bool iny,bool inz, Double_t xmin, Double_t xmax,
			Double_t ymin,Double_t ymax,Double_t zmin, Double_t zmax);
	bool SpectrumTest(TNamed*obj);
	Int_t TestServer();
private:
 virtual void ToDoInCaseOfInterrupt(){};

ClassDef(GSpectrumIdentity,1)//Informations about a spectrum
};

#endif
