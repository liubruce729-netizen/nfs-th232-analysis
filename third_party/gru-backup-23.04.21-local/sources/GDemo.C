// File : GDemo.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GDemo
//
// This class do demonstration of alive fSpectra
//
////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the Ldifficense, or *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************


#include "GDemo.h"

//_____________________________________________________________________________

ClassImp( GDemo);

//______________________________________________________________________________

GDemo::GDemo() {

	fServ = NULL;
	fSpe = NULL;
	fThreadAlive = NULL;
	fServername = "";
	Init();
}

//_____________________________________________________________________________

GDemo::~GDemo() {
	if (fThreadAlive)
		fThreadAlive->Kill();
	if (fServ) {
		fServ->StopServer();
		delete (fServ);
		fServ = NULL;
	}
	if (fSpe) {
		delete (fSpe);
		fSpe = NULL;
	}
	if (fTMaya1) {
		delete (fTMaya1);
		fTMaya1 = NULL;
	}
	if (fTMaya2) {
		delete (fTMaya2);
		fTMaya2 = NULL;
	}
	if (Mycut1) {
		delete (Mycut1);
		Mycut1 = NULL;
	}
	if (Mycut2) {
		delete (Mycut2);
		Mycut2 = NULL;
	}
	if (Mycut3) {
		delete (Mycut3);
		Mycut3 = NULL;
	}

	for (int i = 0; i < NBOFHISTO; i++) {
		if (fHpx[i]) {
			delete (fHpx[i]);
			fHpx[i] = NULL;
		}
		if (fHpxpy[i]) {
			delete (fHpxpy[i]);
			fHpxpy[i] = NULL;
		}
		if (fHprof[i]) {
			delete (fHprof[i]);
			fHprof[i] = NULL;
		}
		if (fGraph[i]) {
			delete (fHprof[i]);
			fHprof[i] = NULL;
		}
		if (fHisto5[i]) {
			delete (fHisto5[i]);
			fHisto5[i] = NULL;
		}
		if (fHisto6[i]) {
			delete (fHisto6[i]);
			fHisto6[i] = NULL;
		}
		if (fHisto7[i]) {
			delete (fHisto7[i]);
			fHisto7[i] = NULL;
		}
	}
}
//_____________________________________________________________________________
void SetHostName(char* name) {

}
//_____________________________________________________________________________
void GDemo::Init() {
	fSizeofhisto = SIZEOFHISTO;
	fNbofhisto = NBOFHISTO;
	fNb_peak = 10;
	fPort = 0;
	fServername = (char*) (gSystem->HostName());
	fTMaya1 = NULL;
	fTMaya2 = NULL;
	Mycut1 = NULL;
	Mycut2 = NULL;
	Mycut3 = NULL;

	for (int i = 0; i < fNbofhisto; i++) {

		fHpx[i] = NULL;
		fHpxpy[i] = NULL;
		fHprof[i] = NULL;
		fHprof[i] = NULL;
		fGraph[i] = NULL;
		fHisto5[i] = NULL;
		fHisto6[i] = NULL;
		fHisto7[i] = NULL;
	}
}
//_____________________________________________________________________________
void GDemo::InitSpectra() {


	Double_t x, y;
	Double_t r, r_max = 10;

	//const Int_t kUPDATE = 1000;

	Int_t fHpx_size = 100;
	Int_t fHpx_min = -4;
	Int_t fHpx_max = 4;
	Int_t fHpxpy_size = 50;
	Int_t fHpxpy_min = -4;
	Int_t fHpxpy_max = +4;

	TString name1;
	TString name2;
	TString name3;
	TString name4;
	TString name5;
	TString name6;
	TString name7;
	TString name_fGraph;

	for (Int_t i = 0; i < fNbofhisto; i++) {

		name1.Form("fHpx_%d", i);
		name2.Form("fHpxpy_%d", i);
		name3.Form("fHprof_%d", i);

		name5.Form("Histo1D_%d", i);

		name6.Form("Histo2D_%d", i);
		name7.Form("Histo1Dpic_%d", i);
		name_fGraph.Form("fGraph1Dp_%d", i);
		name1.ReplaceAll('-', '_');
		name2.ReplaceAll('-', '_');
		name3.ReplaceAll('-', '_');
		name5.ReplaceAll('-', '_');
		name6.ReplaceAll('-', '_');
		name7.ReplaceAll('-', '_');

		fHpx[i] = new TH1F(name1, name1, fHpx_size, fHpx_min, fHpx_max);
		fHpx[i]->SetFillColor(20 + 5 * i);
		fHpxpy[i] = new TH2F(name2, name2, fHpxpy_size, fHpxpy_min, fHpxpy_max,
				fHpxpy_size, fHpxpy_min, fHpxpy_max);
		fHprof[i] = new TProfile(name3, name3, fHpx_size, fHpx_min, fHpx_max,
				0, fHpx_size / 2);
		;
		fHisto5[i] = new TH1S(name5, name5, fSizeofhisto, 0, fSizeofhisto);
		fHisto6[i] = new TH2S(name6, name6, fSizeofhisto, 0, fSizeofhisto,
				fSizeofhisto, 0, fSizeofhisto);
		fHisto7[i] = new TH1F(name7, name7, fSizeofhisto, 0, fSizeofhisto);

		fGraph[i] = new TGraph(fSizeofhisto);
		for (int j = 0; j < fSizeofhisto; j++) {
			r = r_max;
			x = r * sin(((double) j) * 2 * M_PI / fSizeofhisto);
			y = r * cos(((double) j) * 2 * M_PI / fSizeofhisto);
			fGraph[i]->SetPoint(j, x, y);
		}
		fGraph[i]->SetName(name_fGraph);
		fGraph[i]->SetDrawOption("AL*");

		for (Int_t j = 0; j < fSizeofhisto; j++) {
			fHisto5[i] ->Fill(j, (j) * ((float) i / (float) fNbofhisto));

			for (Int_t k = 0; k < fSizeofhisto; k++) {
				fHisto6[i] ->Fill(j, k, j + k);
			}
		}
		for (Int_t j = 0; j < fNb_peak; j++) {
			fHisto7[i] ->Fill((fSizeofhisto * (Float_t)(j + 0.5) / fNb_peak),
					100);
		}
	}

	fTMaya2 = new TMayaHisto("MayaHisto2", "MayaHisto2", 32, 32, -5);
	fTMaya1 = new TMayaHisto("MayaHisto1", "MayaHisto1", 32, 32, -5);

	Mycut1 = new TCutG("Mycut1", 1);
	Mycut2 = new TCutG("Mycut2", 1);
	Mycut3 = new TCutG("Mycut3", 1);

	for (int i = 0; i < 32; i++) {
		fTMaya1->Fill(i, i, 100);
		fTMaya2->Fill(i, i, 100);
	};

}

