// Utilities

#include "Utilities.h"

//_________________________________________________________________________________________
int Slicer(char* inputwords, char*** commandsliced, int** commandsliced_int) {
	// slice inputwords in commandsliced char and  commandsliced_int int vectors
	// return number of words

	char commandstring[strlen(inputwords)+1];
	strcpy(commandstring, inputwords);

	int i;
	int nb_char, nb_words;
	char** local_commandsliced;
	int* local_commandsliced_int;
	char* wordtempo;
	nb_char = strlen(inputwords) + 1;
	char inputwords2[nb_char];
	strcpy(inputwords2, inputwords);

	nb_words = WordsCount(inputwords);
	local_commandsliced_int = new int[nb_words];
	local_commandsliced = new char*[nb_words];

	char* tempo = inputwords2;
	for (i = 0; i < nb_words; i++) {
		wordtempo = strtok(tempo, " ");
		nb_char = strlen(wordtempo) + 1;
		local_commandsliced[i] = new char[nb_char];
		strcpy(local_commandsliced[i], wordtempo);
		local_commandsliced_int[i] = atoi(local_commandsliced[i]);
		tempo = NULL;
	}
	*commandsliced = local_commandsliced;
	*commandsliced_int = local_commandsliced_int;
	return nb_words;
}
//_________________________________________________________________________________
void FreeChar(char** commandsliced, int* commandsliced_int, int nb) {
	int i;

	if (commandsliced_int != NULL){
		delete[] commandsliced_int;
		commandsliced_int=NULL;
	}

	for (i = 0; i < nb; i++) {
		if (commandsliced[i] != NULL){
			delete[] (commandsliced[i]);
			commandsliced[i]= NULL;
		}
	}
	if (commandsliced != NULL){
		delete[] commandsliced;
		commandsliced=NULL;
	}
}
//_________________________________________________________________________________

bool ReplaceChar(char* chaine, char char1, char char2) {
	// replace in TString chaine char1 in char char2.

	bool retour = false;
	int i;
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

char* RemoveWhite(char * input) {
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
char* RemoveChar(char * input, char* toremove, bool frombegin) {
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
bool CompareWordsIgnoreCase(char* word1, char* word2) {
	// compare 2 words with ignoring case.

	return (strcasecmp(word1, word2) == 0);

}
//_________________________________________________________________________________________

int WordsCount(char * mystring) {
	// count nub of word in a string mystring
	char tempomystring[256];
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

//__________________________________________________________________________________________
void Test(int coups) {
	// execute a test
	// if coups >0 a print of "." is done each second

	for (int t = 0; t < coups; t++) {
		printf(".");
		fflush(stdout);
		sleep(1);
	}
	printf("\n");
}

//_________________________________________________________________________________________
void DumpRaw(void *point, int dumpsize, int increment) {

	///  print a dump of vector "point "
	///  if dumpsize =0 , dumpsize = standard = 256  raz\n


	int i, k;
	int nbrcol = 16; // nb de colonnes affich�es

	int asciimin = 32; // range min of a char to be ascii character
	char tempo[128] = "";
	char chartmp = 0;

	int asciimax = 127; // idem but max
	int nbrperline = 16; // nbr of bytes per line
	int nbrline = 0; // nbr lines
	int dumpsize2 = dumpsize;
	int dumpsize3 = dumpsize;
	unsigned char *pChar = NULL;
	unsigned char *pChar2 = NULL;

	nbrline = (int) (dumpsize / nbrperline) + (int) ((dumpsize % nbrperline)
			!= 0);

	pChar = (unsigned char *) ((char*) point + increment);
	pChar2 = pChar;

	if (nbrline < 1)
		nbrline = 1;
	if (nbrcol > dumpsize)
		nbrcol = dumpsize;
	if (increment == dumpsize)
		nbrline = 1;
	for (i = 0; i < nbrline; i++) {
		printf(tempo, "\n%5d %s ", increment, ": ");

		for (k = 0; k < nbrcol; k++) {
			printf(tempo, "%02hx ", (*(pChar2++)));

			dumpsize3--;
			if (dumpsize3 == 0)
				break;
		}
		printf("  ");

		for (k = 0; k < nbrcol; k++) {
			chartmp = pChar[0];
			//memcpy (&chartmp, pChar ,1);
			if ((chartmp >= asciimin) && (chartmp <= asciimax)) {
				printf(tempo, "%c", *pChar);

			} else
				printf(".");
			pChar++;
			dumpsize2--;
			if (dumpsize2 == 0)
				break;
		}
		increment += nbrperline;
	}
	printf("\n");

}

//_________________________________________________________________________________________
