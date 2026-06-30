/**
 \mainpage MFM Library general information
 *
 **\verbinclude "README.md"
 * <a href="README.md"></a>, 
 **/
#include <iomanip> 
#include <iostream>
using namespace std;
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#ifndef NO_MFMXML
#include <tinyxml.h>
#endif
#include "MFMAllFrames.h"
#include "DataParameters.h"
#include "DataScalers.h"
#include "MError.h"
#include "Cobo.h"
#include <cstring>
#include <cstdlib>
#include <time.h>
#include "MFMlib.in.h"
#include "ArgInterpretor.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include "Version.h"
#include <cmath>
#include <typeinfo>

#define  TYPE_FORMAT_MIX         0xFFA0
#define  TYPE_FORMAT_MERG_EBYEN  0xFFA1
#define  TYPE_FORMAT_MERG_EBYTS  0xFFA2
#define  TYPE_FORMAT_MERG_COBO   0xFFA3


enum WorkMode {
	WRITE_FRAMES        = 0x00,
	READ_FRAMES         = 0x01,
	EXTRACT_FRAMES      = 0x02,
	EXTRACT_PATTERN     = 0x03,
	GENERATE_PARA_FILES = 0x04,
	TEST_MODE           = 0x05,
	REMOVE_BAD_FRAMES   = 0x06,
	ARRANGE_FRAMES      = 0x07,
	CREAT_CHC_PAR       = 0x08
};

MFMGeneral     *fGeneral;
MFMCommonFrame *fFrame;
MFMCommonFrame *fInsideframe;
MFMMergeFrame  *fMergeframe;
MFMMergeFrame  *fMergeframe2;

DataParameters *fDataPara;
DataScalers *fDataScal;

WorkMode fWorkMode;
MError Error;
int fDumpsize;
int fSubDumpsize;
int fMaxdump;
int fCount_elseframe;
int fNbFrames; // nb of frame  to generate
int fNbFramesRead; // nb of frame  to read
int fNbSubFrames;
int fNbFramesStart; // no of frame to start the dump if dump is asked
int fNbFramesDump; // no of frame to the dump if dump is asked
uint32_t fEventNumber;
int fNbItems;
int fVerbose;
int fFormat;
int fNbOfEvent;
int fExtDumpSize;
const char *fDeffilename = "mfmfile.dat";
long long fPtInFile = 0;

char fExtFilename[255];
char fActionFilename[255];
char fCurrentFilename[255];
int fExpSize;
int fExpType;
int fExpMaxSize = 2500000;

bool fExtractMode;
bool fActionFile;
bool fPatternMode;

int fExpEndian;
char fPattern[255];

bool InfoCondition();

//int ReadUserFrame(MFMCommonFrame *commonframe);
//int ReadMergeFrame(MFMCommonFrame *commonframe, int fDumpsize, bool display,
//		int noframe);

void WriteUserFrame(int flun, int format, int fNbFrames, int fNbSubFrames = 5);
void WriteMergeFrame(int flun, int fNbFrames, int type, int fNbSubFrames = 5);
void deletefunction();
void PrintQuestion();

void Statistics(bool writeorread = false);
void Help(char *processname);
void Announce(char *process);

int ReadInBadFile(FILE *fLun, char **vector, int *vectorsize,
		int *readframesize, int readsize, long long int filesize);
void ClearVector(char *vector, int vectorsize);

void ReadDefaultFrame(MFMCommonFrame *commonframe, int readsize,
		long long filesize);

void Help();
void HelpList();

bool Test(int verbose);
void WriteFrame();
void ReadFrame();
void ExtractFrame();
void ExtractPattern();
void ParameterFile();
void RemoveBadFrames();
void ArrangeFrames();
void CreatCHCParFile();
int TestMergeFrameInFile(char *vector, int expMaxSize);

//_________________________________________________________________________________________
bool HelpAndExit() {
	Help();
	exit(0);
}

//_________________________________________________________________________________________
int main(int argc, char **argv) {

	MError Error;

	bool mybool   = false;

	fDumpsize      = 16;
	fSubDumpsize   = 16;
	fCount_elseframe = 0;
	fMaxdump       = 128;
	fFormat        = -1;
	fNbFrames      = 20;
	fNbFramesRead  = -1;
	fNbSubFrames   = 5;
	fNbFramesStart = 0;
	fNbFramesDump  = 0;
	fVerbose       = 5;
	fNbItems       = -1; // if -1 ,  we leave frame to get its defined value 

	fExpSize       = -1;
	fExpType       = -1;
	fExpEndian     = -1;
	fExpMaxSize    = 2500000;
	fExtractMode   = false;
	fActionFile    = false;
	fExtDumpSize   = 0;
	fWorkMode      = READ_FRAMES;

	strcpy(fExtFilename, "");
	strcpy(fCurrentFilename, fDeffilename);

	fDataPara = new DataParameters();
	fDataScal = new DataScalers();

	fGeneral = new MFMGeneral();
	fFrame = new MFMCommonFrame();
	fInsideframe = new MFMCommonFrame();
	fMergeframe = new MFMMergeFrame();
	fMergeframe2 = new MFMMergeFrame();
	uint16_t   tabcard[1];    // this is given in example 
	tabcard[0] = 112;         // this is given in example 

	fGeneral->InitStat();
	
	ArgInterpretor ArgInt(argc, argv);

	while (true) {
		if (ArgInt.GetNextCommand() == 0) {
			if (ArgInt.TestNoCommand()) {
				HelpAndExit();
			}
			break;
		}

		ArgInt.Treat2Commands((char*) "-n", (char*) "--number",
				(char*) "number of frames", &fNbFrames);
		ArgInt.Treat2Commands((char*) "-v", (char*) "--verbose",
				(char*) "level of verbose", &fVerbose);
		ArgInt.Treat2Commands((char*) "-nr", (char*) "--numberread",
				(char*) "number of frames to read", &fNbFramesRead);
		ArgInt.Treat2Commands((char*) "-itn", (char*) "--itemnumber",
				(char*) "number of items", &fNbItems);
		ArgInt.Treat2Commands((char*) "-nt", (char*) "--numberstart",
				(char*) "start frames number", &fNbFramesStart);
		ArgInt.Treat2Commands((char*) "-nd", (char*) "--numberdump",
				(char*) "number of frame to dump", &fNbFramesDump);
		ArgInt.Treat2Commands((char*) "-sn", (char*) "--subnumber",
				(char*) "number of sub frames", &fNbSubFrames);
		ArgInt.Treat2Commands((char*) "-d", (char*) "--dump",
				(char*) "dumpsize of each frame", &fDumpsize);
		ArgInt.Treat2Commands((char*) "-sd", (char*) "--subdump",
				(char*) "dumpsize of each sub frame", &fSubDumpsize);
		ArgInt.Treat2Commands((char*) "-fo", (char*) "--format",
				(char*) "format number", &fFormat);
		ArgInt.Treat2Commands((char*) "-f", (char*) "--file",
				(char*) "file name", (char*) (fCurrentFilename));
		if (ArgInt.Treat2Commands((char*) "-a", (char*) "--actionfile",
				(char*) "action file name", (char*) (fActionFilename)))
			fWorkMode = GENERATE_PARA_FILES;
		if (ArgInt.Treat2Commands((char*) "-extf", (char*) "--extractfile",
				(char*) "file name", (char*) (fExtFilename)))
			fWorkMode = EXTRACT_FRAMES;
		if (ArgInt.Treat2Commands((char*) "-rmbf", (char*) "--removebadframes",
				(char*) "file name", (char*) (fExtFilename)))
			fWorkMode = REMOVE_BAD_FRAMES;
		if (ArgInt.Treat2Commands((char*) "-arr", (char*) "--arrangeframes",
				(char*) "file name", (char*) (fExtFilename)))
			fWorkMode = ARRANGE_FRAMES;
		ArgInt.Treat2Commands((char*) "-exps", (char*) "--expectedsize",
				(char*) "expected size", &fExpSize);
		ArgInt.Treat2Commands((char*) "-expt", (char*) "--expectedtype",
				(char*) "expected type", &fExpType);
		ArgInt.Treat2Commands((char*) "-expe", (char*) "--expectedendian",
				(char*) "expected endianess (0 little (Intel), 1 big(PowerPC))",
				&fExpEndian);
		ArgInt.Treat2Commands((char*) "-exms", (char*) "--expectedmaxsize",
				(char*) "expected max size", &fExpMaxSize);
		if (ArgInt.Treat2Commands((char*) "-pat", (char*) "--pattern",
				(char*) "patterntofindindecimalmode", (char*) fPattern))
			fWorkMode = EXTRACT_PATTERN;
		if (ArgInt.Treat1Command((char*) "-w", (char*) "--write"))
			fWorkMode = WRITE_FRAMES;
		if (ArgInt.Treat1Command((char*) "-r", (char*) "--read"))
			fWorkMode = READ_FRAMES;
		if (ArgInt.Treat1Command((char*) "-l", (char*) "--list")) {
			HelpList();
			exit(0);
		}
		if (ArgInt.Treat1Command((char*) "-h", (char*) "--help"))
			HelpAndExit();
		if (ArgInt.Treat1Command((char*) "-ver", (char*) "--version")) {
			Announce(argv[0]);
			exit(0);
		}
		if (ArgInt.Treat1Command((char*) "-chc", (char*) "--chc"))
			fWorkMode = CREAT_CHC_PAR;
		if (ArgInt.Treat1Command((char*) "-test", (char*) "--test"))
			fWorkMode = TEST_MODE;
		ArgInt.TestNoCommandSoExit();
	}
	int retour = 0;
	bool localtest = true;
	fGeneral->SetVerbose(fVerbose);
	switch (fWorkMode) {
	case WRITE_FRAMES:
		WriteFrame();
		break;
	case READ_FRAMES:
		ReadFrame();
		break;
	case EXTRACT_FRAMES:
		ExtractFrame();
		break;
	case EXTRACT_PATTERN:
		ExtractPattern();
		break;
	case GENERATE_PARA_FILES:
		ParameterFile();
		break;
	case REMOVE_BAD_FRAMES:
		RemoveBadFrames();
		break;
	case ARRANGE_FRAMES:
		ArrangeFrames();
		break;
	case CREAT_CHC_PAR:
		CreatCHCParFile();
		break;
	case TEST_MODE:
		localtest = Test(fVerbose);
		if (localtest)
			retour = 0;
		else
			retour = 1;
		break;
	default:
		ReadFrame();
		break;
	}
	deletefunction();
	cout << "-------------------End------------------------" << endl;
	return (retour);
}

