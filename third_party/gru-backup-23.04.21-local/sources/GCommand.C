// Author: $Author: legeard $
//
// Part of GRU
//
// GCommand : Network server to serv Soap command
//
//
//
//////////////////////////////////////////////////////////////////////////

// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************

#include "GCommand.h"
#include "Riostream.h"
#include "TPluginManager.h"
#include "TFile.h"
#include "TSystem.h"
#include "TString.h"
#include "GEtalonnageMatacq.h"
#include "GEtalonnageMust.h"
#include "GNetServerRoot.h"

#if NET_LIB
#include "GNetClientSoap.h"
#include "GNetServerSoap.h"
#endif

ClassImp( GCommand);

bool InitDone;

//_________________________________________________________________________________________
GCommand::GCommand(TObject *above, GAcq* acq) {

	fAcq = acq;
	fNetServRoot = NULL;
	fStateMachine = NULL;
	fSpectra = NULL;
	fReaderDevice = NULL;
	fSpectra = GetSpectra();
	fLocalVerbose=-1;

#if NET_LIB
	fNetServSoap = NULL;
#endif

	SetServ(above);
	fAcqLocalCreation = false;
	if (fAcq == NULL) {
		fReaderDevice = NULL;
		fFilenameSpectra = "histo.root";
		fFilenameTtree = "TTree.root";
		fIsSaveTtree = false;
		fInRun = false;
		fAcqLocalCreation = false;
		fSpectra = NULL;
		if (fNetServRoot != NULL) {
			fSpectra = GetSpectra();
			//fFilenameSpectra = fAcq->GetFilenameSpectra(); TODO
		}

#if NET_LIB
		if (fNetServSoap != NULL) {
		}
#endif

	}
}

//_________________________________________________________________________________________
GCommand::~GCommand() {

	//	if (GetSpeServ()) {
	//	GetSpeServ()->StopGNetServer();
	//	}
	if ((fAcqLocalCreation) && (fAcq != NULL)) {
		delete fAcq;
		fAcq = NULL;
	} else {
		if (fReaderDevice != NULL) {
			delete fReaderDevice;
			fReaderDevice = NULL;
		}
	}
	if (fStateMachine) {
		delete fStateMachine;
		fStateMachine = NULL;
	}
}

//_________________________________________________________________________________________
void GCommand::InitStateMachine() {
	if (fStateMachine==NULL){
		fStateMachine = new GStateMachine();
	}else {
		fStateMachine->Init();
	}
	fStateMachine->SetState(STATE_FIRST);
}

