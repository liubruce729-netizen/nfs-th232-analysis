/*****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  gan_tape_erreur.h                                   *
 *                                                                           *
 *       Auteur       :  OPDEBECK                                            *
 *       Date         :  13 juin 1996                                        *
 *       Modifie le   :   8 juillet 1999                                     *
 *                                                                           *
 *       Objet        :  Fichier entete pour les erreurs.                    *
 *       Modif        : Legeard : reutilisation du fichier .h  de la liste   *
 *                       des codes d'erreur 02 / 2004                        *
 *                                                                           *
 *****************************************************************************/

// ---------------------------------------------------------------------------
// ***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/



#ifndef __DEF_ERR_H
#define __DEF_ERR_H
 

//#define ACQ_OK             0
//#define ACQ_ERRPARAM      -1
//#define ACQ_ISNOTATAPE    -2 
//#define ACQ_NOTALLOC      -3
//#define ACQ_ALRALLOC      -4
//#define ACQ_NOTMOUNT      -5
//#define ACQ_ALREADYMOUNT  -6
//#define ACQ_ENDOFFILE     -7
//#define ACQ_ENDOFTAPE     -8
//#define ACQ_ERRDURREAD    -9
//#define ACQ_DEVWRITLOCK   -10
//#define ACQ_ENDOFBUFFER	  -11
//#define ACQ_INVDATABUF    -12
//#define ACQ_RAFBUFOVF	  -13
//#define ACQ_ERREVNTLEN    -14
//#define ACQ_INVARG        -15
//#define ACQ_ERRDATABUF    -16
//#define ACQ_BADEVENTFORM  -17
#define ACQ_UNKNOWNDEV    -18  	// Unknown device
#define ACQ_NOTOPEN       -19   // No openelsg
#define ACQ_ERRDFT	  -20   // all-purpose error code used for errors not important	
/*#define ACQ_PCAP_ERR	  -21	// error code for error occuring in a libpcap function used in netspy*/

// Erreurs implementees uniquement pour Exemple_relecture	

#define ACQ_UNKBUF        -100 /* Type de buffer inconnu */
#define ACQ_UNKMSB        -101 /* MSB/LSB non determine en mode AutoSwapBuf */
#define ACQ_NOBEGINRUN    -102 /* File is not at begin of run */
#define ACQ_BADFILESTRUCT -103 /* Bad file structure */
#define ACQ_BADEVTSTRUCT  -104 /* Bad event structure in COMMENT block */
#define ACQ_BADRUNNUM     -105 /* Bad run number */
#define ACQ_STREVTTMP     -106 /* Open error on struct event temporary file */
#define ACQ_NOIMPLEMENTED -107 /* functionnality non implemented for this OS */


// Erreurs implementees uniquement pour NetSpy de GRU: redefinition des codes d'erreurs figurant dans netspy.h

#define SPY_FINDDEVICE_ERROR 		-700
#define SPY_OPENDEVICE_ERROR 		-701
#define SPY_CHECKDEVICE_ERROR 		-702
#define SPY_FINDPROTOCOL_ERROR  	-703
#define SPY_COMPILEFILTER_ERROR 	-704
#define SPY_SETFILTER_ERROR 		-705
#define SPY_ALLOCATEMEMORY_ERROR 	-706
#define SPY_GETFRAGMENT_ERROR 		-707
#define SPY_TESTTIME_ERROR 		-708
#define SPY_FINDHEADER_ERROR 		-709
#define SPY_CHECKHEADER_ERROR 		-710

#endif