//_______________________________________________________________________________________________________________________

void WriteFrame() {

// Write various generated MFM frames in a file

	cout << endl;
	cout << "-------------------------------------------------" << endl;
	cout << "|    WRITE of " << fNbFrames << " MFMFrame in file  "
			<< fCurrentFilename << "" << endl;
	cout << "-------------------------------------------------" << endl;
	cout << endl;

	fEventNumber = 0;

	int fLun; // Logical Unit Number
	fLun = open(fCurrentFilename, (O_RDWR | O_CREAT | O_TRUNC), 0644);

	if (fFormat == -1) {
		while ((fFormat < 0) or (fFormat > 0xFFFF)) {
			PrintQuestion();
			cout << "Enter your choice : ";cin >> fFormat;
		}
		if (fFormat == 0) {
			deletefunction();
			exit(0);
		}
	}
	WriteUserFrame(fLun, fFormat, fNbFrames, fNbSubFrames);
	close(fLun);

	Statistics(true);
}
//_______________________________________________________________________________________________________________________

void ParameterFile() {

// Read parameter frame and fill DataParameters object.

	struct stat filestatus;
	long long fFileSize = 0;
	stat(fCurrentFilename, &filestatus);
	fFileSize = filestatus.st_size;
	int minsizeheader = MFM_BLOB_HEADER_SIZE; // =8

	int fLun = 0; // Logical Unit Number
	fLun = open(fCurrentFilename, (O_RDONLY));
	if (fLun <= 0) {
		Error.TreatError(2, 0, "Error open file :", fCurrentFilename);
	}
	cout << endl;
	cout << "-------------------------------------------------------------"
			<< endl;
	cout << "    MFMtest " << "   version : " << MFM_VERSION << endl;
	cout << "|   READ MFMFrame from file : " << fCurrentFilename << "" << endl;
	cout << "-------------------------------------------------------------"
			<< endl;
	cout << endl;
	fMaxdump = 128;

	//------------------------------------------------------------------------
	// Test xml a enlever des que c'est prêt
	//
#ifndef NO_MFMXML

	DataParameters datapar;
	datapar.FillFromActionXMLFile((const char*) fCurrentFilename);

	datapar.DumpListOfNames();
	datapar.CreatXML();
	datapar.WriteXML("bidon1.xml");
	cout
			<< "------------------Test recriture/lecture fichier xml----------------------------------"
			<< endl;
	cout << "-----------------0-----------------------------------\n";
	DataScalers datascal;
	cout << "-----------------1-----------------------------------\n";
	datascal.FillFromActionXMLFile((const char*) fCurrentFilename);
	cout << "-----------------2-----------------------------------\n";
	datascal.DumpListOfNames();
	cout << "-----------------3-----------------------------------\n";
	datascal.CreatXML();
	cout << "-----------------4-----------------------------------\n";
	datascal.WriteXML("bidon.xml");
	cout << "-----------------5-----------------------------------\n";
#else
     cout <<" When compilation of XML is devalided , this action on ACTION FILE is not possible!\n";
    #endif

	close(fLun);

	//------------------------------------------------------------------------

}

//_______________________________________________________________________________________________________________________   
void ReadFrame() {

// Read MFM file and extract infomations; dump, statistic, parameters values

	struct stat filestatus;
	long long fFileSize = 0;
	stat(fCurrentFilename, &filestatus);
	fFileSize = filestatus.st_size;
	int minsizeheader = MFM_BLOB_HEADER_SIZE; // =8
	int fLun = 0; // Logical Unit Number   

	cout << endl;
	cout << "-------------------------------------------------------------"
			<< endl;
	cout << "    MFMtest " << "   version : " << MFM_VERSION << endl;
	cout << "|   READ MFMFrame from file : " << fCurrentFilename << "" << endl;
	cout << "-------------------------------------------------------------"
			<< endl;
	cout << endl;

	fLun = open(fCurrentFilename, (O_RDONLY));
	if (fLun <= 0) {
		Error.TreatError(2, 0, "Error open file :", fCurrentFilename);
	}

	fMaxdump = 128;

	fNbOfEvent = 0;
	UtilVector_c *vector = new UtilVector_c(MFM_BLOB_HEADER_SIZE); // min size =8              
	int framesize = 0;
	long long int count = 0;
	static long long int countp = 0;
	bool display= false;
	display = InfoCondition();
	while (fNbOfEvent != fNbFramesRead) {
		framesize = fFrame->ReadInFile(&fLun, vector);
		fFrame->SetAttributs();
		fPtInFile += framesize;
		if (framesize <= 0)
			break;
		//count = ReadUserFrame(fFrame);
		fGeneral->ReadUserFrame(fFrame,fDumpsize, display,fNbOfEvent);
		countp += count;
		fNbOfEvent++;
	}
	if (vector)
		delete (vector);
	fLun = close(fLun);
	Statistics(false);
}

