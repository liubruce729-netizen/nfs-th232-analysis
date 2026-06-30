
//////////////////////////////////////////////////////////////////////////
//
// ------------------------------------------------------------------------
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DataGenParameters_H
#define DataGenParameters_H

#include <vector>
#include <iostream>
#include <string.h>
#include "MError.h"
#include <string.h>
#include <stdint.h>
#ifndef NO_MFMXML
#include <tinyxml.h>
#endif
#include "XmlTags.h"

using namespace std;

#define MAX_CAR 255

// Author: $Author:  modified by Legeard
/***************************************************************************
 //                        DataGenParameters.cpp  -  Parameters name handling
 //////////////////////////////////////////////////////////////////////////
 //
 //
 // DataGenParameters
 //
 // Virtual class to factorize methode between Parameter and scalers
 //
 ///////////////////////////////////////////////////////////////////////
//______________________________________________________________________________
 *
 */
class DataGenParameters  {


public:
	
	DataGenParameters(void);
	virtual ~DataGenParameters(void);

	virtual int Fill(const char *buffParam)=0;


        virtual bool IsAActionFile(const char *actionfile);
        virtual bool IsFileExiste(string filename, bool withtexterror);


	virtual uint32_t* GetVectorLabelToIndex();

	virtual int GetIndex(const int label) ;

	virtual void TestList()=0;
	virtual int GetLabel(const string parName)=0;
	virtual void FillLabelToIndexVectors()=0;
	virtual void FillLabelToExist()=0;
	virtual int GetIndex(const string parName) =0;
	virtual int GetMaxLabel() =0;

	virtual const char* GetParNameFromIndex(const int index) =0;
	virtual const char* GetParNameFromLabel(const int label) =0;


	virtual int GetNbParameters(void)   const { return fNb_Parameters;}
	virtual void SetNbParameters(int nb)    { fNb_Parameters = nb;}
	virtual void DumpListOfNames() =0;


	virtual const char * getIndent( unsigned int numIndents );
	virtual const char * getIndentAlt( unsigned int numIndents );
#ifndef NO_MFMXML
	virtual int FillFromActionXMLFile (const char *actionXMLfile);
 	virtual TiXmlDocument* GetXmlDoc();
	virtual void dump_to_stdout(TiXmlNode* pParent, unsigned int indent);
	virtual int dump_attribs_to_stdout(TiXmlElement* pElement, unsigned int indent) ;
	virtual TiXmlDocument * OpenXmlFile(const char* pFilename);
	virtual int ReadXML( TiXmlNode* pParent, unsigned int indent );
	virtual void ReadXMLspecific(TiXmlNode* pParent, unsigned int indent)=0;
	virtual int GetNbElementInList()=0 ;
	virtual void CreatXML()=0;
	virtual void WriteXML(const char * filename) ;
#endif

	virtual int* creatLabelToExist(int* maxlabel);
protected:
	void  creatLabelToIndexVectors();
private:

	char* CopyParam(char *Dest, char *Source) ;

protected:
	int fVerbose;
	int fNb_Parameters;
private:
	unsigned int NUM_INDENTS_PER_SPACEbis;
	int bindon;

protected:
	uint32_t* fLabelToIndex; //! vector for fast convertion of Label  to order index
	int* fLabelToExist; //! vector for fast convertion of Label  to exist
#ifndef NO_MFMXML
	TiXmlDocument* fTiXmlDocument ;/// xlm objet to manage xml file
#endif
	MError fError;

};

#endif
