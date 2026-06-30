// Author: $Author: patois $ modified by Legeard
/***************************************************************************
 //                        DataScalers.cpp  -  Scalers name handling
 //////////////////////////////////////////////////////////////////////////
 //
 //
 // DataScalers
 //
 // Handle Scalers names and related offsets in the event buffer.
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

#include "DataScalers.h"
#include "stdio.h"
#include "stdlib.h"

//______________________________________________________________________________
DataScalers::DataScalers(void) {
	//Default constructor. Don't create anything yet.

	// when we'll delete the tree itself
	fLabelToIndex = NULL;
}

//______________________________________________________________________________
DataScalers::~DataScalers(void) {
	if (fLabelToIndex) {
		delete[] fLabelToIndex;
		fLabelToIndex = NULL;
	}
	fList.clear();
}

//_____________________________________________________________________________

int DataScalers::FillFromMfmFrame(const char *buffParam) {
	// Data from the Scaler buffer (buffParam) have been read from disk
	//and parsed by this method to create the list of labels.
	//We work on a copy of buffParam because strtok_r modify his Scalers

	char workString[strlen(buffParam)+1];
	strcpy(workString, buffParam);

	int * param = NULL;
	// param = get param frm
	while (param != NULL) {
		// get name , get label dans scrute dans XML dans la frame

		//		DataScal *scal = new DataScal(name, label);
		//	fList.push_back(*scal);
	}
	//	fNb_Scaler = fList.size();
	creatLabelToIndexVectors();
	return (fNb_Scaler);
}
//______________________________________________________________________________
int DataScalers::Fill(const char *buffParam) {
	return 0;
}
//______________________________________________________________________________
int DataScalers::FillFromXmlFile(const char *xmlfile) {
	// Data from the Scaler buffet (buffParam) have been read from disk and is
	// parsed by this routine who create the list of labels and corresponding
	// label number

	//	if (OpenXmlFile(const char* pFilename))
	//	dump_to_stdout(&fTiXmlDocument);

	// Get Frame of data desctription.
	//  Fill(const char *buffParam);

	return (fNb_Scaler);

}

//______________________________________________________________________________
DataScal* DataScalers::FindScaleObject(const string scalName) {

	DataScal *scal = NULL;
	for (unsigned int i = 0; i < fList.size(); i++) {
		scal = (&fList[i]);

		int comp = ((scal->Name()).compare(scalName));
		if (comp == 0) {
			break;
		} else {
			scal = NULL;
		}
	}
	return scal;
}
//______________________________________________________________________________
int DataScalers::GetLabel(const string scalName) {
	// Return the label number corresponding to a text label.
	DataScal *scal = FindScaleObject(scalName);
	string message;

	if (!scal) {
		message = "Scaler  ";
		message += scalName;
		message += " not found! (DataScalers::GetLabel)";
		fError.TreatError(2, -1, &message);
		return (-1);
	}
	return (scal->Label());
}

//______________________________________________________________________________
int DataScalers::GetLabel(const int index) {
	// Return the Label number corresponding to a text label.
	DataScal *par = NULL;
	par = (DataScal*) (&fList[(unsigned int) index]);
	string message;

	int retour = 0;
	if (!par) {
		message = "Scaler  ";
		message += index;
		message += " not found! (DataScalers::GetLabel(const int index))";
		fError.TreatError(2, -1, &message);
		return (-1);

	}
	retour = par->Label();
	return (retour);
}

//______________________________________________________________________________
int DataScalers::GetNbitsFromLabel(const int label) {
	// Return  number of bit corresponding to index.
	//return (GetNbitsFromIndex(GetIndex(label)));
	return 0;
}

//______________________________________________________________________________
const char* DataScalers::GetParNameFromIndex(const int index) {
	// Return the text label name corresponding to an index.

	DataScal *par = (DataScal*) &(fList[index]);
	string message;

	if (!par) {
		message = "Scaler order index  ";
		message += index;
		message += " not found! (DataScalers::GetParName) ";
		fError.TreatError(2, -1, &message);

		return ("");
	}
	return ((par->Name()).data());
}
//______________________________________________________________________________
const char* DataScalers::GetParNameFromLabel(const int label) {
	// Return the text label name corresponding to an index.
	return (GetParNameFromIndex(GetIndex(label)));
}
//______________________________________________________________________________
void DataScalers::creatLabelToIndexVectors() {
	// Creation of LabelToIndex Vector and fUserLabelToIndex Vector
	string message;
	DataScal *par;
	int i_label;
	uint32_t index = 0;
	int max_label = 0;
	int current_label;

	// research of Max value of label
	max_label = GetMaxLabel();

	message = "Error in creatLabelToIndexVector max_label =0";
	if (max_label == 0) {
		fError.TreatError(2, -1, &message);
		return;
	}
	fLabelToIndex = new uint32_t[max_label + 1];
	fLabelToUserLabel = new uint32_t[max_label + 1];
	for (i_label = 0; i_label <= max_label; i_label++) {
		fLabelToIndex[i_label] = 0;
		fLabelToUserLabel[i_label] = 0;
	}

	for (index = 0; (int) index < fNb_Scaler; index++) {
		par = (DataScal*) &(fList[(int) index]);
		current_label = par->Label();
		fLabelToIndex[current_label] = (uint32_t) index;
	}

}

//______________________________________________________________________________
int* DataScalers::creatLabelToExist(int* max_label) {
	// Creation of LabelToIndex Int Vector
	// use for example for statistics
	//

	string message;
	DataScal *par;
	int i_label;
	int index = 0;
	*max_label = 0;
	int current_label;
	int* LabelToIndex;

	*max_label = GetMaxLabel();
	if (*max_label == 0) {
		fError.TreatError(2, -1, "Error in creatLabelToIndexVectorInt");
		return (NULL);
	}
	LabelToIndex = new int[(*max_label) + 1];
	for (i_label = 0; i_label <= *max_label; i_label++)
		LabelToIndex[i_label] = 0;
	for (index = 0; index < fNb_Scaler; index++) {
		par = (DataScal*) &(fList[(int) index]);
		current_label = par->Label();
		LabelToIndex[current_label] = 1;
	}
	return (LabelToIndex);
}
//______________________________________________________________________________
int DataScalers::GetIndex(const int label) {
	// Return the order index number

	return (fLabelToIndex[label]);
}
//______________________________________________________________________________
int DataScalers::GetMaxLabel() {
	// Return the order index number
	int i, current_label, indexmax = fList.size();
	DataScal *par;
	int max_label = 0;
	for (i = 0; i < indexmax; i++) {
		par = (DataScal*) &(fList[i]);
		if (!par) {
			break;
		}
		current_label = (int) par->Label();
		if (max_label < current_label)
			max_label = current_label;

	}
	return (max_label);
}
//______________________________________________________________________________
void DataScalers::DumpListOfNames() {
	// Dump Scaler names and infos.

	int presentation = 0;
	cout << "--------- DUMPING ScalerS  ----------------------------" << endl;

	for (int i = 0; i < (int) (fList.size()); i++) { // WARNING : index start at one, the first value is boggus
		cout << "Index order :" << i << " " << GetParNameFromIndex(i)
				<< "  Index : " << GetIndex(GetParNameFromIndex(i)) << "  "
				<< "  Label : " << GetLabel(GetParNameFromIndex(i)) << "  ";
		//					<< "  Index : " << GetIndex(i) <<"  ";
		if (presentation++ == 0) {
			cout << "\n";
			presentation = 0;
		}
	}
	if (presentation != 0)
		cout << "\n";
	cout << "--------- END           ----------------------------" << endl;
}
/*
 //______________________________________________________________________________
 int DataScalers::GetUserLabelFromLabel(const int label) {
 // Return the order index number

 return (fLabelToUserLabel[label]);
 }
 */
