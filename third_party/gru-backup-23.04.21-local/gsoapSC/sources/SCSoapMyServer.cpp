#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<string>
#include<math.h>
#include<iostream>
#include<string>
#include<pthread.h>
#include <signal.h> //  our new library
volatile sig_atomic_t flag = 0;
// serveur SOAP
#include <signal.h>
#include "Utilities.h"


#include "SCSoapH.h"
#include "SCSoap.nsmap"
#include <stdlib.h>
Namespace namespaces[1024];
using namespace std;

#define MFM_CHANNELID_BOARD_MSK 0xFFE0
#define MFM_CHANNELID_NUMBER_MSK 0x001F

#define MFM_CONFIG_ONOFF_MSK    0x8000
#define MFM_CONFIG_TIMEBASE_MSK 0x7800
#define MFM_CONFIG_TRIG_MSK     0x0700
#define MFM_CONFIG_SIGNAL_MSK   0x00E0
#define MFM_CONFIG_IDXCHAN_MSK  0x001F
#define MFM_OSCILLO_HEADERSIZE  20

const int mystrlen = 256;
pthread_t pthread_server; // l'identifiant du
pthread_t pthread_spectra_alive; // l'identifiant du
int nb_spectra;// nb of spectra
uint16_t **spectra_us; // vector of vector of spectra
uint32_t *spectra_size;//vector of size of vector


int value_max = 65536;
int size_std = 8192;
int size_max = 1048576;
float rando;
int nb_increment = 0;
int counter=0;
void ResetAllHisto();
int ResetHisto();
void CreatHistos();
void * SpectraAlive();

void InCaseOfControlC(int sig);



//____________________________________________________________________
void GetDumpRawReal(void *point, int dumpsize, int increment,
		string * mydump) {
	// Method to dump buffer on  output
	// if dumpsize =0 , dumpsize = standard = 256 and fGBbuf_increment raz
	// if mydump == NULL result is printed in standard output
	// if mydump is defined output is put in sting mydump
    // dump exacly what is in computer memory
	string *mydumploc = NULL;
	string st;

	if (mydump == NULL) {

		mydumploc = &st;
	} else {
		mydumploc = mydump;
	}

	int i, k;
	int nbrcol = 16; // nb de colonnes affich�es

	int asciimin = 32; // range min of a char to be ascii character
	char tempo[20] = "";
	int asciimax = 127; // idem but max
	int nbrperline = 16; // nbr of bytes per line
	int nbrline = 16; // nbr lines
	unsigned char *pChar;
	unsigned char *pChar2;

	//cout << "debug  MFMCommonFrame::GetDumpRaw2 dumpsize " << dumpsize
		//	<< " increment " << increment << "\n";

	nbrline = (int) (dumpsize / nbrperline) + (int) ((dumpsize % nbrperline)
			!= 0);

	pChar  = (unsigned char *)  ((char*) point + increment);
	pChar2 =  pChar;

	if (nbrline < 1)
		nbrline = 1;

	if (increment == dumpsize)
					nbrline = 1;
	for (i = 0; i < nbrline; i++) {
		sprintf(tempo, "\n%5d %s ", increment, ": ");
		(*mydumploc) += tempo;

		for (k = 0; k < nbrcol; k++) {
			sprintf(tempo,"%02hX ",(unsigned short)(*(pChar2++)));
			*mydumploc += tempo;
		}
		*mydumploc += "  ";
		for (k = 0; k < nbrperline; k++) {
			if ((*pChar >= asciimin) && (*pChar < asciimax)) {
				sprintf(tempo, "%c", *pChar);
				*mydumploc += tempo;
			} else
				*mydumploc += ".";
			pChar++;
		}
		increment += nbrperline;
	}
	*mydumploc += "\n";

	if (mydump == NULL)
		cout << (*mydumploc).data();

}

