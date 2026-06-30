#include "MFMGeneral.h"

//_______________________________________________________________________________
MFMGeneral::MFMGeneral() {
	fVerbose = 0;
	fNbOfEvent = 0;
	fReadSize = 0;
	fCount_elseframe = 0;
	int i = 0;

	fFrametab = new MFMCommonFrame*[MAXINDICETYPE];
	for (i = 0; i < MAXINDICETYPE; i++)
		fFrametab[i] = NULL;
	fInsideframe = new MFMCommonFrame();
	fFrame = new MFMCommonFrame();  // used to treat unknow frames
	fFrame->SetFrameType(MFM_COMMON_FRAME_TYPE);
	fFrametab[MFM_COMMON_FRAME_TYPE] = fFrame;
	fCoboframe = new MFMCoboFrame();
	fCobofframe = new MFMCoboFrame();
	fCoboframe->SetFrameType(MFM_COBO_FRAME_TYPE);
	fFrametab[MFM_COBO_FRAME_TYPE] = fCoboframe;
	fCobofframe = new MFMCoboFrame();
	fCobofframe->SetFrameType(MFM_COBOF_FRAME_TYPE);
	fFrametab[MFM_COBOF_FRAME_TYPE] = fCobofframe;
	fCobotopoframe = new MFMCoboTopoFrame();
	fFrametab[MFM_COBOT_FRAME_TYPE] = fCobotopoframe;
	fMutframe = new MFMMutantFrame();
	fFrametab[MFM_MUTANT_FRAME_TYPE] = fMutframe;
	fExoframe = new MFMExogamFrame();
	fFrametab[MFM_EXO2_FRAME_TYPE] = fExoframe;
	fOscilloframe = new MFMOscilloFrame();
	fFrametab[MFM_OSCI_FRAME_TYPE] = fOscilloframe;
	fDiamantframe = new MFMDiamantFrame();
	fFrametab[MFM_DIAMANT_FRAME_TYPE] = fDiamantframe;
	fEbyframeN = new MFMEbyedatFrame();
	fEbyframeT = new MFMEbyedatFrame();
	fEbyframeNT = new MFMEbyedatFrame();
	fEbyframeN->SetFrameType(MFM_EBY_EN_FRAME_TYPE);
	fEbyframeT->SetFrameType(MFM_EBY_TS_FRAME_TYPE);
	fEbyframeNT->SetFrameType(MFM_EBY_EN_TS_FRAME_TYPE);
	
	fFrametab[MFM_EBY_EN_FRAME_TYPE] = fEbyframeN;
	fFrametab[MFM_EBY_TS_FRAME_TYPE] = fEbyframeT;
	fFrametab[MFM_EBY_EN_TS_FRAME_TYPE] = fEbyframeNT;
	fMergeframeT = new MFMMergeFrame();
	fMergeframeN = new MFMMergeFrame();
	fMergeframeN->SetFrameType(MFM_MERGE_EN_FRAME_TYPE);
	fFrametab[MFM_MERGE_EN_FRAME_TYPE] = fMergeframeN;
	fMergeframeT->SetFrameType(MFM_MERGE_TS_FRAME_TYPE);
	fFrametab[MFM_MERGE_TS_FRAME_TYPE] = fMergeframeT;
	fScalerframe = new MFMScalerDataFrame();
	fFrametab[MFM_SCALER_DATA_FRAME_TYPE] = fScalerframe;
	fRibfframe = new MFMRibfFrame();
	fFrametab[MFM_RIBF_DATA_FRAME_TYPE] = fRibfframe;
	fHelloframe = new MFMHelloFrame();

	fFrametab[MFM_HELLO_FRAME_TYPE] = fHelloframe;
	// fFrametab  [MFM_HELLO_FRAME_TYPE]  = NULL ;               ///to test nb of unknow frame 
	fDatadescriptionframe = new MFMXmlDataDescriptionFrame();
	fFrametab[MFM_XML_DATA_DESCRIPTION_FRAME_TYPE] = fDatadescriptionframe;
	fHeaderframe = new MFMXmlFileHeaderFrame();
	fFrametab[MFM_XML_FILE_HEADER_FRAME_TYPE] = fHeaderframe;
	fChimeraframe = new MFMChimeraFrame();
	fFrametab[MFM_CHIMERA_DATA_FRAME_TYPE] = fChimeraframe;
	fBoxDiagframe = new MFMBoxDiagFrame();
	fFrametab[MFM_BOX_DIAG_FRAME_TYPE] = fBoxDiagframe;
	fVamosICframe = new MFMVamosICFrame();
	fFrametab[MFM_VAMOSIC_FRAME_TYPE] = fVamosICframe;
	fVamosPDframe = new MFMVamosPDFrame();
	fFrametab[MFM_VAMOSPD_FRAME_TYPE] = fVamosPDframe;
	fVamosTACframe = new MFMVamosTACFrame();
	fFrametab[MFM_VAMOSTAC_FRAME_TYPE] = fVamosTACframe;
	fNedaframe = new MFMNedaFrame();
	fFrametab[MFM_NEDA_FRAME_TYPE] = fNedaframe;
	fNedaCompframe = new MFMNedaCompFrame();
	fFrametab[MFM_NEDACOMP_FRAME_TYPE] = fNedaCompframe;
	fS3BaF2frame = new MFMS3BaF2Frame();
	fFrametab[MFM_S3_BAF2_FRAME_TYPE] = fS3BaF2frame;
	fS3Alphaframe = new MFMS3AlphaFrame();
	fFrametab[MFM_S3_ALPHA_FRAME_TYPE] = fS3Alphaframe;
	fS3Ruthframe = new MFMS3RuthFrame();
	fFrametab[MFM_S3_RUTH_FRAME_TYPE] = fS3Ruthframe;
	fS3eGUNframe = new MFMS3eGUNFrame();
	fFrametab[MFM_S3_EGUN_FRAME_TYPE] = fS3eGUNframe;
	fS3Synchroframe = new MFMS3SynchroFrame();
	fFrametab[MFM_S3_SYNC_FRAME_TYPE] = fS3Synchroframe;
	fReaGenericframe = new MFMReaGenericFrame();
	fFrametab[MFM_REA_GENE_FRAME_TYPE] = fReaGenericframe;
	fReaTraceframe = new MFMReaTraceFrame();
	fFrametab[MFM_REA_TRACE_FRAME_TYPE] = fReaTraceframe;
	fSiriusframe = new MFMSiriusFrame();
	fFrametab[MFM_SIRIUS_FRAME_TYPE] = fSiriusframe;
	fDeflectorframe = new MFMS3DeflectorFrame();
	fFrametab[MFM_S3_DEFLECTOR_FRAME_TYPE] = fDeflectorframe;
	fParisframe = new MFMParisFrame();
	fFrametab[MFM_PARIS_FRAME_TYPE] = fParisframe;
	fMesytecframe = new MFMMesytecFrame();
	fFrametab[MFM_MESYTEC_FRAME_TYPE] = fMesytecframe;
	fFasterframe = new MFMFasterFrame();
	fFrametab[MFM_FASTER_FRAME_TYPE] = fFasterframe;
	fFasterDTSframe = new MFMFasterDTSFrame();
	fFrametab[MFM_FASTERDTS_FRAME_TYPE] = fFasterDTSframe;
	fFaziaframe = new MFMFaziaFrame();
	fFrametab[MFM_FAZIA_DATA_FRAME_TYPE] = fFaziaframe;
	
}
//_______________________________________________________________________________
MFMGeneral::~MFMGeneral() {

	if (fFrame)
		delete (fFrame);
	if (fCoboframe)
		delete (fCoboframe);
	if (fCobofframe)
		delete (fCobofframe);
	if (fCobotopoframe)
		delete (fCobotopoframe);
	if (fExoframe)
		delete (fExoframe);
	if (fEbyframeN)
		delete (fEbyframeN);
	if (fEbyframeT)
		delete (fEbyframeT);
	if (fEbyframeNT)
		delete (fEbyframeNT);
	if (fDiamantframe)
		delete (fDiamantframe);
	if (fInsideframe)
		delete (fInsideframe);
	if (fMergeframeN)
		delete (fMergeframeN);
	if (fMergeframeT)
		delete (fMergeframeT);
	if (fOscilloframe)
		delete (fOscilloframe);
	if (fScalerframe)
		delete (fScalerframe);
	if (fRibfframe)
		delete (fRibfframe);
	if (fMutframe)
		delete (fMutframe);
	if (fHelloframe)
		delete (fHelloframe);
	if (fDatadescriptionframe)
		delete (fDatadescriptionframe);
	if (fHeaderframe)
		delete (fHeaderframe);
	if (fChimeraframe)
		delete (fChimeraframe);
	if (fBoxDiagframe)
		delete (fBoxDiagframe);
	if (fVamosPDframe)
		delete (fVamosPDframe);
	if (fVamosICframe)
		delete (fVamosICframe);
	if (fVamosTACframe)
		delete (fVamosTACframe);
	if (fNedaframe)
		delete (fNedaframe);
	if (fNedaCompframe)
		delete (fNedaCompframe);
	if (fS3BaF2frame)
		delete (fS3BaF2frame);
	if (fS3Alphaframe)
		delete (fS3Alphaframe);
	if (fS3Ruthframe)
		delete (fS3Ruthframe);
	if (fS3eGUNframe)
		delete (fS3eGUNframe);
	if (fS3Synchroframe)
		delete (fS3Synchroframe);
	if (fReaGenericframe)
		delete (fReaGenericframe);
	if (fReaTraceframe)
		delete (fReaTraceframe);
	if (fSiriusframe)
		delete (fSiriusframe);
	if (fDeflectorframe)
		delete (fDeflectorframe);
	if (fParisframe)
		delete (fParisframe);
	if (fFaziaframe)
		delete (fFaziaframe);
	if (fFasterframe)
		delete (fFasterframe);
	if (fFasterDTSframe)
		delete (fFasterDTSframe);
	if (fMesytecframe)
		delete (fMesytecframe);
	if (fFrametab)
		delete[] fFrametab;
}
//_______________________________________________________________________________
void MFMGeneral::InitStat() {
	// initialisation of statistic
	int i;
	for (i = 0; i < MAXINDICETYPE; i++) {
	
		if (fFrametab[i] != NULL){
			fFrametab[i]->InitStat();
			}
	}
}