//_______________________________________________________________________________________________________________________
void ExtractFrame() {

// methot to extract frame on condition 

	struct stat filestatus;
	long long fFileSize = 0;
	stat(fCurrentFilename, &filestatus);
	fFileSize = filestatus.st_size;
	int minsizeheader = MFM_BLOB_HEADER_SIZE; // =8

	cout << endl;
	cout << "-------------------------------------------------------------"
			<< endl;
	cout << "|    MFMtest " << "   version : " << MFM_VERSION << endl;
	cout << "|    Extract mode from file " << fCurrentFilename << " to file "
			<< fExtFilename << endl;
	cout << " |    Expected type =" << fExpType << "Expected size =" << fExpSize
			<< endl;
	cout << "-------------------------------------------------------------"
			<< endl;

	int len = 0;
	if (fExpMaxSize < 0)
		fExpMaxSize = 2048;
	int fNonExpectedSizeCount = 0;
	int fExpectedSizeCount = 0;
	fMaxdump = 128;
	FILE *fLun = NULL;
	FILE *fLunOut = NULL;
	FILE *fLunOutlog = NULL;
	char logfile[512];
	char tempos[512];

	fLun = fopen(fCurrentFilename, "r");
	if (fFileSize < fExpMaxSize)
		fExpMaxSize = fFileSize;

	if (strcmp(fExtFilename, "") != 0) {
		fLunOut = fopen(fExtFilename, "w");
		sprintf(logfile, "%s.log", fExtFilename);
		fLunOutlog = fopen(logfile, "w");
		if (fLunOut == NULL) {
			Error.TreatError(2, 0, "Error open file :", fExtFilename);
		}
		if (fLunOutlog == NULL) {
			Error.TreatError(2, 0, "Error open file :", logfile);
		}
		sprintf(tempos, "log of extract frame from %s\n", fCurrentFilename);
		len = strlen(tempos);
		fwrite(tempos, 1, len, fLunOutlog);
		sprintf(tempos,
				"sizefile = %lld expected type = %d expected size %d  expected max size file %d \n\n",
				fFileSize, fExpType, fExpSize, fExpMaxSize);
		len = strlen(tempos);
		fwrite(tempos, 1, len, fLunOutlog);
		sprintf(tempos,
				"count  pt_of_frame   not_good_frame_or_lost_bytes   sizeof_last_write \n");
		len = strlen(tempos);
		fwrite(tempos, 1, len, fLunOutlog);
	}

	int nbofevent = 0;
	char *vector;
	char **pvector;
	int vectorsize = fExpMaxSize;
	vector = (char*) (malloc(vectorsize));
	pvector = &vector;
	int framesize = 0;
	int type = 0;
	int endianness = 0;
	int retour;
	int readframesize = 0;

	long long fReadsize = 0;
	long long previousfReadsize = fReadsize;
	long long temposize;
	cout << "0 % of file\n ";
	int nbmodulo = 100;
	int modulo = fFileSize / nbmodulo;
	int modulocumul = modulo;
	while (true) {
		fReadsize = ftell(fLun);
		temposize = fReadsize;
		if (fReadsize > modulo + modulocumul) {
			cout << (float) fReadsize / (float) (fFileSize) * 100 << "%  "
					<< fReadsize << "/" << fFileSize
					<< " Nb of extracted frames : " << fExpectedSizeCount
					<< endl << flush;
			modulocumul = fReadsize;
		}
		ClearVector(vector, vectorsize);
		retour = ReadInBadFile(fLun, pvector, &vectorsize, &readframesize,
				fReadsize, fFileSize);
		fReadsize = ftell(fLun);
		if (retour != fReadsize - temposize) {
			cout << "Error of readsize " << retour << " =! "
					<< fReadsize - temposize << endl;
		}
		if (retour <= 0)
			break;
		if (fReadsize > fFileSize)
			break;
		if (retour < 0) {
			cout << "Error of readsize " << retour << endl;
			break;
		}
		fFrame->SetAttributs(vector);
		framesize = fFrame->GetFrameSize();
		type = fFrame->GetFrameType();
		endianness = fFrame->GetFrameEndianness();

		if (((framesize != fExpSize) and (fExpSize != -1))
				or ((type != fExpType) and (fExpType != -1))
				or ((endianness != fExpEndian) and (fExpEndian != -1))) {

			if (fExtDumpSize > 0) {
				cout << " ----------Dump Problem------" << fReadsize
						<< "------------\n";

				fFrame->DumpRaw(fExtDumpSize, 0);
			}

			fNonExpectedSizeCount++;
			fseek(fLun, (-retour + 1), SEEK_CUR);

		} else {
			fFrame->FillStat();
			if (fLunOut)
				fwrite(vector, 1, retour, fLunOut);
			sprintf(tempos, "%d %lld %lld %d \n", ++fExpectedSizeCount,
					fReadsize - retour, fReadsize - previousfReadsize - retour,
					retour);
			len = strlen(tempos);
			fwrite(tempos, 1, len, fLunOutlog);
			previousfReadsize = fReadsize;
		}

		if (fVerbose > 1) {
			cout << "\n-- Read frame in file : nb = " << nbofevent
					<< " ----position in file :" << fReadsize << "-----------"
					<< "------------------------------\n";
		}
		framesize = retour;
		ReadDefaultFrame(fFrame, fReadsize, fFileSize);
		nbofevent++;
		fFrame->SetAttributs();
	}
	if (fLunOut != NULL) {
		fclose(fLunOut);
	}
	if (fLunOutlog != NULL) {
		fclose(fLunOut);
	}
	if (fLun != NULL) {
		fclose(fLun);
	}
}

//_______________________________________________________________________________________________________________________
void RemoveBadFrames() {
	// Read and remove bad data/frames in a run file containing MFM Frames

	struct stat filestatus;
	long long fFileSize = 0;
	stat(fCurrentFilename, &filestatus);
	fFileSize = filestatus.st_size;
	int minsizeheader = MFM_BLOB_HEADER_SIZE; // =8
	bool localverbose = false;

	int len = 0;
	if (fExpMaxSize < 0)
		fExpMaxSize = 2048;
	int fNonExpectedSizeCount = 0;
	int fExpectedSizeCount = 0;
	bool flipflopgoodbad = true;
	fMaxdump = 128;
	FILE *fLun = NULL;
	FILE *fLunOut = NULL;
	FILE *fLunOutlog = NULL;
	char logfile[512];
	char tempos[512];
	int nberrort = 0;
	int nberrors = 0;
	int nberrorm = 0;
	long long int nbmodulo = 100;
	long long int modulo = fFileSize / nbmodulo;
	long long int modulocumul = modulo;
	int readsize = 0;
	int NbBadFrames = 0;
	long long int NbSlippages = 0;
	int NbGoodFrames = 0;
	int NbBadMergeFrames = 0;
	uint64_t *pt8B;
	bool endoffile = false;
	char *vector;
	int vectorsize = 3 * fExpMaxSize;
	bool withlogfile = false;
	int framesize = 0;
	int type = 0;
	int FirstSizeToRead = MFM_BLOB_HEADER_SIZE;
	long long int TotalReadSize = 0, ReadSize = 0;

	vector = (char*) (malloc(vectorsize));

	cout << endl;
	cout << "-------------------------------------------------------------"
			<< endl;
	cout << "|    MFMtest " << "   version : " << MFM_VERSION << endl;
	cout << "|    Remove Bad Frames mode from file " << fCurrentFilename
			<< " to file " << fExtFilename << endl;
	cout << "|    Max size frames allowed=" << fExpMaxSize << endl;
	cout << "-------------------------------------------------------------"
			<< endl;

	fLun = fopen(fCurrentFilename, "r");
	if (fFileSize < fExpMaxSize)
		fExpMaxSize = fFileSize;

	if (strcmp(fExtFilename, "") != 0) {
		fLunOut = fopen(fExtFilename, "w");
		if (fLunOut == NULL) {
			Error.TreatError(2, 0, "Warning, output ,no open file :",
					fExtFilename);
		}
		sprintf(logfile, "%s.log", fExtFilename);
		if (withlogfile) {
			fLunOutlog = fopen(logfile, "w");

			if (fLunOutlog == NULL) {
				Error.TreatError(2, 0, "Error open file :", logfile);
			}
			sprintf(tempos, "log of remove bad frames from %s\n",
					fCurrentFilename);
			len = strlen(tempos);
			fwrite(tempos, 1, len, fLunOutlog);
			sprintf(tempos, "sizefile = %lld  expected max size file %d \n\n",
					fFileSize, fExpMaxSize);
			len = strlen(tempos);
			fwrite(tempos, 1, len, fLunOutlog);
			sprintf(tempos,
					"count  pt_of_frame  not_good_frame_or_lost_bytes   sizeof_last_write \n");
			len = strlen(tempos);

			fwrite(tempos, 1, len, fLunOutlog);
		}
	}

	cout << "0 % of file\n ";

	while (true) {
		ReadSize = ftell(fLun);
		if (ReadSize != TotalReadSize) {
			cout << " Error beetwen total read " << TotalReadSize
					<< " and pt on file " << ReadSize << endl;
		}
		if (TotalReadSize > modulo + modulocumul) {
			cout <<(int)(float) (TotalReadSize / (float) (fFileSize) * 100) << "%  "
					<< TotalReadSize << "/" << fFileSize << " -- Slips "
					<< NbSlippages << "  Bad_Merge_F " << NbBadMergeFrames
					<< "  Bad_F " << NbBadFrames << " || Good_F "
					<< NbGoodFrames << endl << flush;
			modulocumul = ReadSize;
		}

		readsize = fread((void*) vector, 1, FirstSizeToRead, fLun);
		if (readsize <= 0) {
			cout << endl << " End of read file\n";
			endoffile = true;
			break;
		}
		if (readsize != FirstSizeToRead) {
			cout << "Nb of read Bytes : " << readsize << " and wanted : "
					<< FirstSizeToRead << " Error in read file ! \n";
			endoffile = true;
			break;
		}
		TotalReadSize += readsize;

		while (true) {
			fFrame->SetAttributsOn8Bytes(vector);
			if (localverbose)
				fFrame->DumpRaw(8, 0);
			type = fFrame->GetFrameType();
			framesize = fFrame->GetFrameSize();

			nberrort = 0;
			nberrors = 0;
			nberrorm = 0;
			if (!fFrame->TestType(type))
				nberrort++;
			if ((framesize > fExpMaxSize) or (framesize < MFM_BLOB_HEADER_SIZE))
				nberrors++;
			if (fFileSize - TotalReadSize < 0)
				nberrors++;

			if (nberrort + nberrors > 0) {

				if (fVerbose >8)cout << "TotalReadSize " <<TotalReadSize<<"  type "<< type << "   size "<< framesize  <<  " error "<<nberrors<<" "<<nberrort  <<"\n";

				if (flipflopgoodbad) {
					NbBadFrames++;
					flipflopgoodbad = false;cout << ">" << flush;
				}

				pt8B = (uint64_t*) vector;
				(*pt8B) = (*pt8B) >> 8;

				readsize = fread((void*) (vector + MFM_BLOB_HEADER_SIZE - 1), 1,
						1, fLun);
				cout << "." << flush;
				if (NbSlippages++ % 40 == 0)
					cout << endl << flush;
				if (readsize == 0)
					endoffile = true;
				TotalReadSize += readsize;
				//cout << TotalReadSize << "/" << fFileSize << endl;
				if (TotalReadSize >= fFileSize) {
					endoffile = true;
					break;
				}
				//cout << "----------------------------------------\n";
			} else {
				readsize = fread((void*) (vector + MFM_BLOB_HEADER_SIZE), 1,
						framesize - MFM_BLOB_HEADER_SIZE, fLun);
				TotalReadSize += readsize;
				if (readsize != framesize - MFM_BLOB_HEADER_SIZE) {
					endoffile = true;
					cout << "BUG>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << readsize
							<< "\n";
					break;
				}
				if (type == MFM_MERGE_EN_FRAME_TYPE
						or type == MFM_MERGE_TS_FRAME_TYPE) {
					int error = TestMergeFrameInFile(vector, fExpMaxSize);
					if (error > 0) {
						NbBadMergeFrames++;
						cout << "M" << flush;
						nberrorm++;
						break;
					}
				}
				if (flipflopgoodbad == false) {
					flipflopgoodbad = true;
					cout << endl;
				}
				if (fLunOut)
					fwrite(vector, 1, framesize, fLunOut);
				NbGoodFrames++;
				break;
			} //end of if (nberror > 0) {
			if (TotalReadSize > fFileSize) {
				endoffile = true;
				break;
			}

		} //end of while (true)
		if (endoffile)
			break;
	} //end of while (true)
	cout << endl << (float) TotalReadSize / (float) (fFileSize) * 100 << "%  "
			<< TotalReadSize << "/" << fFileSize << " -- Slips " << NbSlippages
			<< "  Bad_Merge_F " << NbBadMergeFrames << "  Bad_F " << NbBadFrames
			<< " || Good_F " << NbGoodFrames << endl << flush;
	if (fLunOut != NULL) {
		fclose(fLunOut);
	}
	if (fLunOutlog != NULL) {
		fclose(fLunOutlog);
	}
	if (fLun != NULL) {
		fclose(fLun);
	}
	cout << endl;
}

