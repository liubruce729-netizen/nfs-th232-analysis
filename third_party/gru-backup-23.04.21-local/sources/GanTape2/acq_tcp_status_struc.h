/*
 * 	ACQ_TCP_STATUS_STRUC.h
 *
 *	David OPDEBECK -- Mars 1998
 *
 *      Fonction : Declaration de la structure du status d'acquisition.
 *   
 *      Modifications :
 *        B. Raine 1 dec 98   gen_type.h
 *        B. Piquet 16 mars 1999 pragma alignment for decunix
 *        B. Raine 11 dec 2000   Utilisation des derniers mots de reserve
 */   

#ifndef __ACQ_TCP_STATUS_STRUCT_H
#define __ACQ_TCP_STATUS_STRUCT_H

#include "GEN_TYPE.H"     
/*
 *b. piquet  alignement des variables dans les structures communes...
 */
#ifdef __alpha
#pragma member_alignment save
#pragma nomember_alignment
#endif

typedef struct ACQ_STATUS /* Structure de retour pour un appel get_status */
{

   INT32 on;			/* acquisition on/off 			*/
   INT32 control;		/* control on/off 			*/
   INT32 store;			/* storage on/off 			*/
   INT32 fullcontrol;		/* full control on/off 			*/

   INT32 usercontrol;		/* user control on/off 			*/
   INT32 replay;		/* replay on/off 			*/
   INT32 simul;			/* simulation on/off 			*/
   INT32 debug;			/* mode debug on/off 			*/

   INT32 norun;			/* run courrant 			*/
   INT32 nbrdbuf;		/* nombre total de buffers ( depuis VME ) */
   INT32 nbrdctbuf;		/* nombre de "CONTROL" buffers ( depuis VME ) */
   INT32 nbrdstbuf;		/* nombre de "STORE" buffers ( depuis VME ) */

   INT32 nbmaxbuf_tape;		/* nombre maxi de buffers / bande 	*/
   INT32 nbbuf_tape;		/* nombre de buffers sur la bande actuelle */
   INT32 nbstbuf;		/* nombre de buffers reellement sauvegardes */
   INT32 nbrstevent;		/* nombre d'evenements reellement sauvegardes */

   INT32 nbctbuf;		/* nombre de buffers reellement controles */
   INT32 nbctevent;		/* nombre d'evenement reellement controles */
   INT32 nbevent;		/* nb total d'evenements ( ctrl buffers ) */
   INT32 nbrdevent;		/* nb total d'evenements ( store buffers ) */

   INT32 replay_status;		/* replay status ( ACQ_ON,ACQ_OFF,erreur,..) */
   INT32 acq_buffer_size;	/* taille des buffers d'acquisition 	*/
   INT32 mode_incr_spectres;	/* mode incr des spectres (INDRA ou GANIL ) */
   INT32 mode_spec_alr_incr;	/* mode action quand demarage d'un spectre deja
				   en incrementation 			*/
   INT32 nb_spectr_incr;	/* nombre de spectres incrementes 	*/
   INT32 nb_spectr_susp;	/* nombre de spectres suspendus 	*/
   INT32 avail_memory;		/* memoire disponible 			*/
   INT32 nbrdscbuf;		/* nombre de buffers "SCALE" ( depuis VMS ) */

   INT32 enabledispatch;	/* drapeau d'autorisation de "dispatch" */ 
   INT32 enablefreectrl;	/* drapeau d'autorisation de liberation
					anticipee d'un buffer de controle */
   INT32 nbctrlinfifo;		/* profondeur limite de la fifo controle */
   INT32 nbctrlindispatch;	/* profondeur limite de la fifo dispatch */

   INT32 tape_ready;		/* "true" si bande ou fichier sont dispo. */
   BYTE tape_name[16];		/* nom correspondant 			*/

   INT32 verify_mode;		/* mode de verification 		*/
   INT32 rec_buf_mode;		/* mode de reception des buffers 	*/
   INT32 data_link_type;	/* type de data link 			*/
   INT32 nb_spec_dispo;		/* nombre de spectres declares 		*/

   BYTE target_vme_name[12];	/* nom de la cible VME 			*/

  BYTE prc_acquis_status_ok;   /* etat du process acquisition */
  BYTE prc_read_status_ok;     /* etat du process acq_read_data */
  BYTE prc_ctrl_status_ok;     /* etat du process acq_ctrl_data */
  BYTE prc_store_status_ok;    /* etat du process acq_store_data */

  INT32    acq_data_rate;       /* vitesse d'acquisition (kb/s) */

  BYTE experiment_name[12];     /* nom de l'experience en cours */
  BYTE installation_time[20];   /* date d'installation */

   /* INT32 acq_status_reserve[10]; 31->30 lors de l'ajout du mode.
				 * 30->27 ajout nb spec et memoire dispo.
				 * 27->26 nombre de buffers echelle.
				 * 26->22 dispatch et fifo.
				 * 22->17 nom de bande.
				 * 17->15 drapeaux verif et type.
				 * 15->14 datalinktype & nbspecdispo.
				 * 14->11 vme crate.	
				 */

   INT32 end_of_buffer; 	/* Total de 190 octets 			*/
/*
 *b. piquet  alignement des variables dans les structures communes...
 */
#ifdef __alpha
#pragma member_alignment restore
#endif

} acq_status;    

#endif

