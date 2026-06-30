/*****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  ACQ_MT_FCT_GANIL.c                                  *
 *                                                                           *
 *       Auteur       :  OPDEBECK                                            *
 *       Date         :  16 juillet 1996                                     *
 *                      				                     *
 *                                                                           *
 *       Objet        :  Source des differentes fonctions de traitement et   *
 *			de relecture de buffer fournies par le GANIL pour    *
 *                      les environnements VMS et UNIX.                      * 
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "gan_tape_file.h" 
//#include "gan_tape_general.h"

#include "acq_mt_fct_ganil.h" /* Prototypes bibliotheque EXEMPLE_RELECTURE */

//#include "ERR_GAN.H" /* Pour definir le code retour de rd_evstr.c	*/
//#include "gan_tape_erreur.h"	/* Fichier d'erreurs	*/

#if defined ( __VMS ) || defined ( VMS ) /*******************************/

#include <libdef.h>
#include <lib$routines.h>
#include <ssdef.h>

#include <unixio.h>
#include <file.h>	/* Pour la gestion du fichier temporaire.	*/


#elif defined ( __unix__ ) || (__unix)  || (MACOSX) /***********************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


#else 	/****************************************************************/
#error "Ce package n'est pas compatible avec votre OS"
#endif /*****************************************************************/


#define FICHIER_STR "evt_str.tmp"

/*****************************************************************************
 *                                                                           *
 *           Function  : acq_mt_ini_run_c                                    *
 *           Objet     : Initialisation du contexte de relecture d'une bande.*
 *                       !! La bande en question doit etre allouee, montee   *
 *                       et ouverte.                                         *
 *                                                                           *
 *           Entree    : Lun  -- Numero d'unite logique du fichier ouvert.   *  
 *                       Buff -- Buffer de lecture des donnees.              *
 *                       Size -- Taille du buffer de lecture.                *
 *                       RunNumber -- Numero de run attendu. Il s'agit de    *
 *                         l'adrresse de la variable afin de retourner le    *
 *                         no de run effectivement lu.                       *
 *                       StrEvent[] -- Buffer de mise en memoire.            *
 *                       StrEventSize -- Taille du buffer precedent.         *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande.          *
 *			 .. Ces codes peuvent changer de denomination ..     * 
 *                           - ACQ_OK si aucun probleme.                     *
 *                           - ACQ_NOBEGINRUN si la bande n'est pas en debut *
 *                                            de run.                        *
 *                           - ACQ_BADFILESTRUCT si mauvaise structure de    *
 *                                            device.                        *
 *                           - ACQ_BADEVTSTRUCT si mauvaise structure d'evt. *
 *                           - ACQ_BADRUNNUM si mauvais numero de run.       *
 *                           - ACQ_STREVTTMP si problem d'acces au fichier   *
 *                                   temporaire de structure evenement       *
 *                           - si autre erreur. ( contenue dans errno )      *
 *                                                                           *
 ****************************************************************************/ 

// 2012 legeard ajout de int *ReadEvtSize,char **pointevnt  pour avoir un retour sur la taille de l'evenement et de son pointeur de lecture.



/*****************************************************************************
 *                                                                           *
 *           Function  : get_next_event                                      *
 *           Objet     : Permet d'obtenir l'evenement suivant dans un buffer.*
 *                                                                           *
 *           Entree    : Buff -- Buffer de lecture des donnees.              *
 *                       Size -- Taille du buffer de lecture.                *
 *                       *Event -- Buffer d'evenement.                       *
 *                       SizeEvent -- Taille du buffer precedent.            *
 *                       NumeroEvent -- Numero d'evenement.                  *
 *			plus ensemble de variables globales utilisees pour   *
 *			les boucles.                                         *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande.          *
 *			 .. Ces codes peuvent changer de denomination ..     * 
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_INVDATABUF si pas de buffer correct.   *
 *                              - ACQ_RAFBUFOVF si debordement.              *
 *                              - ACQ_ENDOFBUFFER si plus d'evenement.       *
 *                                                                           *
 *****************************************************************************/ 



int get_next_event (in2p3_buffer_struct *Buff, int Size,
		short int *Event , int SizeEvent , int *NumeroEvent, int *ReadEvtSize,char **pointevnt) {

  /* Variables reutilisees a chaque appel en reutilisant leur valeur 
     courante => on les declare en static */
  static int Index,Pos,EvtTaille;
  
  int Status, Boucle, SizeTempo;

  Status = ACQ_OK;

  /*  deja fait dans la procedure appelante
   * char Header[9];
  strncpy(Header, Buff->les_donnees.Ident, 8);
  Header[8]='\0';
  
  if ( strcmp ( Header, EVENTDB_Id ) != 0 && 
       strcmp ( Header, EVENTCT_Id ) != 0 )
    Status = ACQ_INVDATABUF;
  else
	  Status = ACQ_OK;
	  */
  
  if ( Status == ACQ_OK ) {
    if ( *NumeroEvent == -1 ) {
      Pos = 0;
      Index = 0;
    }
    EvtTaille = Buff->les_donnees.cas.Buff[Pos];
    *pointevnt = (char*)(&(Buff->les_donnees.cas.Buff[Pos]));
    *ReadEvtSize =EvtTaille;
    if ( EvtTaille != 0 ) {
      /* On prend le plus petit des deux valeurs :EvtTaille,SizeEvent. */
      SizeTempo = EvtTaille < SizeEvent ? EvtTaille : SizeEvent; 
      for ( Boucle = 0; Boucle < SizeTempo; Boucle++ )
	Event[Boucle]=Buff->les_donnees.cas.Buff[Pos+Boucle]; 	
      Index++;
      Pos = Pos + EvtTaille;
      if ( Pos > ( Size - 36 )/2 )
	Status = ACQ_RAFBUFOVF;
      else {
	Status = ACQ_OK;
	*NumeroEvent = Event[1];
      }
    }
    else {
      Status = ACQ_ENDOFBUFFER;
      *NumeroEvent = -1;
    }
    return ( Status );
    
    
  }
  return ( Status );
}
	

/*****************************************************************************
 *                                                                           *
 *           Function  : get_next_param                                      *
 *           Objet     : Permet d'obtenir le parametre suivant dans un buffer*
 *                                                                           *
 *           Entree    : AddrParam -- Adresse du param courant dans buffer.  *
 *                       Ligne -- Ligne de 80 caracteres a formater.         *
 *                       Size -- Longueur utile.                             *
 *                       Addr -- Adresse courante en retour, incrementee.    *
 *                       Suite -- Drapeau de suite ( 1=Suite, 0 sinon ).     *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande.          *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_ENDOFBUFFER si buffer plein.           *
 *                                                                           *
 *****************************************************************************/ 
int get_next_param ( int AddrParam,char *Ligne,int Size,int Addr,int Retour) {

	

/* A completer ................... */

	return ( ACQ_OK );


}

/*****************************************************************************
 *                                                                           *
 *           Function  : libere_fichier                                      *
 *           Objet     : Permet de fermer un fichier et de liberer le LUN.   *
 *                                                                           *
 *           Entree    : Le_fichier -- Lun du fichier a clore.               *
 *                                                                           *
 *           Retour    : neant.                                              *
 *                                                                           *
 ****************************************************************************/

void libere_fichier ( int Le_fichier ) {

//	int Etat;
//	Etat =
	close ( Le_fichier );

}




/********************************* FIN ***************************************/
