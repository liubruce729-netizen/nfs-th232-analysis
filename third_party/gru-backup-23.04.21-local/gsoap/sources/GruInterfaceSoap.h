/*
 * InterfaceSoap.h
 *
 *  Created on: 14 déc. 2015
 *      Author: legeard
 */

#ifndef INTERFACESOAP_H_
#define INTERFACESOAP_H_

#include <string>
#include "GruSoapStub.h"
using std::string;
class GruInterfaceSoap {
public:
	GruInterfaceSoap();
	virtual ~GruInterfaceSoap();
	virtual int AwordsRvectorInt( char* httpmessage,char* command, uint32_t ** myvectinpt,uint32_t *myvectinsize );
	virtual int Calim2(char* httpmessage, char* command, char * name1,
			char * name2, int entier, int * verctor1,int sizevect1, int * vector2,int sizevect2);
	virtual int Aword0 (char* httpmessage);
	virtual int Awords  (char* httpmessage,char* command );
	virtual int AwordsRwords( char* httpmessage,char* command, string * charlist );
};

#endif /* INTERFACESOAP_H_ */
