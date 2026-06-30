/*
 * 	ACQ_TCP_FCT_UTILISATEUR.h
 *
 *	David OPDEBECK  --  Mars 1998
 *
 *	Fonction : Prototype et definitions pour les fonctions remote
 *		 appelables depuis un programme utilisateur.
 * 
 *        Includes necessaires : 
 *               gen_type.h
 *               acq_tcp_struct.h 
 *               gan_acq_buf.h
 *
 *      Modifications :
 *         B. Raine le 17 sept 99
 *             Suppression des includes internes
 *         B. Raine le 25 fevrier 2000
 *             Ajout de acq_tcp-read_disk_spectra et acq_tcp_remote_directory
 */    

#include "acq_tcp_struct.h"
#include "acq_tcp_general.h"
#include "gan_acq_buf.h"

#ifndef __ACQ_TCP_FCT_UTILISATEUR_H
#define __ACQ_TCP_FCT_UTILISATEUR_H

#ifdef __cplusplus
extern "C" {
#endif

/*
#include "acq_tcp_parametres_def.h"
*/

/*	int acq_tcp_send_command ( char *cmd_env, char *cmd_ret, int *length ) 
 *
 *	  Cette fonction permet d'envoyer des commandes au serveur de commandes
 *	de l'acquisition par l'intermediaire du reseau sous TCP/IP sous forme
 *	de tableau de caracteres ASCII.
 *
 *	Parametres :
 *	   Entree  :  cmd_env -- commande envoyee au serveur de 256 octets max.
 *
 *	   Sortie :   cmd_ret -- commande retournee par le serveur de 256 oct. max.
 *		      length -- longueur utile en retour.  
 *		  Codes retournes : 0 si tout s'est bien passe.
 *				   -1 s'il y a eu un probleme de connection 
 *				( possibilite de recuperer le status acquisition
 *				et le texte de l'erreur correspondante en appelant 
 *				acq_tcp_get_messerr () ). 
 */
int acq_tcp_send_command ( char *, char *, int * );
  char * sendCmd(char *);


/*	int acq_tcp_get_acquis_status ( acq_tcp_status_struc *buffer ) 
 *
 *	  Cette fonction permet d'obtenir le status de l'acquisition.
 *
 *	Parametres :
 *	   Entree  :  neant.
 *
 *	   Sortie  :  buffer -- adresse de la structure status retournee
 *			( voir description dans acq_tcp_struc.h et dans 
 *			  acq_tcp_status_struct.h ).
 *		  Codes retournes : 0 si tout s'est bien passe.
 *				   -1 s'il y a eu un probleme de connection.
 *				( possibilite de recuperer le status acquisition
 *				et le texte de l'erreur correspondante en appelant 
 *				acq_tcp_get_messerr () ). 
 *
 */
int acq_tcp_get_acquis_status ( acq_tcp_status_struc * ); 



/*	int acq_tcp_get_spec_info (char *name, int *number, buf_entspec *entete,
 *			char *buffer, int max_size, int *len_rendu  )  
 *
 *	  Cette fonction permet d'obtenir l'entete d'un spectre ainsi que sa
 *	table de valeurs.
 *
 *	Parametres :
 *	   Entree  :    name -- nom du spectre, utilise si numbrer=0.
 *			number -- no de spectre, si 0 alors utilisation du nom
 *				et mise a jour du no en retour.
 *			max_size -- taille de buffer.
 *			len_rendu -- taille en octets du spectre.
 *
 *	   Sortie  :    entete -- adresse de la structure entete retournee
 *			( voir description dans acq_tcp_struct.h ).
 *			buffer -- buffer de retour des donnees spectre.
 *			len_rendu -- taille en octets du spectre.
 *		  Codes retournes : 0 si tout s'est bien passe.
 *				   -1 s'il y a eu un probleme de connection.
 *				   -2 si le buffer est trop court.
 *				   -3 si spectre non demare ou inexistant.
 *				   -4 autre erreur...
 *				 ( possibilite de recuperer le status acquisition
 *				et le texte de l'erreur correspondante en appelant 
 *				acq_tcp_get_messerr () ).  
 *
 *  Aout 1998 - Si le buffer devant recevoir les donnees est trop petit alors
 *	 	la fonction renvoie -2 et la taille desiree dans len_rendu.   
 * 
 */
int acq_tcp_get_spec_info (char *, int *, buf_entspec *, char *, int, int *);   



/*  int acq_tcp_read_disk_spectra ( char *name, buf_entspec *header, char *buffer,
                                    int max_size, int *got_size )
C-----auteur,date : Bruno Raine.......24 fevrier 2000
C                   a partir de acq_read_disk_spectra.for, pour lire en remote
C                   les spectres sur disque sous VMS
C
C-----modifications:
C
C
C-----description : donne l'entete d'un spectre, et sa table de valeurs
C
C
C-----arguments   : 
         entree :
C            - name        nom du spectre
C            - max_size    integer*4 taille de ce buffer en bytes
         sortie :
C            - header      record de 256 bytes entete du spectre
C            - buffer      buffer de reception des donnees
C            - got_size    integer*4 taille en bytes du spectre (rendu)
C
C-----appel       : status = acq_tcp_read_disk_spectra(name,header,buffer,max_size,&got_size)
C
C-----retour      : 0   <==> ok entete et table remplies
C                   -1  <==> probleme de connexion (acquisition non lancee)
C                   -2  <==> buffer trop court
C                   -3  <==> spectre inexistant
C                   -4  <==> autre erreur...
                    -100 ==> erreur de connexion TCP
 *				 ( possibilite de recuperer le status acquisition
 *				et le texte de l'erreur correspondante en appelant 
 *				acq_tcp_get_messerr () ).   
C

*/

int acq_tcp_read_disk_spectra( char *name, 
			       buf_entspec *header, char *buffer,
			       int max_size, int *got_size );



/*      int function acq_tcp_remote_directory(char *def_dir,char *filespec,
                                             int option,char *result)
					     
 -----auteur,date : Bruno Raine.......25 fevrier 2000
                    pour obtenir en remote un directory VMS
 
 -----modifications:
 
 -----bibliotheque: acquislib
 
 -----description : donne la liste des fichiers correspondants a une
                    specification donnee
                    A chaque appel, on retourne un nom de fichier, donc
		    pour obtenir un directory complet, il faut boucler
		    jusqu'a obtenir un status en retour = 0
 
 
 -----arguments   : 
        entree :
            - def_dir     default specification (i.e. dev:[dir])
            - filespec    file spec to search (nom*.ext)
            - option              0 abort operation en cours (reinitialisation)
                                    ==> pas de resultat, et retour = 0.
                                  1 result = default+filename+ext+version
				  2 result = filename+ext+version
				  3 result = filename+ext
				  4 result = filename
        sortie :
            - result      if ok, contains the resultant filename
 
 
 
 -----appel       : while (status == 1){
                       status = acq_remote_directory(default,filespec,
                                              option,result,length)
					      printf ("%s" ,result
		    }
                    if (status == 0) then
                       fini ok
                    else
                       fini erreur
                    end if
 

 -----retour      : 1   <==> ok et fichier dans resultat
                    0   <==> ok et fini
                    -1  <==> probleme de connexion (acquisition non lancee)
                    -2   ==> le serveur ne peut pas acceder au directory
		             pour privilege insuffisant par exemple
                    -4  <==> autre erreur...
                    -100 ==> erreur de connexion TCP
 				 ( possibilite de recuperer le status acquisition
 				et le texte de l'erreur correspondante en appelant 
 				acq_tcp_get_messerr () ).   

*/
  int acq_tcp_remote_directory(char *dir, char *filespec,
					int option, char *result);



/*	int acq_tcp_get_ctrl_buffer ( int *buf_num, in2p3_buffer_struct *buffer, 
 *			int max_size, int *len_rendu )  
 *
 *	  Cette fonction permet d'obtenir un buffer de type controle en cours.
 *
 *	Parametres :
 *	   Entree  :    buf_num -- no du dernier buffer recu, 0 lors du premier
 *				appel et a refournir ensuite a chaque fois.
 *			max_size -- taille en octets de buffer.
 *
 *	   Sortie  :    buffer -- adresse du buffer de type structure 
 *			   in2p3_buffer_struct ( voir description dans gan_acq_buf.h ). 
 *			len_rendu -- taille en octets du buffer retourne.
 *		  Codes retournes : 0 si tout s'est bien passe.
 *				   -1 s'il y a eu un probleme de connection.
 *				   -2 si le buffer est trop court.
 *				   -3 si pb de copie de buffer.
 *				   -4 si operation interdite.
 *				   -5 si erreur dans TcpRead.
 *				    1 pas de buffer en cours.
 *				    2 buffer en cours deja lu ( recommencer ).
 *				    3 pas de buffer disponible ( recommencer ).
 *				 ( possibilite de recuperer le status acquisition
 *				et le texte de l'erreur correspondante en appelant 
 *				acq_tcp_get_messerr () ).   
 *
 */


int acq_tcp_get_ctrl_buffer ( int *, in2p3_buffer_struct *, int, int * );




/*	int acq_tcp_get_scale_buffer ( int *buf_num, in2p3_buffer_struct *buffer, 
 *			int max_size, int *len_rendu )   
 *
 *	  Cette fonction permet d'obtenir un buffer de type echelle en cours.
 *
 *	Parametres :
 *	   Entree  :    buf_num -- no du dernier buffer recu, 0 lors du premier
 *				appel et a refournir ensuite a chaque fois.
 *			max_size -- taille en octets de buffer.
 *
 *	   Sortie  :    buffer -- adresse du buffer de type structure 
 *			   in2p3_buffer_struct ( voir description dans gan_acq_buf.h ). 
 *			len_rendu -- taille en octets du buffer retourne.
 *		  Codes retournes : 0 si tout s'est bien passe.
 *				   -1 s'il y a eu un probleme de connection. 
 *				   -2 si le buffer est trop court.
 *				   -3 si pb de copie de buffer.
 *				    1 pas de buffer en cours.
 *				    2 buffer en cours deja lu ( recommencer ).
 *				    3 pas de buffer disponible ( recommencer ).
 *				 ( possibilite de recuperer le status acquisition
 *				et le texte de l'erreur correspondante en appelant 
 *				acq_tcp_get_messerr () ).   
 *			
 */
int acq_tcp_get_scale_buffer ( int *, in2p3_buffer_struct *, int, int * );   


/* Lebertre G le 16.10.02 pour utilisation JNI */
  char* acq_get_bufScale();
  int acq_get_Erreur();

  int etatAcq();
  int etatControle();
  int etatStorage();
  int nbMaxBuffer();
  int nbBuffer();
  int nbBufferLu();
  int nbBufferControle();

/*	int acq_tcp_get_jbus_buffer ( int *buf_num, in2p3_buffer_struct *buffer, 
 *			int max_size, int *len_rendu )   
 *
 *	  Cette fonction permet d'obtenir un buffer de type jbus en cours.
 *
 *	Parametres :
 *	   Entree  :    buf_num -- no du dernier buffer recu, 0 lors du premier
 *				appel et a refournir ensuite a chaque fois.
 *			max_size -- taille en octets de buffer.
 *
 *	   Sortie  :    buffer -- adresse du buffer de type structure 
 *			   in2p3_buffer_struct ( voir description dans gan_acq_buf.h ). 
 *			len_rendu -- taille en octets du buffer retourne.
 *		  Codes retournes : 0 si tout s'est bien passe.
 *				   -1 s'il y a eu un probleme de connection. 
 *				   -2 si le buffer est trop court.
 *				   -3 si pb de copie de buffer.
 *				    1 pas de buffer en cours.
 *				    2 buffer en cours deja lu ( recommencer ).
 *				    3 pas de buffer disponible ( recommencer ).
 *				 ( possibilite de recuperer le status acquisition
 *				et le texte de l'erreur correspondante en appelant 
 *				acq_tcp_get_messerr () ).   
 *			
 */
int acq_tcp_get_jbus_buffer ( int *, in2p3_buffer_struct *, int, int * );   





/*	int acq_tcp_close_connection ( void ) 
 *
 *	  Cette fonction permet de deconnecter le client du serveur
 *
 *	Parametres :
 *	   Entree  :  neant.
 *		
 *	   Sortie  :  neant.   
 *		  Codes retournes : 0 si tout s'est bien passe.
 *				   -1 s'il y a eu un probleme de fermeture.
 *				 ( possibilite de recuperer le status acquisition
 *				et le texte de l'erreur correspondante en appelant 
 *				acq_tcp_get_messerr () ).   
 */     
int acq_tcp_close_connection ( void );




/*	void acq_tcp_get_messerr ( int *Status, char *Buffer )
 *		Fonction permettant de recuperer le texte et le status de la
 *	     derniere erreur d'un fonction utilisateur correspondant a un status 
 *	     sur ACQ_COMMUNICATION. Attention, ce message d'erreur ne sera plus
 *	     valable si un autre fct utilisateur est appelee sans erreur.
 *
 *	Parametres :
 *		Entree : neant.
 *
 *	   	Sortie  : Status -- code de l'erreur demandee.
 *                        Buffer -- adresse de la chaine retournee.
 *		Pas de code retourne.
 */
void acq_tcp_get_messerr ( int *, char * );    



/*	int acq_tcp_send_code_action ( int command, char *BufEnv, int TailleEnv,
 *				char *BufRet, int *TailleRec, typebuffer TypEnv,
				typebuffer TypRet ) 
 *
 *	  Cette fonction permet d'envoyer des commandes au serveur de commandes
 *	de l'acquisition sous forme de code commande directement interprete par
 *	le serveur. Il est ainsi possible d'acceder a toutes les commandes du
 *	serveur.
 *
 *	Parametres :
 *	   Entree  :  commande -- commande envoyee au serveur.
 *		      *BufEnv -- Buffer a envoyer sous forme d'octets.
 *		      TailleEnv -- Taille du buffer a envoyer.
 *		      TypEnv -- Type de structure envoyee ( voir acq_tcp_general.h )
 *		      TypRet -- Type de structure a recevoir.
 *			Ces deux derniers elements sont utilises pour le swap.
 *
 *	   Sortie :   *Bufret -- Buffer recu.
 *		      TailleRec -- longueur utile en retour.  
 *		  Codes retournes : 0 si tout s'est bien passe.
 *				   -1 s'il y a eu un probleme de connection 
 *				( possibilite de recuperer le status acquisition
 *				et le texte de l'erreur correspondante en appelant 
 *				acq_tcp_get_messerr () ).
 *				   -2 si le buffer est trop cours. 
 */
int acq_tcp_send_code_action ( int, char *, int, char *, int *, 
				typebuffer,typebuffer );    

#ifdef __cplusplus
	   }
#endif

#endif
