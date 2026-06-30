#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<string>
#include<math.h>
#include<iostream>
#include<pthread.h>
#include <signal.h> //  our new library

#include "Utilities.h"
#include "GruSoapMyServer.h"
#include "Main.h"

volatile sig_atomic_t Myflag = 0;
// serveur SOAP

#include "GruSoapH.h"
#include "GruSoap.nsmap"
Namespace namespaces[1024];

 bool stopping = false;
 bool running= false;

const int mystrlen = 256;
pthread_t pthread_server ;


void InCaseOfMyControlC(int sig);
//_________________________________________________________________________________________

int ns__AwordsRvectorInt(struct soap *p_soap, // contexte d'ex�cution du service web
		char p_chaine1[mystrlen],// GRU GET SPECTRUM NAME FAMILY
		struct xsd__base64Binary &p_out // pour retourner plusieurs valeurs
) {
	int i, j, indice;
	indice = -1;

	int local_type;
	p_out.__ptr = NULL;
	p_out.__size = 0;
	int commandexecuted = 0;
	char* inputwords;
	char** commandsliced;
	int* commandsliced_int;
	int nbwords = 0;
	const char *temp = "MyFamilySoap";
	char *family;
	printf("Server ns__AwordsRvectorInt [%s]\n", p_chaine1);

	nbwords = Slicer(p_chaine1, &commandsliced, &commandsliced_int);

	if (strcasecmp(commandsliced[0], "GRU") == 0) {
		if (strcasecmp(commandsliced[1], "GET") == 0) {
			if (strcasecmp(commandsliced[2], "SPECTRUM") == 0) {
				commandexecuted++;

				unsigned int valueu;
				unsigned char valuec;
				unsigned int *spectra_currentu;
				unsigned char *spectra_currentc;
				int sizeofelement = sizeof(unsigned int);
				int sizeofcharvector;
				int sizeofhisto;

				char *spectrumname = commandsliced[3];
				if (nbwords >= 5)
					family = commandsliced[4];
				else
					family = (char*) temp;
				for (i = 0; i < nb_traces; i++) {
				//	printf(" %s %s %s %s \n", spectrumname, spectra_name[i],
					//		family, family_name[i]);
					if ((strcmp(spectrumname, spectra_name[i]) == 0)
							&& (strcmp(family, family_name[i]) == 0)) {
						indice = i;
						break;
					}
				}

				if (indice != -1) {

					sizeofhisto = spectra_size[indice];
					local_type = spectra_type[indice];

					spectra_currentc = (unsigned char*) spectra_ui[indice];
					sizeofcharvector = sizeofhisto * sizeofelement;

					p_out.__ptr = (unsigned char*) soap_malloc(p_soap,
							sizeofcharvector);
					p_out.__size = sizeofcharvector;

					char* thecopy = (char*) p_out.__ptr;
					const char* tocopy = (const char*) spectra_currentc;

					memcpy(thecopy, tocopy, sizeofcharvector);

					/*
					 spectra_currentc = (unsigned char*)spectra[indice];
					 spectra_currentu = (unsigned short*)spectra[indice];

					 printf("\nContent of original vector in ushort \n");
					 for (i=0; i<sizeofhisto; i++) {
					 valueu = spectra_currentu[i];
					 printf("%d ", (int)valueu);
					 }

					 printf("\nContent of original vector in char \n");
					 for (i =0; i< sizeofcharvector; i++) {
					 valuec = spectra_currentc[i];
					 printf("%d ", (int)valuec);
					 }

					 spectra_currentc = (unsigned char*)p_out.__ptr;
					 spectra_currentu = (unsigned short*)p_out.__ptr;

					 printf("\nContent of out vector in ushort \n");
					 for (i=0; i<sizeofhisto; i++) {
					 valueu = spectra_currentu[i];
					 printf("%d ", (int)valueu);
					 }

					 printf("\nContent of out vector in char\n");
					 for (i =0; i<p_out.__size; i++) {
					 valuec =(unsigned char)spectra_currentc[i];
					 printf("%d ", int(valuec));
					 }
					 */
				}
			}
		}
	}
	FreeChar(commandsliced, commandsliced_int,nbwords);
	if (commandexecuted > 0) {
		printf("Server sns__AwordsRvectorInt : Command done [%s ]\n", p_chaine1);
	} else {
		printf("Server ns__AwordsRvectorInt : Command not understood [%s ]\n",
				p_chaine1);
	}

	return SOAP_OK;
}