//______________________________________________________________________________

void GDemo::InGSpectra() {
// put all spectra in a data base;
	fSpe = new GSpectra();
	TString family1 = "";
	TString family2;
	TString family3;
	TString family4;
	TString family5;
	TString family6;
	TString family7;
	TString family_fGraph;
	//fServername = "MyComputer";// in case   fServername comme from my Macbook (name is too long)
	cout << "Servername =" << fServername.Data() << " Port =  " << fPort
			<< endl;
	family1.Form("Px_Distribution_%s_%d", (char*) fServername.Data(), fPort);
	family2.Form("Py_vs_px_%s_%d", (char*) fServername.Data(), fPort);
	family3.Form("Profile_%s_%d", (char*) fServername.Data(), fPort);

	family5.Form("Family/1D_%s_%d", (char*) fServername.Data(), fPort);
	family6.Form("Family/2D_%s_%d", (char*) fServername.Data(), fPort);
	family7.Form("Family/1D_pic_%s_%d", (char*) fServername.Data(), fPort);
	family_fGraph.Form("fGraph/1D_%s_%d", (char*) fServername.Data(), fPort);

	family1.ReplaceAll('-', '_');
	family2.ReplaceAll('-', '_');
	family3.ReplaceAll('-', '_');
	family5.ReplaceAll('-', '_');
	family6.ReplaceAll('-', '_');
	family7.ReplaceAll('-', '_');

	for (Int_t i = 0; i < fNbofhisto; i++) {

		fSpe->AddSpectrum(fHpx[i], (char*) family1.Data(), fPort);
		fSpe->AddSpectrum(fHpxpy[i], (char*) family2.Data(), fPort);
		fSpe->AddSpectrum(fHprof[i], (char*) family3.Data(), fPort);

		fSpe->AddSpectrum(fHisto5[i], (char*) family5.Data(), fPort);
		fSpe->AddSpectrum(fHisto6[i], (char*) family6.Data(), fPort);
		fSpe->AddSpectrum(fHisto7[i], (char*) family7.Data(), fPort);
		fSpe->AddSpectrum(fGraph[i], (char*) family_fGraph.Data(), fPort);
	}

	fSpe->AddSpectrum(fTMaya1, (char*) "Maya");
	fSpe->AddSpectrum(fTMaya2, (char*) "Maya");

	fSpe->GetCutFromFile(Mycut1, (char*) "Cuts", (char*) "GruCuts.root");
	fSpe->GetCutFromFile(Mycut2, (char*) "Cuts", (char*) "GruCuts.root");
	fSpe->GetCutFromFile(Mycut3, (char*) "Cuts", (char*) "GruCuts.root");

	//AddCut allready done in GetCutFromFile
	fSpe->AddCut(Mycut1, (char*) "Cuts");
	fSpe->AddCut(Mycut2, (char*) "Cuts");
	fSpe->AddCut(Mycut3, (char*) "Cuts");

	fSpe->GetDB()->MakeDUMP(0, false);

}
//______________________________________________________________________________

void GDemo::GSpectraInMem() {

	fSpe->ReferenceInMemory();
}
//______________________________________________________________________________
GSpectra * GDemo::GetGSpectra() {

	return fSpe;
}
//______________________________________________________________________________

