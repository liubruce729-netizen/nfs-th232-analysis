/*


	Version 1.1 § 25Feb2010
	E.Clement
	Version 1.2 § 09Mar2010
	Ch.Theisen
  	Version 1.3 § 19Mar2010
  	E.Clement 
  	Version 1.4 § Janv2012 for fc15 and GRU 11.11 and beyond
  	E.Clement
	Version 1.5 § July2021
  	E.Clement
	
  
   Point on NarvalWatcher

*/
#include "SpyAnalysisADNE.h"
#include "./GUser.h"
#include "GMFMFile.h"
#include "GNetClientNarval.h"
#include <unistd.h>
#include <signal.h> 
#include <yaml-cpp/yaml.h>
#include <stdlib.h>

using namespace std;
GNetClientNarval *file;
GUser * a;

void StopSpy(int sig){
 printf("\033[35m   ->Info::Stopped by Ctrl-c\033[m \n");
 printf("\033[35mInfo::Closing Connection called\033[m \n");
 
 file->Close();
 sleep(1);
 printf("\033[35mInfo::EndUser() called\033[m \n");
 a->EndUser();
 sleep(1);
 TDatime date;
 printf("End at Date :%d Time : %d \n",date.GetDate(),date.GetTime());
 printf("\033[31mInfo::Abort called\033[m \n");
 abort();
 //exit(0);
 }
bool isPortInUse(int port) {

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return true; // impossible de créer => on suppose occupé
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    bool inUse = false;

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        inUse = true; // échec => port occupé
    }

 

    close(sockfd);
    return inUse;

}
int main(){

  char InputConfig[100];
  int  Watcher_Port,SpyPort;
  
  TObjArray* toks=0;
  TString schaine; 

  gSystem->Exec("notify-send -i dialog-information \"Starting adne:spy by $USER\"");
  //cout<<"Call::Yaml_config_files/config.yaml"<<endl;
  YAML::Node config = YAML::LoadFile("Yaml_config_files/config.yaml");
  
  std::string IP_Narval	=	config["spy"]["ip_adress"].as<std::string>();
  //sprintf(InputConfig,"%s",IP_Narval);
  //TString tmp(IP_Narval.c_str());	
  
  
  
  // Récupère le nom d'utilisateur depuis la variable d'environnement
    const char *username = getenv("USER");

    // Si USER n'existe pas (cas rare), essaie USERNAME (Windows)
    if (username == NULL) {
        username = getenv("USERNAME");
    }

    // Affiche le nom d'utilisateur
    if (username != NULL) {
        printf("Bonjour / Hello / Gute Morgen, %s !\n", username);
    } else {
        printf("Impossible de déterminer le nom d'utilisateur.\n");
    }
  
  
  
  cout<<"Spy on "<<IP_Narval<<" requested"<<endl;
  
  file = new GNetClientNarval(IP_Narval.c_str()); //ip of the acquisition computer running Narval 
  
  
  a= new GUser(file);
  Watcher_Port	=	 config["spy"]["spy_port"].as<int>();	
  //fscanf(runtop,"%d\n",&Watcher_Port);
  file->SetPort(Watcher_Port);
  printf("\033[35m --> Spy  listenning on  port %d \033[m \n",Watcher_Port);  
  file->SetBufferSize(16384);
  
  SpyPort	= 	config["spy"]["spectra_port"].as<int>();
  
	while(isPortInUse(SpyPort)) {
        	std::cout << "Le port " << SpyPort << " est OCCUPE\n";
		SpyPort++;
    	} 
        std::cout << "Le port " << SpyPort << " est libre\n";

  //GAcq * a= new GAcq(file);
  GNetServerRoot *serv = new GNetServerRoot(SpyPort,a);
  serv->StartServer();
  //file->Open();
   

 // a->EventInitWithFileName("/home/vamos_upgrade/ganacq_manip/vamos_agata/GECO/vamos_agata/ACQ/ACTIONS_vamos_agata.CHC_PAR","/home/vamos_upgrade/ganacq_manip/vamos_agata/GECO/vamos_agata/ACQ/ACTIONS_vamos_agata.CHC_STR"); 
  a->EventInit(const_cast<char*>("ACTIONS_experiment.CHC_PAR"),const_cast<char*>("mfm"));
   
  a->SetSpectraMode(1);
  a->InitUser();
  a->LoadCut(false);	
  a->OnlyTreeConversion(false); //ie means here that Is() and Treat() methods are called
  signal(SIGINT, StopSpy);
  
   a->DoRun();  
   serv->StopServer();                   
   file->Close();
   
 
    a->EndUser();
 //  delete (a);
   TDatime date;
   
   printf(" End at Date :%d Time : %d \n",date.GetDate(),date.GetTime());
   return 0;
}


