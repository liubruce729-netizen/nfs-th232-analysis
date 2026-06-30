#include <TApplication.h>
#include <TGClient.h>
#include <TString.h>
#include "General.h"

#include "VigruMain.h"

int main(int argc, char **argv) {

	int soapportdef = 6603;
	int soapport = 6603;
	int testport = 0;
	int rootportdef = 9090;
	int rootport = 9090;
	char *tempo, *wordtempo;
	int SpecificMode = 0;// specific mode , 0 no mode, 1, demo mode , 2 oscillo mode.
	tempo = NULL;
	wordtempo = NULL;
	int nb_char = 0;
	int pid = (int) getpid();
	char* version = (char*) GRU_VERSION;
	int verbose = 0;
	TString configfile;
	configfile = "";
	TString rootfile;
	rootfile = "";
	TString rootserver;
	rootserver = "";
	TString soapserver;
	soapserver = "";
	int treatedinfo = 1;
	for (int i = 1; i < argc; i++) {

		if ((strcasecmp(argv[i], "-h") == 0) or (strcasecmp(argv[i], "--help")
				== 0)) {
			printhelp(rootportdef, soapportdef);
			treatedinfo++;
			return (0);
		}

		nb_char = strlen(argv[i]) + 1;
		tempo = new char[nb_char];
		tempo = strcpy(tempo, argv[i]);
		strcpy(tempo, argv[i]);
		wordtempo = strtok(tempo, "=");

		if ((strcasecmp(wordtempo, "-sp") == 0) or (strcasecmp(wordtempo,
				"--soapport") == 0)) {
			wordtempo = strtok(NULL, "=");
			if (strcasecmp(wordtempo, "") == 0) {
				cout << " command not good\n";
				exit(0);
			}
			testport = atoi(wordtempo);
			if ((testport > 1000) and (testport < 20000)) {
				soapport = testport;
				if (soapserver == "")
					soapserver = "localhost";
			}
			treatedinfo++;
		}

		if ((strcasecmp(wordtempo, "-rp") == 0) or (strcasecmp(wordtempo,
				"--rootport") == 0)) {
			wordtempo = strtok(NULL, "=");

			if (wordtempo == NULL) {
				cout << " command not good\n";
				exit(0);
			}
			if (strcasecmp(wordtempo, "") == 0) {
				cout << " command not good\n";
				exit(0);
			}

			testport = atoi(wordtempo);
			if ((testport > 1000) and (testport < 20000)) {
				rootport = testport;
				if (rootserver == "")
					rootserver = "localhost";
			}
			treatedinfo++;
		}

		if ((strcasecmp(wordtempo, "-rf") == 0) or (strcasecmp(wordtempo,
				"--rootfile") == 0)) {
			wordtempo = strtok(NULL, "=");

			if (wordtempo == NULL) {
				cout << " command not good\n";
				exit(0);
			}
			if (strcasecmp(wordtempo, "") == 0) {
				cout << " command not good\n";
				exit(0);
			}
			rootfile = wordtempo;
			treatedinfo++;
		}

		if ((strcasecmp(wordtempo, "-ss") == 0) or (strcasecmp(wordtempo,
				"--soapserver") == 0)) {
			wordtempo = strtok(NULL, "=");
			if (wordtempo == NULL) {
				cout << " command not good\n";
				exit(0);
			}
			if (strcasecmp(wordtempo, "") == 0) {
				cout << " command not good\n";
				exit(0);
			}
			soapserver = wordtempo;
			if (soapport == 0)
				soapport = soapportdef;
			treatedinfo++;
		}
		if ((strcasecmp(wordtempo, "-rs") == 0) or (strcasecmp(wordtempo,
				"--rootserver") == 0)) {
			wordtempo = strtok(NULL, "=");
			if (wordtempo == NULL) {
				cout << " command not good\n";
				exit(0);
			}
			if (strcasecmp(wordtempo, "") == 0) {
				cout << " command not good\n";
				exit(0);
			}
			rootserver = wordtempo;
			if (rootport == 0)
				rootport = rootportdef;
			treatedinfo++;
		}
		if ((strcasecmp(wordtempo, "-cf") == 0) or (strcasecmp(wordtempo,
				"--config") == 0)) {
			wordtempo = strtok(NULL, "=");
			if (wordtempo == NULL) {
				cout << " command not good\n";
				exit(0);
			}
			if (strcasecmp(wordtempo, "") == 0) {
				cout << " command not good\n";
				exit(0);
			}
			configfile = wordtempo;
			treatedinfo++;
		}
		if ((strcasecmp(wordtempo, "-d") == 0) or (strcasecmp(wordtempo,
				"--default") == 0)) {
			rootserver = "";
			soapserver = "";
			soapport = soapportdef;
			rootport = rootportdef;
			configfile = "";
			rootfile = "";
			i = 100000;
			treatedinfo++;
		}
		if ((strcasecmp(argv[i], "--demo") == 0) ) {
						SpecificMode =1;
						treatedinfo++;
		}
		if ((strcasecmp(argv[i], "--oscillo") == 0) ) {
						SpecificMode =2;
						treatedinfo++;
					}

		if ((strcasecmp(wordtempo,"--verbose") == 0)) {
					wordtempo = strtok(NULL, "=");
					if (wordtempo == NULL) {
						cout << " command not good\n";
						exit(0);
					}
					if (strcasecmp(wordtempo, "") == 0) {
						cout << " command not good\n";
						exit(0);
					}
					verbose = atoi(wordtempo);
					treatedinfo++;
				}

		if ((argc == 2) and (treatedinfo == 1)) {
			configfile = argv[2];
			treatedinfo++;
		}
	}

	if (argc == 1) {
		configfile = "vigru.xml";
	}

	cout << "  *******************************************\n";
	cout << "  *                                         *\n";
	cout << "  *  HELLO  -- You are Running  vigru       *\n";
	cout << "  *   Version : " << version << "                    *\n";
	cout << "  *   PID     : " << pid << "                       *\n";
	cout << "  *******************************************\n";

	if ((rootport != 0) && (rootserver != ""))
		cout << "       Root Server = " << rootserver.Data() << ":" << rootport
				<< "\n";
	if ((soapport != 0) && (soapserver != ""))
		cout << "       Soap Server = " << soapserver.Data() << ":" << soapport
				<< "\n";
	if (rootfile != "")
		cout << "       Root File   = " << rootfile.Data() << "\n";
	if (configfile != "")
		cout << "       Config File =  " << configfile.Data() << "\n";
	Vigru *vigru;
	TApplication * theApp = new TApplication("App", &argc, argv);
	vigru = new Vigru(gClient->GetRoot(), 800, 700, configfile, rootfile,
			rootserver, soapserver, rootport, soapport,SpecificMode);
	vigru->SetVerbose(verbose);
	vigru->SetTeminate(false);
	theApp->Run(true);
	cout << " Bye Bye Vigru!";
	if (vigru) {
		delete (vigru);
		vigru = NULL;
	}
	if (theApp) {
		delete (theApp);
		theApp = NULL;
	}
	return 0;
}