//_______________________________________________________________________________
int MFMGeneral::ReadFrame(char *buff, int dumpsize, int buffsize) {

// Read MFM in a buffer
	int status = 0;
	char *pt = buff;
	fReadSize = 0;
	fNbOfEvent = 0;
	UtilVector_c *vector = new UtilVector_c(MFM_BLOB_HEADER_SIZE); // min size =8              
	int framesize = 0;
	bool display = false;
	while (true) {
		framesize = fFrame->ReadInMem(&pt);
		//
		//fFrame->HeaderDisplay();
		//cout << buffsize<< "  "<<fReadSize+framesize<<" | ";
		if (framesize <= 0)
			break;
		fReadSize += framesize;
		ReadUserFrame (fFrame, dumpsize,display,fNbOfEvent);
		fNbOfEvent++;
		if (fReadSize >= buffsize)
			break;
	}
	//cout<< " |||"<< buffsize<< "  "<<fReadSize<<endl;
	if (vector)
		delete (vector);
	return fReadSize;

}

//_______________________________________________________________________________
void MFMGeneral::ReadUserFrame(MFMCommonFrame *commonframe, int dumpsize,bool display,int noframe) {
	//Read frame and extract informations for statistics

	commonframe->SetAttributs();
	int type = commonframe->GetFrameType();
	int headersize = commonframe->GetHeaderSize();

	if (type == MFM_MERGE_EN_FRAME_TYPE or type == MFM_MERGE_TS_FRAME_TYPE)
		ReadMergeFrame(commonframe, dumpsize, display, noframe);
	else if (fFrametab[type] != NULL) {
		fFrametab[type]->ReadAttributsExtractFrame(fVerbose, dumpsize, display,
				noframe, commonframe->GetPointHeader());
	} else {
		fFrametab[MFM_COMMON_FRAME_TYPE]->ReadAttributsExtractFrame(fVerbose,
				dumpsize, display, noframe, commonframe->GetPointHeader());
	}
}

