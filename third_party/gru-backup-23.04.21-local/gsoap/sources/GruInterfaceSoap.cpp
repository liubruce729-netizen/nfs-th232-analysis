/*
 * InterfaceSoap.cpp
 *
 *  Created on: 14 déc. 2015
 *      Author: legeard
 */
#include "GruSoap.nsmap"
#include "stdsoap2.h"
#include "GruInterfaceSoap.h"
#include"libGruSoap_service.h"
#include "GSoapErrorCode.h"
#include "GSoapErrorCodef.h"
//_________________________________________________________________________________
GruInterfaceSoap::GruInterfaceSoap() {

}

//_________________________________________________________________________________
GruInterfaceSoap::~GruInterfaceSoap() {

}

//_________________________________________________________________________________
int GruInterfaceSoap::Awords(char* httpmessage, char* command) {
	int error = 0;
	ns__ReturnCode r_reponse;
	struct soap v_soap;
	soap_init(&v_soap);
	soap_set_namespaces(&v_soap, GruSoap_namespaces);
	soap_call_ns__Awords(&v_soap, httpmessage, "", command, r_reponse);
	if (v_soap.error) {
		soap_print_fault(&v_soap, stderr);
		error = 1;
	}
	soap_end(&v_soap);
	return error;
}
//_________________________________________________________________________________
int GruInterfaceSoap::AwordsRwords(char* httpmessage, char* command,
		string * charlist) {
	int error = 0;
	struct soap v_soap;
	ns__charliste charlist2;
	soap_init(&v_soap);
	soap_set_namespaces(&v_soap, GruSoap_namespaces);
	error = soap_call_ns__AwordsRwords(&v_soap, httpmessage, (char*) "",
			(char*) command, charlist2);

	*charlist = charlist2.charlistwords;

	printf("Liste GruInterfaceSoap : %s \n", charlist->data());
	//printf("Nb entries : %d", charlist->nb_list);
	printf("\nfin\n");

	if (v_soap.error) {
		soap_print_fault(&v_soap, stderr);
	}
	soap_end(&v_soap);
	return error;
}

//_________________________________________________________________________________
int GruInterfaceSoap::AwordsRvectorInt(char* httpmessage, char* command,
		uint32_t ** myvectinpt, uint32_t *myvectinsize) {
	int error = 0;
	xsd__base64Binary vector;
	struct soap v_soap;
	uint32_t * pt = NULL;
	uint32_t *pttmp = *myvectinpt;
	soap_init(&v_soap);
	soap_set_namespaces(&v_soap, GruSoap_namespaces);

	soap_call_ns__AwordsRvectorInt(&v_soap, httpmessage, (char*) "", command,
			vector);
	if (v_soap.error) {
		soap_print_fault(&v_soap, stderr);
		error = 1;
	} else {
		*myvectinsize = vector.__size / (sizeof(uint32_t));
		pttmp = new uint32_t[*myvectinsize];
		*myvectinpt=pttmp;
		pt = (uint32_t*) (vector.__ptr);
		for (int i = 0; i < *myvectinsize; i++) {
			pttmp[i] = pt[i];
		}
	}
	soap_end(&v_soap);
	return error;
}

//_________________________________________________________________________________
int GruInterfaceSoap::Calim2(char* httpmessage, char* command, char * name1,
		char * name2, int entier, int * verctor1, int sizevect1, int * vector2,
		int sizevect2) {

	ns__ReturnCode r_reponse;
	ns__MyVector mat;
	ns__MyVector voie;
	struct soap v_soap;
	soap_init(&v_soap);

	mat.__ptr = (int *) soap_malloc(&v_soap, sizevect1 * sizeof(int));
	mat.__size = sizevect1;

	voie.__ptr = (int*) soap_malloc(&v_soap, sizevect2 * sizeof(int));
	voie.__size = sizevect2;

	soap_call_ns__Calim2(&v_soap, httpmessage, "", command, name1, entier,
			&mat, &voie, r_reponse);

	printf("Code Retour Calim2 = ");
	message_code(r_reponse.rCode);

	soap_end(&v_soap);

	int error = 0;
	return error;
}
//_________________________________________________________________________________
int GruInterfaceSoap::Aword0(char* httpmessage) {
	ns__ReturnCode r_reponse;
	struct soap v_soap;
	soap_init(&v_soap);
	soap_call_ns__Aword0(&v_soap, httpmessage, "", r_reponse);

	if (v_soap.error) {
		soap_print_fault(&v_soap, stderr);
	}
	printf("Client Aword0 : ");
	message_code(r_reponse.rCode);
	soap_end(&v_soap);
	int error = 0;
	return error;
}
//_________________________________________________________________________________

