// Author: $Author: legeard $
// Part of GRU
//
// GBase : base Class  of all GRU class
// 		contains utilities, common functions, etc
//////////////////////////////////////////////////////////////////////////

// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************


#include "GBase.h"
#include "GInterrupt.h"
ClassImp( GBase);
//_________________________________________________________________________________________
GBase::GBase() {
	fVerbose = 0;
	//GInterrupt *IntHandle =NULL;
	//IntHandle=new GInterrupt(this);
	//IntHandle->Add();// catch of Control keyboard.

}
//_________________________________________________________________________________________
GBase::~GBase() {

}
//_________________________________________________________________________________________
bool GBase::CompareWordsIgnoreCase(char* word1, const char* word2) {
	// compare 2 words with ignoring case.
	return (strcasecmp(word1, word2) == 0);
}
//_________________________________________________________________________________________
bool GBase::CompareWordsIgnoreCase(TString* word1, const char* word2) {
	// compare 2 words with ignoring case.
	return ((word1->CompareTo(word2, TString::kIgnoreCase)) == 0);
}

//_________________________________________________________________________________________
bool GBase::TestGruWord(char* word) {
	// compare if word is "gru" with ignoring case.

	if ((CompareWordsIgnoreCase(word, (const char*) "CALIMERO")) or (CompareWordsIgnoreCase(word, (const char*) "GRU")))
	//if ((CompareWordsIgnoreCase(word, (char*) "GRU")) or (CompareWordsIgnoreCase(word, (char*) "CALIMERO")))
		return true;
	else
		return false;

}
//_________________________________________________________________________________________
int GBase::Slicer(char* inputwords, char*** commandsliced,
		int** commandsliced_int) {
	// slice inputwords in  commandsliced  char and  commandsliced_int int vectors
	// return number of words

	int i;
	int nb_char, nb_words = 0;
	char** local_commandsliced;
	int* local_commandsliced_int;
	char* wordtempo;
	nb_char = (strlen(inputwords)) + 1;
	char inputwords2[nb_char];

	strcpy(inputwords2, inputwords);
	nb_words = WordsCount(inputwords);
	local_commandsliced_int = NULL;
	local_commandsliced = NULL;
	if (nb_words == 0)
		return nb_words;

	local_commandsliced_int = new int[nb_words];
	local_commandsliced = new char*[nb_words];

	char* tempo = inputwords2;
	for (i = 0; i < nb_words; i++) {
		wordtempo = strtok(tempo, " ");
		nb_char = (strlen(wordtempo)) + 1;
		local_commandsliced[i] = new char[nb_char];
		strcpy(local_commandsliced[i], wordtempo);
		local_commandsliced_int[i] = atoi(local_commandsliced[i]);
		tempo = NULL;
	}
	*commandsliced = local_commandsliced;
	*commandsliced_int = local_commandsliced_int;
	return nb_words;
}
//_______________________________________________________________________________
void GBase::RazZone(char* pt,int size,int start_size){
	if (start_size>size) return;
	for (int i = start_size; i <size; i++) {
		pt[i] = 0;
	}
}

//_______________________________________________________________________________
void GBase::ReallocBufferSize(char**pt,int newsize,int oldsize, bool ifinferior) {
	/// Do memory allocation or a reallacation for frame\n
	/// if ifinferior==true the allocaton is forced to size only if the actual size is bigger
	/// ifinferior = true by default (the allocaton is forced to size even if the actual size is bigger)
	char* pt_new=NULL;
	char* ppt=NULL;
	ppt = *pt;
	if (!ifinferior or (newsize > oldsize)) {
		//pt_new = (char*) (realloc((void*) ppt, newsize));
		if (ppt){
			delete[] ppt;
			ppt =NULL;
		}
		pt_new = new char[newsize];
		RazZone(pt_new,newsize,oldsize);
		//set 0 in rest of buffer

		//RazZone(pt_new,newsize,oldsize);
		ppt = pt_new;
		cout << "Resized from "<<oldsize<<" to "<<newsize<<"\n";
	}
	*pt = pt_new;
	cout << "Resize\n";
}
//_________________________________________________________________________________
bool GBase::ReplaceChar(TString* chaine, char char1, char char2) {
	// replace in TString chaine char1 in char char2.

	char tempo[MAX_CARACTERES];
	Int_t i;
	char c;
	bool retour = false;

	strcpy(tempo, chaine->Data());
	i = 0;
	c = ' ';
	while (c != '\0') {
		c = tempo[i];
		if (c == char1) {
			retour = true;
			tempo[i] = char2;// replace char1  by 'char2'
		}
		i++;
	} // end of While (c!=...
	*chaine = tempo;
	return retour;
}
//_________________________________________________________________________________

