/*****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  gan_tape_erreur.h                                   *
 *                                                                           *
 *       Auteur       :  OPDEBECK                                            *
 *       Date         :  13 juin 1996                                        *
 *       Modifie le   :   8 juillet 1999                                     *
 *       B. Raine le 4 aout 2001
 *            - remplace ifdef __VMS par __vms_acq 
 *              pour le traitement des erreurs                 
 *                                                                           *
 *       Objet        :  Fichier entete pour les erreurs.                    *
 *                                                                           *
 *****************************************************************************/

#ifndef __DEF_ERR_H
#define __DEF_ERR_H

#ifdef __vms_acq

#include "acq_codes_erreur.h"

#else

#define ACQ_OK             0
#define ACQ_ERRPARAM      -1
#define ACQ_ISNOTATAPE    -2 
#define ACQ_NOTALLOC      -3
#define ACQ_ALRALLOC      -4
#define ACQ_NOTMOUNT      -5
#define ACQ_ALREADYMOUNT  -6
#define ACQ_ENDOFFILE     -7
#define ACQ_ENDOFTAPE     -8
#define ACQ_ERRDURREAD    -9
#define ACQ_DEVWRITLOCK   -10
#define ACQ_ENDOFBUFFER	  -11
#define ACQ_INVDATABUF    -12
#define ACQ_RAFBUFOVF	  -13
#define ACQ_ERREVNTLEN    -14
#define ACQ_INVARG        -15
#define ACQ_ERRDATABUF    -16
#define ACQ_BADEVENTFORM  -17
#define ACQ_NOBUF         -18
#endif

#define ACQ_OK_TXT            "ACQ_OK"
#define ACQ_ERRPARAM_TXT      "ACQ_ERRPARAM"
#define ACQ_ISNOTATAPE_TXT    "ACQ_ISNOTATAPE"
#define ACQ_NOTALLOC_TXT      "ACQ_NOTALLOC"
#define ACQ_ALRALLOC_TXT      "ACQ_ALRALLOC"
#define ACQ_NOTMOUNT_TXT      "ACQ_NOTMOUNT"
#define ACQ_ALREADYMOUNT_TXT  "ACQ_ALREADYMOUNT"
#define ACQ_ENDOFFILE_TXT     "ACQ_ENDOFFILE"
#define ACQ_ENDOFTAPE_TXT     "ACQ_ENDOFTAPE"
#define ACQ_ERRDURREAD_TXT    "ACQ_ERRDURREAD"
#define ACQ_DEVWRITLOCK_TXT   "ACQ_DEVWRITLOCK"
#define ACQ_ENDOFBUFFER_TXT   "ACQ_ENDOFBUFFER"
#define ACQ_INVDATABUF_TXT    "ACQ_INVDATABUF"
#define ACQ_RAFBUFOVF_TXT     "ACQ_RAFBUFOVF"
#define ACQ_ERREVNTLEN_TXT    "ACQ_ERREVNTLEN"
#define ACQ_INVARG_TXT        "ACQ_INVARG"
#define ACQ_ERRDATABUF_TXT    "ACQ_ERRDATABUF"
#define ACQ_BADEVENTFORM_TXT  "ACQ_BADEVENTFORM"
#define ACQ_NOBUF_TXT         "ACQ_NOBUF"




/* Erreurs implementees uniquement pour Exemple_relecture	*/
/* Attention, ils doivent etre utilises a la fois sous UINIX et sous VMS */

#define ACQ_UNKBUF        -100 /* Type de buffer inconnu */
#define ACQ_UNKMSB        -101 /* MSB/LSB non determine en mode AutoSwapBuf */
#define ACQ_NOBEGINRUN    -102 /* File is not at begin of run */
#define ACQ_BADFILESTRUCT -103 /* Bad file structure */
#define ACQ_BADEVTSTRUCT  -104 /* Bad event structure in COMMENT block */
#define ACQ_BADRUNNUM     -105 /* Bad run number */
#define ACQ_STREVTTMP     -106 /* Open error on struct event temporary file */
#define ACQ_NOIMPLEMENTED -107 /* functionnality non implemented for this OS */


#define ACQ_UNKBUF_TXT        "ACQ_UNKBUF"
#define ACQ_UNKMSB_TXT        "ACQ_UNKMSB"
#define ACQ_NOBEGINRUN_TXT    "ACQ_NOBEGINRUN"
#define ACQ_BADFILESTRUCT_TXT "ACQ_BADFILESTRUCT"
#define ACQ_BADEVTSTRUCT_TXT  "ACQ_BADEVTSTRUCT"
#define ACQ_BADRUNNUM_TXT     "ACQ_BADRUNNUM"
#define ACQ_STREVTTMP_TXT     "ACQ_STREVTTMP"
#define ACQ_NOIMPLEMENTED_TXT "ACQ_NOIMPLEMENTED"


void gan_tape_erreur ( int , char * );

const char * GetTextGanError(int error);


#endif