//_________________________________________________________________________________________
void GCommand::SetServ(TObject *above) {
	if (above != NULL) {
		if (above->InheritsFrom(GNetServerRoot::Class())) {
			fNetServRoot = (GNetServerRoot*) above;
		}

#if NET_LIB
		if (above->InheritsFrom(GNetServerSoap::Class())) {
			fNetServSoap = (GNetServerSoap*) above;
		}
#endif
	}
}
//_________________________________________________________________________________________
GSpectra* GCommand::GetSpectra() {
	GSpectra *sp;
	sp = NULL;
	if (fAcq) {
		sp = fAcq->GetSpectra();
	} else {
		if (fNetServRoot)
			sp = ((GNetServerRoot*) fNetServRoot)->GetSpectra();
	}
	return sp;
}
//_________________________________________________________________________________________
void GCommand::SetAcq(GAcq *acq) {
	fAcq = acq;
	if (fAcq) {
		if (fReaderDevice == NULL)
			fReaderDevice = fAcq->GetDeviceIn();
		fSpectra = fAcq->GetSpectra();
	}
	if (fNetServRoot != NULL) {
		if (fNetServRoot->InheritsFrom(GNetServerRoot::Class())) {
			((GNetServerRoot*) fNetServRoot)->SetSpectra(fSpectra);
		}
	}
}
//_________________________________________________________________________________________
bool GCommand::AnalyseCommandAndDo(char* command, TString* streturn) {
	TString tempos;
	int commanddone = 0;
	bool retour = true;
	int nb_words;
	char** commandsliced = NULL;
	int* commandsliced_int = NULL;
	int type = GA_NOT_DEFINED;
	//TODO verifier la validit� de cela
	//if (streturn == NULL)
	//streturn = new TString();
	//cout <<" debug command="<<command<<endl;
	nb_words = Slicer(command, &commandsliced, &commandsliced_int);
	//cout <<" debug ";
	//DumpCommand( commandsliced, *commandsliced_int);

	// test of fist word that must be always "GRU"
	if (nb_words < 1) {
		fError.TreatError(1, 0, "AnalyseCommandAndDo : Empty command");
		return (false);
	}

	if (fVerbose > 1)
		DumpCommand(commandsliced, nb_words);
	if (!(TestGruWord(commandsliced[0]))) {

		return (false);
	}

	if (nb_words >= 2) {
		// ------------------with no return-------------------------------------------------------------

		if (CompareWordsIgnoreCase(commandsliced[1], "ls")) {
			cout << "  Execution- of gROOT->ls()\n";
			gROOT->ls();
			cout << "\n";
			commanddone++;
		}
		if (CompareWordsIgnoreCase(commandsliced[1], "printtext")) {
			printtext(commandsliced[2]);
			commanddone++;
		}

		if (CompareWordsIgnoreCase(commandsliced[1], "test")) {
			if (nb_words >= 3) {
				Test(commandsliced_int[2]);
			} else
				Test(0);
			commanddone++;
		}

		if (CompareWordsIgnoreCase(commandsliced[1], "kill")) {
			Kill();
			commanddone++;
		}

		if (CompareWordsIgnoreCase(commandsliced[1], "endofpage")) {
			PauseAcq(0);
			commanddone++;
		}

		if (CompareWordsIgnoreCase(commandsliced[1], "run")) {
			if (nb_words >= 3) {
				if (CompareWordsIgnoreCase(commandsliced[2], "start")) {
					if (nb_words > 3) {
						DoRun(commandsliced_int[3]);
					} else {
						DoRun();
					}

					commanddone++;
				}
				// on différencie les cas Calimero ( envoyé par das stop pour un end ) et les autres
				// normalement la commande envoyé par DAS devrait etre "endcalim comme specifié un peu plus bas"
				if (CompareWordsIgnoreCase(commandsliced[2], "stop")&&CompareWordsIgnoreCase(commandsliced[0], "GRU")) {
									StopRun(true);
									commanddone++;
								}
			
				if (CompareWordsIgnoreCase(commandsliced[2], "pause")) {
					if (nb_words > 3)
						PauseAcq(commandsliced_int[3]);
					else
						PauseAcq(-1);
					commanddone++;
				}
				if (CompareWordsIgnoreCase(commandsliced[2], "resume")) {
					PauseAcq(0);
					commanddone++;
				}
				if (CompareWordsIgnoreCase(commandsliced[2], "end")) {
					EndAcq();
					commanddone++;
				}
				if (CompareWordsIgnoreCase(commandsliced[2], "endcalim")) {
					EndAcq(true);
					commanddone++;
				}
				if (CompareWordsIgnoreCase(commandsliced[2], "stop")&&CompareWordsIgnoreCase(commandsliced[0], "calimero")) {
					EndAcq(true);
					commanddone++;
				}
			}
		}

		if (CompareWordsIgnoreCase(commandsliced[1], "initruns")) {
			if (nb_words >= 2) {
				InitRuns();
				commanddone++;
			}
		}

		if (CompareWordsIgnoreCase(commandsliced[1], "initcalim")) {
			if ((nb_words == 9) ||(nb_words == 8)) {
			if (nb_words == 9)
				InitCalim(commandsliced[2], commandsliced[3],
						commandsliced[4], commandsliced[5],
						commandsliced_int[6], commandsliced_int[7],commandsliced[8]);
				else 
				
					InitCalim(commandsliced[2], commandsliced[3],
						commandsliced[4], commandsliced[5],
						commandsliced_int[6], commandsliced_int[7]);	
				commanddone++;
			}
			
		}

		if (CompareWordsIgnoreCase(commandsliced[1], "initacq")) {
			if (nb_words >= 3) {
				if (CompareWordsIgnoreCase(commandsliced[2], "no")) {
					InitGAcq(1);
					commanddone++;
				}
				if (CompareWordsIgnoreCase(commandsliced[2], "guser")) {
					InitGAcq(2);
					commanddone++;
				}
				if (CompareWordsIgnoreCase(commandsliced[2], "infinite")) {
					SetInfiniteReadGAcq();
					commanddone++;
				}
			}
		}
		if (CompareWordsIgnoreCase(commandsliced[1], "verbose")) {
			if (nb_words >= 3) {
				SetVerboseGAcq(commandsliced_int[2]);
				commanddone++;
				
			}
		}
		

		if (CompareWordsIgnoreCase(commandsliced[1], "initevent")) {
			if (nb_words == 3) {
				InitEvent(commandsliced[2]);
				commanddone++;
			}
			if (nb_words >= 4) {
				InitEvent(commandsliced[2], (char*) commandsliced[3]);
				commanddone++;
			}

		}

		if (CompareWordsIgnoreCase(commandsliced[1], "spectrum")) {
			if (nb_words == 4) {
				if (CompareWordsIgnoreCase(commandsliced[2], "reset")) {
					SpectrumReset(commandsliced[3]);
					commanddone++;
				}
			}

			if (nb_words >= 5) {
				if (CompareWordsIgnoreCase(commandsliced[2], "reset")) {
					SpectrumReset(commandsliced[3], commandsliced[4]);
					commanddone++;
				}
			}

		}

		if (CompareWordsIgnoreCase(commandsliced[1], "spectra")) {
			if (nb_words >= 3) {
				if (CompareWordsIgnoreCase(commandsliced[2], "all")) {
					InitModeSpectre(1);
					commanddone++;
				}
				if (CompareWordsIgnoreCase(commandsliced[2], "reset")) {
					SpectraReset();
					commanddone++;
				}
				if (CompareWordsIgnoreCase(commandsliced[2], "save")) {
					if (nb_words >= 4) {
						SpectraSave(commandsliced[3]);
						commanddone++;
					} else {
						SpectraSave();
						commanddone++;
					}
				}
				if (CompareWordsIgnoreCase(commandsliced[2], "list")) {
					SpectraList();
					commanddone++;
				}
				if (CompareWordsIgnoreCase(commandsliced[2], "refresh")) {
					RefreshSpectraDB();
					commanddone++;
				}
			}

		}

		if (CompareWordsIgnoreCase(commandsliced[1], "gcc")) {
			if (nb_words >= 3) {
				Compilation(commandsliced[2], (char*) "++");
				commanddone++;
			}
		}
		if (CompareWordsIgnoreCase(commandsliced[1], "includeuser")) {
			if (nb_words >= 3) {
				SetIncludeUser(commandsliced[2]);
				commanddone++;
			}
		}
		if (CompareWordsIgnoreCase(commandsliced[1], "print")) {
			if (nb_words >= 3) {
				if (CompareWordsIgnoreCase(commandsliced[2], "info")) {
					PrintInformation();
					commanddone++;
					}
				if (CompareWordsIgnoreCase(commandsliced[2], "state")) {
					cout <<fStateMachine->ConvertState()<<"\n";
					commanddone++;
					}
				}
			}

		if (CompareWordsIgnoreCase(commandsliced[1], "dump")) {
			if (nb_words >= 3) {
				if (CompareWordsIgnoreCase(commandsliced[2], "event")) {
					Dump(2);
					commanddone++;
				}

				if (CompareWordsIgnoreCase(commandsliced[2], "buffer")) {
					Dump(1);
					commanddone++;
				}
			}
		}

		if (CompareWordsIgnoreCase(commandsliced[1], "input")) {
			if (nb_words >= 3) {
				if (CompareWordsIgnoreCase(commandsliced[2], "rewind")) {
					RewindDevice();
					commanddone++;
				}

				if (CompareWordsIgnoreCase(commandsliced[2], "buffersize")) {
					if (nb_words >= 4) {
						SetBufferSize(commandsliced_int[3]);
						commanddone++;
					}
				}
				if (CompareWordsIgnoreCase(commandsliced[2], "file")) {
					if (nb_words >= 4) {
						type = GT_TYPE_FILE;
						InitDevice(commandsliced[3], type, 0);
						commanddone++;
					}
				}
				if (CompareWordsIgnoreCase(commandsliced[2], "mfmfile")) {
					if (nb_words >= 4) {
						type = GT_TYPE_MFMFILE;
						InitDevice(commandsliced[3], type, 0);
						commanddone++;
					}
				}
				if (CompareWordsIgnoreCase(commandsliced[2], "tape")) {
					if (nb_words >= 4) {
						type = GT_TYPE_TAPE;
						InitDevice(commandsliced[3], type, 0);
						commanddone++;
					}
				}

				if (CompareWordsIgnoreCase(commandsliced[2], "narval")) {
					if (nb_words >= 5) {
						type = GA_NET_NARVAL;
						InitDevice(commandsliced[3], type, commandsliced_int[4]);
						commanddone++;
					}
				}
				if (CompareWordsIgnoreCase(commandsliced[2], "ganil")) {
					if (nb_words >= 5) {
						type = GA_NET_CLIENT;
						InitDevice(commandsliced[3], type, commandsliced_int[4]);
						commanddone++;
					}
				}
			}
		}

		if (CompareWordsIgnoreCase(commandsliced[1], "spectraserver")) {
			if (nb_words >= 3) {
				if (CompareWordsIgnoreCase(commandsliced[2], "stop")) {
					InitSpectraServer(0, 0);
					commanddone++;
				}
				if (CompareWordsIgnoreCase(commandsliced[2], "start")) {
					if (nb_words == 3) {
						InitSpectraServer(1);
						commanddone++;
					}
					if (nb_words >= 4) {
						InitSpectraServer(1, commandsliced_int[3]);
						commanddone++;
					}
				}
			}
		}
		// ------------------with return-------------------------------------------------------------
		if ((CompareWordsIgnoreCase(commandsliced[1], "GET")) and ((streturn)
				!= NULL)) {
			if (nb_words >= 3) {
				if (CompareWordsIgnoreCase(commandsliced[2], "info")) {
					tempos = GetInformation();
					*streturn = tempos;
					commanddone += 1;
				}

				if (CompareWordsIgnoreCase(commandsliced[2], "PID")) {
					streturn->Form("%d", (int) getpid());
					commanddone += 1;
				}

				if (CompareWordsIgnoreCase(commandsliced[2], "spectra")) {
					if (nb_words >= 4) {
						if (strcasecmp(commandsliced[3], "list") == 0) {
							*streturn = GetSpectraList();
							commanddone += 1;
						}
					}
				}
				if (CompareWordsIgnoreCase(commandsliced[2], "spectra_db")) {
					*streturn > Form("Use specific method ReceiveSpectraDB() ");
					commanddone += 1;
				}

				if (CompareWordsIgnoreCase(commandsliced[2], "dump")) {
					if (nb_words >= 4) {
						if (CompareWordsIgnoreCase(commandsliced[3], "event")) {
							*streturn = GetDump(2);
							commanddone += 1;
						}
						if (CompareWordsIgnoreCase(commandsliced[3], "buffer")) {
							*streturn = GetDump(1);
							commanddone += 1;
						}
					}
				}
			}
		}
	}

	if (commanddone == 1) {
		if (fVerbose > 1)
			fError.Infos("End of Command");
	} else {
		fError.TreatError(1, 0, "End of Command not understood : ", command);

	}

	if (commandsliced) {
		for (int i = 0; i < nb_words; i++) {
			delete[] commandsliced[i];
			commandsliced[i] = NULL;
		}
		delete[] commandsliced;
		commandsliced = NULL;
	}
	if (commandsliced_int) {
		delete[] commandsliced_int;
		commandsliced_int = NULL;
	}

	return retour;
}

