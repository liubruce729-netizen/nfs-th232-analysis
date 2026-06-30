#include "GruSoapH.h"
#include "GruSoap.nsmap"
#include "GSoapErrorCode.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include "GruInterfaceSoap.h"
#include "GSoapErrorCode.h"
#include "GSoapErrorCodef.h"
#include "stdsoap2.h"
using namespace std;
//struct Namespace *namespaces;
Namespace namespaces[1024];
/*
 Pour lancer des commandes de type
 ./GruSoapClient hostservername:portnumber GRU WORD2 WORD3 WORD4 WORD5 WORD6
 */

#define XandY 2  // nb of Mufee card
#define NB_TELESCOPES 4
#define NB_MATES  18  // mates nb /telescop
#define NB_CHANNELS_PER_MATE 16 // nb of channel by mates
#define CHANNEL_SIZE 16384 // Channel size

int fVerbose =1;
void soap_end(int rcode) {

}
bool testwordgru(char* word) {
	if ((strcasecmp(word, "GRU") == 0) or (strcasecmp(word, "CALIMERO") == 0)) {
		return true;
	} else {
		printf("         WORD1  = always GRU (and sometime CALIMERO) \n");
		return false;
	}
}

void store_frame(void * frame, int framesize, string filename) {
	ofstream file_scope(filename.c_str(), ios::out);
	int fLun; // Logical Unit Number
	fLun = open(filename.c_str(), (O_RDWR | O_CREAT | O_TRUNC), 0644);
	if (fLun == -1)
		perror("filename");
	else {
		cout << "write " << framesize << " bytes to " << filename << endl;
		int verif = write(fLun, frame, framesize);
		if (verif != framesize)
			perror(filename.c_str());
	}
}

