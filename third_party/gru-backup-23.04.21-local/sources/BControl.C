/*
 * GTControlConsole.c
 *
 *  Created on: 9 avr. 2018
 *      Author: goux
 */

#include "BControl.h"

using namespace std;

BControl::BControl(char **command, int taille) {
	tailleCommande = taille;
	SetCommandLine(command);
	filename = "";
	fAcq = NULL;	//new GAcq();
	mErreur = false;
	bufSize = DEF_BUF_SIZE;
	actionFile = DEF_ACTION_FILE;
	verbose = DEF_VERBOSE;
	startBlock = DEF_START_BLOCK;
	nbBlock = DEF_NB_BLOCK;
	nbDump = DEF_NB_DUMP;
	nbEvent = DEF_NB_EVENT;
	startEvent = DEF_START_EVENT;
	rawEvent = DEF_RAW_EVENT;
	nbEventDump = DEF_NB_EVENT_DUMP;
	doFile = false;
	doInitBufSize = false;
	doInitEventAction = false;
	doVerbose = false;
	doNbBlock = false;
	doStartBlock = false;
	doDump = false;
	doNbEvent = false;
	doStartEvent = false;
	doEventDump = false;
	doRawEvent = false;
	mError = new CError();
	mapBlock[EVENTDB_Idn] = 0;
	mapBlock[EVENTCT_Idn] = 0;
	mapBlock[EBYEDAT_Idn] = 0;
	mapBlock[SCALER_Idn] = 0;
	mapBlock[ENDRUN_Idn] = 0;
	mapBlock[COMMENT_Idn] = 0;
	mapBlock[FILEH_Idn] = 0;
	mapBlock[PARAM_Idn] = 0;
	mapBlock[EVENTH_Idn] = 0;
	mapBlock[JBUS_Idn] = 0;
	mapBlock[STATUS_Idn] = 0;
	mapBlock[UNKNOWN_Idn] = 0;
}

BControl::~BControl() {
	if (fAcq)
		delete fAcq;
}

void BControl::CorrigerCommandLine() {
	vector<string> chaine(commandLine, commandLine + tailleCommande);
	for (unsigned int i = 1; i != chaine.size(); ++i) {
		if (i < chaine.size() - 1 && chaine[i + 1][0] != '-'
				&& chaine[i + 1][0] != '/') {
			chaine.at(i).append(chaine.at(i + 1));
			chaine.erase(chaine.begin() + i + 1);
			i--;
		}
	}

	tailleCommande = chaine.size();
	commandLine = new char*[chaine.size()];
	for (size_t i = 0; i < chaine.size(); ++i) {
		commandLine[i] = new char[chaine[i].size() + 1];
		strcpy(commandLine[i], chaine[i].c_str());
	}
}

void BControl::SetCommandLine(char **commandLine) {
	this->commandLine = commandLine;
	CorrigerCommandLine();
}

void BControl::Executer() {
	stringstream stream;
	LireLigneCommande();
	VerifierCommande();

	if (!mErreur) {
		//AfficherAttributs();
		SetNameDevice();
		OuvrirDeviceIn();
		InitBufferSize();
		if (verbose >=VERBO_EVENT){
			InitialiserActionFile();
			GenererMapEvent();
		}
		fAcq->GetDeviceIn()->Rewind(true);
		if (verbose >= VERBO_INFO_FILE) {
			fAcq->Infos();
			fAcq->GetDeviceIn()->Infos();
		}

		LireTousBlocks();
		if (verbose >= VERBO_STAT)
			AfficherCompteBlock();
		if (verbose >= VERBO_STAT_EVENT)
			AfficherCompteEvent();
	} else {
		ErrorMessage("An error occurred");
	}

}

void BControl::InitialiserActionFile() {
	if (!doInitEventAction) {
		fAcq->EventInit();
	} else {
		char *cstr = new char[actionFile.length() + 1];
		strcpy(cstr, actionFile.c_str());
		fAcq->EventInit(cstr, (char *) "ganil", false);
	}
}

