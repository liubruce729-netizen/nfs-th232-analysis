
/*


	Orignial Version 1.1 � 25Feb2010
	E.Clement (CNRS/IN2P3/GANIL)
	
  
   Point on File

*/
#include "AnalysisADNE.h"
#include "./GUser.h"
#include "GMFMFile.h"
#include <unistd.h>
#include <signal.h>
#include <yaml-cpp/yaml.h> 
#include <stdlib.h>
#include <cerrno>
#include <cctype>
#include <limits>
#include <string>

using namespace std;


namespace {
std::string TrimConfigValue(std::string value)
{
	std::string::size_type first = 0;
	while(first < value.size() && std::isspace(static_cast<unsigned char>(value[first]))) first++;
	std::string::size_type last = value.size();
	while(last > first && std::isspace(static_cast<unsigned char>(value[last-1]))) last--;
	return value.substr(first,last-first);
}

bool StripSecondsSuffix(std::string &value)
{
	value = TrimConfigValue(value);
	if(value.empty()) return false;
	char last = value[value.size()-1];
	if(last == 's' || last == 'S') {
		value.erase(value.size()-1);
		value = TrimConfigValue(value);
		return true;
	}
	return false;
}

bool ParseNonNegativeSeconds(std::string value, Double_t &seconds)
{
	value = TrimConfigValue(value);
	if(value.empty()) return false;
	char *end = NULL;
	errno = 0;
	Double_t parsed = strtod(value.c_str(),&end);
	if(end == value.c_str() || errno != 0) return false;
	while(end && *end != '\0') {
		if(!std::isspace(static_cast<unsigned char>(*end))) return false;
		end++;
	}
	if(parsed < 0.) return false;
	seconds = parsed;
	return true;
}

bool ParseNonNegativeUInt(std::string value, UInt_t &number)
{
	value = TrimConfigValue(value);
	if(value.empty() || value[0] == '-') return false;
	char *end = NULL;
	errno = 0;
	unsigned long long parsed = strtoull(value.c_str(),&end,10);
	if(end == value.c_str() || errno != 0) return false;
	while(end && *end != '\0') {
		if(!std::isspace(static_cast<unsigned char>(*end))) return false;
		end++;
	}
	if(parsed > std::numeric_limits<UInt_t>::max()) {
		number = std::numeric_limits<UInt_t>::max();
		return true;
	}
	number = static_cast<UInt_t>(parsed);
	return true;
}
}

bool isPortInUse(int port) {

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return true; // impossible de cr�er => on suppose occup�
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    bool inUse = false;

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        inUse = true; // �chec => port occup�
    }

 

    close(sockfd);
    return inUse;

}

void usage()
{

   cout<<"Usage: "<<endl;
   cout<<"AnalysisADNE takes as input a run list in an ascii file"<<endl;
   cout<<"Without argument, the default run list is Yaml_config_files/config.yaml"<<endl;
   cout<<"With an argument, it will process the run list given in argument"<<endl;
}
 

