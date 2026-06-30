// File : GStateMachine.C
// Author: Luc Legeard

//////////////////////////////////////////////////////////////////////////////
//
// Class GStateMachine
// 

//    
////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/mtio.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <math.h>

#include <dirent.h>
#include <sys/ioctl.h>
#include <TObject.h>
#include <TSystem.h>
#include <TProfile.h>
#include "General.h"
#include "GStateMachine.h"

//______________________________________________________________________________

ClassImp( GStateMachine);

//_____________________________________________________________________________
GStateMachine::GStateMachine(void) {
	// Default constructor of StateMachine object;
	Init();

}
//_____________________________________________________________________________
void GStateMachine::Init(){
	fCurrentState = STATE_NO_STATE;
	fStateMachineChar[STATE_NO_STATE] = "STATE_NO_STATE";
	fStateMachineChar[STATE_FIRST] = "STATE_FIRST";
	fStateMachineChar[STATE_CONFIGURATED] = "STATE_CONFIGURATED";
	fStateMachineChar[STATE_READY] = "STATE_READY";
	fStateMachineChar[STATE_INIT_RUN] = "STATE_INIT_RUN";
	fStateMachineChar[STATE_RUNNING] = "STATE_RUNNING";
	fStateMachineChar[STATE_PAUSED] = "STATE_PAUSED";
	fStateMachineChar[STATE_END_RUN] = "STATE_END_RUN";
	fStateMachineChar[STATE_END_ACQ] = "STATE_END_ACQ";
	fStateMachineChar[STATE_KILLED] = "STATE_KILLED";

	fCurrentStateChar = fStateMachineChar[STATE_NO_STATE];
}
//_____________________________________________________________________________
GStateMachine::~GStateMachine() {
	// destructor of GStateMachine object

}
//_____________________________________________________________________________
bool GStateMachine::TestStateToGoTo(enum StateMachine state, bool verbose) {
	TString tempos;

	bool retour = false;
        if (fVerbose > 9) verbose  = true ;
	switch (state) {

	case STATE_NO_STATE:
		if (verbose) {
			tempos.Form("Incoherent test state to go %s", ConvertState(state));
			fError.TreatError(0, 0, tempos);
		}
		retour = false;
		break;

	case STATE_FIRST:
		if ((fCurrentState != STATE_NO_STATE) and (fCurrentState
				!= STATE_END_ACQ)) {
			if (verbose) {
				tempos.Form("Incoherent test change of state from %s to %s",
						ConvertState(fCurrentState), ConvertState(state));
				fError.TreatError(1, 0, tempos);
			}
		} else {
			retour = true;
		}
		break;

	case STATE_CONFIGURATED:
		if ((fCurrentState != STATE_FIRST) and (fCurrentState
				!= STATE_CONFIGURATED)) {
			if (verbose) {
				tempos.Form("Incoherent test change of state from %s to %s",
						ConvertState(fCurrentState), ConvertState(state));
				fError.TreatError(0, 0, tempos);
			}
		} else {
			retour = true;
		}
		break;

	case STATE_READY:
		if ((fCurrentState != STATE_CONFIGURATED) and (fCurrentState
				!= STATE_END_RUN)) {
			if (verbose) {
				tempos.Form("Incoherent test change of state from %s to %s",
						ConvertState(fCurrentState), ConvertState(state));
				fError.TreatError(0, 0, tempos);
			}
		} else {
			retour = true;
		}
		break;

	case STATE_INIT_RUN:
		if (fCurrentState != STATE_READY) {
			if (verbose) {
				tempos.Form("Incoherent test change of state from %s to %s",
						ConvertState(fCurrentState), ConvertState(state));
				fError.TreatError(0, 0, tempos);
			}
		} else {
			retour = true;
		}
		break;

	case STATE_RUNNING:
		if ((fCurrentState != STATE_INIT_RUN) and (fCurrentState
				!= STATE_PAUSED)) {
			if (verbose) {
				tempos.Form("Incoherent test change of state from %s to %s",
						ConvertState(fCurrentState), ConvertState(state));
				fError.TreatError(0, 0, tempos);
			}
		} else {
			retour = true;
		}
		break;

	case STATE_PAUSED:
		if ((fCurrentState != STATE_RUNNING) and (fCurrentState != STATE_PAUSED)) {
			if (verbose){
				tempos.Form("Incoherent test change of state from %s to %s",
						ConvertState(fCurrentState), ConvertState(state));
			fError.TreatError(0, 0, tempos);
			}
		} else {
			retour = true;
		}
		break;

	case STATE_END_RUN:
		if ((fCurrentState != STATE_RUNNING)
				and (fCurrentState != STATE_PAUSED)) {
			if (verbose) {
				tempos.Form("Incoherent test change of state from %s to %s",
						ConvertState(fCurrentState), ConvertState(state));
				fError.TreatError(0, 0, tempos);
			}
		} else {
			retour = true;
		}
		break;

	case STATE_END_ACQ:
		if (fCurrentState != STATE_READY) {
			if (verbose) {
				tempos.Form("Incoherent test change of state from %s to %s",
						ConvertState(fCurrentState), ConvertState(state));
				fError.TreatError(0, 0, tempos);
			}
		} else {
			retour = true;
		}
		break;

	case STATE_KILLED:
		retour = true;
		break;

	default:
		if (verbose) {
			fError.TreatError(1, 0,
					"State machine : no case to go to STATE_NO_STATE");
		}
	}
	return (retour);
}
//_____________________________________________________________________________
void GStateMachine::SetState(enum StateMachine state) {
TString tempos;
if (fVerbose>9) {
				tempos.Form("Set of state from %s to %s",
						ConvertState(fCurrentState), ConvertState(state));
				fError.TreatError(0, 0, tempos);
			}
	fCurrentState = state;
	fCurrentStateChar = ConvertState(state);

}
//_____________________________________________________________________________
enum StateMachine GStateMachine::GetState() {
	return fCurrentState;
}

//_____________________________________________________________________________
const char* GStateMachine::ConvertState(enum StateMachine state) {

	if (state == STATE_NO_STATE)
		state = fCurrentState;
	const char* retour = NULL;
	retour = fStateMachineChar[state];
	if (retour == NULL)
		fError.TreatError(1, 0,
				"Error en return StateMachineChar[state] value ");
	return retour;
}
//_________________________________________________________________________________
////////////////////////////////////////end /////////////////////////////////////