//_________________________________________________________________________________________
void GCommand::PauseAcq(int sec) {

	if (fStateMachine) {
		if (!(fStateMachine->TestStateToGoTo(STATE_PAUSED))) {
			return;
		}
	}

	TString tempos;
	if (fAcq) {
		if (sec <= 0) {
			tempos.Form("Pause ");
			if (fVerbose > 1)
				fError.Infos(tempos);
			fAcq->Pause(sec);
			if (fStateMachine)
				fStateMachine->SetState(STATE_PAUSED);
		} else {
			tempos.Form("Pause for %d secondes", sec);
			fError.Infos(tempos);
			fAcq->Pause(sec);
			if (fStateMachine)
				fStateMachine->SetState(STATE_PAUSED);
		}

	} else {
	//	fError.TreatError(2, 0, "Please initialize the Acq configuration");
	}

}

//_________________________________________________________________________________________
TString GCommand::GetInfo(Int_t* nb_info) {
	TString tempos;
	TString info;

	if (fAcq) {
		info = fAcq->GetInfo(nb_info);
	} else {
		fError.TreatError(2, 0, "please initialize the acq configuration");
	}
	return info;
}
//_________________________________________________________________________________________
void GCommand::printtext (char* info) {
	TString tempos;
	tempos.Form("%s",info);
	fError.TreatError(0, 0, "------------------------------------------------------------------------------");
	fError.TreatError(0, 0, tempos);
	fError.TreatError(0, 0, "------------------------------------------------------------------------------");
}