void BControl::LireTousBlocks() {
	BProgressBar *show_progress = new BProgressBar(
			((GTape*) fAcq->GetDeviceIn())->SizeRuns() / bufSize);
	int nbBuf = 0;
	bool dumperEvent;
	InfoMessage("---------------------- Start reading ----------------------");
	if (verbose < VERBO_HEADER_BLOCK)
		show_progress->displayProgressBarHeader();
	fAcq->GetDeviceIn()->ReadBuffer();
	do {
		dumperEvent = false;
		if (verbose < VERBO_HEADER_BLOCK)
			(*show_progress)++;
		nbBuf++;

		if (((startBlock <= nbBuf && nbBuf < startBlock + nbBlock)
				|| (startBlock <= nbBuf && nbBlock == 0))
				&& (verbose >= VERBO_HEADER_BLOCK || verbose >= VERBO_BLOCK)) {
			dumperEvent = true;

			if (verbose >= VERBO_HEADER_BLOCK)
				AfficherHeaderBlock();
			if (verbose >= VERBO_BLOCK)
				AfficherBlock();
		}
		CompterBlock();

		if ((fAcq->GetDeviceIn()->GetCurrentBuffer()->IsAEventBuffer()) and ( verbose>=VERBO_EVENT)){
			LireTousEvents(dumperEvent);
		}
		if (dumperEvent)
			Barre();
		fAcq->GetDeviceIn()->ReadBuffer();
	} while (fAcq->GetDeviceIn()->GetStatus() == ACQ_OK);
	cout << endl;
	InfoMessage("---------------------- Stop reading ----------------------");
}

void BControl::CompterBlock() {
	int idBlock = fAcq->GetDeviceIn()->GetCurrentBuffer()->fGBuf_type;
	if (mapBlock.find(idBlock) != mapBlock.end())
		mapBlock[idBlock]++;
	else
		mapBlock[UNKNOWN_Idn]++;
}

bool BControl::VerifierCommande() {
	bool juste = true;
	if (!doFile) {
		ErrorMessage("select file using -f or --file");
		juste = false;
	}
	if (verbose < 0 || verbose > 10) {
		juste = false;
		WarningMessage(
				"verbose must be between 0 and 10, default value will be used");
		if (verbose > 10)
			verbose = 10;
		else
			verbose = DEF_VERBOSE;
	}
	if (bufSize <= 0) {
		juste = false;
		WarningMessage("bufsize must be > 0, default value will be used");
		bufSize = DEF_BUF_SIZE;
	}
	if (nbBlock < 0) {
		juste = false;
		WarningMessage("numberDump must be > 0, default value will be used");
		nbBlock = DEF_NB_BLOCK;
	}
	if (startBlock < 0) {
		juste = false;
		WarningMessage("numberstart must be > 0, default value will be used");
		startBlock = DEF_START_BLOCK;
	}
	if (nbDump < 0) {
		juste = false;
		WarningMessage("dump must be > 0, default value will be used");
		nbDump = DEF_NB_BLOCK;
	}
	if (startEvent < 0) {
		juste = false;
		WarningMessage(
				"number of start event must be > 0, default value will be used");
		startEvent = DEF_START_EVENT;
	}
	if (nbEvent < 0) {
		juste = false;
		WarningMessage(
				"number of dumping event must be > 0, default value will be used");
		nbEvent = DEF_NB_EVENT;
	}
	if (nbEventDump < 0) {
		juste = false;
		WarningMessage(
				"number of event dump must be > 0, default value will be used");
		nbEventDump = DEF_NB_EVENT_DUMP;
	}
	return juste;
}

void BControl::AfficherCompteBlock() const {
	map<int, int>::const_iterator it;
	int total = 0;
	stringstream stream;
	stream << "----------------------- STATISTICS -----------------------";
	InfoMessage(&stream);
	InfoMessage("Number of Block in function of there nature :\n");
	stream << " FILEH     : \t" << mapBlock.at(FILEH_Idn)
			<< "\t      EVENTH   : \t" << mapBlock.at(EVENTH_Idn);
	InfoMessage(&stream);
	stream << " COMMENT   : \t" << mapBlock.at(COMMENT_Idn)
			<< "\t      PARAM    : \t" << mapBlock.at(PARAM_Idn);
	InfoMessage(&stream);
	stream << " EBYEDAT   : \t" << mapBlock.at(EBYEDAT_Idn)
			<< "\t      EVENTDB  : \t" << mapBlock.at(EVENTDB_Idn);
	InfoMessage(&stream);
	stream << " EVENTCT   : \t" << mapBlock.at(EVENTCT_Idn)
			<< "\t      SCALER   : \t" << mapBlock.at(SCALER_Idn);
	InfoMessage(&stream);
	stream << " JBUS      : \t" << mapBlock.at(JBUS_Idn)
			<< "\t      STATUS   : \t" << mapBlock.at(STATUS_Idn);
	InfoMessage(&stream);
	stream << " ENDRUN    : \t" << mapBlock.at(ENDRUN_Idn)
			<< "\t      UNKNOWN  : \t" << mapBlock.at(UNKNOWN_Idn);
	InfoMessage(&stream);
	for (it = mapBlock.begin(); it != mapBlock.end(); it++) {
		total += it->second;
	}
	stream << " TOTAL     : \t" << total << "\n";

	InfoMessage(&stream);
}

