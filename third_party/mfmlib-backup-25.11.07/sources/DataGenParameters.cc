// Author: $Author:   Legeard
//////////////////////////////////////////////////////////////////////////
//
//
// DataGenParameters
//
// Virtual class to factorize methode between Parameter and scalers
//
/////////////////////////////////////////////////////////////////////////
//

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#include "DataGenParameters.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include <sys/stat.h>
#include <vector>
#include <list>
using namespace std;
//______________________________________________________________________________
DataGenParameters::DataGenParameters(void) {
	//Default constructor. Don't create anything yet.

	// when we'll delete the tree itself
	fLabelToIndex = NULL;
	SetNbParameters(0);
	fVerbose = 0;
}

//______________________________________________________________________________
DataGenParameters::~DataGenParameters(void) {

	if (fLabelToIndex) {
		delete[] fLabelToIndex;
		fLabelToIndex = NULL;
	}
}
//______________________________________________________________________________
uint32_t* DataGenParameters::GetVectorLabelToIndex() {
	return fLabelToIndex;
}



//______________________________________________________________________________
bool DataGenParameters::IsFileExiste(string filename, bool withtexterror) {

	struct stat bufstat; // necessary structure to test existing of a device ( using 'stat' fonction )
	int LocalStatus = stat(filename.data(), &bufstat);
	bool ret;

	if (LocalStatus == -1) {
		if (withtexterror)
			fError.TreatError(2, LocalStatus, filename.data(),
					" File doesn't existe");
		ret = false;
	} else
		ret = true;
	return ret;
}
#ifndef NO_MFMXML
//______________________________________________________________________________
TiXmlDocument* DataGenParameters::GetXmlDoc() {
	return fTiXmlDocument;
}
//______________________________________________________________________________
int DataGenParameters::FillFromActionXMLFile(const char *actionXMLfile) {

	//Read a parameter xml file
	// Data from the parameter buffet (buffParam) have been read from disk and is
	// parsed by this routine who create the list of labels and corresponding
	// label number

	string test;
	string message;
	ostringstream testi;

	int indent = 1;

	test = actionXMLfile;
	if (IsFileExiste(test, true) == false) {
		return 0;
	}
	TiXmlDocument doc(actionXMLfile);
	if (!doc.LoadFile()) {
		message = "erreur lors du chargement";
		message.append(doc.ErrorDesc());
		fError.TreatError(2, doc.ErrorId(), (const char*) (message.data()));
		return 0;
	}
	return (ReadXML((TiXmlNode*) (&doc), indent));

}
#endif
/*
 //______________________________________________________________________________
 int DataGenParameters::GetNbitsFromLabel(const int label) {
 // Return  number of bit corresponding to index.
 return (GetNbitsFromIndex((label)));
 }
 */
//______________________________________________________________________________
const char* DataGenParameters::GetParNameFromLabel(const int label) {
	// Return the text label name corresponding to an index.
	return (GetParNameFromIndex((label)));
}
//______________________________________________________________________________
void DataGenParameters::creatLabelToIndexVectors() {
	// Creation of LabelToIndex Vector and fUserLabelToIndex Vector
	string message;

	int i_label;

	int max_label = 0;

	// research of Max value of label
	max_label = GetMaxLabel();

	message = "Error in creatLabelToIndexVector max_label =0";
	if (max_label == 0) {
		fError.TreatError(2, -1, &message);
		return;
	}
	if (fLabelToIndex) {
		delete[] fLabelToIndex;
	}
	fLabelToIndex = new uint32_t[max_label + 1];

	for (i_label = 0; i_label <= max_label; i_label++) {
		fLabelToIndex[i_label] = 0;

	}
	FillLabelToIndexVectors();
}

//______________________________________________________________________________
int* DataGenParameters::creatLabelToExist(int* max_label) {
	// Creation of LabelToIndex Int Vector
	// use for example for statistics
	//
	string message;

	int i_label;

	*max_label = 0;

	*max_label = GetMaxLabel();
	if (*max_label == 0) {
		fError.TreatError(2, -1, "Error in creatLabelToIndexVectorInt");
		return (NULL);
	}
	fLabelToExist = new int[(*max_label) + 1];

	for (i_label = 0; i_label <= *max_label; i_label++)
		fLabelToExist[i_label] = 0;

	FillLabelToExist();

	return (fLabelToExist);
}
//______________________________________________________________________________
int DataGenParameters::GetIndex(const int label) {
	// Return the order index number
	return (fLabelToIndex[label]);
}

//______________________________________________________________________________


bool DataGenParameters::IsAActionFile(const char *actionfile) {

	FILE *fp = fopen(actionfile, "r");

	char line[MAX_CAR];
	char* tmp = fgets(line, (MAX_CAR - 1), fp);
	if (!tmp)
		fError.TreatError(2, -1, "IsAActionFile");
	char* token;

	token = strtok(&line[0], " ");
	if (token == NULL)
		return false;

	token = strtok(NULL, " ");
	if (token == NULL or atoi(token) == 0)
		return false;

	token = strtok(NULL, " ");
	if (token == NULL or atoi(token) == 0)
		return false;

	token = strtok(NULL, " ");
	if (token != NULL and atoi(token) == 0)
		return false;

	token = strtok(NULL, " ");
	if (token != NULL)
		return false;

	fclose(fp);

	return true;
}

//______________________________________________________________________________
int DataGenParameters::GetIndex(const string parName) {
	// Return the order index number

	return (GetIndex(GetLabel(parName)));
}

