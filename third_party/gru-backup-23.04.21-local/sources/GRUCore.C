// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#if NET_LIB
#include "GNetServerSoap.h"
#endif
#include "TROOT.h"
#include "TRint.h"
#include <TRootGuiFactory.h>
#include "GRUCore.h"
#include <string>
#include "GInterrupt.h"
#include <signal.h> //  our new library
#include "ArgInterpretor.h"

volatile sig_atomic_t flag = 0;

char* version = (char*) GRU_VERSION;
int Bufsize = BUFSIZE;
int BufSize = Bufsize; // for use of  libgan_tape library
int fSoapport = 6603;
int fVerbose  = 1;
int fRootport = 9090;

#if NET_LIB
GNetServerSoap * netservsoap = NULL;
#endif
GNetServerRoot * netservroot = NULL;
//__________________________________________________________________________________________
void restart() {
	GError ferror;
	ferror.Infos(
			"THE APPLICATION HAS NOT BEEN FULLY INITIALIZED, RESTART //THE APPLICATION PLEASE");
	gROOT->ProcessLine(".q");
}

//__________________________________________________________________________________________
void InCaseOfControlC(int sig) { // can be called asynchronously
	flag = 1; // set flag
	printf("-> Hello I have detected a Ctrl C so I stop !\n");

	netservroot->GetCommand()->Kill();
	
	exit(0);
}
//_________________________________________________________________________________________
void  Help(){
			
   cout<<"Usage: GRUCore [options] "<<endl<<endl;
   cout<<"Options:    [-h|--help]      Display this help" << endl;
   cout<<"            [-ns|--nosoap]   Server soap is not launched" << endl;
   cout<<"            [-rp|--rootport] Root port, port of spectra server [default: "<< fRootport <<  "]"<< endl;
   cout<<"            [-sp|--soapport] Soap port, port of command server [default: "<< fSoapport <<  "]"<< endl;
   cout<<"            [-v |--verbose]  level , Set level of verbose  [default: "<< fVerbose <<  "]"<< endl;
}
//_________________________________________________________________________________________
bool HelpAndExit(){
       Help();
       exit (0);
}
//_________________________________________________________________________________________
void TestRootrc() {
	GError ferror;
	TString tempos;
	struct stat FileStat;
	TString home = gSystem->HomeDirectory();
	TString current = gSystem->WorkingDirectory();
	TPluginHandler *ph = gPluginMgr->FindHandler("GAcq", "GuserPlugin"); //on v�rifie que le plugin existe dans le .rootrc si le fichier .rootrc existe.


	tempos.Form("Plugin.GAcq:  GuserPlugin  GUser   ./GUser.C+  \"GUser()\"");

	if (stat("./.rootrc", &FileStat) == 0) { // if file .rootrc existe

		if (!ph) {
			ofstream file("./.rootrc", ios::app);
			if (file) {
				ferror.Infos("The plugins is append in .rootrc");
				file << tempos.Data() << endl;
				file.close();
				restart();
			}
		}
		return;
	}

	home.Append("/.rootrc");

	if (stat(home.Data(), &FileStat) == 0) {
		ferror.Infos("file .rootrc exist in home directory");
		if (!ph) {
			ofstream file(home.Data(), ios::app);
			if (file) {
				ferror.Infos("the plugins is append in .rootrc");
				file << tempos.Data() << endl;
				file.close();
				restart();
			}
		}
		return;
	}

	ofstream file("./.rootrc", ios::app);
	file << tempos.Data() << endl;
	file.close();

	ferror.Infos("The file .rootrc is created in your current directory");
	ferror.Infos("With these informations:", tempos);
	restart();
}

//_________________________________________________________________________________________
int main(int argc, char **argv) {

  //gROOT->Reset();
  bool nosoap = false;// = 2 n
  int pid = (int) getpid();
  signal(SIGINT, InCaseOfControlC);
    
        
// part of treatement of input arguments      
  ArgInterpretor ArgInt(argc,argv);

 
  while(true){  
     if (ArgInt.GetNextCommand () ==0){
 //           if (treatedinfo == 0) {HelpAndExit();}
            	break;
      }   
      ArgInt.Treat2Commands((char*)"-sp",(char*)"--soapport",(char*)"soap port",&fSoapport);
      ArgInt.Treat2Commands((char*)"-rp",(char*)"--rootport",(char*)"root port of spectra server",&fRootport);
      ArgInt.Treat2Commands((char*)"-v",(char*)"--verbose",(char*)"level of verbose",&fVerbose);
      ArgInt.Treat1Command((char*)"-ns",(char*)"--nosoap",&nosoap);
      if (ArgInt.Treat1Command((char*)"-",(char*)"--")) {
           cout << "unknown option: " <<  ArgInt.GetCommand1() << endl;
           ArgInt.ErrorMessageAndExit();
        }
  }
 
// part of execution of main
	cout << "  ******************************************* " << "\n";
	cout << "  *                                         * " << "\n";
	cout << "  *   HELLO  -- You are Running  "<<argv[0]<<"     * " << "\n";
	cout << "  *   Version :" << version << "                     * \n";
	cout << "  *    Compiled :"<< BUILD_DATE <<"   "<< BUILD_TIME << "        * " << "\n";
	cout << "  *                                         * " << "\n";
	cout << "  ******************************************* " << "\n";
	cout << "PID : " << pid << "  SoapPort : " << fSoapport << "  RootPort : "
			<< fRootport << endl;
	TestRootrc();
	netservroot = new GNetServerRoot(fRootport);
	netservroot->GetCommand()->InitStateMachine();
	netservroot->GetCommand()->GetStateMachine()->SetVerbose(fVerbose);
	netservroot->GetCommand()->SetVerboseGAcq(fVerbose);
	netservroot->StartServer(true);
	TApplication theApp("tapp", &argc, argv);  // use full if we want display a canvas for deb
#if NET_LIB
	if (nosoap == false) {
		netservsoap = new GNetServerSoap(fSoapport);
		if (netservroot){
			netservsoap->SetCommand(netservroot->GetCommand());
			netservsoap->SetRootServer(netservroot);
			netservsoap->SetVerbose (fVerbose);
			netservsoap->GetCommand()->SetVerbose (fVerbose);

			netservsoap->StartServer();
		}
	}
#endif
    while (netservroot->GetRunningFlag()){sleep(1);}//

	cout << "  ****************** End of GRUCore ************ " << "\n";
#if NET_LIB
	if (netservsoap)
		delete (netservsoap);
#endif
	if (netservroot)
	delete (netservroot);
	return 0;
}