//_______________________________________________________________________________________________________________________
void MFMGeneral::ReadMergeFrame(MFMCommonFrame *commonframe, int dumpsize,
		bool display, int noframe) {

	//Read frame and extract informations for statistics in summary mode
	//Must be used for Merge frames

	int type = commonframe->GetFrameType();
	fFrametab[type]->SetAttributs(commonframe->GetPointHeader());
	int nbinsideframe = 0;
	nbinsideframe = ((MFMMergeFrame*) fFrametab[type])->GetNbItems();
	fFrametab[type]->FillStat();
	fFrametab[type]->TestUserPointer(noframe, fVerbose);
	int framesize = ((MFMMergeFrame*) fFrametab[type])->GetFrameSize();
	if (display)
		((MFMMergeFrame*) fFrametab[type])->ExtractInfoFrame(fVerbose,
				dumpsize, noframe);
	((MFMMergeFrame*) fFrametab[type])->ResetReadInMem();
	for (int i = 0; i < nbinsideframe; i++) {
		((MFMMergeFrame*) fFrametab[type])->ReadInFrame(fInsideframe);
		fInsideframe->FillStat();
		ReadUserFrame (fInsideframe,dumpsize, display,noframe);
	}
}
//_______________________________________________________________________________________________________________________

void MFMGeneral::PrintStatCount() const {
	// print statistics in summary mode
	cout << GetStatCount();
}

