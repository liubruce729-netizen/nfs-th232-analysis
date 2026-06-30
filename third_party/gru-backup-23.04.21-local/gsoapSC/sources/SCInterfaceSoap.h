/*
 * CSInterfaceSoap.h
 *
 *  Created on: 16 déc. 2015
 *      Author: legeard
 */

#ifndef SCInterfaceSoap_H_
#define SCInterfaceSoap_H_

class SCInterfaceSoap {
public:
	SCInterfaceSoap();
	virtual ~SCInterfaceSoap();
	virtual void  RazZone(char* pt,int size,int start_size);
	virtual void  ReallocBufferSize(char**pt,int newsize,int oldsize, bool ifinferior=false);
	//virtual int ReadScope(char* httpmessage, xsd__base64Binary_test *v_reponse );
	virtual int ReadScope(char* httpmessage,char ** ptzone, int *sizezone,int *sizevector );
};
#endif /* SCInterfaceSoap_H_ */
