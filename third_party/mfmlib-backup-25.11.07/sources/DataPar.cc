#include <stdio.h>
#include "DataPar.h"

using namespace std;
//_______________________________________________________________________________
DataPar::DataPar(string name, const int32_t label, int32_t nbbits, int depth,
		int sizeinmemory) {
	fName = name;
	fLabel = label;
	fNbits = nbbits;
	fDepth = depth;
	fSizeInMemory = sizeinmemory;

}
//_______________________________________________________________________________
DataPar::DataPar(DataPar *datapar) {
	fName = datapar->fName;
	fLabel = datapar->fLabel;
	fNbits = datapar->fNbits;
	fDepth = datapar->fDepth;
	fSizeInMemory = datapar->fSizeInMemory;

}
//_______________________________________________________________________________
string DataPar::DumpDataPar() {
	stringstream ss;
	string display("");
	ss << fName;
	ss << "  Label : " << fLabel;
	ss << "  Depth : " << fDepth;
	display = ss.str();
	return display;

}
//_______________________________________________________________________________
DataPar::DataPar() {
	fName = "";
	fLabel = 0;
	fNbits = 0;
	fDepth = 0;
	fSizeInMemory = 0;

}
//_______________________________________________________________________________
int DataPar::Label(void) const {
	return (fLabel);
}
//_______________________________________________________________________________
void DataPar::SetLabel(int label) {
	fLabel = label;
}
//_______________________________________________________________________________
void DataPar::SetName(string name) {
	fName = name;
}
//_______________________________________________________________________________
int DataPar::Nbits(void) const {
	return (fNbits);
}
//_______________________________________________________________________________
void DataPar::SetNbits(int32_t bits) {
	fNbits = bits;
}
//_______________________________________________________________________________
void DataPar::SetDepth(int32_t bits) {
	fDepth = bits;
}
//_______________________________________________________________________________
int DataPar::Depth(void) const {
	return (fDepth);
}
//_______________________________________________________________________________
string DataPar::Name(void) const {
	return (fName);
}

//_______________________________________________________________________________
const char * DataPar::getIndent(unsigned int numIndents) {
	static const char * pINDENT = "                                      + ";
	static const unsigned int LENGTH = strlen(pINDENT);
	unsigned int n = numIndents * NUM_INDENTS_PER_SPACE;
	if (n > LENGTH)
		n = LENGTH;

	return &pINDENT[LENGTH - n];
}
//_______________________________________________________________________________
// same as getIndent but no "+" at the end
const char * DataPar::getIndentAlt(unsigned int numIndents) {
	static const char * pINDENT = "                                        ";
	static const unsigned int LENGTH = strlen(pINDENT);
	unsigned int n = numIndents * NUM_INDENTS_PER_SPACE;
	if (n > LENGTH)
		n = LENGTH;

	return &pINDENT[LENGTH - n];
}
#ifndef NO_MFMXML
//_______________________________________________________________________________
void DataPar::ReadXML(TiXmlNode* pParent, unsigned int indent) {
	printf("debugDatapar::ReadXML \n");
	if (!pParent)
		return;
	printf("debugDatapar::ReadXML \n");
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
		printf("Element [%s]", element.data());
		pChild = pParent->FirstChild();
		pText = pChild->ToText();
		if (pText == 0) {
			printf("PB pText==NULL\n");
			break;
		}
		printf("Text: [%s]", pText->Value());
		if (element.compare(XMLTAG_nbbits) == 0) {
			SetNbits(atoi(pChild->Value()));
		}
		if (element.compare(XMLTAG_name) == 0) {
			SetName(pChild->Value());
		}
		if (element.compare(XMLTAG_depth) == 0) {
			SetDepth(atoi(pChild->Value()));
		}
		if (element.compare(XMLTAG_label) == 0) {
			SetLabel(atoi(pChild->Value()));
		}
		break;

	case TiXmlNode::TINYXML_TEXT:
		pText = pParent->ToText();
		printf("Text: [%s]", pText->Value());
		break;

	case TiXmlNode::TINYXML_COMMENT:
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
}

//_______________________________________________________________________________
void DataPar::CreatXML(TiXmlNode* pParent, DataPar * defautpar) {
	string tempo;
	stringstream sstempo("");
	printf("debugDatapar::CreatXML \n");
	if (!pParent)
		return;
	printf("debugDatapar::CreatXML \n");

	TiXmlElement* ElementChannel = new TiXmlElement( XMLTAG_channel );
	pParent->LinkEndChild(ElementChannel);
	TiXmlElement* ElementName = new TiXmlElement( XMLTAG_name );
	ElementChannel->LinkEndChild(ElementName);
	TiXmlText* Textname = new TiXmlText(fName.data());
	ElementName->LinkEndChild(Textname);
	sstempo.str("");
	sstempo << fLabel;
	tempo=sstempo.str();

	TiXmlElement* ElementLabel = new TiXmlElement(XMLTAG_label);
	ElementChannel->LinkEndChild(ElementLabel);
	TiXmlText* Textlabel= new TiXmlText(tempo.data());
	ElementLabel->LinkEndChild(Textlabel);

	if (defautpar->fDepth != fDepth) {
		sstempo.str("");
		sstempo << fDepth;
		tempo=sstempo.str();
		TiXmlElement* ElementDepth = new TiXmlElement(XMLTAG_depth);
		ElementDepth->LinkEndChild(ElementChannel);
		TiXmlText* Textpath = new TiXmlText(tempo.data());
		ElementDepth->LinkEndChild(Textpath);

	}
	if (defautpar->fNbits != fNbits) {
		sstempo.str("");
		sstempo << fNbits;
		tempo= sstempo.str();
		TiXmlElement* Elementnbbits= new TiXmlElement(XMLTAG_nbbits);
		Elementnbbits->LinkEndChild(ElementChannel);
		TiXmlText* Textnbits = new TiXmlText(tempo.data());
	    Elementnbbits->LinkEndChild(Textnbits);
	}

}

//_______________________________________________________________________________
void DataPar::CreatXMLdefault(TiXmlNode* pParent) {
	string tempo;
		stringstream sstempo("");
	   TiXmlElement* Elementnbbits= new TiXmlElement(XMLTAG_nbbits);
	   pParent->LinkEndChild(Elementnbbits);
		sstempo.str("");
		sstempo << fNbits;
		tempo=sstempo.str();
		TiXmlText* TextNbits = new TiXmlText(tempo.data());
		Elementnbbits->LinkEndChild(TextNbits);

		TiXmlElement* Elementdepth= new TiXmlElement(XMLTAG_depth);
		pParent->LinkEndChild(Elementdepth);


		sstempo.str("");
				sstempo << fDepth;
				tempo=sstempo.str();
				TiXmlText* TextDepth = new TiXmlText(tempo.data());
				Elementdepth->LinkEndChild(TextDepth);

}

#endif
