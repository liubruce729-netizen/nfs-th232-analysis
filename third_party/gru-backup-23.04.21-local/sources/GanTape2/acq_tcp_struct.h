/*                                        
 * 	ACQ_TCP_STRUCT.h
 *
 *	David OPDEBECK -- Mai 1997
 *
 *      Fonction : Declaration des structures necessaires pour les messages
 *		d'entete de communication TCP/IP.
 *   
 *      Includes necessaires :
 *         gen_type.h
 *         acq_tcp_general
 *         acq_tcp_parametres_def ???????
 *         acq_tcp_status_struc  ???????
 *         
 *      Modifications :
 *        B. Raine 1 dec 98        gen_type.h
 *        B. Piquet 15-mars 1999   #pragma directive pou alignement des structures
 *        B. Raine 31 aout 99      quelques commentaires
 *        B. Raine 17 sept 99      suppression des includes internes
 *        L. Martina 23 Nov 00     ajout mot magique et version a la structure entete
 *				   spectre 
 *        B. Raine 11 dec 00       maj structure TABSTATUS
 */   
#include "acq_tcp_parametres_def.h"
#include "acq_tcp_status_struc.h"



#define _pipe 0
#ifndef __ACQ_TCP_STRUCT_H
#define __ACQ_TCP_STRUCT_H

#define TABENTETE element_swap SwapEntete[]={ {OCTET,4},{ENTIER32,7},{0,0} };
#define TABCMDASC element_swap SwapCmdAsc[]={ {ENTIER32,1},{0,0} };
#define TABTETSPEC element_swap SwapEntSpe[]={ {ENTIER32,1},{ENTIER32,1},{OCTET,24}, \
			{ENTIER32,1},{OCTET,48},{ENTIER32,4},{OCTET,4}, \
			{ENTIER32,6},{OCTET,128},{0,0} };
#define TABSTATUS element_swap SwapStatus[]={ {ENTIER32,33},{OCTET,16}, \
			{ENTIER32,4},{OCTET,12},{OCTET,4},{ENTIER32,1}, \
                        {OCTET,12},{OCTET,20},{0,0} };
					

typedef struct ELEMENT_SWAP
{
	INT32 Type;
	INT32 Nombre;

} element_swap;
    
/*
 *b. piquet  alignement des variables dans les structures communes...
 */
#ifdef __alpha
#pragma member_alignment save
#pragma nomember_alignment
#endif

typedef struct ACQ_MESS_ENTETE_STRUC  /* Structure d'entete pour le protocole */
{
   BYTE   mess_id[4];	/* Identificateur de message. 	*/
   INT32  rem_nid;	/* noeud emetteur.		*/
   INT32  rem_pid;	/* processus emetteur.		*/
   INT32  host_nid;	/* noeud destinataire.		*/
   INT32  host_pid;	/* processus destinataire.	*/
   INT32  code;		/* code action.			*/
   INT32  com_pid;	/* process communication.	*/
   INT32  l_corps;	/* longueur corps. (en mots de 32 bits - B. Raine) */

} acq_mess_entete_struc;


typedef struct ACQ_MESSAGE_STRUC  /* Ajout d'un buffer de messages a l'entete */
{
   acq_mess_entete_struc tete;
   BYTE corps[ACQ_MESSAGE_MAX_LENGTH];

} acq_message_struc; 



typedef struct ACQ_ERREUR_STRUC  /* structure d'erreur */
{
   BYTE	 routine[20];
   INT32 code;
   INT32 info;
   BYTE  infosup[80];

} acq_erreur_struc;



typedef struct ACQ_SESSION_STRUC
{
   INT32 node_pid;
   INT32 rem_pid;
   INT32 com_pid;
   INT32 channel;
   INT32 priv_level;

} acq_session_struc;



typedef struct ACQ_COM_MESSAGE_STRUC  /* Ajout du corps du message TCP */
{
   acq_message_struc mess;
   BYTE buffer[ACQ_MAXIMAL_BUFFER_SIZE];

} acq_com_message_struc;



