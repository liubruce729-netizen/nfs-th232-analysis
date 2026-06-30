// Author: $Author: legeard $
//
// Part of GRU
//
// GNetServer : Network server to serv  command
//
//
//
//////////////////////////////////////////////////////////////////////////

// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************
#include "GSpectra.h"
#include "GNetServer.h"

#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<string>
#include<iostream>
#include"GError.h"
#include"TObject.h"


ClassImp( GNetServer);

//_________________________________________________________________________________________
GNetServer::GNetServer(int port) {
	// constructor of GNetServer
	fCommand = NULL;
    fSpectra = NULL;
    fAcq = NULL;
	fRunning = false;
	fPort = port;

	fServerReady = false;
	
}
//_________________________________________________________________________________________

GNetServer::~GNetServer() {
	// desctructor
	if (fCommand) {
		delete fCommand;
		fCommand=NULL;
	}

	if (fRunning == true) {
		fRunning = false;
	}
}
//_________________________________________________________________________________________
void GNetServer::InitCalimero() {
	//if (fCommand->InitGlobalCalim()) {
		//	startstartserver();
			//fServerReady = true;
		//}
}

//_________________________________________________________________________________________
void GNetServer::PauseAcq(int time) {
	// ask a pause in acquisition
	// if time > 0  time = timeout
	// if time = 0 pause is resumed
	// if time <0 pause is infinite  until a resume
   if (fCommand && fAcq)
	fCommand->PauseAcq(time);

}

//_________________________________________________________________________________________
void GNetServer::SetCommand(GCommand* command) {

	if (fCommand){
		if (fVerbose >5) fError.Infos("Object GCommand replaced a new one.");
		delete fCommand;
	}
	fCommand = command;
}
//_________________________________________________________________________________________
Int_t GNetServer::Testport(Int_t port, bool autoportincrement) {

	GNetClientRoot testnet((char*) "localhost");
	Int_t ret;
	ret = port;

	for (int i = 0; i < 2000; i++) {
		ret = testnet.TestPortFree(port, (char*) "localhost");
		if (ret > 0)
			break;
		if ((ret <= 0) && (autoportincrement))
			port++;
		if (!autoportincrement)
			break;
	}

	return ret;
}
//_________________________________________________________________________________________
void  GNetServer::ToDoInCaseOfInterrupt(){
	StopServer(false);
}