//_________________________________________________________________________________________
void GCommand::SetVerboseGAcq(int level) {
	TString tempos;
	fLocalVerbose = level;
	if (fAcq) {
		fAcq->SetVerbose(level);
	}
	tempos.Form("SetVerbose applied with level %d ", level);
	fError.Infos( tempos);
}
//_________________________________________________________________________________________
void GCommand::DoRun() {

	if (fReaderDevice) {
		if (fAcq) {
			StopRun(false);
			if (fStateMachine) {

				if (!(fStateMachine->TestStateToGoTo(STATE_INIT_RUN))) {
					return;
				}

				fStateMachine->SetState(STATE_INIT_RUN);
				if (!(fStateMachine->TestStateToGoTo(STATE_RUNNING))) {
					return;
				}
			}
			fAcq->DoRunT(0);
			if (fStateMachine)
				fStateMachine->SetState(STATE_RUNNING);
			fError.Infos("Threaded Run Started ");
		} else {
			fError.TreatError(2, 0, "Please initialize Acquisition");
		}
	} else {
		fError.TreatError(2, 0, "Please initialize the device configuration");
	}
}
//_________________________________________________________________________________________
void GCommand::DoRun(int coups) {
	TString tempos;

	if (fAcq) {
		StopRun(false);
		if (!(fStateMachine->TestStateToGoTo(STATE_INIT_RUN))) {
			return;
		}

		fStateMachine->SetState(STATE_INIT_RUN);
		if (!(fStateMachine->TestStateToGoTo(STATE_RUNNING))) {
			return;
		}
		tempos.Form("Start run on %d events", coups);
		fError.Infos(tempos);
		fStateMachine->SetState(STATE_RUNNING);
		fAcq->DoRun(coups);
		StopRun(true);
	} else {
		fError.TreatError(2, 0, "please initialize the acq configuration");
	}
}
//_________________________________________________________________________________________
TString GCommand::GetSpectraList() const {
	TString tempos;
	TString info;

	if (fSpectra) {
		tempos = fSpectra->GetDB()->GetSpectraList();
	} else {
		fError.TreatError(2, 0, "Please initialize the spectra configuration");
		
	}
	return tempos;
}
//_________________________________________________________________________________________
void GCommand::SpectraList() const {

	if (fSpectra) {
		fSpectra->GetDB()->SpectraList();
	} else {
		fError.TreatError(2, 0, "Please initialize the spectra configuration");
	}
}
//_________________________________________________________________________________________
void GCommand::PrintInformation() {
	cout <<GetInformation().Data()<<"\n";
	cout <<fStateMachine->ConvertState()<<"\n";
}
//_________________________________________________________________________________________
TString GCommand::GetInformation() {
	TString tempos;
	TString info;

	if (fAcq) {
		TString suc, event, buffer, eventnum, buffernum, time;
		suc.Form("%3.1f", fAcq->GetSucces());
		event.Form("%.1f", fAcq->GetRateEvent());
		buffer.Form("%.1f", fAcq->GetRateBuffer());
		eventnum.Form("%d", fAcq->GetNumEvent());
		buffernum.Form("%d", fAcq->GetNumBuffer());
		info = suc + ", " + event + ", " + buffer + ", " + eventnum + ", "
				+ buffernum;

	} else {
		fError.TreatError(1, 0, "Acq not configurated so no acq information ");
	}
	return info;
}
//_________________________________________________________________________________________
void GCommand::Dump(int mode) const {
	if (mode == 1 and fReaderDevice != NULL) {
		GDevice *dev = fAcq->GetDeviceIn();
		GBuffer * buff = dev->GetCurrentBuffer();
		buff->DumpBuffer();
	} else if (mode == 2 and fReaderDevice != NULL) {
		GEventBase *event = fAcq->GetEvent();
		event->DumpEvent();
	} else {
		fError.TreatError(2, 0, "please initialize the device configuration");
	}
}
//_________________________________________________________________________________________
TString GCommand::GetDump(int mode)const {
	TString str;
	if (mode == 1 and fReaderDevice != NULL) {
		GDevice *dev = fAcq->GetDeviceIn();
		GBuffer * buff = dev->GetCurrentBuffer();
		str = buff->GetDumpBuffer();
	} else if (mode == 2 and fReaderDevice != NULL) {
		GEventBase *event = fAcq->GetEvent();
		str = event->GetDumpEvent();
	} else {
		fError.TreatError(2, 0, "please initialize the device configuration");
	}
	return str;
}
//_________________________________________________________________________________________
void GCommand::SetBufferSize(int size) {

	if (fReaderDevice){
		TString tempos;
		tempos.Form("Change size buffer for %d",size);
		if (fVerbose >2)fError.Infos(tempos);
		fReaderDevice->SetBufferSize(size);
		
	}else {
		fError.TreatError(2, 0, "Please initialize device");
	}
}
//_________________________________________________________________________________________
void GCommand::EndAcq(bool calim) {
	if (!(fStateMachine->TestStateToGoTo(STATE_END_ACQ))) {
		return;
	}
	fStateMachine->SetState(STATE_END_ACQ);
	fAcq->EndUser();
	InitDone= false;
	if (calim)
		SpectraSave((char*) "spectraCalim.root");
	else
		SpectraSave((char*) "spectra.root");
	//fAcq->SetSpectraMode(0);
	fReaderDevice->Close();
	//if (fAcqLocalCreation) {
	//	delete (fAcq);
	//	fAcq = NULL;
	//}
	
	fStateMachine->SetState(STATE_FIRST);
}
//_________________________________________________________________________________________
void GCommand::GetPlugin(char *pluginName) {
	TPluginHandler *ph = gPluginMgr->FindHandler("GAcq", pluginName);
	if (!ph) {
		fError.TreatError(2, 0, "No plugin found!");
		return;
	}
	//verifie si code source trouve
	if (ph->CheckPlugin() == -1) {
		TString texte = "Source code for ";
		texte.Append(pluginName);
		texte.Append(" does not exist! \n");
		fError.TreatError(2, 0, texte);
	}
	//compile et charge source
	if (ph->LoadPlugin() == -1) {
		//probleme de compilation?
		TString texteE = " Cannot compile or load plugin ";
		texteE.Append(pluginName);
		texteE.Append("!\n");
		fError.TreatError(2, 0, texteE);
	}
	//creation d'un objet de la classe 'plugin'
	fAcq = (GAcq*) ph->ExecPlugin(0);// --> plgb = new MPlugUser
	if (fAcq != NULL){
		fAcqLocalCreation = true;
		fError.TreatError(0, 0, "Plugin GUser OK");
	}
}

