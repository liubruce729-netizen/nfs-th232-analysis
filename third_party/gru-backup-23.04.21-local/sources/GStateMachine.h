// File : GStateMachine.h
// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GStateMachine
//
// This class manage State Machine for GAcq /GRUCore .
//
//         NO_STATE 
//            |
//            v
//          FIRST<-----------------------------
//            |                               |
//            v                               |
//       CONFIGURATED <----                   |
//            |    |      |                   |
//            v    --------                   |
//    ----->READY->---------------------      |
//    |       |                        |      |
//    |       v                        |      |
//    |   INIT_RUN                     |      |
//    |       |                        |      |
//    |       v                        |      |
//    |      RUN   <->   PAUSED        |      |
//    |       |                        |      |
//    |       v                        v      |
//    ---<-END_RUN                  END_ACQ->--
//            
//         
//           KILLED
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

#ifndef __GStateMachine__
#define __GStateMachine__
#include "General.h"
#include <sstream>
using std::ostream;

#include <TObject.h>
#include <TFile.h>
#include <TString.h>

#include <TROOT.h>
#include "General.h"
#include "GBase.h"
enum StateMachine {
STATE_NO_STATE = 0,
STATE_FIRST= 1,
STATE_CONFIGURATED= 2,
STATE_READY = 3,
STATE_INIT_RUN = 4,
STATE_RUNNING = 5,
STATE_PAUSED = 6,
STATE_END_RUN = 7,
STATE_END_ACQ = 8,
STATE_KILLED = 9
};
#define NUMBER_OF_STATES 10

//_________________________________________________________________________________________

class GStateMachine : public GBase {

	enum StateMachine fCurrentState;
	const char* fCurrentStateChar;//!!

private:

	const char* fStateMachineChar[NUMBER_OF_STATES];//!

public:
	GStateMachine(); // default constructor
	~GStateMachine();

	virtual enum StateMachine GetState();
	virtual void Init();
	virtual void SetState(enum StateMachine state);
	virtual bool TestStateToGoTo(enum StateMachine stat,bool verbise = true);
	virtual const char* ConvertState(enum StateMachine state = STATE_NO_STATE);
private:
	  virtual void ToDoInCaseOfInterrupt(){};
public:

ClassDef(GStateMachine,1); // Allow to manage state  machine for GAcq.
};

#endif
