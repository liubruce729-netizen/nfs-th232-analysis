//--      FILE : GClientMemoryNarval.h 		  ---
//
// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/

#ifndef __GClientMemoryNarval__
#define __GClientMemoryNarval__

//variables d'environnements

#include <sys/time.h>

#include "acq_codes_erreur.h"

#include "GDevice.h"
#include "GSpectrumIdentity.h"

#include <unistd.h>     /* for close() */

//____________________________________________________________________________________________
class GClientMemoryNarval : public GDevice
{

 private:



  Int_t   fPremier_Buffer;	// number of the first received buffer
  Int_t	  fNb_Buffers_Envoyes;			// total number of sent buffers
  Int_t	  fNb_Buffers_Recus;			// actual number of received buffers
  Int_t	  fNb_Buffers_Local;

  Int_t   fLastNumBuff;


  Int_t 	 	fNumero;
  Int_t 	 	fTaille_Envoyee;
  Int_t 	 	fTaille_Max_Buffer;
  Char_t*		fBuffer_Envoye ;   //!
  Int_t 	 	fCode_Action_Retour;
  Int_t 	 	fTaille_Retour;
  Int_t 	 	fTaille_Max_Retour;
  Char_t*		fBuffer_Retour ;//!
  Int_t 	 	fComptbuffOK;
  Int_t 	 	fComptNotAvailable;

  Int_t  fSize_buffer_Narval;

  struct timeval fMt_meanbetw2buf;
  struct timeval fMt_reference;
  struct timezone fTz;
  Int_t           fCurrent_diff_time_buff;
  Int_t           fTempo;// duration  in second during the client stay open.
  bool            fNewOpen;
 private:
  char*           fZoneData ; //!
  int*            fZoneData_cur ; //!
  int             fSizeZoneData;
  int             fTimestampsbuffersize;
  int             fLastTimeNarvalbuf;
 public:
  GClientMemoryNarval (void);					// constructeur par defaut d'un objet GClientMemoryNarval associe a cette machine
  GClientMemoryNarval (const char* host);		        // constructeur par defaut d'un objet GClientMemoryNarval avec le nom de la machine hote � connecter
  ~GClientMemoryNarval ();
  virtual	void 	GClientMemoryNarvalInit (const char* host);			// initialisation des attributs : methode utilisee dans les 2 constructeurs

  /* --------- Gestion de la connection --------- */

  virtual	void 	GetBuffer (bool afficher = false);// recuperation d'un buffer de donnees brute
  // virtual	void 	GetSpectra(char* spectra_name);// Get all spectra  and creat a TTree with them
  virtual  void SetBufNarval(char* zone, int size );


 // These following functions are  abstract in GDevice
  virtual void Open(char mod = 'r') ;
  virtual void Open(char* mod);
  virtual void Close ();
  virtual void ReadBuffer ();						// recuperation d'un buffer de donnees brutes
  virtual void ReadBuffer (GBuffer& _Buffer);
  virtual void WriteBuffer(GBuffer* buffer ) {cout<<" This WriteBuffer do nothing\n";}
  //virtual void Rewind(bool quiet=false) {cout<<" This Rewind do nothing "<< quiet <<"\n"; }
  virtual void Rewind(bool quiet=false) {}
  virtual void Inquire(char* Exp_Name ) {cout<<" This Inquire do nothing " <<  Exp_Name << "\n";}
  virtual TNamed* GetSpectrum(const char* histoname=NULL,TNamed* old_sp=NULL){cout<<" This GetSpectrum do nothing\n";return NULL;};
  virtual TString* GetListSpectra(){cout<<" This GetListSpectra do nothing\n";return NULL;}; // get a Liste name of histograms
  ClassDef (GClientMemoryNarval,1); // Memory  client device for Narval (inherits from GDevice)
};

#endif