bool GBase::ReplaceChar(char* chaine, char char1, char char2) {
	// replace in TString chaine char1 in char char2.

	bool retour = false;
	Int_t i;
	char c;

	i = 0;
	c = ' ';
	while (c != '\0') {
		c = chaine[i];
		if (c == char1) {
			retour = true;
			chaine[i] = char2;// replace char1  by 'char2'
		}
		i++;
	} // end of While (c!=...
	return retour;
}
//_________________________________________________________________________________________

char* GBase::RemoveWhite(char * input) {
	//return a char* removed of white space
	// ex "   Hello Word    "  -> "Hello Word"

	int msize, i, j, k;
	i = 0;
	j = 0;
	k = 0;
	msize = strlen(input);

	j = msize - 1;
	while (input[i] == ' ') {
		i++;
	}
	while (input[j] == ' ') {
		j--;
	}
	char *newstring = new char[j - i + 2];

	for (k = 0; k <= (j - i); k++)
		newstring[k] = input[k + i];
	newstring[j - i + 1] = '\0';

	return newstring;
}

//_________________________________________________________________________________________
char* GBase::RemoveChar(char * input, char* toremove, bool frombegin) {
	//return a char* removed with the part "toremove"
	//if frombegin = true ( default) then the scan begin from begin of word "input"
	// ex RemoveChar("worldIamthebestotheworld","world",0) -> "worldIamthebestothe"
	//    RemoveChar("worldIamthebestotheworld","world") -> "Iamthebestotheworld"
	//    RemoveChar("aworldIamthebestotheworld","world") -> "aworldIamthebestotheworld"

	int isize, csize, newsize;

	isize = strlen(input);
	csize = strlen(toremove);
	newsize = isize - csize;
	char* newstring = NULL;
	char* tmp;
	if (newsize > 0) {
		if (frombegin) {
			tmp = input;
		} else {
			tmp = (char*) (&(input[newsize]));
		}
		if (strncmp(tmp, toremove, csize) == 0) {
			newstring = new char[newsize + 1];
			if (frombegin) {
				strncpy(newstring, (input + csize), newsize);
			} else {
				strncpy(newstring, (input), newsize);
			}
			newstring[newsize] = '\0';
		} else {
			newstring = new char[isize + 1];
			strcpy(newstring, input);
			newstring[isize] = '\0';
		}
	} else {
		newstring = new char[isize + 1];
		strcpy(newstring, input);
		newstring[isize] = '\0';
	}
	return newstring;
}

//_________________________________________________________________________________________
int GBase::WordsCount(char * mystring) {
	int size = strlen(mystring);
	char tempomystring[size+1];
	strcpy(tempomystring, mystring);
	int nb = 0;
	char * pch;

	pch = strtok(tempomystring, " ");
	while (pch != NULL) {
		pch = strtok(NULL, " ");
		nb++;
	}
	return nb;
}
//_________________________________________________________________________________________
void GBase::Test(int coups) {
	// Little method to make tests.

	TString tempos;
	tempos.Form(" Test on %d secondes", coups);
	fError.Infos(tempos);
	for (int t = 0; t < coups; t++) {
		cout << ".";
		cout.flush();
		sleep(1);
	}
	cout << "\n";
}
//____________________________________________________________________
void GBase::SwapInt32(UNSINT32 *Buf, int NbOctets) {
	typedef struct mot32 {
		unsigned char Byte1;
		unsigned char Byte2;
		unsigned char Byte3;
		unsigned char Byte4;
	} SW_MOT32;

	SW_MOT32 Temp, *Mot32;
	int i, NbMots;

	NbMots = NbOctets / 4;
	Mot32 = (SW_MOT32 *) Buf;

	for (i = 0; i < NbMots; i++, Mot32++) {
		Temp.Byte1 = Mot32->Byte4;
		Temp.Byte2 = Mot32->Byte3;
		Temp.Byte3 = Mot32->Byte2;
		Temp.Byte4 = Mot32->Byte1;
		*Mot32 = Temp;
	}
}
//____________________________________________________________________