//_______________________________________________________________________________________________________________________
void ArrangeFrames() {
	// Read and arrange frame with the timestamps
	// the frames with TS = 0 are remove from the output.
    // coding is  not finished ...
	struct stat filestatus;
	long long fFileSize = 0;
	stat(fCurrentFilename, &filestatus);
	fFileSize = filestatus.st_size;
	int minsizeheader = MFM_BLOB_HEADER_SIZE; // =8
	bool localverbose = false;

	int i,len = 0;
	if (fExpMaxSize <= 0)
		fExpMaxSize = 2048;
		
	int fNonExpectedSizeCount = 0;
	int fExpectedSizeCount = 0;
	bool flipflopgoodbad = true;
	fMaxdump = 128;
	int fLun = 0;
	FILE *fLunOut = NULL;

	char tempos[512];
	int readsize = 0;
	bool endoffile = false;
	int framesize = 0;
	int type = 0;
	int FirstSizeToRead = MFM_BLOB_HEADER_SIZE;
	long long int TotalReadSize = 0, ReadSize = 0;
	MFMCommonFrame ** tabOfMFM ;
	if (fExpMaxSize>0)
		tabOfMFM = new MFMCommonFrame*[fExpMaxSize];
	fNbOfEvent = 0;
	UtilVector_c *vector = new UtilVector_c(MFM_BLOB_HEADER_SIZE); // min size =8  

	cout << endl;
	cout << "-------------------------------------------------------------"<< endl;
	cout << "|    MFMtest " << "   version : " << MFM_VERSION << endl;
	cout << "|    Arrange frames form " << fCurrentFilename
			<< " to file " << fExtFilename << endl;
	cout << "|    with a buffer of  =" << fExpMaxSize << " frames" << endl;
	cout << "-------------------------------------------------------------"
			<< endl;

	fLun = open(fCurrentFilename, (O_RDONLY));
	if (fLun <= 0) {
		Error.TreatError(2, 0, "Error open file :", fCurrentFilename);
	}

	if (fFileSize < fExpMaxSize)
		fExpMaxSize = fFileSize;
        
	if (strcmp(fExtFilename, "") != 0) {
		fLunOut = fopen(fExtFilename, "w");
		if (fLunOut == NULL) {
			Error.TreatError(2, 0, "Warning, output ,no open file :",
					fExtFilename);
		}
		
	}
	

	cout << "0 % of file\n";

	long long int count = 0;
	static long long int countp = 0;
	uint64_t ts = 0;
	
	while (fNbOfEvent != fNbFramesRead) {
		framesize = fFrame->ReadInFile(&fLun, vector);
		fFrame->SetAttributs();
		ts = fFrame->GetTimeStamp();
		cout << fNbOfEvent<<" ts = " << ts << " fs = "<<framesize<< endl;
		fPtInFile += framesize;
		if (framesize <= 0)
			break;	
		if ((fLunOut)&&(ts!=0))
			fwrite(vector, 1, framesize, fLunOut);
		countp += count;
		fNbOfEvent++;
	}

	fLun = close(fLun);
	cout << endl << (float) TotalReadSize / (float) (fFileSize) * 100 << "%  " << endl << flush;
	if (fLunOut != NULL) {
		fclose(fLunOut);
	}
	if (vector)
		delete (vector);
	fLun = close(fLun);
	if (fExpMaxSize>0) 
        	for (i = 0; i<fExpMaxSize;i++){
        		if (tabOfMFM[i]!=NULL);
        		delete[]tabOfMFM[i];
        	};
	cout << endl;
}
void CreatCHCParFile(){
// creat a standart CHC parameter file to do test
// number of lines is defined by -n=nb (number of frames) 
// Name of file is fixe to ACTIONS_test.CHC_PAR
int maxline = 1000;
char line[maxline]; 
FILE *fLun = NULL;
int i = 0;
cout << " Creat a standart CHC parameter file ACTIONS_test.CHC_PAR to do test with "<< fNbFrames<< " parameters"<<endl;
fLun = fopen("ACTIONS_test.CHC_PAR", "w");
for (i=0;i<fNbFrames;i++){       
	sprintf(line, "Para_%d       %d       14        1\n", i,i+1);
	int len = strlen(line);
	if (len >= maxline ) cout << "Error , line too long \n";
	fwrite(line, 1, len, fLun);
	cout << line;
}
fclose(fLun);
cout << " ---------Done!---------"<<endl;
}