//_______________________________________________________________________________________________________________________
string MFMGeneral::GetStatCount() const {
    // Get Statistics information and return it in a string
	string display("");
	stringstream ss("");
	ss << "--MFM Frames Counts Summary -- :" << endl;
	int nb = 0;
	unsigned long long int nbframe = 0;
	unsigned long long int nbframeT = 0;
	int maxperline = 5;
	int i = 0;
	for (i = 0; i < MAXINDICETYPE; i++) {
		if (fFrametab[i] != NULL) {
			if (fFrametab[i]->GetCountFrame() != 0) {
				nbframe = fFrametab[i]->GetCountFrame();
				ss << " " << fFrametab[i]->GetTypeShortText() << " : "
						<< fFrametab[i]->GetCountFrame() << " |";
				nb++;
				nbframeT += nbframe;
			}
			if (nb >= maxperline) {
				ss << endl;
				nb = 0;
			}
		}
	}
	ss << endl;
	ss << " Total Frames = " << nbframeT << endl;
	display = ss.str();
	return display;
}
//_______________________________________________________________________________________________________________________
void MFMGeneral::Statistics(bool writeorread) {
// Print full statistics
	int i = 0;
	if (fVerbose > 0) {
		cout << endl;
		cout << "------------------------------------------------" << endl;
		if (writeorread)
			cout << "|              WRITE STATISTICS                |" << endl;
		else
			cout << "|              READ STATISTICS                 |" << endl;
		cout << "------------------------------------------------" << dec
				<< endl;
		if (GetFrameWithType(MFM_XML_FILE_HEADER_FRAME_TYPE) != NULL)
			if (GetFrameWithType(MFM_XML_FILE_HEADER_FRAME_TYPE)->GetCountFrame()
					!= 0) {
				GetFrameWithType(MFM_XML_FILE_HEADER_FRAME_TYPE)->PrintStat(
						GetFrameWithType(MFM_XML_FILE_HEADER_FRAME_TYPE)->GetTypeLongText());
				cout << "----------------------------------------------"
						<< endl;
			}
		if (GetFrameWithType(MFM_XML_DATA_DESCRIPTION_FRAME_TYPE) != NULL)
			if (GetFrameWithType(MFM_XML_DATA_DESCRIPTION_FRAME_TYPE)->GetCountFrame()
					!= 0) {
				GetFrameWithType(MFM_XML_DATA_DESCRIPTION_FRAME_TYPE)->PrintStat(
						GetFrameWithType(MFM_XML_DATA_DESCRIPTION_FRAME_TYPE)->GetTypeLongText());
				cout << "----------------------------------------------"
						<< endl;
			}

		for (i = 0; i < MAXINDICETYPE; i++) {
			if (GetFrameWithType(i) != NULL) {
				if ((GetFrameWithType(i)->GetCountFrame() != 0)
						and (i != MFM_XML_FILE_HEADER_FRAME_TYPE)
						and (i != MFM_XML_DATA_DESCRIPTION_FRAME_TYPE)) {
					GetFrameWithType(i)->PrintStat(
							GetFrameWithType(i)->GetTypeLongText());
					cout << "----------------------------------------------"
							<< endl;
				}

			}
		}
		return;
	}
}

//_______________________________________________________________________________________________________________________
