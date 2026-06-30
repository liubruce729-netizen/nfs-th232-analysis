/**
\mainpage Ganil Root Utilities general information
*
**\verbinclude "README.md"
* <a href="README.md"></a>, 
**/


// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#include "General.h"
#include "GRU.h"
#include "TROOT.h"
#include "TRint.h"
#include <TGClient.h>
#include <stdlib.h>
#include <signal.h>
#include "Vigru.h"
#include <signal.h>


unsigned short stop = 0;//flag of stop

	int main(int argc, char **argv) {
		char* version = (char*) GRU_VERSION;
		char fonction;
		int i;
		char tempo[MAX_CARACTERES];
		int Bufsize = BUFSIZE;
		fonction = 'r';//default  function no graphi

		if (argc > 1) {

			for (i = 1; i < argc; i++) {

				if ((strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--root")
						== 0)) {
					fonction = 'r';
				}
				if ((strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--graphic")
						== 0)) {
					fonction = 'g';
				}

				if ((argv[i][0] == '-') && (argv[i][1] == 'b')) {
					strcpy(tempo, argv[i]);
					tempo[0] = ' ';
					tempo[1] = ' ';
					Bufsize = atoi(tempo);

					if ((Bufsize != 512) && (Bufsize != 1024) && (Bufsize
							!= 2048) && (Bufsize != 4096) && (Bufsize != 8192)
							&& (Bufsize != 16384) && (Bufsize != 32768)
							&& (Bufsize != 65536)) {
						fonction = 'b';
					}
				}
				if ((strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help")
						== 0)) {
					fonction = 'h';
				}

			}
		}

		TTask *run = new MyRun("run", "Process one run");
		gROOT->GetListOfTasks()->Add(run);
		gROOT->GetListOfBrowsables()->Add(run);
		switch (fonction) {
		case 'r': {
			cout << "  ******************************************* " << "\n";
			cout << "  *                                         * " << "\n";
			cout << "  *      HELLO  -- You are Running  GRU     * " << "\n";
			cout << "  * Ganil ROOT Utilities, Version : " << version << "* \n";
			cout << "  *    Compiled :"<< BUILD_DATE <<"   "<< BUILD_TIME << "        * " << "\n";
			cout << "  ******************************************* " << "\n";

			// Create interactive interface
			//argc=0;

			TRint* theApp = new TRint("GRU", &argc, argv, NULL, 0);
			theApp->SetPrompt("GRU>");

			theApp->Run();
			cout << " \n Bye Bye\n";
			delete (theApp);
			break;
		}

		case 'g': {
			cout
					<< "  usage : GRU -r/-g -bbuffersize , no graphic version, sorry!"
					<< "\n";

			break;
		}
		case 'b': {
			cout << "Bad argument \n";
		}
		case 'h':
		default: {
			cout
					<< " Usage : GRU  -h (--help) -r (--root)  -bbufferSize (default =  16384).  ex:  GRU -r -b4096"
					<< "\n";
			break;
		}
		}
	}