//_______________________________________________________________________________________________________________________
int TestMergeFrameInFile(char *vector, int expMaxSize) {

	int i_insframe = 0;
	int nbinsideframe = 0;
	int dumpsize = 16;
	int framesize = 0;
	char *pt_framesize = (char*) &framesize;
	int maxdump = 128;
	int nberror = 0;
	int type = 0;
	char *pt_type = (char*) &type;
	int remainsize = 0;
	char *pt;
	int endian = 0;
	int unit = 0;
	int tmp = 0;
	string st_tmp;
	int headersize = 0;

	fMergeframe2->SetAttributs(vector);
	nbinsideframe = fMergeframe2->GetNbItems();
	framesize = fMergeframe2->GetFrameSize();
	type = fMergeframe2->GetFrameType();
	//fMergeframe2->HeaderDisplay();

	if ((type != MFM_MERGE_EN_FRAME_TYPE) and (type != MFM_MERGE_TS_FRAME_TYPE))
		nberror++;
	if ((framesize > expMaxSize) or (framesize < MFM_BLOB_HEADER_SIZE))
		nberror++;
	if (nberror > 0)
		return nberror;

	fMergeframe2->ResetReadInMem();
	headersize = fMergeframe2->GetHeaderSize();

	if ((headersize != MERGE_EN_HEADERSIZE)
			and (headersize != MERGE_TS_HEADERSIZE)) {
		nberror++;
		return nberror;
	};
	remainsize = framesize - headersize;
	if (remainsize < 0) {
		nberror++;
		return nberror;
	};
	pt = (char*) (vector + headersize);

	if ((pt > (vector + framesize)) or (pt < vector)) {
		nberror++;
		return nberror;
	};

	for (i_insframe = 0; i_insframe < nbinsideframe; i_insframe++) {
		framesize = 0;
		type = 0;
		unit = 0;
		endian = (char) (pt[0]) & MFM_ENDIANNESS_MSK;
		// cout << "i_insframe "<< i_insframe<<" pt " << (long long *) pt<< endl,
		tmp = pt[0] & MFM_UNIT_BLOCK_SIZE_MSK;
		unit = pow((double) 2, tmp);
		if (endian) {
			pt_framesize[0] = pt[1];
			pt_framesize[1] = pt[2];
			pt_framesize[2] = pt[3];
			pt_type[0] = pt[5];
			pt_type[1] = pt[6];
		} else {
			pt_framesize[2] = pt[1];
			pt_framesize[1] = pt[2];
			pt_framesize[0] = pt[3];
			pt_type[1] = pt[5];
			pt_type[0] = pt[6];
		}
		framesize = framesize * unit;

		//fMergeframe2->GetDumpRaw(pt, 32,0,&st_tmp);
		//cout << st_tmp;
		//cout << "TestMergeFrameInFile t "<<type<< "  size "<< framesize <<endl;
		//exit(0);

		if ((type == MFM_MERGE_EN_FRAME_TYPE)
				or (type == MFM_MERGE_TS_FRAME_TYPE)) {
			nberror += TestMergeFrameInFile((char*) pt, expMaxSize);
			nberror++;
		} else {
			if (!fMergeframe2->TestType(type))
				nberror++;
			if ((framesize > expMaxSize) or (framesize < MFM_BLOB_HEADER_SIZE))
				nberror++;
			if ((framesize > remainsize))
				nberror++;
			remainsize -= framesize;
			pt = pt + framesize;
		}
		if (nberror)
			i_insframe = nbinsideframe;
	}

	return nberror;
}

//_______________________________________________________________________________________________________________________

int ReadInBadFile(FILE *fLun, char **vector, int *vectorsize,
		int *readframesize, int readsize, long long filesize) {
	/// Get data from a file, and fill current frame and initialize its attributs and its pointer
	/// if size of actual frame is not enough, and new size is reallocated
	///      fLun   : descriptor of file (given by a previous open)
	///      vector : pointer on pointer will contain frame . if size isn't big enouth, a new value of pointer
	///       and vectorsize of this pointer
	///      return size of read .

	int countsize1 = 0;
	int countsize2 = 0;
	char *vectornew = NULL;
	int FirstSizeToRead = 4;
	int sizetoread = 0;

	//cout <<"debug FirstSizeToRead = " <<FirstSizeToRead<< " Lun = " <<fLun << "  |vector = " << *vector << " |vect size "<<*vectorsize<<"\n";

	countsize1 = fread((void*) (*vector), 1, FirstSizeToRead, fLun);

	if (countsize1 <= 0) {
		cout << endl << " End of read file\n";
		return countsize1;
	}
	if (countsize1 < FirstSizeToRead) {
		if (FirstSizeToRead < (filesize - readsize))
			cout << "Nb of read Bytes " << countsize1
					<< " Error in read file ! \n";
		return countsize1;
	}

	fFrame->SetAttributsOn4Bytes((*vector));
	*readframesize = fFrame->GetFrameSize();
	sizetoread = *readframesize;

	if (*readframesize < fExpMaxSize)
		sizetoread = *readframesize;
	else
		sizetoread = fExpMaxSize;

	if (sizetoread < fExpSize)
		sizetoread = fExpSize;

	if ((*vectorsize < sizetoread)) {
		vectornew = (char*) (realloc((void*) (*vector), sizetoread));
		if (vectornew != NULL) {
			(*vector) = vectornew;
			*vectorsize = sizetoread;
			fFrame->SetAttributs((*vector));
		} else {
			Error.TreatError(1, sizetoread,
					" Memory allocation in MFMCommonFrame:: ReadInFile");
			return 0;
		}
	}

	sizetoread -= FirstSizeToRead;

	countsize2 = fread((void*) ((*vector) + FirstSizeToRead), 1, sizetoread,
			fLun);
	if (countsize2 != sizetoread) {
		if (sizetoread < (filesize - (readsize + FirstSizeToRead))) {
			char info[200];
			sprintf(info,
					"Error in read file ,read /asked to read : %d / %d  (may be end of file?)",
					countsize2, sizetoread);
			Error.TreatError(1, 0, info);
		}
	}
	return countsize2 + countsize1;
}
//_______________________________________________________________________________________________________________________

void ExtractPattern() {
	struct stat filestatus;
	long long fFileSize = 0;
	stat(fCurrentFilename, &filestatus);
	fFileSize = filestatus.st_size;
	int minsizeheader = MFM_BLOB_HEADER_SIZE; // =8

	cout << endl;
	cout << "-------------------------------------------------------------"
			<< endl;
	cout << "|   MFMtest " << "   version : " << MFM_VERSION << endl;
	cout << "|   TEST  MFMFrame in Pattern mode from file " << fCurrentFilename
			<< "" << endl;
	cout << "-------------------------------------------------------------"
			<< endl;

	int len = 0;
	int fNonExpectedSizeCount = 0;
	fMaxdump = 128;
	FILE *fLun = NULL;
	FILE *fLunOutlog = NULL;
	char logfile[512];
	char tempos[600];
	unsigned char cmpvector[1024];
	int countpattern = 0;
	string spattern = fPattern;
	fLun = fopen(fCurrentFilename, "r");
	sprintf(logfile, "%s.pattern.log", fCurrentFilename);
	fLunOutlog = fopen(logfile, "w");
	if (fLunOutlog == NULL) {
		Error.TreatError(2, 0, "Error open file :", logfile);
	}
	sprintf(tempos, "Look for pattern \" %s \" from %s\n", fPattern,
			fCurrentFilename);
	len = strlen(tempos);
	fwrite(tempos, 1, len, fLunOutlog);
	cout << tempos;

	string element;
	unsigned int x;
	int placeofchar = 0, i = 0;
	std::stringstream info;
	while (placeofchar != std::string::npos) {
		placeofchar = spattern.find("_");
		if (placeofchar >= 0) {
			element = spattern.substr(0, placeofchar);
			;
			spattern = spattern.substr(placeofchar + 1);
		} else {
			element = spattern;
		}
		std::stringstream ss;
		ss << std::hex << element;
		ss >> x;
		cmpvector[countpattern] = x;
		//info << countpattern <<"(debug info) placeofchar = "<< placeofchar<< " element = "<< element<<" value_in_dec = "<< std::dec <<(unsigned int)(cmpvector[countpattern])<<endl;
		//len = info.gcount();
		//fwrite((info.str()).c_str(),1,len,fLunOutlog);
		countpattern++;
	}
	len = strlen((info.str()).c_str());
	fwrite(((info.str()).c_str()), 1, len, fLunOutlog);
	sprintf(tempos,
			"\ncount   file_pointer   file_pointer(in hex)    diff_between2matches  \n");
	len = strlen(tempos);
	fwrite(tempos, 1, len, fLunOutlog);
	int count = 0;
	unsigned char *vector;
	int retour;
	long long fReadsize = 0;
	long long previousfReadsize = fReadsize;
	long long temposize;
	cout << "0 % of file\n ";
	int nbmodulo = 100;
	int modulo = fFileSize / nbmodulo;
	int modulocumul = modulo;
	vector = (unsigned char*) (malloc(countpattern));
	ClearVector((char*) vector, countpattern);
	while (true) {
		fReadsize = ftell(fLun);
		temposize = fReadsize;
		if (fReadsize > modulo + modulocumul) {
			cout << (float) fReadsize / (float) (fFileSize) * 100 << "%  "
					<< fReadsize << "/" << fFileSize << " Nb of matches : "
					<< count << endl << flush;
			modulocumul = fReadsize;
		}
		retour = fread((void*) (vector + countpattern - 1), 1, 1, fLun);
		fReadsize = ftell(fLun);

		if (retour != fReadsize - temposize) {
			cout << "Error of readsize " << retour << " =! "
					<< fReadsize - temposize << endl;
		}
		if (retour <= 0)
			break;
		if (fReadsize > fFileSize)
			break;

		if ((memcmp((const char*) vector, (const char*) cmpvector, countpattern)
				== 0) and (fReadsize >= countpattern)) {
			sprintf(tempos, "%d   %lld   %llx   %lld \n", ++count,
					fReadsize - countpattern, fReadsize - countpattern,
					fReadsize - previousfReadsize);

			len = strlen(tempos);
			fwrite(tempos, 1, len, fLunOutlog);
			previousfReadsize = fReadsize;
		}
		for (i = 0; i < countpattern - 1; i++) {
			vector[i] = (unsigned char) vector[i + 1];
		}
	}
	if (fLunOutlog != NULL) {
		fclose(fLunOutlog);
	}
	if (fLun != NULL) {
		fclose(fLun);
	}
}

