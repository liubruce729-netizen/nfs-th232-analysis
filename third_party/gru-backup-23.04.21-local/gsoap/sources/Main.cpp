#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<string>
#include<math.h>
#include<iostream>
#include<pthread.h>
#include <signal.h> //  our new library
#include "Utilities.h"
#include "GruSoapMyServer.h"
#include "Main.h"
volatile sig_atomic_t flag = 0;

bool aliverunning = false;
const int mystrlen = 256;

pthread_t pthread_spectra_alive; // l'identifiant du

int nb_traces;// nb of spectra
unsigned short **spectra_us = NULL;
unsigned int **spectra_ui = NULL;
int *spectra_size = NULL;
char **spectra_name = NULL;
char **family_name = NULL;
unsigned short *spectra_type;//0 trace,1 histo

int value_max = 65536;
int size_std = 8192;
int size_max = 1048576;
float rando;
int nb_increment = 0;

void FreeVectors();
void InCaseOfControlC(int sig);
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
		for (i = 0; i < nb_traces; i++) {
			seed = (unsigned int) rando;
			rando = (float) ((rand_r(&seed) / (RAND_MAX + 1.0)))
					* (spectra_size[i] / 2);
			//rando = (float)(rand()*spectra_size[i]/2)/RAND_MAX;
			if (rando < 2)
				rando = 2;

			for (j = 0; j < spectra_size[i]; j++) {
				value = 64 * sin((float) j / spectra_size[i] * 2 * P * rando);
				value += spectra_ui[i][j];
				if (value < 0)
					value = 0;
				if (value >= value_max)
					value = value_max - 1;
				spectra_ui[i][j] = (unsigned int) value;
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

	nb_traces = 4;
	spectra_ui   = new unsigned int*[nb_traces];
	spectra_size = new int[nb_traces];
	spectra_name = new char*[nb_traces];
	family_name  = new char*[nb_traces];
	spectra_type = new unsigned short[nb_traces];

	for (i = 0; i < nb_traces; i++) {
		spectra_size[i] = (int) (size_std * pow(2, i));
		spectra_ui[i]   = new unsigned int[spectra_size[i]];

		sprintf(tempo, "SoapSpectra%d", spectra_size[i]);
		spectra_name[i] = new char[mystrlen];
		strcpy(spectra_name[i], tempo);

		sprintf(tempo, "MyFamilySoap");
		family_name[i] = new char[mystrlen];
		strcpy(family_name[i], tempo);

		spectra_type[i] = 0;
	}

	for (i = 0; i < nb_traces; i++) {
		printf("%s , size = %d, type = %d  pt spectra[%lu]\n", spectra_name[i],
				spectra_size[i], spectra_type[i],
				(long unsigned) (spectra_ui[i]));
		for (j = 0; j < spectra_size[i]; j++) {
			value = 1024 * sin((float) j / spectra_size[i] * 2 * P * 8);
			spectra_ui[i][j] = (unsigned short) (value + value_max / 2);
		}
		printf("\n");
	}
}
//_________________________________________________________________________________________
int ResetHisto(char* name, char* family) {
	int i, indice;

	indice = -1;
	if (family == NULL) {
		for (i = 0; i < nb_traces; i++) {
			if ((strcmp(name, spectra_name[i]) == 0)) {
				indice = i;
				break;
			}
		}

	} else {

		for (i = 0; i < nb_traces; i++) {

			if ((strcmp(name, spectra_name[i]) == 0) and (strcmp(family,
					family_name[i]) == 0)) {
				indice = i;
				break;
			}
		}

	}

	if (indice == -1) {
		printf("No spectrum found so no reset is not possible");
		return indice;
	} else {
		for (i = 0; i < spectra_size[indice]; i++) {
			spectra_ui[indice][i] = 0;
		}
	}
	return indice;
}

//_________________________________________________________________________________________
void ResetAllHisto() {
	int i, j;
	for (j = 0; j < nb_traces; j++) {
		for (i = 0; i < spectra_size[j]; i++) {
			spectra_ui[j][i] = 0;
		}
	}
}
//__________________________________________________________________________________________

void InCaseOfControlC(int sig) { // can be called asynchronously
	flag = 1; // set flag
	printf("-> Coucou c'est un kill ou un Ctr C alors on arrete !\n");
	pthread_cancel(pthread_spectra_alive);
	FreeVectors();
	exit(0);
}
//__________________________________________________________________________________________
void Launch_SpectraAlive() {

	int data = 0;

	CreatHistos();
	// test if no out signal
	if (aliverunning == false) {
		pthread_create(&pthread_spectra_alive, NULL, SpectraAlive,
				(void *) (&data));
		sleep(1);

	} else {
		printf("Stopping Launch_SpectraAlive\n");
		sleep(3);
		if (pthread_spectra_alive)
			pthread_cancel(pthread_spectra_alive);

		aliverunning = false;
	}
}
//__________________________________________________________________________________________
void FreeVectors() {

	printf (" Free vectors\n");
	if (spectra_type != NULL){
		delete[] spectra_type;
		spectra_type=NULL;
	}
	if (spectra_size != NULL){
		delete[] spectra_size;
		spectra_size=NULL;
	}
	int i;
	for (i = 0; i < nb_traces; i++) {
		if (spectra_ui[i] != NULL){
			delete[] (spectra_ui[i]);
			spectra_ui[i]=NULL;
		}
		if (spectra_name[i] != NULL){
			delete[] (spectra_name[i]);
			spectra_name[i]=NULL;
		}
		if (family_name[i] != NULL){
			delete[] (family_name[i]);
			family_name[i]=NULL;
		}
	}
	if (spectra_ui != NULL){
		delete[] spectra_ui;
		spectra_ui=NULL;
	}
	if (spectra_name != NULL){
		delete[] spectra_name;
		spectra_name=NULL;
	}
	if (family_name != NULL){
		delete[] family_name;
		family_name=NULL;
	}

}
//_________________________________________________________________________________________

int main(int argc, char *argv[]) {

	int i, j;
	unsigned int entier;
	int newport = 0;
	char tempo[mystrlen];
	newport = 6603;

	signal(SIGINT, InCaseOfControlC);

	if (argc > 1) {
		newport = atoi(argv[1]);
		if ((newport >= 20000) and (newport <= 9000)) {
			printf("test sur port %d \n", newport);
		}
	}

	CreatHistos();
	Launch_SpectraAlive();

	Launch_SoapServer(newport);

	pthread_join(pthread_spectra_alive, NULL); // atten la fin du threat to continue.
	FreeVectors();

}