void BControl::SetNameDevice() {
	char *filenamechar = new char[filename.length() + 1];
	strcpy(filenamechar, filename.c_str());

	GTape *fDevice = new GTape(filenamechar);

	if (fAcq)
		delete (fAcq);
	fAcq = new GAcq(fDevice);

}

void BControl::OuvrirDeviceIn() {
	fAcq->GetDeviceIn()->Open('r');
	if (fAcq->GetDeviceIn()->GetIsOpen() == false)
		exit(0);
}

void BControl::Help() const {
	stringstream stream;
	cout
			<< "---------------------------- Blocktest manual -----------------------------"
			<< endl;
	cout << " DESCRIPTION" << endl;
	cout << "\t This software allows you to check your run files."<< endl;
	cout << "\t It replace Ganil_Tape."<< endl << endl;
	cout << " OPTIONS" << endl;

	afficherOptionHelp(COMC_HELP, COML_HELP, "print this help", false);

	afficherOptionHelp(COMC_FILE, COML_FILE,
			"set the path of file to check (required)");

	stream << "set level of information during checking from 0 to 10 (default is "
			<< DEF_VERBOSE << ")";
	afficherOptionHelp(COMC_VERBOSE, COML_VERBOSE, &stream);

	stream << "set the number of the first block to dump (default is "
			<< DEF_START_BLOCK << ")";
	afficherOptionHelp(COMC_START_BLOCK, COML_START_BLOCK, &stream);

	stream << "set the number of block to dump (default is all)";
	afficherOptionHelp(COMC_NB_BLOCK, COML_NB_BLOCK, &stream);

	stream << "set the number of byte to dump in each dump block (default is "
			<< DEF_NB_DUMP << ")";
	afficherOptionHelp(COMC_NB_DUMP, COML_NB_DUMP, &stream);

	stream
			<< "set the path of the action file if no FILEH block, else set the action files name (default is '"
			<< DEF_ACTION_FILE << "')";
	afficherOptionHelp(COMC_ACTION_FILE, COML_ACTION_FILE, &stream);

	stream << "set the buffer size if not define in the file (default is "
			<< DEF_BUF_SIZE << ")";
	afficherOptionHelp(COMC_BUF_SIZE, COML_BUF_SIZE, &stream);

	stream
			<< "set the number of the first event to dump in a dumped block (default is "
			<< DEF_START_EVENT << ")";
	afficherOptionHelp(COMC_START_EVENT, COML_START_EVENT, &stream);

	stream
			<< "set the number of event to dump in a dumped block (default is all)";
	afficherOptionHelp(COMC_NB_EVENT, COML_NB_EVENT, &stream);

	stream << "display raw event data (default is false)";
	afficherOptionHelp(COMC_RAW_EVENT, COML_RAW_EVENT, &stream, false);

	stream
			<< "if raw event data is selected, set the number of byte to dump in each dumped event in each dumped block (default is "
			<< DEF_NB_EVENT_DUMP << ")";
	afficherOptionHelp(COMC_NB_DUMP_EVENT, COML_NB_DUMP_EVENT, &stream);

	cout << " VERSION" ;
	cout << "\t" << GRU_VERSION ;
	cout << right << endl;
	cout << " EXEMPLE of USE" << endl;
	cout << "\t" << "Blocktest -f=Run3.dat -v=1"<<endl;
	cout << "\t" << "Blocktest -f=Run31.dat --initeventaction=ACTIONS_local1.CHC_PAR -ibs=65536 -v=10 -nd=1 -nt=10 -ned=10";
	cout <<endl;
	cout << "\t> nota :  the option \"-ibs\" doesn't work if isn't associated with the option \"--initeventaction\" ";

	cout <<endl;

	Barre();
}

void afficherOptionHelp(const char *commandeCourte, const char *commandeLongue,
		stringstream *explication, bool avecEgal) {
	afficherOptionHelp(commandeCourte, commandeLongue, explication->str(),
			avecEgal);
	explication->str("");
}

