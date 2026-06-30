#ifndef DataScalers_H
#define DataScalers_H
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
#include <vector>
#include <iostream>
#include "XmlTags.h"
#include "DataScal.h"
#include "DataGenParameters.h"
#include "MError.h"
#include <string.h>
#include <stdint.h>
#ifndef NO_MFMXML
#include <tinyxml.h>
#endif
#include "XmlTags.h"
using namespace std;

#define MAX_CAR 255

class DataScalers : public DataGenParameters  {



public:
	
	DataScalers(void);
	virtual ~DataScalers(void);
	virtual int Fill(const char *buffParam) ;
	virtual int FillFromMfmFrame(const char *buffParam);
	virtual int FillFromXmlFile(const char *actionfile);

	virtual int GetLabel(const string parName) ;
	virtual int GetLabel(const int index) ;


	virtual int GetIndex(const int label) ;
	virtual int GetIndex(const string parName) ;
	virtual int GetMaxLabel();
//	virtual int GetNbits(const string parName) ;
//  virtual int GetNbitsFromLabel(const int label) ;
//	virtual int GetNbitsFromIndex(const int label) ;
	virtual int GetNbitsFromLabel(const int label);
	virtual const char* GetParNameFromIndex(const int index) ;
	virtual const char* GetParNameFromLabel(const int label) ;
	virtual void FillLabelToIndexVectors();
	virtual void FillLabelToExist();
	virtual void TestList();
	virtual void DumpList();
	virtual void DumpListOfNames();
	virtual DataScal* FindScaleObject(const string name);
	virtual int* creatLabelToExist(int* maxlabel);
	virtual int  GetNbElementInList() ;
#ifndef NO_MFMXML
	virtual void ReadXMLspecific(TiXmlNode* pParent, unsigned int indent) ;
	virtual void ReadXMLcpu(TiXmlNode* pParent, unsigned int indent) ;
	virtual void ReadXMLvoie(TiXmlNode* pParent, unsigned int indent) ;
	virtual void CreatXML();
#endif 


private:
	void creatLabelToIndexVectors();
	char* CopyScaler(char *Dest, char *Source) ;
	
	
private:

	vector<DataScal> fList; //! Scaler List, maybe a better structure could be made
	int fClassStatus; // Status error
	int fNb_Scaler; // nb of Scaler
	uint32_t* fLabelToIndex; //! vector for fast convertion of Label  to order index
	uint32_t* fLabelToUserLabel; //! vector for fast convertion of Label  to order index
#ifndef NO_MFMXML
	TiXmlDocument* fTiXmlDocument ;/// xlm objet to manage xml file
#endif
public:
	MError fError;
};

#endif
