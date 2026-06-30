/************************************************************************/
/*                                                                      */
/* Author : Bruno Raine                                                 */
/*                                                                      */
/* Creation date : 1er octobre 2010                                     */
/*                                                                      */
/* Modifications :
      2 septembre 2014 - F. Saillant - recv_timeout=15
      9 janvier 2015   - B. Raine - add arguments to getinfo
                                    improve help command
*/
/*                                                                      */
/************************************************************************/
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include "GSoapErrorCode.h"
#include "SCSoap.nsmap"
#include "SCSoapH.h"



using namespace std;
Namespace namespaces[100];
struct soap v_soap;
static void store_frame(void * frame, int framesize, string filename);

int main ( int argc, char ** argv )
{
  string host;
  int i, size;
  	unsigned char j = 0;
  	int port = 6603;
  	char destination[200];
  	char *command= NULL;
  	strcpy(destination, "localhost");
  	bool soapsend = false;
  	char *word;
  	char destination_port[256];
  	int commanddone = 0;
  	int number_of_word_not_include_in_message;
  	number_of_word_not_include_in_message = 0;

  	cout << setiosflags(ios::unitbuf);

  char *env_host = getenv("SC_HOST");
  char *env_port = getenv("SC_PORT");

  if (argc < 2) {
	  cout <<" SCC client for slow control, get trace from numexo2 card and cread a MFM file.\n";
	  cout <<" Usage :   SCC host:port ";
	  return (0);

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

  soap_init(&v_soap);

  soap_set_namespaces(&v_soap, SCSoap_namespaces);
  xsd__base64Binary_test response;

  soap_call_sc__ReadScope(&v_soap,destination_port,"", response);

 // soap_call___ns1__ReadScope(&v_soap,destination_port,"", response);
 // soap_call___ns1__getScaler(&p_soap, destination_port,
  	// 	"", &_ns1__getScaler,
  	  //	&_ns1__getScalerResponse);
  if (v_soap.error) {
  			soap_print_fault(&v_soap, stderr);
  			soap_end(&v_soap);
  			return(1);
  		}
  if (!v_soap.error)
    {
      if ( response.__size == 0 )
	cout << "returned size = 0 bytes " << endl;
      else if (*response.__ptr == '<')
	cout << "ERROR : " << response.__ptr;
      else
	{
	  cout << "returned size = " <<  response.__size << " bytes " << endl;
	  //dump((char *)response.__ptr, 64);
	  store_frame(response.__ptr,  response.__size, "scope_frame.dat");
    }
      soapsend=true;
}
  if (soapsend)
  soap_end(&v_soap);
}

void store_frame(void * frame, int framesize, string filename)
{
  ofstream file_scope(filename.c_str(), ios::out);
  int fLun; // Logical Unit Number
  fLun = open(filename.c_str(), (O_RDWR | O_CREAT | O_TRUNC), 0644);
  if ( fLun == -1 )
    perror("filename");
  else
    {
      cout << "write " << framesize << " bytes to " << filename << endl;
      int verif = write(fLun, frame, framesize);
      if (verif != framesize)
	perror(filename.c_str());
    }
}