//_________________________________________________________________________________________
GSpectraDB *GCommand::GetSpectraBD() {
	// return Spectra Data Base
	return (fAcq->GetSpectra())->GetDB();
}

//_________________________________________________________________________________________
void GCommand::InitEvent(char *source, char *type) {

	if (fAcq) {
		if (CompareWordsIgnoreCase(source, (char*) "fromrun")) {
			if (type == NULL) {
				fAcq->EventInit();
			} else
				fAcq->EventInit((char*) "bidon", type);
		} else {
			if (type == NULL)
				fAcq->EventInit(source);
			else
				fAcq->EventInit(source, type);
		}
	} else {
		fError.TreatError(2, 0, " Acq not constructed");
	}
}

//_________________________________________________________________________________________
void GCommand::InitGAcq(int mode) {

	if (!(fStateMachine->TestStateToGoTo(STATE_CONFIGURATED))) {
		return;
	}
	TString tempos;
	if (fReaderDevice != NULL) {
		fReaderDevice->Open();
	} else {
		fError.TreatError(1, 0, "Device is null");
		return ;
	}

	if ((GetAcq())) {
		delete (fAcq);
		fAcq = NULL;
		if (fVerbose >5) GetError()->TreatError(0, 0, "Delete previous GAcq object");

	}
	if (fReaderDevice != NULL and fReaderDevice->GetIsOpen()) {
		switch (mode) {
		case 1:
			if (fAcq == NULL) {
				SetAcq(new GAcq(fReaderDevice));
				fAcqLocalCreation = true;
				fStateMachine->SetState(STATE_CONFIGURATED);
				if (fLocalVerbose <0) GetAcq()->SetVerbose(fLocalVerbose);
			} else {
				fError.TreatError(1, 0, "Acquisition already instancied\n");
				fStateMachine->SetState(STATE_CONFIGURATED);
			}
			break;
		case 2:
			if (fAcq == NULL) {
				GetPlugin((char*) "GuserPlugin");
				if (fAcq) {
					fAcq->SetDevIn(fReaderDevice);
					SetAcq(fAcq);
					fStateMachine->SetState(STATE_CONFIGURATED);

				} else {
					fError.TreatError(
							2,
							0,
							"Verify your file is named Guser \n and it is placed in current directory or home directory \n");
				}
			}
			break;
		case 3:

			if (fAcq) {
				fAcq->SetInfiniteRead(true);

			} else {
				fError.TreatError(
						2,
						0,
						"Verify your file is named Guser \n and it is placed in current directory or home directory \n");
			}
			break;
		}

	}
	if (fAcq) {
		if (fReaderDevice)
			fReaderDevice->Rewind();
		else
			fError.TreatError(2, 0, "GDevice is Null or not opened\n");
	} else {
		delete (fReaderDevice);
		fReaderDevice = NULL;
		fError.TreatError(2, 0,
				"Initialization of Gacq is failed because , GDevice is Null or not opened\n");
	}
}

//_________________________________________________________________________________________
void GCommand::InitRuns() {
	if (!fStateMachine->TestStateToGoTo(STATE_READY)) {
		return;
	}
	fAcq->InitUser();
	fStateMachine->SetState(STATE_READY);
}

