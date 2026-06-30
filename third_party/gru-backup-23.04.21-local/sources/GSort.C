// Author: Luc Legeard
//////////////////////////////////////////////////////////////////////////////
//
// Class GSort
//
// This Class  merges and sorts events with timestamps order from differents run files.
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

#include "General.h"
#include "GSort.h"
#include "GTape.h"
#include "GAcq.h"
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

ClassImp(GSort);
//_______________________________________________________________________________
GSort::GSort() {

}
//_______________________________________________________________________________
GSort::~GSort() {
}

//_______________________________________________________________________________
void GSort::Help() {

	cout << " GSort : merges and sorts events with timestamps order from differents files.\n";
	cout << " Usage   : GSort * gs= new GSort(); \n";
	cout << "           gs->Merge(filename1 filemane2 [filename3] [filename4] etc.....);\n";
	cout << " Example : GSort * gs= new GSort();\n";
	cout << "           gs->Merge(\"/data/e656X/e656/acquisition/run_12/run_0077.dat.07Oct13_05h58m49s /data/e656X/e656/acquisition/run_13/run_0077.dat.07Oct13_05h58m49s /data/e656X/e656/acquisition/run_14/run_0077.dat.07Oct13_05h58m49s\");\n";
	cout << " Output  : is a file named Sorted_filename1 \n\n";

}
//_______________________________________________________________________________

