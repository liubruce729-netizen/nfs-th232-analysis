
//////////////////////////////////////////////////////////////////////////
// Class to interpret aguments of main  in a C file  (main (argc, argv))
// see example/tutorial at the end of this file
// all arguments can be like : 
// EXECUTABLE -arg1 --argment2 -arg3=int -arg5 = float  --arg6=text --arg7 = text ...
//
// -----------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <string.h>
#ifndef ArgInterpretor_H
#define ArgInterpretor_H
using namespace std;
class ArgInterpretor{
  private:
  char toto[288];
  string fAllcommands ;
  string fCommand;  // current command ( ex : "-v=10")
  string fCommand1; // part of current command before "=" ( ex : "-v=")
  string fCommand2; // part of current command after "=" if it existe (ex : "10")
  int fSumCommand; // sum of treated informations
  int fNbCommand ; //  number of subcommand in current command
  public: 
  ArgInterpretor(int argc, char ** argv);
  ~ArgInterpretor(void);
  int    ReplaceString(string* text, string * before , string * after);
  string GetCommand1()const;
  string GetCommand2()const;
  int    GetNextCommand();
  string TreatmentOfArguments(int argc, char **argv);

  void ErrorMessageAndExit()const ;
  bool  Treat2Commands(char * text1, char*  text2,char * texterror,int * inttoreturn);
  bool  Treat2Commands(string text1, string text2,string texterror,int * inttoreturn);
  bool  Treat2Commands(char * text1, char*  text2,char * texterror,float * floattoreturn);
  bool  Treat2Commands(string text1, string text2,string texterror,float * floattoreturn);
  bool  Treat2Commands(char * text1, char*  text2,char * texterror,char* texttoreturn);
  bool  Treat2Commands(string text1, string text2,string texterror,string * texttoreturn);
  bool  Treat1Command(char *  text1, char*  text2, bool * para =NULL );
  bool  Treat1Command(string  text1, string text2, bool * para =NULL );
  bool TestNoCommand()const;
  void TestNoCommandSoExit()const;
  int  GetSumCommand()const;
  };
  
//_________________________________________________________________________________________
ArgInterpretor::ArgInterpretor(int argc, char ** argv){
  // argc and arv is repport of main arguments (main (int argc,char** argv))

  fAllcommands = TreatmentOfArguments (argc, argv);
  fNbCommand   = 0 ;
  fSumCommand  = 0;
}
//_________________________________________________________________________________________
ArgInterpretor::~ArgInterpretor(void){
}
//_________________________________________________________________________________________
int  ArgInterpretor::ReplaceString(string* text, string * before , string * after){
size_t index = 0;
int sizeofbefore = before->length();
int nbofreplacement;
while (true) {
     /* Locate the substring to replace. */
     index = text->find((*before), index);
     if (index == std::string::npos) break; // npos= max value for size of string

     /* Make the replacement. */
     text->replace(index, sizeofbefore, *after);

     /* Advance index forward so the next iteration doesn't pick it up as well. */
     index += sizeofbefore;
   }
return nbofreplacement;
}

//_________________________________________________________________________________________
string ArgInterpretor::GetCommand1()const{
// return the =first part of current command
return fCommand1;
}

