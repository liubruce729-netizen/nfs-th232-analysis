void displayCuts() {
  TFile* disp = new TFile("../histoDiamant.root");
  char name[1024];
  for(int i = 04; i <9; i++) {
    sprintf(name, "can_%d", i);
    TCanvas* cans = new TCanvas(name);
    cans->Divide(3, 3);
    for(int j = 0; j < 9; j++) {
      cans->cd(j+1);
      int number = i*9 + j;
      sprintf(name, "diamantCut%d", number);
     // TCanvas* can = new TCanvas(name);
      sprintf(name, "CsIE_PI%d", number);
      TH2* hist = (TH2*) disp->Get(name);
      if(hist == NULL) {
        cout << "Could not find " << name << endl;
      } else {
        hist->Draw("colz");
      }      
      sprintf(name, "diamantCut%d.root", number); 
      TFile* cutFile = new TFile(name);
      if(cutFile->IsZombie()) {
        cout << "Cutfile " << name << " does not exists or is a bad file. " << endl;
      } else {
        cout << "Opened " << name << endl;
        TCutG* proton = (TCutG*) cutFile->Get("proton");
        TCutG* twoproton = (TCutG*) cutFile->Get("twoproton");
        TCutG* alpha = (TCutG*) cutFile->Get("alpha");
        TCutG* charged = (TCutG*) cutFile->Get("charged");
        if(proton != NULL) 
          proton->Draw("same");
	  else 
	  cout << "No cut for protons in " << name << endl;
        if(twoproton != NULL) 
          twoproton->Draw("same");
	  else 
	  cout << "No cut for twoprotons in " << name << endl;
        if(alpha != NULL) 
          alpha->Draw("same");    
	  else 
	  cout << "No cut for alpha in " << name << endl;
        if(charged != NULL) 
          charged->Draw("same");    
	  else 
	  cout << "No cut for charged in " << name << endl;
      }
     cutFile->Close();
    }
  } 

}
