
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




  GTape *file = new GTape("run1.dat");   // Get first Device ( in this case  run  file)
  file->Open();                          // Open Device
  GUser * a= new GUser(file);            // creat user treatement environement
  a->EventInit();                        // event initialisation
  a->SetSpectraMode(1);                  // Declare all raw parameters as histograms
  a->SetScalerMode(1,"./scale.root");    // Do not make a run with scaler  events
  a->SetTTreeMode (1,"./runstd.root");   // Do a standart TTree ( one leaf  for each parameter) of events
  a->InitUser();     // Do Init User()

  file->Rewind();        // rewind run
  a->DoRun(2000);        // do treaments on 2000 first events ( 0 = all);
  file->Close();
  //..if you want to add a second run in same analyse uncomment next line.........................
  //file->SetDevice("run2.dat"); // change of run
  //file->Open(); 
  //a->DoRun(2000);   // do treaments on 2000 first events
  //file->Close();
  //........... 

  a->EndUser();              // must be explicitly called , if it needs
  a->SpeSave("histo.root"); // save all declared histogram 
  delete (a);   // finish 
gROOT->ProcessLine(".q");
}


