
{  
  gROOT->Reset(); 

char command[100];
  if (strncmp(gROOT->GetVersion(), "5.3",3)==0){
	printf ("version de root 5.3XXXX\n");

	 TString test = gSystem->Getenv("GRUDIR");
	if (test.CompareTo("")==0) sprintf(command ,".include /home/acqexp/GRU/GRUcurrent/include");
	  else sprintf(command ,".include %s/include",test.Data());
	gROOT->ProcessLine(command);
	gROOT->ProcessLine(".L ./GUser_C.so");//load and compile TUiser class
 }else{
	R__LOAD_LIBRARY(libHist);
	R__LOAD_LIBRARY(GUser_C);
}


 GNetClientNarval  *net = new GNetClientNarval("NarvalActorWatcherHost"); //
 net->SetPort (NarvalActorWatcherPort);
 net->SetBufferSize(BufferSizeToSet);

  GUser * a= new GUser(net);          // creat user treatement environement 
  GNetServerRoot * serv = new GNetServerRoot(9090,a);
  a->EventInit("ExperimentName","mfm");                      // event initialisation
  a->SetSpectraMode(1);                // Declare all raw parameters as histograms 
  a->InitUser();
 serv->StartServer();
  a->DoRun();                          // a->DoRun(2000);do treaments on 2000 first events ( 0 = all);
  
  net->Close();                       
  a->EndUser();
              // must be explicitly called , if it needs
  a->SpeSave("histo.root"); // save all declared histogram 
  delete (a);   // finish 
gROOT->ProcessLine(".q");
}


