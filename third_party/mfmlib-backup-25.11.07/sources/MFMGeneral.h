#ifndef _MFMGeneral_
#define _MFMGeneral_

#include "MFMAllFrames.h"


class MFMGeneral
{

MFMCommonFrame *   fFrame;
MFMCommonFrame*    fInsideframe;
MFMCommonFrame **  fFrametab;

MFMCoboFrame *     fCoboframe;
MFMCoboFrame *     fCobofframe;
MFMCoboTopoFrame * fCobotopoframe;

MFMExogamFrame *   fExoframe;
MFMDiamantFrame *  fDiamantframe;
MFMEbyedatFrame *  fEbyframeN;
MFMEbyedatFrame *  fEbyframeT;
MFMEbyedatFrame *  fEbyframeNT;
MFMMergeFrame *    fMergeframeN;
MFMMergeFrame *    fMergeframeT;
MFMOscilloFrame *  fOscilloframe;
MFMScalerDataFrame * fScalerframe;
MFMRibfFrame *     fRibfframe;
MFMMutantFrame *   fMutframe;
MFMHelloFrame *    fHelloframe;
MFMXmlDataDescriptionFrame * fDatadescriptionframe;
MFMXmlFileHeaderFrame *      fHeaderframe;
MFMChimeraFrame *  fChimeraframe;
MFMBoxDiagFrame *  fBoxDiagframe;
MFMVamosICFrame *  fVamosICframe;
MFMVamosPDFrame *  fVamosPDframe;
MFMVamosTACFrame * fVamosTACframe;
MFMNedaFrame *     fNedaframe;
MFMNedaCompFrame * fNedaCompframe;
MFMS3BaF2Frame *   fS3BaF2frame;
MFMS3AlphaFrame *  fS3Alphaframe;
MFMS3RuthFrame *   fS3Ruthframe;
MFMS3eGUNFrame *   fS3eGUNframe;
MFMReaGenericFrame *  fReaGenericframe;
MFMReaTraceFrame *    fReaTraceframe;
MFMS3SynchroFrame *   fS3Synchroframe;
MFMSiriusFrame *      fSiriusframe;
MFMS3DeflectorFrame * fDeflectorframe;
MFMParisFrame *       fParisframe;
MFMMesytecFrame *     fMesytecframe;
MFMFaziaFrame *       fFaziaframe;
MFMFasterFrame *      fFasterframe;
MFMFasterDTSFrame *   fFasterDTSframe;
int fVerbose ;
int fCount_elseframe ;
int fReadSize ;
long long fNbOfEvent;
public:
MFMGeneral();
virtual ~MFMGeneral();
void InitStat();
void read();
void stat() ;
void SetVerbose(int verb) { fVerbose = verb;};
MFMCommonFrame* GetFrameWithType(int type) { return  fFrametab[type];};
void ReadUserFrame(MFMCommonFrame* commonframe,int dumpsize, bool display,int noframe);
void ReadMergeFrame(MFMCommonFrame* commonframe,int dumpsize, bool display,int noframe);
int  ReadFrame(char* buff ,int dumpsize, int buffsize );
void Statistics(bool writeorread = false);
string GetStatCount()const;
void PrintStatCount() const;
};

#endif
