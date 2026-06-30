// File : GVMeter.C
// Author: Luc Legeard

#include "GVMeter.h"
#include "math.h"
#include <sys/stat.h>
#include <iostream>
#include <math.h>
#include <Riostream.h>
#include <TVirtualX.h>
ClassImp(GVMeter)

using namespace  std;
//______________________________________________________________________________
GVMeter::GVMeter(const TGWindow *p, int w , int h):TGMainFrame(p, w, h)  {
	//Constructeur.

	prev_load = 0.0;
	old_memUsage = 0;
	testframe=NULL;

    if (p ==NULL) p= gClient->GetRoot();

    cout <<" gClient->GetRoot() "<<(long long*) (gClient->GetRoot()) <<"\n";
	fActInfo = 1;
	fSpeedo = new TGSpeedo(this, 0.0, 100.0, "CPU", "[%]");

	testframe = new TGTab(this);

	fSpeedo->Connect("OdoClicked()", "GVMeter", this, "ToggleInfos()");
	fSpeedo->Connect("LedClicked()", "GVMeter", this, "CloseWindow()");
	Connect("CloseWindow()", "GVMeter", this, "CloseWindow()");
	AddFrame(fSpeedo, new TGLayoutHints(kLHintsCenterX | kLHintsCenterX));
	fSpeedo->SetDisplayText("Used RAM", "[MB]");



	fTimer= new TTimer();
	fTimer->Connect("Timeout()", "GVMeter", this, "Update()()");

	fBgnd = fSpeedo->GetPicture();
	gVirtualX->ShapeCombineMask(GetId(), 0, 0, fBgnd->GetMask());
	SetBackgroundPixmap(fBgnd->GetPicture());
	SetWMSizeHints(fBgnd->GetWidth(), fBgnd->GetHeight(), fBgnd->GetWidth(),
			fBgnd->GetHeight(), 1, 1);

	MapSubwindows();
	MapWindow();
	// To avoid closing the window while TGSpeedo is drawing
	DontCallClose();
	// To avoid closing the window while TGSpeedo is drawing
	Resize(GetDefaultSize());
	// Set fixed size
	SetWMSizeHints(GetDefaultWidth(), GetDefaultHeight(), GetDefaultWidth(),
			GetDefaultHeight(), 1, 1);
	SetWindowName("ROOT CPU Load Meter");
	fTimer->Start(100, kFALSE);

	// set threshold values
	fSpeedo->SetThresholds(12.5, 50.0, 87.5);
	// set threshold colors
	fSpeedo->SetThresholdColors(TGSpeedo::kGreen, TGSpeedo::kOrange,
			TGSpeedo::kRed);
	// enable threshold
	fSpeedo->EnableThreshold();
	fSpeedo->SetScaleValue(0.0, 5);
	// enable peak marker
	fSpeedo->EnablePeakMark();

}
//______________________________________________________________________________

GVMeter::~GVMeter() {

	if (fTimer){
	delete fTimer;
	fTimer=NULL;
	}
	if (fSpeedo) {
	delete fSpeedo;
	fSpeedo=NULL;
	}
if (testframe) {
		delete testframe;
		testframe=NULL;
		}
}
//______________________________________________________________________________
void GVMeter::Update() {
	MemInfo_t memInfo;
	CpuInfo_t cpuInfo;
	Float_t act_load = 0.0;
	Int_t memUsage = 0;
	prev_load = act_load;
	old_memUsage = memUsage;

	// Get CPU informations
	gSystem->GetCpuInfo(&cpuInfo, 100);
	// actual CPU load
	act_load = cpuInfo.fTotal;
	// Get Memory informations
	gSystem->GetMemInfo(&memInfo);
	// choose which value to display
	if (GetActInfo() == 0)
		memUsage = memInfo.fMemTotal;
	else if (GetActInfo() == 1)
		memUsage = memInfo.fMemUsed;
	else if (GetActInfo() == 2)
		memUsage = memInfo.fMemFree;
	// small threshold to avoid "trembling" needle
	if (fabs(act_load - prev_load) > 0.9) {
		fSpeedo->SetScaleValue(act_load, 10);
		prev_load = act_load;
	}
	// update only if value has changed
	if (memUsage != old_memUsage) {
		fSpeedo->SetOdoValue(memUsage);
		old_memUsage = memUsage;
	}
}

//______________________________________________________________________________
void GVMeter::ToggleInfos() {
	// Toggle information displayed in Analog Meter

	if (fActInfo < 2)
		fActInfo++;
	else
		fActInfo = 0;
	if (fActInfo == 0)
		fSpeedo->SetDisplayText("Total RAM", "[MB]");
	else if (fActInfo == 1)
		fSpeedo->SetDisplayText("Used RAM", "[MB]");
	else if (fActInfo == 2)
		fSpeedo->SetDisplayText("Free RAM", "[MB]");
}
//______________________________________________________________________________
void GVMeter::CloseWindow() {
	// Close Window.

	if (fTimer)
		fTimer->TurnOff();
//	DestroyWindow();
	UnmapWindow();cout <<" debug 2 CloseWindow\n";
	//DeleteWindow();cout <<" debug 3 CloseWindow\n";
	//Cleanup();cout <<" debug 4 CloseWindow\n";

}