int main(int argc, char* argv[]){

	char RunToProcess[100];
	char RunRoot[100];
	char ligne[5];
	char ScalerFile[100];
	string input_directory;
	struct SysInfo_t infos;
	struct MemInfo_t ramInfo;
	double sizeInMB;
	double difference;
	TObjArray* toks=0;
	TString schaine; 
	int RunNumb,RunNumbSub,SpyPort;
	FILE * runtop;
	gSystem->GetSysInfo(&infos);
	gSystem->GetMemInfo(&ramInfo);
	
	
	// R�cup�re le nom d'utilisateur depuis la variable d'environnement
    const char *username = getenv("USER");

    // Si USER n'existe pas (cas rare), essaie USERNAME (Windows)
    if (username == NULL) {
        username = getenv("USERNAME");
    }

    // Affiche le nom d'utilisateur
    if (username != NULL) {
        printf("Bonjour / Hello / Gute Morgen, %s !\n", username);
    } else {
        printf("Impossible de d�terminer le nom d'utilisateur.\n");
    }
  
  
	
	
	printf("\033[33mWELCOME : \033[m \n");
	
	printf("\033[33m****************************** \033[m \n");
	cout<< "The CPU speed  "<<infos.fCpuSpeed<< " MHz"<< " |  Physical RAM "<<infos.fPhysRam <<" MB"<<endl;
	cout<<"The available RAM  is "<<ramInfo.fMemFree<<" MB"<<endl;
	cout << "" << endl;
	gSystem->Exec("notify-send -i dialog-information \"Starting adne:analysis by $USER\"");
	
	YAML::Node config = YAML::LoadFile("Yaml_config_files/config.yaml");
	std::string inputFile  = config["analysis"]["filename"].as<std::string>();
	UInt_t maxEventsToProcess = 0; // EN: numeric max_events means processed events after start_time. CN: 数字形式max_events表示start_time之后处理的event数
	Bool_t maxEventsIsTimeWindow = false;
	Double_t maxTimeWindowSeconds = 0.;
	Double_t startTimeSeconds = 0.; // EN: seconds after the first frame TS. CN: 相对第一个frame时间戳的起始秒数
	if(config["analysis"]["max_events"]) {
		std::string configuredMaxEvents = config["analysis"]["max_events"].as<std::string>();
		std::string maxValue = configuredMaxEvents;
		Bool_t hasSecondsSuffix = StripSecondsSuffix(maxValue);
		if(hasSecondsSuffix) {
			Double_t parsedSeconds = 0.;
			if(ParseNonNegativeSeconds(maxValue,parsedSeconds)) {
				if(parsedSeconds > 0.) {
					maxEventsIsTimeWindow = true;
					maxTimeWindowSeconds = parsedSeconds;
				}
			}
			else {
				cerr << "Invalid analysis.max_events time value: " << configuredMaxEvents << endl;
			}
		}
		else {
			UInt_t parsedEvents = 0;
			if(ParseNonNegativeUInt(maxValue,parsedEvents)) {
				maxEventsToProcess = parsedEvents;
			}
			else {
				cerr << "Invalid analysis.max_events value: " << configuredMaxEvents << "; use an integer event count or a seconds value such as 30s" << endl;
			}
		}
	}
	if(config["analysis"]["start_time"]) {
		std::string configuredStartTime = config["analysis"]["start_time"].as<std::string>();
		std::string startValue = configuredStartTime;
		StripSecondsSuffix(startValue);
		Double_t parsedStartTime = 0.;
		if(ParseNonNegativeSeconds(startValue,parsedStartTime)) {
			startTimeSeconds = parsedStartTime;
		}
		else {
			cerr << "Invalid analysis.start_time value: " << configuredStartTime << "; use seconds, e.g. 0s or 12.5s" << endl;
		}
	}
	if(config["analysis"]["start_event"]) {
		cerr << "analysis.start_event is deprecated and ignored; use analysis.start_time in seconds" << endl;
	}
	if(startTimeSeconds > 0.) {
		cout << "ADNE start time per input file: " << startTimeSeconds << " s after first frame TS" << endl;
	}
	if(maxEventsIsTimeWindow) {
		cout << "ADNE max time window per input file: " << maxTimeWindowSeconds << " s after start_time" << endl;
	}
	else if(maxEventsToProcess > 0) {
		cout << "ADNE max events per input file after start_time: " << maxEventsToProcess << endl;
	}
	
	
	
	
	if(argc==1){
		cerr<<"->I will proceed the default run list from the Yaml_config_files "<<endl;
		runtop=fopen(inputFile.c_str(),"r");cout << "" << endl;
	}
	else if (argc==2){
	 	string arg(argv[1]);
		if(strcmp(argv[1], "help") == 0) {
			usage();
			return 0;	
		}
		else {
	 		cerr<<"->I will proceed a specifix run list ::"<< arg<<endl;
	 		runtop=fopen(arg.c_str(),"r");cout << "" << endl;
		}
	}
	else{
		usage();
		return 0;
	}
	
	if(runtop== NULL) return 0; 

	fscanf(runtop,"%s\n",RunToProcess);
	schaine.Form("%s",RunToProcess);
	
	while(schaine.BeginsWith("#")){
		fscanf(runtop,"%s\n",RunToProcess);
		schaine.Form("%s",RunToProcess);
	}
	rewind(runtop);

	GMFMFile *file= new GMFMFile(RunToProcess);
	GUser * aa= new GUser(file);  
	file->SetBufferSize(16384*4);
	file->Open();
	
	SpyPort= config["analysis"]["spectra_port"].as<int>();
	
	while(isPortInUse(SpyPort)) {
        	std::cout << "Le port " << SpyPort << " est OCCUPE\n";
		SpyPort++;
    	} 
        std::cout << "Le port " << SpyPort << " est libre, I use it\n";

	GNetServerRoot *serv = new GNetServerRoot(SpyPort,aa);
	
	
	
	aa->EventInit(const_cast<char*>("ACTIONS_experiment.CHC_PAR"),const_cast<char*>("mfm")); //ACTION FILE specified in GUser:: Constructor
	aa->InitUser();

	
	//printf("\033[32mInfo:: Ignore the two previous error messages \033[m \n");

	
	file->Close();
	//aa->SetSpectraMode(1);
	//aa->SetUserMode(1);     // Do GUser::InitUser()
	
	
	

	while (fscanf(runtop,"%s\n",RunToProcess) != EOF) {

		schaine.Form("%s",RunToProcess);
		if(schaine.BeginsWith("//")) {
			cerr<<RunToProcess <<"=>skipped"<<endl;
			continue;
		}
		if(schaine.BeginsWith("#"))  {
			cerr<<RunToProcess<< "=>skipped"<<endl;
			continue;
		}
		
		
		printf("\033[32mInfo:: File to process is %s \033[m \n",RunToProcess);
		
		
		//calculate the file size
		std::ifstream Testfile(RunToProcess, std::ios::binary | std::ios::ate);
		if (!Testfile.is_open()) {
        		std::cerr << "Erreur : Impossible d'ouvrir le fichier " << RunToProcess << std::endl;
	        	return -1.0;
    		}
		std::streamsize size = Testfile.tellg();
		Testfile.close();
		sizeInMB = static_cast<double>(size) / (1024.0 * 1024.0);
		
		
		
		
		file->SetDevice(RunToProcess);
		file->Open();
		cerr<< "File Open"<<endl;
		printf("\033[32mInfo:: File Open\033[m \n");

		schaine.ReplaceAll("data/","");
		
		
		
		
		if(schaine.BeginsWith("run_")){
			schaine.ReplaceAll("run_","");
			toks = schaine.Tokenize(".");
			RunNumb= ((TObjString* )toks->At(0))->GetString().Atoi();
			//if((Int_t)((TObjString* )(toks->GetEntries()))==3){RunNumbSub= 0;}
			if((Int_t)((toks->GetEntries()))==3){RunNumbSub= 0;}
			else {RunNumbSub= ((TObjString* )toks->At(3))->GetString().Atoi();}
		}
		else{
			cerr<<" That's not a what I expected "<<endl; 
			//gets(ligne);
			RunNumb=RunNumbSub=0;
			break;
		}
		
                sprintf(RunRoot,"out/run_%d_r%d.root",RunNumb,RunNumbSub);
		
	
	
	
	
	
	bool exogam2		=	config["analysis"]["tree_exogam2"].as<bool>();
	bool trigger		=	config["analysis"]["tree_trigger"].as<bool>();
	bool MW			=	config["analysis"]["tree_MW"].as<bool>();
	bool CsI		=	config["analysis"]["tree_CsI"].as<bool>();
	bool Neda		=	config["analysis"]["tree_Neda"].as<bool>();
	bool Paris		=	config["analysis"]["tree_Paris"].as<bool>();
	bool Generic		=	config["analysis"]["tree_Generic"].as<bool>();
	bool EbyE		=	config["analysis"]["tree_EbyE"].as<bool>();
	bool exogam2REA		=	config["analysis"]["tree_exogam2REA"].as<bool>();	
	bool VamosIC		=	config["analysis"]["tree_VamosIC"].as<bool>();
	bool onlytreeconv	= 	config["analysis"]["OnlyTreeConversion"].as<bool>();
	bool rawTree		=	false;
	bool nfsTree		=	false;
	YAML::Node nfsExoAna = config["nfs_exo_ana"];
	if(nfsExoAna && nfsExoAna["raw_tree"]) rawTree = nfsExoAna["raw_tree"].as<bool>();
	if(nfsExoAna && nfsExoAna["tree"]) nfsTree = nfsExoAna["tree"].as<bool>();
	if(rawTree) cout<<"NFS raw MFM tree enabled"<<endl;
	if(nfsTree) cout<<"NFS extracted tree enabled"<<endl;
	
		if(exogam2||trigger||MW||CsI||Neda||Paris||Generic||EbyE||exogam2REA||VamosIC||rawTree||nfsTree){ //We produce a ROOT tree, so stop the spy to avoid the Ubuntu22 mux() problem
				serv->StopServer();
				aa->OnlyTreeConversion(onlytreeconv); //false means that we also call the Treat() methods after Is(); true means only Is() methods is call
				aa->InitTTreeUser(RunRoot,exogam2,trigger,MW,CsI,Neda,Paris,Generic,EbyE,exogam2REA,VamosIC,rawTree);	//this will make TreeFillBool and/or RawTreeFillBool true			
				aa->LoadCut(false); // false means that CutG will not be read due to mux prob
				printf("\033[32m -------------  Type 1  **********   \033[m \n");


				
		}//
		else{ //no Tree
				aa->OnlyTreeConversion(onlytreeconv);//false means that we also call the Treat() methods after Is(); true means only Is() methods is call
				aa->LoadCut(true); //ie spy from a Run file; false means that CutG will be read due to mux prob
				serv->StartServer();
				printf("\033[32m -------------  Type 2  **********   \033[m \n");


		}
		
 		printf("\033[32m **********  Run #%d sub %d treatment starts **********   \033[m \n",RunNumb,RunNumbSub);
		aa->SetRunTimeWindow(startTimeSeconds,maxEventsToProcess,maxEventsIsTimeWindow,maxTimeWindowSeconds);
		 //Start Time
		std::time_t start = std::time(nullptr);	
		if(maxEventsToProcess > 0 && startTimeSeconds <= 0. && !maxEventsIsTimeWindow) aa->DoRun(maxEventsToProcess);
		else aa->DoRun();
		serv->StopServer();
		file->Close();
		aa->EndUser(); 
		std::time_t end = std::time(nullptr);
		difference = std::difftime(end, start);
		printf("\033[33mProcessing time was %lf secondes at %f Mo/sec.\033[m \n",difference,sizeInMB/difference);
		
		
	}
	
	   gSystem->Exec("notify-send -i dialog-warning \"Analysis is now completed by $USER\"");

	cout<< "Continue (press enter) " <<endl;
	fgets(ligne,sizeof(ligne),stdin);
	//delete (aa);
	cout << "delete GUser"<<endl;
	//delete (file);
	cout << "delete GMFMFile"<<endl;

   TDatime date;
   //printf("Leaving the processing at Date : %d Time : %d \n",date.GetDate(),date.GetTime());
   
   	std::time_t now = std::time(nullptr);
   	std::tm* localTime = std::localtime(&now);
	char buffer[80];
 	std::strftime(buffer, sizeof(buffer), "%A, %d %B %Y - %H:%M:%S", localTime);
   // Affiche la date et l'heure avec des couleurs pour xterm
    std::cout << "\033[1;36m" // Cyan clair et gras

              << "Closing the process : "

              << "\033[1;36m" // Blanc gras

              << buffer

              << "\033[0m"    // R�initialise la couleur

              << std::endl;
   return 0;
}