//_________________________________________________________________________________________

int ns__AwordsRwords(struct soap *p_soap, // contexte d'execution du service web
		char p_chaine1[mystrlen],// Command
		struct ns__charliste &p_out // pour retourner plusieurs valeurs
) {
	bool commandexecuted = false;
	char* inputwords;
	char** commandsliced;
	int* commandsliced_int;
	int nbwords = 0;
	p_out.nb_list = 0;
	p_out.charlistwords = NULL;
	printf("Server ns__AwordsRwords  [%s]\n", p_chaine1);

	nbwords = Slicer(p_chaine1, &commandsliced, &commandsliced_int);

	if (nbwords >= 2) {

		if (strcasecmp(commandsliced[1], "get") == 0) {
			if (nbwords >= 3) {
				if (strcasecmp(commandsliced[2], "info") == 0) {
					int dimension_info;
					dimension_info = 0;
					int i, j;
					int nb_info = 2;
					p_out.nb_list = nb_info;

					p_out.charlistwords = (char*) soap_malloc(p_soap, 256);

					sprintf(p_out.charlistwords,
							"nb of spectra %d  increment %d\n", nb_traces,
							nb_increment);

					commandexecuted = true;
				}
				if (strcasecmp(commandsliced[2], "spectra") == 0) {
					if (nbwords >= 4) {
						if (strcasecmp(commandsliced[3], "list") == 0) {

							int dimension_listespectra;
							dimension_listespectra = 0;
							int i, j;

							p_out.nb_list = nb_traces;

							for (i = 0; i < nb_traces; i++) {
								dimension_listespectra += strlen(
										spectra_name[i]) + 1 + strlen(
										family_name[i]) + 2; // space + /0
							}

							//printf("Debug1 dimension_liste= %d \n", dimension_listespectra,);
							p_out.charlistwords = (char*) soap_malloc(p_soap,
									dimension_listespectra);
							p_out.charlistwords[0] ='\0';
							for (i = 0; i < nb_traces; i++) {
								strcat(p_out.charlistwords, spectra_name[i]);
								strcat(p_out.charlistwords, " ");
								strcat(p_out.charlistwords, family_name[i]);
								strcat(p_out.charlistwords, " ");
							}
							printf("List spectra [%d]: %s\n", p_out.nb_list,
									p_out.charlistwords);
							commandexecuted = true;
						}
					}
				}
				if (strcasecmp(commandsliced[2], "spectra_db") == 0) {
					if (nbwords == 3) {

						int dimension_listespectra;
						dimension_listespectra = 0;
						int i, j;

						p_out.nb_list = nb_traces;

						for (i = 0; i < nb_traces; i++) {
							dimension_listespectra += strlen(spectra_name[i])
									+ 1 + strlen(family_name[i]) + 2; // 2=/0 +space
						}

						//printf("Debug1 dimension_liste= %d \n", dimension_listespectra,);
						p_out.charlistwords = (char*) soap_malloc(p_soap,
								dimension_listespectra);
						//sprintf(p_out.charlistwords,"");
						p_out.charlistwords[0] = '\0';
						for (i = 0; i < nb_traces; i++) {
							strcat(p_out.charlistwords, spectra_name[i]);
							strcat(p_out.charlistwords, " ");
							strcat(p_out.charlistwords, family_name[i]);
							strcat(p_out.charlistwords, " ");
						}

						printf("Liste spectra [%d]: %s\n", p_out.nb_list,
								p_out.charlistwords);
						commandexecuted = true;

					}
				}

			}
		}
	}
	FreeChar(commandsliced, commandsliced_int,nbwords);
	if (commandexecuted) {
		printf("Server ns__AwordsRwords : Command done [%s ]\n", p_chaine1);
	} else {
		printf("Server ns__AwordsRwords : Command not understood [%s ]\n",
				p_chaine1);
	}
	return SOAP_OK;
}