void afficherOptionHelp(const char *commandeCourte, const char *commandeLongue,
		string explication, bool avecEgal) {
	char *cstr = new char[explication.length() + 1];
	strcpy(cstr, explication.c_str());
	afficherOptionHelp(commandeCourte, commandeLongue, cstr, avecEgal);
}

void afficherOptionHelp(const char *commandeCourte, const char *commandeLongue,
		const char *explication, bool avecEgal) {
	stringstream stream;
	int setwComCourte = 5;
	int setwComLongue = 20;
	int setwExplication = 5;

	stream << commandeCourte;
	if (avecEgal)
		stream << "=";
	cout << left << "\t" << setw(setwComCourte) << stream.str();
	cout << setw(1) << " or ";
	stream.str("");
	stream << commandeLongue;
	if (avecEgal)
		stream << "=";
	cout << setw(setwComLongue) << stream.str() << setw(setwExplication) << "\t"
			<< explication << endl;
	stream.str("");
}

void BControl::ErrorMessage(std::string message) {
	mErreur = true;
	mError->TreatError(3, 0, &message);
}

void BControl::ErrorMessage(std::stringstream* stream) {
	ErrorMessage(stream->str());
	stream->str("");
}

void BControl::InfoMessage(std::string message) const {
	mError->TreatError(0, 0, &message);
}

void BControl::InfoMessage(std::stringstream* stream) const {
	InfoMessage(stream->str());
	stream->str("");
}

void BControl::WarningMessage(std::string message) const {
	mError->TreatError(1, 0, &message);
}

void BControl::WarningMessage(std::stringstream* stream) const {
	WarningMessage(stream->str());
	stream->str("");
}

void BControl::AfficherAttributs() const {
	cout << "fichier : " << filename << endl;
	cout << "action file :" << actionFile << endl;
	cout << "buffSize : " << bufSize << endl;
	cout << "verbose : " << verbose << endl;
	cout << "startBlock : " << startBlock << endl;
	cout << "nbBlock : " << nbBlock << endl;
	cout << "nbDump : " << nbDump << endl;
}

void BControl::Barre() const {
	cout
			<< "---------------------------------------------------------------------------"
			<< endl;
}

bool BControl::LireCommande(const char *commandeLine,
		const char *commandeCourte, const char* commandeLongue, bool *doSmthg,
		bool avecValeur) {
	char *tempos = new char[strlen(commandeLine) + 1];
	strcpy(tempos, commandeLine);
	char *wordtempo = strtok(tempos, "=");

	stringstream stream;

	if ((strcasecmp(wordtempo, commandeCourte) == 0)
			or (strcasecmp(wordtempo, commandeLongue) == 0)) {
		if (*doSmthg) {
			stream << "multiple " << commandeCourte << " or " << commandeLongue;
			ErrorMessage(&stream);
		} else {
			*doSmthg = true;
			if (avecValeur) {
				wordtempo = strtok(NULL, "=");
				if (wordtempo == NULL || strcasecmp(wordtempo, "") == 0) {
					stream << commandeCourte << " or " << commandeLongue
							<< " invalid";
					ErrorMessage(&stream);
				} else {
					return true;
				}
			} else {
				return true;
			}
		}
	}
	return false;
}

