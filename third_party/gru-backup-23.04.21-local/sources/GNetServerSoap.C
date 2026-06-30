// Author: $Author: legeard $
//
// Part of GRU
//
// GNetServerSoap : Network server for Soap server command
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

#include "GNetServerSoap.h"
#include "GNetClientSoap.h"
#include "GStateMachine.h"
#include "GNetServerRoot.h"

#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<string>
#include<iostream>
#include"GError.h"
// serveur SOAP
#ifndef NO_GSOAP
#include "GSoapErrorCode.h"
#include "GSoapErrorCodef.h"

#include "GruSoapH.h"
#include "GruSoap2.nsmap"

struct Namespace namespaces2;
#endif

ClassImp( GNetServerSoap);
int fLocalVerbose; 

GNetServerSoap * MyThis;

//_________________________________________________________________________________________
GNetServerSoap::GNetServerSoap(int port) :
	GNetServer(port) {
	// constructor of GNetServerSoap
	MyThis = this;
	fRunning = false;
	fRootServer = NULL;
	strcpy(this->fNameThread, "GRU_ServerSoap");
	fPrompt_wait = false;
	fMessage = new char[64];
	InitCommand();
	fLocalVerbose =1;
}

//_________________________________________________________________________________________

GNetServerSoap::~GNetServerSoap() {
	// desctructor
	if (fMessage)
	delete fMessage;
	if (GetRunningFlag()) {
		SetRunningFlag(false);
	}
}
//_________________________________________________________________________________________
bool testwordgru(char* word) {
	if ((strcasecmp(word, "GRU") == 0) or (strcasecmp(word, "CALIMERO") == 0)) {
		return true;
	} else {
		printf("         WORD1  = always GRU (and sometime CALIMERO) \n");
		return false;
	}
}
//_________________________________________________________________________________________
void GNetServerSoap::SetVerbose (int verb){
	
	fLocalVerbose = verb;
	GBase::SetVerbose(verb);
}
//_________________________________________________________________________________________
void GNetServerSoap::InitCommand() {
	SetCommand(new GCommand((TObject*) this));
}

//_________________________________________________________________________________________
void GNetServerSoap::StopServer(bool quiet) {
	if (GetRunningFlag())
		SetRunningFlag(false);
}

//_________________________________________________________________________________________
void GNetServerSoap::StartServer(bool quiet) {
	//set and start network soap server
	
#ifndef NO_GSOAP

	struct soap v_soap; // Context of soap service
	//struct soap *v_tsoap;    // we clone the context SOAP for the thread
	//pthread_t v_tid;         // l'identifiant du thread identifian
	// on initialise la socket
	int port = GetPort();
	TString tempos;
	tempos.Form("Init Soap Server  in port : %d ", port);
	fError.Infos(tempos);
	soap_init(&v_soap);
	v_soap.bind_flags = SO_REUSEADDR; 
	int test;
	soap_set_namespaces(&v_soap, GruSoap_namespaces2);
	// on cree la socket mere de connexion

	for (int i = 0; i < 10; i++) {
		test = soap_bind(&v_soap, NULL, port, 100);
		if (test < 0) {
			fError.TreatError(2, 0,
					"Error in init soap server , try again in few seconds!");
			WaitWithPoint(10);
			cout << "\n" << flush;
		} else {
			i = 20;
		}
	}
	if (test < 0) {
		fError.TreatError(
				2,
				0,
				"Error in init soap server , check if port is free or if a other soap server is running");
		exit(0);
		return;
	}

	fPrompt_wait = true;
	// on gere une boucle infinie pour recevoir les requetes
	SetRunningFlag(true);
	while (GetRunningFlag()) {
		if (fPrompt_wait)
			fError.Infos("Waiting Soap Commands...>>");
		fPrompt_wait = false;
		v_soap.accept_timeout = 1; // on rend l'attente non bloquante
		if (soap_accept(&v_soap) < 0) {
			continue;
		}
		GruSoap_serve(&v_soap);
		soap_destroy(&v_soap);
		soap_end(&v_soap);
		fPrompt_wait = true;
		//------------------------
	}
	soap_done(&v_soap);
	fError.Infos("End of soap server");

#else
	TString tempos;
	tempos.Form("Compile with NO_GSOAP , so GNetServerSoap:StartServer inoperative ");
	fError.Infos(tempos);

#endif	
}

#ifndef NO_GSOAP
//_________________________________________________________________________________________

