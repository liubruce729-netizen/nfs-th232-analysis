
//////////////////////////////////////////////////////////////////////////

// Class to manage list of  parameters ( also called literal label) with their numerical label.
// ---------------------------------------------------------------------------
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <vector>
#include <iostream>
#include "DataPar.h"
#include "MError.h"
#include <string.h>
#include <stdint.h>
#ifndef NO_MFMXML
#include <tinyxml.h>
#endif
#include "DataGenParameters.h"
using namespace std;
#ifndef DataParameters_H
#define DataParameters_H
#define MAX_CAR 255
//______________________________________________________________________________
class DataParameters : public DataGenParameters{

public:
	

	DataParameters(void);
	virtual ~DataParameters(void);

	virtual int Fill(const char *buffParam);
 	virtual int FillFromActionFile(const char *actionfile);
        virtual bool IsAActionFile(const char *actionfile);

	virtual void FillLabelToIndexVectors();
	virtual void FillLabelToExist();
	virtual int GetLabel(const string parName) ;
	virtual int GetLabel(const int index) ;

	virtual int GetIndex(const int label) ;
	virtual int GetIndex(const string parName) ;
	virtual int GetMaxLabel();
	virtual int GetNbits(const string parName) ;

	virtual int GetNbitsFromIndex(const int label) ;
	virtual int GetNbitsFromLabel(const int label);
	virtual int GetDepth(const string parName) ;
	virtual int GetDepthFromLabel(const int label) ;
	virtual int GetDepthFromIndex(const int index);

	virtual const char* GetParNameFromIndex(const int index) ;
	virtual const char* GetParNameFromLabel(const int label) ;

	virtual void TestList();
	virtual void DumpListPara();
	virtual void DumpListOfNames ();
	virtual DataPar* FindParameterObject(const string name);
	virtual int GetNbElementInList() {
		return fNb_Parameters;
	}
#ifndef NO_MFMXML
	virtual void ReadXMLspecific(TiXmlNode* pParent, unsigned int indent);
	virtual void CreatXML();
#endif
private:

	char* CopyParam(char *Dest, char *Source) ;
private:

	DataPar DefaultDataPar ;// Data parameter containing default values

	vector<DataPar> fList; //!! Parameters List, maybe a better structure could be made
public:
	MError fError;

};

#endif