void BControl::LireLigneCommande() {
	char *tempos;

	if (tailleCommande <= 1) {
		stringstream stream;
		stream << "No command found, use " << COMC_HELP << " or " << COML_HELP
				<< " to see the manual.";
		ErrorMessage(&stream);
		exit(0);
	} else {
		for (int i = 1; i < tailleCommande; i++) {
			if ((strcasecmp(commandLine[i], COMC_HELP) == 0)
					or (strcasecmp(commandLine[i], COML_HELP) == 0)) {
				Help();
				exit(0);
			}

			tempos = new char[strlen(commandLine[i]) + 1];
			strcpy(tempos, commandLine[i]);

			if (LireCommande(tempos, COMC_FILE, COML_FILE, &doFile)) {
				filename.assign(valeurCommande(tempos));
			} else if (LireCommande(tempos, COMC_BUF_SIZE,
			COML_BUF_SIZE, &doInitBufSize)) {
				bufSize = atoi(valeurCommande(tempos));
				doInitBufSize = true;
			} else if (LireCommande(tempos, COMC_VERBOSE,
			COML_VERBOSE, &doVerbose)) {
				verbose = atoi(valeurCommande(tempos));
			} else if (LireCommande(tempos, COMC_NB_BLOCK,
			COML_NB_BLOCK, &doNbBlock)) {
				nbBlock = atoi(valeurCommande(tempos));
			} else if (LireCommande(tempos, COMC_START_BLOCK,
			COML_START_BLOCK, &doStartBlock)) {
				startBlock = atoi(valeurCommande(tempos));
			} else if (LireCommande(tempos, COMC_ACTION_FILE,
			COML_ACTION_FILE, &doInitEventAction)) {
				actionFile.assign(valeurCommande(tempos));
			} else if (LireCommande(tempos, COMC_NB_DUMP,
			COML_NB_DUMP, &doDump)) {
				nbDump = atoi(valeurCommande(tempos));
			} else if (LireCommande(tempos, COMC_NB_EVENT,
			COML_NB_EVENT, &doNbEvent)) {
				nbEvent = atoi(valeurCommande(tempos));
			} else if (LireCommande(tempos, COMC_START_EVENT,
			COML_START_EVENT, &doStartEvent)) {
				startEvent = atoi(valeurCommande(tempos));
			} else if (LireCommande(tempos, COMC_RAW_EVENT,
			COML_RAW_EVENT, &doRawEvent, false)) {
				rawEvent = true;
			} else if (LireCommande(tempos, COMC_NB_DUMP_EVENT,
			COML_NB_DUMP_EVENT, &doEventDump)) {
				nbEventDump = atoi(valeurCommande(tempos));
			} else {
				if (!doFile) {
					doFile = true;
					filename.assign(strtok(tempos, "="));
				} else {
					stringstream mess;
					mess << "invalid command : " << strtok(tempos, "=");
					ErrorMessage(&mess);
				}
			}
		}
	}
}

bool BControl::InitBufferSize() {
	stringstream stream;
	bool contientFILEH=false;
	int size = 0;
	if ((!doInitBufSize) and (bufSize != 0)) {
        //cout << "-------------------GetBufSizeFromFILEH-------------\n";
		size = ((GTape*) fAcq->GetDeviceIn())->GetBufSizeFromFILEH(true);
		if (size <= -1) {
			contientFILEH = false;
			if (!doInitBufSize) {
				stream << "You must specify buffer size value using "
						<< COMC_BUF_SIZE << " or " << COML_BUF_SIZE;
				ErrorMessage(&stream);
				exit(0);
			}
		} else {
			contientFILEH = true;
			if (bufSize != size) {
				stream
						<< "The buffer size value is not the same as the one written in the file. Change from "
						<< bufSize << " to " << size << ".";
				WarningMessage(&stream);
				bufSize = size;
			}
		}
	}
	//cout << "-------------------SetBufferSize-------------\n";
	fAcq->GetDeviceIn()->SetBufferSize(bufSize);
	return contientFILEH;
}

char* valeurCommande(char* commande) {
	char *tmp = new char[strlen(commande) + 1];
	strcpy(tmp, commande);

	tmp = strtok(tmp, "=");
	tmp = strtok(NULL, "=");

	return tmp;
}

void BControl::AfficherBlock() const {
	fAcq->GetDeviceIn()->GetBuffer()->DumpBuffer(nbDump + 1);
}

void BControl::AfficherHeaderBlock() const {
	cout << fAcq->GetDeviceIn()->GetBuffer()->GetDumpBufferHeader();
	cout << endl;
}

void BControl::AfficherEvent() const {
	if (!rawEvent)
		fAcq->DumpEvent('n');
	else {
		AfficherEventBrut();
	}
}