int main(int argc, char **argv) {
	int i, size;
	unsigned char j = 0;
	int port = 6603;
	char destination[200];
	char *command = NULL;
	strcpy(destination, "localhost");
	bool soapsend = false;
	char *word;
	char destination_port[256];
	int commanddone = 0;
	int number_of_word_not_include_in_message;
	number_of_word_not_include_in_message = 0;

	xsd__base64Binary v_reponse;
	ns__ReturnCode r_reponse;
	ns__charliste t_reponse;

	struct soap v_soap;
	//soap_init(&v_soap);
	v_soap.send_timeout = 100;     // 10 seconds max socket delay 
	v_soap.recv_timeout = 10;  
	soap_init(&v_soap);
	v_soap.send_timeout = 10;  
  	   // 10 seconds max socket delay 
  	//v_soap.accept_timeout = 30; // server stops after 1 hour of inactivity 
  	v_soap.max_keep_alive = 10;  // max keep-alive sequence
  	//v_soap.linger_time = 5;
  	v_soap.bind_flags = SO_REUSEADDR;
	//soap_init(&v_soap);
	
	if (fVerbose >9){
		printf("v_soap.linger_time       = %d\n", v_soap.linger_time);
  		printf("v_soap.send_timeout      = %d\n", v_soap.send_timeout );
  		printf("v_soap.recv_timeout      = %d\n", v_soap.recv_timeout);
  		printf("v_soap.max_keep_alive    = %d\n", v_soap.max_keep_alive);
  		printf("v_soap.bind_flags        = %d\n", v_soap.bind_flags );
  		printf("SO_LINGER                = %d\n", SO_LINGER);
	  	printf("SO_REUSEADDR             = %d\n", SO_REUSEADDR);
	}

	soap_set_namespaces(&v_soap, GruSoap_namespaces);
	unsigned char* pt = NULL;
	int entier = 0;
	int entier2 = 0;

	/*	if (argc == 1) {
	 //Pour lancer des commandes de type
	 //GRUC host:port
	 //pour faire des test uniquement

	 sprintf(destination_port, "%s:%d", destination, port);
	 soap_call_ns__Aword0(&v_soap, destination_port, "", r_reponse);

	 if (v_soap.error) {
	 soap_print_fault(&v_soap, stderr);
	 }
	 printf("Client Word0 : ");
	 message_code(r_reponse.rCode);
	 soap_end(&v_soap);
	 commanddone++;
	 }*/

	if ((argc == 2) or (argc == 1)) {
		// simple help
		if ((argc == 2) and ((strcasecmp(argv[1], "help") == 0) or (strcasecmp(
				argv[1], "--help") == 0) or (strcasecmp(argv[1], "-h") == 0))
				or (argc == 1)) {
			printf("GRUC usage \n");
			printf(
					"       GRUC server_host_name:port_number [WORD1] [WORD2].....\n");
			printf("            WORD1  = always GRU \n");
			printf("            WORD2  =  TEST  , INFO ...\n");
			printf("            WORD3  = argument to complete WORD2 ");
			printf("            example : GRUC localhost:6603 TEST 5\n");
			printf(
					"            example : GRUC localhost:6603 GRU GET SPECTRA LIST\n");
			printf(
					"            example : GRUC localhost:6603 GRU GET SPECTRUM NAME FAMILLY\n");
			printf(
					"            example : GRUC localhost:6603 GRU GET SPECTRUM SoapSpectra8192 MyFamilySoap\n");
			printf(
					"            example : GRUC localhost:6603 GRU RUN 500  1 1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1\n");
			printf(
					"            example : GRUC localhost:6603 GRU INPUT NARVAL NarvalActorWatcherHost NarvalActorWatcherPort \n");
			printf(
					"            example : GRUC localhost:6603 GRU INPUT BUFFERSIZE BufferSizeToSet\n");
			printf(
					"            example : GRUC localhost:6603 GRU INITACQ GUSER\n");
			printf(
					"            example : GRUC localhost:6603 GRU INITEVENT ExperimentName\n");

			printf(
					"            example : GRUC localhost:6603 GRU SPECTRA ALL\n");
			printf("            example : GRUC localhost:6603 GRU INITRUNS 1\n");

			printf("            example : GRUC localhost:6603 GRU RUN START\n");
			printf("            example : GRUC localhost:6603 GRU RUN STOP\n");
			printf("            example : GRUC localhost:6603 GRU KILL\n");
			commanddone++;
		}
	}

	if (argc >= 2) {
		// we test the 2nd argument (argv[1])
		// if it is in the format : host:port_number
		word = new char[strlen(argv[1]) + 1];
		char* wordtmp, *wordtoken;
		strcpy(word, argv[1]);
		if (strcasecmp(argv[1], word) == 0) {
			wordtoken = strtok_r(word, ":", &wordtmp);

			if ((strcasecmp(wordtoken, argv[1]) == 0)) {
				printf("%s not in format server_host_name:port_number \n",
						argv[1]);
				return (0);
			}

			strcpy(destination, wordtoken);
			wordtoken = strtok_r(NULL, ":", &wordtmp);
			int test = 0;

			test = atoi(wordtoken);

			if ((test > 1000) && (test < 20000)) {

				number_of_word_not_include_in_message = 1;
				port = test;
				sprintf(destination_port, "http://%s:%d", destination, port);

			} else {
				printf(
						"Port number not understood.. so we keep default port number : %d\n",
						port);
			}
		}
	}

	if (argc >= 3) {
		if (!testwordgru(argv[2])) {
			return 0;
		}
		int length_command = 0;
		for (int i = 2; i < argc; i++) {
			length_command += strlen(argv[i]) + 1;

		}

		command = new char[length_command + 1];
		strcpy(command, "");
		for (int i = 2; i < argc; i++) {
			sprintf(command, "%s %s", command, argv[i]);
		}
	}

	if (argc == 3) {

		soap_call_ns__Aword0(&v_soap, destination_port, "", r_reponse);

		if (v_soap.error) {
			soap_print_fault(&v_soap, stderr);
		}
		printf("Client Aword0 : ");
		message_code(r_reponse.rCode);
		soap_end(&v_soap);
		return (0);
	}

	if (argc > 3) {
		if (strcasecmp(argv[3], "GETNUMTRACE") == 0) {
		}

		if (strcasecmp(argv[3], "get") == 0) {
			if (argc > 4) {
				if (strcasecmp(argv[4], "spectrum") == 0) {
					soap_call_ns__AwordsRvectorInt(&v_soap, destination_port,
							"", command, v_reponse);
					if (v_soap.error) {
						soap_print_fault(&v_soap, stderr);
					}
					pt = v_reponse.__ptr;
					int * vecteuru =(int*)(v_reponse.__ptr);
					size = v_reponse.__size;
					int sizeofelement = sizeof(int);

					printf("retour Size trace= %ld: \n", (long) size);
					printf("v_reponse ptr=%ld\n", (long) (v_reponse.__ptr));
					int sizeofhisto = v_reponse.__size / sizeofelement;

					for (i = 0; i < sizeofhisto; i++) {
						int value = vecteuru[i];
						printf ( "-%d", value);
						if (((i+1)%16)==0) printf( "\n");
					}
					printf("\n-fin\n");
					printf("Client Spectra : ");
					message_code(r_reponse.rCode);
					commanddone++;
				}

				if (commanddone == 0) {

					soap_call_ns__AwordsRwords(&v_soap, destination_port, "",
												command, t_reponse);
					if (v_soap.error) {
							soap_print_fault(&v_soap, stderr);
							}
					printf("Liste  : %s \n", t_reponse.charlistwords);
					printf("Nb entries : %d", t_reponse.nb_list);
					printf("\nfin\n");
					commanddone++;

				}
			}
		}// end fo : if (strcasecmp(argv[3], "get") == 0) {

		if ((strcasecmp(argv[3], "run") == 0) and (argc > 9)) {

			//   Pour lancer des commandes de type
			//  ./GRUC localhost:6603 GRU RUN 500  1 1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1
			//   mais en fait le nombre de parametre (1 1 1 1...) est * 4 pour atteindre 72 
			int vector_mat[XandY * NB_TELESCOPES];
			int vector_voie[NB_TELESCOPES * NB_MATES];
			//xsd__base64Binary mat;
			//xsd__base64Binary voie;
			ns__MyVector mat;
			ns__MyVector voie;
			unsigned char* pt = NULL;
			//mat.size  = XandY * NB_TELESCOPES;
			//mat.vector = vector_mat;
			//voie.size = NB_TELESCOPES * NB_MATES;
			//voie.vector = vector_voie;
			//mat.__ptr = (unsigned char*) soap_malloc(&v_soap,
				//					XandY * NB_TELESCOPES*sizeof(int));
			mat.__ptr = (int *) soap_malloc(&v_soap,
												XandY * NB_TELESCOPES*sizeof(int));
			mat.__size = XandY * NB_TELESCOPES;

			//*sizeof(int);

			//voie.__ptr = (unsigned char*) soap_malloc(&v_soap,
				//							NB_TELESCOPES * NB_MATES*sizeof(int));
			voie.__ptr = (int*) soap_malloc(&v_soap,
													NB_TELESCOPES * NB_MATES*sizeof(int));
			voie.__size = NB_TELESCOPES * NB_MATES;
			//*sizeof(int);

			entier = atoi(argv[3 + number_of_word_not_include_in_message]);

			char* thecopy = (char*) mat.__ptr;
			const char* tocopy = (const char*) vector_mat;

			for (i = 0; i < XandY * NB_TELESCOPES; i++) {
				vector_mat[i] = 4000;
			}
			memcpy(thecopy, tocopy, XandY * NB_TELESCOPES*sizeof(int));
			 thecopy = (char*) voie.__ptr;
			 tocopy = (const char*) vector_voie;
			for (i = 0; i < NB_TELESCOPES * NB_MATES; i++) {
				vector_voie[i] = 0;
			}
			memcpy(thecopy, tocopy, NB_TELESCOPES * NB_MATES*sizeof(int));

			// Modif du tele no_tele en fonction des entrees
			for (i = 0; i < NB_MATES; i++) {
				if (argc > i + 3 + number_of_word_not_include_in_message) {
					int value = atoi(argv[i + 4 + number_of_word_not_include_in_message]);
					vector_voie[i + 0*NB_MATES] = value;
					vector_voie[i + 1*NB_MATES] = value;
					vector_voie[i + 2*NB_MATES] = value;
					vector_voie[i + 3*NB_MATES] = value;
				}
			}
			memcpy(thecopy, tocopy, NB_TELESCOPES * NB_MATES*sizeof(int));
			printf(" Command send : %s %s %s %d  \n", destination_port,
					argv[2], argv[2 + number_of_word_not_include_in_message],
					entier);
			printf( "-----------------mat :  value of gene----------------------\n");

			for (i = 0; i < XandY * NB_TELESCOPES; i++) {
				printf("%d ", vector_mat[i] );
				if ((i + 1) % (XandY * NB_TELESCOPES) == 0) printf("\n");
			}

			printf( "----------------selected-channels-------------------------\n");
			for (i = 0; i < NB_TELESCOPES * NB_MATES; i++) {
				printf("%2d  ", vector_voie[i]);
				if ((i + 1) % NB_MATES == 0) printf("\n");
			}
			printf( "-----------------------------------------------------------\n");

			/**/
			soap_call_ns__Calim2(&v_soap, destination_port, "", argv[2], argv[2
					+ number_of_word_not_include_in_message], entier,
					&mat, &voie, r_reponse);
			/*
			soap_call_ns__Calim2(&v_soap, destination_port, "", argv[2], argv[2
							+ number_of_word_not_include_in_message], entier,
							vector_mat, vector_voie, r_reponse);
			if (v_soap.error) {
				soap_print_fault(&v_soap, stderr);
			}
*/
			if (fVerbose>0){
				printf("Code Retour Calim2 = ");
				message_code(r_reponse.rCode);
				printf("\n");
			}
			commanddone++;
		}
		if (commanddone == 0) {

			soap_call_ns__Awords(&v_soap, destination_port, "", command,
					r_reponse);

			if (v_soap.error) {
				soap_print_fault(&v_soap, stderr);
			}
			printf("Client Awords : ");
			message_code(r_reponse.rCode);
			commanddone++;
		}
	} // if >=3

	if (soapsend)
		soap_end(&v_soap);

	if (commanddone != 1) {
		printf("Error in command : nb of executed command = %d\n", commanddone);
	}

	return (0);

}
