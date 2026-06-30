
{  

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


  TString ExpName  = gSystem->GetEnv();
  TString Hostname = gSystem->GetEnv();
  GTape *net = new GNetClientNarval(HostName.Data()); 
  net->SetPort (10201);  
  
  GUser * a= new GUser(net);          // creat user treatement environement 
  a->EventInit(ExpName.Data());                      // event initialisation 
  a->SetSpectraMode(1);                // Declare all raw parameters as histograms 
  a->InitUser();
  a->DoRun();                          // a->DoRun(2000);do treaments on 2000 first events ( 0 = all);
  net->Close();                       
  a->EndUser();              // must be explicitly called , if it needs
  a->SpeSave("histo.root"); // save all declared histogram 
  delete (a);   // finish 
gROOT->ProcessLine(".q");
}