//_________________________________________________________________________________________
string ArgInterpretor::GetCommand2()const{
// return the =second part of current command(if it existes)
return fCommand2;
}
//_________________________________________________________________________________________
int ArgInterpretor::GetSumCommand()const{
// return sum of all commands
return fSumCommand;
}
//_________________________________________________________________________________________
string ArgInterpretor::TreatmentOfArguments(int argc, char **argv){
 // remove all space before or after '='
 // and place in new tab the result
 // argv[0] , name of process is not treated
 // return full string

 string inputchar;
 string before =" =";
 string after = "=";
 int i=0;
for (i=1; i<(argc) ;i++){
  inputchar.append ( ((argv)[i]));
  inputchar.append ( " ");
  }

  ReplaceString(&inputchar, &before , &after);
  before ="= ";
  ReplaceString(&inputchar, &before , &after);

return inputchar;
}
//_________________________________________________________________________________________
int ArgInterpretor::GetNextCommand(){
	// extract first  command1 and command 2 from fAllcommands 
	// the command are separed by space char " "
	// string command1 // part of fAllcommands before "=" ( ex : "-v=")
	// string command2 // par of fAllcommands after "=" if it existe (ex : "10")
	// fAllcommands is retuned shorter 
	// return is 0 if no command found ,1 if only command1 is found, 2 if command1 and command2 existe

	string command="";
	fCommand1 ="";
	fCommand2 ="";
	fNbCommand =0;
	int placeofchar= fAllcommands.find(" ");
	if (placeofchar!=std::string::npos){
		command = fAllcommands.substr(0,placeofchar);
		fCommand1= command;
		fAllcommands = fAllcommands.substr(placeofchar+1);
		placeofchar = command.find("=");
		fNbCommand++;
		if (placeofchar!=std::string::npos){
			fCommand1= command.substr(0,placeofchar);
			if (placeofchar!=std::string::npos){
				fCommand2=command.substr(placeofchar+1);
				fNbCommand++;
				}
			}
	}
	//cout <<"nb command ="<<fNbCommand<<"  command1 = "<< fCommand1<< " command2 = "<< fCommand2<<"\n";
        return fNbCommand;
}
//_______________________________________________________________________________________________________________________
void ArgInterpretor::ErrorMessageAndExit() const{
	cout << " Command not good or unknown option, see help with -h option\n";
	exit(0);
}
//_________________________________________________________________________________________
bool ArgInterpretor::Treat2Commands(char * text1, char* text2,char * texterror,int * para ){
// text1 and text2 are the 2 part of input arguments ( which was separted by a "=" , before)
// texterror is the information to give in case of empty text2 
// para is the return in integer or text2
 if((fCommand1==text1) || (fCommand1==text2 )) {
	if(fNbCommand<2) {
        	cout << "["<<fCommand1<<"] must be followed by "<<texterror<<"\n";
                ErrorMessageAndExit();
        }
        *para = atoi(fCommand2.c_str()); 
        fSumCommand++;
        return true;
 }
 return false;  
}
//_________________________________________________________________________________________
bool ArgInterpretor::Treat2Commands(string text1, string text2, string texterror,int * para ){
// text1 and text2 are the 2 part of input arguments ( which was separted by a "=" , before)
// texterror is the information to give in case of empty text2 
// para is the return in integer or text2
 if((fCommand1==text1) || (fCommand1==text2 )) {
	if(fNbCommand<2) {
        	cout << "["<<fCommand1<<"] must be followed by "<<texterror<<"\n";
                ErrorMessageAndExit();
        }
        *para = atoi(fCommand2.c_str()); 
        fSumCommand++;
        return true;
 }
 return false;  
}
//_________________________________________________________________________________________
bool ArgInterpretor::Treat2Commands(char * text1, char* text2,char * texterror,float * para ){
 if((fCommand1==text1) || (fCommand1==text2)) {
	if(fNbCommand<2) {
        	cout << "["<<fCommand1<<"] must be followed by "<<texterror<<"\n";
                ErrorMessageAndExit();
        }
        *para = atof(fCommand2.c_str()); 
         fSumCommand++;
        return true;
 }
 return false;  
}
//_________________________________________________________________________________________
bool ArgInterpretor::Treat2Commands(string text1, string text2, string texterror,float * para ){
 if((fCommand1==text1) || (fCommand1==text2 )) {
	if(fNbCommand<2) {
        	cout << "["<<fCommand1<<"] must be followed by "<<texterror<<"\n";
                ErrorMessageAndExit();
        }
        *para = atof(fCommand2.c_str()); 
        fSumCommand++;
        return true;
 }
 return false;  
}
//_________________________________________________________________________________________
bool ArgInterpretor::Treat2Commands(char * text1, char* text2,char * texterror,char* texttoreturn){
 if((fCommand1==text1) || (fCommand1==text2 )) {
	if(fNbCommand<2) {
                  cout << "["<<fCommand1<<"] must be followed by" <<texterror<<"\n";
                  ErrorMessageAndExit();
               }
	strcpy (texttoreturn ,fCommand2.c_str()); 
	fSumCommand++;
	return true;
 }
 return false;
}

//_________________________________________________________________________________________
bool ArgInterpretor::Treat2Commands(string text1, string text2,string texterror,string * texttoreturn){
 if((fCommand1==text1) || (fCommand1==text2 )) {
	if(fNbCommand<2) {
                  cout << "["<<fCommand1<<"] must be followed by" <<texterror<<"\n";
                  ErrorMessageAndExit();
               }
	*texttoreturn = fCommand2; 
	fSumCommand++;
	return true;
 }
 return false;
}


