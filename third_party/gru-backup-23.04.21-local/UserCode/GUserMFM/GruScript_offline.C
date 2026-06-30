// For methode of SetTTreeMode( ...)
// { TREE_NO = 0 , TREE_STANDARD = 1, TREE_ONE_VECTOR = 2, TREE_USER = 3}


{  
// specify good include dir!

  gROOT->Reset(); 
  char command[100];
if (strncmp(gROOT->GetVersion(), "5.3",3)==0){
	printf ("version de root 5.3XXXX\n");
	 TString test = gSystem->Getenv("GRUDIR");
	if (test.CompareTo("")==0) sprintf(command ,".include /home/acqexp/GRU/GRUcurrent/include");
	  else sprintf(command ,".include %s/include",test.Data());

	gROOT->ProcessLine(".L ./GUser_C.so");//load and compile TUiser class
 }else{
	R__LOAD_LIBRARY(libHist)
	R__LOAD_LIBRARY(GUser_C)
}


  GMFMFile *file = new GMFMFile("./run1.dat");   // Get first Device ( in this case  run  file)
  file->Open();                          // Open Device
  GUser * a= new GUser(file);            // creat user treatement environement
  a->EventInit("etest","mfm",false);                        // event initialisation
  a->SetSpectraMode(1);                  // Declare all raw parameters as histograms
  a->SetScalerMode(1,"./scale.root");    // Do not make a run with scaler  events
  a->SetTTreeMode ((TTreeMode)1,"./runstd.root");   // Do a standart TTree ( one leaf  for each parameter) of events
  a->InitUser();     // Do Init User()

  file->Rewind();        // rewind run
  a->DoRun();        // do treaments of all events
  //a->DoRun(2000);  // do treaments on first 2000 events
  file->Close();
  //..if you want to add a second run in same analyse uncomment next line.........................
  //file->SetDevice("run2.dat"); // change of run
  //file->Open(); 
  //a->DoRun();
  //file->Close();
  //........... 

  a->EndUser();              // must be explicitly called , if it needs
  a->SpeSave("histo.root"); // save all declared histogram 
  delete (a);   // finish 
gROOT->ProcessLine(".q");
}


