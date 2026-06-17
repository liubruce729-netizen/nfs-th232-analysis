void writeCuts( int cutNumber) {
  char name[1024];
  sprintf(name, "CutDiamant/diamantCut%d.root", cutNumber);
  TFile* file = new TFile(name, "RECREATE");
  file->WriteObject(proton, "proton", "");
  file->WriteObject(twoproton, "twoproton", "");
  file->WriteObject(alpha, "alpha", "");
  file->Close();
  cout << "Exported Cuts into file " << name << endl;
  delete proton;
  delete twoproton;
  delete alpha;
}
