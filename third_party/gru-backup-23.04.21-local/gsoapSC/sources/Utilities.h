#ifndef __MySuperUtilities__
#define __MySuperUtilities__

#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <string.h>


# define MAX_CARACTERES 256

// Utilities

int   Slicer(char* inputwords, char*** commandsliced, int** commandsliced_int);
void  FreeChar(char** commandsliced, int* commandsliced_int, int nb);
char* RemoveChar(char * input, char* toremove, bool frombegin);
char* RemoveWhite(char * input);
bool  ReplaceChar(char* chaine, char char1, char char2);
int   WordsCount(char * mystring);
void  Test(int coups);
void  DumpRaw(void *point, int dumpsize, int increment);

#endif