//_______________________________________________________________________________
const char * DataGenParameters::getIndent(unsigned int numIndents) {
	static const char * pINDENT = "                                      + ";
	static const unsigned int LENGTH = strlen(pINDENT);
	unsigned int n = numIndents * NUM_INDENTS_PER_SPACE;
	if (n > LENGTH)
		n = LENGTH;

	return &pINDENT[LENGTH - n];
}
//_______________________________________________________________________________
// same as getIndent but no "+" at the end
const char * DataGenParameters::getIndentAlt(unsigned int numIndents) {
	static const char * pINDENT = "                                        ";
	static const unsigned int LENGTH = strlen(pINDENT);
	unsigned int n = numIndents * NUM_INDENTS_PER_SPACE;
	if (n > LENGTH)
		n = LENGTH;

	return &pINDENT[LENGTH - n];
}
#ifndef NO_MFMXML
//_______________________________________________________________________________
void DataGenParameters::WriteXML(const char * filename) {
	if (fTiXmlDocument)
		fTiXmlDocument->SaveFile(filename);
}
//_______________________________________________________________________________
// load the named file
TiXmlDocument * DataGenParameters::OpenXmlFile(const char* pFilename) {
	fTiXmlDocument = new TiXmlDocument(pFilename);
	//bool loadOkay = fTiXmlDocument->LoadFile();
	TiXmlDocument doc(pFilename);
	bool loadOkay = doc.LoadFile();
	if (loadOkay) {
		ReadXML((TiXmlNode*) (&doc), 0); // defined later in the tutorial
	} else {
		fError.TreatError(2, 0,
				"Failed to load file  DataScalers::OpenXmlFile : ", pFilename);
		//delete fTiXmlDocument;
		//fTiXmlDocument =NULL;
	}
	return fTiXmlDocument;
}

//_______________________________________________________________________________
int DataGenParameters::dump_attribs_to_stdout(TiXmlElement* pElement,
		unsigned int indent) {
	if (!pElement)
		return 0;

	TiXmlAttribute* pAttrib = pElement->FirstAttribute();
	int i = 0;
	int ival;
	double dval;
	const char* pIndent = getIndent(indent);
	printf("\n");
	while (pAttrib) {
		printf("%s%s: value=[%s]", pIndent, pAttrib->Name(), pAttrib->Value());

		if (pAttrib->QueryIntValue(&ival) == TIXML_SUCCESS)
			printf(" int=%d", ival);
		if (pAttrib->QueryDoubleValue(&dval) == TIXML_SUCCESS)
			printf(" d=%1.1f", dval);
		printf("\n");
		i++;
		pAttrib = pAttrib->Next();
	}
	return i;
}
//_______________________________________________________________________________
void DataGenParameters::dump_to_stdout(TiXmlNode* pParent, unsigned int indent) {
	if (!pParent)
		return;

	TiXmlNode* pChild;
	TiXmlText* pText;
	int t = pParent->Type();
	printf("%s", getIndent(indent));
	int num;

	switch (t) {
	case TiXmlNode::TINYXML_DOCUMENT:
		printf("Document");
		break;
	case TiXmlNode::TINYXML_DECLARATION:
		printf("Declaration");
		break;
	case TiXmlNode::TINYXML_ELEMENT:
		printf("Element [%s]", pParent->Value());
		num = dump_attribs_to_stdout(pParent->ToElement(), indent + 1);
		switch (num) {
		case 0:
			printf(" (No attributes)");
			break;
		case 1:
			printf("%s1 attribute", getIndentAlt(indent));
			break;
		default:
			printf("%s%d attributes", getIndentAlt(indent), num);
			break;
		}
		break;
	case TiXmlNode::TINYXML_TEXT:
		pText = pParent->ToText();
		printf("Text: [%s]", pText->Value());
		break;

	case TiXmlNode::TINYXML_COMMENT:
		if (fVerbose)
			printf("Comment: [%s]", pParent->Value());
		break;
	case TiXmlNode::TINYXML_UNKNOWN:
		fError.TreatError(1, 0, "TiXmlNode Unknown");
		printf("Unknown");
		break;
	default:
		break;
	}
	printf("\n");
	for (pChild = pParent->FirstChild(); pChild != 0; pChild
			= pChild->NextSibling()) {
		dump_to_stdout(pChild, indent + 1);
	}
}
//_______________________________________________________________________________
int DataGenParameters::ReadXML(TiXmlNode* pParent, unsigned int indent) {
	if (!pParent)
		return 0;

	TiXmlNode* pChild;
	TiXmlText* pText;
	int t = pParent->Type();
	printf("%s", getIndent(indent));

	string element;

	switch (t) {
	case TiXmlNode::TINYXML_DOCUMENT:
		printf("Document");
		break;
	case TiXmlNode::TINYXML_DECLARATION:
		printf("Declaration");
		break;
	case TiXmlNode::TINYXML_ELEMENT:
		element = pParent->Value();
		if ((element.compare(XMLTAG_SCALER_DESCRIPTION)) or (element.compare(
				XMLTAG_PARAMETER_DESCRIPTION)))
			ReadXMLspecific(pParent, indent);
		break;
	case TiXmlNode::TINYXML_TEXT:
		pText = pParent->ToText();
		printf("Text: [%s]", pText->Value());
		break;

	case TiXmlNode::TINYXML_COMMENT:
		if (fVerbose)
			printf("Comment: [%s]", pParent->Value());
		break;
	case TiXmlNode::TINYXML_UNKNOWN:
		fError.TreatError(1, 0, "TiXmlNode Unknown");
		printf("Unknown");
		break;
	default:
		break;
	}
	printf("\n");
	for (pChild = pParent->FirstChild(); pChild != 0; pChild
			= pChild->NextSibling()) {
		ReadXML(pChild, indent + 1);
	}
	fNb_Parameters = GetNbElementInList();
	if (fNb_Parameters > 0)
		creatLabelToIndexVectors();
	return (fNb_Parameters);
}
//_______________________________________________________________________________
#endif // NO_MFMXML