//_________________________________________________________________________________________
int sc__ReadScope(struct soap *p_soap, xsd__base64Binary_test &p_out) {
	int i, j, indice;

	int local_type;
	p_out.__ptr = NULL;
	p_out.__size =0;
	int header_size = MFM_OSCILLO_HEADERSIZE;
	int sizeofcontener =0;
	int sizeofelement = sizeof(unsigned short);

    for (int i = 0 ; i<nb_spectra; i++){
    	sizeofcontener += spectra_size[i]*sizeofelement+header_size;
    }

	printf("Server sc__ReadScope %d\n",counter++);

	unsigned int valueu;
	unsigned char valuec;
	unsigned int *spectra_currentu;
	unsigned char *spectra_currentc;



	// header for mfm with size of 16k elements,
	// 81 0A 40 00   00 11 00 01   0A 00 02 00   00 40 00 00    01 00 41 F9
	// convertion in decimal
	// 81->129 0A->10  40->64  00->00
	// 00->00  11->17  00->00  01->01
	// 0A->10  00->00  02->02  00->00
	// 00->00  40->64  00->00  00->00
	// 01->01  00->00  41->101 F9->249

	p_out.__size = sizeofcontener;
	p_out.__ptr = (unsigned char*) soap_malloc(p_soap,p_out.__size);

#define MFM_CHANNELID_BOARD_MSK 0xFFE0
#define MFM_CHANNELID_NUMBER_MSK 0x001F

#define MFM_CONFIG_ONOFF_MSK    0x8000
#define MFM_CONFIG_TIMEBASE_MSK 0x7800
#define MFM_CONFIG_TRIG_MSK     0x0700
#define MFM_CONFIG_SIGNAL_MSK   0x00E0
#define MFM_CONFIG_IDXCHAN_MSK  0x001F
#define MFM_OSCILLO_HEADERSIZE  20

	char header[]= {
			(char)0x81,0x0A,0x40,0x00,
			0x00,0x11,0x00,0x01,
			0x0A,0x00,0x02,0x00,
			0x00,0x40,0x00,0x00,
			0x00,0x00,0x40,(char)0xF9
	};

	char* thecopy = (char*) p_out.__ptr;
	const char* tocopy ;
	const char* ptonframe ;
	int16_t channel =0;
	int16_t cardchannel=112;

	int16_t config_onof =0;
	int16_t timefactor=0;
	int16_t tigger=0;
	int16_t signal=0;
	int16_t nochanneltomeme=0;


    int16_t * ptchannel;
    int16_t * ptconfig;
    ptchannel = (int16_t*)(&(header[16]));
    ptconfig  = (int16_t*)(&(header[18]));


	for (int i = 0 ; i<nb_spectra; i++){
		channel = (i %4)+1;
		timefactor = i%16;
		timefactor =16/channel;
		*ptchannel=((cardchannel<<5)&MFM_CHANNELID_BOARD_MSK) + (channel&MFM_CHANNELID_NUMBER_MSK) ;

		*ptconfig =
				((config_onof<<15)&MFM_CONFIG_ONOFF_MSK)+
				((timefactor<<11)&MFM_CONFIG_TIMEBASE_MSK)+
				((tigger<<8)&MFM_CONFIG_TRIG_MSK)+
				((signal<<5)&MFM_CONFIG_SIGNAL_MSK)+
				((nochanneltomeme)&MFM_CONFIG_IDXCHAN_MSK);

		tocopy = (const char*)header;
		ptonframe=thecopy;
		memcpy(thecopy, tocopy, header_size);
		thecopy+=header_size;
		tocopy = (const char*) spectra_us[i];
		memcpy(thecopy, tocopy,spectra_size[i]*sizeofelement);
		thecopy+=spectra_size[i]*sizeofelement;
		GetDumpRawReal((void*) ptonframe,64,0,NULL);
		printf("\n-------\n");
	}



	return SOAP_OK;
}
int sc__ServerExit(struct soap *p_soap, sc__Response & response) {
	return SOAP_OK;
}

int sc__CreateDevice(struct soap *p_soap, std::string DeviceName,
		std::string Type, std::string LinuxDeviceName, sc__Response & response) {
	return SOAP_OK;
}
;

int sc__CreateMemoryDevice(struct soap *p_soap, std::string DeviceName,
		std::string LinuxDeviceName, unsigned int LowAddress,
		unsigned int HighAddress, sc__Response & response) {
	return SOAP_OK;
}
;

int sc__CreateRegister(struct soap *p_soap, std::string DeviceName,
		std::string RegisterName, unsigned int OffsetAddress,
		std::string RegType, std::string RegAccess, sc__Response & response) {
	return SOAP_OK;
}
;

int sc__CreateRegister_DefVal(struct soap *p_soap, std::string DeviceName,
		std::string RegisterName, unsigned int OffsetAddress,
		std::string RegType, std::string RegAccess, std::string DefValue,
		sc__Response & response) {
	return SOAP_OK;
}
;

int sc__WriteRegister(struct soap *p_soap, std::string DeviceName,
		std::string RegisterName, std::string Data,
		sc__ResponseString & response) {
	return SOAP_OK;
}
;

int sc__ReadRegister(struct soap *p_soap, std::string DeviceName,
		std::string RegisterName, sc__ResponseRegister & response) {
	return SOAP_OK;
}
;