void GSort::Merge(char *liste_files) {

	char** liste_files_vector = NULL;
	int* liste_files_vector_int = NULL;

	int entries = Slicer(liste_files, &liste_files_vector,
			&liste_files_vector_int);

	int testnum = 000;
	int verbose = 0;
	bool buffoutisempty = true;
	int itestnum = 0;

	GTape *file[entries];
	GTape *fileout;
	int NbBuffersOut;
	GAcq *a[entries];
	int statusb[entries];
	int statusf[entries];
	int NbBuffersIn[entries];
	int NbEventsIn[entries];
	bool runfinished[entries];
	int i = 0;
	int countofchange = 0;
	long long int timestamp[entries];
	GBufferIn2p3* buffout;
	int fEventCounter = 0;
	int numOfFirstEventInBuffer = 0;
	long long int tsolder = 0;
	int entryolder = 0;
	int loopcount1, loopcount2;
	int lastentryolder = 0;
	int buffersize = 0;
	TString filename1 = liste_files_vector[0];
	TString devname;
	TString filenametempo, filenameout, fileentete;
	fileentete.Form("./runentete.dat");
	filenametempo.Form("Runtempo.dat");
	TObjArray * listedir;
	filename1 = filename1.Remove(TString::kBoth, '/');
	listedir = filename1.Tokenize("/");
	filename1 = (listedir->At(listedir->GetLast()))->GetName();
	filenameout.Form("Sorted_%s", filename1.Data());
	loopcount1 = 0;
	loopcount2 = 0;

	for (i = 0; i < entries; i++) {
		cout << "-----------------------Init and first read " << i
				<< "-------------------\n";

		devname.Form("%s", liste_files_vector[i]);

		file[i] = new GTape(devname.Data());
		if (i == 0) {
			file[0]->ExtractHeaders((char*) (fileentete.Data()));
			buffersize = file[0]->GetBufferSize();
		}
		file[i]->Open();
		a[i] = new GAcq(file[i]);
		a[i]->EventInit();
		a[i]->InitUser();
		statusb[i] = ACQ_OK;
		statusf[i] = ACQ_OK;
		runfinished[i] = false;
		while (true) {
			file[i]->ReadBuffer();
			if (file[i]->GetCurrentBuffer()->IsEventBuffer() == true)
				break;
		}
		if (verbose) {
			cout << "Read buffer in branch : " << i << " buff number : "
					<< file[i]->GetCurrentBuffer()->GetNumBuf();
			file[i]->GetCurrentBuffer()->DumpBuffer(128, 0);
		}
		NbBuffersIn[i] = 1;
		NbEventsIn[i] = 1;
	}
	cout
			<< "----------------------define out file and out buffer--------------\n";
	fileout = new GTape(filenametempo.Data());
	fileout->SetBufferSize(buffersize);
	fileout->SetCompressionLevel(0);
	fileout->Close();
	fileout->Open('n');
	NbBuffersOut = 0;
	buffout = new GBufferIn2p3(buffersize);
	buffout->RazBuffer();

	// get 3 first events
	for (i = 0; i < entries; i++) {
		cout << "-----------------------Get first event in branch " << i
				<< "--------------\n";
		while (true) {//80180E3ACQ_ENDOFBUFFER

			statusb[i] = a[i]->GetEvent()->NextEvent(
					file[i]->GetCurrentBuffer());
			if (statusb[i] == ACQ_OK)
				break;
			else {

				file[i]->ReadBuffer();
				statusf[i] = file[i]->GetStatus();
				if (statusf[i] != ACQ_OK)
					statusf[i] = ACQ_ENDOFFILE;
			}
		}
		if (verbose)
			file[i]->GetCurrentBuffer()->DumpBuffer(64, 0);
		timestamp[i] = a[i]->GetEvent()->GetTimeStamp();
	}

	cout << "-----------------------loops----------------------------\n";
	while (true) {

		if (loopcount1++ > 100000) {
			loopcount1 = 0;
			cout << "." << flush;
			if (loopcount2++ > 30) {
				cout << "\n";
				loopcount2 = 0;
			}
		}

		if (testnum != 0)
			if (itestnum++ == testnum)
				break;
		lastentryolder = entryolder;
		entryolder = 0;
		for (i = 0; i < entries; i++) {
			if (runfinished[i] != true) {
				tsolder = timestamp[i];
				entryolder = i;
				break;
			}
		}
		for (i = 0; i < entries; i++) {
			if (runfinished[i] != true) {
				if (timestamp[i] == 0)
					cout << "\nWarning timestamp = 0 for banche " << i
							<< " General Event counter : " << fEventCounter
							<< "  Branch Event counter : " << NbEventsIn[i]
							<< "\n";
				if (timestamp[i] <= timestamp[entryolder])
					entryolder = i;
			}

		}
		tsolder = timestamp[entryolder];
		// result
		if (verbose) {
			cout << "-----The time stamp sorted-" << fEventCounter
					<< " for branch:" << entryolder << "--";
			cout << "Time " << tsolder << "\n";
			for (i = 0; i < entries; i++) {
				cout << " TS[" << i << "]: " << timestamp[i];
			}
			cout << endl;
		}

		//verif
		for (i = 0; i < entries; i++) {
			if ((timestamp[i] < timestamp[entryolder]) and (runfinished[i]
					!= true))
				cout << " error for event " << fEventCounter << "\n";
		}
		if (lastentryolder != entryolder)
			countofchange++;

		if (runfinished[entryolder] != true) {
			fEventCounter++;
			NbEventsIn[entryolder]++;

			GEvent * localevent = (GEvent *) a[entryolder]->GetEvent();

			//store our buffer if full
			if ((int) (buffout->GetBufSize())
					- (int) (buffout->GetUsedEventsSize() + GANIL_BUF_HD_SIZE)
					<= (int) (localevent->GetEventDataSize() + 4)) {

				NbBuffersOut++;
				buffout->MakeEBYEDATHeader(NbBuffersOut, 1523, 1541, 1234,
						(fEventCounter - numOfFirstEventInBuffer));

				fileout->WriteBuffer(buffout);
				numOfFirstEventInBuffer = fEventCounter;
				buffout->RazBuffer();
				buffoutisempty = true;
				if (verbose) {
					cout << " OUT --------------Buffers read= ";
					for (i = 0; i < entries; i++)
						cout << "-" << NbBuffersIn[i];
					cout << "----Buffers written--" << NbBuffersOut
							<< "----!\n";
				}

			}

			//store our event in buffer
			//cout <<" fillbuffout\n";

			localevent->FillBufferWithRawEvent(buffout);
			buffoutisempty = false;
		}
		//_______________________________________________________________________________________________
		if (verbose) {
			cout << "get newevent\n";
			file[entryolder]->GetCurrentBuffer()->DumpBuffer(256, 0);
		}
		if (!(runfinished[entryolder])) {
			statusb[entryolder] = a[entryolder]->GetEvent()->NextEvent(
					file[entryolder]->GetCurrentBuffer());

			if (statusb[entryolder] != ACQ_OK) {
				while (true) {
					if (runfinished[entryolder])
						break;
					NbBuffersIn[entryolder]++;
					file[entryolder]->ReadBuffer();
					if (verbose) {
						cout << "Read buffer in branch " << entryolder
								<< " number "
								<< file[entryolder]->GetCurrentBuffer()->GetNumBuf();
						file[entryolder]->GetCurrentBuffer()->DumpBuffer(16, 0);
					}
					if (file[entryolder]->GetStatus() == ACQ_ENDOFFILE) {
						runfinished[entryolder] = true;
						statusf[entryolder] = file[entryolder]->GetStatus();
						statusb[entryolder] = ACQ_ENDOFFILE;
						timestamp[entryolder] = -1;
						break;
					}
					if (file[entryolder]->GetCurrentBuffer()->IsEventBuffer()
							== true)
						break;
				}
				statusf[entryolder] = file[entryolder]->GetStatus();
				if (!(runfinished[entryolder]))
					statusb[entryolder] = a[entryolder]->GetEvent()->NextEvent(
							file[entryolder]->GetCurrentBuffer());

			}
			timestamp[entryolder] = a[entryolder]->GetEvent()->GetTimeStamp();
		}

		int sum = 0;
		int sum2 = 0;
		for (i = 0; i < entries; i++) {
			if (statusf[i] != ACQ_OK) {
				//cout << "statusf["<<i<<"] = "<<statusf[i]<<"\n";
				sum++;
			}
			if (runfinished[i]) {
				//cout << "runfinished["<<i<<"] = "<<runfinished[i]<<"\n";
				sum2++;
			}
		}
		for (i = 0; i < entries; i++) {
			if (statusf[i] != ACQ_OK)
				sum++;
		}
		if ((sum == entries) || (sum2 == entries)) {
			cout
					<< "-----------------------End of inputes files ---------------------\n";

			break;
		}

	}
	if (buffoutisempty == false) {
		NbBuffersOut++;
		buffout->MakeEBYEDATHeader(NbBuffersOut, 1523, 1541, 1234,
				(fEventCounter - numOfFirstEventInBuffer));
		//buffout->DumpBuffer(256,0);
		fileout->WriteBuffer(buffout);
		numOfFirstEventInBuffer = fEventCounter;
		buffout->RazBuffer();
		NbBuffersOut++;
		buffout->MakeEndRunHeader(NbBuffersOut);
		fileout->WriteBuffer(buffout);
		buffoutisempty = true;
		if (verbose) {
			cout << " OUT --------------Buffers read= ";
			for (i = 0; i < entries; i++)
				cout << "-" << NbBuffersIn[i];
			cout << "----Buffers written--" << NbBuffersOut << "----!\n";

		}
	}

	for (i = 0; i < entries; i++) {
		cout << "Branch : " << i << " Nb Buffers : " << NbBuffersIn[i]
				<< " Nb events  out :" << NbEventsIn[i] << "\n";
	}

	TString tempos;
	cout
			<< "\n------Wait few seconds to write final file---------------------\n";
	tempos.Form("cat %s %s > %s", fileentete.Data(), filenametempo.Data(),
			filenameout.Data());
	gSystem->Exec(tempos);
	cout << "Nb event  out      :" << fEventCounter << "\n";
	cout << "Nb Buffers out     :" << NbBuffersOut << "\n";
	cout << "Nb of change       :" << countofchange << "\n";
	cout << "-----------------------End---------------------\n";
	tempos.Form("rm %s %s ", fileentete.Data(), filenametempo.Data());
	gSystem->Exec(tempos);

	if (liste_files_vector) {
		for (int i = 0; i <entries; i++) {
			delete[] liste_files_vector[i];
			liste_files_vector[i] = NULL;
		}
		delete[] liste_files_vector;
		liste_files_vector = NULL;
	}
	if (liste_files_vector_int) {
		delete[] liste_files_vector_int;
		liste_files_vector_int = NULL;
	}

	return;
}