//_________________________________________________________________________________________
void GCommand::InitDevice(char *source, Int_t type, Int_t port) {

	/*if (fAcq != NULL) {
		SetDevIn()
		delete (fAcq);
		fAcq = NULL;
		fIsSaveTtree = false;
		fInRun = false;
	}*/
	TString tempos;
	
	if (fReaderDevice) {
		delete (fReaderDevice);
		fReaderDevice = NULL;
	}

	if ((type == GT_TYPE_TAPE) || (type == GT_TYPE_FILE))
		fReaderDevice = (GDevice*) (new GTape(source));
		
	if ((type == GT_TYPE_MFMFILE))
		fReaderDevice = (GDevice*) (new GMFMFile(source));	
	if(fAcq)
		fAcq->SetDevIn(fReaderDevice);
		
#if NET_LIB
	if (type == GA_NET_CLIENT) {
		fError.TreatError(1,0,"GNetClientGanil is obsolete");
	}
	if (type == GA_NET_NARVAL) {
		fReaderDevice = (GDevice*) (new GNetClientNarval(source));
		((GNetClientNarval*) fReaderDevice)->SetPort(port);
		if (fVerbose >0)tempos.Form("Set device port : %d",port);
		
		fError.Infos(tempos);
	}
#endif
	if(fAcq)
	fAcq->SetDevIn(fReaderDevice);
	

}

//_________________________________________________________________________________________
void GCommand::RewindDevice() {
	if ((fReaderDevice->GetType() == GT_TYPE_TAPE) || (fReaderDevice->GetType()
			== GT_TYPE_FILE))
		fReaderDevice->Rewind();
}

//_________________________________________________________________________________________
void GCommand::InitModeSpectre(int mode) {
	if (fAcq) {
		switch (mode) {
		case 1:
			fAcq->SetSpectraMode(1);
			break;
		default:
			fAcq->SetSpectraMode(0);
			break;
		}
	} else {
		fError.TreatError(1, 0, " InitModeSpectre impossible, GAcq not built");
	}
}
//_________________________________________________________________________________________
bool GCommand::InRun() {
	return fInRun;
}

//_________________________________________________________________________________________
bool GCommand::IsInit() {
	if (fReaderDevice != NULL) {
		if (fReaderDevice->GetIsOpen() and fReaderDevice != NULL) {
			return true;
		}
	}
	return false;
}

//_________________________________________________________________________________________
void GCommand::SpectraSave(char* filename) {

	if (filename != NULL) {
		fFilenameSpectra = filename;
	}
	if (fFilenameSpectra) {
		fAcq->SpeSave(fFilenameSpectra.Data());
		fError.Infos("Spectra saved  in ", fFilenameSpectra);
	} else {
		fError.TreatError(2, 0, "Spectra not Save, file name empty");
	}
}
//_________________________________________________________________________________________
void GCommand::SpectraReset() {
	if (GetSpectra() == NULL) {
		fError.TreatError(1, 0, "GCommand::SpectraReset,  No data base defined");
	}
	GetSpectra()->RazDB();
}
//_________________________________________________________________________________________
void GCommand::SaveTtree(char *str, int type) {
	if (fAcq) {
		switch (type) {
		case 1:
			fAcq->SetTTreeMode(TREE_STANDARD, (char *) str);
			break;
		case 2:
			fAcq->SetTTreeMode(TREE_ONE_VECTOR, (char *) str);
			break;
		case 3:
			fAcq->SetTTreeMode(TREE_USER, (char *) str);
			break;
		default:
			break;
		}
		fError.Infos("The Analysis save a Ttree in ", (char *) str);
	} else {
		fError.TreatError(1, 0, "No Gacq instantied");
	}
}
//_________________________________________________________________________________________
void GCommand::SetSaveName(char *str) {
	fFilenameSpectra = str;
}

//_________________________________________________________________________________________
void GCommand::SetRun(bool etat) {
	fInRun = etat;
}

//_________________________________________________________________________________________
void GCommand::StopRun(bool etat) {

	if (etat) {
		if (!(fStateMachine->TestStateToGoTo(STATE_END_RUN))) {
			return;
		}

		if (fAcq)
			fAcq->SetStop(etat);
		fStateMachine->SetState(STATE_END_RUN);
		if (!(fStateMachine->TestStateToGoTo(STATE_READY))) {
			return;
		}
		fStateMachine->SetState(STATE_READY);
	} else {
		if (fAcq)
			fAcq->SetStop(etat);
	}

}
//_________________________________________________________________________________________
void GCommand::SetIncludeUser(char* include) {
	TString tempos;
	if (strlen(include) != 0) {
		TString tempos;
		tempos.Form(".include %s", include);
		gROOT->ProcessLine(tempos);
		fError.Infos("gROOT->ProcessLine", tempos);
	} else {
		fError.TreatError(1, 0, "include path is empty");
	}
}
//_________________________________________________________________________________________
void GCommand::SetInfiniteReadGAcq(bool mode) {
	if (fAcq)
		fAcq->SetInfiniteRead(mode);
}
//_________________________________________________________________________________________
void GCommand::Compilation(char *nameuser, char* opt) {
	TString tempos;
	TString cmd;
	if (strlen(nameuser) != 0) {
		cmd = ".L ";
		cmd.Append(nameuser);
		cmd.Append("++");
		tempos.Form("Compilation of %s is running...", cmd.Data());
		fError.Infos(tempos);
		gROOT->ProcessLine(cmd);
	} else {
		fError.TreatError(1, 0, "Can't compil because source file is upty");
	}
}
/*
//_________________________________________________________________________________________
bool GCommand::InitGlobalCalim(char * cardname) {
	GAcq * etalon;
	etalon = NULL;
	bool ret = false;
	GetError()->Infos("Init Global Calibration with type of Card : ", cardname);
			
	if ((GetDevice()) == NULL) {
		ret = false;
		GetError()->TreatError(2, 0, "Init is impossible, Device not defined");
		return ret;
		}
	if ((GetAcq())) {
		delete (fAcq);
		fAcq = NULL;
		if (fVerbose >5) GetError()->TreatError(0, 0, "Delete previous GAcq object");

	}
	if (CompareWordsIgnoreCase(cardname, "MATACQ")) {
		GEtalonnageMatacq *etalonMatacq = new GEtalonnageMatacq(GetDevice(), NULL);
		etalon = (GEtalonnageMatacq *) etalonMatacq;
	}
	if (CompareWordsIgnoreCase(cardname, "MUVI")) {
		GEtalonnageMust * etalonMust = new GEtalonnageMust(GetDevice(),NULL);
		etalon = (GEtalonnageMust *) etalonMust;
	}
	if (etalon)
			SetAcq(etalon);
			
	if ((GetAcq())) {
		ret = true;
		if (fLocalVerbose >=0) GetAcq()->SetVerbose(fLocalVerbose);	
	} else {
		GetError()->TreatError(2, 0, "Acquistion context not defined");
		
	}
	return ret;
}
*/
//_________________________________________________________________________________________

