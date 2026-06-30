/*
 * SCInterfaceSoap.cpp
 *
 *  Created on: 16 déc. 2015
 *      Author: legeard
 */

#include "SCSoap.nsmap"
#include "SCSoapH.h"
#include "SCInterfaceSoap.h"
#include "sc_soap_services.hh"

struct Namespace namespaces[1024];

//_________________________________________________________________________________
SCInterfaceSoap::SCInterfaceSoap() {
	// TODO Stub du constructeur généré automatiquement

}
//_________________________________________________________________________________
SCInterfaceSoap::~SCInterfaceSoap() {
	// TODO !CodeTemplates.destructorstub.tododesc!
}

//_________________________________________________________________________________
int SCInterfaceSoap::ReadScope(char*httpmessage, char ** ptzone,int *sizezone,
		int *myvectinsize) {
	//zone ( pt and size) buffer where data will be copied
	//myvectinsize , size of return buffer from soap connection.
	//if myvectinsize is smaller than myvectinsize, zone will be resized.

	int error = 0;
	xsd__base64Binary_test vector;

	char * pt = NULL;
	char *pttmp =NULL;
	int new_size = 0;
	struct soap v_soap;
	soap_init(&v_soap);
	soap_set_namespaces(&v_soap, SCSoap_namespaces);

	soap_call_sc__ReadScope(&v_soap, httpmessage, (char*) "", vector);

	if (v_soap.error) {
		soap_print_fault(&v_soap, stderr);
		error = 1;
	} else {

		new_size= vector.__size / (sizeof(char));
		pt = (char*) (vector.__ptr);

		*myvectinsize = new_size;
		if (new_size>*sizezone){
			ReallocBufferSize(ptzone,new_size,*sizezone);
			*sizezone = new_size;
		}
		pttmp = *ptzone;
		memcpy(*ptzone, pt , new_size);
	}
	soap_end(&v_soap);

	return error;
}

//_______________________________________________________________________________
void  SCInterfaceSoap::ReallocBufferSize(char**pt,int newsize,int oldsize, bool ifinferior) {
	/// Do memory allocation or a reallacation for frame\n
	/// if ifinferior==true the allocaton is forced to size event if the acutal size is bigger
	/// ifinferior = true by default
	char* pt_new=NULL;
	char* ppt=NULL;
	ppt = *pt;
	if (!ifinferior or (newsize > oldsize)) {
		pt_new = (char*) (realloc((void*) ppt, newsize));
		//set 0 in rest of buffer
		RazZone(pt_new,newsize,oldsize);
		ppt = pt_new;
	}
	*pt = pt_new;
}
//_______________________________________________________________________________
void  SCInterfaceSoap::RazZone(char* pt,int size,int start_size){
	if (start_size>size) return;
	for (int i = start_size; i <size; i++) {
		pt[i] = 0;
	}
}
//_______________________________________________________________________________