//_______________________________________________________________________________________________________________________
void deletefunction() {
// delete various objects;

	if (fGeneral)
		delete (fGeneral);
	if (fFrame)
		delete (fFrame);
	if (fInsideframe)
		delete (fInsideframe);
}

//_______________________________________________________________________________________________________________________
void PrintQuestion() {
	
	cout <<endl<< " Which Frame you want to generate?\n" << endl;
	int i;
	for ( i=1; i< MFM_MAX_TYPE;i++){
		if (fGeneral->GetFrameWithType(i)!=NULL) {
			cout.width(35);
			cout <<fGeneral->GetFrameWithType(i)->GetTypeText();
			cout.width(5);
			cout <<hex <<i;
			cout.width(0);
			cout << "  =>  " << dec << i <<endl;
		}
	}
	cout << endl;
	
	cout.width(35); 
	cout <<"Mixte of all Frames";
	cout.width(5);
	cout <<hex <<TYPE_FORMAT_MIX;
	cout.width(0);
	cout << "  =>  " << dec << TYPE_FORMAT_MIX <<endl;
	
	cout.width(35); 
	cout <<"Merge of Ebyedats in EventNumber";
	cout.width(5);
	cout <<hex <<TYPE_FORMAT_MIX;
	cout.width(0);
	cout << "  =>  " << dec << TYPE_FORMAT_MERG_EBYEN <<endl;
	
	cout.width(35); 
	cout <<"Merge of Ebyedats in TimeStamp";
	cout.width(5);
	cout <<hex <<TYPE_FORMAT_MIX;
	cout.width(0);
	cout << "  =>  " << dec << TYPE_FORMAT_MERG_EBYTS <<endl;
	
	cout.width(35); 
	cout <<"Merge of Cobo in EventNumber";
	cout.width(5);
	cout <<hex <<TYPE_FORMAT_MIX;
	cout.width(0);
	cout << "  =>  " << dec << TYPE_FORMAT_MERG_COBO <<endl;
	

}
//_______________________________________________________________________________________________________________________
void Announce(char *progname) {
	cout << "-------------------------------------------------" << endl;
	cout << "   " << progname << endl;
	cout << "         Version    : " << MFM_VERSION << endl;
	cout << "         Build date : " << BUILD_MFM_DATE << " " << BUILD_MFM_TIME
			<< endl;
	cout << "-------------------------------------------------" << endl;
}
//_______________________________________________________________________________________________________________________
void Help() {
	cout << endl;
	cout << " MFMtest " << "  Version : " << MFM_VERSION << endl;
	cout
			<< "           Utility to generate and use MFM frame in different formats \n";
	cout << "           The generated frames are stored in a file  \n";
	cout << " Usage  : \n";

	cout << " MFMtest.exe [-h] or [--help] , print this help \n";
	cout << "             [-ver] or [--version] , print version \n";
	cout
			<< "             [-f=filname] or [--file=filename] , set name of file to read or"
			<< endl;
	cout << " 	        to write , default is \"" << fDeffilename << "\"\n";
	cout << "             [-w] or [--write], set write mode \n";
	cout << "             [-r] or [--read], set read mode \n";
	cout
			<< "             [-n=nb] or [--number=nb],  set nb number of frames to write,"
			<< endl;
	cout << "               default is" << fNbFrames << endl;
	cout
			<< "             [-nr=nb] or [--numberread=nb],  set nb number of frames to read,"
			<< endl;
	cout << "               default is" << fNbFramesRead << " \n";
	cout
			<< "             [-nd=nb] or [--numberdump=nb],  set nb number of frames to "
			<< endl;
	cout << "               asked, default is all \n";
	cout
			<< "             [-nt=nb] or [--numberstart=nb],  set nb  number of frames to start "
			<< endl;
	cout << "               dump if asked (defautlt =  " << fNbFrames << ") \n";
	cout
			<< "             [-sn=nb] or [--subnumber=nb], set nb number (nb) of subframes to "
			<< endl;
	cout << "               write, default is " << fNbFrames << " \n";
	cout
			<< "             [-v=nb] or [--verbose], give more information during read or "
			<< endl;
	cout << "               write(default : " << fVerbose << ") \n";
	cout
			<< "             [-d=nb] or [--dump=nb],  number of bytes to dump.(defautlt = "
			<< fSubDumpsize << ") \n";
	cout
			<< "             [-sd=nb] or [--subdump=nb],  number of bytes to dump in frame"
			<< endl;
	cout << "               inside a merge frame.(defautlt = " << fSubDumpsize
			<< ") \n";
	cout
			<< "             [-fo=nb] or [--format=nb], set format of frame to write, if not"
			<< endl;
	cout << "               precised, you will have this menu to chose\n";
	cout
			<< "             [-itn=nb] or [--itemnumbre=nb], set nb of items frame to write, "
			<< endl;
	cout << "               in case of allowed frame\n";
	cout
			<< "             [-l] or [--list], print list of available types of frames "
			<< endl;
	cout
			<< "             [-rmbf=file] or [--removebadframes=file], remove bad frames , expectedmaxsize can be ajusted with [-exms=s],"
			<< endl;
	cout
			<< "             [-extf=file] or [--extractfile=file], set MFMTest in extract mode,"
			<< endl;
	cout
			<< "               file is  outpout file, expected format and expected size have to"
			<< endl;
	cout
			<< "               be filled (Default is Hello frame and default size of HelloFrame)"
			<< endl;
	cout << "               if file = NULL , only statistique are display"
			<< endl;
	cout
			<< "             [-exps=s] or [--expectedsize=s], set expected size in extract mode "
			<< endl;
	;
	cout
			<< "             [-expt=t] or [--expectedtype=t], set expected type in extract mode"
			<< endl;
	;
	cout
			<< "             [-exms=s] or [--expectedmaxsize =s], set expected max size in "
			<< endl;
	cout << "               extract mode\n";
	cout
			<< "             [-exds=s] or [--extractdumsize =s], set dump size in case of "
			<< endl;
	cout << "               problem in extract mode\n";
	cout
			<< "             [-pat=text] or [--pattern = text], set MMFtext un read partern mode"
			<< endl;
	cout
			<< "               , text have have to be like that\"C0_12_00_00_FF_00_FF_01\", a "
			<< endl;
	cout
			<< "               filename.pattern.log is created and give location of patterns\n";
	cout << "             [-test] or [--test], Launch Unit tests " << endl;
	cout << "             [-chc] or [--chc], creat standard test ACTION_test.CHC_PAR file to test analysis of EBYEDAT data " << endl;
	cout << "                                number of lines (parameters) is defined with -n = x or --number = x,default is" << fNbFrames << endl;

	PrintQuestion();
	cout << endl;
	cout << endl;
	cout
			<< "   Examples MFMtest.exe --write -fo=2 -n=100 : write with a choosen format = 2 and with 100 generated frames\n";
	cout << "             MFMtest.exe --read           : read file \""
			<< fDeffilename << "\" \n";
	cout
			<< "             MFMtest.exe --read -f=run.dat: read \"run.dat\" file \n";
	cout << endl;
	

}
//_______________________________________________________________________________________________________________________

