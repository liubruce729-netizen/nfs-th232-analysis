// File : GError.C
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GError
//
// Manager Errors
//
////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

// In this Class
// somme time , fonction "cout" doen't work so we have to use printf
// to solve this probleme.

#include "General.h"
#include "GError.h"
#include  <stdlib.h>
#include  "TString.h"

#if GERRORNOCOLOR
#define COLOR_STD    ""

#define BLACK_STD    ""
#define RED_STD      ""
#define GREEN_STD    ""
#define YELLOW_STD   ""
#define BLUE_STD     ""
#define PURPLE_STD   ""
#define CYAN_STD     ""
#define GREY_STD     ""

#define BLACK_BOLD    ""
#define RED_BOLD      ""
#define GREEN_BOLD    ""
#define YELLOW_BOLD   ""
#define BLUE_BOLD     ""
#define PURPLE_BOLD   ""
#define CYAN_BOLD     ""
#define GREY_BOLD     ""

#else
#define COLOR_STD    "\033[0m"

#define BLACK_STD    "\033[0;30m"
#define RED_STD      "\033[0;31m"
#define GREEN_STD    "\033[0;32m"
#define YELLOW_STD   "\033[0;33m"
#define BLUE_STD     "\033[0;34m"
#define PURPLE_STD   "\033[0;35m"
#define CYAN_STD     "\033[0;36m"
#define GREY_STD     "\033[0;37m"

#define BLACK_BOLD    "\033[1;30m"
#define RED_BOLD      "\033[1;31m"
#define GREEN_BOLD    "\033[1;32m"
#define YELLOW_BOLD   "\033[1;33m"
#define BLUE_BOLD     "\033[1;34m"
#define PURPLE_BOLD   "\033[1;35m"
#define CYAN_BOLD     "\033[1;36m"
#define GREY_BOLD     "\033[1;37m"
#endif



//______________________________________________________________________________


GError::GError() {
	fNbInfo = 0; // number of  info
	fNbError = 0; // number of  errors
	fNbCriticalError = 0; // number of critical errors
	fNbCriticalErrorMax = 20; // number of errors
	fVerbose = 0; // Level of verbose (0-10)
	fNbWarning= 0; // nb of warning
	fNbDebug = 0; // nb of debug
}

//_____________________________________________________________________________

GError::~GError() {
	// destructor of GError object
}
//_____________________________________________________________________________
void GError::Infos(const char *message, const char *message2,
		const char* comment) const {
	// call :TreatError(int level,int status , char *message,char* comment )

	TreatError(0, 0, message, message2, comment);
}
//_____________________________________________________________________________
void GError::Barre() {
	// print bar
	cout<<"-----------------------------------------------------------------------------------\n"<<flush;
}
//_____________________________________________________________________________
void GError::Infos(char *message, TString *message2, TString* comment)const {
	// call :TreatError(int level,int status , char *message,char* comment )
	TreatError(0, 0, message, message2, comment);
}
//_____________________________________________________________________________
void GError::Infos(TString *message, TString *message2, TString* comment)const {
	// call :TreatError(int level,int status , char *message,char* comment )
	TreatError(0, 0, message->Data(), message2->Data(), (char*)comment->Data());
}
//_____________________________________________________________________________
void GError::TreatError(int level, int status, TString *message,
		TString *message2, TString* comment)const {
	// call :TreatError(int level,int status , char *message,char* comment )
	TreatError(level, status, message->Data(), message2->Data(),
			(char*)comment->Data());
}
//_________________________________________________________________________________________
void GError::Test(int coups) const{
	// Little method to make tests.

	TString tempos;
	tempos.Form(" Test on %d secondes", coups);
	cout << tempos.Data() <<endl;
	for (int t = 0; t < coups; t++) {
		cout << ".";
		cout.flush();
		sleep(1);
	}
	cout << "\n";
}
//_____________________________________________________________________________