void BControl::AfficherEventBrut() const {
	fAcq->SetStatus(
			fAcq->GetEvent()->NextEvent(
					fAcq->GetDeviceIn()->GetCurrentBuffer()));
	int i;
	stringstream hexStream;
	hexStream << hex;
	char tempo[20];
	stringstream charStream;
	stringstream debutStream;
	CTRL_EVENT *pCtrlEvent =
			(CTRL_EVENT *) ((GEvent*) fAcq->GetEvent())->GetEventCtrl();
	short *brutData = &(pCtrlEvent->ct_par);
	char *pcharDate = (char *) &(pCtrlEvent->ct_par);
	int eventLength = pCtrlEvent->ct_len;
	eventLength = eventLength * 2; // in char size
	int header_size = (int) ((char*) brutData - (char*) pCtrlEvent);
	int scanLength = eventLength - header_size;
	cout << "- Dumping Event, nb :" << fAcq->GetEvent()->fEventNumber
			<< " --timestamps: " << fAcq->GetEvent()->fTimeStamp
			<< " --Length = " << scanLength / 2 << " (in char size) -";
	cout << endl;
	for (i = 0; i < scanLength / 2 && i < nbEventDump; i++) {
		sprintf(tempo, "%02x  ", brutData[i]);
		hexStream << tempo;
		if ((*pcharDate >= 32) && (*pcharDate < 127)) {
			sprintf(tempo, "%c", *pcharDate);
			charStream << tempo;
		} else
			charStream << ".";
		pcharDate++;
		if (i % 8 == 7) {
			debutStream << (i / 8) * 8 << " : ";
			cout << setw(8) << debutStream.str() << " " << hexStream.str()
					<< "  " << charStream.str() << endl;
			debutStream.str("");
			hexStream.str("");
			hexStream << hex;
			charStream.str("");
		}
	}
	debutStream << (i / 8) * 8 << " : ";
	cout << setw(8) << debutStream.str() << " " << hexStream.str() << "  "
			<< charStream.str() << endl;
}

void BControl::CompterEvent() {
	map<int, int> mapMultiEventTmp;
	for (int i = 0; i < fAcq->GetEvent()->GetArrayLabelValueSize() / 2; i++) {
		mapMultiEventTmp[fAcq->GetEventArrayLabelValue_Label(i)]++;
		if (mapEvent[fAcq->GetEventArrayLabelValue_Label(i)] >= 1)
			mapEvent[fAcq->GetEventArrayLabelValue_Label(i)]++;
		else
			mapEvent[fAcq->GetEventArrayLabelValue_Label(i)]--;
	}
	map<int, int>::iterator it;
	for (it = mapMultiEventTmp.begin(); it != mapMultiEventTmp.end(); it++)
		if (it->second > 1)
			mapMultiEvent[it->first] += it->second - 1;
}

void BControl::AfficherCompteEvent() {
	map<int, int>::iterator it;
	stringstream stream;
	int i = 0;
	int totalGood = 0;
	InfoMessage("Number of Event in function of there nature :");
	for (it = mapEvent.begin(); it != mapEvent.end(); it++) {
		if (it->second >= 1) {
			i++;
			totalGood += it->second - 1;
			stream << setw(5) << it->first << setw(1) << " : " << setw(7)
					<< it->second - 1 << setw(1) << " | ";
			if (i % 5 == 0) {
				InfoMessage(&stream);
			}
		}
	}
	InfoMessage(&stream);

	stream << "Total of good label " << totalGood << endl;
	InfoMessage(&stream);

	for (it = mapEvent.begin(); it != mapEvent.end(); it++) {
		if (it->second < 0) {
			stream << "Error for label : " << it->first << ", "
					<< (it->second) * -1 << " times.";
			InfoMessage(&stream);
		}
	}

	for (it = mapMultiEvent.begin(); it != mapMultiEvent.end(); it++)
		if (it->second > 0) {
			stream << "Multi label " << it->first << ", " << it->second
					<< " times.";
			InfoMessage(&stream);
		}

}

void BControl::GenererMapEvent() {
	int max_label = 0;
	int* LabelToExist = fAcq->GetDataParameters()->creatLabelToExist(
			&max_label);
	if (LabelToExist == NULL) {
		stringstream stream;
		stream
				<< "Event generating failed, there are some possibilities like : \r\n\t"
						"- There is no PARAM block in the file. Specify the parameters action file using "
				<< COMC_ACTION_FILE << " or " << COML_ACTION_FILE
				<< ".\r\n\t"
						"- There is a PARAM block in the file but no FILEH block."
						"\r\nRead Carefully all error above.";
		ErrorMessage(&stream);
		exit(0);
	}
	for (int i = 0; i <= max_label; i++)
		mapEvent[i] = LabelToExist[i]; //1 ou 0
}

void BControl::LireTousEvents(bool afficherEvent) {
	int nb = 0;
	do {
		nb++;
		if (((startEvent <= nb && nb < startEvent + nbEvent)
				|| (startEvent <= nb && nbEvent == 0)) && verbose >= VERBO_EVENT
				&& afficherEvent)
			AfficherEvent();
		else
			fAcq->SetStatus(
					fAcq->GetEvent()->NextEvent(
							fAcq->GetDeviceIn()->GetCurrentBuffer()));
		if (fAcq->GetStatus() == ACQ_OK)
			CompterEvent();

	} while (fAcq->GetStatus() == ACQ_OK);
}
