// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GEventMFM
//
// This class manage event in Ganil format
// The associated methods do dump....
//
/////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#include <string>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <dirent.h>
#include "math.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <TROOT.h>
#include <TSystem.h>
#include <unistd.h>
#include "General.h"
#include "GEventMFM.h"
#include "GBuffer.h"
#include "GBufferMFM.h"
#include "Cobo.h"

extern "C" {
#include "GEN_TYPE.H"
#include "STR_EVT.H"
#include "ERR_GAN.H"
#include "EQUIPDES.H"
#include "acq_swap_buf.h"
#include "gan_acq_buf.h"
#include "acq_codes_erreur.h"
#include "math.h"
}

#include <memory> // auto_ptr
#include <iostream>
#include <fstream>
#include <cstdlib>

//_______________________________________________________________________________
ClassImp( GEventMFM);

GEventMFM::GEventMFM(DataParameters* parameter) :
	GEventBase(parameter) {
	//Constructor
	fStatus = ACQ_OK;
	fEventNumber = -1;
	fEventCount = 0;
	fTimeStamp = 0;

	fLaw = 0;
	fProba = 0.6;
	SetRandomLaw(fLaw);
	SetRandomProba(fProba);
	fLipflop = true;
	pCurrentFrame     = NULL;
	pBasicFrame       = new MFMBasicFrame();
	pCoboFrame        = new MFMCoboFrame();
	pExogamFrame      = new MFMExogamFrame();
	pCommonFrame      = new MFMCommonFrame();
	pEbyedatFrame     = new MFMEbyedatFrame();
	pOscilloFrame     = new MFMOscilloFrame();
	pScalerFrame      = new MFMScalerDataFrame();
	pMergeFrame       = new MFMMergeFrame();
	pRibfFrame        = new MFMRibfFrame();
	pMutantFrame      = new MFMMutantFrame();
	pHelloFrame       = new MFMHelloFrame();
	pChimeraFrame     = new MFMChimeraFrame();
	pBoxDiagFrame     = new MFMBoxDiagFrame();
	pVamosICFrame     = new MFMVamosICFrame();
	pVamosPDFrame     = new MFMVamosPDFrame();
	pVamosTACFrame    = new MFMVamosTACFrame();
	pNedaFrame        = new MFMNedaFrame();
	pNedaCompFrame    = new MFMNedaCompFrame();

	pS3BaF2Frame      = new MFMS3BaF2Frame();
	pS3AlphaFrame     = new MFMS3AlphaFrame();
	pS3RuthFrame      = new MFMS3RuthFrame();
	pS3eGUNFrame      = new MFMS3eGUNFrame();
	pS3SynchroFrame   = new MFMS3SynchroFrame();
	pDiamantFrame     = new MFMDiamantFrame();
	pReaGenericFrame  = new MFMReaGenericFrame();
	pReaTraceFrame    = new MFMReaTraceFrame();
	pSiriusFrame      = new MFMSiriusFrame();
	pDeflectorFrame   = new MFMS3DeflectorFrame();
	pParisFrame       = new MFMParisFrame();

}
//_______________________________________________________________________________
GEventMFM::~GEventMFM() {
	//destructor
	if (pBasicFrame) {
		delete (pBasicFrame);
		pBasicFrame = NULL;
	}
	if (pCoboFrame) {
		delete (pCoboFrame);
		pCoboFrame = NULL;
	}
	if (pExogamFrame) {
		delete (pExogamFrame);
		pExogamFrame = NULL;
	}
	if (pCommonFrame) {
		delete (pCommonFrame);
		pCommonFrame = NULL;
	}
	if (pEbyedatFrame) {
		delete (pEbyedatFrame);
		pEbyedatFrame = NULL;
	}
	if (pOscilloFrame) {
		delete (pOscilloFrame);
		pOscilloFrame = NULL;
	}
	if (pMergeFrame) {
		delete (pMergeFrame);
		pMergeFrame = NULL;
	}
	if (pRibfFrame) {
		delete (pRibfFrame);
		pRibfFrame = NULL;
	}
	if (pScalerFrame) {
		delete (pScalerFrame);
		pScalerFrame = NULL;
	}
	if (pHelloFrame) {
		delete (pHelloFrame);
		pHelloFrame = NULL;
	}
	if (pChimeraFrame) {
		delete (pChimeraFrame);
		pChimeraFrame = NULL;
	}
	if (pBoxDiagFrame) {
		delete (pBoxDiagFrame);
		pBoxDiagFrame = NULL;
	}
	if (pVamosICFrame) {
		delete (pVamosICFrame);
		pVamosICFrame = NULL;
	}
	if (pVamosPDFrame) {
		delete (pVamosPDFrame);
		pVamosPDFrame = NULL;
	}
	if (pVamosTACFrame) {
		delete (pVamosTACFrame);
		pVamosTACFrame = NULL;
	}
	if (pDiamantFrame) {
		delete (pDiamantFrame);
		pDiamantFrame = NULL;
	}
	if (pS3BaF2Frame) {
		delete (pS3BaF2Frame);
		pS3BaF2Frame = NULL;
	}
	if (pS3AlphaFrame) {
		delete (pS3AlphaFrame);
		pS3AlphaFrame = NULL;
	}
	if (pS3RuthFrame) {
		delete (pS3RuthFrame);
		pS3RuthFrame = NULL;
	}
	if (pS3eGUNFrame) {
		delete (pS3eGUNFrame);
		pS3eGUNFrame = NULL;
	}
	if (pS3SynchroFrame) {
		delete (pS3SynchroFrame);
		pS3SynchroFrame = NULL;
	}
	if (pNedaFrame) {
		delete (pNedaFrame);
		pNedaFrame = NULL;
	}
	if (pNedaCompFrame) {
		delete (pNedaCompFrame);
		pNedaCompFrame = NULL;
	}
	if (pReaGenericFrame) {
		delete (pReaGenericFrame);
		pReaGenericFrame = NULL;
	}
	if (pReaTraceFrame) {
		delete (pReaTraceFrame);
		pReaTraceFrame = NULL;
	}
	if (pSiriusFrame) {
		delete (pSiriusFrame);
		pSiriusFrame = NULL;
	}
	if (pMutantFrame) {
		delete (pMutantFrame);
		pMutantFrame = NULL;
	}
	if (pDeflectorFrame) {
		delete (pDeflectorFrame);
		pDeflectorFrame = NULL;
	}
	if (pParisFrame) {
		delete (pParisFrame);
		pParisFrame = NULL;
	}
}
//______________________________________________________________________________
int GEventMFM::NextEvent(GBuffer* _buffer) {
	// Read an event from  buffer _buffer
	// return status

	fStatus = ACQ_OK;
	TString tempos;
	//cout <<"_buffer = " << _buffer<<"\n";
	ReadNextEvent(_buffer);
	return (fStatus);
}
//______________________________________________________________________________
int GEventMFM::ReadNextEvent(GBuffer* _buffer) {
	fStatus = ACQ_OK;
	fEventReadSize = 0;
	fEvt_increment = 0;
	fEventNumber = 0;
	fTimeStamp = 0;

	if (pEventBrut_char != NULL) {
		pEventBrut_char = NULL;
		fStatus = ACQ_ENDOFBUFFER;
	} else {
		fEventReadSize = _buffer->GetReadSize(); // Size of the brut data buffer
		fEvbsize = fEventReadSize;
		pEventBrut_char = _buffer->GetBufDataChar();
		pCommonFrame->SetAttributs(pEventBrut_char);
		//fFrameType = pCommonFrame->TestType(pEventBrut_char);
		fFrameType = pCommonFrame->GetFrameType();
		pCurrentFrame = NULL;

		switch (fFrameType) {

		case MFM_COBOF_FRAME_TYPE:
		case MFM_COBO_FRAME_TYPE: {
			pCurrentFrame = pCoboFrame;
			pCoboFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pCoboFrame->GetEventNumber();
			fTimeStamp = pCoboFrame->GetTimeStamp();
			break;
		}
		case MFM_EXO2_FRAME_TYPE: {
			pCurrentFrame = pExogamFrame;
			pExogamFrame->SetAttributs((void*) pEventBrut_char);
			pExogamFrame->GetEventNumber();
			fEventNumber = pExogamFrame->GetEventNumber();
			pExogamFrame->GetTimeStamp();
			fTimeStamp = pExogamFrame->GetTimeStamp();
			break;
		}
		case MFM_EBY_EN_FRAME_TYPE:
		case MFM_EBY_TS_FRAME_TYPE:
		case MFM_EBY_EN_TS_FRAME_TYPE: {
			pCurrentFrame = pEbyedatFrame;
			pEbyedatFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pEbyedatFrame->GetEventNumber();
			fTimeStamp = pEbyedatFrame->GetTimeStamp();
			EventUnravelling();
			break;
		}
		case MFM_OSCI_FRAME_TYPE: {
			pCurrentFrame = pOscilloFrame;
			pOscilloFrame->SetAttributs((void*) pEventBrut_char);
			fTimeStamp = 0;
			fTimeStamp = 0;
			break;
		}
		case MFM_MERGE_TS_FRAME_TYPE:
		case MFM_MERGE_EN_FRAME_TYPE: {
			pCurrentFrame = pMergeFrame;
			pMergeFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pMergeFrame->GetEventNumber();
			fTimeStamp = pMergeFrame->GetTimeStamp();
			break;
		}
		case MFM_RIBF_DATA_FRAME_TYPE: {
			pCurrentFrame = pRibfFrame;
			pRibfFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pRibfFrame->GetEventNumber();
			fTimeStamp = pRibfFrame->GetTimeStamp();
			break;
		}
		case MFM_MUTANT_FRAME_TYPE: {
			pCurrentFrame = pMutantFrame;
			pMutantFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pMutantFrame->GetEventNumber();
			fTimeStamp = pMutantFrame->GetTimeStamp();
			break;
		}
		case MFM_SCALER_DATA_FRAME_TYPE: {
			pCurrentFrame = pScalerFrame;
			pScalerFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pScalerFrame->GetEventNumber();
			fTimeStamp = pScalerFrame->GetTimeStamp();
			break;
		}

		case MFM_HELLO_FRAME_TYPE: {
			pCurrentFrame = pHelloFrame;
			pHelloFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pHelloFrame->GetEventNumber();
			fTimeStamp = pHelloFrame->GetTimeStamp();
			break;
		}
		case MFM_CHIMERA_DATA_FRAME_TYPE: {
			pCurrentFrame = pChimeraFrame;
			pChimeraFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pChimeraFrame->GetEventNumber();
			fTimeStamp = pChimeraFrame->GetTimeStamp();
			break;
		}
		case MFM_BOX_DIAG_FRAME_TYPE: {
			pCurrentFrame = pBoxDiagFrame;
			pBoxDiagFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pBoxDiagFrame->GetEventNumber();
			fTimeStamp =   pBoxDiagFrame->GetTimeStamp();
			break;
		}
		case MFM_VAMOSIC_FRAME_TYPE: {
			pCurrentFrame = pVamosICFrame;
			pVamosICFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pVamosICFrame->GetEventNumber();
			fTimeStamp = pVamosICFrame->GetTimeStamp();
			break;
		}
		case MFM_VAMOSPD_FRAME_TYPE: {
			pCurrentFrame = pVamosPDFrame;
			pVamosPDFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pVamosPDFrame->GetEventNumber();
			fTimeStamp = pVamosPDFrame->GetTimeStamp();
			break;
		}
		case MFM_VAMOSTAC_FRAME_TYPE: {
			pCurrentFrame = pVamosTACFrame;
			pVamosTACFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pVamosTACFrame->GetEventNumber();
			fTimeStamp = pVamosTACFrame->GetTimeStamp();
			break;
		}
		case MFM_NEDA_FRAME_TYPE: {
			pCurrentFrame = pNedaFrame;
			pNedaFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pNedaFrame->GetEventNumber();
			fTimeStamp = pNedaFrame->GetTimeStamp();
			break;
		}
		case MFM_NEDACOMP_FRAME_TYPE: {
			pCurrentFrame = pNedaCompFrame;
			pNedaCompFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pNedaCompFrame->GetEventNumber();
			fTimeStamp = pNedaCompFrame->GetTimeStamp();
			break;
		}
		case MFM_DIAMANT_FRAME_TYPE: {
			pCurrentFrame = pDiamantFrame;
			pDiamantFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pDiamantFrame->GetEventNumber();
			fTimeStamp = pDiamantFrame->GetTimeStamp();
			break;
		}
		case MFM_S3_BAF2_FRAME_TYPE: {
			pCurrentFrame = pS3BaF2Frame;
			pS3BaF2Frame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pS3BaF2Frame->GetEventNumber();
			fTimeStamp = pS3BaF2Frame->GetTimeStamp();
			break;
		}
		case MFM_S3_ALPHA_FRAME_TYPE:
		{
			pCurrentFrame = pS3AlphaFrame;
			pS3AlphaFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pS3AlphaFrame->GetEventNumber();
			fTimeStamp = pS3AlphaFrame->GetTimeStamp();
			break;
		}
		case MFM_S3_EGUN_FRAME_TYPE:
		{
			pCurrentFrame = pS3eGUNFrame;
			pS3eGUNFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pS3eGUNFrame->GetEventNumber();
			fTimeStamp = pS3eGUNFrame->GetTimeStamp();
			break;
		}
		case MFM_S3_RUTH_FRAME_TYPE:
		{
			pCurrentFrame = pS3RuthFrame;
			pS3RuthFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pS3RuthFrame->GetEventNumber();
			fTimeStamp = pS3RuthFrame->GetTimeStamp();
			break;
		}
		case MFM_S3_SYNC_FRAME_TYPE:
		{
			pCurrentFrame = pS3SynchroFrame;
			pS3SynchroFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pS3SynchroFrame->GetEventNumber();
			fTimeStamp = pS3SynchroFrame->GetTimeStamp();
			break;
		}
		case MFM_REA_GENE_FRAME_TYPE:
		{
			pCurrentFrame = pReaGenericFrame;
			pReaGenericFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pReaGenericFrame->GetEventNumber();
			fTimeStamp = pReaGenericFrame->GetTimeStamp();
			break;
		}
		case MFM_REA_TRACE_FRAME_TYPE:
		{
			pCurrentFrame = pReaTraceFrame;
			pReaTraceFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pReaTraceFrame->GetEventNumber();
			fTimeStamp = pReaTraceFrame->GetTimeStamp();
			break;
		}
		case MFM_SIRIUS_FRAME_TYPE:
		{
			pCurrentFrame = pSiriusFrame;
			pSiriusFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pSiriusFrame->GetEventNumber();
			fTimeStamp = pSiriusFrame->GetTimeStamp();
			break;
		}
		case MFM_S3_DEFLECTOR_FRAME_TYPE:
		{
			pCurrentFrame = pDeflectorFrame;
			pDeflectorFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pDeflectorFrame->GetEventNumber();
			fTimeStamp = pDeflectorFrame->GetTimeStamp();
			break;
		}
		case MFM_PARIS_FRAME_TYPE:
		{
			pCurrentFrame = pParisFrame;
			pParisFrame->SetAttributs((void*) pEventBrut_char);
			fEventNumber = pParisFrame->GetEventNumber();
			fTimeStamp = pParisFrame->GetTimeStamp();
			break;
		}
		default: {
			pCurrentFrame = pCommonFrame;
			fEventNumber = 0;
			fTimeStamp = 0;
			break;
		}

		}// end of switch
		if (fFrameType != MFM_SCALER_DATA_FRAME_TYPE)
			_buffer->SetNumBuf(fEventNumber);
	}

	return fStatus;
}
//______________________________________________________________________________
int GEventMFM::GetNbItems() {
	if ((fFrameType == MFM_COBO_FRAME_TYPE) or (fFrameType
			== MFM_COBOF_FRAME_TYPE)) {
		return ((MFMCoboFrame*) pCurrentFrame)->GetNbItems();
	}
	if (fFrameType == 0) {
		fError.TreatError(3, 0, "This MFM Type can't be used!");
		return ((MFMBasicFrame*) pCurrentFrame)->GetNbItems();
	}
	if ((fFrameType == MFM_EBY_TS_FRAME_TYPE) or (fFrameType
			== MFM_EBY_EN_FRAME_TYPE) or (fFrameType
			== MFM_EBY_EN_TS_FRAME_TYPE)) {
		return ((MFMEbyedatFrame*) pCurrentFrame)->GetNbItems();
	}
	if ((fFrameType == MFM_MERGE_TS_FRAME_TYPE) || (fFrameType
			== MFM_MERGE_EN_FRAME_TYPE)) {
		pCurrentFrame = pMergeFrame;
		return ((MFMMergeFrame*) pCurrentFrame)->GetNbItems();
	}
	if ((fFrameType == MFM_SCALER_DATA_FRAME_TYPE)) {
		pCurrentFrame = pScalerFrame;
		return ((MFMScalerDataFrame*) pCurrentFrame)->GetNbItems();
	}
	if ((fFrameType == MFM_OSCI_FRAME_TYPE)) {
		pCurrentFrame = pOscilloFrame;
		return ((MFMOscilloFrame*) pCurrentFrame)->GetNbItems();
	}
	if ((fFrameType == MFM_NEDA_FRAME_TYPE)) {
		pCurrentFrame = pNedaFrame;
		return ((MFMNedaFrame*) pCurrentFrame)->GetNbItems();
	}
	if ((fFrameType == MFM_REA_TRACE_FRAME_TYPE)) {
		pCurrentFrame = pReaTraceFrame;
		return ((MFMReaTraceFrame*) pCurrentFrame)->GetNbItems();
	}
	if ((fFrameType == MFM_SIRIUS_FRAME_TYPE)) {
		pCurrentFrame = pSiriusFrame;
		return ((MFMSiriusFrame*) pCurrentFrame)->GetNbItems();
	}

	return 0;
}
//______________________________________________________________________________
long long GEventMFM::GetTimeStamp() {
	fTimeStamp = 0;
	if ((fFrameType == MFM_COBO_FRAME_TYPE) or (fFrameType
			== MFM_COBOF_FRAME_TYPE)) {
		fTimeStamp = ((MFMCoboFrame*) pCurrentFrame)->GetTimeStamp();
	}

	if (fFrameType == MFM_EXO2_FRAME_TYPE) {
		fTimeStamp = ((MFMExogamFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_EBY_TS_FRAME_TYPE) or (fFrameType
			== MFM_EBY_EN_FRAME_TYPE) or (fFrameType
			== MFM_EBY_EN_TS_FRAME_TYPE)) {
		fTimeStamp = ((MFMEbyedatFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_MERGE_TS_FRAME_TYPE) || (fFrameType
			== MFM_MERGE_EN_FRAME_TYPE)) {
		fTimeStamp = ((MFMMergeFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_RIBF_DATA_FRAME_TYPE)) {
		fTimeStamp = ((MFMRibfFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_MUTANT_FRAME_TYPE)) {
		fTimeStamp = ((MFMMutantFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_SCALER_DATA_FRAME_TYPE)) {
		pCurrentFrame = pScalerFrame;
		return ((MFMScalerDataFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_CHIMERA_DATA_FRAME_TYPE)) {
		pCurrentFrame = pChimeraFrame;
		return ((MFMChimeraFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_BOX_DIAG_FRAME_TYPE)) {
		pCurrentFrame = pBoxDiagFrame;
		return ((MFMBoxDiagFrame*) pCurrentFrame)->GetTimeStamp();
		}
	if ((fFrameType == MFM_VAMOSIC_FRAME_TYPE)) {
		pCurrentFrame = pVamosICFrame;
		return ((MFMVamosICFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_VAMOSPD_FRAME_TYPE)) {
		pCurrentFrame = pVamosPDFrame;
		return ((MFMVamosPDFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_VAMOSTAC_FRAME_TYPE)) {
		pCurrentFrame = pVamosTACFrame;
		return ((MFMVamosTACFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_NEDA_FRAME_TYPE)) {
		pCurrentFrame = pNedaFrame;
		return ((MFMNedaFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_NEDACOMP_FRAME_TYPE)) {
		pCurrentFrame = pNedaCompFrame;
		return ((MFMNedaCompFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_DIAMANT_FRAME_TYPE)) {
		pCurrentFrame = pDiamantFrame;
		return ((MFMDiamantFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_S3_BAF2_FRAME_TYPE)) {
		pCurrentFrame = pS3BaF2Frame;
		return ((MFMS3BaF2Frame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_S3_ALPHA_FRAME_TYPE)) {
		pCurrentFrame = pS3AlphaFrame;
		return ((MFMS3AlphaFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_S3_EGUN_FRAME_TYPE)) {
		pCurrentFrame = pS3eGUNFrame;
		return ((MFMS3eGUNFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_S3_RUTH_FRAME_TYPE)) {
		pCurrentFrame = pS3RuthFrame;
		return ((MFMS3RuthFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType == MFM_S3_SYNC_FRAME_TYPE)) {
		pCurrentFrame = pS3SynchroFrame;
		return ((MFMS3SynchroFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType ==  MFM_REA_GENE_FRAME_TYPE)){
		pCurrentFrame = pReaGenericFrame;
		return ((MFMReaGenericFrame*)pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType ==  MFM_REA_TRACE_FRAME_TYPE)){
		pCurrentFrame = pReaTraceFrame;
		return ((MFMReaTraceFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType ==  MFM_SIRIUS_FRAME_TYPE)){
		pCurrentFrame = pReaTraceFrame;
		return ((MFMSiriusFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType ==  MFM_S3_DEFLECTOR_FRAME_TYPE)){
		pCurrentFrame = pDeflectorFrame;
		return ((MFMS3DeflectorFrame*) pCurrentFrame)->GetTimeStamp();
	}
	if ((fFrameType ==  MFM_PARIS_FRAME_TYPE)){
		pCurrentFrame = pParisFrame;
		return ((MFMParisFrame*) pCurrentFrame)->GetTimeStamp();
	}	
	return fTimeStamp;
}
//______________________________________________________________________________
Int_t GEventMFM::GetEventNumber() {
	fEventNumber = 0;
	if ((fFrameType == MFM_COBO_FRAME_TYPE) or (fFrameType
			== MFM_COBOF_FRAME_TYPE)) {
		fEventNumber = ((MFMCoboFrame*) pCurrentFrame)->GetEventNumber();
	}
	if (fFrameType == MFM_EXO2_FRAME_TYPE) {
		fEventNumber = ((MFMExogamFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_EBY_TS_FRAME_TYPE) or (fFrameType
			== MFM_EBY_EN_FRAME_TYPE) or (fFrameType
			== MFM_EBY_EN_TS_FRAME_TYPE)) {
		fEventNumber = ((MFMEbyedatFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_MERGE_TS_FRAME_TYPE) || (fFrameType
			== MFM_MERGE_EN_FRAME_TYPE)) {
		fEventNumber = ((MFMMergeFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_RIBF_DATA_FRAME_TYPE)) {
		fEventNumber = ((MFMRibfFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_MUTANT_FRAME_TYPE)) {
		fEventNumber = ((MFMMutantFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_SCALER_DATA_FRAME_TYPE)) {
		pCurrentFrame = pScalerFrame;
		return ((MFMScalerDataFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_CHIMERA_DATA_FRAME_TYPE)) {
		pCurrentFrame = pChimeraFrame;
		return ((MFMChimeraFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_BOX_DIAG_FRAME_TYPE)) {
		pCurrentFrame = pBoxDiagFrame;
		return ((MFMBoxDiagFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_VAMOSIC_FRAME_TYPE)) {
		pCurrentFrame = pVamosICFrame;
		return ((MFMVamosICFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_VAMOSPD_FRAME_TYPE)) {
		pCurrentFrame = pVamosPDFrame;
		return ((MFMVamosPDFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_VAMOSTAC_FRAME_TYPE)) {
		pCurrentFrame = pVamosTACFrame;
		return ((MFMVamosTACFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_NEDA_FRAME_TYPE)) {
		pCurrentFrame = pNedaFrame;
		return ((MFMNedaFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_NEDACOMP_FRAME_TYPE)) {
		pCurrentFrame = pNedaCompFrame;
		return ((MFMNedaCompFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_DIAMANT_FRAME_TYPE)) {
		pCurrentFrame = pDiamantFrame;
		return ((MFMDiamantFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_S3_BAF2_FRAME_TYPE)) {
		pCurrentFrame = pS3BaF2Frame;
		return ((MFMS3BaF2Frame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_S3_ALPHA_FRAME_TYPE)) {
		pCurrentFrame = pS3AlphaFrame;
		return ((MFMS3AlphaFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_S3_EGUN_FRAME_TYPE)) {
		pCurrentFrame = pS3eGUNFrame;
		return ((MFMS3eGUNFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_S3_RUTH_FRAME_TYPE)) {
		pCurrentFrame = pS3RuthFrame;
		return ((MFMS3RuthFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType == MFM_S3_SYNC_FRAME_TYPE)) {
		pCurrentFrame = pS3SynchroFrame;
		return ((MFMS3SynchroFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType ==  MFM_REA_GENE_FRAME_TYPE)){
		pCurrentFrame = pReaGenericFrame;
		return ((MFMReaGenericFrame*)pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType ==  MFM_REA_TRACE_FRAME_TYPE)){
		pCurrentFrame = pReaTraceFrame;
		return ((MFMReaTraceFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType ==  MFM_SIRIUS_FRAME_TYPE)){
		pCurrentFrame = pSiriusFrame;
		return ((MFMSiriusFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType ==  MFM_S3_DEFLECTOR_FRAME_TYPE)){
		pCurrentFrame = pDeflectorFrame;
		return ((MFMS3DeflectorFrame*) pCurrentFrame)->GetEventNumber();
	}
	if ((fFrameType ==  MFM_PARIS_FRAME_TYPE)){
		pCurrentFrame = pParisFrame;
		return ((MFMParisFrame*) pCurrentFrame)->GetEventNumber();
	}
	return fEventNumber;
}
//______________________________________________________________________________
void GEventMFM::RazEvent() {
	fEventNumber = 0;
	fStatus = ACQ_OK;
	fEventReadSize = 0;
	fEvt_increment = 0;
	fEventNumber = 0;
	fTimeStamp = 0;
	pEventBrut_char = NULL;
}

//______________________________________________________________________________
TString GEventMFM::GetDumpHeader() {
	TString mydump = " no dump with GetDumpHeader\n";
	TString tempos;
	//TODO
	return mydump;
}
//______________________________________________________________________________
TString GEventMFM::GetDumpEvent(char mode) {
	// Dump Event
	TString stretrun;
	stretrun = pCurrentFrame->GetDumpRaw(16, 0);
	return stretrun;
}

//______________________________________________________________________________
void GEventMFM::ClearData(int value) {
	//Clear Physical Data  Array or give a fixe value ( default =0);
	GEventBase::ClearData();

}
//______________________________________________________________________________
int GEventMFM::GetArrayLabelValueSize() {
	int type = 0;
	int size = 0;
	MFMCommonFrame* frame;
	MFMEbyedatFrame ebyframe;
	frame = GetFrame();
	type = GetFrameType();
	if ((type == MFM_EBY_EN_FRAME_TYPE) or (type == MFM_EBY_TS_FRAME_TYPE)
			or (type == MFM_EBY_EN_TS_FRAME_TYPE)) {
		ebyframe.SetAttributs(frame->GetPointHeader());
		size = ebyframe.GetNbItems();
		//cout <<" GEventMFM::GetArrayLabelValueSize() "<< size<<endl;
	}
	return size * 2;
}
//______________________________________________________________________________
UShort_t GEventMFM::GetArrayLabelValue_Label(UShort_t position) {
	int type = 0;
	UShort_t label = 0;
	UShort_t value = 0;
	MFMCommonFrame* frame;
	MFMEbyedatFrame ebyframe;
	frame = GetFrame();
	type = GetFrameType();
	if ((type == MFM_EBY_EN_FRAME_TYPE) or (type == MFM_EBY_TS_FRAME_TYPE)
			or (type == MFM_EBY_EN_TS_FRAME_TYPE)) {
		ebyframe.SetAttributs(frame->GetPointHeader());
		ebyframe.EbyedatGetParameters(position, &label, &value);
	}
	return label;
}
//______________________________________________________________________________
UShort_t GEventMFM::GetArrayLabelValue_Value(UShort_t position) {
	MFMCommonFrame* frame;
	int type = 0;
	UShort_t label = 0;
	UShort_t value = 0;
	MFMEbyedatFrame ebyframe;
	frame = GetFrame();
	type = GetFrameType();
	if ((type == MFM_EBY_EN_FRAME_TYPE) or (type == MFM_EBY_TS_FRAME_TYPE)
			or (type == MFM_EBY_EN_TS_FRAME_TYPE)) {
		ebyframe.SetAttributs(frame->GetPointHeader());
		ebyframe.EbyedatGetParameters(position, &label, &value);
	}
	return value;
}
//______________________________________________________________________________
int GEventMFM::EventUnravelling(void) {
	//to do
	// If mode is variable length event, we have to reconstruct the Data buffer
	// from the given event.
	// WARNING: temporary the default: we dont check that it's really the case
	TString tempos;
	Int_t i;

	fStatus = ACQ_OK;

	int nb_item = 0;

	int type = 0;
	uint16_t label, value;
	MFMCommonFrame* frame;
	MFMEbyedatFrame ebyframe;
	frame = GetFrame();
	type = GetFrameType();
	ClearData();
	if ((type == MFM_EBY_EN_FRAME_TYPE) or (type == MFM_EBY_TS_FRAME_TYPE)
			or (type == MFM_EBY_EN_TS_FRAME_TYPE)) {
		ebyframe.SetAttributs(frame->GetPointHeader());
		//ebyframe.HeaderDisplay();
		//cout << " Couples of label/value : \n" << (ebyframe.GetDumpData())<<flush;
		nb_item = ebyframe.GetNbItems();
		for (i = 0; i < nb_item; i++) {
			ebyframe.EbyedatGetParameters(i, &label, &value);
			//cout << "Debug  GEventMFM::EventUnravelling  noitem "<<i<<"  index  "<< pParameter->GetIndex(label) << " value "<< value <<"\n";
			GetArray()[pParameter->GetIndex(label)] = value;
			//cout << " index "<< pParameter->GetIndex(label) << " value "<< value <<"\n";
		}
	}

	return fStatus;
}
//______________________________________________________________________________
void GEventMFM::MakeEventHeader(int enventtype, int nbofpara) {

	if ((enventtype == MFM_COBO_FRAME_TYPE) or (enventtype
			== MFM_COBOF_FRAME_TYPE)) {
		int headersize = 128;
		int itemsize = 4;
		if (enventtype == MFM_COBOF_FRAME_TYPE)
			itemsize = 2;
		int nbitem = COBO_NB_SAMPLES * COBO_NB_AGET_CHANNEL * COBO_NB_AGET;
		int framesize = nbitem * itemsize + headersize;
		int revision = 4;
		int unitBlock_size = 64;
		pCoboFrame->MFM_make_header(unitBlock_size, 0, enventtype, revision,
				(int) (framesize / unitBlock_size), (headersize
						/ unitBlock_size), itemsize, nbitem);
		pCurrentFrame = pCoboFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}

	if (enventtype == MFM_EXO2_FRAME_TYPE) {
		int framesize = EXO_FRAMESIZE;
		int revision = 1;
		int unitBlock_size = 4;
		pExogamFrame->MFM_make_header(unitBlock_size, 0, enventtype, revision,
				(int) (framesize / unitBlock_size), true);
		pCurrentFrame = pExogamFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}
	if (enventtype == MFM_CHIMERA_DATA_FRAME_TYPE) {
		int framesize = CHI_FRAMESIZE;
		int revision = 1;
		int unitBlock_size = 4;
		pChimeraFrame->MFM_make_header(unitBlock_size, 0, enventtype, revision,
				(int) (framesize / unitBlock_size), true);
		pCurrentFrame = pChimeraFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}
	if (enventtype == MFM_BOX_DIAG_FRAME_TYPE) {
		int framesize = NUMEXO_FRAMESIZE;
		int revision = 1;
		int unitBlock_size = NUMEXO_STD_UNIT_BLOCK_SIZE;
		pBoxDiagFrame->MFM_make_header(unitBlock_size, 0, enventtype, revision,
				(int) (framesize / unitBlock_size), true);
		pCurrentFrame = pBoxDiagFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}
	if (enventtype == MFM_VAMOSIC_FRAME_TYPE) {
		int framesize = pVamosICFrame->GetDefinedFrameSize();
		int revision = 1;
		int unitBlock_size = 4;
		pVamosICFrame->MFM_make_header(unitBlock_size, 0, enventtype, revision,
				(int) (framesize / unitBlock_size), true);
		pCurrentFrame = pVamosICFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}
	if (enventtype == MFM_VAMOSPD_FRAME_TYPE) {
		int framesize = VAMOSPD_FRAMESIZE;
		int revision = 1;
		int unitBlock_size = 4;
		pVamosPDFrame->MFM_make_header(unitBlock_size, 0, enventtype, revision,
				(int) (framesize / unitBlock_size), true);
		pCurrentFrame = pVamosPDFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}
	if (enventtype == MFM_VAMOSTAC_FRAME_TYPE) {
		int framesize = pVamosTACFrame->GetDefinedFrameSize();
		int revision = 1;
		int unitBlock_size = 4;
		pVamosTACFrame->MFM_make_header(unitBlock_size, 0, enventtype, revision,
				(int) (framesize / unitBlock_size), true);
		pCurrentFrame = pVamosTACFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}
	if ((enventtype == MFM_EBY_TS_FRAME_TYPE) or (enventtype
			== MFM_EBY_EN_FRAME_TYPE) or (enventtype
			== MFM_EBY_EN_TS_FRAME_TYPE)) {

		int headersize = 0;
		if (enventtype == MFM_EBY_TS_FRAME_TYPE)
			headersize = EBYEDAT_TS_HEADERSIZE;
		if (enventtype == MFM_EBY_EN_FRAME_TYPE)
			headersize = EBYEDAT_EN_HEADERSIZE;
		if (enventtype == MFM_EBY_EN_TS_FRAME_TYPE)
			headersize = EBYEDAT_ENTS_HEADERSIZE;
		int itemsize = 4;
		int nbitem = 33;
		if (nbofpara != 0)
			nbitem = nbofpara;
		int unitBlock_size = 2;
		int framesize = nbitem * itemsize + headersize;
		int revision = 1;

		pEbyedatFrame->MFM_make_header(unitBlock_size, 0, enventtype, revision,
				(int) (framesize / unitBlock_size), (headersize
						/ unitBlock_size), itemsize, nbitem);
		pCurrentFrame = pEbyedatFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}
	if (enventtype == MFM_OSCI_FRAME_TYPE) {
		int unitBlock_size = 2;
		int headersize = MFM_OSCILLO_HEADERSIZE;
		int itemsize = 2;
		int nbitem = 16384;
		int framesize = nbitem * itemsize + headersize;
		int revision = 0;
		pOscilloFrame->MFM_make_header(unitBlock_size, 0, enventtype, revision,
				(int) (framesize / unitBlock_size), (headersize
						/ unitBlock_size), itemsize, nbitem);
		pCurrentFrame = pOscilloFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}
	if (enventtype == MFM_NEDA_FRAME_TYPE) {
		int unitBlock_size = NEDA_STD_UNIT_BLOCK_SIZE;
		int headersize = NEDA_HEADERSIZE;
		int itemsize = sizeof(MFM_Neda_Item);
		int nbitem = NEDA_NB_OF_ITEMS;
		int framesize = headersize + nbitem * itemsize + sizeof(MFM_Neda_EOF);
		int revision = 0;

		pNedaFrame->MFM_make_header(unitBlock_size, 0, enventtype, revision,
				(int) (framesize / unitBlock_size), (headersize
						/ unitBlock_size), itemsize, nbitem);
		pNedaFrame->SetLocationId(1, 112);
		pNedaFrame->SetLeInterval(3);
		pNedaFrame->SetZcoInterval(4);
		pNedaFrame->SetTdcValue(5);
		pNedaFrame->SetSlowIntegral(6);
		pNedaFrame->SetFastIntegral(7);
		pNedaFrame->SetBitfield(8);
		pNedaFrame->SetAbsMax(9);
		pNedaFrame->FillEndOfFrame();
		pCurrentFrame = pNedaFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}
	if (enventtype == MFM_NEDACOMP_FRAME_TYPE) {
		int unitBlock_size = NEDACOMP_STD_UNIT_BLOCK_SIZE;
		int headersize = NEDACOMP_HEADERSIZE;
		int framesize = NEDACOMP_FRAMESIZE;
		int revision = 0;
		pNedaCompFrame->MFM_make_header(unitBlock_size, 0, enventtype, revision,
			(int) (framesize / unitBlock_size), (headersize
			/ unitBlock_size));
		pNedaCompFrame->SetLocationId(1, 112);
		pCurrentFrame = pNedaCompFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}
	if (enventtype == MFM_S3_SYNC_FRAME_TYPE) {
		uint32_t unitBlock_size  = MFM_S3_BAF2_FRAME_TYPE;
		uint32_t framesize = NUMEXO_FRAMESIZE;
		uint32_t revision = 0;
		revision = 1;
		pDiamantFrame->MFM_make_header(unitBlock_size, 0, enventtype,
		revision, (int) (framesize / unitBlock_size), true);
		pCurrentFrame = pS3BaF2Frame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}
	if (enventtype == MFM_S3_ALPHA_FRAME_TYPE) {
		uint32_t unitBlock_size  = NUMEXO_STD_UNIT_BLOCK_SIZE;;
		uint32_t framesize = NUMEXO_FRAMESIZE;
		uint32_t revision = 0;
		revision = 1;
		pDiamantFrame->MFM_make_header(unitBlock_size, 0, enventtype,
			revision, (int) (framesize / unitBlock_size), true);
		pCurrentFrame = pS3AlphaFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}
	if (enventtype == MFM_S3_RUTH_FRAME_TYPE) {
		uint32_t unitBlock_size  = NUMEXO_STD_UNIT_BLOCK_SIZE;;
		uint32_t framesize = NUMEXO_FRAMESIZE;
		uint32_t revision = 0;
		revision = 1;
		pDiamantFrame->MFM_make_header(unitBlock_size, 0, enventtype,
			revision, (int) (framesize / unitBlock_size), true);
		pCurrentFrame = pS3RuthFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}
	if (enventtype == MFM_S3_EGUN_FRAME_TYPE) {
		uint32_t unitBlock_size  = NUMEXO_STD_UNIT_BLOCK_SIZE;;
		uint32_t framesize = NUMEXO_FRAMESIZE;
		uint32_t revision = 0;;
		revision = 1;
		pDiamantFrame->MFM_make_header(unitBlock_size, 0, enventtype,
			revision, (int) (framesize / unitBlock_size), true);
		pCurrentFrame = pS3eGUNFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;

	}	
	if (enventtype == MFM_S3_SYNC_FRAME_TYPE) {
		uint32_t unitBlock_size  = NUMEXO_STD_UNIT_BLOCK_SIZE;;
		uint32_t framesize = NUMEXO_FRAMESIZE;
		uint32_t revision = 0;
		revision = 1;
		pDiamantFrame->MFM_make_header(unitBlock_size, 0, enventtype,
			revision, (int) (framesize / unitBlock_size), true);
		pCurrentFrame = pDiamantFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;

	}
	if (enventtype == MFM_DIAMANT_FRAME_TYPE) {
		uint32_t unitBlock_size  = NUMEXO_STD_UNIT_BLOCK_SIZE;;
		uint32_t framesize = NUMEXO_FRAMESIZE;
		uint32_t revision = 0;
		revision = 1;
		pDiamantFrame->MFM_make_header(unitBlock_size, 0, enventtype,
				revision, (int) (framesize / unitBlock_size), true);
		pCurrentFrame = pDiamantFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}
	if (enventtype == MFM_REA_GENE_FRAME_TYPE) {
		uint32_t unitBlock_size  = NUMEXO_STD_UNIT_BLOCK_SIZE;
		uint32_t framesize = NUMEXO_FRAMESIZE;
		uint32_t revision = 0;
		revision = 1;
		pReaGenericFrame->MFM_make_header(unitBlock_size, 0, enventtype,
				revision, (int) (framesize / unitBlock_size), true);
		pCurrentFrame = pReaGenericFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}
	if (enventtype == MFM_REA_TRACE_FRAME_TYPE) {
		int unitBlock_size = REA_TRACE_STD_UNIT_BLOCK_SIZE;
		int headersize = REA_TRACE_HEADERSIZE;
		int itemsize = sizeof(MFM_ReaTrace_Item);
		int nbitem = REA_TRACE_NB_OF_ITEMS;
		int framesize = headersize + nbitem * itemsize + sizeof(MFM_ReaTraceCheckSum);
		int revision = 0;
		pReaTraceFrame->MFM_make_header(unitBlock_size, 0, enventtype, revision,
				(int) (framesize / unitBlock_size), (headersize
						/ unitBlock_size), itemsize, nbitem);
		pReaTraceFrame->SetLocationId(1, 112);
		pReaTraceFrame->SetSetupTrace(1966);
		pReaTraceFrame->SetCheckSum(66);
		pCurrentFrame = pReaTraceFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	}
	if (enventtype == MFM_SIRIUS_FRAME_TYPE) {
	   int unitBlock_size =pSiriusFrame->GetDefinedUnitBlockSize();
	   	int i = 0;
		int headersize = pSiriusFrame->GetDefinedHeaderSize();
		int itemsize = pSiriusFrame->GetItemSizeFromStructure();
		int nbitem = pSiriusFrame->GetDefinedNbItems();
		int framesize = headersize + nbitem * itemsize ;
		int revision = 0;
		pSiriusFrame->MFM_make_header(unitBlock_size, 0, enventtype, revision,
				(int) (framesize / unitBlock_size), (headersize
						/ unitBlock_size), itemsize, nbitem);
		pDeflectorFrame->SetLocationId(8, 112);
		pSiriusFrame->SetGain(1966);
		for (i = 0; i < 16; i++) { 
			pSiriusFrame->SetFeedBack(i,i);
		}
		pCurrentFrame = pSiriusFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	
	}
		if (enventtype == MFM_S3_DEFLECTOR_FRAME_TYPE) {
	   int unitBlock_size =pDeflectorFrame->GetDefinedUnitBlockSize();

		int headersize = pDeflectorFrame->GetDefinedHeaderSize();
		int framesize = pDeflectorFrame->GetDefinedFrameSize(); ;
		int revision = 0;
		pDeflectorFrame->MFM_make_header(unitBlock_size, 0, enventtype, revision,
				(int) (framesize / unitBlock_size), (headersize
						/ unitBlock_size));
		pDeflectorFrame->SetLocationId(8, 112);
		pDeflectorFrame->SetDeflector(1966);
		pCurrentFrame = pDeflectorFrame;
		pEventBrut_char = (char*) (pCurrentFrame->GetPointHeader());
		return;
	
	}
	cout << " ERROR !! in GEventMFM::MakeEventHeader() no fillEvent done for type = " << enventtype << endl;
}

//______________________________________________________________________________
void GEventMFM::EventInitWithFileName(char* actionFilePAR, char* actionFileSTR) {
	GEventBase::EventInitWithFileNameBase(actionFilePAR, actionFileSTR);
	return;
}
//______________________________________________________________________________
void GEventMFM::FillEvent(int enventtype, long long timestamp, int number) {
	// methode to do simulation , filling event with ramdon or ramp data
	fEventNumber = number;
        
	if ((enventtype == MFM_COBO_FRAME_TYPE) or (enventtype
			== MFM_COBOF_FRAME_TYPE)) {
		pCoboFrame->FillDataWithRamdomValueIntroducingNbItems((uint64_t)timestamp, (uint32_t)number);
		return;
	}
	if (enventtype == MFM_EXO2_FRAME_TYPE) {
		pExogamFrame->FillDataWithRamdomValue(timestamp, number);
		return;
	}
	if (enventtype == MFM_CHIMERA_DATA_FRAME_TYPE) {
		pChimeraFrame->FillDataWithRamdomValue(timestamp, number);
		return;
	}
	if (enventtype == MFM_BOX_DIAG_FRAME_TYPE) {
		pBoxDiagFrame->FillDataWithRamdomValue(timestamp, number);
		return;
	}
	if (enventtype == MFM_VAMOSIC_FRAME_TYPE) {
		pVamosICFrame->FillDataWithRamdomValue(timestamp, number);
		return;
	}
	if (enventtype == MFM_VAMOSPD_FRAME_TYPE) {
		pVamosPDFrame->FillDataWithRamdomValue(timestamp, number);
		return;
	}
	if (enventtype == MFM_VAMOSTAC_FRAME_TYPE) {
		pVamosTACFrame->FillDataWithRamdomValue(timestamp, number);
		return;
	}
	if (enventtype == MFM_OSCI_FRAME_TYPE) {
		pOscilloFrame->FillDataWithRamdomValueIntroducingNbItems(number, 1);
		return;
	}
	if (enventtype == MFM_SCALER_DATA_FRAME_TYPE) {
		pScalerFrame->FillDataWithRamdomValueIntroducingNbItems(timestamp, number);
		return;
	}
	if ((enventtype == MFM_EBY_TS_FRAME_TYPE) or (enventtype
			== MFM_EBY_EN_FRAME_TYPE) or (enventtype
			== MFM_EBY_EN_TS_FRAME_TYPE)) {
		pEbyedatFrame->FillDataWithRamdomValueIntroducingNbItems(timestamp, number);
		return;
	}
	if ((enventtype == MFM_NEDA_FRAME_TYPE)) {
		pNedaFrame->FillDataWithRamdomValueIntroducingNbItems(timestamp, number);
		return;
	}
	if ((enventtype == MFM_NEDACOMP_FRAME_TYPE)) {
		pNedaCompFrame->FillDataWithRamdomValue(timestamp, number);
		return;
	}
	if ((enventtype == MFM_DIAMANT_FRAME_TYPE)) {
		pDiamantFrame->FillDataWithRamdomValue(timestamp, number);
		return;
	}
	if ((enventtype == MFM_S3_BAF2_FRAME_TYPE)) {
		pS3BaF2Frame->FillDataWithRamdomValue(timestamp, number);
		return;
	}
	if ((enventtype == MFM_S3_ALPHA_FRAME_TYPE)) {
		pS3AlphaFrame->FillDataWithRamdomValue(timestamp, number);
	}
	if ((enventtype == MFM_S3_EGUN_FRAME_TYPE)) {
		pS3eGUNFrame->FillDataWithRamdomValue(timestamp, number);
		return;
	}
	if ((enventtype == MFM_S3_RUTH_FRAME_TYPE)) {
		pS3RuthFrame->FillDataWithRamdomValue(timestamp, number);
		return;
	}
	if ((enventtype == MFM_S3_SYNC_FRAME_TYPE)) {
		pS3SynchroFrame->FillDataWithRamdomValue(timestamp, number);
		return;
	}
	if ((enventtype == MFM_REA_GENE_FRAME_TYPE)) {
		pReaGenericFrame->FillDataWithRamdomValue(timestamp, number);
		return;
	}
	if ((enventtype == MFM_REA_TRACE_FRAME_TYPE)) {
		pReaTraceFrame->FillDataWithRamdomValueIntroducingNbItems(timestamp, number);
		return;
	}
	if ((enventtype == MFM_SIRIUS_FRAME_TYPE)) {
		pSiriusFrame->FillDataWithRamdomValueIntroducingNbItems(timestamp, number);
	}
	if ((enventtype == MFM_S3_DEFLECTOR_FRAME_TYPE)) {
		pDeflectorFrame->FillDataWithRamdomValue(timestamp, number);
		return;
	}
	cout << " ERROR !! in GEventMFM::FillEvent no fillEvent done for type = " << enventtype << endl;
}
//______________________________________________________________________________
void GEventMFM::DumpMadeEvent() {
	//TODO
}
//______________________________________________________________________________
void GEventMFM::FillBufferWithEvent(GBuffer* buffer) {

	// a MFM Buffer is exactly a envent ( in this case)
	int eventsize = pCurrentFrame->GetFrameSize();

	((GBufferMFM*) buffer)->SetBufSize(eventsize);
	memcpy(buffer->fGBuf_data, pCurrentFrame->GetPointHeader(), eventsize);
}
//______________________________________________________________________________

bool GEventMFM::IsRaz() {
	if (fEventNumber == -1)
		return true;
	else
		return false;
}
//______________________________________________________________________________
void GEventMFM::GetItem(int i) {
	((MFMBasicFrame*) pCurrentFrame)->GetItem(i);
	//	}

}
//______________________________________________________________________________

////////////////////////////////////////fin /////////////////////////////////////