void GError::TreatError(int level, int status, char *message,
		TString *message2, TString* comment) const{
	// call :TreatError(int level,int status , char *message,char* comment )
	TreatError(level, status, message, message2->Data(), comment->Data());
}

//_____________________________________________________________________________

void GError::SetDebugVerbose(int level) {
	// Set Level of verbose  0<=level<=10
	if (level<0 || level> 10)cout<<"---> Warning , strange debug level :"<< level <<"\n";
	else
	fVerbose = level;
}
//_____________________________________________________________________________
void GError::TreatError(int level, int status, const char *message,
		const char *message2, const char* comment)const {
	// Level = 0 info
	//         1 warnig
	//         2 Error
	//         3 Critical error ( exit if nb error is toot hig)
	//         4 Fatal error ( exit)

	string WorE;
	string tempo;
	WorE="";



	if ((level<0 || level> 5))printf("--> Warning , strange error level : %d\n",level);

	if ( level ==0) {
		WorE  = BLUE_BOLD ;
		WorE += "--> Info   : ";
		WorE += COLOR_STD;// restaure standart color
		fNbInfo ++;
	}

	if ( level ==1) {
		WorE  = YELLOW_STD;// change color in orange
		WorE += "--> Warning: ";
		WorE += COLOR_STD ;
		fNbWarning ++;
	}
	if ( level ==2) {
		WorE =  RED_BOLD;
		WorE += "--> Error  : ";
		WorE += COLOR_STD;// change color in red
		fNbError++;
	}
	if ( level ==3) {
		WorE = RED_BOLD;
		WorE += "--> Critical Error : ";
		WorE += COLOR_STD;// change color in red
		fNbCriticalError++;
	}
	if ( level ==4)
	{
		WorE  =  RED_BOLD;
		WorE += "--> Fatal Error : ";
		WorE += COLOR_STD;
	}
	printf("%s",WorE.data());
	if ((level !=0 )&& ( status!=0)) printf("%x ",status);

	printf("%s", message);
	if (message2!=NULL) printf("  %s", message2);
	printf( "\n");
	if (comment!=NULL) {
		if (fVerbose) {printf("%s\n", comment);
		}
	}

	if ( fNbCriticalError> fNbCriticalErrorMax) {
		printf(" Maximun of critical error reached,  exit\n");
		fflush (stdout);
		exit(0);
	}
	fflush (stdout);
	if ( level ==4 ) exit(0);

}


//_____________________________________________________________________________
void GError::TreatDebug(int level, int status, TString *message,
		TString *message2, TString* comment) const{
	// call :TreatDebug(int level,int status , char *message,char* comment )
	TreatDebug(level, status, message->Data(), message2->Data(),
			(char*)comment->Data());
}

//_____________________________________________________________________________

void GError::TreatDebug(int level, int status, char *message,
		TString *message2, TString* comment)const {
	// call :TreatDebug(int level,int status , char *message,char* comment )
	TreatDebug(level, status, message, message2->Data(), comment->Data());
}
//_____________________________________________________________________________
void GError::TreatDebug(int level, int status, const char *message,
		const char *message2, const char* comment)const {
	//         1 ->10

	TString WorE;
	WorE = "--> Debug : ";
	TString tempo;
	tempo.Form("--> Warning, Strange debug level :%d \n", level);
	if ( (level<0 || level> 10))printf ("%s",tempo.Data());

	if ( level >9 && level <21)
	{
		WorE = "--> Debug : ";

	}

	if ( (fVerbose >= level)) {
		tempo.Form("%s%s\n",WorE.Data(),message);
		printf("%s",tempo.Data());
		if (message2!=NULL) {
			tempo.Form("--->          %s\n",message2);
			printf("%s",tempo.Data());
		}
		if (fVerbose)
		{
			if(comment!=NULL) {
				tempo.Form("%s\n",comment);
				printf ("%s",tempo.Data());
			}
		}
	}
	fflush (stdout);
}
//_____________________________________________________________________________