void GBase::SwapInt64(long long *Buf,int nbByte) {
	unsigned char *tmp1, *tmp2;
	unsigned long long tempo ;
	tempo = 0;
	tmp1 = (unsigned char*)Buf;
	tmp2 = (unsigned char*) (&tempo);

	if (nbByte>8 ) fError.TreatError(2,0,"SwapInt64 with nb of Bytes > 8");
	for (int i = 0; i < nbByte; i++) {
		tempo =((unsigned long long)(tempo) << 8);
		memcpy(tmp2,tmp1+i,1);
	}
	*Buf = tempo;
}

//____________________________________________________________________

void GBase::SwapInt16(UNSINT16 *Buf, int NbOctets) {
	typedef struct mot16 {
		unsigned char Byte1;
		unsigned char Byte2;
	} SW_MOT16;

	SW_MOT16 Temp, *Mot16;
	int i, NbMots;

	NbMots = NbOctets / 2;
	Mot16 = (SW_MOT16 *) Buf;

	for (i = 0; i < NbMots; i++, Mot16++) {
		Temp.Byte1 = Mot16->Byte2;
		Temp.Byte2 = Mot16->Byte1;
		*Mot16 = Temp;
	}
}
//____________________________________________________________________
void GBase::GetDumpRaw2  (void *point, int dumpsize, int sizeframe, int increment,
		string * mydump) const{

if (dumpsize>sizeframe) dumpsize =sizeframe;
GetDumpRaw(point,  dumpsize,  increment, mydump) ;
}
//____________________________________________________________________
void GBase::GetDumpRaw(void *point, int dumpsize, int increment,
		string * mydump) const {
	// Method to dump buffer on  output
	// if dumpsize =0 , dumpsize = standard = 256 and fGBbuf_increment raz
	// if mydump == NULL result is printed in standard output
	// if mydump is defined output is put in sting mydump
    // the output is arranged in short not what is in memory of computer cause of swap  for short representation
	string *mydumploc;
	string st;
	if (mydump == NULL) {
		mydumploc = &st;
	} else {
		mydumploc = mydump;
	}
	int i, k;
	int nbrcol = 16; // nb de colonnes affich�es
    int nbofprint =0;    int nbofpoint =0;
	int asciimin = 32; // range min of a char to be ascii character
	char tempo[20] = "";
	int asciimax = 127; // idem but max
	int nbrperline = 16; // nbr of bytes per line
	int nbrline = 1; // nbr lines
	unsigned char *pCharc;
	unsigned char *pChar;

	nbrline = (int) (dumpsize / nbrperline) + (int) ((dumpsize % nbrperline)
			!= 0);
	pChar  = (unsigned char *)  ((char*) point + increment);
	pCharc =pChar;
	if (nbrline < 1)
		nbrline = 1;

	if (increment == dumpsize)
					nbrline = 1;
	for (i = 0; i < nbrline; i++) {
		sprintf(tempo, "\n%5d %s ", increment, ": ");
		*mydumploc += tempo;

		for (k = 0; k < nbrcol; k++) {
			if (nbofprint++<dumpsize ){
			sprintf(tempo,"%02hX ",(unsigned short)(*(pCharc++)));
			*mydumploc += tempo;
			}
		}
		*mydumploc += "  ";
		for (k = 0; k < nbrperline; k++) {
			if (nbofpoint++<dumpsize ){
			if ((*pChar >= asciimin) && (*pChar < asciimax)) {
				sprintf(tempo, "%c", *pChar);
				*mydumploc += tempo;
			} else
				*mydumploc += ".";
			pChar++;
		}
		}
		increment += nbrperline;
	}
	*mydumploc += "\n";

	if (mydump == NULL)
		cout << (*mydumploc).data();
}
//____________________________________________________________________
void GBase::GetDumpRawReal  (void *point, int dumpsize, int increment,
		string * mydump) const{
	// Method to dump buffer on  output
	// if dumpsize =0 , dumpsize = standard = 256 and fGBbuf_increment raz
	// if mydump == NULL result is printed in standard output
	// if mydump is defined output is put in sting mydump
    // dump exacly what is in computer memory
	string *mydumploc;
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
		*mydumploc += tempo;

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

//____________________________________________________________________
void GBase::WaitWithPoint(int nsec)const {
for (int j = 0; j < nsec; j++) {cout << "." << flush;sleep(1);}
					cout << "\n" << flush;

}
//______________________________________________________________________________
void GBase::WaitAChar() const {
	//fonction to wait a action on keyboard
	bool conti = true;
	char c;
	while (conti) {
		printf("Enter 'q' to quit  - 'c' to continue\n");
		c = getchar();
		if (c == 'q') exit(0);
		if (c == 'c') conti = false;
	}

}
//____________________________________________________________________