//_________________________________________________________________________________________

int ns__Aword0(struct soap *p_soap, struct ns__ReturnCode &p_out // pour retourner plusieurs valeurs
) {
	int i;
	p_out.rCode = SOAP_OK;
	printf("----Test-----ns__Word0---do nothing \n");
	printf("Server Word0 [rCode =%d]\n", (int) p_out.rCode);
	return SOAP_OK;
}

//_________________________________________________________________________________________

int ns__Awords(struct soap *p_soap, char p_chaine1[mystrlen],// GRUSOAP
		struct ns__ReturnCode &p_out // pour retourner plusieurs valeurs
) {
	int i;
	p_out.rCode = SOAP_OK;
	char* inputwords;
	char** commandsliced;
	int* commandsliced_int;
	int nbwords = 0; // nb of words only in command
	int commandexecuted = 0;

	nbwords = Slicer(p_chaine1, &commandsliced, &commandsliced_int);

	if (nbwords > 0) {
		if (strcasecmp(commandsliced[0], "GRU") == 0) {
			if (nbwords > 1) {
				if (strcasecmp(commandsliced[1], "SPECTRUM") == 0) {

					if (nbwords >= 5) {

						if (strcasecmp(commandsliced[2], "RESET") == 0) {
							ResetHisto(commandsliced[3], commandsliced[4]);
							commandexecuted++;
						}
					}
					if (nbwords == 4) {
						if (strcasecmp(commandsliced[2], "RESET") == 0) {
							ResetHisto(commandsliced[3]);
							commandexecuted++;
						}

					}
				}
				if (strcasecmp(commandsliced[1], ("KILL")) == 0) {
					InCaseOfMyControlC(0);
				}
				if (strcasecmp(commandsliced[1], ("TEST")) == 0) {
					int test = 0;
					if (nbwords > 2) {
						test = commandsliced_int[2];
					}
					Test(test);
						commandexecuted++;
					printf(
							"--------------TEST done on %d second(s)---------------------\n",
							test);
				}
				if (strcasecmp(commandsliced[1], ("spectra")) == 0) {
					if (strcasecmp(commandsliced[2], ("reset")) == 0) {
						printf(
								"-----------------reset all-------------------------\n");
						ResetAllHisto();
						commandexecuted++;
					}
				}
			}
		}
	}
	FreeChar(commandsliced, commandsliced_int,nbwords);
	if (commandexecuted == 1) {
		printf("Server ns__Awords : Command done [%s ]\n", p_chaine1);
	} else {
		printf("Server ns__Awords : Command not understood [%s ]\n", p_chaine1);
		printf("Nb of commandexecuted %d\n", commandexecuted);
	}
	return SOAP_OK;

}

//_________________________________________________________________________________________

int ns__Calim2(struct soap *p_soap, // contexte d'ex�cution du service web
		char p_chaine1[mystrlen], char p_chaine2[256], int p_code,// nombre de coups
		struct ns__MyVector * p_vector_mat, struct ns__MyVector * p_vector_voie,
		//struct xsd__base64Binary * p_vector_mat,
		//struct xsd__base64Binary * p_vector_voie,
		struct ns__ReturnCode &p_out // pour retourner plusieurs valeurs
) {
	int i;
	int size;
	int size_m = 8;
	int size_v = 72;
	int * ptr;
	int sizeofelement = sizeof(int);
	int * vectori;
	int value = 0; 
	p_out.rCode = SOAP_OK;
	printf("ns__Calim2 [%s:%s:%d:%d:%d]\n", p_chaine1, p_chaine2, p_code,
			size_m, size_v);
			
       p_out.rCode = SOAP_OK;
			
	p_out.rCode = p_code * 2;
	printf("-----------------mat-%d------------------------\n", size_m);
	vectori = (int*) (p_vector_mat->__ptr);
	for (i = 0; i < size_m; i++) {
		//printf("%d ", p_vector_mat.vector[i]);
		value = vectori[i];

		printf(" %d", vectori[i]);
		if ((i + 1) % (size_m) == 0)
			printf("\n");
	}
	printf("-----------------voie-%d------------------------\n", size_v);
	vectori = (int*) (p_vector_voie->__ptr);
	for (i = 0; i < size_v; i++) {
		printf(" %d", vectori[i]);
		if ((i + 1) % ((size_v / size_m) * 2) == 0)
			printf("\n");
	}
	return SOAP_OK;
}
//_________________________________________________________________________________________