int ns__Aword0(struct soap *p_soap, struct ns__ReturnCode &p_out) // pour retourner plusieurs valeurs
{

	TString tempos;

	p_out.rCode = RETOUR_SOAP_OK;

	tempos.Form("Server ns__Aword0s  [ rCode =%s]", message_code((p_out.rCode),MyThis->GetMessage()));
	//Command->GetError()->Infos(tempos);

	if (!(MyThis->GetCommand()->AnalyseCommandAndDo((char*) ""))) { //TO DO verifier fonctionnement de Command->AnalyseCommandAndDo("")
		MyThis->GetCommand()->GetError()->TreatError(1, 0,
				"ns__Aword0 : It is not a GRU Net commmand! ");
		p_out.rCode = RETOUR_SOAP_NOT_GRU_COMMAND;
	}

	return SOAP_OK;

}
//_________________________________________________________________________________________

int ns__Awords(struct soap *p_soap, char p_chaine1[256],// GRU
		struct ns__ReturnCode &p_out // pour retourner plusieurs valeurs
) {

	TString tempos;
	TString commandstring;
	commandstring = p_chaine1;
	p_out.rCode = RETOUR_SOAP_OK;

	tempos.Form("Server ns__Awords  [%s  rCode =%s]", p_chaine1, message_code((
			p_out.rCode),MyThis->GetMessage()));
	//Command->GetError()->Infos(tempos);

	if (!(MyThis->GetCommand()->AnalyseCommandAndDo(p_chaine1))) {
		MyThis->GetCommand()->GetError()->TreatError(1, 0,
				"ns__Awords : It is not a GRU Net commmand : ", p_chaine1);
		p_out.rCode = RETOUR_SOAP_NOT_GRU_COMMAND;
	}

	return SOAP_OK;

}
//_________________________________________________________________________________________

int ns__AwordsRwords(struct soap *p_soap, // contexte d'execution du service web
		char p_chaine1[256],//
		struct ns__charliste &p_out // pour retourner plusieurs valeurs
) {
	TString streturn;
	if(MyThis->GetCommand()->GetVerbose() ) printf("Server ns__AwordsRwords  [%s  ]\n", p_chaine1);

	TString tempos;
	TString commandstring;
	commandstring = p_chaine1;

	if ((MyThis->GetCommand()->AnalyseCommandAndDo(p_chaine1, &streturn))) {
		p_out.charlistwords = (char*) soap_malloc(p_soap, streturn.Sizeof());
		strcpy(p_out.charlistwords, streturn.Data());
		
		p_out.nb_list = MyThis->WordsCount((char*) (streturn.Data()));

	} else {
		MyThis->GetCommand()->GetError()->TreatError(1, 0,
				"ns__AwordsRwords : It is not a GRU Net command! ");
	}
	return SOAP_OK;
}
//_________________________________________________________________________________________
int ns__AwordsRvectorInt(struct soap *p_soap, // contexte d'execution du service web
		char p_chaine1[256],// GRUSOAP  SPECTRUM Spectrum name family name
		struct xsd__base64Binary &p_out // pour retourner plusieurs valeurs
) {

	TString tempos;
	tempos.Form("No action with ns__Spectra in this case");
	MyThis->GetCommand()->GetError()->Infos(tempos);

	return SOAP_OK;
}
//_________________________________________________________________________________________