typedef union MESSAGE_TCP  /* Definition d'une union pour utilise comme un buffer */ 
{
   acq_com_message_struc message;
   BYTE octets[sizeof(acq_com_message_struc)];

} message_tcp; 



typedef struct STRUCT_PIPE  /* Type de message utlise pour les comm inter-process */ 
{
   INT32 action;
   BYTE buffer[80];

} struct_pipe;


/*
typedef union MESSAGE_PIPE
{
   struct_pipe message;
   BYTE octets[sizeof(struct_pipe)];
} message_pipe;

*/

/*
 *   Une simple structure avec un entier et un buffer de caracteres.
 */

typedef union BUFCMDASCII
{
   struct UTILE
   {
      INT32 taille;
      BYTE buffer[ACQ_MAXIMAL_BUFFER_SIZE-sizeof(INT32)];
   } utile;
   BYTE buffer[ACQ_MAXIMAL_BUFFER_SIZE];

} buf_cmd_ascii;      



typedef union ACQ_TCP_STATUS_STRUC
{

   acq_status utile_stat;
   BYTE buf_stat[sizeof (acq_status)];
    
} acq_tcp_status_struc;


typedef struct TETE_SPEC /* Structure pour les entetes de spectres */
{
   INT32 num_run;		/* No de run 				*/
   INT32 magik;			/* mot magique de swapping 0xcafefade   */
   BYTE version[4];             /* numero de version = Vx.y             */
   BYTE reserve_1[4];		/* reserve 				*/
   BYTE nom_run[16];		/* nom du run 				*/
   INT32 num_spectre;		/* No de spectre 			*/
   BYTE type_spectre[2];	/* type de spectre ( 1D ou 2D ) 	*/
   BYTE reserve_2[10];		/* reserve 				*/
   BYTE nom_spectre[16];	/* nom du spectre ( 15 carac. utilises )*/
   BYTE date[12];		/* date de creation ("jj-mmm-aaaa"->11 car ut)*/
   BYTE heure[8];		/* heure de creation ("hh:mn:ss" -> 8 car ut )*/ 
   INT32 nat_spec;		/* reserve ulterieure 			*/
   INT32 codeur_x;		/* valeur max. du codeur en X 		*/
   INT32 codeur_y;		/* valeur max. du codeur en Y 		*/
   INT32 taille_canal;		/* taille utile par canal ( 16 ou 32 ) 	*/
   BYTE type_canal[4];		/* type variable ( "I*4" ou "I*2" ) 	*/
   INT32 dim_x;			/* dimension du spectre en X 		*/
   INT32 dim_y;			/* dimension du spectre en Y 		*/
   INT32 min_x;			/* No du canal min en X 		*/
   INT32 min_y;			/* No du canal min en Y 		*/
   INT32 max_x;			/* No du canal max en X 		*/
   INT32 max_y;			/* No du canal max en Y 		*/
   BYTE commentaire[80];	/* commentaire utilisateur 		*/
   BYTE nom_par_x[16];		/* nom du param X ( 15 carac. utiles ) 	*/	
   BYTE nom_par_y[16];  	/* nom du param Y ( ---------------- ) 	*/
   BYTE unite_x[8];		/* unite en X ( 8 caract util ) 	*/
   BYTE unite_y[8];		/* unite en Y ( ------------- ) 	*/

} tete_spec;


typedef union BUF_ENTSPEC
{
   tete_spec entete;
   BYTE buf_ent[sizeof(tete_spec)];

} buf_entspec;


typedef struct ENTETEASCII
{
   char Commande[20];
   char Service[20];
   char Securite[20];
   char Corps[196];

} entete_ascii;


typedef union BUFENTETEASCII
{
   entete_ascii Entete;
   char Buffer[sizeof(entete_ascii)];
   
} buf_entete_ascii;

/*
 *b. piquet  alignement des variables dans les structures communes...
 */
#ifdef __alpha
#pragma member_alignment restore
#endif


#endif         
