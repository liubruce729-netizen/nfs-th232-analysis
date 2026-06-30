#include <stdio.h>
#include "DataScal.h"

using namespace std;

DataScal::DataScal(string name, uint32_t label, uint64_t count ,
			uint64_t frequency , int32_t status , uint64_t tics ,
			int32_t acqstatus ) {

	fLabel = label;
	fCount = count;
	fFrequency = frequency;
	fStatus = status;
	fTics = tics;
	fAcqStatus = acqstatus;
	fName = name;
}


//_______________________________________________________________________________
DataScal::DataScal(DataScal *DataScal) {

	fLabel = DataScal->fLabel;
	fCount = DataScal->fCount;
	fFrequency = DataScal->fFrequency;
	fStatus = DataScal->fStatus;
	fTics = DataScal->fTics;
	fAcqStatus = DataScal->fAcqStatus;
	fName = DataScal->fName;

}

//_______________________________________________________________________________
DataScal::DataScal() {

	fName = "";
	fLabel = 0;
	fCount= 0;
	fFrequency= 0;
	fStatus= 0;
	fTics= 0;
	fAcqStatus= 0;
}
//_______________________________________________________________________________
uint32_t DataScal::Label(void) const {
	return (fLabel);
}
//_______________________________________________________________________________
void DataScal::SetLabel(uint32_t label) {
	fLabel = label;
}
//_______________________________________________________________________________
uint64_t DataScal::Count(void) const {
	return (fCount);
}
//_______________________________________________________________________________
void DataScal::SetCount(uint64_t count) {
	fCount = count;
}
//_______________________________________________________________________________
uint64_t DataScal::Frequency(void) const {
	return (fFrequency);
}
//_______________________________________________________________________________
void DataScal::SetFrequency(uint64_t f) {
	fFrequency = f;
}
//_______________________________________________________________________________
int32_t DataScal::Status(void) const {
	return (fStatus);
}
//_______________________________________________________________________________
void DataScal::SetStatus(int32_t status) {
	fStatus = status;
}
//_______________________________________________________________________________
uint64_t DataScal::Tics(void) const {
	return (fTics);
}
//_______________________________________________________________________________
void DataScal::SetTics(uint64_t tics) {
	fTics = tics;
}
//_______________________________________________________________________________
int32_t DataScal::AcqStatus(void) const {
	return (fAcqStatus);
}
//_______________________________________________________________________________
void DataScal::SetAcqStatus(int32_t acqstatus) {
	fAcqStatus = acqstatus;
}
//_______________________________________________________________________________
string DataScal::Name(void) const {
	return (fName);
}
//_______________________________________________________________________________
void DataScal::SetName(string name) {
	fName = name;
}

//_______________________________________________________________________________
string DataScal::DumpDataScal() {
	stringstream ss;
	string display("");
	ss << fName;
	ss << "  Count : " << fCount;
	ss << "  Label : " << fLabel;
	ss << "  Frequency : " << fFrequency;
	ss << "  Status : " << fStatus;
	ss << "  Tics : " << fTics;
	ss << "  AcqStatus : " << fAcqStatus;
	display = ss.str();
	return display;

}
# ifndef NO_MFMXML
//_______________________________________________________________________________
void DataScal::ReadXML(TiXmlNode* pParent, unsigned int indent) {
	printf("debugDataScal::ReadXML \n");
	if (!pParent)
		return;
	printf("debugDataScal::ReadXML \n");
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

		if (element.compare(XMLTAG_nom) == 0) {
			SetName(pChild->Value());
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
void DataScal::CreatXML(TiXmlNode* pParent, DataScal * defautpar) {
	string tempo;
	stringstream sstempo("");
	printf("debugDataScal::CreatXML \n");
	if (!pParent)
		return;
	printf("debugDataScal::CreatXML \n");

	TiXmlElement* ElementChannel = new TiXmlElement(XMLTAG_channelscal );
	pParent->LinkEndChild(ElementChannel);
	TiXmlElement* ElementName = new TiXmlElement(XMLTAG_nom);
	ElementChannel->LinkEndChild(ElementName);
	TiXmlText* Textname = new TiXmlText(fName.data());
	ElementName->LinkEndChild(Textname);
	sstempo.str("");
	sstempo << fLabel;
	tempo = sstempo.str();
	TiXmlElement* ElementLabel = new TiXmlElement(XMLTAG_label);
	ElementChannel->LinkEndChild(ElementLabel);
	TiXmlText* Textlabel = new TiXmlText(tempo.data());
	ElementLabel->LinkEndChild(Textlabel);

}
#endif
//_______________________________________________________________________________
const char * DataScal::getIndent(unsigned int numIndents) {
	static const char * pINDENT = "                                      + ";
	static const unsigned int LENGTH = strlen(pINDENT);
	unsigned int n = numIndents * NUM_INDENTS_PER_SPACE;
	if (n > LENGTH)
		n = LENGTH;

	return &pINDENT[LENGTH - n];
}
//_______________________________________________________________________________
