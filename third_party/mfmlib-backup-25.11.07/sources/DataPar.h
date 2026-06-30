//////////////////////////////////////////////////////////////////////////

//
// ---------------------------------------------------------------------------
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DataPar_H
#define DataPar_H
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <stdint.h>
#ifndef NO_MFMXML

#include <tinyxml.h>
#endif
#include "MError.h"
#include "XmlTags.h"

using namespace std;

//______________________________________________________________________________
class DataPar {

public:
	DataPar(string name, const int label, int nbbits = 16, int depth = 1,
			int sizeinmemory = 16);
	DataPar(DataPar *datapar);
	DataPar() ;
	int Label(void) const ;

	void SetLabel(int label) ;
	void SetName(string name);

	int Nbits(void) const ;
	void SetNbits(int32_t bits);
	void SetDepth(int32_t bits);
	int Depth(void) const;
	string Name(void) const;
	const char * getIndent(unsigned int numIndents);
	const char * getIndentAlt(unsigned int numIndents) ;
	#ifndef NO_MFMXML
	void ReadXML(TiXmlNode* pParent, unsigned int indent);
	void CreatXML(TiXmlNode* pParent,DataPar * defautpar);

	void CreatXMLdefault(TiXmlNode* pParent) ;
#endif

	string DumpDataPar() ;
private:
	int32_t fLabel; // Parameter Label
	int32_t fNbits; // Parameter size  in bit number
	int32_t fDepth; // Parameter depth
	int32_t fSizeInMemory;// Size in bit number  of parameter taken in memory  > fNbits
	string fName; // name of parameter
	MError fError;

};

#endif