void HelpList() {
	cout <<" Info for the MFM header begin (B=Byte) :"<<endl;
	cout <<"    1B : Endianness[7](0=BigEndian), Blobness[6], Notused[5,4], BlockSize[3,2,1,0]  "<<endl;
	cout <<"    3B : Framesize (ex 02 00 00) =   131072*Blocksize (Big Endian (ex Power PC)) , 2*Blocksize (Little Endian (ex intel x86))"<<endl;
	cout <<"    1B : Data Source"<<endl;
	cout <<"    2B : Frame Type"<<endl;
	cout <<"    1B : Revision"<<endl;
	cout <<" -> If not blob we continue with "<<endl;
	cout <<"    2B : Header Size (ex 02 00 = 32 * BlockSize (Little Endian)"<<endl;
	cout <<"    2B : ItemSize  ( = 0 for Layered Frame else Basic Frame)"<< endl;
	cout <<"    4B : NbItems "<<endl;
		
	cout <<endl<< " List of MFM types" << endl;
	int i;
	for ( i=1; i< MFM_MAX_TYPE;i++){
		if (fGeneral->GetFrameWithType(i)!=NULL) {
			cout.width(35);
			cout <<fGeneral->GetFrameWithType(i)->GetTypeText();
			cout.width(5);
			cout <<hex <<i;
			cout.width(0);
			cout << "  =  " << dec << i <<endl;
		}
	}
	

}
//_______________________________________________________________________________________________________________________
/*
int ReadUserFrame(MFMCommonFrame *commonframe) {
	commonframe->SetAttributs();
	int type = commonframe->GetFrameType();
	int headersize = commonframe->GetHeaderSize();
	// part of use of frame
	// example
	bool display = InfoCondition();
	static long long int count = 0;
	count++;
	int noframe = fNbOfEvent;
	if (type == MFM_MERGE_EN_FRAME_TYPE or type == MFM_MERGE_TS_FRAME_TYPE) {
		ReadMergeFrame(commonframe, fDumpsize, display, noframe);
	} else {
		if ((fGeneral->GetFrameWithType(type)) != NULL) {
			(fGeneral->GetFrameWithType(type))->ReadAttributsExtractFrame(
					fVerbose, fDumpsize, display, noframe,
					commonframe->GetPointHeader());
		} else {
			fCount_elseframe++;
			(fGeneral->GetFrameWithType(MFM_COMMON_FRAME_TYPE))->ReadAttributsExtractFrame(
					fVerbose, fDumpsize, display, noframe,
					commonframe->GetPointHeader());
		}
	}
	return count;
}*/
//_______________________________________________________________________________________________________________________

bool InfoCondition() {
	bool condition = (((fNbFramesDump == 0) && (fNbFramesStart <= fNbOfEvent))
			|| ((fNbFramesStart <= fNbOfEvent)
					&& (fNbFramesStart + fNbFramesDump > fNbOfEvent)));
	return condition;
}

//_______________________________________________________________________________________________________________________
/*int ReadMergeFrame(MFMCommonFrame *commonframe, int fDumpsize, bool display,
		int noframe) {
	MFMMergeFrame *localmergeptframe;
	int type = 0;
	int count = 0;
	type = commonframe->GetFrameType();
	localmergeptframe = (MFMMergeFrame*) (fGeneral->GetFrameWithType(type));
	localmergeptframe->SetAttributs(commonframe->GetPointHeader());
	int nbinsideframe = 0;
	nbinsideframe = localmergeptframe->GetNbItems();
	localmergeptframe->TestUserPointer(noframe, fVerbose);
	int framesize = localmergeptframe->GetFrameSize();
	if (display)
		localmergeptframe->ExtractInfoFrame(fVerbose, fDumpsize, noframe);
	localmergeptframe->ResetReadInMem();
	localmergeptframe->FillStat();
	for (int i = 0; i < nbinsideframe; i++) {
		localmergeptframe->ReadInFrame(fInsideframe);
		//fInsideframe->FillStat();
		ReadUserFrame(fInsideframe);
	}
	return nbinsideframe;
}
*/
//_______________________________________________________________________________________________________________________
void WriteUserFrame(int lun, int format, int fNbFrames, int fNbSubFrames) {

	int i = 0;

	if (format == TYPE_FORMAT_MIX) {	//_____________________Mix of various frame_____________________________________________________

		fGeneral->GetFrameWithType(MFM_XML_FILE_HEADER_FRAME_TYPE)->WriteRandomFrame(
				lun, 1, fVerbose, fDumpsize, MFM_XML_FILE_HEADER_FRAME_TYPE);
		fGeneral->GetFrameWithType(MFM_XML_DATA_DESCRIPTION_FRAME_TYPE)->WriteRandomFrame(
				lun, 1, fVerbose, fDumpsize,
				MFM_XML_DATA_DESCRIPTION_FRAME_TYPE);
		for (i = 1; i < MAXINDICETYPE; i++) {
			if (fGeneral->GetFrameWithType(i) != NULL) {
				if ((i != MFM_MERGE_EN_FRAME_TYPE)
						and (i != MFM_MERGE_TS_FRAME_TYPE)
						and (i != MFM_XML_FILE_HEADER_FRAME_TYPE)
						and (i != MFM_XML_DATA_DESCRIPTION_FRAME_TYPE)) {
					fGeneral->GetFrameWithType(i)->WriteRandomFrame(lun,
							fNbFrames, fVerbose, fDumpsize, i);
				}
			}
		}
		return;
	}
	//_____________________Merge of  Ebyedat in eventnumber_____________________________________________
	if (format == TYPE_FORMAT_MERG_EBYEN) {
		int type = MFM_EBY_EN_FRAME_TYPE;
		WriteMergeFrame(lun, fNbFrames, type, fNbSubFrames);
		return;
	}
	//_____________________ Merge of  Ebyedat in  time stamp________________________________________________
	else if (format == TYPE_FORMAT_MERG_EBYTS) {
		int type = MFM_EBY_TS_FRAME_TYPE;
		WriteMergeFrame(lun, fNbFrames, type, fNbSubFrames);
		return;
	//_____________________ Merge of  cobos________________________________________________
	} else if (format == TYPE_FORMAT_MERG_COBO) {
		int type = MFM_COBO_FRAME_TYPE;
		WriteMergeFrame(lun, fNbFrames, type, fNbSubFrames);
		return;
	}

	else if (fGeneral->GetFrameWithType(format) != NULL) {
		fGeneral->GetFrameWithType(format)->WriteRandomFrame(lun, fNbFrames,
				fVerbose, fDumpsize, format);
	} else {
		cout << " Choice not good : " << format << "\n";
		return;
	}

}