void * handler(void * p_soap) {

	struct soap * v_soap = (struct soap *) p_soap;
	pthread_detach(pthread_self());
	GruSoap_serve(v_soap);
	soap_destroy(v_soap);
	soap_end(v_soap);
	soap_done(v_soap);
	free(v_soap);
	printf(" Waiting for command>>>\n");
	pthread_exit(NULL);
	return NULL;
}

//_________________________________________________________________________________________
void * soapserver(void * newport) {
	pthread_detach(pthread_self());
	struct soap v_soap; // contexte du service SOAP
	struct soap *v_tsoap; // on clone le contexte SOAP pour le thread
	pthread_t v_tid; // l'identifiant du thread de connexion
	// on initialise la socket
	
	soap_init(&v_soap);
	v_soap.send_timeout = 10;     // 10 seconds max socket delay 
  	v_soap.recv_timeout = 10;     // 10 seconds max socket delay 
  	//v_soap.accept_timeout = 30; // server stops after 1 hour of inactivity 
  	v_soap.max_keep_alive = 10;  // max keep-alive sequence
  	v_soap.linger_time = 5;
  	v_soap.bind_flags = SO_REUSEADDR; 
  	printf("Info on parameters of soap server :\n");
  	printf(" v_soap.linger_time       = %d\n", v_soap.linger_time);
  	printf(" v_soap.send_timeout      = %d\n", v_soap.send_timeout );
  	printf(" v_soap.recv_timeout      = %d\n", v_soap.recv_timeout);
  	printf(" v_soap.max_keep_alive    = %d\n", v_soap.max_keep_alive);
  	printf(" v_soap.bind_flags        = %d\n", v_soap.bind_flags );
 
	int theport;
	theport = *((int*) newport);
	soap_set_namespaces(&v_soap, GruSoap_namespaces);

	// on cree la socket mere de connexion
	int j= 0;
	int code =-1;
	while ((j++<10) && (code<0)){
	code = soap_bind(&v_soap, NULL, theport, 100);
			if (code < 0) {
			soap_print_fault(&v_soap, stderr);
			printf("Starting server on  port %d  with Error code %d \n", theport,code);
			printf("\nWait one minute and retry %d \n",j);
			int i = 0;
			for ( i =0; i < 10;i++ ) { printf(" . ");fflush(stdout);sleep(1);}; printf("\n");
		}
	}
	if  (code < 0) {
		printf(" Starting server impossible\n");
		exit (0);
		}

	printf(" Demarrage serveur soap sur port : %d\n", theport);
	printf(" Waiting for command>>>\n");

	// on gere une boucle infinie pour recevoir les requ�tes
	while  (stopping == false) {

		v_soap.accept_timeout = 1; // on rend l'attente non bloquante
		if (soap_accept(&v_soap) < 0) {
			continue;
		}

		v_tsoap = soap_copy(&v_soap);
		pthread_create(&v_tid, NULL, handler, (void *) v_tsoap); //lancement du thread de connexion

	}
	printf(" End command>>>\n");
	soap_done(&v_soap);
	pthread_exit(NULL);// exit du theard serveur soap
	return NULL;
}

//__________________________________________________________________________________________
void InCaseOfMyControlC(int sig) { // can be called asynchronously
	Myflag = 1; // set flag
	printf("-> Coucou c'est un kill ou un Ctr C alors on arrete !\n");
	pthread_cancel(pthread_server);
	exit(0);
}
//__________________________________________________________________________________________
void  Launch_SoapServer (int port) {

int data = 0;
	// test if no out signal
	if (running ==false){
	stopping = false;

		pthread_create(&pthread_server, NULL, soapserver, (void *) (&port));
		sleep(1);
		running = true;
		}else{
			stopping = true;
			printf ( "Stopping server\n");
			sleep(3);
			if (pthread_server)
				pthread_cancel(pthread_server);
			running =false	;
	}

}