int sc__CreateRegisterInt(struct soap *p_soap, std::string DeviceName,
		std::string RegisterName, unsigned int OffsetAddress,
		std::string RegType, std::string RegAccess, sc__Response & response) {
	return SOAP_OK;
}
;

int sc__CreateRegisterInt_DefVal(struct soap *p_soap, std::string DeviceName,
		std::string RegisterName, unsigned int OffsetAddress,
		std::string RegType, std::string RegAccess, int DefValue,
		sc__Response & response) {
	return SOAP_OK;
}
;

int sc__WriteRegisterInt(struct soap *p_soap, std::string DeviceName,
		std::string RegisterName, int Data, sc__ResponseInt & response) {
	return SOAP_OK;
}
;

int sc__ReadRegisterInt(struct soap *p_soap, std::string DeviceName,
		std::string RegisterName, sc__ResponseInt & response) {
	return SOAP_OK;
}
;

int sc__Reset(struct soap *p_soap, std::string DeviceName,
		sc__Response &Response) {
	return SOAP_OK;
}
;

int sc__GetInfo(struct soap *p_soap, sc__ResponseString &Response) {
	cout <<" test OK"<< endl;
	return SOAP_OK;
}
;

int sc__GetInfoDevice(struct soap *p_soap, std::string DeviceName,
		sc__ResponseString &Response) {
	return SOAP_OK;
}
;

int sc__SetDebugLevel(struct soap *p_soap, std::string DeviceName,
		int DebugLevel, sc__ResponseInt & response) {
	return SOAP_OK;
}
;

int sc__GetDebugLevel(struct soap *p_soap, sc__ResponseInt & response) {
	return SOAP_OK;
}
;

// In the following methods, set DeviceName = "" to access the Module services

int sc__Describe(struct soap *p_soap, std::string DeviceName, std::string,
		std::string, sc__ResponseString & response) {
	return SOAP_OK;
}
;

int sc__Prepare(struct soap *p_soap, std::string DeviceName, std::string,
		std::string, sc__ResponseString & response) {
	return SOAP_OK;
}
;

int sc__Configure(struct soap *p_soap, std::string DeviceName, std::string,
		std::string, sc__ResponseString & response) {
	return SOAP_OK;
}
;

int sc__Start(struct soap *p_soap, std::string DeviceName,
		sc__Response & response) {
	return SOAP_OK;
}
;

int sc__Stop(struct soap *p_soap, std::string DeviceName,
		sc__Response & response) {
	return SOAP_OK;
}
;

int sc__Breakup(struct soap *p_soap, std::string DeviceName,
		sc__Response & response) {
	return SOAP_OK;
}
;

int sc__Undo(struct soap *p_soap, std::string DeviceName,
		sc__Response & response) {
	return SOAP_OK;
}
;

int sc__GetState(struct soap *p_soap, std::string DeviceName,
		sc__ResponseState & response) {
	return SOAP_OK;
}
;
//_________________________________________________________________________________________

void * handler(void * p_soap) {

	struct soap * v_soap = (struct soap *) p_soap;
	pthread_detach(pthread_self());
	SCSoap_serve(v_soap);
	soap_destroy(v_soap);
	soap_end(v_soap);
	soap_done(v_soap);
	free(v_soap);
	printf(" Waiting for command>>>\n");
	pthread_exit(NULL);
	return NULL;
}

//_________________________________________________________________________________________

