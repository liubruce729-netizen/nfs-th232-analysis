//gsoap ns service name: GruSoap
//gsoap ns service style: document
//gsoap ns service encoding: literal
//gsoap ns service location: http://localhost:6603
//gsoap ns schema namespace: urn:GruSoap


#ifndef __libGruSoap_ServiceH__
#define __libGruSoap_ServiceH__


class xsd__base64Binary
{
 public:
   unsigned char* __ptr;
   int __size;
};

class ns__MyVector
{
 public:
   int * __ptr;
   int __size;
};

class ns__charliste
{
 public:
   char* charlistwords;
   int   nb_list;
};
class ns__ReturnCode
{
   int rCode;
};
int ns__AwordsRvectorInt( char*, xsd__base64Binary &out );
//int ns__Calim2( char* ,char*,int, int[8], int[72]  , ns__ReturnCode &out ); // non
//int ns__Calim2( char* ,char*,int, xsd__base64Binary*, xsd__base64Binary*, ns__ReturnCode &out ); // ok
int ns__Calim2( char* ,char*,int, ns__MyVector*, ns__MyVector*, ns__ReturnCode &out ); // ok
//int ns__Calim2( char* ,char*,int, int*, int*, ns__ReturnCode &out );  // non
int ns__Aword0  	 ( ns__ReturnCode &out ); // no parameter ( fonction to test connection)
int ns__Awords  	 ( char* , ns__ReturnCode &out ); //ask in words,
int ns__AwordsRwords (char* , ns__charliste &out ); // ask in words, return in word

#endif