void GCommand::InitCalim(char* exp_name, char* cardname, char*host_name,
		char* para_name, int nbpara, int calimode, char *eventinitmode) {
		// event init mode = MFM or GANIL 
		// calim mode = 1,2,3 
		// called by command GRU INITCALIM....
		// in this 
	if (InitDone) {
		GetError()->Infos("Init already done");
		return;
	}
	char const * defautlmode = "ganil";
        if (eventinitmode == NULL)
        	eventinitmode =(char *) defautlmode;
        if (CompareWordsIgnoreCase(cardname, "GANIL"))
		if (fStateMachine){
			if (!fStateMachine->TestStateToGoTo(STATE_CONFIGURATED)) {
			return;
			}
		}
	if (fStateMachine)
		fStateMachine->SetState(STATE_CONFIGURATED);
	GEtalonnageMatacq* pEtalonnageMatacq = NULL;
	GEtalonnageMust*   pEtalonnageMust   = NULL;

	
	//---------------------------InitGlobalCalim(cardname);-------------------------------

	GetError()->Infos("Init Global Calibration with type of Card : ", cardname);
			
	if ((GetDevice()) == NULL) {
		GetError()->TreatError(2, 0, "Init is impossible, Device not defined");
		}
	if ((GetAcq())) {
		delete (fAcq);
		fAcq = NULL;
		if (fVerbose >5) GetError()->TreatError(0, 0, "Delete previous GAcq object");
	}
	if (CompareWordsIgnoreCase(cardname, "MATACQ")) {
		pEtalonnageMatacq = new GEtalonnageMatacq(GetDevice(), NULL);
		SetAcq(pEtalonnageMatacq);
		pEtalonnageMatacq->InitCalim( exp_name, cardname, host_name,
		 para_name,  nbpara,  calimode, eventinitmode);
	}
	else if (CompareWordsIgnoreCase(cardname, "MUVI")) {
		pEtalonnageMust    = new GEtalonnageMust(GetDevice(),NULL);
		SetAcq(pEtalonnageMust);
		pEtalonnageMust->InitCalim( exp_name, cardname, host_name,
		 para_name,  nbpara,  calimode, eventinitmode);
	}
	else  GetError()->TreatError(2, 0, "Init not done because card init mode ( MATACQ or MUVI) not spécified");
	
	if ((GetAcq())) {
		if (fLocalVerbose >=0) GetAcq()->SetVerbose(fLocalVerbose);	
	} else {
		GetError()->TreatError(2, 0, "Acquistion context not defined");
		
	}
	
	//---------------------------fin InitGlobalCalim(cardname);-------------------------------
	/*
	TString tempos;
	(GetAcq())->GetSpectra()->RazDB();
	(GetAcq())->EventInit(exp_name, (char*) eventinitmode, false);

	if (strcmp(para_name, "") == 0)
		GetAcq()->SetSpectraMode(1);
	else
		GetAcq()->SetSpectraMode(1); // Etalon->SetSpectraMode(1,para_name,nbpara);

	if (pEtalonnageMatacq)
		pEtalonnageMatacq->InitEtalonnage(para_name);
	if (pEtalonnageMust) {
		pEtalonnageMust->InitEtalonnage(para_name);
	}
	if ((GetAcq())->GetDeviceIn()->GetType() == GT_TYPE_FILE)
		(GetAcq())->GetDeviceIn()->Rewind();
	
	tempos.Form("Init Multi  Run on card %s", cardname);
	GetError()->Infos(tempos);

	if (pEtalonnageMatacq) {
		pEtalonnageMatacq->SetModeCalib(calimode);
		pEtalonnageMatacq->InitUser();
	}

	if (pEtalonnageMust) {
		pEtalonnageMust->SetModeCalib(calimode);
		pEtalonnageMust->InitUser();
	}
*/
	InitDone = true;
	if (fStateMachine){
	fStateMachine->SetState(STATE_CONFIGURATED);
	if (!fStateMachine->TestStateToGoTo(STATE_READY)) {
		return;
	}
	fStateMachine->SetState(STATE_READY);
	}
}