//_______________________________________________________________________________________________________________________
void WriteMergeFrame(int lun, int fNbFrames, int type, int fNbSubFrames) {
	uint32_t eventnum;
	int32_t unitBlock_size;
	uint32_t mergeFramesize = 0;
	uint32_t revision = 1;
	uint32_t headersize = 0;
	uint64_t timestamp = 0;
	int verif;
	uint32_t nbitem = fNbSubFrames;
	char info[64];

	fEventNumber = 0;
	mergeFramesize = 0;
	bool cobotag = false;
	int merge_type = MFM_MERGE_EN_FRAME_TYPE;

	MFMCommonFrame *insideframe[nbitem];
	MFMMergeFrame *localmergeptframe;
	MFMCommonFrame *localinsideframe = fGeneral->GetFrameWithType(type);
	localmergeptframe = (MFMMergeFrame*) (fGeneral->GetFrameWithType(
	MFM_MERGE_TS_FRAME_TYPE));
	unitBlock_size = localmergeptframe->GetDefinedUnitBlockSize();

	if (type == MFM_EBY_TS_FRAME_TYPE) {
		headersize = MERGE_TS_HEADERSIZE;
		merge_type = MFM_MERGE_TS_FRAME_TYPE;
		strcpy(info, "--Header Ebyedat  with TS--");
	}

	if (type == MFM_EBY_EN_FRAME_TYPE) {
		headersize = MERGE_EN_HEADERSIZE;
		merge_type = MFM_MERGE_EN_FRAME_TYPE;
		strcpy(info, "-- Header Ebyedat with EN--");
	}

	if (type == MFM_EBY_EN_TS_FRAME_TYPE) {
		headersize = MERGE_TS_HEADERSIZE;
		merge_type = MFM_MERGE_TS_FRAME_TYPE;
		strcpy(info, "-- Header Ebyedat with TS and EN--");
	}
	localmergeptframe =
			(MFMMergeFrame*) (fGeneral->GetFrameWithType(merge_type));
	if (type == MFM_COBO_FRAME_TYPE) {
		cobotag = true;
		headersize = MERGE_EN_HEADERSIZE;
		merge_type = MFM_MERGE_EN_FRAME_TYPE;
		strcpy(info, "-- Header COBO --");
		for (int j = 0; j < nbitem; j++) {
			insideframe[j] = (MFMCommonFrame*) (new MFMCoboFrame());
		}
	} else {
		MFMEbyedatFrame *ebyframe2p[nbitem];
		for (int j = 0; j < nbitem; j++) {
			insideframe[j] = (MFMCommonFrame*) (new MFMEbyedatFrame());
		}
	}
	// generation of fNbFrames contents
	for (int i = 0; i < fNbFrames; i++) {
		fEventNumber++;
		mergeFramesize = headersize;
		int framesizelocal;
		uint64_t timestamp;
		timestamp = localmergeptframe->GetTimeStampUs();

		for (int j = 0; j < nbitem; j++) {
			((MFMCoboFrame*) insideframe[j])->SetWantedFrameType(type);
			((MFMCoboFrame*) insideframe[j])->GenerateAFrameExample(timestamp++,
					fEventNumber);
			((MFMCoboFrame*) insideframe[j])->SetAttributs();
			framesizelocal = ((MFMCoboFrame*) insideframe[j])->GetFrameSize();
			mergeFramesize += insideframe[j]->GetFrameSize();
			framesizelocal = insideframe[j]->GetFrameSize();
		}
		cout << "-- Merge Frame -with " << nbitem
				<< " inside frames , so begin to have a look to inside frames first------\n";
		localmergeptframe->MFM_make_header(unitBlock_size, 0, merge_type,
				revision, (int) (mergeFramesize / unitBlock_size),
				(headersize / unitBlock_size), 0, nbitem);
		if (merge_type == MFM_MERGE_TS_FRAME_TYPE)
			localmergeptframe->SetTimeStamp(timestamp);
		else
			localmergeptframe->SetEventNumber(fEventNumber);
		localmergeptframe->SetAttributs();
		localmergeptframe->ResetAdd();
		for (int j = 0; j < nbitem; j++) {
			int insframesize = insideframe[j]->GetFrameSize();
			cout << " ----> inside frame ---- ";
			localmergeptframe->AddFrame(insideframe[j]);
			insideframe[j]->ExtractInfoFrame(fVerbose, fDumpsize, j);
			localinsideframe->SetAttributs(insideframe[j]->GetPointHeader());
			localinsideframe->FillStat(); // used just for stat
		}
		localmergeptframe->FillStat();
		localmergeptframe->ExtractInfoFrame(fVerbose, fDumpsize, i);

		if (lun != 0)
			verif = write(lun, localmergeptframe->GetPointHeader(),
					mergeFramesize);
		else
			cout << " Error , file not open\n";

		if (verif != mergeFramesize)
			Error.TreatError(2, 0, "Error of write");
	}

	for (int j = 0; j < nbitem; j++) {
		if (insideframe[j])
			delete insideframe[j];
	}

}

//_______________________________________________________________________________________________________________________
void ClearVector(char *vector, int vectorsize) {
	int i;
	for (i = 0; i < vectorsize; i++)
		vector[i] = 0;
}
//______________________________________________________________________________________________________________________
void ReadDefaultFrame(MFMCommonFrame *commonframe, int readsize,
		long long filesize) {
	if (fVerbose > 1) {
		commonframe->HeaderDisplay(
				(char*) ((commonframe->WhichFrame()).c_str()));
		if (fVerbose > 3) {
			int framesize = commonframe->GetFrameSize();
			int dump = fDumpsize;
			if (framesize < dump)
				dump = framesize;
			if (dump < 4)
				dump = 4;
			if (dump > (filesize - readsize))
				dump = filesize - readsize;
			commonframe->DumpRaw(dump, 0);
		}
	}
}
//_______________________________________________________________________________________________________________________
void Statistics(bool writeorread) {

	fGeneral->Statistics(writeorread);
	fGeneral->PrintStatCount();

}
//_______________________________________________________________________________________________________________________

bool Test(int verbose) {
// 

	int type;
	int fLun; // Logical Unit Number
	char *vector;
	char **pvector;

	bool totaltest = true;
	int minsizeheader = MFM_BLOB_HEADER_SIZE; // =8
	cout << "-------------------TEST-----------------------" << endl;
//totaltest = totaltest and fCoboframe->UnitTest(verbose);
//totaltest = totaltest and fExoframe->UnitTest(verbose);
//totaltest = totaltest and fEbyframe->UnitTest(verbose);
//totaltest = totaltest and fMergeframe->UnitTest(verbose);
//totaltest = totaltest and fScalerframe->UnitTest(verbose);
//totaltest = totaltest and fOscilloframe->UnitTest(verbose);
//totaltest = totaltest and fInsideframe->UnitTest(verbose);
//totaltest = totaltest and fRibfframe->UnitTest(verbose);
//totaltest = totaltest and fMutframe->UnitTest(verbose);
//totaltest = totaltest and fDatadescriptionframe->UnitTest(verbose);
//totaltest = totaltest and fHeaderframe->UnitTest(verbose);
//totaltest = totaltest and fHelloframe->UnitTest(verbose);
//totaltest = totaltest and fChimeraframe ->UnitTest(verbose);
//totaltest = totaltest and fBoxDiagframe ->UnitTest(verbose);
//totaltest = totaltest and fVamosICframe ->UnitTest(verbose);
//totaltest = totaltest and fVamosPDframe ->UnitTest(verbose);
//totaltest = totaltest and fVamosTACframe ->UnitTest(verbose);
//totaltest = totaltest and fDiamantframe->UnitTest(verbose);
//totaltest = totaltest and fNedaframe->UnitTest(verbose);
//totaltest = totaltest and fNedaCompframe ->UnitTest(verbose);
//totaltest = totaltest and fS3BaF2frame->UnitTest(verbose);
//totaltest = totaltest and fS3Alphaframe->UnitTest(verbose);
//totaltest = totaltest and fS3Ruthframe->UnitTest(verbose);
//totaltest = totaltest and fS3eGUNframe->UnitTest(verbose);
//totaltest = totaltest and fS3Synchroframe->UnitTest(verbose);
	totaltest = totaltest
			and ((MFMReaGenericFrame*) (fGeneral->GetFrameWithType(
			MFM_REA_GENE_FRAME_TYPE)))->UnitTest(verbose);
	totaltest = totaltest and ((MFMReaTraceFrame*) (fGeneral->GetFrameWithType(
	MFM_REA_TRACE_FRAME_TYPE)))->UnitTest(verbose);
//totaltest = totaltest and fSiriusframe->UnitTest(verbose);
//totaltest = totaltest and fDeflectorframe->UnitTest(verbose);

	totaltest = totaltest and ((MFMParisFrame*) (fGeneral->GetFrameWithType(
	MFM_PARIS_FRAME_TYPE)))->UnitTest(verbose);
	cout << endl << ">>>>TOTAL TEST  = " << totaltest << endl;

	return (totaltest);
}

