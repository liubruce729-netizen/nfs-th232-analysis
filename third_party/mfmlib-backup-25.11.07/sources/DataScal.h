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

#ifndef DataScal_H
#define DataScal_H

#include <string.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#ifndef NO_MFMXML
#include <tinyxml.h>
#endif
#include "MError.h"
#include "XmlTags.h"
using namespace std;


//______________________________________________________________________________
class DataScal {

private:
	uint32_t fLabel;    // Scaler Label
	int32_t  fStatus;   // Status Channel
	uint64_t fCount;    // Value of scaler
	uint64_t fFrequency;//Frequency
	uint64_t fTics;     //Time
	int32_t  fAcqStatus;// status acq
	int32_t  fBidon1;   // to fill 64 memory allignement
	string   fName;     // name of parameter
	MError fError;
public:
	DataScal();

	DataScal(string name, uint32_t label, uint64_t count = 0,
			uint64_t frequency = 0, int32_t status = 0, uint64_t tics = 0,
			int32_t acqstatus = 0) ;

	DataScal(DataScal *DataScal) ;

	uint32_t Label(void) const ;
	void SetLabel(uint32_t label) ;
	uint64_t Count(void) const ;
	void SetCount(uint64_t count) ;
	uint64_t Frequency(void) const ;
	void SetFrequency(uint64_t f) ;
	int32_t Status(void) const ;
	void SetStatus(int32_t status) ;
	uint64_t Tics(void) const ;
	void SetTics(uint64_t tics) ;
	int32_t AcqStatus(void) const ;
	void SetAcqStatus(int32_t acqstatus);
	string Name(void) const ;
	void SetName(string name);

	string DumpDataScal();
#ifndef NO_MFMXML
	void ReadXML(TiXmlNode* pParent, unsigned int indent);
	void CreatXML(TiXmlNode* pParent, DataScal * defautpar);
#endif
	const char * getIndent(unsigned int numIndents);// retur a string of numIndents for presentation
	//
};

#endif
