#include <iostream>
using namespace std;
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "stdio.h"
#include "stdlib.h"
#include <iostream> 
#ifndef NO_MFMXML
#include <tinyxml.h>
#endif
#include "MFMAllFrames.h"
#include "CUtilities.h"
MFMCommonFrame *  fFrame;
MFMSiriusFrame *  fSiriusframe;


const char * filename = "mfmfile.dat";
int fVerbose=5;
int fDumpsize =32;

void ReadUserFrame(MFMCommonFrame* commonframe);
void ReadDefaultFrame(MFMCommonFrame* commonframe) ;
void ReadSiriusFrame(MFMCommonFrame* commonframe) ;
//_________________________________________________________________________________________
int main(int argc, char **argv) {

  fFrame       = new MFMCommonFrame();
  fSiriusframe = new MFMSiriusFrame();
  int fLun = 0; // Logical Unit Number
  fLun = open(filename, (O_RDONLY));
  long long fPtInFile =0;
  long long fNbOfEvent=0;
  char* vector;
  char ** pvector;
  
  int vectorsize = 8; // min_sizeheader		
  
  vector = (char*) (malloc(vectorsize));
  pvector = &vector;
  
  UtilVector_c utilvector(vectorsize);
  int framesize = 0;
  
  if (fLun <= 0) {
	printf("Error open file : %s", filename);
  	}
  cout << endl;
  cout << "-------------------------------------------------------------" << endl;
  cout << "     test " ;
  cout << "|   READ MFMFrame from file : " << filename << "" << endl;
  cout << "-------------------------------------------------------------" << endl;
  cout << endl;

   while (true) {
     cout << "-------------------------------------------------------------" << endl;
     
     	 // to get frame from file , you can chose one of these 3 possibilities in function of yours needs
         //framesize = fFrame->ReadInFile(&fLun, pvector, &vectorsize); 
         //framesize = fFrame->ReadInFile(&fLun, &utilvector); 
         framesize = fFrame->ReadInFile(&fLun); 
         
         fPtInFile+=framesize;
         if (framesize <= 0) break;
         ReadUserFrame(fFrame); // give infomation on frame ( header information and first memory dump)
         fNbOfEvent++;
         fFrame->SetAttributs();
	 }

  fLun = close(fLun);

  cout << "-------------------End--------with "<<fNbOfEvent<<" frames----------------" << endl;
  if (fFrame)  delete (fFrame);
  if (fSiriusframe)     delete  (fSiriusframe) ;
  return (0);
}

//_______________________________________________________________________________________________________________________

void ReadUserFrame(MFMCommonFrame* commonframe) {
	commonframe->SetAttributs();

	int type = commonframe->GetFrameType();
	// part of use of frame
	// example
       
	switch (type) {

	case MFM_COBOF_FRAME_TYPE:
	case MFM_COBO_FRAME_TYPE: {
	        printf(" It is a Cobo frame\n");
		break;
	}
	case MFM_EBY_EN_FRAME_TYPE:
	case MFM_EBY_TS_FRAME_TYPE:
	case MFM_EBY_EN_TS_FRAME_TYPE: {
		printf(" It is a Eby frame\n");
		break;
	}
	case MFM_HELLO_FRAME_TYPE: {
		printf(" It is a Hello frame\n");
		break;
	}
	case MFM_XML_FILE_HEADER_FRAME_TYPE: {
		printf(" It is a Header information frame\n");
		break;
	}

	case MFM_REA_TRACE_FRAME_TYPE: {
		printf(" It is a Header information frame\n");
		break;
	}
	case MFM_REA_GENE_FRAME_TYPE: {
		printf(" It is a Header information frame\n");
		//ReadDefaultFrame(commonframe);
		break;
	}
	case MFM_SIRIUS_FRAME_TYPE: {
	       
		ReadSiriusFrame(commonframe);
		break;
	}
	
	default: {
		printf(" It is a other frame\n");
		break;
	}

	}// end of switch
}



//______________________________________________________________________________________________________________________
void ReadDefaultFrame(MFMCommonFrame* commonframe) {
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
			commonframe->DumpRaw(dump, 0);
		}
	}
}
//______________________________________________________________________________________________________________________
void ReadSiriusFrame(MFMCommonFrame* commonframe) {
        int i=0;
        uint16_t value;
	fSiriusframe->SetAttributs(commonframe->GetPointHeader());

	if (fVerbose > 1) {
		fSiriusframe->HeaderDisplay(
				(char*) ((fSiriusframe->WhichFrame()).c_str()));
		if (fVerbose > 3) {
			int framesize = fSiriusframe->GetFrameSize();
			int dump = fDumpsize;
			if (framesize < dump)
				dump = framesize;
			if (dump < 4)
				dump = 4;
			fSiriusframe->DumpRaw(dump, 0);
		}
				
	}
	//display the 10 first value of trace in sirius frame 
	for (i=0;i<10;i++) {
		        fSiriusframe->GetParameters(i,&value);
			printf (" Value[%d]=%d \n",i, value);
			}
}
//______________________________________________________________________________________________________________________
