
{  
  // specify good include dir!
  gROOT->Reset(); 
  char command[100];
  if (strncmp(gROOT->GetVersion(), "5.3",3)==0){
	  printf ("version de root 5.3XXXX\n");
	  char command[100];
	  TString test = gSystem->Getenv("GRUDIR");
	  if (test.CompareTo("")==0) sprintf(command ,".include /home/acqexp/GRU/GRUcurrent/include");
	  else sprintf(command ,".include %s/include",test.Data());

	  gROOT->ProcessLine(command);
	  gROOT->ProcessLine(".L ./GUser_C.so"); //load and compile GUser class
  }else{
	  R__LOAD_LIBRARY(libHist)
	  R__LOAD_LIBRARY(GUser_C)

  }


  TString dirname      = "/data/my_directory/run/";
   TString filename     = "run_0000.dat.01-01-11_11h11m11s"; // Get first Device ( in this case  run  file)
   TString spectraname  = "histo_00_t.root";
   TString fullfilename ;
   fullfilename.Form("%s%s",dirname.Data(),filename.Data());
   
   GTape *file = new GTape(fullfilename.Data());   // Get first Device ( in this case  run  file)


  GUser * a= new GUser(file);            // creat user treatement environement
  a->EventInit("local" );                      // event initialisation
  a->SetSpectraMode(1);                  // Declare all raw parameters as histograms
  a->SetScalerMode(1,"./scale.root");    // Do not make a run with scaler  events
  a->SetTTreeMode (1,"./runstd.root");   // Do a standart TTree ( one leaf  for each parameter) of events
  a->InitUser();     // Do Init User()

 while (file->IsExiste()){
  file->Open(); 
  file->Rewind();        // rewind run
  a->DoRun();        
  file->Close();
  subrun++;
  fullfilename.Form("%s%s.%d",dirname.Data(),filename.Data(),subrun);
  file->SetDevice(fullfilename.Data());

}

  a->EndUser();              // must be explicitly called , if it needs
  a->SpeSave(spectraname.Data()); // save all declared histogram
  delete (a);   // finish 
gROOT->ProcessLine(".q");
}