void GDemo::InitRootServer(int port) {

	fPort = port;
	fServ = new GNetServerRoot(port, fSpe);
	fPort = fServ->StartServer(true, true);
}

//______________________________________________________________________________
void GDemo::SpectreAliveT() {

	// Launch  a Thread
	if (fThreadAlive)
		return;

	fThreadAlive = new TThread("DemoThread", (void(*)(void*)) &SpectreAlive,
			(void*) this);

	// ... et on lance l'execution du thread
	fThreadAlive ->Run();

	fError.TreatError(0,0,"Thread for aliving spectra is running");
	return;
}
//______________________________________________________________________________
void GDemo::SpectreAlive() {
	SpectreAlive((void*) this);
}
//______________________________________________________________________________
void GDemo::SpectreAlive(void * arg) {
	GDemo* iGDemo = (GDemo*) arg;

	int i = 0;
	Int_t j;

	Double_t r, r_max = 10, teta;
	float angle1, angle2;
	Int_t test1, test2, ntour = 10;
	Float_t x, y, px, py, pz;
	px = 1;
	py = 1;
	TRandom dz;

	while (true) {
		i++;

		iGDemo->fTMaya1->Reset();
		iGDemo->fTMaya2->Reset();
		angle1 = (((((float) rand())) / RAND_MAX * 64) - 32) / 32;
		angle2 = (((((float) rand())) / RAND_MAX * 64) - 32) / 32;
		for (int ik = 0; ik < 32; ik++) {
			int testi = ik;
			int testj1 = 16 + angle1 * ik;
			int testj2 = 16 + angle2 * ik;
			if ((0 < testi) && (testi < 32) && (0 < testj1) && (testj1 < 32))
				iGDemo->fTMaya1->Fill(testi, testj1, 100);
			if ((0 < testi) && (testi < 32) && (0 < testj2) && (testj2 < 32))
				iGDemo->fTMaya2->Fill(testi, testj2, 100);
		}

		for (int l = 0; l < iGDemo->fNbofhisto; l++) {
			r = 0;
			double barreau, entrebarreau, r1;
			for (int j = 0; j < iGDemo->fSizeofhisto; j++) {
				int currenttour = ((double) j * ntour) / iGDemo->fSizeofhisto;
				teta = ((double) j * 2 * M_PI * ntour / (iGDemo->fSizeofhisto)
						+ (double) (i % iGDemo->fSizeofhisto) * (2 * M_PI)
								/ iGDemo->fSizeofhisto);
				if (l >= iGDemo->fNbofhisto / 2)
					teta = -teta;
				barreau = (double) 1 / ntour;
				entrebarreau = barreau * ((double) (j % ((iGDemo->fSizeofhisto)
						/ ntour))) / ((iGDemo->fSizeofhisto) / ntour);
				r1 = currenttour * barreau + entrebarreau;
				if (r1 > r)
					r = r1;
				x = r_max * r * sin(teta);
				y = r_max * r * cos(teta);
				if (x > r_max)
					x = r_max;
				if (y > r_max)
					y = r_max;
				iGDemo->fGraph[l]->SetPoint(j, x, y);
			}
		}

		for (j = 0; j < iGDemo->fNbofhisto; j++) {
			dz.Rannor(px, py);
			pz = px * px + py * py;

			iGDemo->fHpx[j]->Fill(px);
			switch (j) {
			case 1:
				if ((iGDemo->Mycut1->IsInside(px, py)))
					iGDemo->fHpxpy[j]->Fill(px, py);
				break;
			case 2:
				if ((iGDemo->Mycut2->IsInside(px, py)))
					iGDemo->fHpxpy[j]->Fill(px, py);
				break;
			case 3:
				if ((iGDemo->Mycut3->IsInside(px, py)))
					iGDemo->fHpxpy[j]->Fill(px, py);
				break;
			case 4:
				if (((iGDemo->Mycut1->IsInside(px, py))
						|| (iGDemo->Mycut2->IsInside(px, py))
						|| (iGDemo->Mycut3->IsInside(px, py))))
					iGDemo->fHpxpy[j]-> Fill(px, py);
				break;
			default:
				iGDemo->fHpxpy[j]->Fill(px, py);
				break;
			}
			iGDemo->fHprof[j]->Fill(px, pz);

			test1 = (int) (((((float) rand()) / RAND_MAX)
					* iGDemo->fSizeofhisto));
			iGDemo->fHisto5[j]->Fill(test1);
			test2 = (int) (((((float) rand()) / RAND_MAX)
					* iGDemo->fSizeofhisto));
			iGDemo->fHisto6[j]->Fill(test1, test2);

			px = test1 / iGDemo->fSizeofhisto;
			py = test2 / iGDemo->fSizeofhisto;

		}

		if (i == 100000)
			i = 0;
		usleep(1000);
	}

	return;

}

//______________________________________________________________________________________________________
void GDemo::ToDoInCaseOfInterrupt() {
	fThreadAlive->Kill();
	if (fServ) {
		fServ->StopServer();
	}
}
/**/
//______________________________________________________________________________________________________