//______________________________________________________________________________
void DataScalers::DumpList() {
	// Dump Scalers

	for (int i = 0; i < fNb_Scaler; i++) {
		cout << GetParNameFromIndex((int) i) << "   Index = " << (int) i
				<< "  Label :" << (int) GetLabel((int) i) << "\n";
	}
}

//______________________________________________________________________________
int DataScalers::GetIndex(const string parName) {
	// Return the order index number

	return (GetIndex(GetLabel(parName)));
}
//______________________________________________________________________________
void DataScalers::TestList() {
	char *name;
	int label;
	cout << " Test Scaler list" << endl;
	for (int i = 0; i < fNb_Scaler; i++) {

		name = (char*) GetParNameFromIndex(i);
		label = GetLabel(i);

		cout
				<< "------------------------------------------------------------------\n";
		cout << "From Index : " << i << " GetScalNameFromIndex(index)=" << name
				<< " GetLabel(index)=" << label << "\n";
		cout << "From Name : " << name << " GetLabel(Name)=" << GetLabel(name)
				<< " GetIndex(name)=" << GetIndex(name) << "\n";
		cout << "From Label : " << label << " GetParNameFromLabel()="
				<< GetParNameFromLabel(label) << " GetIndex(label)="
				<< GetIndex(name) << "\n";
	}
}
//______________________________________________________________________________
void DataScalers::FillLabelToIndexVectors() {

}
//______________________________________________________________________________
void DataScalers::FillLabelToExist() {

}
//_______________________________________________________________________________
int DataScalers::GetNbElementInList() {
	return fList.size();
}
#ifndef NO_MFMXML
//_______________________________________________________________________________
void DataScalers::ReadXMLspecific(TiXmlNode* pParent, unsigned int indent) {
	TiXmlNode* pChild;
	TiXmlText* pText;
	int t = pParent->Type();
	printf("%s", getIndent(indent));
	string element;
	element = pParent->Value();


		switch (t) {
		case TiXmlNode::TINYXML_DOCUMENT:
			printf("Document");
			break;
		case TiXmlNode::TINYXML_DECLARATION:
			printf("Declaration");
			break;
		case TiXmlNode::TINYXML_ELEMENT:
			if ((element.compare(XMLTAG_cpu))){
				pChild = pParent->FirstChild();
			ReadXMLcpu(pChild, indent + 1);
				ReadXMLcpu(pParent, indent);
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
}
//_______________________________________________________________________________
void DataScalers::ReadXMLcpu(TiXmlNode* pParent, unsigned int indent) {
	TiXmlNode* pChild;
		TiXmlText* pText;
		int t = pParent->Type();
		printf("%s", getIndent(indent));
		string element;
		element = pParent->Value();
			switch (t) {
			case TiXmlNode::TINYXML_DOCUMENT:
				printf("Document");
				break;
			case TiXmlNode::TINYXML_DECLARATION:
				printf("Declaration");
				break;
			case TiXmlNode::TINYXML_ELEMENT:
				if ((element.compare(XMLTAG_cpu))){
					pChild = pParent->FirstChild();
				ReadXMLcpu(pChild, indent + 1);
					ReadXMLcpu(pParent, indent);
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
}
//_______________________________________________________________________________
void DataScalers::ReadXMLvoie(TiXmlNode* pParent, unsigned int indent) {
	TiXmlNode* pChild;
	TiXmlText* pText;
	int t = pParent->Type();
	printf("%s", getIndent(indent));
	string element;
	element = pParent->Value();
	printf("Element [%s]", element.data());
	if (element.compare(XMLTAG_SCALER_DESCRIPTION) == 0){
		printf("---normalement on ne le détécte pas ici \n");
	}
		printf("---scaler_description ok\n");

	if (element.compare(XMLTAG_module) == 0) {

		ReadXMLspecific(pParent, indent + 1);
		printf("---cpu ok\n");
	}
	if (element.compare(XMLTAG_channelscal) == 0) {
		DataScal *currentscal = new DataScal();
		currentscal->ReadXML(pParent, indent + 1);
		fList.push_back(currentscal);
		printf("---channel ok\n");
	}

}


//______________________________________________________________________________
void DataScalers::CreatXML() {
}
#endif
//______________________________________________________________________________
