/***************************************************************************** 
 *                                                                           *
 *	Fichier		: gan_acq_swap_buf.h                                      *
 *                                                                           *
 *	Auteur		: Bruno Raine                                        *
 *	Date		: 03 dec 96                                          *
 *	Modifie le	:                                                    *
 *                                                                           *
 *	Objet		: Description du package gan_acq_swap_buf            *
 *      Remarques       : Necessite d'inclure avant ce fichier               *
 *                           GEN_TYPE.H et gan_acq_buf.h   
 *      modif Legeard 9/1/2020  rename     SwapIntXX ->   AcqSwapIntXX      *
 *                                                                           *
 ****************************************************************************/
#ifndef __GanAcqSwapBuf_H
#define __GanAcqSwapBuf_H

/* Definition des prototypes */

/* Swappe les buffers d'acquisition GANIL en fonction de leur type et des
   macros d'options definies (en principe dans gan_acq_param.h) */
int acq_swap_buf(in2p3_buffer_struct *Buff, int Size);

/* Routines a usage general pour swapper des entiers 32 et 16 bits 
   Attention pour les deux fonctions qui suivent, aucun control n'est fait 
   sur la valeur de NbOctets. Ce doit etre un multiple de 4 pour SwapInt32 et
   un multiple de 2 pour SwapInt16 */
void AcqSwapInt32(UNSINT32 *Buf, int NbOctets);
void AcqSwapInt16(UNSINT16 *Buf, int NbOctets);

#endif /* Fin GanAcqSwapBuf */