int ns__Calim2(
		struct soap *p_soap, // contexte d'ex�cution du service web
		char p_chaine1[MAX_CARACTERES], char p_chaine2[MAX_CARACTERES],
		int p_coups,
		ns__MyVector* p_valgene,
		ns__MyVector* p_voie,
		struct ns__ReturnCode &p_out // pour retourner plusieurs valeurs
) {
	TString tempos;
	tempos = p_chaine1;
	tempos.ToLower();
	strcpy(p_chaine1, tempos.Data());
	tempos = p_chaine2;
	tempos.ToLower();
	strcpy(p_chaine2, tempos.Data());
	int size_mat = NB_TELESCOPES * X_AND_Y;
	int size_voie = NB_MATES * NB_TELESCOPES;
	int * vectori;
	p_out.rCode = RETOUR_SOAP_NOT_OK; //
	int voie[NB_MATES * NB_TELESCOPES];
        int valgene[NB_MATES * NB_TELESCOPES];
	int command = 0;
	
	// command trace
	tempos.Form ("Command Received : %s %s  nb_coups = %d",p_chaine1,p_chaine2,p_coups);
	if (fLocalVerbose>1)MyThis->GetCommand()->GetError()->Infos(tempos);
	tempos.Form ("with 2 vectors of size %d and %d", NB_TELESCOPES * 2,NB_MATES * NB_TELESCOPES);
	if (fLocalVerbose>3)MyThis->GetCommand()->GetError()->Infos(tempos);
	
	int i;
	vectori = (int*) (p_valgene->__ptr);

	if (fLocalVerbose>3)printf("-----------------mat :  value of gene----------------------\n");
	for (i = 0; i < size_mat; i++) {
		valgene[i]= vectori[i];
	if (fLocalVerbose>3)	printf("%d ", vectori[i]);
	if (fLocalVerbose>3)	if ((i + 1) % (X_AND_Y * NB_TELESCOPES) == 0)
			printf("\n");
	}

	if (fLocalVerbose>3)printf("----------------selected-channels-------------------------\n");
	vectori = (int*) (p_voie->__ptr);
	for (i = 0; i < size_voie; i++) {
		voie[i]= vectori[i];
	if (fLocalVerbose>3)	printf("%2d  ", vectori[i]);
	if (fLocalVerbose>3)	if ((i + 1) % NB_MATES == 0)
			printf("\n");
	}
	if (fLocalVerbose>3) printf("-----------------------------------------------------------\n");
	
	/*	for (i = 0; i < NB_TELESCOPES * 2; i++) {
	 cout <<"debug ns__Calim2 p_valgene["<<i<<"] = "<<p_valgene[i]<<"\n";
	 }
	 for (i = 0; i < NB_MATES * NB_TELESCOPES; i++) {
	 cout <<"debug ns__Calim2 p_voie["<<i<<"] = "<<p_voie[i]<<"\n";
	 }*/
	if (!(testwordgru(p_chaine1))) {
		cout << p_chaine1 << " : is not a GRUCore Net commmand! : \n";
		p_out.rCode = RETOUR_SOAP_NOT_GRU_COMMAND;
		return SOAP_OK;
	}

	if (strcasecmp(p_chaine2, "run") == 0) {
		if (fLocalVerbose>3) 
		
		cout<<dec << "---------------------DoRun( p_coups = " << p_coups
				<< ", p_valgene[0]...(example) = " << valgene[0]
				<< ", p_voie[0] =" << p_voie << " )----------------------\n";
		MyThis->GetCommand()->DoRun(p_coups, valgene, voie);
		command++;
		p_out.rCode = RETOUR_SOAP_OK;
	}

	if (command != 1) {
		MyThis->GetCommand()->GetError()->TreatError(1, 0, "Command found!");
		p_out.rCode = RETOUR_SOAP_NOT_GRU_COMMAND;
	}

	// p_out.vecteur.push_back(1);
	return SOAP_OK;
}
#endif
//_________________________________________________________________________________________
void* GNetServerSoap::handler(void * p_soap) {
#ifndef NO_GSOAP
	struct soap * v_soap = (struct soap *) p_soap;
	pthread_detach(pthread_self());
	GruSoap_serve(v_soap);
	soap_destroy(v_soap);
	soap_end(v_soap);
	soap_done(v_soap);
	free(v_soap);
	pthread_exit(NULL);
#else
	TString tempos;
	tempos.Form("Compile with NO_GSOAP , so GNetServerSoap:handler inoperative ");
	fError.Infos(tempos);
#endif
	return NULL;

}
//_________________________________________________________________________________________
void GNetServerSoap::SetRootServer(GNetServerRoot * serv) {
	fRootServer = serv;
}
//_________________________________________________________________________________________
void GNetServerSoap::InitSpectraServer(int mode, int port) {
	// mode =1 -> init  , mode =0 -> stop
	if (mode > 0) {
		if (fRootServer != NULL) {
			//	fRootServer =	new GNetServerRoot(GetCommand()->GetAcq());
			if (port > 100)
				fRootServer->SetPort(port);
			//	fRootServer->StartServer();
			sleep(1);
		} else {
			fError.TreatError(1, 0,
					"Impossible to start Spectra Server RootServer does'nt exist");
		}
	} else {
		if (fRootServer) {
			((GNetServerRoot*) fRootServer)->StopServer();
			delete (fRootServer);
			fRootServer = NULL;
			sleep(1);
		}
	}
}
//_________________________________________________________________________________________