void * soapserver(void * newport) {
	pthread_detach(pthread_self());
	struct soap v_soap; // contexte du service SOAP
	struct soap *v_tsoap; // on clone le contexte SOAP pour le thread
	pthread_t v_tid; // l'identifiant du thread de connexion
	// on initialise la socket
	soap_init(&v_soap);
	v_soap.bind_flags = SO_REUSEADDR;
	int theport;
	theport = *((int*) newport);
	soap_set_namespaces(&v_soap, SCSoap_namespaces);

	// on cree la socket mere de connexion
	int code = soap_bind(&v_soap, NULL, theport, 100);
	if (code < 0) {
		soap_print_fault(&v_soap, stderr);
		printf("\nStarting Error : code %d \n", code);
		printf("\nWait one minute and retry \n");
		exit(0);
		pthread_exit(NULL);
		return NULL;
	}

	printf(" Demarrage serveur soap sur port : %d\n", theport);
	printf(" Waiting for command>>>\n");

	// on gere une boucle infinie pour recevoir les requ�tes
	for (;;) {
		v_soap.accept_timeout = 1; // on rend l'attente non bloquante
		if (soap_accept(&v_soap) < 0) {
			continue;
		}

		v_tsoap = soap_copy(&v_soap);
		pthread_create(&v_tid, NULL, handler, (void *) v_tsoap); //lancement du thread de connexion

	}
	printf(" End command>>>\n");
	soap_done(&v_soap);
	pthread_exit(NULL);// exit du theard serveur soap
	return NULL;
}
//_________________________________________________________________________________________
void * SpectraAlive(void * data) {
	int np = 10;
	int N, i, j;
	float P;
	unsigned int seed;
	float value;
	sleep(1);
	P = 1.570796327 * 2;

	seed = (unsigned int) rando;
	printf("pthread_spectra_alive created\n");
	sleep(20);
	printf("begin of modified values\n");
	for (;;) {
		nb_increment++;
		for (i = 0; i < nb_spectra; i++) {
			seed = (unsigned int) rando;
			rando = (float) ((rand_r(&seed) / (RAND_MAX + 1.0)))
					* (spectra_size[i] / 2);
			//rando = (float)(rand()*spectra_size[i]/2)/RAND_MAX;
			if (rando < 2)
				rando = 2;

			for (j = 0; j < spectra_size[i]; j++) {
				value = 64 * sin((float) j / spectra_size[i] * 2 * P * rando);
				value += spectra_us[i][j];
				if (value < 0)
					value = 0;
				if (value >= value_max)
					value = value_max - 1;
				spectra_us[i][j] = (uint16_t) value;
			}
		}
		sleep(5);
	}
}

//_________________________________________________________________________________________
void CreatHistos() {

	int i, j, data = 0;
	float P, value;
	P = 1.570796327 * 2;
	unsigned int seed;
	seed = 184987;
	char tempo[MAX_CARACTERES];
	rando = (128 * (rand_r(&seed) / (RAND_MAX + 1.0)));

	nb_spectra = 4;
	spectra_us = new uint16_t*[nb_spectra];
	spectra_size = new uint32_t[nb_spectra];

	for (i = 0; i < nb_spectra; i++) {
		spectra_size[i] = (int) (size_std * pow(2, i));
		spectra_size[i] = 16384;
		spectra_us[i]   = new uint16_t[spectra_size[i]];

	}
	ResetHisto();
	for (i = 0; i < nb_spectra; i++) {
		printf("spectra , size = %d,  pt spectra[%lu]\n",
				spectra_size[i],
				(long unsigned) (spectra_us[i]));
		for (j = 0; j < spectra_size[i]; j++) {
			value = 1024 * sin((float) j / spectra_size[i] * 2 * P * 8);
			spectra_us[i][j] = (unsigned short) (value + value_max / 2);
			//printf("%d", spectra[i][j]);
		}
		printf("\n");
	}
}
//_________________________________________________________________________________________
int ResetHisto() {
	int i, indice;
	for (indice = 0; indice < nb_spectra; indice++){
		for (i = 0; i < spectra_size[indice]; i++) {
			spectra_us[indice][i] = 0;
		}
	}
	return indice;
}


//_________________________________________________________________________________________
void ResetAllHisto() {
	int i, j;
	for (j = 0; j < nb_spectra; j++) {
		for (i = 0; i < spectra_size[j]; i++) {
			spectra_us[j][i] = 0;
		}
	}
}


//__________________________________________________________________________________________
void InCaseOfControlC(int sig) { // can be called asynchronously
	flag = 1; // set flag
	printf("-> Coucou c'est un kill ou un Ctr C alors on arrete !\n");
	pthread_cancel(pthread_spectra_alive);
	pthread_cancel(pthread_server);
	exit(0);
}

//_________________________________________________________________________________________

int main(int argc, char *argv[]) {

	int i, j;
	unsigned int entier;
	int newport = 0;
	char tempo[mystrlen];
	newport = 6603;
	int data = 0;

	signal(SIGINT, InCaseOfControlC);

	if (argc > 1) {
		newport = atoi(argv[1]);
		if ((newport >= 20000) and (newport <= 9000)) {
			printf("test sur port %d \n", newport);
		}
	}

	CreatHistos();

	pthread_create(&pthread_spectra_alive, NULL, SpectraAlive, (void *) (&data));
	//pthread_create(&pthread_server, NULL, soapserver, (void *) (&newport));
	soapserver(&newport);
	sleep(1);
	pthread_join(pthread_spectra_alive, NULL); // atten la fin du threat to continue.

	delete[] spectra_size;
	for (i = 0; i < nb_spectra; i++) {
		delete[] (spectra_us[i]);
	}
	delete[] spectra_us;
}
