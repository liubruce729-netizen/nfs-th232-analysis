//--      FILE : GNetClientNarval.h 		  --
//--    AUTHOR : Luc Legeard    --

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#ifndef __GNetClientNarval__
#define __GNetClientNarval__

//variables d'environnements

#include <sys/time.h>

#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <netdb.h>
#include "acq_codes_erreur.h"
#include "GDevice.h"
#include "GBufferIn2p3.h"
#include "GBufferMFM.h"
#include "GSpectrumIdentity.h"

#include <unistd.h>     /* for close() */

//____________________________________________________________________________________________
class GNetClientNarval : public GDevice
{

 private:

  Int_t	  fSocket;				// socket number
  char*   fPortChar ;			//! port number
  Int_t   fPremier_Buffer;	// number of the first received buffer
  Int_t	  fNb_Buffers_Envoyes;			// total number of sent buffers
  Int_t	  fNb_Buffers_Recus;			// actual number of received buffers
  Int_t	  fNb_Buffers_Local;
  Int_t   fLastNumBuff;
  Int_t   fSize_narval_buffer; //
  Int_t   fBufferSizeIn2p3;// fixed size in case of in2p3buffer
  bool    fNarvalBufferEndReached;
  GBufferIn2p3 * fBufferIn2p3;
  GBufferMFM   * fBufferMFM;
 

  struct sockaddr_in fServAddr; /* server address */
  /* quelques attributs utilises regulierement dans les methodes.
     Ces variables sont declarees en attributs de la classe uniquement par souci de proprete, pour eviter de les declarer a chaque debut de methode
     dans toutes les methodes ou ces attributs sont utilises, ils servent principalement au fonctions TCP/IP de la librairie 'acq_tcp_lib_client.a'. */

  Int_t 	 		fNumero;
  Int_t 	 		fTaille_Envoyee;
  Int_t 	 		fTaille_Max_Buffer;
  Int_t 	 		fCode_Action_Retour;
  Int_t 	 		fTaille_Retour;
  Int_t 	 		fTaille_Max_Retour;

  Int_t 	 		fComptbuffOK;
  Int_t 	 		fComptNotAvailable;


  struct timeval fMt_permitclose;
  struct timeval fMt_meanbetw2buf;
  struct timeval fMt_reference;
  struct timezone fTz;
Int_t fCurrent_diff_time_buff;
  Int_t           fTempo;// duration  in second during the client stay open.
  bool            fNewOpen;


 private:
  char*                  fZoneData_char ;     //! pointer on the begin Narval buffer
  char*                  fZoneData_cur_char ; //! pointer on Narval buffer
  int*                   fZoneData_cur_int ;  //!pointer on Narval buffer
  int                    fSizeZoneData;
  int                    fTimestampsbuffersize;
  int                    fLastTimeNarvalbuf;
 public:
  GNetClientNarval (void);				// constructeur par defaut d'un objet GNetClientNarval associe a cette machine
  GNetClientNarval (const char* host);		        // constructeur par defaut d'un objet GNetClientNarval avec le nom de la machine hote � connecter
  ~GNetClientNarval ();
  virtual	void 	GNetClientNarvalInit (const char* host);	// initialisation des attributs : methode utilisee dans les 2 constructeurs

  /* --------- Gestion de la connection --------- */
  virtual   void    SetDevice (const char* _host);				// change le nom de la machine hote
  virtual   void    SetPort (Int_t port);                   // change port number

  virtual   bool    GetBufferNarval (bool afficher = false);// recuperation d'un buffer de donnees brute
  virtual   void    GetBuffer (bool afficher = false);// recuperation d'un buffer de donnees brute
  virtual   void    SetfTimeStampsSize(Int_t timesp){fTimestampsbuffersize =timesp;}
  // virtual	void 	GetSpectra(char* spectra_name);// Get all spectra  and creat a TTree with them
  virtual void MyClose(Int_t tempo =0 );
  virtual void Close(Int_t tempo);

 // These following functions are  abstract in GDevice
  virtual void Open(char mod = 'r') ;
  virtual void Open(char* mod);
  virtual void Close ();
  virtual void ReadBuffer ();                // recuperation d'un buffer de donnees brutes
  virtual int  ReadAll(int sockfd, char*buff, int sizeToRead);
  //virtual void Rewind(bool quiet=false) {cout<<" This Rewind do nothing "<< quiet <<"\n"; }
  virtual void Rewind(bool quiet=false) { }
  virtual void Inquire(char* Exp_Name ) {cout<<" This Inquire do nothing " <<  Exp_Name << "\n";}
  virtual TNamed* GetSpectrum(const char* histoname=NULL,TNamed* old_sp=NULL){cout<<" This GetSpectrum do nothing\n";return NULL;};
  virtual TString* GetListSpectra(){cout<<" This GetListSpectra do nothing\n";return NULL;}; // get a Liste name of histograms
  virtual void WriteBuffer(GBuffer* buffer ) {cout<<" This WriteBuffer do nothing\n";}
  ClassDef (GNetClientNarval, 1); // Network client device (inherits from GDevice)
};

#endif
