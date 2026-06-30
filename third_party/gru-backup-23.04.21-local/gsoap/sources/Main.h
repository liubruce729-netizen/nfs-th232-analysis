#ifndef GruSoapMain_H
#define GruSoapMain_H


#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<string>
#include<math.h>
#include<iostream>
#include<pthread.h>
#include <signal.h> //  our new library



extern int nb_traces;// nb of spectra
extern unsigned short **spectra_us;
extern unsigned int **spectra_ui;
extern int *spectra_size;
extern char **spectra_name ;
extern char **family_name ;
extern unsigned short *spectra_type;//0 trace,1 histo
extern int nb_increment;
void  ResetAllHisto();
int   ResetHisto(char* name, char* family = NULL);
void  CreatHistos();
void * SpectraAlive();
void  Launch_SoapServer (int port);
void  Launch_SpectraAlive ();


#endif