//_________________________________________________________________________________________
bool ArgInterpretor::Treat1Command(char * text1, char* text2,bool * para ){
 if((fCommand1==text1) ||(fCommand1==text2 )) {
	if(fNbCommand>1) {
        	cout << "["<<fCommand1<<"] must not be followed by \"= something\" \n";
                ErrorMessageAndExit();
        }
        if (para) {*para = true;}
        fSumCommand++;
        return true;
 }
 return false;  
}
//_________________________________________________________________________________________
bool ArgInterpretor::Treat1Command(string text1, string text2,bool * para ){
 if((fCommand1==text1) ||(fCommand1==text2 )) {
	if(fNbCommand>1) {
        	cout << "["<<fCommand1<<"] must not be followed by \"= something\" \n";
                ErrorMessageAndExit();
        }
        if (para) {*para = true;}
        fSumCommand++;
        return true;
 }
 return false;  
}
//_________________________________________________________________________________________
void ArgInterpretor::TestNoCommandSoExit()const{
	if (TestNoCommand()) ErrorMessageAndExit();
	return;
}
//_________________________________________________________________________________________
bool ArgInterpretor::TestNoCommand()const{
	if (fSumCommand==0) return true;
	else return false;
}
//_________________________________________________________________________________________

#endif
//
// this part is just to give example and tutorial for usage of  ArgInterpretor class
// it is a main using ArgInterpretor 
// To do the tutorial , copy the text above  in a "classtest.cc" file
// compile it with this command 
// g++ -o classtest classtest.cc
// and test it with the generated "classtest" executable
/*

#include "ArgInterpretor.h"
//_________________________________________________________________________________________
void Help() {
	cout<<
	"classtest : this program is just to test class : ArgInterpretor\n";
	cout << " Usage  : \n";
	cout << "            [-h] or [--help]  print this help \n";
	cout << "            [-ver or --version] , print version \n";
	cout << "            [-t=mytext] or [--text=mytext] a text to display \n";
	cout << "            [-n=mynumber] or [--number=mynumber] set number = mynumber\n";
	cout << "            [-v=level] or [--verbose=level] set level of verbose\n";
	cout << "            [-r] or [--read] set read mod to true\n";
	cout << "            [-fp=float] or [--floatparameter]  set floatparameter = float\n";
}
//_________________________________________________________________________________________
bool HelpAndExit(){
       Help();
       exit (0);
}
//_________________________________________________________________________________________
void Announce(char* progname,int version){
	cout << "-------------------------------------------------" << endl;
	cout << "        "<<progname << endl;
	cout << "         Version : "<<version << endl;
	cout << "-------------------------------------------------" << endl;
}
//_________________________________________________________________________________________
int main(int argc, char **argv) {

int number=0;
int verbose =0;
string mytext ;
int version = 2;
bool readmode=false;
string stringreadmode ="false";
float floatpara=0.0;

mytext = "Default text";

// part of treatement of input arguments
ArgInterpretor ArgInt(argc,argv);
  while(true)
         {  
            if (ArgInt.GetNextCommand () ==0){
            	if (ArgInt.TestNoCommand()) {HelpAndExit();} // comment this line if you want the executable can run without argument
            	break;
             }
            ArgInt.Treat2Commands((string)"-v","--verbose","level of verbose",&verbose);
            ArgInt.Treat2Commands((string)"-fp","--floatparameter","parameter in float",&floatpara);
            ArgInt.Treat2Commands((string)"-n","--number","number", &number);
            ArgInt.Treat2Commands((string)"-t","--text"," a text",&mytext);
            ArgInt.Treat1Command((string)"-r","--read",&readmode);
            if (ArgInt.Treat1Command((string)"-h","--help" )) HelpAndExit();
            if (ArgInt.Treat1Command((string)"-ver","--version") ) {Announce(argv[0],version);exit(0);}
            ArgInt.TestNoCommandSoExit();
            
         }
         
// part execution of main using the result of treatement of input arguments   
     if (readmode) stringreadmode = "true";
     cout << "\n";
     cout << " We running "<< argv[0]<<" program  to test ArgIntepretor class\n";
     cout << " The level verbose is " << verbose<<"\n";
     cout << " The text is : "<< mytext << "\n";
     cout << " The read mode is "<< stringreadmode<<"\n";
     cout << " The float parameter is "<<floatpara<<"\n";
     cout << "\n";
     cout << "  Try  : \"classtest -ver \" to know version \n"; 
     cout << "\n";
 }    
//_________________________________________________________________________________________ 
*/  
