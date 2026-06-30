/*
 * 	ACQ_TCP_PARAMETRES_DEF.h
 *
 *	David OPDEBECK -- Mai 1997
 *
 *      Fonction : Declaration de certains parametres necessaires pour
 *		la communication TCP/IP dans l'acquisition.
 *   
 */   

#ifndef __ACQ_TCP_PARAMETRES_DEF_H
#define __ACQ_TCP_PARAMETRES_DEF_H


/*  !! Parametres de buffers d'acqusition. !! */
#define ACQ_MAXIMAL_BUFFER_SIZE		131072
#define ACQ_MESSAGE_IDENT		"GAG."
#define ACQ_MESSAGE_MAX_LENGTH		4096
#define ACQ_MESSAGE_POOL_SIZE		16384
#define ACQ_SA_MAX_SESSION		32
#define ACQ_MESSAGE_LENGTH		4096
#define ACQ_USUAL_MESSAGE_LENGTH	512
#define ACQ_ASCII_COMMAND_SIZE		256

/* Nombre maxi de clones pouvant etre executes par le serveur simultanement. */
#define MAXCLONE 			32


/*    IDENTIFICATEUR_SERVEUR est utilise par acq_dn_open dans acq_communication
 * et dans acq_command pour determiner si cet open se fait pour un serveur ou
 * ( SYS$NET ) ou pour un client ( autre indentificateur ). Modifier cette 
 * ligne si des changements interviennent dans la structure d'appel de 
 * acq_dn_open dans ACQ_COMMUNICATION.
 */ 
#define IDENTIFICATEUR_SERVEUR "SYS$NET"    


/*   Parametres definis pour les erreurs traitees par l'ancien
 *  ACQ_VME_IMPRESS.FOR
 */
#define ACQ_COM_OPEN 	1
#define ACQ_COM_WRITE 	2
#define ACQ_COM_READ 	3
#define ACQ_COM_CLOSE 	4
#define ACQ_COM_INIT    5000

#endif
