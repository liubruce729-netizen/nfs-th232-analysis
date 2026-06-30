
#ifndef __CUtilities__
#define __CUtilities__

#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


//
// Few util classes for vectors management
//

//___________________________________________________________________________________
// UtilVector_c Class to manage a vector of char
//___________________________________________________________________________________
class UtilVector_c
{
  private:
  char * fPt;
  char * fPtTempo;
  int    fSize;
  int    fUsedSize;
  bool   IsSpaceInstanciedInThisClass;
  
  char ** fPt_ext;    // used in case of use of external buffer
  int  *  fSize_ext;
  
  public:

//___________________________________________________________________________________
  UtilVector_c(){
    fPt       = NULL;
    fPtTempo  = NULL;
    fSize     = 0;    
    fUsedSize = 0;
    fPt_ext=NULL;    
    fSize_ext=NULL;
    IsSpaceInstanciedInThisClass= false;
  }
//_______________________________________________________________________________
  UtilVector_c(int size){
    fUsedSize = 0;
    fSize     = 0;
    fPtTempo  = NULL;
    fPt       = NULL;
    fPt_ext   = NULL;    
    fSize_ext = NULL;
    ReSize(size);
    IsSpaceInstanciedInThisClass= true;
    Reset();
  }  
//_______________________________________________________________________________  
  UtilVector_c(char* buff, int size){
   SetExternalPointers(buff, size);
  }
//_______________________________________________________________________________  

UtilVector_c(char** buff, int* size){
   SetExternalPointers( buff, size);
  }
//_______________________________________________________________________________

~UtilVector_c() {
   if (IsSpaceInstanciedInThisClass){
    if (fPt ){
      //printf( " DEBUG : Delete UtilVector_c;:~UtilVector_c : %lld \n",(long long*) fPt);
      free(fPt);
      fPt = NULL;
      fSize = 0;
      }
   } 
  }
 //_______________________________________________________________________________  

 void SetExternalPointers(char** buff, int *size){
    fUsedSize = *size;
    fSize     = *size;
    fPt_ext   =  buff ;
    fSize_ext =  size ;
    fPtTempo  = NULL;
    if(fPt and IsSpaceInstanciedInThisClass)  free(fPt);
    fPt       = *buff;
    IsSpaceInstanciedInThisClass= false;
 }
 
 
//_______________________________________________________________________________  

void SetExternalPointers(char* buff, int size){
    fUsedSize = size;
    fSize     = size;
    fPtTempo  = NULL;
    fPt_ext   = &buff;
    fSize_ext = &size;
    
    if(fPt and IsSpaceInstanciedInThisClass) free(fPt);
    fPt       = buff;
    IsSpaceInstanciedInThisClass = false;
 }
 //_______________________________________________________________________________  

void SetExternalPointers(UtilVector_c* vect){
    fUsedSize = vect->GetUsedSize();
    fSize     = vect->GetSize();;
    fPtTempo  = NULL;
    if(fPt and IsSpaceInstanciedInThisClass) free(fPt);
    fPt       = vect->GetPointer();
    IsSpaceInstanciedInThisClass= false;
 }
//_______________________________________________________________________________

void ReSize(int size) {
    int i=0;
   // int old= fSize;
    if (fPt == NULL){
	fPtTempo = (char*) (malloc(size)); 
	for (int i = 0; i < size; i++) ((char*) fPtTempo)[i] = 0;
    }else{
	fPtTempo = (char*) (realloc((void*) fPt, size));
	//cout << " Debug   UtilVector::Resize from "<< fSize<<" > "<< size<<endl;
	if (size > 0)
	if (size > fSize)
		for ( i = fSize; i < size; i++)  fPtTempo[i] = 0;
	}
    //printf( " DEBUG : UtilVector_c:ReSize(%d), oldsize = %d, pt = %lld old pt = %lld \n",size,old,(long long *)fPtTempo,(long long *)fPt);
    fPt   = fPtTempo;
    if (fPt_ext !=NULL) *fPt_ext =  fPtTempo;
    fSize = size; 
    if (fSize_ext !=NULL) *fSize_ext = size;
  }
//_______________________________________________________________________________

void Reset(){
     for (int i = 0; i < fSize; i++)((char*) fPt)[i] = 0; 
  }
//_______________________________________________________________________________

char *GetPointer()const{
    return fPt;
  }
//_______________________________________________________________________________

unsigned char GetValue(int i){
    return ((char)((char* )fPt)[i]);
  }
//_______________________________________________________________________________

void SetValue(int i, char c){
   ((char* )fPt)[i] = c;
  }

//_______________________________________________________________________________

int GetSize() const{
    return fSize;
  }
//_______________________________________________________________________________
  int GetUsedSize()const{
     return fUsedSize;
  }
//_______________________________________________________________________________

void SetUsedSize(int usedsize){
    fUsedSize = usedsize;
  }
//_______________________________________________________________________________

void ResizeIfNecessary(int size) {
  if (size > fSize) ReSize(size);
  }
//_______________________________________________________________________________

void Reset(int nb){
  // reset only the nb first bytes
     for (int i = 0; i < nb; i++)((char*) fPt)[i] = 0; 
}
//_______________________________________________________________________________

void CopyAndResizeIfNecessary(char* givenPt,int size) {
// Copy data from a given point and resize our buffer if is not  big enougth
    ResizeIfNecessary(size);
    memcpy (fPt,givenPt,size);
}
 //_______________________________________________________________________________
void DumpRaw( int dumpsize, int increment=0,
		std::string * mydump =NULL) const{
   void *point = fPt;		
   DumpRaw(point,dumpsize,increment, mydump) ;		
}
//_______________________________________________________________________________
void DumpRaw(void *point, int dumpsize, int increment=0,
		std::string * mydump =NULL) const{

	///  Creat a string of dump of memory space\n
	///  point : pointer bo dump begin\n
	///  if dumpsize =0 , dumpsize = standard = 256 \n

	std::string *mydumploc;

	if (mydump == NULL) {
		std::string st;
		mydumploc = &st;
	} else {
		mydumploc = mydump;
	}

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
		sprintf(tempo, "\n%5d %s ", increment, ": ");
		*mydumploc += tempo;
		for (k = 0; k < nbrcol; k++) {
			sprintf(tempo, "%02hX ", (unsigned short) ((*(pChar2++))));
			*mydumploc += tempo;
			dumpsize3--;
			if (dumpsize3 == 0)
				break;
		}
		*mydumploc += "  ";

		for (k = 0; k < nbrcol; k++) {
			chartmp = pChar[0];
			//memcpy (&chartmp, pChar ,1);
			if ((chartmp >= asciimin) && (chartmp <= asciimax)) {
				sprintf(tempo, "%c", *pChar);
				*mydumploc += tempo;
			} else
				*mydumploc += ".";
			pChar++;
			dumpsize2--;
			if (dumpsize2 == 0)
				break;
		}
		increment += nbrperline;
	}
	*mydumploc += "\n";

	if (mydump == NULL)
		std::cout << (*mydumploc).data();
}
//_______________________________________________________________________________
 
  
};// end of class
//_______________________________________________________________________________
#endif