//_________________________________________________________________________________________
void GCommand::DoRun(int coups, int* p_mat, int* p_voie) {
	if (fStateMachine){
	if (!(fStateMachine->TestStateToGoTo(STATE_INIT_RUN))) {
		return;
	}

	fStateMachine->SetState(STATE_INIT_RUN);
	}

	if (!InitDone) {
		GetError()->TreatError(1, 0, "Init not done so impossible to do a run");
		return;
	}
	bool context;
	context = true;
	if (fVerbose >3)GetError()->Infos(
			"----------------------------------------------------------------------");

	(fAcq)->SetCoups(coups);

	if (!GetDevice()->GetIsOpen()) {
		context = false;
		GetDevice()->Open();
	}

	((GEtalonnageMust*) fAcq)->SetVectors(p_mat, p_voie);
	if (fStateMachine)
	if (!(fStateMachine->TestStateToGoTo(STATE_RUNNING))) {
		return;
	}
	if (fStateMachine)fStateMachine->SetState(STATE_RUNNING);

	(fAcq)->DoRun(coups);

	if (!context)
		GetDevice()->Close();
	if (fStateMachine)
	if (!(fStateMachine->TestStateToGoTo(STATE_END_RUN))) {
		return;
	}

	if (fStateMachine)fStateMachine->SetState(STATE_END_RUN);
	if (fStateMachine)
	if (!(fStateMachine->TestStateToGoTo(STATE_READY))) {
		return;
	}
	if (fStateMachine)fStateMachine->SetState(STATE_READY);

}
//_________________________________________________________________________________________
void GCommand::ToDoInCaseOfInterrupt() {
	Kill();
}
//_________________________________________________________________________________________
void GCommand::Kill() {
    if (fStateMachine){
         
    	if ((fStateMachine->TestStateToGoTo(STATE_END_RUN,false))) StopRun(true);
    	if ((fStateMachine->TestStateToGoTo(STATE_END_ACQ,false))) EndAcq();

    	fStateMachine->SetState(STATE_KILLED);
    }
	GetError()->Infos("Kill send");

	//EndRun(); //TODO

	if (fAcq) {
		delete (fAcq);
		fAcq = NULL;
	}
	if (fNetServRoot != NULL) {
		((GNetServerRoot*) fNetServRoot)->StopServer(false);
	}

#if NET_LIB
	if (fNetServSoap != NULL) {
		((GNetServerSoap*) fNetServSoap)->StopServer();
	}
#endif
	exit(0);
	return;
}

//_________________________________________________________________________________________
void GCommand::InitSpectraServer(int mode, int port) {
	fError.TreatError(0, 0, "InitSpectra Soap Server starting");
#if NET_LIB
	if (fNetServSoap!=NULL) {
		if (fNetServSoap->InheritsFrom((GNetServerSoap::Class()))) {
			((GNetServerSoap*) fNetServSoap)->InitSpectraServer(mode, port);
		}
	}
#endif
}
//_________________________________________________________________________________________
void GCommand::ConfigVigru() {

	//	GSpectraDB * DB;
	//	DB = ((GEtalonnage*) fAcq)->GetSpectra()->GetDB();
	//	GVClass *GvClass = new GVClass(DB);

}
//_________________________________________________________________________________________
void GCommand::RefreshSpectraDB() {
	// TODO verifie fSpectra

	if (fSpectra)
		fSpectra->UpdateSpectraList();
	else
		fError.TreatError(1, 0,
				"Command::RefreshSpectraDB() : no data base defined");
	return;
}

//_________________________________________________________________________________________
void GCommand::SpectrumReset(char* namespectrum, char* family) {
	// reset histogram named 'namespectra' and with family 'family' to client

	GSpectrumIdentity * id;

	if (fSpectra == NULL) {
		fError.TreatError(1, 0, "No data base defined");
		return;
	}

	TNamed *sp = NULL;

	//TMessage mess(kMESS_OBJECT);

	//fThreadNet->Lock();
	if (family != NULL) {
		id = fSpectra->GetIdentity(namespectrum, family);
	} else {
		id = fSpectra->GetIdentity(namespectrum);
	}
	if (id == NULL) {
		fError.TreatError(1, 0, "Spectrum doesn't exist :", namespectrum);
		return;
	}
	sp = id->GetSpectrum();
	if (sp == NULL) {
		fError.TreatError(1, 0, "Spectrum doesn't exist :", namespectrum);
		return;
	}

	if (!(sp->InheritsFrom(TNamed::Class()))) {
		fError.TreatError(1, 0, "It's not a TNamed : ", namespectrum);
		//fThreadNet-> UnLock();
		return;
	}
	// if(fVerbose)

	id->Reset();

	return;
}
//_________________________________________________________________________________________
void GCommand::DumpCommand(char** commandsliced, int nb_words) const {
	TString tempos;
	tempos = "Received Command = ";
	for (int i = 0; i < nb_words; i++) {
		tempos += commandsliced[i];
		tempos += " ";
	}
	if (fVerbose > 1)
		fError.Infos(tempos);
}
/*
 //_________________________________________________________________________________________
 bool GCommand::AnalyseCommandAndDo(char* command) {

 TString tempos;

 int commanddone = 0;
 bool retour = true;
 int nb_words;
 int type = GA_NOT_DEFINED;
 char** commandsliced = NULL;
 int* commandsliced_int = NULL;
 nb_words = Slicer(command, &commandsliced, &commandsliced_int);

 if (nb_words == 0)
 return (false);
 // test of fist word that must be always "GRU"
 if (!(TestGruWord(commandsliced[0]))) {
 return (false);
 }
 if (CompareWordsIgnoreCase(commandsliced[1], "kill")) {
 commanddone = 1;
 Kill();
 kill_flag = 1;
 }
 if (CompareWordsIgnoreCase(commandsliced[1], "initruns")) {
 if (nb_words >= 3) {
 InitRuns(commandsliced_int[2]);
 commanddone++;
 }

 if (CompareWordsIgnoreCase(commandsliced[1], "initcalim")) {
 if (nb_words == 8) {
 InitMultiRun(commandsliced[2], commandsliced[3],
 commandsliced[4], commandsliced[5],
 commandsliced_int[6], commandsliced_int[7]);
 commanddone++;
 }
 */