void printhelp(int rootportdef, int soapportdef) {
	cout
			<< " Usage : vigru [-rf=file -ss=host -sr=host -rp=n -sp=n -cf=file -h -d] \n";
	cout << "\t --help or -h to get this help\n";
	cout << "\t --rootport=nb or -rp=nb, nb define root port for root server\n";
	cout << "\t   else default is " << rootportdef << " \n";
	cout << "\t --soapport=nb or -sp=nb, nb define soap port for soap server\n";
	cout << "\t   else default is " << soapportdef << " \n";
	cout << "\t --soapserver=servername or -ss=servername,  servername define server of soap server\n";
	cout << "\t    name else default is localhost\n";
	cout << "\t --rootserver=servername or -rs=servername,  servername define server of root server\n";
	cout << "\t    name else default is localhost\n";
	cout << "\t --rootfile=filename or -rf=filename,  filename define root file\n";
	cout << "\t   name containing root spectra\n";
	cout << "\t --default or -d to launch vigru without any preconfig\n";
	cout << "\t --config=filename or -cf=filename,  filename define xml file\n";
	cout << "\t   name containing configuration\n";
	cout << "\t --demo : run demo spectra in memo device\n";
	cout << "\t --oscillo : run specific configruation in oscilloscope mode, the soapserver and port soap server \n";
	cout << "\t             have be notified with --soapport and --soapserver  \n";
	cout << "\t --verbose=nb : verbose mode( 0<=nb<=10), default nb = 0 ( no verbose) \n";
	cout << "\t if there is only one argument is taken as configfile ,\n";
	cout
			<< "\t config file name (must be a .xml file ex: \"vigru config.xml\")\n";
	return;
}
